// Misc

#include <ICvar.h>
#include <IMemoryUtils.h>
#include <ISvenModAPI.h>
#include <IInventory.h>
#include <IClient.h>
#include <IClientWeapon.h>

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

Vector g_vecSpinAngles(0.f, 0.f, 0.f);

static int s_iStickTarget = 0;
static Vector s_vecStickPrevPos;

static int s_nSinkState = 0;
static int s_iWeaponID = -1;

static bool s_bFreeze = false;
static bool s_bFreeze2 = false;
bool g_bForceFreeze2 = false;

static float s_flTopColorDelay = 0.0f;
static float s_flBottomColorDelay = 0.0f;

static int s_iTopColorOffset = 0;
static int s_iBottomColorOffset = 0;

static float s_flWeaponOffset[32] =
{
	0.0f, // 0
	-1.5f, // 1
	-4.0f, // 2
	0.0f, // 3
	-2.5f, // 4
	0.0f, // 5
	-4.0f, // 6
	-5.0f, // 7
	-9.2f, // 8
	-3.0f, // 9
	-5.0f, // 10
	-8.0f, // 11
	-4.0f, // 12
	0.0f, // 13
	0.0f, // 14
	-3.0f, // 15
	0.0f, // 16
	-4.15f, // 17
	-0.7f, // 18
	0.0f, // 19
	-2.0f, // 20
	0.0f, // 21
	-9.0f, // 22
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

static float GetWeaponOffset(cl_entity_s *pViewModel)
{
	if ( ClientWeapon()->IsCustom() )
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

			if ( !strcmp(pszModelName, "v_357.mdl") )
			{
				return -6.2f;
			}
		}
	}
	else
	{
		int iWeaponID = Client()->GetCurrentWeaponID();
		constexpr int iMaxWeapons = (sizeof(s_flWeaponOffset) / sizeof(s_flWeaponOffset[0]));

		Assert( iWeaponID >= 0 && iWeaponID < iMaxWeapons );

		iWeaponID = (int)clamp(iWeaponID, 0, iMaxWeapons - 1);

		return s_flWeaponOffset[ iWeaponID ];
	}

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Commands, CVars..
//-----------------------------------------------------------------------------

CON_COMMAND(sc_test, "Retrieve entity's info")
{
	if (args.ArgC() > 1)
	{
		int index = atoi(args[1]);

		cl_entity_s *pEntity = g_pEngineFuncs->GetEntityByIndex(index);

		if (pEntity)
		{
			Msg("Entity Pointer: %X\n", pEntity);

			if (pEntity->player)
			{
				Msg("Player Info Pointer: %X\n", g_pEngineStudio->PlayerInfo(index - 1));

				hud_player_info_t playerInfo;
				ZeroMemory(&playerInfo, sizeof(hud_player_info_s));

				g_pEngineFuncs->GetPlayerInfo(index, &playerInfo);

				if (playerInfo.name && playerInfo.model && *playerInfo.model)
					Msg("Model: %s\n", playerInfo.model);

				Msg("Top Color: %d\n", playerInfo.topcolor);
				Msg("Bottom Color: %d\n", playerInfo.bottomcolor);
			}
			else if (pEntity->model && pEntity->model->name)
			{
				Msg("Model: %s\n", pEntity->model->name);
			}
		}
	}
	else
	{
		ConMsg("Usage:  sc_test <entindex>\n");
	}
}

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
	if ( Client()->IsDead() )
		return;

	if (Client()->GetViewOffset().z == VEC_DUCK_VIEW.z)
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

CON_COMMAND(sc_stick, "Follow a player")
{
	if (args.ArgC() >= 2)
	{
		if ( Client()->IsDead() )
		{
			Msg("You're dead\n");
			return;
		}

		int iPlayerIndex = atoi(args[1]);

		if (iPlayerIndex > 0 && iPlayerIndex <= MAXCLIENTS)
		{
			cl_entity_t *pPlayer = g_pEngineFuncs->GetEntityByIndex(iPlayerIndex);

			if (pPlayer)
			{
				s_iStickTarget = iPlayerIndex;
				s_vecStickPrevPos = pPlayer->prevstate.origin;
			}
			else
			{
				Msg("No such player\n");
			}
		}
		else
		{
			s_iStickTarget = 0;
		}
	}
	else
	{
		Msg("Usage:  sc_stick <player index>\n");
	}
}

CON_COMMAND_NO_WRAPPER(sc_one_tick_exploit, "Exploits an action on one tick")
{
	Msg(g_Config.cvars.one_tick_exploit ? "One Tick Exploit disabled\n" : "One Tick Exploit enabled\n");
	g_Config.cvars.one_tick_exploit = !g_Config.cvars.one_tick_exploit;
}

CON_COMMAND_NO_WRAPPER(sc_fastcrowbar, "Toggle fast crowbar")
{
	Msg(g_Config.cvars.fast_crowbar ? "Fast Crowbar disabled\n" : "Fast Crowbar enabled\n");
	g_Config.cvars.fast_crowbar = !g_Config.cvars.fast_crowbar;
}

CON_COMMAND_NO_WRAPPER(sc_fastcrowbar2, "Toggle fast crowbar [auto freeze]")
{
	Msg(g_Config.cvars.fast_crowbar2 ? "Fast Crowbar #2 disabled\n" : "Fast Crowbar #2 enabled\n");
	g_Config.cvars.fast_crowbar2 = !g_Config.cvars.fast_crowbar2;
}

CON_COMMAND_NO_WRAPPER(sc_fastmedkit, "Toggle fast medkit")
{
	Msg(g_Config.cvars.fast_medkit ? "Fast Medkit disabled\n" : "Fast Medkit enabled\n");
	g_Config.cvars.fast_medkit = !g_Config.cvars.fast_medkit;
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

CON_COMMAND_NO_WRAPPER(sc_print_skybox_name, "sc_print_skybox_name - Prints current skybox name")
{
	if (g_pPlayerMove && g_pPlayerMove->movevars)
	{
		Msg("Skybox: %s\n", g_pPlayerMove->movevars->skyName);
	}
}

CON_COMMAND_NO_WRAPPER(sc_print_steamids, "sc_print_steamids - Print Steam64 IDs of players")
{
	for (int i = 1; i <= MAXCLIENTS; i++)
	{
		player_info_t *pPlayerInfo = g_pEngineStudio->PlayerInfo(i - 1);

		if ( pPlayerInfo )
		{
			Msg("%d. %s - %llu\n", i, pPlayerInfo->name, pPlayerInfo->m_nSteamID);
		}
	}
}

CON_COMMAND(sc_register_on_tick_command, "sc_register_on_tick_command - Register a command that will be called each tick")
{
	if (args.ArgC() >= 3)
	{
		const char *pszAlias = args[1];
		const char *pszCommand = args[2];

		if (*pszAlias == 0)
		{
			Msg("No alias\n");
			return;
		}
		
		if (*pszCommand == 0)
		{
			Msg("No command\n");
			return;
		}

		if (strlen(pszAlias) > 32)
		{
			Msg("Alias' name is too long!\n");
			return;
		}

		std::string sAlias = pszAlias;
		std::string sCommand = pszCommand;

		g_Misc.m_OnTickCommands.insert_or_assign( sAlias, sCommand );

		Msg("On tick command with alias \"%s\" was registered/replaced\n", pszAlias);
	}
	else
	{
		Msg("Usage:  sc_register_on_tick_command <alias> <command>\n");
	}
}

CON_COMMAND(sc_remove_on_tick_command, "sc_remove_on_tick_command - Remove a command that called each tick")
{
	if (args.ArgC() >= 2)
	{
		const char *pszAlias = args[1];

		if (*pszAlias == 0)
		{
			Msg("No alias\n");
			return;
		}
		
		if (strlen(pszAlias) > 32)
		{
			Msg("Alias' name is too long!\n");
			return;
		}

		std::string sAlias = pszAlias;

		auto found = g_Misc.m_OnTickCommands.find( sAlias );

		if ( found != g_Misc.m_OnTickCommands.end() )
		{
			if ( g_Misc.m_OnTickCommands.erase( sAlias ) == 1 )
			{
				Msg("On tick command with alias \"%s\" was removed\n", pszAlias);
			}
			else
			{
				Msg("Failed to remove on tick command with given alias\n");
			}
		}
		else
		{
			Msg("On tick command with alias \"%s\" isn't registered\n", pszAlias);
		}
	}
	else
	{
		Msg("Usage:  sc_remove_on_tick_command <alias>\n");
	}
}

CON_COMMAND_NO_WRAPPER(sc_print_on_tick_commands, "sc_print_on_tick_commands - Prints all on tick commands")
{
	Msg("[Alias = Command]\n");

	for (const std::pair<std::string, std::string> &pair : g_Misc.m_OnTickCommands)
	{
		Msg("%s = \"%s\"\n", pair.first.c_str(), pair.second.c_str());
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
	if (s_bFreeze2 || g_bForceFreeze2)
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
			if ( (pList->p->m_iFlags & HUD_ACTIVE) != 0 )
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
	// Execute on tick commands
	for (const std::pair<std::string, std::string> &pair : m_OnTickCommands)
	{
		g_pEngineFuncs->ClientCmd( pair.second.c_str() );
	}

	// Clamp speedhack values
	if ( sc_speedhack.GetFloat() < 0.0f )
		sc_speedhack.SetValue("0");
	
	if ( sc_speedhack_ltfx.GetFloat() < -100.0f )
		sc_speedhack_ltfx.SetValue("-100");

	// Set speedhack
	UTIL_SetGameSpeed( static_cast<double>(sc_speedhack.GetFloat()) );
	*dbRealtime += static_cast<double>(sc_speedhack_ltfx.GetFloat()) * frametime;

	FakeLag(frametime);
	ColorPulsator();
	TertiaryAttackGlitch();

	if ( g_Config.cvars.rotate_dead_body && g_pPlayerMove->dead )
	{
		Vector va;

		g_pEngineFuncs->GetViewAngles(va);
		cmd->viewangles = va;
	}

	if ( !Client()->IsSpectating() )
	{
		AutoSelfSink();
		AutoCeilClipping(cmd);
		AutoJump(cmd);
		JumpBug(frametime, cmd);
		DoubleDuck(cmd);
		FastRun(cmd);
		Spinner(cmd);
		Stick(cmd);
		OneTickExploit(cmd);
	}

	if ( s_bFreeze )
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
		if ( is_visible && type == ET_PLAYER && ent->index != Client()->GetPlayerIndex() )
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
// One tick exploit
//-----------------------------------------------------------------------------

void CMisc::OneTickExploit(struct usercmd_s *cmd)
{
	if ( !Client()->IsDead() )
	{
		bool bDoRapidAction = false;

		if ( g_Config.cvars.fast_crowbar )
		{
			if ( Client()->GetCurrentWeaponID() == WEAPON_CROWBAR )
			{
				if ( cmd->buttons & IN_ATTACK )
				{
					bDoRapidAction = true;
				}
			}
		}
		
		if ( g_Config.cvars.fast_medkit )
		{
			if ( Client()->GetCurrentWeaponID() == WEAPON_MEDKIT )
			{
				if ( cmd->buttons & (IN_ATTACK | IN_ATTACK2) )
				{
					bDoRapidAction = true;
				}
			}
		}

		if ( g_Config.cvars.fast_crowbar2 )
		{
			if ( Client()->GetCurrentWeaponID() == WEAPON_CROWBAR )
			{
				if ( cmd->buttons & IN_ATTACK )
				{
					bDoRapidAction = true;
					g_bForceFreeze2 = true;
				}
				else if ( Client()->ButtonLast() & IN_ATTACK )
				{
					bDoRapidAction = true;
					g_bForceFreeze2 = false;
				}
			}
		}

		if (bDoRapidAction)
		{
			if (m_iFakeLagCounter < 45)
			{
				bSendPacket = false;
				m_iFakeLagCounter++;
			}
			else
			{
				bSendPacket = true;
				m_iFakeLagCounter = 0;
			}

			UTIL_SetGameSpeed(20000.f);

			cmd->forwardmove = 0.f;
			cmd->sidemove = 0.f;
		}
	}
	else
	{
		m_iFakeLagCounter = 0;
	}

	if ( g_Config.cvars.one_tick_exploit )
	{
		if ( m_iOneTickExploitLagInterval < g_Config.cvars.one_tick_exploit_lag_interval )
		{
			bSendPacket = false;
			m_iOneTickExploitLagInterval++;
		}
		else
		{
			bSendPacket = true;
			m_iOneTickExploitLagInterval = 0;
		}

		UTIL_SetGameSpeed( g_Config.cvars.one_tick_exploit_speedhack );
	}
	else
	{
		m_iOneTickExploitLagInterval = 0;
	}
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

	if (g_Config.cvars.jumpbug && Client()->GetFallVelocity() > 500.0f && g_pPlayerMove->movevars != NULL)
	{
		Vector vecPredictVelocity = Client()->GetVelocity() * frametime;

		vecPredictVelocity.z = 0.f; // 2D only, height will be predicted separately

		Vector vecPredictOrigin = g_pPlayerMove->origin + vecPredictVelocity;
		Vector vBottomOrigin = vecPredictOrigin;

		vBottomOrigin.z -= 8192.0f;

		pmtrace_t *pTrace = g_pEngineFuncs->PM_TraceLine(vecPredictOrigin,
														 vBottomOrigin,
														 PM_NORMAL,
														 (Client()->GetFlags() & FL_DUCKING) ? 1 : 0 /* g_pPlayerMove->usehull */,
														 -1);

		float flHeight = fabsf(pTrace->endpos.z - vecPredictOrigin.z);
		//float flGroundNormalAngle = acos(pTrace->plane.normal.z);

		if ( /* flGroundNormalAngle <= acosf(0.7f) && */ Client()->GetWaterLevel() == WL_NOT_IN_WATER && g_pEngineFuncs->PM_WaterEntity(pTrace->endpos) == -1 )
		{
			float flFrameZDist = fabsf( (Client()->GetFallVelocity() + (g_pPlayerMove->movevars->gravity * frametime)) * frametime );

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
				if (flFrameZDist > 0.f && fabsf(flHeight - flFrameZDist * 1.5f) <= 20.f)
				{
					float flNeedSpeed = fabsf(flHeight - 19.f);
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

		if ( bForceUnduck )
		{
			cmd->buttons &= ~IN_DUCK;

			bForceUnduck = false;
		}
		else if ( Client()->IsOnGround() )
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
	if ( g_Config.cvars.fastrun && Client()->IsOnGround() )
	{
		static bool bFastRun = false;
		float flMaxSpeed = Client()->GetMaxSpeed();

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

static bool IsFiring(usercmd_t *cmd)
{
	static float last_displacer_state = 0.f;
	static int throw_nade_state = 0;

	if ( Client()->GetCurrentWeaponID() > WEAPON_NONE )
	{
		switch ( Client()->GetCurrentWeaponID() )
		{
		case WEAPON_RPG:
			if ( ClientWeapon()->GetWeaponData()->iuser4 && ClientWeapon()->GetWeaponData()->fuser1 != 0.f )
				return true;

			if ( cmd->buttons & IN_ATTACK2 )
				return false;

			break;

		case WEAPON_GAUSS:
			if ( ClientWeapon()->GetWeaponData()->fuser4 > 0.f )
			{
				if ( Client()->ButtonLast() & IN_ATTACK2 )
				{
					if ( !(cmd->buttons & IN_ATTACK2) )
						return true;
				}
				else if ( Client()->ButtonLast() & IN_ALT1 )
				{
					if ( !(cmd->buttons & IN_ALT1) )
						return true;
				}
				else if ( ClientWeapon()->GetWeaponData()->fuser4 == 1.f )
				{
					return true;
				}

				return false;
			}

			break;

		case WEAPON_HANDGRENADE:
			if ( ClientWeapon()->GetWeaponData()->fuser1 < 0.f && throw_nade_state != 2 )
			{
				throw_nade_state = 1;

				if ( Client()->ButtonLast() & (IN_ATTACK | IN_ATTACK2) )
				{
					if ( !(cmd->buttons & (IN_ATTACK | IN_ATTACK2)) )
						return true;
				}
				else
				{
					if ( !(cmd->buttons & (IN_ATTACK | IN_ATTACK2)) )
						throw_nade_state = 2;
				}
			}

			if ( ClientWeapon()->GetWeaponData()->fuser2 < 0.f && throw_nade_state == 2 )
				return true;

			throw_nade_state = 0;
			return false;

		case WEAPON_DISPLACER:
			last_displacer_state = ClientWeapon()->GetWeaponData()->fuser1;

			if ( last_displacer_state == 1.f )
				return true;

			return false;
		}

		if ( cmd->buttons & (IN_ATTACK | IN_ATTACK2) && Client()->CanAttack() && !ClientWeapon()->IsReloading() )
		{
			if (cmd->buttons & IN_ATTACK)
			{
				if ( ClientWeapon()->CanPrimaryAttack() )
					return true;
			}
			else
			{
				if ( ClientWeapon()->CanSecondaryAttack() )
					return true;
			}
		}
	}

	return false;
}

static bool IsBusyWithLongJump(usercmd_t *cmd)
{
	if ( cmd->buttons & IN_JUMP && Client()->IsOnGround() )
	{
		if ( Client()->IsDucking() || Client()->GetFlags() & FL_DUCKING)
		{
			if ( cmd->buttons & IN_DUCK && g_pPlayerMove->flDuckTime > 0.f )
			{
				const char *pszValue = g_pEngineFuncs->PhysInfo_ValueForKey("slj");
				bool bCanSuperJump = (pszValue && *pszValue == '1');

				if ( bCanSuperJump && Client()->GetVelocity().Length() > 50.f)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void CMisc::Spinner(struct usercmd_s *cmd)
{
	bool bAnglesChanged = false;
	m_bSpinCanChangePitch = false;

	if (g_Config.cvars.spin_yaw_angle)
	{
		if ( !g_Config.cvars.lock_pitch && !g_Config.cvars.spin_pitch_angle )
			g_vecSpinAngles.x = cmd->viewangles.x;

		g_vecSpinAngles.y += g_Config.cvars.spin_yaw_rotation_angle;
		g_vecSpinAngles.y = NormalizeAngle(g_vecSpinAngles.y);

		bAnglesChanged = true;
	}
	else if (g_Config.cvars.lock_yaw)
	{
		if ( !g_Config.cvars.lock_pitch && !g_Config.cvars.spin_pitch_angle )
			g_vecSpinAngles.x = cmd->viewangles.x;

		g_vecSpinAngles.y = g_Config.cvars.lock_yaw_angle;
		bAnglesChanged = true;
	}

	if (g_Config.cvars.spin_pitch_angle)
	{
		if ( !g_Config.cvars.lock_yaw && !g_Config.cvars.spin_yaw_angle )
			g_vecSpinAngles.y = cmd->viewangles.y;

		g_vecSpinAngles.x += g_Config.cvars.spin_pitch_rotation_angle;
		g_vecSpinAngles.x = NormalizeAngle(g_vecSpinAngles.x);

		m_bSpinCanChangePitch = true;
		bAnglesChanged = true;
	}
	else if (g_Config.cvars.lock_pitch)
	{
		if ( !g_Config.cvars.lock_yaw && !g_Config.cvars.spin_yaw_angle )
			g_vecSpinAngles.y = cmd->viewangles.y;

		g_vecSpinAngles.x = g_Config.cvars.lock_pitch_angle;

		m_bSpinCanChangePitch = true;
		bAnglesChanged = true;
	}

	if (bAnglesChanged)
	{
		if (Client()->GetMoveType() == MOVETYPE_WALK && Client()->GetWaterLevel() <= WL_FEET)
		{
			if ( !(cmd->buttons & IN_USE || cmd->impulse == 201 || IsFiring(cmd) || IsBusyWithLongJump(cmd)) )
			{
				m_flSpinPitchAngle = g_vecSpinAngles.x / -3.0f;
				UTIL_SetAnglesSilent(g_vecSpinAngles, cmd);
			}
			else
			{
				m_bSpinCanChangePitch = false;
			}
		}
		else
		{
			m_bSpinCanChangePitch = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Stick
//-----------------------------------------------------------------------------

void CMisc::Stick(struct usercmd_s *cmd)
{
	if (s_iStickTarget)
	{
		cl_entity_t *pPlayer = g_pEngineFuncs->GetEntityByIndex(s_iStickTarget);

		if ( pPlayer && pPlayer->curstate.messagenum >= g_pEngineFuncs->GetLocalPlayer()->curstate.messagenum && !Client()->IsDead() )
		{
			Vector2D vecForward;
			Vector2D vecRight;

			Vector vPredictPos = pPlayer->curstate.origin + (pPlayer->curstate.origin - s_vecStickPrevPos);

			Vector2D vecDir = vPredictPos.AsVector2D() - g_pPlayerMove->origin.AsVector2D();
			vecDir.NormalizeInPlace();

			vecForward.x = cosf(cmd->viewangles.y * static_cast<float>(M_PI) / 180.f);
			vecForward.y = sinf(cmd->viewangles.y * static_cast<float>(M_PI) / 180.f);

			vecRight.x = vecForward.y;
			vecRight.y = -vecForward.x;

			vecForward *= Client()->GetMaxSpeed();
			vecRight *= Client()->GetMaxSpeed();

			float forwardmove = DotProduct(vecForward, vecDir);
			float sidemove = DotProduct(vecRight, vecDir);

			cmd->forwardmove = forwardmove;
			cmd->sidemove = sidemove;

			if (Client()->GetWaterLevel() > WL_NOT_IN_WATER && pPlayer->curstate.origin.z >= g_pPlayerMove->origin.z)
			{
				if ( Client()->GetFlags() & FL_WATERJUMP )
					cmd->buttons |= IN_DUCK;

				cmd->upmove = Client()->GetMaxSpeed();
			}

			s_vecStickPrevPos = pPlayer->curstate.origin;
		}
		else
		{
			s_iStickTarget = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Auto Ceil Clipping
//-----------------------------------------------------------------------------

void CMisc::AutoCeilClipping(struct usercmd_s *cmd)
{
	static bool jumped = false;

	if ( g_Config.cvars.auto_ceil_clipping && !Client()->IsDead() )
	{
		if (jumped)
		{
			if ( !Client()->IsOnGround() && Client()->GetWaterLevel() <= WL_FEET)
			{
				cmd->buttons |= IN_DUCK;

				// Suicide only if we got apex or started falling
				if ( Client()->GetVelocity().z <= 0.f )
				{
					Vector vecStart = g_pPlayerMove->origin;
					Vector vecEnd = vecStart + Vector(0.f, 0.f, VEC_DUCK_HULL_MAX.z);

					pmtrace_t *pTrace = g_pEngineFuncs->PM_TraceLine(vecStart, vecEnd, PM_NORMAL, (Client()->GetFlags() & FL_DUCKING) ? 1 : 0, -1);

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
			if ( !Client()->IsOnGround() && Client()->GetVelocity().z > 0.f)
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

	if ( g_Config.cvars.fakelag_adaptive_ex_interp )
	{
		if (ex_interp->value != 0.01f)
			ex_interp->value = 0.01f;

		bSetInterpOnce = true;
	}
	else if ( bSetInterpOnce )
	{
		if (ex_interp->value == 0.01f)
			ex_interp->value = 0.1f;

		bSetInterpOnce = false;
	}

	if ( g_Config.cvars.fakelag )
	{
		bool bFakeLag = true;

		if ( g_Config.cvars.fakelag_move != 0 )
		{
			float flVelocity = Client()->GetVelocity().Length2D();

			if ( g_Config.cvars.fakelag_move == 1 ) // On land
			{
				if ( flVelocity > 0.f )
					bFakeLag = false;
			}
			else if ( g_Config.cvars.fakelag_move == 2 ) // On move
			{
				if ( flVelocity == 0.f )
					bFakeLag = false;
			}
			else if ( g_Config.cvars.fakelag_move == 3 ) // In air
			{
				if ( Client()->IsOnGround() )
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
				Vector velocity = Client()->GetVelocity();
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
	else if (s_nSinkState == 2 && Client()->GetViewOffset().z == VEC_DUCK_VIEW.z)
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

		if ( !pViewModel )
			return;

		if ( Client()->GetCurrentWeaponID() == WEAPON_NONE )
			return;

		float offset = GetWeaponOffset(pViewModel);

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

	m_iFakeLagCounter = 0;
	m_iOneTickExploitLagInterval = 0;
}

CMisc::~CMisc()
{
	m_OnTickCommands.clear();
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

	m_pfnCHud__Think = MemoryUtils()->FindPattern( SvenModAPI()->Modules()->Client, Patterns::Client::CHud__Think );

	if ( !m_pfnCHud__Think )
	{
		Warning("Couldn't find function CHud::Think\n");
		return false;
	}

	m_pfnNetchan_Transmit = MemoryUtils()->FindPattern( SvenModAPI()->Modules()->Hardware, Patterns::Hardware::Netchan_Transmit );

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