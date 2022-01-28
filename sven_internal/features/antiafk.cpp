// Anti-AFK

#include "antiafk.h"

#include "../sdk.h"
#include "../interfaces.h"

#include "../game/utils.h"
#include "../game/console.h"
#include "../game/mathlib.h"
#include "../config.h"

//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;

//-----------------------------------------------------------------------------

CAntiAFK g_AntiAFK;

//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_antiafk, ConCommand_AntiAFK, "sc_antiafk [mode] - Set Anti-AFK Mode")
{
	if (CMD_ARGC() >= 2)
	{
		int nMode = atoi(CMD_ARGV(1));

		if (nMode >= 0 && nMode <= 5)
		{
			g_Config.cvars.antiafk = nMode;
		}
	}
	else
	{
		sc_antiafk.PrintUsage();
	}
}

void RotateCamera()
{
	Vector va;

	float flRotationAngle = g_Config.cvars.antiafk_rotation_angle;
	float flRotationAngleAbs = fabsf(flRotationAngle);

	static float s_flPitchDirection = 1.0f;

	g_pEngineFuncs->GetViewAngles(va);

	if (s_flPitchDirection > 0.0f)
	{
		if (va.x + flRotationAngleAbs > 89.0f)
			s_flPitchDirection = -1.0f;
		else
			va.x += flRotationAngleAbs;
	}

	if (s_flPitchDirection < 0.0f)
	{
		if (va.x - flRotationAngleAbs < -89.0f)
			s_flPitchDirection = 1.0f;
		else
			va.x -= flRotationAngleAbs;
	}

	va.y += flRotationAngle;
	va.y = NormalizeAngle(va.y);

	g_pEngineFuncs->SetViewAngles(va);
}

void WalkAround(struct usercmd_s *cmd, int &delay, int &movement_button, const int delay_count)
{
REPEAT:
	if (movement_button == IN_MOVERIGHT)
	{
		if (delay > delay_count)
		{
			movement_button = IN_BACK;
			delay = 0;

			goto REPEAT;
		}
		else
		{
			cmd->sidemove = g_pPlayerMove->clientmaxspeed;
		}
	}
	else if (movement_button == IN_BACK)
	{
		if (delay > delay_count)
		{
			movement_button = IN_MOVELEFT;
			delay = 0;

			goto REPEAT;
		}
		else
		{
			cmd->forwardmove = -g_pPlayerMove->clientmaxspeed;
		}
	}
	else if (movement_button == IN_MOVELEFT)
	{
		if (delay > delay_count)
		{
			movement_button = IN_FORWARD;
			delay = 0;

			goto REPEAT;
		}
		else
		{
			cmd->sidemove = -g_pPlayerMove->clientmaxspeed;
		}
	}
	else if (movement_button == IN_FORWARD)
	{
		if (delay > delay_count)
		{
			movement_button = IN_MOVERIGHT;
			delay = 0;

			goto REPEAT;
		}
		else
		{
			cmd->forwardmove = g_pPlayerMove->clientmaxspeed;
		}
	}

	cmd->buttons |= movement_button;
	++delay;
}

void CAntiAFK::CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	AntiAFK(cmd);
}

void CAntiAFK::AntiAFK(struct usercmd_s *cmd)
{
	int nMode = g_Config.cvars.antiafk;

	if (!nMode)
		return;

	if (nMode == 1)
	{
		static bool forward_step = true;

		cmd->forwardmove = forward_step ? 100.0f : -100.0f;

		forward_step = !forward_step;
	}
	else if (nMode == 2)
	{
		g_pEngineFuncs->pfnClientCmd("gibme");
	}
	else if (nMode == 3)
	{
		constexpr int delay_count = 30;

		static int delay = 0;
		static int attack_button_idx = 0;
		static int attack_button = IN_ATTACK;

		static const int attack_buttons[] =
		{
			IN_ATTACK,
			IN_JUMP,
			IN_DUCK,
			IN_USE,
			IN_CANCEL,
			IN_LEFT,
			IN_RIGHT,
			IN_ATTACK2,
			IN_RUN,
			IN_RELOAD
			// IN_ALT1
		};

		++delay;

		if (delay > delay_count)
		{
			if (attack_button_idx == (sizeof(attack_buttons) / sizeof(*attack_buttons)))
				attack_button_idx = 0;

			attack_button = attack_buttons[attack_button_idx++];

			delay = 0;
		}

		cmd->buttons |= attack_button;

		{
			constexpr int delay_count = 60;

			static int delay = 0;
			static int movement_button = IN_MOVERIGHT;

			WalkAround(cmd, delay, movement_button, delay_count);

			RotateCamera();
		}
	}
	else if (nMode == 4)
	{
		constexpr int delay_count = 60;

		static int delay = 0;
		static int movement_button = IN_MOVERIGHT;

		WalkAround(cmd, delay, movement_button, delay_count);

		RotateCamera();
	}
	else if (nMode == 5)
	{
		cmd->buttons |= IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT;
		cmd->sidemove = g_pPlayerMove->clientmaxspeed;

		RotateCamera();
	}
}

//-----------------------------------------------------------------------------

void CAntiAFK::Init()
{
	//antiafk = REGISTER_CVAR("sc_antiafk", "0", 0);
	//antiafk_rotation_angle = REGISTER_CVAR("sc_antiafk_rotation_angle", "-0.7", 0);
}