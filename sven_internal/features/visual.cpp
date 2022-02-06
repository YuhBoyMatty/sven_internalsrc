// Visual

#pragma comment(lib, "OpenGL32.lib")

#include <Windows.h>
#include <gl/GL.h>

#include <algorithm>

#include "visual.h"

#include "../sdk.h"
#include "../interfaces.h"

#include "../modules/client.h"
#include "../modules/vgui.h"

#include "../game/utils.h"
#include "../game/drawing.h"
#include "../game/class_table.h"
#include "../game/usermsg.h"
#include "../game/console.h"
#include "../game/mathlib.h"
#include "../game/player_utils.h"

#include "../features/firstperson_roaming.h"

#include "../utils/vtable_hook.h"

#include "../config.h"

#define MAXENTS 8192
#define MAXCLIENTS 32
//#define PROCESS_PLAYER_BONES_ONLY

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern vgui::IPanel *g_pPanel;
extern vgui::ISurface *g_pSurface;

extern playermove_s *g_pPlayerMove;
extern extra_player_info_t *g_pPlayerExtraInfo;

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

typedef float bone_matrix3x4_t[MAXSTUDIOBONES][3][4];

struct bone_s
{
	Vector vecPoint[MAXSTUDIOBONES];
	int nParent[MAXSTUDIOBONES] = { -1 };
};

#ifdef PROCESS_PLAYER_BONES_ONLY
Hitbox g_Bones[MAXCLIENTS + 1];
#else
bone_s g_Bones[MAXENTS + 1];
#endif

//-----------------------------------------------------------------------------
// Global Vars
//-----------------------------------------------------------------------------

CVisual g_Visual;
bone_matrix3x4_t *g_pBoneTransform = NULL;

cvar_s *r_drawentities = NULL;

pfnUserMsgHook UserMsgHook_ScreenShake_Original = NULL;
pfnUserMsgHook UserMsgHook_ScreenFade_Original = NULL;

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_wallhack, ConCommand_Wallhack, "sc_wallhack - Simple OpenGL wallhack")
{
	Msg(g_Config.cvars.wallhack ? "Wallhack disabled\n" : "Wallhack enabled\n");
	g_Config.cvars.wallhack = !g_Config.cvars.wallhack;
}

CON_COMMAND_FUNC(sc_wallhack_white_walls, ConCommand_WhiteWalls, "sc_wallhack_white_walls - Lambert wallhack")
{
	Msg(g_Config.cvars.wallhack_white_walls ? "White Walls disabled\n" : "White Walls enabled\n");
	g_Config.cvars.wallhack_white_walls = !g_Config.cvars.wallhack_white_walls;
}

CON_COMMAND_FUNC(sc_wallhack_wireframe, ConCommand_Wireframe, "sc_wallhack_wireframe - Wireframe view")
{
	Msg(g_Config.cvars.wallhack_wireframe ? "Wireframe disabled\n" : "Wireframe enabled\n");
	g_Config.cvars.wallhack_wireframe = !g_Config.cvars.wallhack_wireframe;
}

CON_COMMAND_FUNC(sc_wallhack_wireframe_models, ConCommand_WireframeModels, "sc_wallhack_wireframe_models - Wireframe view (entity models only)")
{
	Msg(g_Config.cvars.wallhack_wireframe_models ? "Wireframe Models disabled\n" : "Wireframe Models enabled\n");
	g_Config.cvars.wallhack_wireframe_models = !g_Config.cvars.wallhack_wireframe_models;
}

//-----------------------------------------------------------------------------
// Visual Hack
//-----------------------------------------------------------------------------

void CVisual::Process()
{
	m_iScreenWidth = g_ScreenInfo.width;
	m_iScreenHeight = g_ScreenInfo.height;

	Lightmap();

	ESP();

	DrawCrosshair();
	ShowSpeed();
}

void CVisual::ShowSpeed()
{
	if (!g_Config.cvars.show_speed || g_pPlayerMove->iuser1 != 0)
		return;

	float flSpeed = g_Local.flVelocity;

	g_Drawing.DrawStringF(g_hESP2,
						 int(m_iScreenWidth * g_Config.cvars.speed_width_fraction),
						 int(m_iScreenHeight * g_Config.cvars.speed_height_fraction),
						 int(255.f * g_Config.cvars.speed_color[0]),
						 int(255.f * g_Config.cvars.speed_color[1]),
						 int(255.f * g_Config.cvars.speed_color[2]),
						 int(255.f * g_Config.cvars.speed_color[3]),
						 FONT_ALIGN_CENTER,
						 "%.1f",
						 flSpeed);
}

void CVisual::Lightmap()
{
	if (!g_Config.cvars.lightmap_override)
	{
		g_pEngineFuncs->OverrideLightmap(0);
		return;
	}

	static bool replaced = false;

	if (g_Config.cvars.lightmap_brightness > 0.f)
	{
		if (!replaced)
		{
			g_pEngineFuncs->OverrideLightmap(1);
			replaced = true;
		}
		else
		{
			replaced = false;
		}

		g_pEngineFuncs->SetLightmapColor(g_Config.cvars.lightmap_color[0], g_Config.cvars.lightmap_color[1], g_Config.cvars.lightmap_color[2]);
		g_pEngineFuncs->SetLightmapDarkness(g_Config.cvars.lightmap_brightness);
	}
}

void CVisual::DrawCrosshair()
{
	if (!g_Config.cvars.draw_crosshair)
		return;

	if (g_pPlayerMove->iuser1 == 0 || g_Config.cvars.fp_roaming_draw_crosshair && g_FirstPersonRoaming.GetTargetPlayer())
	{
		g_Drawing.DrawCrosshair((m_iScreenWidth / 2) + 1, (m_iScreenHeight / 2) + 1, 0, 0, 0, 255, 10, 4, 3);
		g_Drawing.DrawCrosshair((m_iScreenWidth / 2) + 1, (m_iScreenHeight / 2) + 1, 255, 255, 255, 255);
	}
}

void CVisual::ESP()
{
	if (!g_Config.cvars.esp)
		return;

	cl_entity_s *pLocal = g_pEngineFuncs->GetLocalPlayer();
	cl_entity_s *pViewModel = g_pEngineFuncs->GetViewModel();

	int nLocalPlayer = pLocal->index;

	// What a mess lol
	for (int i = 1; i <= MAXENTS; ++i)
	{
		if (i == nLocalPlayer)
			continue;

		cl_entity_s *pEntity = g_pEngineFuncs->GetEntityByIndex(i);

		studiohdr_t *pStudioHeader = NULL;
		model_s *pModel = NULL;
		class_info_t classInfo = { CLASS_PLAYER, FL_CLASS_FRIEND };

		if (!pEntity || !(pModel = pEntity->model) || *pModel->name != 'm' || (!pEntity->player && pEntity->curstate.solid <= SOLID_TRIGGER) || pEntity->curstate.movetype == MOVETYPE_NONE)
			continue;

		if (pEntity == pViewModel || pEntity->curstate.messagenum < pLocal->curstate.messagenum || pEntity->curstate.maxs.z == 0.0f && pEntity->curstate.mins.z == 0.0f)
			continue;

		if (!(pStudioHeader = (studiohdr_t *)g_pEngineStudio->Mod_Extradata(pModel)) || pStudioHeader->numhitboxes <= 3)
			continue;

		bool bPlayer = pEntity->player;
		float flDistance = (pEntity->origin - pLocal->origin).Length();

		if (flDistance > 4000.0f)
			continue;

		float vecScreenBottom[2], vecScreenTop[2];

		Vector vecBottom = pEntity->origin;
		Vector vecTop = pEntity->origin;

		if (!bPlayer)
		{
			classInfo = GetEntityClassInfo(pModel->name);

			// Don't process if entity isn't an ESP's target, or its dead body or trash
			if (g_Config.cvars.esp_targets == 2 || IsEntityClassDeadbody(classInfo, pEntity->curstate.solid) || IsEntityClassTrash(classInfo))
				continue;
		}
		else
		{
			if (g_Config.cvars.esp_targets == 1)
				continue;

			vecBottom.z -= pEntity->curstate.maxs.z;
		}

		vecTop.z += pEntity->curstate.maxs.z;

		bool bScreenBottom = WorldToScreen(vecBottom, vecScreenBottom);
		bool bScreenTop = WorldToScreen(vecTop, vecScreenTop);

		if (bScreenBottom && bScreenTop)
		{
			bool bIsEntityFriend = IsEntityClassFriend(classInfo);
			bool bIsEntityNeutral = IsEntityClassNeutral(classInfo);

			int r = int(255.f * g_Config.cvars.esp_friend_color[0]);
			int g = int(255.f * g_Config.cvars.esp_friend_color[1]);
			int b = int(255.f * g_Config.cvars.esp_friend_color[2]);

			float boxHeight = vecScreenTop[1] - vecScreenBottom[1];

			if (bPlayer)
			{
				if (pEntity->curstate.usehull)
					boxHeight *= 0.9985f;
			}
			else if (bIsEntityNeutral)
			{
				r = 255;
				g = 255;
				b = 0;
			}
			else if (!bIsEntityFriend)
			{
				r = int(255.f * g_Config.cvars.esp_enemy_color[0]);
				g = int(255.f * g_Config.cvars.esp_enemy_color[1]);
				b = int(255.f * g_Config.cvars.esp_enemy_color[2]);
			}

			int nBox = g_Config.cvars.esp_box;
			bool bOutline = g_Config.cvars.esp_box_outline;

			g_Drawing.FillArea(int(vecScreenBottom[0] - (boxHeight * 0.25f)), int(vecScreenBottom[1]), int(boxHeight / 2), int(boxHeight), r, g, b, g_Config.cvars.esp_box_fill);

			if (nBox == 1)
			{
				g_Drawing.Box(int(vecScreenBottom[0] - (boxHeight * 0.25f)), int(vecScreenBottom[1]), int(boxHeight / 2), int(boxHeight), 1, r, g, b, 200);
				
				if (bOutline)
					g_Drawing.BoxOutline(vecScreenBottom[0] - (boxHeight * 0.25f), vecScreenBottom[1], boxHeight / 2, boxHeight, 1, r, g, b, 200);
			}
			else if (nBox == 2)
			{
				g_Drawing.DrawCoalBox(int(vecScreenBottom[0] - (boxHeight * 0.25f)), int(vecScreenBottom[1]), int(boxHeight / 2), int(boxHeight), 1, r, g, b, 255);

				if (bOutline)
					g_Drawing.DrawOutlineCoalBox(int(vecScreenBottom[0] - (boxHeight * 0.25f)), int(vecScreenBottom[1]), int(boxHeight / 2), int(boxHeight), 1, r, g, b, 255);
			}
			else if (nBox == 3)
			{
				g_Drawing.BoxCorner(int(vecScreenBottom[0] - (boxHeight * 0.25f)), int(vecScreenBottom[1]), int(boxHeight / 2), int(boxHeight), 1, r, g, b, 255);

				if (bOutline)
					g_Drawing.BoxCornerOutline(int(vecScreenBottom[0] - (boxHeight * 0.25f)), int(vecScreenBottom[1]), int(boxHeight / 2), int(boxHeight), 1, r, g, b, 255);
			}

			if (g_Config.cvars.esp_box_index)
			{
				int y = int(vecScreenBottom[1] + (boxHeight / 2));
				g_Drawing.DrawStringF(g_hESP, int(vecScreenBottom[0]), y, 255, 255, 255, 255, FONT_ALIGN_CENTER, "%d", i);
			}
			
			if (g_Config.cvars.esp_box_distance)
			{
				int y = int(vecScreenTop[1] + (-8.f - boxHeight));
				g_Drawing.DrawStringF(g_hESP, int(vecScreenBottom[0]), y, 255, 255, 255, 255, FONT_ALIGN_CENTER, "%.1f", flDistance);
			}

			if (bPlayer)
			{
				if (g_Config.cvars.esp_box_player_health)
				{
					int iHealth, iActualHealth;

					if ( ( iHealth = int(GetPlayerHealth(i)) ) != 0 )
					{
						iActualHealth = iHealth;

						if (iHealth == -1)
							iHealth = 0;
						else if (iHealth > 100)
							iHealth = 100;

						int y = int(vecScreenBottom[1] + (boxHeight - 8.0f));

						g_Drawing.DrawStringF(g_hESP,
											  int(vecScreenBottom[0]),
											  y,
											  int( 255.f * (iHealth > 50 ? 1.f - 2.f * (iHealth - 50) / 100.f : 1.f) ),
											  int( 255.f * ((iHealth > 50 ? 1.f : 2.f * iHealth / 100.f)) ),
											  0,
											  255,
											  FONT_ALIGN_CENTER,
											  "%d",
											  iActualHealth);
					}
				}

				if (g_Config.cvars.esp_box_player_armor)
				{
					float flArmor = GetPlayerArmor(i);
					int y = int(vecScreenBottom[1] + (boxHeight + 8.f));

					if (flArmor >= 0.f)
						g_Drawing.DrawStringF(g_hESP, int(vecScreenBottom[0]), y, 153, 191, 255, 255, FONT_ALIGN_CENTER, "%.1f", flArmor);
				}

				if (g_Config.cvars.esp_box_player_name)
				{
					int y = int(vecScreenTop[1] + (8.f - boxHeight));
					player_info_s *pPlayer = g_pEngineStudio->PlayerInfo(i - 1);

					g_Drawing.DrawStringF(g_hESP, int(vecScreenBottom[0]), y, r, g, b, 255, FONT_ALIGN_CENTER, "%s", pPlayer->name);
				}

				if (g_Config.cvars.esp_skeleton_type == 1)
					continue;
			}
			else
			{
				if (g_Config.cvars.esp_box_entity_name)
				{
					int y = int(vecScreenTop[1] + (8.f - boxHeight));
					g_Drawing.DrawStringF(g_hESP, int(vecScreenBottom[0]), y, r, g, b, 255, FONT_ALIGN_CENTER, "%s", GetEntityClassname(classInfo));
				}

				if (g_Config.cvars.esp_skeleton_type == 2)
					continue;
			}

		#ifdef PROCESS_PLAYER_BONES_ONLY
			if ((g_Config.cvars.esp_skeleton || g_Config.cvars.esp_bones_name) && bPlayer)
		#else
			if (g_Config.cvars.esp_skeleton || g_Config.cvars.esp_bones_name)
		#endif
			{
				mstudiobone_t *pBone = (mstudiobone_t *)((byte *)pStudioHeader + pStudioHeader->boneindex);

				for (int j = 0; j < pStudioHeader->numbones; ++j)
				{
					bool bBonePoint = false;
					float vBonePoint[2];

					if ((bBonePoint = WorldToScreen(g_Bones[i].vecPoint[j], vBonePoint)) && g_Config.cvars.esp_bones_name)
						g_Drawing.DrawStringF(g_hESP, int(vBonePoint[0]), int(vBonePoint[1]), 255, 255, 255, 255, FONT_ALIGN_CENTER, "%s", pBone[j].name);

					if (g_Config.cvars.esp_skeleton)
					{
						float vParentPoint[2];

						if (!bBonePoint || g_Bones[i].nParent[j] == -1)
							continue;

						if (!WorldToScreen(g_Bones[i].vecPoint[g_Bones[i].nParent[j]], vParentPoint))
							continue;

						g_Drawing.DrawLine(int(vBonePoint[0]), int(vBonePoint[1]), int(vParentPoint[0]), int(vParentPoint[1]), 255, 255, 255, 255);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void CVisual::ProcessBones()
{
	if (!g_Config.cvars.esp)
		return;

	cl_entity_s *pEntity = g_pEngineStudio->GetCurrentEntity();
	cl_entity_s *pViewModel = g_pEngineFuncs->GetViewModel();
	cl_entity_s *pLocal = g_pEngineFuncs->GetLocalPlayer();

#ifdef PROCESS_PLAYER_BONES_ONLY
	if (!pEntity->player)
		return;
#endif

	int index = pEntity->index;
	int nLocalPlayer = pLocal->index;

	studiohdr_t *pStudioHeader = NULL;
	model_s *pModel = NULL;

	if (index == nLocalPlayer || !pEntity || !(pModel = pEntity->model) || *pModel->name != 'm' || (!pEntity->player && pEntity->curstate.solid <= 1) || pEntity->curstate.movetype == MOVETYPE_NONE)
		return;

	if (pEntity == pViewModel || pEntity->curstate.messagenum < pLocal->curstate.messagenum || pEntity->curstate.maxs.z == 0.0f && pEntity->curstate.mins.z == 0.0f)
		return;

	if (!(pStudioHeader = (studiohdr_t *)g_pEngineStudio->Mod_Extradata(pModel)) || pStudioHeader->numhitboxes <= 3)
		return;

	float vecScreenBottom[2], vecScreenTop[2];

	Vector vecBottom = pEntity->origin;
	Vector vecTop = pEntity->origin;

#ifndef PROCESS_PLAYER_BONES_ONLY
	if (pEntity->player)
#endif
		vecBottom.z -= pEntity->curstate.maxs.z;

	vecTop.z += pEntity->curstate.maxs.z;

	bool bScreenBottom = WorldToScreen(vecBottom, vecScreenBottom);
	bool bScreenTop = WorldToScreen(vecTop, vecScreenTop);

	//memset(g_Bones[index].vecPoint, 0, sizeof(bone_s::vecPoint));
	//memset(g_Bones[index].nParent, -1, sizeof(bone_s::nParent));

	if (bScreenBottom && bScreenTop)
	{
		//mstudiobbox_t *pHitbox = (mstudiobbox_t *)((byte *)pStudioHeader + pStudioHeader->hitboxindex);
		mstudiobone_t *pBone = (mstudiobone_t *)((byte *)pStudioHeader + pStudioHeader->boneindex);
		Vector vecFrameVelocity = pEntity->curstate.velocity * (pEntity->curstate.animtime - pEntity->prevstate.animtime);

		for (int i = 0; i < pStudioHeader->numbones; ++i)
		{
			Vector vecBone = Vector((*g_pBoneTransform)[i][0][3], (*g_pBoneTransform)[i][1][3], (*g_pBoneTransform)[i][2][3]) + vecFrameVelocity;

			g_Bones[index].vecPoint[i] = vecBone;
			g_Bones[index].nParent[i] = pBone[i].parent;
		}
	}
}

bool CVisual::StudioRenderModel()
{
	ProcessBones();

	float flDrawEntitiesMode = float(g_Config.cvars.draw_entities) + 1.0f;

	if (flDrawEntitiesMode == 6.0f)
	{
		cl_entity_s *pEntity = g_pEngineStudio->GetCurrentEntity();

		if ( pEntity->player )
			r_drawentities->value = 2.0f;
		else
			r_drawentities->value = 1.0f;
	}
	else
	{
		r_drawentities->value = flDrawEntitiesMode;
	}

	if (g_Config.cvars.wallhack && r_drawentities->value >= 2.0f && r_drawentities->value <= 5.0f)
	{
		glDisable(GL_DEPTH_TEST);

		g_pStudioRenderer->StudioRenderFinal_Hardware();

		glEnable(GL_DEPTH_TEST);

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------

int UserMsgHook_ScreenShake(const char *pszUserMsg, int iSize, void *pBuffer)
{
	if (g_Config.cvars.no_shake)
		return 0;

	return UserMsgHook_ScreenShake_Original(pszUserMsg, iSize, pBuffer);
}

int UserMsgHook_ScreenFade(const char *pszUserMsg, int iSize, void *pBuffer)
{
	if (g_Config.cvars.no_fade)
		return 0;

	return UserMsgHook_ScreenFade_Original(pszUserMsg, iSize, pBuffer);
}

//-----------------------------------------------------------------------------

void CVisual::Init()
{
	g_pBoneTransform = (bone_matrix3x4_t *)g_pEngineStudio->StudioGetLightTransform();
	r_drawentities = g_pEngineFuncs->pfnGetCvarPointer("r_drawentities");

	UserMsgHook_ScreenShake_Original = HookUserMsg("ScreenShake", UserMsgHook_ScreenShake);
	UserMsgHook_ScreenFade_Original = HookUserMsg("ScreenFade", UserMsgHook_ScreenFade);
}