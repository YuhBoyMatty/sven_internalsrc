// Client Module

#include "client.h"

#include "menu.h"

#include "../interfaces.h"
#include "../game/utils.h"
#include "../game/console.h"
#include "../game/player_utils.h"

#include "../config.h"
#include "../patterns.h"

#include "../libdasm/libdasm.h"
#include "../utils/vtable_hook.h"
#include "../utils/signature_scanner.h"
#include "../utils/trampoline_hook.h"

#include "../features/advanced_mute_system.h"
#include "../features/strafer.h"
#include "../features/antiafk.h"
#include "../features/keyspam.h"
#include "../features/camhack.h"
#include "../features/misc.h"
#include "../features/firstperson_roaming.h"
#include "../features/message_spammer.h"
#include "../features/skybox.h"
#include "../features/custom_vote_popup.h"
#include "../features/chat_colors.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef int (*HUD_InitFn)(void);
typedef int (*HUD_VidInitFn)(void);
typedef void (*CL_CreateMoveFn)(float, struct usercmd_s *, int);
typedef void (*HUD_PlayerMoveFn)(struct playermove_s *, int);
typedef void (*V_CalcRefdefFn)(struct ref_params_s *);
typedef void (*HUD_PostRunCmdFn)(struct local_state_s *, struct local_state_s *, struct usercmd_s *, int, double, unsigned int);
typedef int (*HUD_Key_EventFn)(int, int, const char *);

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern bool bSendPacket;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

local_player_s g_Local;
playermove_s *g_pPlayerMove = NULL;
extra_player_info_t *g_pPlayerExtraInfo = NULL;

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

HUD_VidInitFn HUD_VidInit_Original = NULL;
HUD_InitFn HUD_Init_Original = NULL;
CL_CreateMoveFn CL_CreateMove_Original = NULL;
HUD_PlayerMoveFn HUD_PlayerMove_Original = NULL;
V_CalcRefdefFn V_CalcRefdef_Original = NULL;
HUD_PostRunCmdFn HUD_PostRunCmd_Original = NULL;
HUD_Key_EventFn HUD_Key_Event_Original = NULL;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

static void UpdateLocalPlayer()
{
	Vector vBottomOrigin = g_pPlayerMove->origin; vBottomOrigin.z -= 8192.0f;
	pmtrace_t *pTrace = g_pEngineFuncs->PM_TraceLine(g_pPlayerMove->origin, vBottomOrigin, PM_NORMAL, /* g_pPlayerMove->usehull */ (g_pPlayerMove->flags & FL_DUCKING) ? 1 : 0, -1);

	g_Local.flHeight = fabsf(pTrace->endpos.z - g_pPlayerMove->origin.z);
	g_Local.flGroundNormalAngle = static_cast<float>(acos(pTrace->plane.normal[2]) * 180.0 / M_PI);
	g_Local.flVelocity = g_pPlayerMove->velocity.Length2D();
}

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

int HUD_Init_Hooked(void)
{
	g_VotePopup.OnHUDInit();
	g_ChatColors.OnHUDInit();
	g_CamHack.OnHUDInit();
	g_AntiAFK.OnHUDInit();

	return HUD_Init_Original();
}

int HUD_VidInit_Hooked(void)
{
	g_VotePopup.OnVideoInit();
	g_ChatColors.OnVideoInit();
	g_CamHack.OnVideoInit();
	g_AntiAFK.OnVideoInit();

	return HUD_VidInit_Original();
}

void CL_CreateMove_Hooked(float frametime, struct usercmd_s *cmd, int active)
{
	g_Skybox.Think();
	g_ChatColors.OnCreateMove();

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

	g_Strafer.CreateMove(frametime, cmd, active);
	g_KeySpam.CreateMove(frametime, cmd, active);
	g_Misc.CreateMove(frametime, cmd, active);
	g_AntiAFK.CreateMove(frametime, cmd, active);
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

int HUD_Key_Event_Hooked(int down, int keynum, const char *pszCurrentBinding)
{
	if (g_bMenuEnabled)
		return 0;

	g_VotePopup.OnKeyPress(down, keynum);

	return HUD_Key_Event_Original(down, keynum, pszCurrentBinding);
}

//-----------------------------------------------------------------------------
// Init/release client module
//-----------------------------------------------------------------------------

void InitClientModule()
{
	extern void *pGetClientColor; // Chat Colors

	INSTRUCTION instruction;
	int disassembledBytes = 0;

	pGetClientColor = FIND_PATTERN(L"client.dll", Patterns::Client::GetClientColor);

	if (!pGetClientColor)
	{
		Sys_Error("'GetClientColor' failed initialization\n");
		return;
	}

	BYTE *pdisGetClientColor = (BYTE *)pGetClientColor;

	do
	{
		int length = get_instruction(&instruction, pdisGetClientColor, MODE_32);

		disassembledBytes += length;
		pdisGetClientColor += length;

		if (instruction.type == INSTRUCTION_TYPE_MOVSX && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
		{
			g_pPlayerExtraInfo = reinterpret_cast<extra_player_info_t *>(instruction.op2.displacement);
			break;
		}
	} while (disassembledBytes < 0x80);

	// Init features
	InitAMS();

	g_MessageSpammer.Init();
	g_AntiAFK.Init();
	g_Strafer.Init();
	g_KeySpam.Init();
	g_CamHack.Init();
	g_Misc.Init();
	g_Skybox.Init();
	g_VotePopup.Init();
	g_ChatColors.Init();

	// Hook client functions
	HUD_Init_Original = g_pClientFuncs->HUD_Init;
	g_pClientFuncs->HUD_Init = HUD_Init_Hooked;
	
	HUD_VidInit_Original = g_pClientFuncs->HUD_VidInit;
	g_pClientFuncs->HUD_VidInit = HUD_VidInit_Hooked;
	
	HUD_PostRunCmd_Original = g_pClientFuncs->HUD_PostRunCmd;
	g_pClientFuncs->HUD_PostRunCmd = HUD_PostRunCmd_Hooked;

	V_CalcRefdef_Original = g_pClientFuncs->V_CalcRefdef;
	g_pClientFuncs->V_CalcRefdef = V_CalcRefdef_Hooked;

	CL_CreateMove_Original = g_pClientFuncs->CL_CreateMove;
	g_pClientFuncs->CL_CreateMove = CL_CreateMove_Hooked;

	HUD_PlayerMove_Original = g_pClientFuncs->HUD_PlayerMove;
	g_pClientFuncs->HUD_PlayerMove = HUD_PlayerMove_Hooked;
	
	HUD_Key_Event_Original = g_pClientFuncs->HUD_Key_Event;
	g_pClientFuncs->HUD_Key_Event = HUD_Key_Event_Hooked;
}

void ShutdownClientModule()
{
	//g_pClientFuncs->CL_CreateMove = CL_CreateMove_Original;
	//g_pClientFuncs->HUD_PlayerMove = HUD_PlayerMove_Original;
}