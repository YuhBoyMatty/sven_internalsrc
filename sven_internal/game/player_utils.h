// Player Utils

#ifndef PLAYER_UTILS_H
#define PLAYER_UTILS_H

#ifdef _WIN32
#pragma once
#endif

#include <stdint.h>

#include "../interfaces.h"

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

inline uint64_t GetPlayerSteamID(int nPlayerIndex)
{
	player_info_s *pPlayerInfo = g_pEngineStudio->PlayerInfo(nPlayerIndex - 1); // array of elements

	if (!pPlayerInfo)
		return 0;

	return *reinterpret_cast<uint64_t *>((unsigned char *)pPlayerInfo + 0x248); // FIXME: validate that offset is right
}

float GetPlayerHealth(int index);
float GetPlayerArmor(int index);

#endif // PLAYER_UTILS_H