// Client Module

#pragma once

#include "../sdk.h"

typedef qboolean (*Netchan_CanPacketFn)(netchan_t *);
typedef int (*HUD_InitFn)(void);
typedef void (*CL_CreateMoveFn)(float, struct usercmd_s *, int);
typedef void (*HUD_PlayerMoveFn)(struct playermove_s *, int);
typedef void (*V_CalcRefdefFn)(struct ref_params_s *);
typedef void (*HUD_PostRunCmdFn)(struct local_state_s *, struct local_state_s *, struct usercmd_s *, int, double, unsigned int);

struct local_player_s
{
	float flHeight = 0.0f;
	float flGroundNormalAngle = 1.0f;
	float flVelocity = 0.0f;
};

void InitClientModule();

void ReleaseClientModule();

extern local_player_s g_Local;