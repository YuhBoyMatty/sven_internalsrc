// Misc

#include <ICvar.h>
#include <IMemoryUtils.h>
#include <ISvenModAPI.h>
#include <IInventory.h>

#include <convar.h>
#include <dbg.h>

#include <hl_sdk/cl_dll/hud.h>
#include <hl_sdk/common/entity_types.h>
#include <hl_sdk/common/com_model.h>
#include <hl_sdk/engine/APIProxy.h>
#include <netchan.h>

#include "misc.h"

#include <algorithm>
#include <cmath>

#include "../modules/patches.h"
#include "../game/utils.h"

#include "../config.h"
#include "../patterns.h"

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern bool bSendPacket;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK(BOOL, WINAPI, fQueryPerformanceCounter, LARGE_INTEGER *);
DECLARE_HOOK(void, __cdecl, fNetchan_Transmit, netchan_t *, int, unsigned char *);

DECLARE_CLASS_HOOK(void, CHud__Think, CHud *);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CMisc g_Misc;

cvar_t *ex_interp = NULL;
cvar_t *default_fov = NULL;

static int s_nSinkState = 0;
static int s_iWeaponID = -1;

static bool s_bFreeze = false;
static bool s_bFreeze2 = false;

static float s_flTopColorDelay = 0.0f;
static float s_flBottomColorDelay = 0.0f;

static int s_iTopColorOffset = 0;
static int s_iBottomColorOffset = 0;

static float s_flWeaponOffset[32] =
{
	0.0f, // 0
	0.0f, // 1
	-4.0f, // 2
	0.0f, // 3
	-2.5f, // 4
	0.0f, // 5
	-4.0f, // 6
	-5.0f, // 7
	-9.2f, // 8
	-3.0f, // 9
	0.0f, // 10
	-8.0f, // 11
	-4.0f, // 12
	0.0f, // 13
	0.0f, // 14
	0.0f, // 15
	0.0f, // 16
	-4.15f, // 17
	0.0f, // 18
	0.0f, // 19
	0.0f, // 20
	0.0f, // 21
	0.0f, // 22
	-7.3f, // 23
	-4.25f, // 24
	0.0f, // 25
	-2.0f, // 26
	-2.7f, // 27
	0.0f, // 28
	-7.5f, // 29
	0.0f, // 30
	0.0f, // 31
};

//-----------------------------------------------------------------------------
// Common Functions
//-----------------------------------------------------------------------------

static float GetWeaponOffset(cl_entity_s *pViewModel, int iWeaponID)
{
	// ToDo: use class_table

	if (iWeaponID == 0)
	{
		const char *pszModelName = pViewModel->model->name;

		if (pszModelName && *pszModelName)
		{
			const char *pszModelNameEnd = pszModelName + strlen(pszModelName);

			while (pszModelNameEnd > pszModelName)
			{
				if (*(pszModelNameEnd - 1) == '/')
				{
					pszModelName = pszModelNameEnd;
					break;
				}

				--pszModelNameEnd;
			}

			if (!strcmp(pszModelName, "v_crowbar.mdl"))
			{
				return -1.5f;
			}
			else if (!strcmp(pszModelName, "v_pipe_wrench.mdl"))
			{
				return -2.0f;
			}
			else if (!strcmp(pszModelName, "v_medkit.mdl"))
			{
				return -0.7f;
			}
			else if (!strcmp(pszModelName, "v_bgrap.mdl"))
			{
				return -9.0f;
			}
			else if (!strcmp(pszModelName, "v_357.mdl"))
			{
				return -6.2f;
			}
			else if (!strcmp(pszModelName, "v_egon.mdl"))
			{
				return -5.0f;
			}
			else if (!strcmp(pszModelName, "v_squeak.mdl"))
			{
				return -3.0f;
			}
		}
	}
	else
	{
		return s_flWeaponOffset[iWeaponID];
	}

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Commands, CVars..
//-----------------------------------------------------------------------------

CON_COMMAND_EXTERN_NO_WRAPPER(sc_autojump, ConCommand_AutoJump, "Toggle autojump")
{
	Msg(g_Config.cvars.autojump ? "Auto Jump disabled\n" : "Auto Jump enabled\n");
	g_Config.cvars.autojump = !g_Config.cvars.autojump;
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_doubleduck, ConCommand_DoubleDuck, "Toggle doubleduck")
{
	Msg(g_Config.cvars.doubleduck ? "Double Duck disabled\n" : "Double Duck enabled\n");
	g_Config.cvars.doubleduck = !g_Config.cvars.doubleduck;
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_jumpbug, ConCommand_JumpBug, "Toggle jumpbug")
{
	Msg(g_Config.cvars.jumpbug ? "Jump Bug disabled\n" : "Jump Bug enabled\n");
	g_Config.cvars.jumpbug = !g_Config.cvars.jumpbug;
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_fakelag, ConCommand_FakeLag, "Toggle fake lag")
{
	Msg(g_Config.cvars.fakelag ? "Fake Lag disabled\n" : "Fake Lag enabled\n");
	g_Config.cvars.fakelag = !g_Config.cvars.fakelag;
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_fastrun, ConCommand_FastRun, "Toggle fast run")
{
	Msg(g_Config.cvars.fastrun ? "Fast Run disabled\n" : "Fast Run enabled\n");
	g_Config.cvars.fastrun = !g_Config.cvars.fastrun;
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_auto_ceil_clipping, ConCommand_AutoCeilClipping, "Automatically suicide when you touch a ceil to perform clipping")
{
	Msg(g_Config.cvars.auto_ceil_clipping ? "Auto Ceil-Clipping disabled\n" : "Auto Ceil-Clipping enabled\n");
	g_Config.cvars.auto_ceil_clipping = !g_Config.cvars.auto_ceil_clipping;
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_selfsink, ConCommand_AutoSelfSink, "Perform self sink")
{
	if ( g_pPlayerMove->iuser1 != 0 || g_pPlayerMove->dead )
		return;

	if (g_pPlayerMove->view_ofs.z == VEC_DUCK_VIEW.z)
	{
		g_pEngineFuncs->SetViewAngles(Vector(-0.1f, -90.0f, 0.0f));
		g_pEngineFuncs->ClientCmd("+jump\n");

		s_nSinkState = 1;
	}
	else
	{
		g_pEngineFuncs->ClientCmd("+duck\n");

		s_nSinkState = 2;
	}
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_reset_colors, ConCommand_ResetColors, "Reset colors in Color Pulsator")
{
	s_iBottomColorOffset = s_iTopColorOffset;
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_sync_colors, ConCommand_SyncColors, "Sync. change time for top and bottom colors in Color Pulsator")
{
	if (s_flTopColorDelay > s_flBottomColorDelay)
		s_flBottomColorDelay = s_flTopColorDelay;

	if (s_flBottomColorDelay > s_flTopColorDelay)
		s_flTopColorDelay = s_flBottomColorDelay;
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_drop_empty_weapon, ConCommand_DropEmptyWeapon, "Drop an empty weapon from your inventory")
{
	for (int i = 0; i < Inventory()->GetMaxWeaponSlots(); i++)
	{
		for (int j = 0; j < Inventory()->GetMaxWeaponPositions(); j++)
		{
			WEAPON *pWeapon = Inventory()->GetWeapon(i, j);

			if (pWeapon && !Inventory()->HasAmmo(pWeapon))
			{
				Inventory()->DropWeapon(pWeapon);
				return;
			}
		}
	}
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_freeze, ConCommand_Freeze, "Block connection with a server")
{
	Msg(s_bFreeze ? "Connection restored\n" : "Connection blocked\n");
	s_bFreeze = !s_bFreeze;
}

CON_COMMAND_EXTERN_NO_WRAPPER(sc_freeze2, ConCommand_Freeze2, "Block connection with a server with 2nd method")
{
	Msg(s_bFreeze ? "Connection restored\n" : "Connection blocked\n");
	s_bFreeze2 = !s_bFreeze2;
}

CON_COMMAND(sc_print_skybox_name, "sc_print_skybox_name - Prints current skybox name")
{
	if (g_pPlayerMove && g_pPlayerMove->movevars)
	{
		Msg("Skybox: %s\n", g_pPlayerMove->movevars->skyName);
	}
}

static void freeze_toggle_key_down()
{
	Msg("Connection blocked\n");
	s_bFreeze = true;
}

static void freeze_toggle_key_up()
{
	Msg("Connection restored\n");
	s_bFreeze = false;
}

static void freeze2_toggle_key_down()
{
	Msg("Connection blocked\n");
	s_bFreeze2 = true;
}

static void freeze2_toggle_key_up()
{
	Msg("Connection restored\n");
	s_bFreeze2 = false;
}

static ConCommand input_command__sc_freeze_toggle("+sc_freeze_toggle", freeze_toggle_key_down, "Freeze input");
static ConCommand output_command__sc_freeze_toggle("-sc_freeze_toggle", freeze_toggle_key_up, "Freeze output");

static ConCommand input_command__sc_freeze2_toggle("+sc_freeze2_toggle", freeze2_toggle_key_down, "Freeze #2 input");
static ConCommand output_command__sc_freeze2_toggle("-sc_freeze2_toggle", freeze2_toggle_key_up, "Freeze #2 output");

ConVar sc_speedhack("sc_speedhack", "1", FCVAR_CLIENTDLL, "sc_speedhack [value] - Set speedhack value");
ConVar sc_speedhack_ltfx("sc_speedhack_ltfx", "0", FCVAR_CLIENTDLL, "sc_speedhack_ltfx [value] - Set LTFX speedhack value; 0 - disable, value < 0 - slower, value > 0 - faster");

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC(BOOL, WINAPI, HOOKED_fQueryPerformanceCounter, LARGE_INTEGER *lpPerformanceCount)
{
	static LONGLONG oldfakevalue = 0;
	static LONGLONG oldrealvalue = 0;

	LONGLONG newvalue;

	if (oldfakevalue == 0 || oldrealvalue == 0)
	{
		oldfakevalue = lpPerformanceCount->QuadPart;
		oldrealvalue = lpPerformanceCount->QuadPart;
	}

	BOOL result = ORIG_fQueryPerformanceCounter(lpPerformanceCount);

	newvalue = lpPerformanceCount->QuadPart;
	newvalue = oldfakevalue + (LONGLONG)((newvalue - oldrealvalue) * static_cast<double>(g_Config.cvars.application_speed));

	oldrealvalue = lpPerformanceCount->QuadPart;
	oldfakevalue = newvalue;

	lpPerformanceCount->QuadPart = newvalue;

	return result;
}

DECLARE_FUNC(void, __cdecl, HOOKED_fNetchan_Transmit, netchan_t *chan, int lengthInBytes, unsigned char *data)
{
	if (s_bFreeze2)
	{
		ORIG_fNetchan_Transmit(chan, 0, NULL); // cancel size and data
		return;
	}

	ORIG_fNetchan_Transmit(chan, lengthInBytes, data);
}

DECLARE_CLASS_FUNC(void, HOOKED_CHud__Think, CHud *pHud)
{
	if (g_Config.cvars.remove_fov_cap)
	{
		HUDLIST *pList = *reinterpret_cast<HUDLIST **>(pHud);

		while (pList)
		{
			if ((pList->p->m_iFlags & HUD_ACTIVE) != 0)
				pList->p->Think();
			pList = pList->pNext;
		}

		*((float *)pHud + 4) = 0.f; // m_flMouseSensitivity
		*((int *)pHud + 22) = int(default_fov->value); // m_iFOV
	}
	else
	{
		ORIG_CHud__Think(pHud);
	}
}

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

void CMisc::CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	// Clamp speedhack values
	if (sc_speedhack.GetFloat() < 0.0f)
		sc_speedhack.SetValue("0");
	
	if (sc_speedhack_ltfx.GetFloat() < -100.0f)
		sc_speedhack_ltfx.SetValue("-100");

	// Set speedhack
	UTIL_SetGameSpeed(static_cast<double>(sc_speedhack.GetFloat()));
	*dbRealtime += static_cast<double>(sc_speedhack_ltfx.GetFloat()) * frametime;

	FakeLag(frametime);
	ColorPulsator();
	TertiaryAttackGlitch();

	if (g_Config.cvars.rotate_dead_body && g_pPlayerMove->dead)
	{
		Vector va;

		g_pEngineFuncs->GetViewAngles(va);
		cmd->viewangles = va;
	}

	if (g_pPlayerMove->iuser1 < 1)
	{
		AutoSelfSink();
		AutoCeilClipping(cmd);
		AutoJump(cmd);
		DoubleDuck(cmd);
		Spinner(cmd);
		FastRun(cmd);
		JumpBug(frametime, cmd);
	}

	if (s_bFreeze)
		bSendPacket = false;
}

void CMisc::V_CalcRefdef(struct ref_params_s *pparams)
{
	QuakeGuns_V_CalcRefdef();

	if (m_bSpinCanChangePitch)
	{
		cl_entity_t *pLocal = g_pEngineFuncs->GetLocalPlayer();

		pLocal->angles.x = m_flSpinPitchAngle;
		pLocal->curstate.angles.x = m_flSpinPitchAngle;
		pLocal->prevstate.angles.x = m_flSpinPitchAngle;
		pLocal->latched.prevangles.x = m_flSpinPitchAngle;
	}
}

void CMisc::HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed)
{
	QuakeGuns_HUD_PostRunCmd(to);
	NoWeaponAnim_HUD_PostRunCmd(to);
}

static int line_beamindex = 0;

void CMisc::OnAddEntityPost(int is_visible, int type, struct cl_entity_s *ent, const char *modelname)
{
	if (g_Config.cvars.show_players_push_direction)
	{
		if (is_visible && type == ET_PLAYER && ent->index != g_pPlayerMove->player_index + 1)
		{
			Vector vecEnd;
			Vector vecEnd2;

			Vector vecBegin = ent->origin;

			Vector vecPushDir(1.f, 0.f, 0.f);
			Vector vecPushDir2(0.f, 1.f, 0.f);

			if (ent->curstate.usehull)
				vecBegin[2] += VEC_DUCK_HULL_MIN.z + 1.5f;
			else
				vecBegin[2] += VEC_HULL_MIN.z + 1.5f;

			vecEnd = vecBegin + vecPushDir * g_Config.cvars.push_direction_length;
			vecEnd2 = vecBegin + vecPushDir2 * g_Config.cvars.push_direction_length * (1.f / 3.f);

			if (!line_beamindex)
				line_beamindex = g_pEngineFuncs->pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");

			// Opposite direction
			g_pEngineFuncs->pEfxAPI->R_BeamPoints(vecBegin,
												  vecEnd,
												  line_beamindex,
												  0.001f, // life time
												  g_Config.cvars.push_direction_width,
												  0.f, // amplitude
												  32.f, // brightness
												  2.f, // speed
												  0, // startFrame
												  0.f, // framerate
												  g_Config.cvars.push_direction_color[0],
												  g_Config.cvars.push_direction_color[1],
												  g_Config.cvars.push_direction_color[2]);

			// 90 deg. opposite direction that also lets you to push a player
			g_pEngineFuncs->pEfxAPI->R_BeamPoints(vecBegin,
												  vecEnd2,
												  line_beamindex,
												  0.001f, // life time
												  g_Config.cvars.push_direction_width,
												  0.f, // amplitude
												  32.f, // brightness
												  2.f, // speed
												  0, // startFrame
												  0.f, // framerate
												  g_Config.cvars.push_direction_color[0],
												  g_Config.cvars.push_direction_color[1],
												  g_Config.cvars.push_direction_color[2]);
		}
	}
}

void CMisc::OnVideoInit()
{
	line_beamindex = 0;
}

//-----------------------------------------------------------------------------
// Auto Jump
//-----------------------------------------------------------------------------

void CMisc::AutoJump(struct usercmd_s *cmd)
{
	static bool bAllowJump = false;

	if (g_Config.cvars.autojump && cmd->buttons & IN_JUMP)
	{
		if (bAllowJump && GetAsyncKeyState(VK_SPACE))
		{
			cmd->buttons &= ~IN_JUMP;
			bAllowJump = false;
		}
		else
		{
			bAllowJump = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Auto Jumpbug
//-----------------------------------------------------------------------------

void CMisc::JumpBug(float frametime, struct usercmd_s *cmd)
{
	static int nJumpBugState = 0;

	if (g_Config.cvars.jumpbug && g_pPlayerMove->flFallVelocity >= 500.0f)
	{
		Vector vBottomOrigin = g_pPlayerMove->origin; vBottomOrigin.z -= 8192.0f;
		pmtrace_t *pTrace = g_pEngineFuncs->PM_TraceLine(g_pPlayerMove->origin, vBottomOrigin, PM_NORMAL, /* g_pPlayerMove->usehull */ (g_pPlayerMove->flags & FL_DUCKING) ? 1 : 0, -1);

		float flHeight = fabsf(pTrace->endpos.z - g_pPlayerMove->origin.z);
		float flGroundNormalAngle = static_cast<float>(acos(pTrace->plane.normal[2]) * 180.0 / M_PI);

		if (flGroundNormalAngle <= acosf(0.7f) * 180.0f / static_cast<float>(M_PI) && g_pPlayerMove->waterlevel == WL_NOT_IN_WATER)
		{
			float flPlayerHeight = flHeight; // = 0.0f
			float flFrameZDist = fabsf((g_pPlayerMove->flFallVelocity + (800.0f * frametime)) * frametime);

			//if (g_Local.flGroundNormalAngle > 1.0f)
			//{
				//bool bDuck = g_pPlayerMove->flags & FL_DUCKING;
				//Vector vBottomOrigin = g_pPlayerMove->origin; vBottomOrigin.z -= 8192.0f;

				//pmtrace_t *pTrace = g_pEngineFuncs->PM_TraceLine(g_pPlayerMove->origin, vBottomOrigin, PM_NORMAL, bDuck ? 1 : 0 /* g_pPlayerMove->usehull */, -1);
				//flPlayerHeight = fabsf(g_pPlayerMove->origin.z - pTrace->endpos.z + (bDuck ? 18.0f : 36.0f));
			//}
			//else
			//{
				//flPlayerHeight = g_Local.flHeight;
			//}

			cmd->buttons |= IN_DUCK;
			cmd->buttons &= ~IN_JUMP;

			switch (nJumpBugState)
			{
			case 1:
				cmd->buttons &= ~IN_DUCK;
				cmd->buttons |= IN_JUMP;

				nJumpBugState = 2;
				break;

			case 2:
				nJumpBugState = 0;
				break;

			default:
				if (fabsf(flPlayerHeight - flFrameZDist * 1.5f) <= 20.0f && flFrameZDist > 0.0f)
				{
					float flNeedSpeed = fabsf(flPlayerHeight - 19.f);
					float flScale = fabsf(flNeedSpeed / flFrameZDist);

					UTIL_SetGameSpeed(flScale);

					nJumpBugState = 1;
				}
				break;
			}
		}
		else
		{
			nJumpBugState = 0;
		}
	}
	else
	{
		nJumpBugState = 0;
	}
}

//-----------------------------------------------------------------------------
// Auto Doubleduck
//-----------------------------------------------------------------------------

void CMisc::DoubleDuck(struct usercmd_s *cmd)
{
	if (g_Config.cvars.doubleduck && GetAsyncKeyState(VK_LCONTROL))
	{
		static bool bForceUnduck = false;

		if (bForceUnduck)
		{
			cmd->buttons &= ~IN_DUCK;

			bForceUnduck = false;
		}
		else if (g_pPlayerMove->onground != -1)
		{
			cmd->buttons |= IN_DUCK;

			bForceUnduck = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Fast Run
//-----------------------------------------------------------------------------

void CMisc::FastRun(struct usercmd_s *cmd)
{
	if (g_Config.cvars.fastrun && g_pPlayerMove->onground != -1)
	{
		static bool bFastRun = false;
		float flMaxSpeed = g_pPlayerMove->clientmaxspeed;

		if ((cmd->buttons & IN_FORWARD && cmd->buttons & IN_MOVELEFT) || (cmd->buttons & IN_BACK && cmd->buttons & IN_MOVERIGHT))
		{
			if (bFastRun)
			{
				cmd->sidemove -= flMaxSpeed; // sqrtf(2.0f) * flMaxSpeed   vvv
				cmd->forwardmove -= flMaxSpeed;

				bFastRun = false;
			}
			else
			{
				cmd->sidemove += flMaxSpeed;
				cmd->forwardmove += flMaxSpeed;

				bFastRun = true;
			}
		}
		else if ((cmd->buttons & IN_FORWARD && cmd->buttons & IN_MOVERIGHT) || (cmd->buttons & IN_BACK && cmd->buttons & IN_MOVELEFT))
		{
			if (bFastRun)
			{
				cmd->sidemove -= flMaxSpeed;
				cmd->forwardmove += flMaxSpeed;

				bFastRun = false;
			}
			else
			{
				cmd->sidemove += flMaxSpeed;
				cmd->forwardmove -= flMaxSpeed; // sqrtf(2.0f) * flMaxSpeed  ^^^

				bFastRun = true;
			}
		}
		else if (cmd->buttons & IN_FORWARD || cmd->buttons & IN_BACK)
		{
			if (bFastRun)
			{
				cmd->sidemove -= flMaxSpeed;

				bFastRun = false;
			}
			else
			{
				cmd->sidemove += flMaxSpeed;
				bFastRun = true;
			}
		}
		else if (cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT)
		{
			if (bFastRun)
			{
				cmd->forwardmove -= flMaxSpeed;
				bFastRun = false;
			}
			else
			{
				cmd->forwardmove += flMaxSpeed;
				bFastRun = true;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Spinner
//-----------------------------------------------------------------------------

void CMisc::Spinner(struct usercmd_s *cmd)
{
	static Vector vSpinAngles(0.f, 0.f, 0.f);

	bool bAnglesChanged = false;
	m_bSpinCanChangePitch = false;

	if (g_Config.cvars.spin_yaw_angle)
	{
		if ( !g_Config.cvars.lock_pitch && !g_Config.cvars.spin_pitch_angle )
			vSpinAngles.x = cmd->viewangles.x;

		vSpinAngles.y += g_Config.cvars.spin_yaw_rotation_angle;
		vSpinAngles.y = NormalizeAngle(vSpinAngles.y);

		bAnglesChanged = true;
	}
	else if (g_Config.cvars.lock_yaw)
	{
		if ( !g_Config.cvars.lock_pitch && !g_Config.cvars.spin_pitch_angle )
			vSpinAngles.x = cmd->viewangles.x;

		vSpinAngles.y = g_Config.cvars.lock_yaw_angle;
		bAnglesChanged = true;
	}

	if (g_Config.cvars.spin_pitch_angle)
	{
		if ( !g_Config.cvars.lock_yaw && !g_Config.cvars.spin_yaw_angle )
			vSpinAngles.y = cmd->viewangles.y;

		vSpinAngles.x += g_Config.cvars.spin_pitch_rotation_angle;
		vSpinAngles.x = NormalizeAngle(vSpinAngles.x);

		m_bSpinCanChangePitch = true;
		bAnglesChanged = true;
	}
	else if (g_Config.cvars.lock_pitch)
	{
		if ( !g_Config.cvars.lock_yaw && !g_Config.cvars.spin_yaw_angle )
			vSpinAngles.y = cmd->viewangles.y;

		vSpinAngles.x = g_Config.cvars.lock_pitch_angle;

		m_bSpinCanChangePitch = true;
		bAnglesChanged = true;
	}

	if (bAnglesChanged)
	{
		if ((g_pPlayerMove && g_pPlayerMove->movetype == MOVETYPE_WALK && g_pPlayerMove->waterlevel <= WL_FEET) &&
			!(cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2 || cmd->buttons & IN_USE || cmd->buttons & IN_ALT1))
		{
			m_flSpinPitchAngle = vSpinAngles.x / -3.0f;
			UTIL_SetAnglesSilent(vSpinAngles, cmd);
		}
		else
		{
			m_bSpinCanChangePitch = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Auto Ceil Clipping
//-----------------------------------------------------------------------------

void CMisc::AutoCeilClipping(struct usercmd_s *cmd)
{
	static bool jumped = false;

	if ( g_Config.cvars.auto_ceil_clipping && g_pPlayerMove->iuser1 == 0 && !g_pPlayerMove->dead )
	{
		if (jumped)
		{
			if (g_pPlayerMove->onground == -1 && g_pPlayerMove->waterlevel <= WL_FEET)
			{
				cmd->buttons |= IN_DUCK;

				// Suicide only if we got apex or started falling
				if (g_pPlayerMove->velocity.z <= 0.f)
				{
					Vector vecStart = g_pPlayerMove->origin;
					Vector vecEnd = vecStart + Vector(0.f, 0.f, VEC_DUCK_HULL_MAX.z);

					pmtrace_t *pTrace = g_pEngineFuncs->PM_TraceLine(vecStart, vecEnd, PM_NORMAL, (g_pPlayerMove->flags & FL_DUCKING) ? 1 : 0, -1);

					if (pTrace->fraction < 1.0f)
					{
						g_pEngineFuncs->ClientCmd("kill\n");
						jumped = false;
					}
				}
			}
			else
			{
				jumped = false;
			}
		}
		else
		{
			if (g_pPlayerMove->onground == -1 && g_pPlayerMove->velocity.z > 0.f)
				jumped = true;
			else
				jumped = false;
		}
	}
	else
	{
		jumped = false;
	}
}

//-----------------------------------------------------------------------------
// Fake Lag
//-----------------------------------------------------------------------------

void CMisc::FakeLag(float frametime)
{
	static bool bSetInterpOnce = false;

	if (g_Config.cvars.fakelag_adaptive_ex_interp)
	{
		if (ex_interp->value != 0.01f)
			ex_interp->value = 0.01f;

		bSetInterpOnce = true;
	}
	else if (bSetInterpOnce)
	{
		if (ex_interp->value == 0.01f)
			ex_interp->value = 0.1f;

		bSetInterpOnce = false;
	}

	if (g_Config.cvars.fakelag)
	{
		bool bFakeLag = true;

		if (g_Config.cvars.fakelag_move != 0)
		{
			float flVelocity = g_pPlayerMove->velocity.Length2D();

			if (g_Config.cvars.fakelag_move == 1) // On land
			{
				if (flVelocity > 0.f)
					bFakeLag = false;
			}
			else if (g_Config.cvars.fakelag_move == 2) // On move
			{
				if (flVelocity == 0.f)
					bFakeLag = false;
			}
			else if (g_Config.cvars.fakelag_move == 3) // In air
			{
				if (g_pPlayerMove->onground != -1)
					bFakeLag = false;
			}
		}

		if (bFakeLag)
		{
			static int choked = 0;
			static int good = 0;

			if (g_Config.cvars.fakelag_type == 0) // Dynamic
			{
				if (choked < g_Config.cvars.fakelag_limit)
				{
					bSendPacket = false;

					choked++;

					good = 0;
				}
				else
				{
					float one = g_Config.cvars.fakelag_limit / 100.f;
					float tmp = one * g_Config.cvars.fakelag_variance;

					good++;

					if (good > tmp)
					{
						choked = 0;
					}
				}
			}
			else if (g_Config.cvars.fakelag_type == 1) // Maximum
			{
				choked++;

				if (choked > 0)
					bSendPacket = false;

				if (choked > g_Config.cvars.fakelag_limit)
					choked = -1; // 1 tick valid
			}
			else if (g_Config.cvars.fakelag_type == 2) // Flucture
			{
				static bool jitter = false;

				if (jitter)
					bSendPacket = false;

				jitter = !jitter;
			}
			else if (g_Config.cvars.fakelag_type == 3) // Break lag compensation
			{
				Vector velocity = g_pPlayerMove->velocity;
				velocity.z = 0;
				float len = velocity.Length() * frametime;

				int choke = std::min<int>(static_cast<int>(std::ceilf(64.0f / len)), 14);
				if (choke > 14) return;

				static int choked = 0;
				if (choked > choke)
				{
					bSendPacket = true;
					choked = 0;
				}
				else
				{
					bSendPacket = false;
					choked++;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Auto Selfsink
//-----------------------------------------------------------------------------

void CMisc::AutoSelfSink() // improve it tf
{
	if (s_nSinkState == 1)
	{
		g_pEngineFuncs->ClientCmd("kill;-jump;-duck\n");

		s_nSinkState = 0;
	}
	else if (s_nSinkState == 2 && g_pPlayerMove->view_ofs.z == VEC_DUCK_VIEW.z)
	{
		ConCommand_AutoSelfSink();
	}
}

//-----------------------------------------------------------------------------
// Tertiary Attack Glitch
//-----------------------------------------------------------------------------

void CMisc::TertiaryAttackGlitch()
{
	if (g_Config.cvars.tertiary_attack_glitch)
	{
		if (!IsTertiaryAttackGlitchPatched())
		{
			EnableTertiaryAttackGlitch();
		}
	}
	else if (IsTertiaryAttackGlitchPatched())
	{
		DisableTertiaryAttackGlitch();
	}

	if (IsTertiaryAttackGlitchInit_Server())
	{
		if (g_Config.cvars.tertiary_attack_glitch)
		{
			if (!IsTertiaryAttackGlitchPatched_Server())
			{
				EnableTertiaryAttackGlitch_Server();
			}
		}
		else if (IsTertiaryAttackGlitchPatched_Server())
		{
			DisableTertiaryAttackGlitch_Server();
		}
	}
}

//-----------------------------------------------------------------------------
// Color Pulsator
//-----------------------------------------------------------------------------

void CMisc::ColorPulsator()
{
	static char command_buffer[32];

	if (g_Config.cvars.color_pulsator)
	{
		if (g_Config.cvars.color_pulsator_top && g_pEngineFuncs->Sys_FloatTime() - s_flTopColorDelay >= g_Config.cvars.color_pulsator_delay)
		{
			if (s_iTopColorOffset > 12)
				s_iTopColorOffset = 0;
			
			s_flTopColorDelay = g_pEngineFuncs->Sys_FloatTime() + g_Config.cvars.color_pulsator_delay;

			sprintf_s(command_buffer, sizeof(command_buffer), "topcolor %d\n", s_iTopColorOffset * 20);
			g_pEngineFuncs->ClientCmd(command_buffer);

			++s_iTopColorOffset;
		}

		if (g_Config.cvars.color_pulsator_bottom && g_pEngineFuncs->Sys_FloatTime() - s_flBottomColorDelay >= g_Config.cvars.color_pulsator_delay)
		{
			if (s_iBottomColorOffset > 12)
				s_iBottomColorOffset = 0;

			s_flBottomColorDelay = g_pEngineFuncs->Sys_FloatTime() + g_Config.cvars.color_pulsator_delay;

			sprintf_s(command_buffer, sizeof(command_buffer), "bottomcolor %d\n", s_iBottomColorOffset * 20);
			g_pEngineFuncs->ClientCmd(command_buffer);

			++s_iBottomColorOffset;
		}
	}
}

//-----------------------------------------------------------------------------
// No Weapon Animations
//-----------------------------------------------------------------------------

void CMisc::NoWeaponAnim_HUD_PostRunCmd(struct local_state_s *to)
{
	cl_entity_s *pViewModel = g_pEngineFuncs->GetViewModel();

	if (!pViewModel)
		return;

	static int s_iWeaponID = -1;
	static int s_iWaitTicks = 0;
	static char *s_pszWeaponName = NULL;

	int nWeaponID = to->client.m_iId;

	if (g_Config.cvars.no_weapon_anim == 2)
	{
		if (s_iWeaponID != nWeaponID || s_iWeaponID == 0 && (s_pszWeaponName && *s_pszWeaponName && pViewModel->model->name && *pViewModel->model->name && strcmp(pViewModel->model->name, s_pszWeaponName)))
		{
			s_pszWeaponName = pViewModel->model->name;
			s_iWeaponID = nWeaponID;

			s_iWaitTicks = 5;
		}

		if (s_iWaitTicks > 0)
		{
			g_pEngineFuncs->WeaponAnim(0, 0);

			--s_iWaitTicks;
		}
	}
	else if (g_Config.cvars.no_weapon_anim == 1)
	{
		g_pEngineFuncs->WeaponAnim(0, 0);

		s_iWeaponID = nWeaponID;
		s_iWaitTicks = 0;
	}
}

//-----------------------------------------------------------------------------
// Quake Guns
//-----------------------------------------------------------------------------

void CMisc::QuakeGuns_V_CalcRefdef()
{
	if (g_Config.cvars.quake_guns)
	{
		cl_entity_s *pViewModel = g_pEngineFuncs->GetViewModel();

		if (!pViewModel)
			return;

		if (s_iWeaponID == -1)
			return;

		float offset = GetWeaponOffset(pViewModel, s_iWeaponID);

		Vector va, right;

		float *org = pViewModel->origin;
		float *ang = pViewModel->angles;

		g_pEngineFuncs->GetViewAngles(va);
		g_pEngineFuncs->AngleVectors(va, NULL, right, NULL);

		org[0] += right[0] * offset;
		org[1] += right[1] * offset;
		org[2] += right[2] * offset;
	}
}

void CMisc::QuakeGuns_HUD_PostRunCmd(struct local_state_s *to)
{
	s_iWeaponID = to->client.m_iId;
}

//-----------------------------------------------------------------------------
// Init
//-----------------------------------------------------------------------------

CMisc::CMisc()
{
	m_pfnQueryPerformanceCounter = NULL;
	m_pfnNetchan_Transmit = NULL;
	m_pfnCHud__Think = NULL;

	m_hQueryPerformanceCounter = 0;
	m_hNetchan_Transmit = 0;
	m_hCHud__Think = 0;

	m_flSpinPitchAngle = 0.f;
	m_bSpinCanChangePitch = false;
}

bool CMisc::Load()
{
	ex_interp = CVar()->FindCvar("ex_interp");
	default_fov = CVar()->FindCvar("default_fov");

	if ( !ex_interp )
	{
		Warning("Can't find cvar ex_interp\n");
		return false;
	}

	if ( !default_fov )
	{
		Warning("Can't find cvar default_fov\n");
		return false;
	}

	m_pfnQueryPerformanceCounter = (void *)QueryPerformanceCounter;

	m_pfnCHud__Think = MemoryUtils()->FindPattern( g_pModules->Client, Patterns::Client::CHud__Think );

	if ( !m_pfnCHud__Think )
	{
		Warning("Couldn't find function CHud::Think\n");
		return false;
	}

	m_pfnNetchan_Transmit = MemoryUtils()->FindPattern( g_pModules->Hardware, Patterns::Hardware::Netchan_Transmit );

	if ( !m_pfnNetchan_Transmit )
	{
		Warning("Couldn't find function Netchan_Transmit\n");
		return false;
	}

	return true;
}

void CMisc::PostLoad()
{
	m_hQueryPerformanceCounter = DetoursAPI()->DetourFunction( m_pfnQueryPerformanceCounter, HOOKED_fQueryPerformanceCounter, GET_FUNC_PTR(ORIG_fQueryPerformanceCounter) );
	m_hNetchan_Transmit = DetoursAPI()->DetourFunction( m_pfnNetchan_Transmit, HOOKED_fNetchan_Transmit, GET_FUNC_PTR(ORIG_fNetchan_Transmit) );
	m_hCHud__Think = DetoursAPI()->DetourFunction( m_pfnCHud__Think, HOOKED_CHud__Think, GET_FUNC_PTR(ORIG_CHud__Think) );
}

void CMisc::Unload()
{
	DetoursAPI()->RemoveDetour( m_hQueryPerformanceCounter );
	DetoursAPI()->RemoveDetour( m_hNetchan_Transmit );
	DetoursAPI()->RemoveDetour( m_hCHud__Think );
}