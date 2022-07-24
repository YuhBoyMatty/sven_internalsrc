// Aim Feature

#include <ICvar.h>
#include <IClient.h>
#include <IClientWeapon.h>
#include <IMemoryUtils.h>
#include <ISvenModAPI.h>
#include <convar.h>
#include <dbg.h>

#include <math/mathlib.h>
#include <hl_sdk/engine/APIProxy.h>

#include "aim.h"

#include "../game/entitylist.h"
#include "../game/utils.h"

#include "../patterns.h"
#include "../config.h"

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CAim g_Aim;

Vector *ev_punchangle = NULL;

//-----------------------------------------------------------------------------
// ConCommands/CVars
//-----------------------------------------------------------------------------

//CON_COMMAND(sc_no_recoil, "Compensates recoil")
//{
//	Msg(g_Config.cvars.no_recoil ? "No Recoil disabled\n" : "No Recoil enabled\n");
//	g_Config.cvars.no_recoil = !g_Config.cvars.no_recoil;
//}
//
//CON_COMMAND(sc_no_recoil_visual, "Removes visual effect of recoil")
//{
//	Msg(g_Config.cvars.no_recoil_visual ? "No Recoil Visual disabled\n" : "No Recoil Visual enabled\n");
//	g_Config.cvars.no_recoil_visual = !g_Config.cvars.no_recoil_visual;
//}

//-----------------------------------------------------------------------------
// CAim implementations
//-----------------------------------------------------------------------------

CAim::CAim()
{
	m_pfnV_PunchAxis = NULL;
}

void CAim::CreateMove(float frametime, usercmd_t *cmd, int active)
{
	Aimbot(cmd);
	NoRecoil(cmd);
}

void CAim::Aimbot(usercmd_t *cmd)
{
	int iWeaponID, iClip;
	WEAPON *pWeapon;

	if ( !g_Config.cvars.aimbot && !g_Config.cvars.silent_aimbot && !g_Config.cvars.ragebot )
		return;

	// Don't have any weapon
	if ( ( iWeaponID = Client()->GetCurrentWeaponID() ) == WEAPON_NONE )
		return;

	// We're dead
	if ( Client()->IsDead() )
		return;

	// Take out a weapon rn + exception for displacer
	if ( !Client()->CanAttack() && iWeaponID != WEAPON_DISPLACER )
		return;

	// Don't use crowbar, medkit etc..
	if ( !IsHoldingAppropriateWeapon(iWeaponID) )
		return;

	// Whoops.. what happened with the weapon?
	if ( ( pWeapon = Inventory()->GetWeapon(iWeaponID) ) == NULL )
		return;

	// We're empty
	if ( !Inventory()->HasAmmo(pWeapon) )
		return;
	
	iClip = ClientWeapon()->Clip();

	// Going to reload
	if ( CheckReload(iWeaponID, iClip, cmd) )
		return;

	if ( g_Config.cvars.ragebot )
	{
		bool bStillFiring = IsStillFiring(iWeaponID, cmd);
		bool bCanPrimaryAttack = ClientWeapon()->CanPrimaryAttack();
		bool bCanSecondaryAttack = ClientWeapon()->CanSecondaryAttack();

		if ( bStillFiring || bCanPrimaryAttack || bCanSecondaryAttack )
		{
			CEntity *pTarget = FindBestTarget();

			if (pTarget != NULL)
			{
				// Yes, no hitboxes.. Will be fixed later
				Vector vecTargetPoint = m_vecTargetPoint + pTarget->m_vecVelocity * pTarget->m_frametime;
				Vector vecDir = vecTargetPoint - ( Client()->GetOrigin() + Client()->GetViewOffset() );

				if (bStillFiring)
				{
					Vector vAngles;
					DirectionToAngles(vecDir, vAngles);

					UTIL_SetAnglesSilent(vAngles, cmd);
				}
				else
				{
					float flDistance = vecDir.Length();

					if ( IsDistanceAllowsUseWeapon(iWeaponID, flDistance) )
					{
						int fAttackButton = ConcludeAttackButton( iWeaponID, iClip, flDistance );

						if ( ( fAttackButton == IN_ATTACK && bCanPrimaryAttack ) || ( fAttackButton == IN_ATTACK2 && bCanSecondaryAttack ) )
						{
							Vector vAngles;
							DirectionToAngles(vecDir, vAngles);

							UTIL_SetAnglesSilent(vAngles, cmd);
							cmd->buttons |= fAttackButton;
						}
					}
				}
			}
		}
	}
	else if ( g_Config.cvars.aimbot || g_Config.cvars.silent_aimbot )
	{
		if ( IsStillFiring(iWeaponID, cmd) || IsFiring(iWeaponID, cmd) )
		{
			CEntity *pTarget = FindBestTarget();

			if (pTarget != NULL)
			{
				// Yes, no hitboxes.. Will be fixed later
				Vector vecTargetPoint = m_vecTargetPoint + pTarget->m_vecVelocity * pTarget->m_frametime;
				Vector vecDir = vecTargetPoint - ( Client()->GetOrigin() + Client()->GetViewOffset() );

				float flDistance = vecDir.Length();

				if ( IsDistanceAllowsUseWeapon(iWeaponID, flDistance) )
				{
					Vector vAngles;
					DirectionToAngles(vecDir, vAngles);

					if (g_Config.cvars.silent_aimbot)
					{
						UTIL_SetAnglesSilent(vAngles, cmd);
					}
					else
					{
						cmd->viewangles = vAngles;
						g_pEngineFuncs->SetViewAngles(vAngles);
					}
				}
			}
		}
	}
}

bool CAim::CheckReload(int iWeaponID, int iClip, usercmd_t *cmd)
{
	if ( ClientWeapon()->IsReloading() )
		return true;

	if (iClip == 0)
	{
		if ( iWeaponID == WEAPON_RPG )
		{
			// Can't reload while using laser homing
			if ( ClientWeapon()->GetWeaponData()->iuser4 && ClientWeapon()->GetWeaponData()->fuser1 != 0.f )
			{
				return false;
			}
		}

		cmd->buttons |= IN_RELOAD;
		return true;
	}

	return false;
}

bool CAim::IsHoldingAppropriateWeapon(int iWeaponID)
{
	switch (iWeaponID)
	{
	case WEAPON_CROWBAR:
	case WEAPON_WRENCH:
	case WEAPON_MEDKIT:
	case WEAPON_HANDGRENADE:
	case WEAPON_TRIPMINE:
	case WEAPON_SATCHEL:
	case WEAPON_BARNACLE_GRAPPLE:
		return false;
	}

	return true;
}

bool CAim::IsDistanceAllowsUseWeapon(int iWeaponID, float flDistance)
{
	switch (iWeaponID)
	{
	case WEAPON_RPG:
		if ( flDistance <= 340.f )
			return false;

		return true;

	case WEAPON_CROSSBOW:
		if ( flDistance <= 128.f )
			return false;

		return true;
		
	case WEAPON_EGON:
		if ( flDistance <= 128.f && flDistance > 2048.f )
			return false;

		return true;

	case WEAPON_DISPLACER:
		if ( flDistance <= 350.f )
			return false;

		return true;
		
	case WEAPON_SPORE_LAUNCHER:
		if ( flDistance <= 500.f && flDistance > 800.f )
			return false;

		return true;

	case WEAPON_SHOTGUN:
	case WEAPON_SNARK:
		if ( flDistance > 500.f )
			return false;

		return true;
	}

	return true;
}

int CAim::ConcludeAttackButton(int iWeaponID, int iClip, float flDistance)
{
	switch (iWeaponID)
	{
	case WEAPON_SHOTGUN:
		if ( flDistance <= 256.f && iClip > 1 )
			return IN_ATTACK2;

		return IN_ATTACK;

	case WEAPON_HORNETGUN:
		return IN_ATTACK2;
	}

	return IN_ATTACK;
}

bool CAim::IsFiring(int iWeaponID, usercmd_t *cmd)
{
	switch ( iWeaponID )
	{
	case WEAPON_DESERT_EAGLE:
	case WEAPON_MP5:
	case WEAPON_M16:
	case WEAPON_CROSSBOW:
	case WEAPON_RPG:
	case WEAPON_EGON:
	case WEAPON_SNIPER_RIFLE:
	case WEAPON_M249:
	case WEAPON_DISPLACER:
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
		else if ( cmd->buttons & IN_ATTACK2 )
		{
			return false;
		}

		break;
	}

	if ( cmd->buttons & (IN_ATTACK | IN_ATTACK2) )
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

	return false;
}

bool CAim::IsStillFiring(int iWeaponID, usercmd_t *cmd)
{
	// To hit a target, we still need to aim after firing from a weapon

	switch (iWeaponID)
	{
	case WEAPON_M16:
		if ( ClientWeapon()->GetWeaponData()->fuser2 != 0.f )
			return true;

		break;

	case WEAPON_RPG:
		if ( ClientWeapon()->GetWeaponData()->iuser4 && ClientWeapon()->GetWeaponData()->fuser1 != 0.f )
			return true;

		break;

	case WEAPON_DISPLACER:
		if ( ClientWeapon()->GetWeaponData()->fuser1 == 1.f )
			return true;

		break;
	}

	return false;
}

CEntity *CAim::FindBestTarget()
{
	Vector va;
	Vector vForward;

	float flDistanceSqr = FLT_MAX;
	float flMaxDistanceSqr = M_SQR( g_Config.cvars.aimbot_distance );

	CEntity *pTarget = NULL;
	CEntity *pEnts = g_EntityList.GetList();

	cl_entity_t *pLocal = g_pEngineFuncs->GetLocalPlayer();
	Vector vecEyes = Client()->GetOrigin() + Client()->GetViewOffset();

	if ( g_Config.cvars.aimbot_consider_fov )
	{
		g_pEngineFuncs->GetViewAngles(va);
		AngleVectors(va, &vForward, NULL, NULL);
	}

	for (register int i = 1; i <= g_EntityList.GetMaxEntities(); i++)
	{
		CEntity &ent = pEnts[i];

		if ( !ent.m_bValid )
			continue;

		if ( !ent.m_bEnemy )
			continue;

		if ( ent.m_bItem )
			continue;

		if ( !ent.m_bAlive )
			continue;

		if ( ent.m_classInfo.id == CLASS_NPC_SNARK )
			continue;

		// Ignore not active sentry or turret
		if ( ( ent.m_pEntity->curstate.sequence == 0 || ent.m_pEntity->curstate.sequence == 5 ) &&
			(ent.m_classInfo.id == CLASS_NPC_SENTRY || ent.m_classInfo.id == CLASS_NPC_TURRET) )
			continue;

		float dist_sqr = (pLocal->curstate.origin - ent.m_pEntity->curstate.origin).LengthSqr();

		if ( dist_sqr < flDistanceSqr )
		{
			Vector vecMins = ent.m_vecOrigin + ent.m_vecMins;
			Vector vecMaxs = ent.m_vecOrigin + ent.m_vecMaxs;

			Vector vecTargetPoint = (vecMins + (vecMaxs - vecMins) * 0.6);

			if ( g_Config.cvars.aimbot_consider_fov )
			{
				float angle = acos( vForward.Dot( (vecTargetPoint - vecEyes).Normalize() ) ) * 180.f / static_cast<float>(M_PI);

				if ( angle > g_Config.cvars.aimbot_fov )
					continue;
			}

			if ( IsCanSeeTarget(&ent, vecEyes, vecTargetPoint) )
			{
				pTarget = &ent;
				flDistanceSqr = dist_sqr;
				m_vecTargetPoint = vecTargetPoint;
			}
		}
	}

	return pTarget;
}

bool CAim::IsCanSeeTarget(CEntity *pEntity, Vector &vecEyes, Vector &vecPoint)
{
	pmtrace_t trace;

	g_pEventAPI->EV_SetTraceHull(PM_HULL_POINT);
	g_pEventAPI->EV_PlayerTrace(vecEyes, vecPoint, PM_NORMAL, -1, &trace);

	return ( g_pEventAPI->EV_IndexFromTrace(&trace) == pEntity->m_pEntity->index );
	//return ( trace.fraction == 1.f );
}

void CAim::DirectionToAngles(Vector &vecDir, Vector &vecAngles)
{
	vecAngles.x = -atan2f(vecDir.z, vecDir.Length2D()) * 180.f / static_cast<float>(M_PI);
	vecAngles.y = atan2f(vecDir.y, vecDir.x) * 180.f / static_cast<float>(M_PI);
	vecAngles.z = 0.f;
}

void CAim::NoRecoil(usercmd_t *cmd)
{
	if ( g_Config.cvars.no_recoil && !Client()->IsDead() && Client()->HasWeapon() && Client()->CanAttack() && !ClientWeapon()->IsReloading() )
	{
		// Can be wrong..

		if ( cmd->buttons & IN_ATTACK )
		{
			if ( ClientWeapon()->IsCustom() || (!ClientWeapon()->IsCustom() && ClientWeapon()->CanPrimaryAttack()) )
			{
				Vector vecNoRecoil = (m_vecPunchAngle + m_vecEVPunchAngle) * 2;
				vecNoRecoil.z = 0.f;

				cmd->viewangles -= vecNoRecoil;
			}
		}
		else if (cmd->buttons & IN_ATTACK2)
		{
			if ( ClientWeapon()->IsCustom() || (!ClientWeapon()->IsCustom() && ClientWeapon()->CanSecondaryAttack()) )
			{
				Vector vecNoRecoil = (m_vecPunchAngle + m_vecEVPunchAngle) * 2;
				vecNoRecoil.z = 0.f;

				cmd->viewangles -= vecNoRecoil;
			}
		}
	}
}

void CAim::Pre_V_CalcRefdef(ref_params_t *pparams)
{
	m_vecPunchAngle = *reinterpret_cast<Vector *>(pparams->punchangle);
	m_vecEVPunchAngle = *ev_punchangle;
}

void CAim::Post_V_CalcRefdef(ref_params_t *pparams)
{
	if ( g_Config.cvars.no_recoil_visual )
	{
		*reinterpret_cast<Vector *>(pparams->viewangles) -= m_vecPunchAngle + m_vecEVPunchAngle;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CAim::Load()
{
	m_pfnV_PunchAxis = MemoryUtils()->FindPattern( SvenModAPI()->Modules()->Client, Patterns::Client::V_PunchAxis );

	if ( !m_pfnV_PunchAxis )
	{
		Warning("Couldn't find function \"V_PunchAxis\"\n");
		return false;
	}

	return true;
}

void CAim::PostLoad()
{
	ud_t inst;
	MemoryUtils()->InitDisasm(&inst, m_pfnV_PunchAxis, 32, 64);

	do
	{
		if (inst.mnemonic == UD_Imovss && inst.operand[0].type == UD_OP_MEM && inst.operand[0].index == UD_R_EAX &&
			inst.operand[0].scale == 4 && inst.operand[0].offset == 32 && inst.operand[1].type == UD_OP_REG && inst.operand[1].base == UD_R_XMM0)
		{
			ev_punchangle = reinterpret_cast<Vector *>(inst.operand[0].lval.udword);
			break;
		}

	} while ( MemoryUtils()->Disassemble(&inst) );
}

void CAim::Unload()
{

}