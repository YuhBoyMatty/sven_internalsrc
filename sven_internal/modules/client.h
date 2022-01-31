// Client Module

#pragma once

#include "../sdk.h"

struct local_player_s
{
	float flHeight = 0.0f;
	float flGroundNormalAngle = 1.0f;
	float flVelocity = 0.0f;

	Vector vecNoRecoil = { 0.f, 0.f, 0.f };
};

void InitClientModule();

void ReleaseClientModule();

extern local_player_s g_Local;