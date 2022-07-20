// Aim Feature

#include <ICvar.h>
#include <IClient.h>
#include <IClientWeapon.h>
#include <IMemoryUtils.h>
#include <ISvenModAPI.h>
#include <convar.h>
#include <dbg.h>

#include <hl_sdk/engine/APIProxy.h>

#include "aim.h"

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
	NoRecoil(cmd);
}

void CAim::NoRecoil(usercmd_t *cmd)
{
	if ( g_Config.cvars.no_recoil && !Client()->IsDead() && Client()->HasWeapon() && Client()->CanAttack() )
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