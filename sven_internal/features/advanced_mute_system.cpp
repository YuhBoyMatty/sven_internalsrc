// Advanced Mute System

#include "advanced_mute_system.h"

#include <stdio.h>
#include <stdint.h>

#include "../interfaces.h"
#include "../game/utils.h"
#include "../game/console.h"

#include "../patterns.h"

#include "../utils/hash_table.h"
#include "../utils/signature_scanner.h"
#include "../utils/trampoline_hook.h"

#include "../config.h"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

// Mute flags
#define MUTE_NONE ( 0 )
#define MUTE_VOICE ( 0x10 )
#define MUTE_CHAT ( 0x20 )
#define MUTE_ALL ( MUTE_VOICE | MUTE_CHAT )

// Database specifics
#define AMS_VERSION ( 1 )
#define AMS_HEADER ( 0x2F77 )

// Hash table size
#define HASH_TABLE_SIZE ( 256 )

// Make sure we're processing valid player
#define GUARD_VALIDATE_PLAYER_INDEX(idx) \
	if ((idx) < 1 || (idx) > 32) return; \
	auto pLocal = g_pEngineFuncs->GetLocalPlayer(); \
	if (!pLocal || pLocal->index == (idx)) return;

//-----------------------------------------------------------------------------
// Global vars
//-----------------------------------------------------------------------------

bool g_bProcessingChat = false;
int g_nLastIndexedPlayer = -1;

// Init hash table
CHashTable64<HASH_TABLE_SIZE, uint32_t> g_MutedPlayers;

// Database's stream
FILE *g_pFileDB = NULL;

// Mask of banned clients
uint32_t g_BanMask = 0;

// Cvars
//cvar_s *mute_everything = NULL;
cvar_s *voice_clientdebug = NULL;
cvar_s *voice_modenable = NULL;

player_info_s *g_pLastPlayer = NULL;

//-----------------------------------------------------------------------------
// Init hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(Print_Hook);
TRAMPOLINE_HOOK(SaveState_Hook);
TRAMPOLINE_HOOK(SetPlayerBan_Hook);
TRAMPOLINE_HOOK(InternalFindPlayerSquelch_Hook);
TRAMPOLINE_HOOK(IsPlayerBlocked_Hook);
TRAMPOLINE_HOOK(SetPlayerBlockedState_Hook);
TRAMPOLINE_HOOK(UpdateServerState_Hook);
TRAMPOLINE_HOOK(HACK_GetPlayerUniqueID_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

PrintFn Print_Original = NULL;
SaveStateFn SaveState_Original = NULL;
SetPlayerBanFn SetPlayerBan_Original = NULL;
InternalFindPlayerSquelchFn InternalFindPlayerSquelch_Original = NULL;
IsPlayerBlockedFn IsPlayerBlocked_Original = NULL;
SetPlayerBlockedStateFn SetPlayerBlockedState_Original = NULL;
UpdateServerStateFn UpdateServerState_Original = NULL;
HACK_GetPlayerUniqueIDFn HACK_GetPlayerUniqueID_Original = NULL;
GetPlayerUniqueIDFn GetPlayerUniqueID_Original = NULL;

//-----------------------------------------------------------------------------
// Purpose: get player's steamid
//-----------------------------------------------------------------------------

inline uint64_t GetSteamID(int nPlayerIndex)
{
	g_pLastPlayer = g_pEngineStudio->PlayerInfo(nPlayerIndex - 1); // array of elements

	if (!g_pLastPlayer)
		return 0;

	return *reinterpret_cast<uint64_t *>((BYTE *)g_pLastPlayer + 0x248);
}

inline uint64_t GetClientSteamID(int nClient) // a
{
	g_pLastPlayer = g_pEngineStudio->PlayerInfo(nClient);

	if (!g_pLastPlayer)
		return 0;

	return *reinterpret_cast<uint64_t *>((BYTE *)g_pLastPlayer + 0x248);
}

//-----------------------------------------------------------------------------
// Purpose: load muted players in hash table from file muted_players.db
//-----------------------------------------------------------------------------

void LoadMutedPlayers()
{
	g_pFileDB = fopen("sven_internal/muted_players.db", "rb");

	if (g_pFileDB)
	{
		int buffer = 0;

		fread(&buffer, 1, sizeof(short), g_pFileDB);

		if (buffer != AMS_HEADER)
		{
			g_pEngineFuncs->Con_Printf("[AMS] Error: invalid format of file ../sven_internal/muted_players.db\n");
			return;
		}

		buffer = 0;
		fread(&buffer, 1, sizeof(char), g_pFileDB);

		if (buffer < 1)
		{
			g_pEngineFuncs->Con_Printf("[AMS] Error: invalid version\n");
			return;
		}

		static struct MutedPlayerEntry
		{
			uint32_t steamid_pair1;
			uint32_t steamid_pair2;
			uint32_t flags;
		} s_MutedPlayerBuffer;

		while (fread(&s_MutedPlayerBuffer, 1, sizeof(MutedPlayerEntry), g_pFileDB) == sizeof(MutedPlayerEntry))
		{
			uint64_t steamid = *reinterpret_cast<uint64_t *>(&s_MutedPlayerBuffer.steamid_pair1);

			g_MutedPlayers.AddEntry(steamid, s_MutedPlayerBuffer.flags);
		}

		fclose(g_pFileDB);
	}
	else
	{
		g_pEngineFuncs->Con_Printf("[AMS] Warning: missing file ../sven_internal/muted_players.db\n");
	}

	g_pFileDB = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: save hash table in file muted_players.db
//-----------------------------------------------------------------------------

void IterateMutedPlayers(void *entry)
{
	CHashEntry64<uint32_t> *pEntry = reinterpret_cast<CHashEntry64<uint32_t> *>(entry);

	fwrite(&pEntry->key, 1, sizeof(uint64_t), g_pFileDB);
	fwrite(&pEntry->value, 1, sizeof(uint32_t), g_pFileDB);
}

void SaveMutedPlayers()
{
	g_pFileDB = fopen("muted_players.db", "wb");

	if (g_pFileDB)
	{
		int buffer = 0;

		buffer = AMS_HEADER;
		fwrite(&buffer, 1, sizeof(short), g_pFileDB);

		buffer = AMS_VERSION;
		fwrite(&buffer, 1, sizeof(char), g_pFileDB);

		g_MutedPlayers.IterateEntries(IterateMutedPlayers);

		fclose(g_pFileDB);
	}
	else
	{
		g_pEngineFuncs->Con_Printf("[AMS] Error: cannot create file muted_players.db\n");
	}

	g_pFileDB = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: remove all muted players from hash table and clear it
//-----------------------------------------------------------------------------

void RemoveMutedPlayers()
{
	g_MutedPlayers.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: get muted player from hash table
//-----------------------------------------------------------------------------

CHashEntry64<uint32_t> *__fastcall GetMutedPlayer(uint64_t steamid)
{
	return g_MutedPlayers.GetEntry(steamid);
}

//-----------------------------------------------------------------------------
// Purpose: add player in hash table, or merge flags if player already in table
//-----------------------------------------------------------------------------

void OnPlayerInTable(void *entry, void *value)
{
	CHashEntry64<uint32_t> *pEntry = reinterpret_cast<CHashEntry64<uint32_t> *>(entry);

	pEntry->value |= *reinterpret_cast<uint32_t *>(value);
}

bool AddMutedPlayer(uint64_t steamid, uint32_t flags)
{
	return g_MutedPlayers.AddEntry(steamid, flags, OnPlayerInTable);
}

//-----------------------------------------------------------------------------
// Purpose: remove player from hash table if resulting flag is MUTE_NONE
//-----------------------------------------------------------------------------

bool OnPlayerQueuedToRemove(void *entry, void *value)
{
	CHashEntry64<uint32_t> *pEntry = reinterpret_cast<CHashEntry64<uint32_t> *>(entry);

	pEntry->value &= ~(*reinterpret_cast<uint32_t *>(value));

	if (pEntry->value == MUTE_NONE)
		return true;

	return false;
}

bool RemoveMutedPlayer(uint64_t steamid, uint32_t flags)
{
	return g_MutedPlayers.RemoveEntry(steamid, flags, OnPlayerQueuedToRemove);
}

//-----------------------------------------------------------------------------
// ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(ams_mute_voice, ConCommand_MuteVoice, "ams_mute_voice [player index] - Mute a player's voice by index")
{
	if (CMD_ARGC() < 2)
		return;

	int nPlayerIndex = atoi(CMD_ARGV(1));

	GUARD_VALIDATE_PLAYER_INDEX(nPlayerIndex);

	uint64_t steamid = GetSteamID(nPlayerIndex);

	AddMutedPlayer(steamid, MUTE_VOICE);

	g_pEngineFuncs->Con_Printf("[AMS] Player %s muted (voice)\n", g_pLastPlayer->name);
}

CON_COMMAND_FUNC(ams_mute_chat, ConCommand_MuteChat, "ams_mute_chat [player index] - Mute a player's chat by index")
{
	if (CMD_ARGC() < 2)
		return;

	int nPlayerIndex = atoi(CMD_ARGV(1));

	GUARD_VALIDATE_PLAYER_INDEX(nPlayerIndex);

	uint64_t steamid = GetSteamID(nPlayerIndex);

	AddMutedPlayer(steamid, MUTE_CHAT);

	g_pEngineFuncs->Con_Printf("[AMS] Player %s muted (chat)\n", g_pLastPlayer->name);
}

CON_COMMAND_FUNC(ams_mute_all, ConCommand_MuteAll, "ams_mute_all [player index] - Mute all player communications by index")
{
	if (CMD_ARGC() < 2)
		return;

	int nPlayerIndex = atoi(CMD_ARGV(1));

	GUARD_VALIDATE_PLAYER_INDEX(nPlayerIndex);

	uint64_t steamid = GetSteamID(nPlayerIndex);

	AddMutedPlayer(steamid, MUTE_ALL);

	g_pEngineFuncs->Con_Printf("[AMS] Player %s muted\n", g_pLastPlayer->name);
}

CON_COMMAND_FUNC(ams_unmute_voice, ConCommand_UnmuteVoice, "ams_unmute_voice [player index] - Mute a player's voice by index")
{
	if (CMD_ARGC() < 2)
		return;

	int nPlayerIndex = atoi(CMD_ARGV(1));

	GUARD_VALIDATE_PLAYER_INDEX(nPlayerIndex);

	uint64_t steamid = GetSteamID(nPlayerIndex);

	RemoveMutedPlayer(steamid, MUTE_VOICE);

	g_pEngineFuncs->Con_Printf("[AMS] Player %s unmuted (voice)\n", g_pLastPlayer->name);
}

CON_COMMAND_FUNC(ams_unmute_chat, ConCommand_UnmuteChat, "ams_unmute_chat [player index] - Unmute a player's chat by index")
{
	if (CMD_ARGC() < 2)
		return;

	int nPlayerIndex = atoi(CMD_ARGV(1));

	GUARD_VALIDATE_PLAYER_INDEX(nPlayerIndex);

	uint64_t steamid = GetSteamID(nPlayerIndex);

	RemoveMutedPlayer(steamid, MUTE_CHAT);

	g_pEngineFuncs->Con_Printf("[AMS] Player %s unmuted (chat)\n", g_pLastPlayer->name);
}

CON_COMMAND_FUNC(ams_unmute_all, ConCommand_UnmuteAll, "ams_unmute_all [player index] - Unmute all player communications by index")
{
	if (CMD_ARGC() < 2)
		return;

	int nPlayerIndex = atoi(CMD_ARGV(1));

	GUARD_VALIDATE_PLAYER_INDEX(nPlayerIndex);

	uint64_t steamid = GetSteamID(nPlayerIndex);

	RemoveMutedPlayer(steamid, MUTE_ALL);

	g_pEngineFuncs->Con_Printf("[AMS] Player %s unmuted\n", g_pLastPlayer->name);
}

static int s_MutedPlayersCount = 0;

void ShowMutedPlayers(void *entry)
{
	CHashEntry64<uint32_t> *pEntry = reinterpret_cast<CHashEntry64<uint32_t> *>(entry);

	g_pEngineFuncs->Con_Printf("%d >> SteamID: %llu | Voice: %d | Chat: %d\n", ++s_MutedPlayersCount, pEntry->key, (pEntry->value & MUTE_VOICE) != 0, (pEntry->value & MUTE_CHAT) != 0);
}

CON_COMMAND_FUNC(ams_show_muted_players, ConCommand_ShowMutedPlayers, "ams_show_muted_players - Show all muted players in the console")
{
	g_pEngineFuncs->Con_Printf("====================== Muted Players ======================\n");

	s_MutedPlayersCount = 0;

	g_MutedPlayers.IterateEntries(ShowMutedPlayers);

	g_pEngineFuncs->Con_Printf("====================== Muted Players ======================\n");
}

CON_COMMAND_FUNC(ams_show_current_muted_players, ConCommand_ShowCurrentMutedPlayers, "ams_show_current_muted_players - Show current muted players in the console")
{
	auto pLocal = g_pEngineFuncs->GetLocalPlayer();

	if (!pLocal)
		return;

	g_pEngineFuncs->Con_Printf("====================== Muted Players ======================\n");

	int nLocalPlayer = pLocal->index;

	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (i == nLocalPlayer)
			continue;

		auto steamid = GetSteamID(i);

		if (!steamid)
			continue;

		auto player = GetMutedPlayer(steamid);

		if (!player)
			continue;

		g_pEngineFuncs->Con_Printf("#%d >> Player: %s | Voice: %d | Chat: %d\n", i, g_pLastPlayer->name, (player->value & MUTE_VOICE) != 0, (player->value & MUTE_CHAT) != 0);
	}

	g_pEngineFuncs->Con_Printf("====================== Muted Players ======================\n");
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

// Called when you receive messages to print in the chat
void __fastcall Print_Hooked(void *thisptr, int edx, uintptr_t a1, int a2, int a3)
{
	g_bProcessingChat = true;

	Print_Original(thisptr, a1, a2, a3);

	g_bProcessingChat = false;
}

// Called when exit from game
void __fastcall SaveState_Hooked(void *thisptr, int edx, uintptr_t a1)
{
	//SaveState_Original(thisptr, a1);

	SaveMutedPlayers();
	RemoveMutedPlayers();
}

// Get pointer to muted player (would hook CVoiceBanMgr::GetPlayerBan but it crashes the game)
void *__fastcall InternalFindPlayerSquelch_Hooked(void *thisptr, int edx, char *pszPlayerUniqueID)
{
	auto steamid = GetSteamID(g_nLastIndexedPlayer);

	if (!steamid)
		return NULL;

	auto player = GetMutedPlayer(steamid);

	if (player && (player->value & MUTE_VOICE || g_Config.cvars.ams_mute_everything))
		return player;

	return NULL;
}

void __fastcall SetPlayerBan_Hooked(void *thisptr, int edx, char *pszPlayerUniqueID, bool bMute)
{
	auto steamid = GetSteamID(g_nLastIndexedPlayer);

	if (!steamid)
		return;

	if (bMute)
		AddMutedPlayer(steamid, MUTE_VOICE);
	else
		RemoveMutedPlayer(steamid, MUTE_VOICE);
}

bool __fastcall IsPlayerBlocked_Hooked(void *thisptr, int edx, int nPlayerIndex)
{
	auto steamid = GetSteamID(nPlayerIndex);

	if (!steamid)
		return false;

	auto player = GetMutedPlayer(steamid);

	if (player)
	{
		if (g_Config.cvars.ams_mute_everything)
			return true;

		if (g_bProcessingChat)
		{
			if (player->value & MUTE_CHAT)
				return true;
		}
		else if (player->value & MUTE_VOICE)
		{
			return true;
		}
	}
	
	return false;
}

// Called when you (un)mute player via scoreboard
void __fastcall SetPlayerBlockedState_Hooked(void *thisptr, int edx, int nPlayerIndex, bool bMute)
{
	auto steamid = GetSteamID(nPlayerIndex);

	if (!steamid)
		return;

	if (bMute)
		AddMutedPlayer(steamid, MUTE_VOICE);
	else
		RemoveMutedPlayer(steamid, MUTE_VOICE);
}

qboolean GetPlayerUniqueID_Hooked(int iPlayer, char playerID[16])
{
	g_nLastIndexedPlayer = iPlayer;
	return GetPlayerUniqueID_Original(iPlayer, playerID);
}

// Send to server the mask of muted players that we don't want to hear
void __fastcall UpdateServerState_Hooked(void *thisptr, int edx, bool bForce)
{
	static float flForceBanMaskTime;
	static char command_buffer[64];

	char const *pLevelName = g_pEngineFuncs->pfnGetLevelName();
	bool bClientDebug = static_cast<bool>(voice_clientdebug->value);

	if (*pLevelName == 0 && bClientDebug)
	{
		g_pEngineFuncs->Con_Printf("CVoiceStatus::UpdateServerState: pLevelName[0]==0\n");
		return;
	}

	uint32_t banMask = 0;

	bool bMuteEverything = g_Config.cvars.ams_mute_everything;
	bool bVoiceModEnable = static_cast<bool>(voice_modenable->value);

	// thisptr members
	float *m_LastUpdateServerState = (float *)((BYTE *)thisptr + 0x18);
	int *m_bServerModEnable = (int *)((BYTE *)thisptr + 0x1C);

	// validate cvar 'voice_modenable'
	if (bForce || static_cast<bool>(*m_bServerModEnable) != bVoiceModEnable)
	{
		*m_bServerModEnable = static_cast<int>(bVoiceModEnable);

		sprintf_s(command_buffer, sizeof(command_buffer), "VModEnable %d", bVoiceModEnable);
		g_pEngineFuncs->pfnClientCmd(command_buffer);

		if (bClientDebug)
			g_pEngineFuncs->Con_Printf("CVoiceStatus::UpdateServerState: Sending '%s'\n", command_buffer);
	}

	// build ban mask
	for (uint32_t i = 0; i < MAX_CLIENTS; ++i)
	{
		uint64_t steamid = GetClientSteamID(i);

		if (!steamid)
			continue;

		auto player = GetMutedPlayer(steamid);

		if (player && (player->value & MUTE_VOICE || bMuteEverything))
			banMask |= 1 << i; // one bit, one client
	}

	if (g_BanMask != banMask || (g_pEngineFuncs->GetClientTime() - flForceBanMaskTime >= 5.0f))
	{
		sprintf_s(command_buffer, sizeof(command_buffer), "vban %X", banMask); // vban [ban_mask]

		if (bClientDebug)
			g_pEngineFuncs->Con_Printf("CVoiceStatus::UpdateServerState: Sending '%s'\n", command_buffer);

		g_pEngineFuncs->pfnClientCmd(command_buffer);
		g_BanMask = banMask;
	}
	else if (bClientDebug)
	{
		g_pEngineFuncs->Con_Printf("CVoiceStatus::UpdateServerState: no change\n");
	}

	*m_LastUpdateServerState = flForceBanMaskTime = g_pEngineFuncs->GetClientTime();

	//UpdateServerState_Original(thisptr, bForce);
}

bool HACK_GetPlayerUniqueID_Hooked(int nPlayerIndex, char *pszPlayerUniqueID)
{
	g_nLastIndexedPlayer = nPlayerIndex;
	return HACK_GetPlayerUniqueID_Original(nPlayerIndex, pszPlayerUniqueID);
}

//-----------------------------------------------------------------------------
// Init/release Advanced Mute System
//-----------------------------------------------------------------------------

void InitAMS()
{
	void *pPrint = FIND_PATTERN(L"client.dll", Patterns::Client::CHudBaseTextBlock__Print);

	if (!pPrint)
	{
		ThrowError("CHudBaseTextBlock::Print failed initialization\n");
		return;
	}
	
	void *pSaveState = FIND_PATTERN(L"client.dll", Patterns::Client::CVoiceBanMgr__SaveState);

	if (!pSaveState)
	{
		ThrowError("CVoiceBanMgr::SaveState failed initialization\n");
		return;
	}
	
	void *pSetPlayerBan = FIND_PATTERN(L"client.dll", Patterns::Client::CVoiceBanMgr__SetPlayerBan);

	if (!pSetPlayerBan)
	{
		ThrowError("CVoiceBanMgr::SetPlayerBan failed initialization\n");
		return;
	}
	
	void *pInternalFindPlayerSquelch = FIND_PATTERN(L"client.dll", Patterns::Client::CVoiceBanMgr__InternalFindPlayerSquelch);

	if (!pInternalFindPlayerSquelch)
	{
		ThrowError("CVoiceBanMgr::InternalFindPlayerSquelch failed initialization\n");
		return;
	}
	
	void *pIsPlayerBlocked = FIND_PATTERN(L"client.dll", Patterns::Client::CVoiceStatus__IsPlayerBlocked);

	if (!pIsPlayerBlocked)
	{
		ThrowError("CVoiceStatus::IsPlayerBlocked failed initialization\n");
		return;
	}
	
	void *pSetPlayerBlockedState = FIND_PATTERN(L"client.dll", Patterns::Client::CVoiceStatus__SetPlayerBlockedState);

	if (!pSetPlayerBlockedState)
	{
		ThrowError("CVoiceStatus::SetPlayerBlockedState failed initialization\n");
		return;
	}

	void *pUpdateServerState = FIND_PATTERN(L"client.dll", Patterns::Client::CVoiceStatus__UpdateServerState);

	if (!pUpdateServerState)
	{
		ThrowError("CVoiceStatus::UpdateServerState failed initialization\n");
		return;
	}
	
	void *pHACK_GetPlayerUniqueID = FIND_PATTERN(L"client.dll", Patterns::Client::HACK_GetPlayerUniqueID);

	if (!pHACK_GetPlayerUniqueID)
	{
		ThrowError("HACK_GetPlayerUniqueID failed initialization\n");
		return;
	}
	
	GetPlayerUniqueID_Original = g_pEngineFuncs->GetPlayerUniqueID;
	g_pEngineFuncs->GetPlayerUniqueID = GetPlayerUniqueID_Hooked;

	// Console variables
	//mute_everything = REGISTER_CVAR("ams_mute_everything", "0", 0);
	voice_clientdebug = g_pEngineFuncs->pfnGetCvarPointer("voice_clientdebug");
	voice_modenable = g_pEngineFuncs->pfnGetCvarPointer("voice_modenable");

	// Trampoline hook
	HOOK_FUNCTION(Print_Hook, pPrint, Print_Hooked, Print_Original, PrintFn);
	HOOK_FUNCTION(SaveState_Hook, pSaveState, SaveState_Hooked, SaveState_Original, SaveStateFn);
	HOOK_FUNCTION(SetPlayerBan_Hook, pSetPlayerBan, SetPlayerBan_Hooked, SetPlayerBan_Original, SetPlayerBanFn);
	HOOK_FUNCTION(InternalFindPlayerSquelch_Hook, pInternalFindPlayerSquelch, InternalFindPlayerSquelch_Hooked, InternalFindPlayerSquelch_Original, InternalFindPlayerSquelchFn);
	HOOK_FUNCTION(IsPlayerBlocked_Hook, pIsPlayerBlocked, IsPlayerBlocked_Hooked, IsPlayerBlocked_Original, IsPlayerBlockedFn);
	HOOK_FUNCTION(SetPlayerBlockedState_Hook, pSetPlayerBlockedState, SetPlayerBlockedState_Hooked, SetPlayerBlockedState_Original, SetPlayerBlockedStateFn);
	HOOK_FUNCTION(UpdateServerState_Hook, pUpdateServerState, UpdateServerState_Hooked, UpdateServerState_Original, UpdateServerStateFn);
	HOOK_FUNCTION(HACK_GetPlayerUniqueID_Hook, pHACK_GetPlayerUniqueID, HACK_GetPlayerUniqueID_Hooked, HACK_GetPlayerUniqueID_Original, HACK_GetPlayerUniqueIDFn);

	RemoveMutedPlayers();
	LoadMutedPlayers();
}