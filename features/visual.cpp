// Visual

#include <Windows.h>
#include <gl/GL.h>
#include <omp.h>

#include <algorithm>

#include <dbg.h>
#include <convar.h>
#include <IClient.h>
#include <IPlayerUtils.h>

#include <hl_sdk/engine/studio.h>
#include <hl_sdk/cl_dll/cl_dll.h>
#include <hl_sdk/cl_dll/StudioModelRenderer.h>
#include <hl_sdk/common/cl_entity.h>
#include <hl_sdk/common/com_model.h>
#include <hl_sdk/common/r_studioint.h>

#include "visual.h"
#include "camhack.h"
#include "firstperson_roaming.h"

#include "../game/utils.h"
#include "../game/drawing.h"
#include "../game/class_table.h"

#include "../config.h"

//#define PROCESS_PLAYER_BONES_ONLY

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

cvar_t *r_drawentities = NULL;

UserMsgHookFn ORIG_UserMsgHook_ScreenShake = NULL;
UserMsgHookFn ORIG_UserMsgHook_ScreenFade = NULL;

//-----------------------------------------------------------------------------
// Console Commands
//-----------------------------------------------------------------------------

CON_COMMAND_EXTERN(sc_wallhack, ConCommand_Wallhack, "Simple OpenGL wallhack")
{
	Msg(g_Config.cvars.wallhack ? "Wallhack disabled\n" : "Wallhack enabled\n");
	g_Config.cvars.wallhack = !g_Config.cvars.wallhack;
}

CON_COMMAND_EXTERN(sc_wallhack_negative, ConCommand_NegativeMode, "Night Mode wallhack")
{
	Msg(g_Config.cvars.wallhack_negative ? "Negative mode disabled\n" : "Negative mode enabled\n");
	g_Config.cvars.wallhack_negative = !g_Config.cvars.wallhack_negative;
}

CON_COMMAND_EXTERN(sc_wallhack_lambert, ConCommand_WhiteWalls, "Lambert wallhack")
{
	Msg(g_Config.cvars.wallhack_white_walls ? "Lambert wallhack disabled\n" : "Lambert wallhack enabled\n");
	g_Config.cvars.wallhack_white_walls = !g_Config.cvars.wallhack_white_walls;
}

CON_COMMAND_EXTERN(sc_wallhack_wireframe, ConCommand_Wireframe, "Wireframe view")
{
	Msg(g_Config.cvars.wallhack_wireframe ? "Wireframe disabled\n" : "Wireframe enabled\n");
	g_Config.cvars.wallhack_wireframe = !g_Config.cvars.wallhack_wireframe;
}

CON_COMMAND_EXTERN(sc_wallhack_wireframe_models, ConCommand_WireframeModels, "Wireframe view (entity models only)")
{
	Msg(g_Config.cvars.wallhack_wireframe_models ? "Wireframe Models disabled\n" : "Wireframe Models enabled\n");
	g_Config.cvars.wallhack_wireframe_models = !g_Config.cvars.wallhack_wireframe_models;
}

CON_COMMAND_EXTERN(sc_esp, ConCommand_ESP, "Toggle ESP")
{
	Msg(g_Config.cvars.esp ? "ESP disabled\n" : "ESP enabled\n");
	g_Config.cvars.esp = !g_Config.cvars.esp;
}

//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------

static bool IsWorldModelItem(const char *pszModelName)
{
	if (*pszModelName && *pszModelName == 'w' && pszModelName[1] && pszModelName[1] == '_')
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Visual Hack
//-----------------------------------------------------------------------------

void CVisual::OnVideoInit()
{
	m_flTime = 0.f;

	ResetJumpSpeed();
}

void CVisual::OnHUDRedraw(float flTime)
{
	m_flTime = flTime;

	ShowSpeed();
}

void CVisual::Process()
{
	m_iScreenWidth = g_ScreenInfo.width;
	m_iScreenHeight = g_ScreenInfo.height;

	Lightmap();

	ESP();

	DrawCrosshair();
}

void CVisual::ResetJumpSpeed()
{
	m_flPrevTime = m_flTime;
	m_flFadeTime = g_Config.cvars.jumpspeed_fade_duration;
	m_flJumpSpeed = 0.f;

	m_clFadeFrom[0] = int(255.f * g_Config.cvars.speed_color[0]);
	m_clFadeFrom[1] = int(255.f * g_Config.cvars.speed_color[1]);
	m_clFadeFrom[2] = int(255.f * g_Config.cvars.speed_color[2]);

	m_bOnGround = true;
}

void CVisual::ShowSpeed()
{
	if ( !g_pClient->IsSpectating() && !(g_CamHack.IsEnabled() && g_Config.cvars.camhack_hide_hud) )
	{
		float flSpeed;

		if (g_Config.cvars.show_speed)
		{
			if ( g_Config.cvars.show_vertical_speed )
				flSpeed = g_pPlayerMove->velocity.Length();
			else
				flSpeed = g_pPlayerMove->velocity.Length2D();

			g_Drawing.DrawNumber(flSpeed > 0.f ? int(floor(flSpeed)) : int(ceil(flSpeed)),
								 int(m_iScreenWidth * g_Config.cvars.speed_width_fraction),
								 int(m_iScreenHeight * g_Config.cvars.speed_height_fraction),
								 int(255.f * g_Config.cvars.speed_color[0]),
								 int(255.f * g_Config.cvars.speed_color[1]),
								 int(255.f * g_Config.cvars.speed_color[2]),
								 FONT_ALIGN_CENTER);

			if (g_Config.cvars.show_jumpspeed)
			{
				int r = int(255.f * g_Config.cvars.speed_color[0]);
				int g = int(255.f * g_Config.cvars.speed_color[1]);
				int b = int(255.f * g_Config.cvars.speed_color[2]);

				float flFadeDuration = g_Config.cvars.jumpspeed_fade_duration;
				int iSpriteHeight = g_Drawing.GetNumberSpriteHeight();

				if (flFadeDuration > 0.0f)
				{
					if ( !g_pClient->IsOnGround() )
					{
						if ( m_bOnGround && g_pClient->Buttons() & IN_JUMP )
						{
							float flDifference = flSpeed - m_flJumpSpeed;

							if (flDifference != 0.0f)
							{
								if (flDifference > 0.0f)
								{
									m_clFadeFrom[0] = 0;
									m_clFadeFrom[1] = 255;
									m_clFadeFrom[2] = 0;
								}
								else
								{
									m_clFadeFrom[0] = 255;
									m_clFadeFrom[1] = 0;
									m_clFadeFrom[2] = 0;
								}

								m_flFadeTime = 0.0f;
								m_flJumpSpeed = flSpeed;
							}
						}

						m_bOnGround = false;
					}
					else
					{
						m_bOnGround = true;
					}

					float flDelta = V_max(m_flTime - m_flPrevTime, 0.0f);

					m_flFadeTime += flDelta;

					if (m_flFadeTime > flFadeDuration || !IsFloatFinite(m_flFadeTime) )
						m_flFadeTime = flFadeDuration;

					float flFadeFrom_R = int(255.f * g_Config.cvars.speed_color[0]) - m_clFadeFrom[0] / flFadeDuration;
					float flFadeFrom_G = int(255.f * g_Config.cvars.speed_color[1]) - m_clFadeFrom[1] / flFadeDuration;
					float flFadeFrom_B = int(255.f * g_Config.cvars.speed_color[2]) - m_clFadeFrom[2] / flFadeDuration;

					r = int(int(255.f * g_Config.cvars.speed_color[0]) - flFadeFrom_R * (flFadeDuration - m_flFadeTime));
					g = int(int(255.f * g_Config.cvars.speed_color[1]) - flFadeFrom_G * (flFadeDuration - m_flFadeTime));
					b = int(int(255.f * g_Config.cvars.speed_color[2]) - flFadeFrom_B * (flFadeDuration - m_flFadeTime));

					m_flPrevTime = m_flTime;
				}

				g_Drawing.DrawNumber(m_flJumpSpeed > 0.f ? int(floor(m_flJumpSpeed)) : int(ceil(m_flJumpSpeed)),
									 int(m_iScreenWidth * g_Config.cvars.speed_width_fraction),
									 int(m_iScreenHeight * g_Config.cvars.speed_height_fraction) - (iSpriteHeight + iSpriteHeight / 4),
									 r,
									 g,
									 b,
									 FONT_ALIGN_CENTER);
			}
		}
		else if (g_Config.cvars.show_speed_legacy)
		{
			float flSpeed;

			if ( g_Config.cvars.show_vertical_speed_legacy )
				flSpeed = g_pPlayerMove->velocity.Length();
			else
				flSpeed = g_pPlayerMove->velocity.Length2D();

			g_Drawing.DrawStringF(g_hFontSpeedometer,
								  int(m_iScreenWidth * g_Config.cvars.speed_width_fraction_legacy),
								  int(m_iScreenHeight * g_Config.cvars.speed_height_fraction_legacy),
								  int(255.f * g_Config.cvars.speed_color_legacy[0]),
								  int(255.f * g_Config.cvars.speed_color_legacy[1]),
								  int(255.f * g_Config.cvars.speed_color_legacy[2]),
								  int(255.f * g_Config.cvars.speed_color_legacy[3]),
								  FONT_ALIGN_CENTER,
								  "%.1f",
								  flSpeed);
		}
	}
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
	if ( !g_Config.cvars.draw_crosshair || (g_CamHack.IsEnabled() && g_Config.cvars.camhack_hide_hud) )
		return;

	if (g_pPlayerMove->iuser1 == 0 || g_Config.cvars.fp_roaming_draw_crosshair && g_FirstPersonRoaming.GetTargetPlayer())
	{
		if (g_Config.cvars.draw_crosshair_outline)
		{
			g_Drawing.DrawCrosshairShadow((m_iScreenWidth / 2) - 1,
											(m_iScreenHeight / 2) - 1,
											int(255.f * g_Config.cvars.crosshair_outline_color[0]),
											int(255.f * g_Config.cvars.crosshair_outline_color[1]),
											int(255.f * g_Config.cvars.crosshair_outline_color[2]),
											int(255.f * g_Config.cvars.crosshair_outline_color[3]),
											g_Config.cvars.crosshair_size,
											g_Config.cvars.crosshair_gap,
											g_Config.cvars.crosshair_thickness,
											g_Config.cvars.crosshair_outline_thickness);
			
			//g_Drawing.DrawCrosshair((m_iScreenWidth / 2) - 1,
			//						(m_iScreenHeight / 2) - 1,
			//						int(255.f * g_Config.cvars.crosshair_outline_color[0]),
			//						int(255.f * g_Config.cvars.crosshair_outline_color[1]),
			//						int(255.f * g_Config.cvars.crosshair_outline_color[2]),
			//						int(255.f * g_Config.cvars.crosshair_outline_color[3]),
			//						g_Config.cvars.crosshair_size,
			//						g_Config.cvars.crosshair_gap,
			//						g_Config.cvars.crosshair_thickness + g_Config.cvars.crosshair_outline_thickness);

			if (g_Config.cvars.draw_crosshair_dot)
			{
				g_Drawing.DrawDotShadow((m_iScreenWidth / 2) - 1,
										(m_iScreenHeight / 2) - 1,
										int(255.f * g_Config.cvars.crosshair_outline_color[0]),
										int(255.f * g_Config.cvars.crosshair_outline_color[1]),
										int(255.f * g_Config.cvars.crosshair_outline_color[2]),
										int(255.f * g_Config.cvars.crosshair_outline_color[3]),
										g_Config.cvars.crosshair_thickness,
										g_Config.cvars.crosshair_outline_thickness);

				//g_Drawing.DrawDot((m_iScreenWidth / 2) - 1,
				//				  (m_iScreenHeight / 2) - 1,
				//				  int(255.f * g_Config.cvars.crosshair_outline_color[0]),
				//				  int(255.f * g_Config.cvars.crosshair_outline_color[1]),
				//				  int(255.f * g_Config.cvars.crosshair_outline_color[2]),
				//				  int(255.f * g_Config.cvars.crosshair_outline_color[3]),
				//				  g_Config.cvars.crosshair_thickness + g_Config.cvars.crosshair_outline_thickness);
			}
		}

		if (g_Config.cvars.draw_crosshair_dot)
		{
			g_Drawing.DrawDot((m_iScreenWidth / 2) - 1,
							  (m_iScreenHeight / 2) - 1,
							  int(255.f * g_Config.cvars.crosshair_color[0]),
							  int(255.f * g_Config.cvars.crosshair_color[1]),
							  int(255.f * g_Config.cvars.crosshair_color[2]),
							  int(255.f * g_Config.cvars.crosshair_color[3]),
							  g_Config.cvars.crosshair_thickness);
		}

		g_Drawing.DrawCrosshair((m_iScreenWidth / 2) - 1,
								(m_iScreenHeight / 2) - 1,
								int(255.f * g_Config.cvars.crosshair_color[0]),
								int(255.f * g_Config.cvars.crosshair_color[1]),
								int(255.f * g_Config.cvars.crosshair_color[2]),
								int(255.f * g_Config.cvars.crosshair_color[3]),
								g_Config.cvars.crosshair_size,
								g_Config.cvars.crosshair_gap,
								g_Config.cvars.crosshair_thickness);
	}
}

void CVisual::ESP()
{
	if ( !g_Config.cvars.esp )
		return;

	cl_entity_s *pLocal = g_pEngineFuncs->GetLocalPlayer();
	cl_entity_s *pViewModel = g_pEngineFuncs->GetViewModel();

	int nLocalPlayer = pLocal->index;

#pragma omp parallel for /* num_threads(2) */ // not sure if it's worth it
	for (register int i = 1; i <= MAXENTS; ++i)
	{
		// What a mess lol

		if ( i == nLocalPlayer )
			continue;

		bool bPlayer;
		bool bItem;
		bool bClassInfoRetrieved = false;

		float flDistance;
		float vecScreenBottom[2], vecScreenTop[2];

		const char *pszModelName;
		const char *pszSlashLastOccur;

		studiohdr_t *pStudioHeader = NULL;
		model_s *pModel = NULL;

		cl_entity_s *pEntity = g_pEngineFuncs->GetEntityByIndex(i);
		class_info_t classInfo = { CLASS_PLAYER, FL_CLASS_FRIEND };

		if ( !pEntity || !(pModel = pEntity->model) || *pModel->name != 'm' || pEntity == pViewModel || pEntity->curstate.messagenum < pLocal->curstate.messagenum )
			continue;

		if ( pszSlashLastOccur = strrchr(pModel->name, '/') )
			pszModelName = pszSlashLastOccur + 1;
		else
			continue;

		if ( bItem = IsWorldModelItem(pszModelName) )
		{
			classInfo = GetEntityClassInfo(pModel->name);
			bClassInfoRetrieved = true;

			if ( !(classInfo.flags & FL_CLASS_ITEM) )
				bItem = false;
		}

		if ( !bItem )
		{
			if ( (!pEntity->player && pEntity->curstate.solid <= SOLID_TRIGGER) || pEntity->curstate.movetype == MOVETYPE_NONE )
				continue;

			if ( pEntity->curstate.maxs.z == 0.0f && pEntity->curstate.mins.z == 0.0f )
				continue;

			if ( !(pStudioHeader = (studiohdr_t *)g_pEngineStudio->Mod_Extradata(pModel)) || pStudioHeader->numhitboxes == 0 )
				continue;
		}
		else if ( !g_Config.cvars.esp_show_items )
		{
			continue;
		}

		bPlayer = pEntity->player;
		flDistance = (pEntity->origin - pLocal->origin).Length();

		if ( flDistance > g_Config.cvars.esp_distance )
			continue;

		float boxWidth;

		Vector vecBottom = pEntity->origin;
		Vector vecTop = pEntity->origin;

		if ( !bPlayer )
		{
			if ( !bClassInfoRetrieved )
				classInfo = GetEntityClassInfo(pModel->name);

			if ( classInfo.id == CLASS_NONE && g_Config.cvars.esp_ignore_unknown_ents )
				continue;

			// Don't process if entity isn't an ESP's target, or its dead body or some kind of trash
			if ( g_Config.cvars.esp_targets == 2 || IsEntityClassDeadbody(classInfo, pEntity->curstate.solid) || IsEntityClassTrash(classInfo) )
				continue;

			if ( !bItem )
			{
				vecTop.z += pEntity->curstate.maxs.z;
				vecBottom.z -= pEntity->curstate.mins.z;

				boxWidth = max(pEntity->curstate.maxs.x - pEntity->curstate.mins.x, pEntity->curstate.maxs.y - pEntity->curstate.mins.y) /
					(pEntity->curstate.maxs.z - pEntity->curstate.mins.z);
			}
		}
		else
		{
			if ( g_Config.cvars.esp_targets == 1 )
				continue;

			// Ducking
			if ( pEntity->curstate.usehull )
			{
				vecTop.z += VEC_DUCK_HULL_MAX.z;
				vecBottom.z += VEC_DUCK_HULL_MIN.z;

				boxWidth = (VEC_DUCK_HULL_MAX.x - VEC_DUCK_HULL_MIN.x) / (VEC_DUCK_HULL_MAX.z - VEC_DUCK_HULL_MIN.z);
			}
			else
			{
				vecTop.z += pEntity->curstate.maxs.z;
				vecBottom.z -= pEntity->curstate.maxs.z;

				boxWidth = (VEC_HULL_MAX.x - VEC_HULL_MIN.x) / (VEC_HULL_MAX.z - VEC_HULL_MIN.z);
			}
		}

		bool bScreenBottom = UTIL_WorldToScreen(vecBottom, vecScreenBottom);
		bool bScreenTop = UTIL_WorldToScreen(vecTop, vecScreenTop);

		if ( bScreenBottom && bScreenTop )
		{
			if (bPlayer && g_Config.cvars.esp_show_visible_players)
			{
				pmtrace_t trace;

				Vector vecStart = Client()->GetOrigin() + Client()->GetViewOffset();
				Vector vecEnd = vecBottom + ((vecTop - vecBottom) * 0.5f);

				g_pEventAPI->EV_SetTraceHull(PM_HULL_POINT);
				g_pEventAPI->EV_PlayerTrace(vecStart, vecEnd, PM_WORLD_ONLY, -1, &trace);

				if (trace.fraction != 1.f)
				{
					continue;
				}
			}

			int iHealth = bPlayer ? g_pPlayerUtils->GetHealth(i) : 0;

			bool bIsEntityFriend = IsEntityClassFriend(classInfo);
			bool bIsEntityNeutral = IsEntityClassNeutral(classInfo);

			int r = int(255.f * g_Config.cvars.esp_friend_color[0]);
			int g = int(255.f * g_Config.cvars.esp_friend_color[1]);
			int b = int(255.f * g_Config.cvars.esp_friend_color[2]);

			float boxHeight = vecScreenTop[1] - vecScreenBottom[1];

			if ( bPlayer && iHealth < -1 ) // enemy team
				bIsEntityFriend = false;

			if ( bItem )
			{
				boxHeight = 0.f;

				r = int(255.f * g_Config.cvars.esp_item_color[0]);
				g = int(255.f * g_Config.cvars.esp_item_color[1]);
				b = int(255.f * g_Config.cvars.esp_item_color[2]);
			}
			else if ( bIsEntityNeutral )
			{
				r = int(255.f * g_Config.cvars.esp_neutral_color[0]);
				g = int(255.f * g_Config.cvars.esp_neutral_color[1]);
				b = int(255.f * g_Config.cvars.esp_neutral_color[2]);
			}
			else if ( !bIsEntityFriend )
			{
				r = int(255.f * g_Config.cvars.esp_enemy_color[0]);
				g = int(255.f * g_Config.cvars.esp_enemy_color[1]);
				b = int(255.f * g_Config.cvars.esp_enemy_color[2]);
			}

			boxWidth = boxHeight * boxWidth;

			if (bPlayer)
			{
				// Box Fill
				if ( g_Config.cvars.esp_box_targets != 1 )
				{
					DrawBox(bPlayer, bItem, iHealth, boxHeight, boxWidth, vecScreenBottom, r, g, b);
				}

				// Distance
				if ( g_Config.cvars.esp_box_distance && g_Config.cvars.esp_distance_mode != 1 )
				{
					int y = int(vecScreenTop[1] + (-8.f - boxHeight));
					g_Drawing.DrawStringF(g_hFontESP, int(vecScreenBottom[0]), y, 255, 255, 255, 255, FONT_ALIGN_CENTER, "%.1f", flDistance);
				}

				// General Info
				if (g_Config.cvars.esp_player_style == 0) // Default
				{
					DrawPlayerInfo_Default(i, iHealth, bIsEntityFriend, boxHeight, vecScreenBottom, vecScreenTop);
				}
				else if (g_Config.cvars.esp_player_style == 1) // SAMP
				{
					DrawPlayerInfo_SAMP(i, iHealth, (bool)pEntity->curstate.usehull, bIsEntityFriend, vecTop);
				}
				else if (g_Config.cvars.esp_player_style == 2) // Left 4 Dead
				{
					DrawPlayerInfo_L4D(i, iHealth, (bool)pEntity->curstate.usehull, bIsEntityFriend, vecTop);
				}

				if ( g_Config.cvars.esp_skeleton_type == 1 )
					continue;
			}
			else
			{
				// Box Fill
				if ( g_Config.cvars.esp_box_targets != 2 )
				{
					DrawBox(bPlayer, bItem, iHealth, boxHeight, boxWidth, vecScreenBottom, r, g, b);
				}

				// Distance
				if ( g_Config.cvars.esp_box_distance && g_Config.cvars.esp_distance_mode != 2 )
				{
					int y = int(vecScreenTop[1] + (-8.f - boxHeight));
					g_Drawing.DrawStringF(g_hFontESP, int(vecScreenBottom[0]), y, 255, 255, 255, 255, FONT_ALIGN_CENTER, "%.1f", flDistance);
				}

				// General Info
				if (g_Config.cvars.esp_entity_style == 0) // Default
				{
					DrawEntityInfo_Default(i, classInfo, boxHeight, vecScreenBottom, vecScreenTop, r, g, b);
				}
				else if (g_Config.cvars.esp_entity_style == 1) // SAMP
				{
					DrawEntityInfo_SAMP(i, classInfo, vecTop, r, g, b);
				}
				else if (g_Config.cvars.esp_entity_style == 2) // Left 4 Dead
				{
					DrawEntityInfo_L4D(i, classInfo, vecTop, r, g, b);
				}

				if ( g_Config.cvars.esp_skeleton_type == 2 )
					continue;
			}

			if ( bItem )
				continue;

			DrawBones(i, pStudioHeader);
		}
	}
}

void CVisual::DrawPlayerInfo_Default(int index, int iHealth, bool bIsEntityFriend, float boxHeight, float vecScreenBottom[2], float vecScreenTop[2])
{
	if (g_Config.cvars.esp_box_player_health && iHealth != 0)
	{
		int r, g, b;

		int iActualHealth = iHealth;

		if (iHealth == -1)
			iActualHealth = iHealth = 0;
		else if (iHealth > 100)
			iHealth = 100;

		int y = int(vecScreenBottom[1] + (boxHeight - 8.0f));

		if (bIsEntityFriend)
		{
			r = int( 255.f * (iHealth > 50 ? 1.f - 2.f * (iHealth - 50) / 100.f : 1.f) );
			g = int( 255.f * ((iHealth > 50 ? 1.f : 2.f * iHealth / 100.f)) );
			b = 0;
		}
		else
		{
			iActualHealth = -1;

			r = 0;
			g = 255;
			b = 255;
		}

		g_Drawing.DrawStringF(g_hFontESP,
								int(vecScreenBottom[0]),
								y,
								r,
								g,
								b,
								255,
								FONT_ALIGN_CENTER,
								"%d",
								iActualHealth);
	}

	if (g_Config.cvars.esp_box_player_armor)
	{
		float flArmor = g_pPlayerUtils->GetArmor(index);
		int y = int(vecScreenBottom[1] + (boxHeight + 8.f));

		if (flArmor > 0.f)
			g_Drawing.DrawStringF(g_hFontESP, int(vecScreenBottom[0]), y, 153, 191, 255, 255, FONT_ALIGN_CENTER, "%.1f", flArmor);
	}

	if (g_Config.cvars.esp_box_player_name || g_Config.cvars.esp_box_index)
	{
		static char szIndex[16];
		player_info_t *pPlayer = NULL;

		if (g_Config.cvars.esp_box_index)
			snprintf(szIndex, sizeof(szIndex), g_Config.cvars.esp_box_player_name ? " (%d)" : "(%d)", index);

		int nickname_r, nickname_g, nickname_b;
		int y = int(vecScreenTop[1] + (8.f - boxHeight));

		if (bIsEntityFriend)
		{
			nickname_r = int(255.f * g_Config.cvars.esp_friend_player_color[0]);
			nickname_g = int(255.f * g_Config.cvars.esp_friend_player_color[1]);
			nickname_b = int(255.f * g_Config.cvars.esp_friend_player_color[2]);
		}
		else
		{
			nickname_r = int(255.f * g_Config.cvars.esp_enemy_player_color[0]);
			nickname_g = int(255.f * g_Config.cvars.esp_enemy_player_color[1]);
			nickname_b = int(255.f * g_Config.cvars.esp_enemy_player_color[2]);
		}

		g_Drawing.DrawStringF(g_hFontESP, int(vecScreenBottom[0]), y, nickname_r, nickname_g, nickname_b, 255, FONT_ALIGN_CENTER, "%s%s",
								g_Config.cvars.esp_box_player_name ? (pPlayer = g_pEngineStudio->PlayerInfo(index - 1), pPlayer->name) : "",
								g_Config.cvars.esp_box_index ? szIndex : "");
	}
}

void CVisual::DrawPlayerInfo_SAMP(int index, int iHealth, bool bDucking, bool bIsEntityFriend, Vector vecTop)
{
	constexpr int iBarWidth = 42;
	constexpr int iBarHeight = 4;
	constexpr int iThickness = 1;

	float vecScreen[2];

	if (bDucking)
		vecTop.z += 12.f;
	else
		vecTop.z += 6.f;

	if ( !UTIL_WorldToScreen(vecTop, vecScreen) )
		return;

	int offset_x = int(vecScreen[0]);
	int offset_y = int(vecScreen[1]);

	int health_offset_y = offset_y;

	if (g_Config.cvars.esp_box_player_armor)
	{
		float flArmor = g_pPlayerUtils->GetArmor(index);

		if (flArmor > 0.f)
		{
			if (flArmor > 100.f)
				flArmor = 100.f;

			float flFraction = flArmor / 100.f;

			// Thickness
			{
				constexpr int iWidth = iBarWidth + 2 * iThickness;
				constexpr int iHeight = iBarHeight + 2 * iThickness;

				g_Drawing.FillArea(offset_x - iWidth / 2, offset_y - iHeight / 2, iWidth, iHeight, 0, 0, 0, 255);
			}

			if (flArmor != 100.f)
			{
				g_Drawing.FillArea(offset_x - iBarWidth / 2, offset_y - iBarHeight / 2, iBarWidth, iBarHeight, 40, 40, 40, 255);
			}

			g_Drawing.FillArea(offset_x - iBarWidth / 2, offset_y - iBarHeight / 2, int((float)iBarWidth * flFraction), iBarHeight, 200, 200, 200, 255);

			health_offset_y += ((iBarHeight + 2 * iThickness) / 2) + 5;
		}
	}

	if (g_Config.cvars.esp_box_player_health && iHealth != 0)
	{
		if (iHealth < 0)
			iHealth = 0;
		else if (iHealth > 100)
			iHealth = 100;

		int y = health_offset_y;
		float flFraction = (float)iHealth / 100.f;

		// Thickness
		{
			constexpr int iWidth = iBarWidth + 2 * iThickness;
			constexpr int iHeight = iBarHeight + 2 * iThickness;

			g_Drawing.FillArea(offset_x - iWidth / 2, y - iHeight / 2, iWidth, iHeight, 0, 0, 0, 255);
		}

		if (iHealth != 100)
		{
			g_Drawing.FillArea(offset_x - iBarWidth / 2, y - iBarHeight / 2, iBarWidth, iBarHeight, 76, 11, 20, 255);
		}

		g_Drawing.FillArea(offset_x - iBarWidth / 2, y - iBarHeight / 2, int((float)iBarWidth * flFraction), iBarHeight, 187, 32, 40, 255);
	}

	if (g_Config.cvars.esp_box_player_name || g_Config.cvars.esp_box_index)
	{
		static char szIndex[16];
		player_info_t *pPlayer = NULL;

		if (g_Config.cvars.esp_box_index)
			snprintf(szIndex, sizeof(szIndex), g_Config.cvars.esp_box_player_name ? " (%d)" : "(%d)", index);

		int nickname_r, nickname_g, nickname_b;
		int y = offset_y - ((iBarHeight + 2 * iThickness) / 2) - 14;

		if (bIsEntityFriend)
		{
			nickname_r = int(255.f * g_Config.cvars.esp_friend_player_color[0]);
			nickname_g = int(255.f * g_Config.cvars.esp_friend_player_color[1]);
			nickname_b = int(255.f * g_Config.cvars.esp_friend_player_color[2]);
		}
		else
		{
			nickname_r = int(255.f * g_Config.cvars.esp_enemy_player_color[0]);
			nickname_g = int(255.f * g_Config.cvars.esp_enemy_player_color[1]);
			nickname_b = int(255.f * g_Config.cvars.esp_enemy_player_color[2]);
		}

		g_Drawing.DrawStringF(g_hFontESP, offset_x, y, nickname_r, nickname_g, nickname_b, 255, FONT_ALIGN_CENTER, "%s%s",
								g_Config.cvars.esp_box_player_name ? (pPlayer = g_pEngineStudio->PlayerInfo(index - 1), pPlayer->name) : "",
								g_Config.cvars.esp_box_index ? szIndex : "");
	}
}

void CVisual::DrawPlayerInfo_L4D(int index, int iHealth, bool bDucking, bool bIsEntityFriend, Vector vecTop)
{
	float vecScreen[2];

	if (bDucking)
		vecTop.z += 24.f;
	else
		vecTop.z += 14.f;

	if ( !UTIL_WorldToScreen(vecTop, vecScreen) )
		return;

	int offset_x = vecScreen[0];
	int offset_y = vecScreen[1] + 25;

	if (g_Config.cvars.esp_box_player_name || g_Config.cvars.esp_box_player_health || g_Config.cvars.esp_box_player_armor)
	{
		const char *szFormatString;
		static char szInfo[16];

		player_info_t *pPlayer;

		szFormatString = g_Config.cvars.esp_box_player_name ? " (%d)" : "(%d)";

		if (g_Config.cvars.esp_box_player_health && g_Config.cvars.esp_box_player_armor)
		{
			if (iHealth < 0)
				iHealth = 0;
			else if (iHealth > 100)
				iHealth = 100;
			
			float flArmor = g_pPlayerUtils->GetArmor(index);

			if (flArmor > 0.f)
				snprintf(szInfo, sizeof(szInfo), g_Config.cvars.esp_box_player_name ? " (%d) [%.1f]" : "(%d) [%.1f]", iHealth, flArmor);
			else
				snprintf(szInfo, sizeof(szInfo), g_Config.cvars.esp_box_player_name ? " (%d)" : "(%d)", iHealth);
		}
		else if (g_Config.cvars.esp_box_player_health)
		{
			if (iHealth < 0)
				iHealth = 0;
			else if (iHealth > 100)
				iHealth = 100;

			snprintf(szInfo, sizeof(szInfo), g_Config.cvars.esp_box_player_name ? " (%d)" : "(%d)", iHealth);
		}
		else if (g_Config.cvars.esp_box_player_armor)
		{
			float flArmor = g_pPlayerUtils->GetArmor(index);

			if (flArmor > 0.f)
				snprintf(szInfo, sizeof(szInfo), g_Config.cvars.esp_box_player_name ? " [%.1f]" : "[%.1f]", flArmor);
			else
				szInfo[0] = '\0';
		}
		else
		{
			szInfo[0] = '\0';
		}

		int nickname_r, nickname_g, nickname_b;

		int x = vecScreen[0];
		int y = vecScreen[1];

		if (bIsEntityFriend)
		{
			nickname_r = int(255.f * g_Config.cvars.esp_friend_player_color[0]);
			nickname_g = int(255.f * g_Config.cvars.esp_friend_player_color[1]);
			nickname_b = int(255.f * g_Config.cvars.esp_friend_player_color[2]);
		}
		else
		{
			nickname_r = int(255.f * g_Config.cvars.esp_enemy_player_color[0]);
			nickname_g = int(255.f * g_Config.cvars.esp_enemy_player_color[1]);
			nickname_b = int(255.f * g_Config.cvars.esp_enemy_player_color[2]);
		}

		g_Drawing.DrawStringF(g_hFontESP2, x, y, nickname_r, nickname_g, nickname_b, 255, FONT_ALIGN_CENTER, "%s%s",
								g_Config.cvars.esp_box_player_name ? (pPlayer = g_pEngineStudio->PlayerInfo(index - 1), pPlayer->name) : "",
								szInfo);
	}
}

void CVisual::DrawEntityInfo_Default(int index, class_info_t classInfo, float boxHeight, float vecScreenBottom[2], float vecScreenTop[2], int r, int g, int b)
{
	if (g_Config.cvars.esp_box_entity_name || g_Config.cvars.esp_box_index)
	{
		static char szIndex[16];

		if (g_Config.cvars.esp_box_index)
			snprintf(szIndex, sizeof(szIndex), g_Config.cvars.esp_box_entity_name ? " (%d)" : "(%d)", index);

		int y = int(vecScreenTop[1] + (8.f - boxHeight));
		g_Drawing.DrawStringF(g_hFontESP, int(vecScreenBottom[0]), y, r, g, b, 255, FONT_ALIGN_CENTER, "%s%s",
								g_Config.cvars.esp_box_entity_name ? GetEntityClassname(classInfo) : "",
								g_Config.cvars.esp_box_index ? szIndex : "");
	}
}

void CVisual::DrawEntityInfo_SAMP(int index, class_info_t classInfo, Vector vecTop, int r, int g, int b)
{
	if (g_Config.cvars.esp_box_entity_name || g_Config.cvars.esp_box_index)
	{
		float vecScreen[2];
		vecTop.z += 6.f;

		UTIL_WorldToScreen(vecTop, vecScreen);

		int x = int(vecScreen[0]);
		int y = int(vecScreen[1]);

		static char szIndex[16];

		if (g_Config.cvars.esp_box_index)
			snprintf(szIndex, sizeof(szIndex), g_Config.cvars.esp_box_entity_name ? " (%d)" : "(%d)", index);

		g_Drawing.DrawStringF(g_hFontESP, x, y, r, g, b, 255, FONT_ALIGN_CENTER, "%s%s",
								g_Config.cvars.esp_box_entity_name ? GetEntityClassname(classInfo) : "",
								g_Config.cvars.esp_box_index ? szIndex : "");
	}
}

void CVisual::DrawEntityInfo_L4D(int index, class_info_t classInfo, Vector vecTop, int r, int g, int b)
{
	if (g_Config.cvars.esp_box_entity_name || g_Config.cvars.esp_box_index)
	{
		float vecScreen[2];
		vecTop.z += 14.f;

		UTIL_WorldToScreen(vecTop, vecScreen);

		int x = int(vecScreen[0]);
		int y = int(vecScreen[1]);

		static char szIndex[16];

		if (g_Config.cvars.esp_box_index)
			snprintf(szIndex, sizeof(szIndex), g_Config.cvars.esp_box_entity_name ? " (%d)" : "(%d)", index);

		g_Drawing.DrawStringF(g_hFontESP2, x, y, r, g, b, 255, FONT_ALIGN_CENTER, "%s%s",
								g_Config.cvars.esp_box_entity_name ? GetEntityClassname(classInfo) : "",
								g_Config.cvars.esp_box_index ? szIndex : "");
	}
}

void CVisual::DrawBox(bool bPlayer, bool bItem, int iHealth, float boxHeight, float boxWidth, float vecScreenBottom[2], int r, int g, int b)
{
	if ( (bPlayer && (iHealth > 0 || iHealth < -1)) || (!bPlayer && !bItem) )
	{
		int nBox = g_Config.cvars.esp_box;
		bool bOutline = g_Config.cvars.esp_box_outline;

		if ( g_Config.cvars.esp_box_fill != 0 )
		{
			g_Drawing.FillArea(int(vecScreenBottom[0] - (boxWidth * 0.5f)), int(vecScreenBottom[1]), int(boxWidth), int(boxHeight), r, g, b, g_Config.cvars.esp_box_fill);
		}

		if (nBox == 1)
		{
			g_Drawing.Box(int(vecScreenBottom[0] - (boxWidth * 0.5f)), int(vecScreenBottom[1]), int(boxWidth), int(boxHeight), 1, r, g, b, 200);
				
			if (bOutline)
				g_Drawing.BoxOutline(vecScreenBottom[0] - (boxWidth * 0.5f), int(vecScreenBottom[1]), int(boxWidth), int(boxHeight), 1, r, g, b, 200);
		}
		else if (nBox == 2)
		{
			g_Drawing.DrawCoalBox(int(vecScreenBottom[0] - (boxWidth * 0.5f)), int(vecScreenBottom[1]), int(boxWidth), int(boxHeight), 1, r, g, b, 255);

			if (bOutline)
				g_Drawing.DrawOutlineCoalBox(int(vecScreenBottom[0] - (boxWidth * 0.5f)), int(vecScreenBottom[1]), int(boxWidth), int(boxHeight), 1, r, g, b, 255);
		}
		else if (nBox == 3)
		{
			g_Drawing.BoxCorner(int(vecScreenBottom[0] - (boxWidth * 0.5f)), int(vecScreenBottom[1]), int(boxWidth), int(boxHeight), 1, r, g, b, 255);

			if (bOutline)
				g_Drawing.BoxCornerOutline(int(vecScreenBottom[0] - (boxWidth * 0.5f)), int(vecScreenBottom[1]), int(boxWidth), int(boxHeight), 1, r, g, b, 255);
		}
	}
}

void CVisual::DrawBones(int index, studiohdr_t *pStudioHeader)
{
	#ifdef PROCESS_PLAYER_BONES_ONLY
	if ((g_Config.cvars.esp_skeleton || g_Config.cvars.esp_bones_name) && bPlayer)
#else
	if (g_Config.cvars.esp_skeleton || g_Config.cvars.esp_bones_name)
#endif
	{
	#pragma warning(push)
	#pragma warning(disable : 6011)

		mstudiobone_t *pBone = (mstudiobone_t *)((byte *)pStudioHeader + pStudioHeader->boneindex);

		for (int j = 0; j < pStudioHeader->numbones; ++j)
		{
			bool bBonePoint = false;
			float vBonePoint[2];

			if ((bBonePoint = UTIL_WorldToScreen(g_Bones[index].vecPoint[j], vBonePoint)) && g_Config.cvars.esp_bones_name)
				g_Drawing.DrawStringF(g_hFontESP, int(vBonePoint[0]), int(vBonePoint[1]), 255, 255, 255, 255, FONT_ALIGN_CENTER, "%s", pBone[j].name);

			if (g_Config.cvars.esp_skeleton)
			{
				float vParentPoint[2];

				if ( !bBonePoint || g_Bones[index].nParent[j] == -1 )
					continue;

				if ( !UTIL_WorldToScreen(g_Bones[index].vecPoint[g_Bones[index].nParent[j]], vParentPoint) )
					continue;

				g_Drawing.DrawLine(int(vBonePoint[0]), int(vBonePoint[1]), int(vParentPoint[0]), int(vParentPoint[1]), 255, 255, 255, 255);
			}
		}

	#pragma warning(pop)
	}
}

//-----------------------------------------------------------------------------
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

	if (index == nLocalPlayer)
		return;

	if (!pEntity || !(pModel = pEntity->model) || *pModel->name != 'm' || (!pEntity->player && pEntity->curstate.solid <= SOLID_TRIGGER) || pEntity->curstate.movetype == MOVETYPE_NONE)
		return;

	if (pEntity == pViewModel || pEntity->curstate.messagenum < pLocal->curstate.messagenum || pEntity->curstate.maxs.z == 0.0f && pEntity->curstate.mins.z == 0.0f)
		return;

	if (!(pStudioHeader = (studiohdr_t *)g_pEngineStudio->Mod_Extradata(pModel)) || pStudioHeader->numhitboxes == 0)
		return;

	float vecScreenBottom[2], vecScreenTop[2];

	Vector vecBottom = pEntity->origin;
	Vector vecTop = pEntity->origin;

#ifndef PROCESS_PLAYER_BONES_ONLY
	if (pEntity->player)
#endif
		vecBottom.z -= pEntity->curstate.maxs.z;

	vecTop.z += pEntity->curstate.maxs.z;

	bool bScreenBottom = UTIL_WorldToScreen(vecBottom, vecScreenBottom);
	bool bScreenTop = UTIL_WorldToScreen(vecTop, vecScreenTop);

	//memset(g_Bones[index].vecPoint, 0, sizeof(bone_s::vecPoint));
	//memset(g_Bones[index].nParent, -1, sizeof(bone_s::nParent));

	if (bScreenBottom && bScreenTop)
	{
		memset(g_Bones[index].nParent, -1, sizeof(int) * MAXSTUDIOBONES);

		//mstudiobbox_t *pHitbox = (mstudiobbox_t *)((byte *)pStudioHeader + pStudioHeader->hitboxindex);
		mstudiobone_t *pBone = (mstudiobone_t *)((byte *)pStudioHeader + pStudioHeader->boneindex);
		Vector vecFrameVelocity = pEntity->curstate.velocity * (pEntity->curstate.animtime - pEntity->prevstate.animtime);

		Assert( pStudioHeader->numbones < MAXSTUDIOBONES );

		for (int i = 0; i < pStudioHeader->numbones; ++i)
		{
			Vector vecBone = Vector( (*g_pBoneTransform)[i][0][3], (*g_pBoneTransform)[i][1][3], (*g_pBoneTransform)[i][2][3] ) + vecFrameVelocity;

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
	else if (flDrawEntitiesMode == 7.0f)
	{
		cl_entity_s *pEntity = g_pEngineStudio->GetCurrentEntity();

		if ( pEntity->player )
			r_drawentities->value = 3.0f;
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
// Hooked user messages
//-----------------------------------------------------------------------------

int UserMsgHook_ScreenShake(const char *pszUserMsg, int iSize, void *pBuffer)
{
	if (g_Config.cvars.no_shake)
		return 0;

	return ORIG_UserMsgHook_ScreenShake(pszUserMsg, iSize, pBuffer);
}

int UserMsgHook_ScreenFade(const char *pszUserMsg, int iSize, void *pBuffer)
{
	if (g_Config.cvars.no_fade)
		return 0;

	return ORIG_UserMsgHook_ScreenFade(pszUserMsg, iSize, pBuffer);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CVisual::CVisual()
{
	m_flTime = 0.f;

	m_flPrevTime = 0.f;
	m_flFadeTime = g_Config.cvars.jumpspeed_fade_duration;
	m_flJumpSpeed = 0.f;

	m_clFadeFrom[0] = int(255.f * g_Config.cvars.speed_color[0]);
	m_clFadeFrom[1] = int(255.f * g_Config.cvars.speed_color[1]);
	m_clFadeFrom[2] = int(255.f * g_Config.cvars.speed_color[2]);

	m_bOnGround = true;

	m_hUserMsgHook_ScreenShake = 0;
	m_hUserMsgHook_ScreenFade = 0;

	m_iScreenWidth = 1920;
	m_iScreenHeight = 1080;
}

bool CVisual::Load()
{
	r_drawentities = CVar()->FindCvar("r_drawentities");

	if ( !r_drawentities )
	{
		Warning("Can't find cvar \"r_drawentities\"\n");
		return false;
	}

	return true;
}

void CVisual::PostLoad()
{
	g_Drawing.SetupFonts();

	g_pBoneTransform = (bone_matrix3x4_t *)g_pEngineStudio->StudioGetLightTransform();

	m_hUserMsgHook_ScreenShake = Hooks()->HookUserMessage( "ScreenShake", UserMsgHook_ScreenShake, &ORIG_UserMsgHook_ScreenShake );
	m_hUserMsgHook_ScreenFade = Hooks()->HookUserMessage( "ScreenFade", UserMsgHook_ScreenFade, &ORIG_UserMsgHook_ScreenFade );
}

void CVisual::Unload()
{
	Hooks()->UnhookUserMessage( m_hUserMsgHook_ScreenShake );
	Hooks()->UnhookUserMessage( m_hUserMsgHook_ScreenFade );
}