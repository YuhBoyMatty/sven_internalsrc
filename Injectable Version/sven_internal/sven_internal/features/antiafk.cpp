// Anti-AFK

#include "antiafk.h"

#include "../sdk.h"
#include "../interfaces.h"

#include "../game/utils.h"
#include "../game/console.h"
#include "../game/mathlib.h"
#include "../game/player_utils.h"

#include "../config.h"

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CAntiAFK g_AntiAFK;

//-----------------------------------------------------------------------------
// ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_antiafk, ConCommand_AntiAFK, "sc_antiafk [mode] - Set Anti-AFK Mode [0-5]")
{
	if (CMD_ARGC() >= 2)
	{
		int nMode = atoi(CMD_ARGV(1));

		if (nMode >= 0 && nMode <= 5)
		{
			g_Config.cvars.antiafk = nMode;
		}
		else
		{
			Msg("Wrong Anti-AFK mode\n");
			Msg("Available modes:\n0 - Off\n1 - Step Forward & Back\n2 - Spam Gibme\n3 - Walk Around & Spam Inputs\n4 - Walk Around\n5 - Go Right\n");
		}
	}
	else
	{
		sc_antiafk.PrintUsage();
	}
}

//-----------------------------------------------------------------------------
// Anti-AFK implementations
//-----------------------------------------------------------------------------

CAntiAFK::CAntiAFK()
{
	m_bDead = true;
	m_bComingBackToAFKPoint = false;

	m_vecAFKPoint.x = 0.f;
	m_vecAFKPoint.y = 0.f;

	m_flComingBackStartTime = -1.f;
}

void CAntiAFK::Reset()
{
	m_bDead = true;
	m_bComingBackToAFKPoint = false;

	m_vecAFKPoint.x = 0.f;
	m_vecAFKPoint.y = 0.f;

	m_flComingBackStartTime = -1.f;
}

void CAntiAFK::OnHUDInit()
{
	Reset();
}

void CAntiAFK::OnVideoInit()
{
	Reset();
}

void CAntiAFK::CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	AntiAFK(cmd);
}

void CAntiAFK::OnRevive()
{
	if (!g_pPlayerMove)
	{
		m_bDead = true;
		return;
	}

	m_vecAFKPoint.x = g_pPlayerMove->origin.x;
	m_vecAFKPoint.y = g_pPlayerMove->origin.y;

	m_bDead = false;
	m_bComingBackToAFKPoint = false;

	m_flComingBackStartTime = -1.f;
}

void CAntiAFK::OnDie()
{
	m_bDead = true;
	m_bComingBackToAFKPoint = false;

	m_vecAFKPoint.x = 0.f;
	m_vecAFKPoint.y = 0.f;

	m_flComingBackStartTime = -1.f;
}

void CAntiAFK::AntiAFK(struct usercmd_s *cmd)
{
	int nMode = g_Config.cvars.antiafk;

	if (!nMode)
	{
		m_flComingBackStartTime = -1.0f;
		m_bComingBackToAFKPoint = false;
		m_bDead = true;
		return;
	}

	bool bDead = !IsLocalPlayerAlive();
	
	if (bDead != m_bDead)
	{
		if (bDead && !m_bDead)
			OnDie();
		else
			OnRevive();
	}

	if (bDead)
		return;

	if (g_Config.cvars.antiafk_stay_within_range)
	{
		if (m_vecAFKPoint.x == 0.0f && m_vecAFKPoint.y == 0.0f)
			m_vecAFKPoint = g_pPlayerMove->origin.Make2D();

		Vector2D vecOrigin = g_pPlayerMove->origin.Make2D();
		float flDistanceToAFKPointSqr = (m_vecAFKPoint - vecOrigin).LengthSqr();

		if (m_bComingBackToAFKPoint || flDistanceToAFKPointSqr > M_SQR(g_Config.cvars.antiafk_stay_radius)) // moved out of range
		{
			bool bReset = false;

			if (m_flComingBackStartTime == -1.0f)
			{
				m_flComingBackStartTime = g_pEngineFuncs->GetClientTime();
			}
			else if (g_Config.cvars.antiafk_reset_stay_pos && g_pEngineFuncs->GetClientTime() - m_flComingBackStartTime >= 10.f)
			{
				// Coming to the AFK point too long, reset current state
				Reset();
				bReset = true;
			}
			
			if ( !bReset )
			{
				if (m_bComingBackToAFKPoint && flDistanceToAFKPointSqr <= M_SQR(25.0f))
				{
					m_bComingBackToAFKPoint = false;
					m_flComingBackStartTime = -1.0f;
					return;
				}

				Vector2D vecForward;
				Vector2D vecRight;

				Vector2D vecDir = (m_vecAFKPoint - vecOrigin).Normalize();

				m_bComingBackToAFKPoint = true;

				// Rotate the wish vector by a random direction, must help if we stuck somewhere
				int nRandom = g_pEngineFuncs->pfnRandomLong(0, 1);

				float flTheta = g_Config.cvars.antiafk_stay_radius_offset_angle * static_cast<float>(M_PI) / 180.0f;

				float ct = cosf(flTheta);
				float st = sinf(flTheta);

				if (nRandom % 2) // counter clockwise
				{
					vecDir.x = vecDir.x * ct - vecDir.y * st;
					vecDir.y = vecDir.x * st + vecDir.y * ct;
				}
				else // clockwise
				{
					vecDir.x = vecDir.x * ct + vecDir.y * st;
					vecDir.y = -vecDir.x * st + vecDir.y * ct;
				}

				// Forward angles
				vecForward.x = cosf(cmd->viewangles.y * static_cast<float>(M_PI) / 180.f);
				vecForward.y = sinf(cmd->viewangles.y * static_cast<float>(M_PI) / 180.f);

				// Make a right vector of angles. Rotate forward vector as a complex number by 90 deg.
				vecRight.x = vecForward.y;
				vecRight.y = -vecForward.x;

				// Multiply by max movement speed
				vecForward = vecForward * g_pPlayerMove->clientmaxspeed;
				vecRight = vecRight * g_pPlayerMove->clientmaxspeed;

				// Project onto direction vector
				float forwardmove = DotProduct(vecForward, vecDir);
				float sidemove = DotProduct(vecRight, vecDir);

				// Apply moves
				cmd->forwardmove = forwardmove;
				cmd->sidemove = sidemove;

				return;
			}
		}
	}
	else
	{
		m_bComingBackToAFKPoint = false;
		m_flComingBackStartTime = -1.0f;
	}

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

	if (g_pPlayerMove->waterlevel == 3)
	{
		cmd->upmove = g_pPlayerMove->clientmaxspeed;
	}
}

void CAntiAFK::RotateCamera()
{
	if (g_Config.cvars.antiafk_rotate_camera)
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
		else if (s_flPitchDirection < 0.0f)
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
}

void CAntiAFK::WalkAround(struct usercmd_s *cmd, int &delay, int &movement_button, const int delay_count)
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

//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------

void CAntiAFK::Init()
{
	//antiafk = REGISTER_CVAR("sc_antiafk", "0", 0);
	//antiafk_rotation_angle = REGISTER_CVAR("sc_antiafk_rotation_angle", "-0.7", 0);
}