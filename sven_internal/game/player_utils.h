// Player Utils

#ifndef PLAYER_UTILS_H
#define PLAYER_UTILS_H

#ifdef _WIN32
#pragma once
#endif

#include <stdint.h>

#include "../interfaces.h"

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;

//-----------------------------------------------------------------------------
// Structures declaration
//-----------------------------------------------------------------------------

struct extra_player_info_t // FIXME: update SDK, struct from cl_dll/hud.h
{
	short frags;
	short deaths;
	short playerclass;
	short health;
	bool dead;
	short teamnumber;
	char teamname[16];

	// Sven Co-op specific
	char pad_1[60];
};

//-----------------------------------------------------------------------------
// Util functions
//-----------------------------------------------------------------------------

inline bool IsSteamID64(uint64_t steamID)
{
	return steamID & 76561197960265728;
}

inline uint64_t GetPlayerSteamID(int nPlayerIndex)
{
	player_info_s *pPlayerInfo = g_pEngineStudio->PlayerInfo(nPlayerIndex - 1); // array of elements

	if (!pPlayerInfo)
		return 0;

	return *reinterpret_cast<uint64_t *>((unsigned char *)pPlayerInfo + 0x248); // FIXME: validate that offset is right
}

inline bool IsLocalPlayerAlive()
{
	if (g_pPlayerMove && g_pPlayerMove->iuser1 == 0 && !g_pPlayerMove->dead)
		return true;

	return false;
}

float GetPlayerHealth(int index);
float GetPlayerArmor(int index);

#endif // PLAYER_UTILS_H