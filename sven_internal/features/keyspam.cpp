// Key Spam

#include "keyspam.h"

#include "../sdk.h"
#include "../game/utils.h"
#include "../game/console.h"

#include "../config.h"

//-----------------------------------------------------------------------------

CKeySpam g_KeySpam;

//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_spam_use, ConCommand_UseSpam, "sc_spam_use - Toggle E spam")
{
	g_pEngineFuncs->pfnClientCmd("-use");

	Msg(g_Config.cvars.keyspam_e ? "Use Spam disabled\n" : "Use Spam enabled\n");
	g_Config.cvars.keyspam_e = !g_Config.cvars.keyspam_e;
}

CON_COMMAND_FUNC(sc_spam_forward, ConCommand_ForwardSpam, "sc_spam_forward - Toggle W spam")
{
	g_pEngineFuncs->pfnClientCmd("-forward");

	Msg(g_Config.cvars.keyspam_w ? "Forward Spam disabled\n" : "Forward Spam enabled\n");
	g_Config.cvars.keyspam_w = !g_Config.cvars.keyspam_w;
}

CON_COMMAND_FUNC(sc_spam_back, ConCommand_BackSpam, "sc_spam_back - Toggle S spam")
{
	g_pEngineFuncs->pfnClientCmd("-back");

	Msg(g_Config.cvars.keyspam_s ? "Back Spam disabled\n" : "Back Spam enabled\n");
	g_Config.cvars.keyspam_s = !g_Config.cvars.keyspam_s;
}

CON_COMMAND_FUNC(sc_spam_snark, ConCommand_SnarkSpam, "sc_spam_snark - Toggle Q spam")
{
	Msg(g_Config.cvars.keyspam_q ? "Snark Spam disabled\n" : "Snark Spam enabled\n");
	g_Config.cvars.keyspam_q = !g_Config.cvars.keyspam_q;
}

CON_COMMAND_FUNC(sc_spam_ctrl, ConCommand_CtrlSpam, "sc_spam_ctrl - Toggle CTRL spam")
{
	Msg(g_Config.cvars.keyspam_ctrl ? "CTRL Spam disabled\n" : "CTRL Spam enabled\n");
	g_Config.cvars.keyspam_ctrl = !g_Config.cvars.keyspam_ctrl;
}

//-----------------------------------------------------------------------------

void CKeySpam::CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	KeySpam();
}

void CKeySpam::KeySpam()
{
	bool bSpam = !g_Config.cvars.keyspam_hold_mode;

	if (g_Config.cvars.keyspam_e)
	{
		static bool key_down = true;

		if (bSpam || GetAsyncKeyState(0x45)) // E
		{
			if (key_down)
				g_pEngineFuncs->pfnClientCmd("-use");
			else
				g_pEngineFuncs->pfnClientCmd("+use");

			key_down = !key_down;
		}
		else
		{
			g_pEngineFuncs->pfnClientCmd("-use");
			key_down = false;
		}
	}

	if (g_Config.cvars.keyspam_w)
	{
		static bool key_down = true;

		if (bSpam || GetAsyncKeyState(0x57)) // W
		{
			if (key_down)
				g_pEngineFuncs->pfnClientCmd("-forward");
			else
				g_pEngineFuncs->pfnClientCmd("+forward");

			key_down = !key_down;
		}
		else
		{
			g_pEngineFuncs->pfnClientCmd("-forward");
			key_down = false;
		}
	}

	if (g_Config.cvars.keyspam_s)
	{
		static bool key_down = true;

		if (bSpam || GetAsyncKeyState(0x53)) // S
		{
			if (key_down)
				g_pEngineFuncs->pfnClientCmd("-back");
			else
				g_pEngineFuncs->pfnClientCmd("+back");

			key_down = !key_down;
		}
		else
		{
			g_pEngineFuncs->pfnClientCmd("-back");
			key_down = false;
		}
	}
	
	if (g_Config.cvars.keyspam_ctrl)
	{
		static bool key_down = true;

		if (bSpam || GetAsyncKeyState(VK_LCONTROL)) // CTRL
		{
			if (key_down)
				g_pEngineFuncs->pfnClientCmd("-duck");
			else
				g_pEngineFuncs->pfnClientCmd("+duck");

			key_down = !key_down;
		}
		else
		{
			g_pEngineFuncs->pfnClientCmd("-duck");
			key_down = false;
		}
	}

	if (g_Config.cvars.keyspam_q && (bSpam || GetAsyncKeyState(0x51))) // Q
	{
		g_pEngineFuncs->pfnClientCmd("lastinv");
	}
}

//-----------------------------------------------------------------------------

void CKeySpam::Init()
{
	//keyspam_hold = REGISTER_CVAR("sc_spam_hold", "1", 0);
}