// Chat Colors

#include "chat_colors.h"

#include "../sdk.h"
#include "../config.h"

#include "../game/utils.h"
#include "../game/console.h"
#include "../game/player_utils.h"

#include "trampoline_hook.h"

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef float *(__cdecl *GetClientColorFn)(int);

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern extra_player_info_t *g_pPlayerExtraInfo;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CChatColors g_ChatColors;

void *pGetClientColor = NULL;

GetClientColorFn GetClientColor_Original = NULL;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(GetClientColor_Hook);

//-----------------------------------------------------------------------------
// Utilities for parsing files
//-----------------------------------------------------------------------------

#define CC_COMMENT_PREFIX ";#"
#define CC_STRIP_CHARS (" \t\n")

#define CC_STRIP_CHARS_LEN (sizeof(CC_STRIP_CHARS) - 1)
#define CC_COMMENT_PREFIX_LEN (sizeof(CC_COMMENT_PREFIX) - 1)

static int cc_contains_chars(char ch, const char *chars, size_t length)
{
	for (size_t i = 0; i < length; ++i)
	{
		if (chars[i] == ch)
			return 1;
	}

	return 0;
}

static char *cc_lstrip(char *str)
{
	while (*str && cc_contains_chars(*str, CC_STRIP_CHARS, CC_STRIP_CHARS_LEN))
		++str;

	return str;
}

static void cc_rstrip(char *str)
{
	char *end = str + strlen(str) - 1;

	if (end < str)
		return;

	while (end >= str && cc_contains_chars(*end, CC_STRIP_CHARS, CC_STRIP_CHARS_LEN))
	{
		*end = '\0';
		--end;
	}
}

static char *cc_strip(char *str)
{
	char *result = cc_lstrip(str);
	cc_rstrip(result);
	return result;
}

static void cc_remove_comment(char *str)
{
	while (*str && !cc_contains_chars(*str, CC_COMMENT_PREFIX, CC_COMMENT_PREFIX_LEN))
		++str;

	if (*str)
		*str = '\0';
}

static bool cc_is_assignment(char c)
{
	return c == ':';
}

static bool cc_is_whitespace(char c)
{
	return c == ' ' || c == '\t';
}

static bool cc_is_digit(char c)
{
	return c >= '0' && c <= '9';
}

static bool cc_is_forbidden_symbol(char c)
{
	return !cc_is_whitespace(c) && !cc_is_digit(c) && !cc_is_assignment(c);
}

//-----------------------------------------------------------------------------
// ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_chat_colors_load_players,
				 ConCommand_ChatColorsLoadPlayers,
				 "sc_chat_colors_load_players - loads list of Steam ID's from file 'sven_internal/chat_colors_players.txt'")
{
	g_ChatColors.LoadPlayers();
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

float *GetClientColor_Hooked(int playerIndex)
{
	if (playerIndex > 0)
	{
		int nTeamNumber = g_pPlayerExtraInfo[playerIndex].teamnumber;
		int *nUserColorNumber = g_ChatColors.FindPlayerInList(playerIndex);

		if (nUserColorNumber)
		{
			switch (*nUserColorNumber)
			{
			case 1:
				return g_Config.cvars.chat_color_one;

			case 2:
				return g_Config.cvars.chat_color_two;

			case 3:
				return g_Config.cvars.chat_color_three;

			case 4:
				return g_Config.cvars.chat_color_four;

			case 5:
				return g_Config.cvars.chat_color_five;
			}
		}
		else if (nTeamNumber < 1 || nTeamNumber > 4)
		{
			return g_Config.cvars.player_name_color;
		}
	}

	return GetClientColor_Original(playerIndex);
}

//-----------------------------------------------------------------------------
// CChatColors implementations
//-----------------------------------------------------------------------------

CChatColors::CChatColors() : m_HashTable(16)
{
}

void CChatColors::LoadPlayers()
{
	static char szBuffer[512];
	FILE *file = fopen("sven_internal/chat_colors_players.txt", "r");

	if (file)
	{
		int nLine = 0;
		token_state state = TOKEN_READ_OK;

		m_HashTable.Clear();

		while (fgets(szBuffer, sizeof(szBuffer), file))
		{
			nLine++;

			uint64_t steamID = 0;
			int colornum = 0;

			// Format >> ( STEAMID : TEAM_NUMBER )
			if ( (state = ReadToken(&steamID, &colornum, szBuffer)) == TOKEN_READ_OK )
			{
				if (colornum < 1 || colornum > 5)
				{
					Msg("[Chat Colors] Invalid color number!\n");
				}
				else
				{
					if (steamID & (((uint64_t)1 << 56) | ((uint64_t)1 << 52) | ((uint64_t)1 << 32)))
						m_HashTable.Insert(steamID, colornum);
					else
						Msg("[Chat Colors] Trying to insert invalid Steam ID!\n");
				}

			#ifdef _DEBUG
				Msg(">> %llu : %d\n", steamID, colornum);
			#endif
			}
			else if (state == TOKEN_READ_FAILED) // TOKEN_READ_FAILED
			{
				Msg("[Chat Colors] Failed to parse file 'sven_internal/chat_colors_players.txt' at line %d\n", nLine);
				break;
			}
		}

		if (state != TOKEN_READ_FAILED)
		{
			//Msg("[Chat Colors] Successfully parsed file 'sven_internal/chat_colors_players.txt'\n");
		}

		fclose(file);
	}
	else
	{
		Msg("[Chat Colors] Missing file 'sven_internal/chat_colors_players.txt'\n");
	}
}

CChatColors::token_state CChatColors::ReadToken(uint64_t *steamID, int *colornum, const char *pszBuffer)
{
	int read_state = TOKEN_READ_STEAMID;

	char *buffer = cc_lstrip((char *)pszBuffer);
	cc_remove_comment(buffer);
	cc_rstrip(buffer);

	if (!*buffer) // empty
		return TOKEN_READ_EMPTY_BUFFER;

	bool bFoundSteamID = false;

	char *pszCurrentPosition = buffer;
	char *pszColorNumber = NULL;

	for (int i = 0; i < 3; i++)
	{
		if (read_state == TOKEN_READ_STEAMID)
		{
			while (*pszCurrentPosition)
			{
				if ( cc_is_forbidden_symbol(*pszCurrentPosition) )
				{
					Msg("[Chat Colors] Unrecognized symbol\n");
					return TOKEN_READ_FAILED;
				}

				if ( cc_is_whitespace(*pszCurrentPosition) || cc_is_assignment(*pszCurrentPosition) )
				{
					bool bAssignment = cc_is_assignment(*pszCurrentPosition);
					*pszCurrentPosition = 0;

					*steamID = (uint64_t)strtoll(buffer, NULL, 10);

					read_state = TOKEN_READ_ASSIGNMENT;

					if ( bAssignment )
						*pszCurrentPosition = ':';
					else
						pszCurrentPosition++;

					break;
				}

				pszCurrentPosition++;
			}

			if (!*pszCurrentPosition)
			{
				Msg("[Chat Colors] Failed to find Steam ID and an assignment symbol\n");
				goto EOF_REACHED;
			}
		}
		else if (read_state == TOKEN_READ_ASSIGNMENT)
		{
			while (*pszCurrentPosition)
			{
				if ( cc_is_forbidden_symbol(*pszCurrentPosition) )
				{
					Msg("[Chat Colors] Unrecognized symbol\n");
					return TOKEN_READ_FAILED;
				}

				if ( cc_is_digit(*pszCurrentPosition) )
				{
					Msg("[Chat Colors] Unexpectedly found a digit instead of an assignment symbol\n");
					return TOKEN_READ_FAILED;
				}

				if ( cc_is_assignment(*pszCurrentPosition) )
				{
					pszCurrentPosition++;

					read_state = TOKEN_READ_TEAM_NUMBER;

					break;
				}

				pszCurrentPosition++;
			}

			if (!*pszCurrentPosition)
			{
				Msg("[Chat Colors] Failed to find an assignment symbol and team number\n");
				goto EOF_REACHED;
			}
		}
		else if (read_state == TOKEN_READ_TEAM_NUMBER)
		{
			while (*pszCurrentPosition)
			{
				if ( cc_is_forbidden_symbol(*pszCurrentPosition) )
				{
					Msg("[Chat Colors] Unrecognized symbol\n");
					return TOKEN_READ_FAILED;
				}

				if ( cc_is_assignment(*pszCurrentPosition) )
				{
					Msg("[Chat Colors] Found repeated assignment symbol\n");
					return TOKEN_READ_FAILED;
				}

				if ( pszColorNumber && cc_is_whitespace(*pszCurrentPosition) )
				{
					Msg("[Chat Colors] Found white space symbol after team number\n");
					return TOKEN_READ_FAILED;
				}

				if ( cc_is_digit(*pszCurrentPosition) )
				{
					if (!pszColorNumber)
						pszColorNumber = pszCurrentPosition;
				}

				pszCurrentPosition++;
			}

			if (pszColorNumber)
			{
				*colornum = atoi(pszColorNumber);
				return TOKEN_READ_OK;
			}
			else
			{
				Msg("[Chat Colors] Failed to find team number\n");
				break;
			}
		}
	}

EOF_REACHED:
	Msg("[Chat Colors] Reached end of file without result\n");
	return TOKEN_READ_FAILED;
}

int *CChatColors::FindPlayerInList(int playerIndex)
{
	return m_HashTable.Find( GetPlayerSteamID(playerIndex) );
}

//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------

void CChatColors::Init()
{
	HOOK_FUNCTION(GetClientColor_Hook, pGetClientColor, GetClientColor_Hooked, GetClientColor_Original, GetClientColorFn);

	LoadPlayers();
}