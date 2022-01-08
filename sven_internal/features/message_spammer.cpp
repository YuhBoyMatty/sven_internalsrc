// Message Spammer

#include <regex>

#include "message_spammer.h"

#include "../interfaces.h"
#include "../game/utils.h"
#include "../game/console.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

CMessageSpammer g_MessageSpammer;

//-----------------------------------------------------------------------------
// ConCommands & ConVars
//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_ms_add, ConCommand_AddSpamTask, "sc_ms_add [taskname] - Add spam task from file ../sven_internal/message_spammer/[taskname].txt")
{
	if (CMD_ARGC() >= 2)
		g_MessageSpammer.AddTask(CMD_ARGV(1));
}

CON_COMMAND_FUNC(sc_ms_remove, ConCommand_RemoveSpamTask, "sc_ms_remove [taskname] - Remove spam task by name")
{
	if (CMD_ARGC() >= 2)
		g_MessageSpammer.RemoveTask(CMD_ARGV(1));
}

CON_COMMAND_FUNC(sc_ms_reload, ConCommand_ReloadSpamTask, "sc_ms_reload [taskname] - Reload spam task by name")
{
	if (CMD_ARGC() >= 2)
		g_MessageSpammer.ReloadTask(CMD_ARGV(1));
}

CON_COMMAND_FUNC(sc_ms_keywords, ConCommand_PrintSpamKeyWords, "sc_ms_keywords - Prints all keywords")
{
	g_pEngineFuncs->Con_Printf("[Message Spammer] Keywords:\n");
	g_pEngineFuncs->Con_Printf("loop | must be defined at the beginning\n");
	g_pEngineFuncs->Con_Printf("send [message] | send a given message to the game chat\n");
	g_pEngineFuncs->Con_Printf("sleep [delay] | pause a running task for a given delay\n");
}

CON_COMMAND_FUNC(sc_ms_print, ConCommand_PrintSpamTasks, "sc_ms_print - Print all spam tasks")
{
	g_MessageSpammer.PrintTasks();
}

CONVAR(sc_ms_debug, "0", "sc_ms_debug [0/1] - Enable debugging for Message Spammer");

//-----------------------------------------------------------------------------
// CMessageSpammer
//-----------------------------------------------------------------------------

CMessageSpammer::CMessageSpammer()
{
}

CMessageSpammer::~CMessageSpammer()
{
	for (size_t i = 0; i < m_tasks.size(); ++i)
	{
		CSpamTask *pTask = m_tasks[i];
		delete pTask;
	}

	m_tasks.clear();
}

void CMessageSpammer::Init()
{
}

void CMessageSpammer::CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	RunTasks();
}

void CMessageSpammer::RunTasks()
{
	for (size_t i = 0; i < m_tasks.size(); ++i)
	{
		CSpamTask *pTask = m_tasks[i];

		if (!pTask->Run())
		{
			m_tasks.erase(m_tasks.begin() + i);
			delete pTask;
			--i;
		}
	}
}

void CMessageSpammer::PrintTasks()
{
	g_pEngineFuncs->Con_Printf("[Message Spammer] Spam tasks list:\n");

	for (size_t i = 0; i < m_tasks.size(); ++i)
	{
		CSpamTask *pTask = m_tasks[i];

		g_pEngineFuncs->Con_Printf("%d: %s (loop: %d, waiting: %d)\n", i, pTask->GetName(), pTask->IsLooped(), pTask->IsWaiting());
	}
}

bool CMessageSpammer::AddTask(const char *pszTaskName)
{
	if (GetTask(pszTaskName))
		return ReloadTask(pszTaskName);

	static char buffer[512];

	sprintf_s(buffer, sizeof(buffer), "sven_internal/message_spammer/%s.txt", pszTaskName);

	FILE *file = fopen(buffer, "r");

	if (file)
	{
		int nLine = 0;

		bool bLoopVarFound = false;
		bool bParsingOperators = false;
		bool bDebug = static_cast<bool>(sc_ms_debug.GetCVar()->value);

		std::regex regex_loop("^loop[\n]{0,1}$");
		std::regex regex_send("^send (.+)[\n]{0,1}$");
		std::regex regex_sleep("^sleep ([0-9.]+)[\n]{0,1}$");

		CSpamTask *pTask = new CSpamTask(pszTaskName);

		if (bDebug)
			g_pEngineFuncs->Con_Printf("< Parsing task: %s >\n", pszTaskName);

		while (fgets(buffer, sizeof(buffer), file))
		{
			std::cmatch match;
			nLine++;

			if (!bParsingOperators && !bLoopVarFound)
			{
				if (std::regex_search(buffer, match, regex_loop))
				{
					if (bDebug)
						g_pEngineFuncs->Con_Printf("[%d] Found action | loop\n", nLine);

					bLoopVarFound = true;
					continue;
				}
			}

			if (std::regex_search(buffer, match, regex_send))
			{
				if (bDebug)
					g_pEngineFuncs->Con_Printf("[%d] Found action | send %s\n", nLine, match[1].str().c_str());

				CSpamOperatorSend *pOperator = new CSpamOperatorSend();

				pOperator->SetOperand(match[1].str().c_str());
				pTask->AddOperator(reinterpret_cast<ISpamOperator *>(pOperator));

				bParsingOperators = true;
			}
			else if (std::regex_search(buffer, match, regex_sleep))
			{
				if (bDebug)
					g_pEngineFuncs->Con_Printf("[%d] Found action | sleep %s\n", nLine, match[1].str().c_str());

				CSpamOperatorSleep *pOperator = new CSpamOperatorSleep();

				pOperator->SetOperand(strtof(match[1].str().c_str(), NULL));
				pTask->AddOperator(reinterpret_cast<ISpamOperator *>(pOperator));

				bParsingOperators = true;
			}
		}

		if (bDebug)
			g_pEngineFuncs->Con_Printf("< Parsing finished >\n");

		pTask->SetLoop(bLoopVarFound);
		m_tasks.push_back(pTask);

		g_pEngineFuncs->Con_Printf("[Message Spammer] Spam task %s successfully parsed\n", pszTaskName);

		fclose(file);
		return true;
	}
	else
	{
		g_pEngineFuncs->Con_Printf("[Message Spammer] Failed to open file called %s.txt\n", pszTaskName);
	}

	return false;
}

bool CMessageSpammer::ReloadTask(const char *pszTaskName)
{
	bool bReloaded = RemoveTask(pszTaskName) && AddTask(pszTaskName);

	if (bReloaded)
		g_pEngineFuncs->Con_Printf("[Message Spammer] Spam task %s has been reloaded\n", pszTaskName);
	else
		g_pEngineFuncs->Con_Printf("[Message Spammer] Failed to reload spam task %s\n", pszTaskName);

	return bReloaded;
}

bool CMessageSpammer::RemoveTask(const char *pszTaskName)
{
	for (size_t i = 0; i < m_tasks.size(); ++i)
	{
		CSpamTask *pTask = m_tasks[i];
		const char *pszName = pTask->GetName();

		if (pszName && !strcmp(pszName, pszTaskName))
		{
			m_tasks.erase(m_tasks.begin() + i);
			delete pTask;

			g_pEngineFuncs->Con_Printf("[Message Spammer] Spam task %s has been removed\n", pszTaskName);

			return true;
		}
	}

	return false;
}

CSpamTask *CMessageSpammer::GetTask(const char *pszTaskName)
{
	for (size_t i = 0; i < m_tasks.size(); ++i)
	{
		CSpamTask *pTask = m_tasks[i];
		const char *pszName = pTask->GetName();

		if (pszName && !strcmp(pszName, pszTaskName))
			return pTask;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// CSpamTask
//-----------------------------------------------------------------------------

CSpamTask::CSpamTask(const char *pszName)
{
	m_pszName = (const char *)strdup(pszName);
	m_bLoop = false;
	m_iOperatorBegin = 0;
}

CSpamTask::~CSpamTask()
{
	if (m_pszName)
		free((void *)m_pszName);

	for (size_t i = 0; i < m_operators.size(); ++i)
	{
		ISpamOperator *pOperator = m_operators[i];
		delete pOperator;
	}

	m_operators.clear();
}

bool CSpamTask::Run()
{
	size_t i;

	if (IsFinished())
		return false;

	for (i = m_iOperatorBegin; i < m_operators.size(); ++i)
	{
		ISpamOperator *pOperator = m_operators[i];

		m_iOperatorBegin = i;

		if (IsWaiting())
			return true;

		pOperator->Run(m_spamInfo);
	}

	if (i == m_operators.size())
	{
		if (m_bLoop)
			m_iOperatorBegin = 0; // go through the operators list again
		else
			m_iOperatorBegin = m_operators.size();
	}

	return true;
}

bool CSpamTask::IsWaiting()
{
	return g_pEngineFuncs->Sys_FloatTime() < m_spamInfo.flNextRunTime;
}

bool CSpamTask::IsFinished()
{
	return (!m_bLoop && m_operators.size() == m_iOperatorBegin) || m_operators.size() == 0;
}

bool CSpamTask::IsLooped()
{
	return m_bLoop;
}

void CSpamTask::SetLoop(bool bLoop)
{
	m_bLoop = bLoop;
}

void CSpamTask::ResetWaiting()
{
	m_spamInfo.flNextRunTime = 0.f;
}

void CSpamTask::AddOperator(ISpamOperator *pOperator)
{
	m_operators.push_back(pOperator);
}

//-----------------------------------------------------------------------------
// CSpamOperatorSend
//-----------------------------------------------------------------------------

CSpamOperatorSend::CSpamOperatorSend()
{
	m_pszMessage = NULL;
}

CSpamOperatorSend::~CSpamOperatorSend()
{
	if (m_pszMessage)
		free((void *)m_pszMessage);
}

void CSpamOperatorSend::Run(CSpamInfo &spamInfo)
{
	static char command_buffer[256];

	sprintf_s(command_buffer, sizeof(command_buffer), "say %s", m_pszMessage);
	g_pEngineFuncs->pfnClientCmd(command_buffer);
}

void CSpamOperatorSend::SetOperand(const char *pszMessage)
{
	m_pszMessage = (const char *)strdup(pszMessage);
}

//-----------------------------------------------------------------------------
// CSpamOperatorSleep
//-----------------------------------------------------------------------------

CSpamOperatorSleep::CSpamOperatorSleep()
{
	m_flSleepDelay = 0.f;
}

void CSpamOperatorSleep::Run(CSpamInfo &spamInfo)
{
	spamInfo.flNextRunTime = g_pEngineFuncs->Sys_FloatTime() + m_flSleepDelay;
}

void CSpamOperatorSleep::SetOperand(float flSleepDelay)
{
	m_flSleepDelay = flSleepDelay;
}