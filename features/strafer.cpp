// Vectorial Strafer

#include <ICvar.h>
#include <convar.h>
#include <dbg.h>

#include <hl_sdk/engine/APIProxy.h>

#include "strafer.h"

#include "../game/utils.h"
#include "../strafe/strafe.h"

#include "../config.h"

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CStrafer g_Strafer;

Strafe::StrafeData g_strafeData;

cvar_s *sv_friction = NULL;
cvar_s *sv_accelerate = NULL;
cvar_s *sv_airaccelerate = NULL;
cvar_s *sv_stopspeed = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static void UpdateStrafeData(float flYaw)
{
	*reinterpret_cast<Vector *>(g_strafeData.player.Velocity) = g_pPlayerMove->velocity;
	*reinterpret_cast<Vector *>(g_strafeData.player.Origin) = g_pPlayerMove->origin;

	g_strafeData.vars.OnGround = g_pPlayerMove->onground != -1;
	g_strafeData.vars.EntFriction = g_pPlayerMove->friction;
	g_strafeData.vars.ReduceWishspeed = g_strafeData.vars.OnGround && (g_pPlayerMove->flags & FL_DUCKING);
	g_strafeData.vars.Maxspeed = g_pPlayerMove->maxspeed;
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
// ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND_NO_WRAPPER(sc_strafe, "Toggle Vectorial Strafing")
{
	Msg(g_Config.cvars.strafe ? "Vectorial Strafer disabled\n" : "Vectorial Strafer enabled\n");
	g_Config.cvars.strafe = !g_Config.cvars.strafe;
}

CON_COMMAND_NO_WRAPPER(sc_strafe_ignore_ground, "Ignore ground when strafe")
{
	Msg(g_Config.cvars.strafe_ignore_ground ? "Enabled strafer when on ground\n" : "Disabled strafer when on ground\n");
	g_Config.cvars.strafe_ignore_ground = !g_Config.cvars.strafe_ignore_ground;
}

CON_COMMAND(sc_strafe_dir, "Set strafing direction. Directions:\n\t0 - to the left\n\t1 - to the right\n\t2 - best strafe\n\t3 - to view angles")
{
	if (args.ArgC() > 1)
	{
		int dir = atoi(args[1]);

		if (dir >= 0 && dir <= 3)
		{
			g_Config.cvars.strafe_dir = dir;
		}
	}
	else
	{
		ConMsg("Usage:  sc_strafe_dir <direction>\n");
	}
}

CON_COMMAND(sc_strafe_type, "Set strafing type. Types:\n\t0 - Max acceleration strafing\n\t1 - Max angle strafing\n\t2 - Max deceleration strafing\n\t3 - Const speed strafing")
{
	if (args.ArgC() > 1)
	{
		int type = atoi(args[1]);

		if (type >= 0 && type <= 3)
		{
			g_Config.cvars.strafe_type = type;
		}
	}
	else
	{
		ConMsg("Usage:  sc_strafe_type <type>\n");
	}
}

//-----------------------------------------------------------------------------
// CStrafer implementations
//-----------------------------------------------------------------------------

void CStrafer::CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	StrafeVectorial(cmd);
}

void CStrafer::StrafeVectorial(struct usercmd_s *pCmd)
{
	if (g_Config.cvars.strafe_ignore_ground && g_pPlayerMove->onground != -1 || pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
		return;

	if (g_Config.cvars.antiafk || g_pPlayerMove->dead || g_pPlayerMove->iuser1 != 0 || g_pPlayerMove->movetype != MOVETYPE_WALK || g_pPlayerMove->waterlevel > WL_FEET)
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
//-----------------------------------------------------------------------------

bool CStrafer::Load()
{
	sv_friction = CVar()->FindCvar("sv_friction");
	sv_accelerate = CVar()->FindCvar("sv_accelerate");
	sv_airaccelerate = CVar()->FindCvar("sv_airaccelerate");
	sv_stopspeed = CVar()->FindCvar("sv_stopspeed");

	if ( !sv_friction )
	{
		Warning("Can't find cvar \"sv_friction\"\n");
		return false;
	}
	
	if ( !sv_accelerate )
	{
		Warning("Can't find cvar \"sv_accelerate\"\n");
		return false;
	}
	
	if ( !sv_airaccelerate )
	{
		Warning("Can't find cvar \"sv_airaccelerate\"\n");
		return false;
	}
	
	if ( !sv_stopspeed )
	{
		Warning("Can't find cvar \"sv_stopspeed\"\n");
		return false;
	}

	g_strafeData.frame.UseGivenButtons = true;
	g_strafeData.frame.buttons = Strafe::StrafeButtons();
	g_strafeData.frame.buttons.AirLeft = Strafe::Button::LEFT;
	g_strafeData.frame.buttons.AirRight = Strafe::Button::RIGHT;

	return true;
}

void CStrafer::PostLoad()
{

}

void CStrafer::Unload()
{

}