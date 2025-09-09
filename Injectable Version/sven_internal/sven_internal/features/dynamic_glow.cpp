// Dynamic Glow

#include "dynamic_glow.h"

#include "../sdk.h"
#include "../config.h"

#include "../interfaces.h"

#define DYNAMIC_LIGHT_LIFE_TIME 0.05f

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CDynamicGlow g_DynamicGlow;

//-----------------------------------------------------------------------------
// Dynamic Glow implementation
//-----------------------------------------------------------------------------

void CDynamicGlow::OnHUDRedraw() // glow local player
{
	if (g_Config.cvars.dyn_glow_self && g_pPlayerMove->iuser1 == 0)
	{
		float flRadius = g_Config.cvars.dyn_glow_self_radius;
		float flDecay = g_Config.cvars.dyn_glow_self_decay;
		float flDieTime = g_pEngineFuncs->GetClientTime() + DYNAMIC_LIGHT_LIFE_TIME;

		CreateDynamicLight(g_pPlayerMove->player_index + 1, g_pPlayerMove->origin, g_Config.cvars.dyn_glow_self_color, flRadius, flDecay, flDieTime);
	}
}

void CDynamicGlow::OnAddEntityPost(int is_visible, int type, struct cl_entity_s *ent, const char *modelname)
{
	if (is_visible && *modelname == 'm')
	{
		if (type == ET_PLAYER)
		{
			if (g_Config.cvars.dyn_glow_players && ent->index != g_pPlayerMove->player_index + 1)
			{
				float flRadius = g_Config.cvars.dyn_glow_players_radius;
				float flDecay = g_Config.cvars.dyn_glow_players_decay;
				float flDieTime = g_pEngineFuncs->GetClientTime() + DYNAMIC_LIGHT_LIFE_TIME;

				CreateDynamicLight(ent->index, ent->curstate.origin, g_Config.cvars.dyn_glow_players_color, flRadius, flDecay, flDieTime);
			}
		}
		else
		{
			const char *pszSlashLastOccur = strrchr(modelname, '/');

			if (pszSlashLastOccur)
				modelname = pszSlashLastOccur + 1;

			if (modelname[0] == 'w' && modelname[1] == '_') // an item
			{
				if (g_Config.cvars.dyn_glow_items)
				{
					float flRadius = g_Config.cvars.dyn_glow_items_radius;
					float flDecay = g_Config.cvars.dyn_glow_items_decay;
					float flDieTime = g_pEngineFuncs->GetClientTime() + DYNAMIC_LIGHT_LIFE_TIME;

					CreateDynamicLight(ent->index, ent->curstate.origin, g_Config.cvars.dyn_glow_items_color, flRadius, flDecay, flDieTime);
				}
			}
			else
			{
				if (g_Config.cvars.dyn_glow_entities)
				{
					if (ent->curstate.solid > SOLID_TRIGGER && ent->curstate.movetype != MOVETYPE_NONE)
					{
						float flRadius = g_Config.cvars.dyn_glow_entities_radius;
						float flDecay = g_Config.cvars.dyn_glow_entities_decay;
						float flDieTime = g_pEngineFuncs->GetClientTime() + DYNAMIC_LIGHT_LIFE_TIME;

						CreateDynamicLight(ent->index, ent->curstate.origin, g_Config.cvars.dyn_glow_entities_color, flRadius, flDecay, flDieTime);
					}
				}
			}
		}
	}
}

void CDynamicGlow::CreateDynamicLight(int entindex, float *vOrigin, float *pColor24, float flRadius, float flDecay, float flDieTime)
{
	dlight_t *pDynamicLight = g_pEngineFuncs->pEfxAPI->CL_AllocDlight(g_Config.cvars.dyn_glow_attach ? entindex : 0);

	pDynamicLight->color.r = int(255.f * pColor24[0]);
	pDynamicLight->color.g = int(255.f * pColor24[1]);
	pDynamicLight->color.b = int(255.f * pColor24[2]);

	pDynamicLight->origin = vOrigin;
	pDynamicLight->die = flDieTime;
	pDynamicLight->radius = flRadius;
	pDynamicLight->decay = flDecay;
}