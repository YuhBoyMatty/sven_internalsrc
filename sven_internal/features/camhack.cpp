// Cam Hack

#include "camhack.h"

#include "../sdk.h"
#include "../interfaces.h"
#include "../game/utils.h"
#include "../game/console.h"
#include "../game/mathlib.h"

#include "../config.h"

//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;

//-----------------------------------------------------------------------------

CCamHack g_CamHack;

bool g_bCamHack = false;

Vector g_vecCameraOrigin;
Vector g_vecCameraAngles;

Vector g_vecViewAngles;
Vector g_vecVirtualVA;

//cvar_s *camhack_speed_factor = NULL;

GetViewInfoFn GetViewInfo_Original = NULL;

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

static inline void PM_NoClip(Vector &origin, Vector &va, float frametime, struct usercmd_s *cmd)
{
	Vector		wishvel;
	Vector		forward;
	Vector		right;
	float		fmove, smove;

	g_pEngineFuncs->pfnAngleVectors(va, forward, right, NULL);

	fmove = cmd->forwardmove;
	smove = cmd->sidemove;

	for (int i = 0; i < 3; ++i)
	{
		wishvel[i] = forward[i] * fmove + right[i] * smove;
	}

	wishvel[2] += cmd->upmove;

	if (g_Config.cvars.camhack_speed_factor >= 0.0f)
		wishvel = wishvel * g_Config.cvars.camhack_speed_factor;

	VectorMA(origin, frametime, wishvel, origin);
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_camhack, ConCommand_CamHack, "sc_camhack - Toggle CamHack")
{
	if (g_pPlayerMove->iuser1 > 0)
		return;

	g_pEngineFuncs->Con_Printf(g_bCamHack ? "CamHack disabled\n" : "CamHack enabled\n");
	g_bCamHack = !g_bCamHack;

	if (g_bCamHack)
	{
		g_pEngineFuncs->GetViewAngles(g_vecViewAngles);

		g_vecCameraOrigin = g_pPlayerMove->origin + g_pPlayerMove->view_ofs;
		g_vecCameraAngles = g_vecVirtualVA = g_vecViewAngles;
	}
}

CON_COMMAND_FUNC(sc_camhack_reset_roll, ConCommand_CamHackResetRoll, "sc_camhack_reset_roll - Reset camera's roll axis to zero")
{
	if (g_bCamHack)
	{
		g_vecCameraAngles.z = 0.0f;
	}
}

CON_COMMAND_FUNC(sc_camhack_reset, ConCommand_CamHackReset, "sc_camhack_reset - Teleport to your model's position")
{
	if (g_bCamHack)
	{
		g_pEngineFuncs->GetViewAngles(g_vecViewAngles);

		g_vecCameraOrigin = g_pPlayerMove->origin + g_pPlayerMove->view_ofs;
		g_vecCameraAngles = g_vecViewAngles;
	}
}

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

void CCamHack::CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	if (g_bCamHack)
	{
		if (cmd->buttons & IN_JUMP)
			cmd->upmove += g_pPlayerMove->clientmaxspeed;

		if (cmd->buttons & IN_DUCK)
			cmd->upmove -= g_pPlayerMove->clientmaxspeed;

		cmd->upmove *= 0.75f;

		g_vecCameraAngles = g_vecCameraAngles + (cmd->viewangles - g_vecVirtualVA);
		g_vecVirtualVA = cmd->viewangles;

		if (cmd->buttons & IN_ATTACK)
			g_vecCameraAngles.z -= 0.3f;

		if (cmd->buttons & IN_ATTACK2)
			g_vecCameraAngles.z += 0.3f;

		NormalizeAngles(g_vecCameraAngles);

		ClampViewAngles(g_vecCameraAngles);

		PM_NoClip(g_vecCameraOrigin, g_vecCameraAngles, g_pPlayerMove->frametime, cmd);

		cmd->viewangles = g_vecViewAngles;

		cmd->forwardmove = 0.0f;
		cmd->sidemove = 0.0f;
		cmd->upmove = 0.0f;

		cmd->impulse = 0;
		cmd->buttons = 0;
	}
}

void CCamHack::V_CalcRefdef(struct ref_params_s *pparams)
{
	if (g_bCamHack)
	{
		if (g_pPlayerMove->iuser1 == 0)
		{
			*reinterpret_cast<Vector *>(pparams->vieworg) = g_vecCameraOrigin;
			*reinterpret_cast<Vector *>(pparams->viewangles) = g_vecCameraAngles;
		}
		else
		{
			g_bCamHack = false;
		}
	}
}

//-----------------------------------------------------------------------------

void GetViewInfo_Hooked(float *origin, float *upv, float *rightv, float *vpnv)
{
	GetViewInfo_Original(origin, upv, rightv, vpnv);

	if (g_bCamHack)
	{
		if (origin)
			*reinterpret_cast<Vector *>(origin) = g_vecCameraOrigin;

		Vector forward, right, up;

		g_pEngineFuncs->pfnAngleVectors(g_vecCameraAngles, forward, right, up);

		if (upv)
			*reinterpret_cast<Vector *>(upv) = up;

		if (rightv)
			*reinterpret_cast<Vector *>(rightv) = right;

		if (vpnv)
			*reinterpret_cast<Vector *>(vpnv) = forward;
	}
}

//-----------------------------------------------------------------------------

void CCamHack::Init()
{
	GetViewInfo_Original = g_pEngineStudio->GetViewInfo;
	g_pEngineStudio->GetViewInfo = GetViewInfo_Hooked;
	
	//camhack_speed_factor = REGISTER_CVAR("camhack_speed_factor", "1.0", 0);
}