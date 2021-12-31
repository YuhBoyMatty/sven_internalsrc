// First-Person Roaming

#include "firstperson_roaming.h"

#include "../interfaces.h"
#include "../config.h"

#include "../game/utils.h"
#include "../game/mathlib.h"

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;

//-----------------------------------------------------------------------------
// Global Vars
//-----------------------------------------------------------------------------

CFirstPersonRoaming g_FirstPersonRoaming;

//-----------------------------------------------------------------------------
// Implementations
//-----------------------------------------------------------------------------

CFirstPersonRoaming::CFirstPersonRoaming() : m_iTarget(-1), m_iSpectatorMode(0), m_pTarget(NULL)
{
	m_vPrevAngles.x = m_vPrevAngles.y = m_vPrevAngles.z = 0.f;
}

cl_entity_s *CFirstPersonRoaming::GetTargetPlayer()
{
	return m_pTarget;
}

void CFirstPersonRoaming::V_CalcRefdef(struct ref_params_s *pparams)
{
	int iSpectatorMode = g_pPlayerMove->iuser1;
	int iTarget = g_pPlayerMove->iuser2;

	if (g_Config.cvars.fp_roaming && iSpectatorMode == 3 && iTarget >= 1) // OBS_ROAMING = 3
	{
		m_pTarget = g_pEngineFuncs->GetEntityByIndex(iTarget); // target player

		if (!m_pTarget || !m_pTarget->model) // player is invalid
		{
			m_iSpectatorMode = 3;
			m_iTarget = -1;
			m_pTarget = NULL;
			return;
		}

		// spectator mode or target was changed
		if (m_iSpectatorMode != 3 || iTarget != m_iTarget)
			GetPlayerViewAngles(m_vPrevAngles);

		m_iSpectatorMode = 3;
		m_iTarget = iTarget;
		
		// Override view
		*reinterpret_cast<Vector *>(pparams->vieworg) = m_pTarget->origin + Vector(0.f, 0.f, m_pTarget->curstate.usehull ? 12.5f : 28.5f /* VEC_DUCK_VIEW.z : VEC_VIEW.z */);
		*reinterpret_cast<Vector *>(pparams->viewangles) = m_vPrevAngles;

		// Lerp angles
		if (g_Config.cvars.fp_roaming_lerp)
		{
			Vector from, to;
			from = m_vPrevAngles;

			GetPlayerViewAngles(to);
			LerpAngles(from, to, g_Config.cvars.fp_roaming_lerp_value, m_vPrevAngles);
		}
		else
		{
			GetPlayerViewAngles(m_vPrevAngles);
		}
	}
	else
	{	
		m_iSpectatorMode = iSpectatorMode;
		m_iTarget = -1;
		m_pTarget = NULL;
	}
}

void CFirstPersonRoaming::GetPlayerViewAngles(Vector &vOutput)
{
	vOutput = m_pTarget->curstate.angles;

	// Transform this weird Pitch range [-9.887 ... 9.887] to [-89.0 ... 89.0]
	vOutput.x *= 10.0f * (89.0f / 98.876953f);
}