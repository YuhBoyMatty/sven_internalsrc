// Vectorial Strafer

#include "strafer.h"

#include "../sdk.h"

#include "../game/utils.h"
#include "../game/console.h"
#include "../strafe/strafe.h"

#include "../config.h"

//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;

//-----------------------------------------------------------------------------

CStrafer g_Strafer;

Strafe::StrafeData g_strafeData;

cvar_s *sv_friction = NULL;
cvar_s *sv_accelerate = NULL;
cvar_s *sv_airaccelerate = NULL;
cvar_s *sv_stopspeed = NULL;

//-----------------------------------------------------------------------------

void UpdateStrafeData(float flYaw)
{
	*reinterpret_cast<Vector *>(g_strafeData.player.Velocity) = g_pPlayerMove->velocity;
	*reinterpret_cast<Vector *>(g_strafeData.player.Origin) = g_pPlayerMove->origin;

	g_strafeData.vars.OnGround = g_pPlayerMove->onground != -1;
	g_strafeData.vars.EntFriction = g_pPlayerMove->friction;
	g_strafeData.vars.ReduceWishspeed = g_strafeData.vars.OnGround && (g_pPlayerMove->flags & FL_DUCKING);
	g_strafeData.vars.Maxspeed = g_pPlayerMove->clientmaxspeed;
	g_strafeData.vars.Stopspeed = sv_stopspeed->value;
	g_strafeData.vars.Friction = sv_friction->value;
	g_strafeData.vars.Accelerate = sv_accelerate->value;
	g_strafeData.vars.Airaccelerate = sv_airaccelerate->value;
	g_strafeData.vars.Frametime = g_pPlayerMove->frametime; // 1.0f / 200.0f (1.0f / fps_max)

	g_strafeData.frame.Strafe = static_cast<bool>(g_Config.cvars.strafe);
	g_strafeData.frame.SetDir(static_cast<Strafe::StrafeDir>(static_cast<int>(g_Config.cvars.strafe_dir)));
	g_strafeData.frame.SetType(static_cast<Strafe::StrafeType>(static_cast<int>(g_Config.cvars.strafe_type)));

	//if (!*strafe_yaw->string)
		g_strafeData.frame.SetYaw(static_cast<double>(flYaw));
	//else
	//	g_strafeData.frame.SetYaw(static_cast<double>(strafe_yaw->value));
}

//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_strafe, ConCommand_VectorialStrafe, "sc_strafe - Toggle Vectorial Strafing")
{
	g_pEngineFuncs->Con_Printf(g_Config.cvars.strafe ? "Vectorial Strafer disabled\n" : "Vectorial Strafer enabled\n");
	g_Config.cvars.strafe = !g_Config.cvars.strafe;
}

//-----------------------------------------------------------------------------

void CStrafer::CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	StrafeVectorial(cmd);
}

void CStrafer::StrafeVectorial(struct usercmd_s *pCmd)
{
	if (g_Config.cvars.strafe_ignore_ground && g_pPlayerMove->onground != -1 || pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
		return;

	if (g_Config.cvars.antiafk || g_pPlayerMove->dead || g_pPlayerMove->movetype != MOVETYPE_WALK)
		return;

	Vector viewangles;
	g_pEngineFuncs->GetViewAngles(viewangles);

	UpdateStrafeData(viewangles[1]);

	if (g_strafeData.frame.Strafe)
	{
		Strafe::ProcessedFrame out;
		out.Yaw = viewangles[1];

		Strafe::Friction(g_strafeData);

		Strafe::StrafeVectorial(g_strafeData, out, false);

		if (out.Processed)
		{
			pCmd->forwardmove = out.Forwardspeed;
			pCmd->sidemove = out.Sidespeed;

			viewangles[1] = static_cast<float>(out.Yaw);
		}
	}

	g_pEngineFuncs->SetViewAngles(viewangles);
}

//-----------------------------------------------------------------------------

void CStrafer::Init()
{
	sv_friction = g_pEngineFuncs->pfnGetCvarPointer("sv_friction");
	sv_accelerate = g_pEngineFuncs->pfnGetCvarPointer("sv_accelerate");
	sv_airaccelerate = g_pEngineFuncs->pfnGetCvarPointer("sv_airaccelerate");
	sv_stopspeed = g_pEngineFuncs->pfnGetCvarPointer("sv_stopspeed");

	g_strafeData.frame.UseGivenButtons = true;
	g_strafeData.frame.buttons = Strafe::StrafeButtons();
	g_strafeData.frame.buttons.AirLeft = Strafe::Button::LEFT;
	g_strafeData.frame.buttons.AirRight = Strafe::Button::RIGHT;
}