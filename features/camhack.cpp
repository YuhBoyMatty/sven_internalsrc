// Cam Hack

#include <hl_sdk/engine/APIProxy.h>

#include <ISvenModAPI.h>
#include <client_state.h>
#include <convar.h>
#include <dbg.h>

#include "camhack.h"

#include "../game/utils.h"
#include "../config.h"

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CCamHack g_CamHack;

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

static inline void user_PM_NoClip(Vector &origin, Vector &va, float frametime, struct usercmd_s *cmd)
{
	Vector		wishvel;
	Vector		forward;
	Vector		right;
	float		fmove, smove;

	AngleVectors(va, &forward, &right, NULL);

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

static void ClampViewAngles(Vector &viewangles)
{
	if (viewangles[0] > 89.0f)
		viewangles[0] = 89.0f;

	if (viewangles[0] < -89.0f)
		viewangles[0] = -89.0f;

	if (viewangles[2] > 50.0f)
		viewangles[2] = 50.0f;

	if (viewangles[2] < -50.0f)
		viewangles[2] = -50.0f;
}

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

CON_COMMAND_EXTERN_NO_WRAPPER(sc_camhack, ConCommand_CamHack, "Toggle CamHack")
{
	if (g_pPlayerMove->iuser1 != 0 || SvenModAPI()->GetClientState() != CLS_ACTIVE)
		return;

	if (g_CamHack.IsEnabled())
	{
		Msg("CamHack disabled\n");
		g_CamHack.Disable();
	}
	else
	{
		Msg("CamHack enabled\n");
		g_CamHack.Enable();
	}
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_camhack_reset_roll, ConCommand_CamHackResetRoll, "Reset camera's roll axis to zero")
{
	if (g_CamHack.IsEnabled())
	{
		g_CamHack.ResetRollAxis();
	}
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_camhack_reset, ConCommand_CamHackReset, "Teleport to your original position")
{
	if (g_CamHack.IsEnabled())
	{
		g_CamHack.ResetOrientation();
	}
}

//-----------------------------------------------------------------------------
// Cam Hack Callbacks
//-----------------------------------------------------------------------------

bool CCamHack::StudioRenderModel()
{
	if (m_bEnabled && !g_pClientFuncs->CL_IsThirdPerson() && g_pStudioRenderer->m_pCurrentEntity == g_pEngineFuncs->GetViewModel())
		return true;

	return false;
}

void CCamHack::OnVideoInit()
{
	if (m_bEnabled)
	{
		Disable();
	}
}

void CCamHack::CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	if (g_CamHack.IsEnabled())
	{
		if (cmd->buttons & IN_JUMP)
			cmd->upmove += g_pPlayerMove->clientmaxspeed;

		if (cmd->buttons & IN_DUCK)
			cmd->upmove -= g_pPlayerMove->clientmaxspeed;

		cmd->upmove *= 0.75f;

		g_CamHack.m_vecCameraAngles = g_CamHack.m_vecCameraAngles + (cmd->viewangles - g_CamHack.m_vecVirtualVA);
		g_CamHack.m_vecVirtualVA = cmd->viewangles;

		if (cmd->buttons & IN_ATTACK)
			g_CamHack.m_vecCameraAngles.z -= 0.3f;

		if (cmd->buttons & IN_ATTACK2)
			g_CamHack.m_vecCameraAngles.z += 0.3f;

		NormalizeAngles(g_CamHack.m_vecCameraAngles);

		ClampViewAngles(g_CamHack.m_vecCameraAngles);

		user_PM_NoClip(g_CamHack.m_vecCameraOrigin, g_CamHack.m_vecCameraAngles, g_pPlayerMove->frametime, cmd);

		cmd->viewangles = g_CamHack.m_vecViewAngles;

		cmd->forwardmove = 0.0f;
		cmd->sidemove = 0.0f;
		cmd->upmove = 0.0f;

		cmd->impulse = 0;
		cmd->buttons = 0;
	}
}

void CCamHack::V_CalcRefdef(struct ref_params_s *pparams)
{
	if (g_CamHack.IsEnabled())
	{
		if (g_pPlayerMove->iuser1 == 0)
		{
			cl_entity_t *pLocal = g_pEngineFuncs->GetLocalPlayer();

			*reinterpret_cast<Vector *>(pparams->vieworg) = g_CamHack.m_vecCameraOrigin;
			*reinterpret_cast<Vector *>(pparams->viewangles) = g_CamHack.m_vecCameraAngles;

			pLocal->angles.x = g_CamHack.m_flSavedPitchAngle;
			pLocal->curstate.angles.x = g_CamHack.m_flSavedPitchAngle;
			pLocal->prevstate.angles.x = g_CamHack.m_flSavedPitchAngle;
			pLocal->latched.prevangles.x = g_CamHack.m_flSavedPitchAngle;
		}
		else
		{
			g_CamHack.Disable();
		}
	}
}

//-----------------------------------------------------------------------------
// Cam Hack implementations
//-----------------------------------------------------------------------------

CCamHack::CCamHack()
{
	m_bEnabled = false;

	m_bEnableFirstPerson = false;
	m_bEnableThirdPerson = false;

	m_flSavedPitchAngle = 0.0f;

	m_vecCameraOrigin = { 0.0f, 0.0f, 0.0f };
	m_vecCameraAngles = { 0.0f, 0.0f, 0.0f };

	m_vecViewAngles = { 0.0f, 0.0f, 0.0f };
	m_vecVirtualVA = { 0.0f, 0.0f, 0.0f };
}

void CCamHack::Enable()
{
	m_bEnabled = true;

	g_pEngineFuncs->GetViewAngles(m_vecViewAngles);

	m_vecCameraOrigin = g_pPlayerMove->origin + g_pPlayerMove->view_ofs;
	m_vecCameraAngles = m_vecVirtualVA = m_vecViewAngles;
	m_flSavedPitchAngle = NormalizeAngle(m_vecViewAngles.x) / -3.0f;

	if (g_Config.cvars.camhack_show_model)
	{
		if (!g_pClientFuncs->CL_IsThirdPerson())
		{
			g_pEngineFuncs->ClientCmd("sc_chasecam\n");

			m_bEnableFirstPerson = true;
			m_bEnableThirdPerson = false;
		}
		else
		{
			m_bEnableFirstPerson = false;
			m_bEnableThirdPerson = true;
		}
	}
	else
	{
		if (g_pClientFuncs->CL_IsThirdPerson())
			g_pEngineFuncs->ClientCmd("sc_chasecam\n");

		m_bEnableFirstPerson = m_bEnableThirdPerson = false;
	}
}

void CCamHack::Disable()
{
	m_bEnabled = false;

	if (m_bEnableFirstPerson && g_pClientFuncs->CL_IsThirdPerson())
		g_pEngineFuncs->ClientCmd("firstperson\n");

	if (m_bEnableThirdPerson && !g_pClientFuncs->CL_IsThirdPerson())
		g_pEngineFuncs->ClientCmd("thirdperson\n");
}

void CCamHack::ResetRollAxis()
{
	m_vecCameraAngles.z = 0.0f;
}

void CCamHack::ResetOrientation()
{
	g_pEngineFuncs->GetViewAngles(m_vecViewAngles);

	m_vecCameraOrigin = g_pPlayerMove->origin + g_pPlayerMove->view_ofs;
	m_vecCameraAngles = m_vecViewAngles;
}