// Client Module

#pragma once

#include "../sdk.h"
#include "../game/player_utils.h"

//-----------------------------------------------------------------------------
// Struct declarations
//-----------------------------------------------------------------------------

struct local_player_s
{
	float flHeight = 0.0f;
	float flGroundNormalAngle = 1.0f;
	float flVelocity = 0.0f;

	Vector vecNoRecoil = { 0.f, 0.f, 0.f };
};

//-----------------------------------------------------------------------------
// Exports
//-----------------------------------------------------------------------------

extern local_player_s g_Local;
extern extra_player_info_t *g_pPlayerExtraInfo;

//-----------------------------------------------------------------------------
// Init/release client module
//-----------------------------------------------------------------------------

void InitClientModule();

void ShutdownClientModule();