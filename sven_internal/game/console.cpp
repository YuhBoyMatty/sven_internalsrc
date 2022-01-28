// Console Utils

#include "console.h"
#include "utils.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CConCommandBase *CConCommandBase::s_pConCommandBaseList = NULL;

void ConVar_Register()
{
	CConCommandBase *pCurrent = CConCommandBase::s_pConCommandBaseList;

	while (pCurrent)
	{
		pCurrent->m_bRegistered = true;
		pCurrent->Register();
		pCurrent = pCurrent->GetNext();
	}
}

void ConVar_Unregister()
{
}

void PrintConsoleHelp()
{
	Msg("=================================== Console Help ===================================\n");

	CConCommandBase *pConCommandBase = CConCommandBase::s_pConCommandBaseList;

	while (pConCommandBase)
	{
		switch (pConCommandBase->GetType())
		{
		case CONSOLE_VARIABLE:
			Msg("Variable: %s    Description: %s\n", pConCommandBase->GetName(), pConCommandBase->GetHelpText());
			break;

		case CONSOLE_COMMAND:
			Msg("Command: %s    Description: %s\n", pConCommandBase->GetName(), pConCommandBase->GetHelpText());
			break;

		case CONSOLE_COMMAND_TOGGLE:
			Msg("Toggle Command: %s    Description: %s\n", pConCommandBase->GetName(), pConCommandBase->GetHelpText());
			break;
		}

		pConCommandBase = pConCommandBase->GetNext();
	}

	Msg("=================================== Console Help ===================================\n");
}

//-----------------------------------------------------------------------------
// CConCommandBase
//-----------------------------------------------------------------------------

CConCommandBase::CConCommandBase()
{
	m_pNext = NULL;

	m_pszName = NULL;
	m_pszHelpText = NULL;

	m_bRegistered = false;
}

CConCommandBase::CConCommandBase(const char *pszName, const char *pszDescription)
{
	Create(pszName, pszDescription);
}

CConCommandBase::~CConCommandBase()
{
}

void CConCommandBase::Create(const char *pszName, const char *pszDescription)
{
	m_pNext = s_pConCommandBaseList;
	s_pConCommandBaseList = this;

	m_pszName = pszName;
	m_pszHelpText = pszDescription;

	m_bRegistered = false;
}

void CConCommandBase::Register()
{
}

void CConCommandBase::Unregister()
{
}

console_object_t CConCommandBase::GetType() const
{
	return CONSOLE_UNDEFINED;
}

const char *CConCommandBase::GetName() const
{
	return m_pszName;
}

const char *CConCommandBase::GetHelpText() const
{
	return m_pszHelpText;
}

void CConCommandBase::PrintUsage() const
{
	Msg("Usage: %s\n", m_pszHelpText);
}

//-----------------------------------------------------------------------------
// CConCommand
//-----------------------------------------------------------------------------

CConCommand::CConCommand()
{
	m_pfnCallback = NULL;
}

CConCommand::CConCommand(const char *pszName, FnConCommandCallback pfnCallback, const char *pszDescription)
{
	m_pfnCallback = pfnCallback;

	CConCommandBase::Create(pszName, pszDescription);
}

CConCommand::~CConCommand()
{
}

void CConCommand::Register()
{
	if (m_pfnCallback && m_pszName)
		g_pEngineFuncs->pfnAddCommand(m_pszName, m_pfnCallback);
}

void CConCommand::Unregister()
{
}

console_object_t CConCommand::GetType() const
{
	return CONSOLE_COMMAND;
}

//-----------------------------------------------------------------------------
// CConToggleCommand
//-----------------------------------------------------------------------------

CConToggleCommand::CConToggleCommand()
{
	m_pszKeyDownName = NULL;
	m_pszKeyUpName = NULL;

	m_pfnKeyDownCallback = NULL;
	m_pfnKeyUpCallback = NULL;
}

CConToggleCommand::CConToggleCommand(const char *pszName, const char *pszKeyDownName, const char *pszKeyUpName,
									 FnConCommandCallback pfnKeyDownCallback, FnConCommandCallback pfnKeyUpCallback, const char *pszDescription)
{
	m_pszKeyDownName = pszKeyDownName;
	m_pszKeyUpName = pszKeyUpName;

	m_pfnKeyDownCallback = pfnKeyDownCallback;
	m_pfnKeyUpCallback = pfnKeyUpCallback;

	CConCommandBase::Create(pszName, pszDescription);
}

CConToggleCommand::~CConToggleCommand()
{
}

void CConToggleCommand::Register()
{
	if (m_pszKeyDownName && m_pfnKeyDownCallback)
		g_pEngineFuncs->pfnAddCommand(m_pszKeyDownName, m_pfnKeyDownCallback);

	if (m_pszKeyUpName && m_pfnKeyUpCallback)
		g_pEngineFuncs->pfnAddCommand(m_pszKeyUpName, m_pfnKeyUpCallback);
}

void CConToggleCommand::Unregister()
{
}

console_object_t CConToggleCommand::GetType() const
{
	return CONSOLE_COMMAND_TOGGLE;
}

//-----------------------------------------------------------------------------
// CConVar
//-----------------------------------------------------------------------------

CConVar::CConVar()
{
	m_pszDefaultValue = NULL;
	m_pCVar = NULL;
}

CConVar::CConVar(const char *pszName, const char *pszDefaultValue, const char *pszDescription)
{
	m_pszDefaultValue = pszDefaultValue;
	m_pCVar = NULL;

	CConCommandBase::Create(pszName, pszDescription);
}

CConVar::~CConVar()
{
}

void CConVar::Register()
{
	if (m_pszName && m_pszDefaultValue)
		m_pCVar = g_pEngineFuncs->pfnRegisterVariable(m_pszName, m_pszDefaultValue, 0);
}

void CConVar::Unregister()
{
}

console_object_t CConVar::GetType() const
{
	return CONSOLE_VARIABLE;
}