// Client Module

#include "client.h"

#include "menu.h"

#include "../interfaces.h"
#include "../game/utils.h"
#include "../game/console.h"

#include "../config.h"
#include "../patterns.h"

#include "../libdasm/libdasm.h"
#include "../utils/vtable_hook.h"
#include "../utils/signature_scanner.h"
#include "../utils/trampoline_hook.h"

#include "../features/advanced_mute_system.h"
#include "../features/strafer.h"
#include "../features/antiafk.h"
#include "../features/autovote.h"
#include "../features/keyspam.h"
#include "../features/camhack.h"
#include "../features/misc.h"
#include "../features/firstperson_roaming.h"
#include "../features/message_spammer.h"
#include "../features/skybox.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//-----------------------------------------------------------------------------
// Global Vars
//-----------------------------------------------------------------------------

local_player_s g_Local;

playermove_s *g_pPlayerMove = NULL;
netchan_t *g_pNetchan = NULL;

bool bSendPacket = true;

//-----------------------------------------------------------------------------
// Init hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(Netchan_CanPacket_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

Netchan_CanPacketFn Netchan_CanPacket_Original = NULL;
HUD_InitFn HUD_Init_Original = NULL;
CL_CreateMoveFn CL_CreateMove_Original = NULL;
HUD_PlayerMoveFn HUD_PlayerMove_Original = NULL;
V_CalcRefdefFn V_CalcRefdef_Original = NULL;
HUD_PostRunCmdFn HUD_PostRunCmd_Original = NULL;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void UpdateLocalPlayer()
{
	Vector vBottomOrigin = g_pPlayerMove->origin; vBottomOrigin.z -= 8192.0f;
	pmtrace_t *pTrace = g_pEngineFuncs->PM_TraceLine(g_pPlayerMove->origin, vBottomOrigin, PM_NORMAL, /* g_pPlayerMove->usehull */ (g_pPlayerMove->flags & FL_DUCKING) ? 1 : 0, -1);

	g_Local.flHeight = fabsf(pTrace->endpos.z - g_pPlayerMove->origin.z);
	g_Local.flGroundNormalAngle = static_cast<float>(acos(pTrace->plane.normal[2]) * 180.0 / M_PI);
	g_Local.flVelocity = g_pPlayerMove->velocity.Length2D();
}

CON_COMMAND_FUNC(toggle, Toggle_Cmd, "toggle [cvar_name] [value #1] [value #2] - Toggle between two values")
{
	if (CMD_ARGC() >= 4)
	{
		const char *pszCvar = CMD_ARGV(1);
		cvar_s *pCvar = g_pEngineFuncs->pfnGetCvarPointer(pszCvar);

		if (pCvar)
		{
			const char *pszValue = pCvar->string;

			char *pszValueOne = CMD_ARGV(2);
			char *pszValueTwo = CMD_ARGV(3);

			if (!strcmp(pszValue, pszValueOne))
			{
				g_pEngineFuncs->pfnCvar_Set(pszCvar, pszValueTwo);
			}
			else
			{
				g_pEngineFuncs->pfnCvar_Set(pszCvar, pszValueOne);
			}
		}
	}
	else
	{
		toggle.PrintUsage();
	}
}

CON_COMMAND_FUNC(sc_load_config, ReloadConfig_Cmd, "sc_load_config - Load config from file sven_internal.ini")
{
	g_Config.Load();
}

CON_COMMAND_FUNC(sc_save_config, SaveConfig_Cmd, "sc_save_config - Save config to file sven_internal.ini")
{
	g_Config.Save();
}

CON_COMMAND(test, "test [entidx] - Retrieves an entity info")
{
	if (CMD_ARGC() >= 2)
	{
		int index = atoi(CMD_ARGV(1));

		cl_entity_s *pEntity = g_pEngineFuncs->GetEntityByIndex(index);

		if (pEntity)
		{
			Msg("Entity Pointer: %X\n", pEntity);

			if (pEntity->player)
			{
				Msg("Player Info Pointer: %X\n", g_pEngineStudio->PlayerInfo(index - 1));

				hud_player_info_t playerInfo;
				ZeroMemory(&playerInfo, sizeof(hud_player_info_s));

				g_pEngineFuncs->pfnGetPlayerInfo(index, &playerInfo);

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
		test.PrintUsage();
	}
};

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

static Vector s_lastViewAngles = { 0.0f, 0.0f, 0.0f };

void OnMenuOpen()
{
	g_pEngineFuncs->GetViewAngles(s_lastViewAngles);
}

void OnMenuClose()
{
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

qboolean Netchan_CanPacket_Hooked(netchan_t *chan)
{
	if (!bSendPacket)
		return 0;

	return Netchan_CanPacket_Original(chan);
}

int HUD_Init_Hooked(void)
{
	int result = HUD_Init_Original();

	return result;
}

void CL_CreateMove_Hooked(float frametime, struct usercmd_s *cmd, int active)
{
	g_Skybox.Think();

	bSendPacket = true;

	static int s_nWaitFrames = 0;

	if (g_bMenuEnabled)
	{
		cmd->viewangles = s_lastViewAngles;
		goto SKIP_CREATEMOVE;
	}

	if (g_bMenuClosed)
	{
		if (++s_nWaitFrames > 5)
		{
			g_bMenuClosed = false;
		}
		else
		{
			cmd->viewangles = s_lastViewAngles;
			g_pClientFuncs->IN_ClearStates();

			goto SKIP_CREATEMOVE;
		}
	}
	else
	{
		s_nWaitFrames = 0;
	}

	CL_CreateMove_Original(frametime, cmd, active);

SKIP_CREATEMOVE:
	if (!g_pPlayerMove || !g_pEngineFuncs->GetLocalPlayer())
		return;

	UpdateLocalPlayer();

	g_AntiAFK.CreateMove(frametime, cmd, active);
	g_Strafer.CreateMove(frametime, cmd, active);
	g_KeySpam.CreateMove(frametime, cmd, active);
	g_Misc.CreateMove(frametime, cmd, active);
	g_CamHack.CreateMove(frametime, cmd, active);
	g_MessageSpammer.CreateMove(frametime, cmd, active);
}

void HUD_PlayerMove_Hooked(struct playermove_s *ppmove, int server)
{
	HUD_PlayerMove_Original(ppmove, server);

	g_pPlayerMove = ppmove;
}

void V_CalcRefdef_Hooked(struct ref_params_s *pparams)
{
	V_CalcRefdef_Original(pparams);

	g_Misc.V_CalcRefdef(pparams);
	g_CamHack.V_CalcRefdef(pparams);
	g_FirstPersonRoaming.V_CalcRefdef(pparams);
}

void HUD_PostRunCmd_Hooked(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed)
{
	HUD_PostRunCmd_Original(from, to, cmd, runfuncs, time, random_seed);

	g_Misc.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);
}

//-----------------------------------------------------------------------------
// Init/release client module
//-----------------------------------------------------------------------------

void InitClientModule()
{
	//void *pNetchan_TransmitString = LookupForString(L"hw.dll", "%s:Outgoing");
	//void *pNetchan_Transmit = FindAddress(L"hw.dll", pNetchan_TransmitString);

	//pNetchan_Transmit = (void *)((BYTE *)pNetchan_Transmit - 0x75);

	void *pNetchan_CanPacket = FIND_PATTERN(L"hw.dll", Patterns::Hardware::Netchan_CanPacket);

	if (!pNetchan_CanPacket)
	{
		ThrowError("'Netchan_CanPacket' failed initialization\n");
		return;
	}

	InitAMS();
	InitAutoVote();

	g_MessageSpammer.Init();
	g_AntiAFK.Init();
	g_Strafer.Init();
	g_KeySpam.Init();
	g_CamHack.Init();
	g_Misc.Init();
	g_Skybox.Init();

	//HUD_Init_Original = g_pClientFuncs->HUD_Init;
	//g_pClientFuncs->HUD_Init = HUD_Init_Hooked;
	
	HUD_PostRunCmd_Original = g_pClientFuncs->HUD_PostRunCmd;
	g_pClientFuncs->HUD_PostRunCmd = HUD_PostRunCmd_Hooked;

	V_CalcRefdef_Original = g_pClientFuncs->V_CalcRefdef;
	g_pClientFuncs->V_CalcRefdef = V_CalcRefdef_Hooked;

	CL_CreateMove_Original = g_pClientFuncs->CL_CreateMove;
	g_pClientFuncs->CL_CreateMove = CL_CreateMove_Hooked;

	HUD_PlayerMove_Original = g_pClientFuncs->HUD_PlayerMove;
	g_pClientFuncs->HUD_PlayerMove = HUD_PlayerMove_Hooked;

	HOOK_FUNCTION(Netchan_CanPacket_Hook, pNetchan_CanPacket, Netchan_CanPacket_Hooked, Netchan_CanPacket_Original, Netchan_CanPacketFn);

	REGISTER_COMMAND("sc_help", PrintConsoleHelp);

	//REGISTER_COMMAND("toggle", Toggle_Cmd);
	//REGISTER_COMMAND("sc_load_config", ReloadConfig_Cmd);
	//REGISTER_COMMAND("sc_save_config", SaveConfig_Cmd);
}

void ReleaseClientModule()
{
	g_pClientFuncs->CL_CreateMove = CL_CreateMove_Original;
	g_pClientFuncs->HUD_PlayerMove = HUD_PlayerMove_Original;
}