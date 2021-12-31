// Auto Vote

#include "autovote.h"

#include "../sdk.h"

#include "../game/utils.h"
#include "../game/votepopup.h"
#include "../game/usermsg.h"
#include "../game/console.h"

#include "../utils/hash_table.h"
#include "../utils/vtable_hook.h"
#include "../utils/trampoline_hook.h"
#include "../utils/signature_scanner.h"
#include "../patterns.h"

#include "../config.h"

//-----------------------------------------------------------------------------

CHashTable64<> voteFilter;

CVotePopup *g_pVotePopup = NULL;
CVoteInfo g_VoteInfo;

//cvar_s *autovote_mode = NULL;
//cvar_s *autovote_custom = NULL;
//cvar_s *autovote_ignore_filter = NULL;

bool g_bParsingVoteMessage = false;

//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(MsgFunc_VoteMenu_Hook);
TRAMPOLINE_HOOK(READ_BYTE_Hook);
TRAMPOLINE_HOOK(READ_STRING_Hook);

//-----------------------------------------------------------------------------

MsgFunc_VoteMenuFn MsgFunc_VoteMenu_Original = NULL;

READ_BYTE_Fn READ_BYTE_Original = NULL;
READ_STRING_Fn READ_STRING_Original = NULL;

CVotePopup__OpenFn CVotePopup__Open_Original = NULL;
CVotePopup__CloseFn CVotePopup__Close_Original = NULL;

pfnUserMsgHook UserMsgHook_VoteMenu_Original = NULL;

//-----------------------------------------------------------------------------

static int util_contains_chars(char ch, const char *chars, size_t length)
{
	for (size_t i = 0; i < length; ++i)
	{
		if (chars[i] == ch)
			return 1;
	}

	return 0;
}

static char *util_lstrip(char *str)
{
	while (*str && util_contains_chars(*str, " \t\n", 3))
		++str;

	return str;
}

static void util_rstrip(char *str)
{
	char *end = str + strlen(str) - 1;

	if (end < str)
		return;

	while (end >= str && util_contains_chars(*end, " \t\n", 3))
	{
		*end = '\0';
		--end;
	}
}

static char *util_strip(char *str)
{
	char *result = util_lstrip(str);
	util_rstrip(result);
	return result;
}

static void util_remove_comment(char *str)
{
	while (*str && !util_contains_chars(*str, ";#", 2))
		++str;

	if (*str)
		*str = '\0';
}

//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_reload_autovote_filter, LoadVoteFilter, "sc_reload_autovote_filter - Load SteamID's to filter from file autovote_filter.txt")
{
	FILE *file = fopen("sven_internal/autovote_filter.txt", "r");

	voteFilter.RemoveAll();

	if (file)
	{
		int nAdded = 0;

		static int bufferSize = 256;
		static char *pszFileBuffer = NULL;

		if (!pszFileBuffer)
			pszFileBuffer = (char *)malloc(bufferSize);

		long int endpos;
		fseek(file, 0, SEEK_END);
		endpos = ftell(file);
		rewind(file);

		while (fgets(pszFileBuffer, bufferSize, file))
		{
			size_t length = strlen(pszFileBuffer);

			while (pszFileBuffer[length - 1] != '\n' && ftell(file) != endpos)
			{
				bufferSize *= 2;

				void *realloc_mem = realloc(pszFileBuffer, bufferSize);

				if (!realloc_mem)
				{
					fclose(file);
					return;
				}

				pszFileBuffer = (char *)realloc_mem;
				fgets(pszFileBuffer + length, bufferSize - length, file);

				length = strlen(pszFileBuffer);
			}

			char *str = util_lstrip(pszFileBuffer);
			util_remove_comment(str);
			util_rstrip(str);

			if (!*str)
				continue;

			uint64_t steamID = atoll(str);

			if (steamID & (((uint64_t)1 << 56) | ((uint64_t)1 << 52) | ((uint64_t)1 << 32)))
			{
				voteFilter.AddEntry(steamID, 0);
				++nAdded;
			}
		}

		g_pEngineFuncs->Con_Printf("[AutoVote Filter] Added %d users\n", nAdded);
		fclose(file);
	}
}

//-----------------------------------------------------------------------------

void __fastcall CVotePopup__Open_Hooked(void *thisptr, int edx)
{
	CVotePopup__Open_Original(thisptr);

	int nMode = g_Config.cvars.autovote_mode;

	if (!nMode)
		return;

	switch (g_VoteInfo.m_type)
	{
	case VOTEKILL:
	case VOTEKICK:
	case VOTEBAN:
	{
		bool bOppositeVote = false;

		if (!g_Config.cvars.autovote_ignore_filter)
		{
			bool bNameFound = false;

			char *pszName = NULL;
			char *pszVoteBuffer = g_VoteInfo.m_pszVoteMessage;

			while (*pszVoteBuffer)
			{
				if (*pszVoteBuffer == '"')
				{
					if (!bNameFound)
					{
						pszName = ++pszVoteBuffer;
						bNameFound = true;
					}
					else
					{
						*pszVoteBuffer = 0;
					}
				}

				++pszVoteBuffer;
			}

			if (pszName)
			{
				for (int i = 0; i < 32; ++i)
				{
					player_info_s *pPlayerInfo = g_pEngineStudio->PlayerInfo(i);

					if (pPlayerInfo && !strcmp(pszName, pPlayerInfo->name))
					{
						uint64_t steamID = *(uint64_t *)((PBYTE)pPlayerInfo + 0x248);
						auto filteredSteamID = voteFilter.GetEntry(steamID);

						if (filteredSteamID)
							bOppositeVote = true;

						break;
					}
				}
			}
		}

		if (nMode == 1)
		{
			if (bOppositeVote)
				g_pVotePopup->SlotInput(1);
			else
				g_pVotePopup->SlotInput(0);
		}
		else // if (nMode == 2)
		{
			if (bOppositeVote)
				g_pVotePopup->SlotInput(0);
			else
				g_pVotePopup->SlotInput(1);
		}

		break;
	}

	default:
		if (g_Config.cvars.autovote_custom)
		{
			if (nMode == 1)
				g_pVotePopup->SlotInput(0);
			else // if (nMode == 2)
				g_pVotePopup->SlotInput(1);
		}

		break;
	}
}

void __fastcall CVotePopup__Close_Hooked(void *thisptr, int edx)
{
	CVotePopup__Close_Original(thisptr);

	if (g_VoteInfo.m_pszVoteMessage)
	{
		free(g_VoteInfo.m_pszVoteMessage);
		g_VoteInfo.m_pszVoteMessage = NULL;
	}

	if (g_VoteInfo.m_pszVoteYes)
	{
		free(g_VoteInfo.m_pszVoteYes);
		g_VoteInfo.m_pszVoteYes = NULL;
	}

	if (g_VoteInfo.m_pszVoteNo)
	{
		free(g_VoteInfo.m_pszVoteNo);
		g_VoteInfo.m_pszVoteNo = NULL;
	}
}

int __fastcall MsgFunc_VoteMenu_Hooked(void *thisptr, int edx, int a1, int a2, int a3)
{
	static bool bVTableHooked = false;

	g_bParsingVoteMessage = true;
	g_pVotePopup = reinterpret_cast<CVotePopup *>(thisptr);

	if (!bVTableHooked)
	{
		CVotePopup__Open_Original = (CVotePopup__OpenFn)CVTableHook::HookFunction(g_pVotePopup, CVotePopup__Open_Hooked, 128);
		CVotePopup__Close_Original = (CVotePopup__CloseFn)CVTableHook::HookFunction(g_pVotePopup, CVotePopup__Close_Hooked, 129);

		bVTableHooked = true;
	}

	MsgFunc_VoteMenu_Original(thisptr, a1, a2, a3);

	g_bParsingVoteMessage = false;

	return 1; // always 1
}

int UserMsgHook_VoteMenu(const char *pszUserMsg, int iSize, void *pBuffer)
{
	BEGIN_READ(pBuffer, iSize);

	g_VoteInfo.m_type = static_cast<VoteType>(READ_BYTE());

	g_VoteInfo.m_pszVoteMessage = strdup(READ_STRING());
	g_VoteInfo.m_pszVoteYes = strdup(READ_STRING());
	g_VoteInfo.m_pszVoteNo = strdup(READ_STRING());

	return UserMsgHook_VoteMenu_Original(pszUserMsg, iSize, pBuffer);
}

//-----------------------------------------------------------------------------

void InitAutoVote()
{
	void *pMsgFunc_VoteMenu = FIND_PATTERN(L"client.dll", Patterns::Client::CVotePopup__MsgFunc_VoteMenu);

	if (!pMsgFunc_VoteMenu)
	{
		ThrowError("CVotePopup::MsgFunc_VoteMenu failed initialization\n");
		return;
	}

	void *pREAD_BYTE = FIND_PATTERN(L"client.dll", Patterns::Client::READ_BYTE);

	if (!pREAD_BYTE)
	{
		ThrowError("READ_BYTE failed initialization\n");
		return;
	}

	void *pREAD_STRING = FIND_PATTERN(L"client.dll", Patterns::Client::READ_STRING);

	if (!pREAD_STRING)
	{
		ThrowError("READ_STRING failed initialization\n");
		return;
	}

	HOOK_FUNCTION(MsgFunc_VoteMenu_Hook, pMsgFunc_VoteMenu, MsgFunc_VoteMenu_Hooked, MsgFunc_VoteMenu_Original, MsgFunc_VoteMenuFn);

	UserMsgHook_VoteMenu_Original = HookUserMsg("VoteMenu", UserMsgHook_VoteMenu);

	LoadVoteFilter();
}

/*

int READ_BYTE_Hooked()
{
	int result = READ_BYTE_Original();

	if (g_bParsingVoteMessage)
	{
		g_VoteInfo.m_type = static_cast<VoteType>(result);
	}

	return result;
}

char *READ_STRING_Hooked()
{
	char *result = READ_STRING_Original();

	if (g_bParsingVoteMessage)
	{
		static int msg_sequence = 0;

		if (msg_sequence == 0)
		{
			if (*result)
				g_VoteInfo.m_pszVoteMessage = strdup(result);
		}
		else if (msg_sequence == 1)
		{
			if (*result)
				g_VoteInfo.m_pszVoteYes = strdup(result);
		}
		else if (msg_sequence == 2)
		{
			if (*result)
				g_VoteInfo.m_pszVoteNo = strdup(result);
		}

		++msg_sequence;

		if (msg_sequence > 2)
			msg_sequence = 0;
	}

	return result;
}

Init:
	//HOOK_FUNCTION(READ_BYTE_Hook, pREAD_BYTE, READ_BYTE_Hooked, READ_BYTE_Original, READ_BYTE_Fn);
	//HOOK_FUNCTION(READ_STRING_Hook, pREAD_STRING, READ_STRING_Hooked, READ_STRING_Original, READ_STRING_Fn);
	//autovote_mode = REGISTER_CVAR("sc_autovote_mode", "0", 0);
	//autovote_custom = REGISTER_CVAR("sc_autovote_custom", "1", 0);
	//autovote_ignore_filter = REGISTER_CVAR("sc_autovote_ignore_filter", "0", 0);

*/