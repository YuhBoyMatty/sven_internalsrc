#include <sys.h>
#include <dbg.h>
#include <convar.h>

#include "menu.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl2.h"

#include "../utils/menu_styles.h"
#include "../config.h"

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

DECLARE_HOOK(BOOL, APIENTRY, wglSwapBuffers, HDC);
DECLARE_HOOK(BOOL, WINAPI, SetCursorPos, int, int);

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

static CCommand s_DummyCommand;

static HWND hGameWnd = NULL;
static WNDPROC hGameWndProc = NULL;

bool g_bMenuEnabled = false;
bool g_bMenuClosed = false;

CMenuModule g_MenuModule;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// Restores window style
void WindowStyle()
{
	ImGuiStyle *style = &ImGui::GetStyle();

	style->FramePadding = ImVec2(6.f, 4.f);
	style->WindowPadding = ImVec2(8.f, 8.f);
	style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style->WindowMenuButtonPosition = ImGuiDir_None;
	style->ItemSpacing = ImVec2(20.f, 5.f);
	style->ItemInnerSpacing = ImVec2(20.f, 20.f);
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 15.0f;
	style->GrabMinSize = 15.0f;
	style->GrabRounding = 7.0f;
}

//-----------------------------------------------------------------------------
// Menu
//-----------------------------------------------------------------------------

void CMenuModule::Draw()
{
	if ( !m_bThemeLoaded )
	{
		LoadMenuTheme();
		WindowStyle();

		m_bThemeLoaded = true;
	}

	ImGui::GetIO().MouseDrawCursor = g_bMenuEnabled;
	ImGui::GetStyle().Alpha = g_Config.cvars.menu_opacity;

	if ( g_bMenuEnabled )
	{
		// Main Window
		ImGui::SetNextWindowSize(ImVec2(250.0f, 316.0f), ImGuiCond_FirstUseEver);

		if (g_Config.cvars.menu_auto_resize)
		{
			ImGui::Begin("Sven Internal", &g_bMenuEnabled, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImGui::SetWindowSize(ImVec2(250.0f, 316.0f));
		}
		else
		{
			ImGui::Begin("Sven Internal", &g_bMenuEnabled, ImGuiWindowFlags_NoCollapse);
	    }

		{
			SYSTEMTIME SysTime;
			GetLocalTime(&SysTime);

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2.f) - (42.5f));
			ImGui::Text("Time: %02d:%02d:%02d", SysTime.wHour, SysTime.wMinute, SysTime.wSecond);

			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (84));

			ImVec4 vColor;

			if (ImGui::GetIO().Framerate < 30.f)
			{
				vColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			}
			else
			{
				vColor = ImVec4(0.0f, 1.0f, 0.1f, 1.0f);
			}

			ImGui::TextColored(vColor, "%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (16));
			ImGui::Text("General");

			ImGui::Spacing();

			// Main Buttons
			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("Aim", ImVec2(149, 28)))
			{
				m_bMenuAim ^= true;
			}

			ImGui::Spacing();
			
			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("Visuals", ImVec2(149, 28)))
			{
				m_bMenuVisuals ^= true;
			}

			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("HUD", ImVec2(149, 28)))
			{
				m_bMenuHud ^= true;
			}

			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("Utility", ImVec2(149, 28)))
			{
				m_bMenuUtility ^= true;
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("Config", ImVec2(149, 28)))
			{
				m_bMenuConfig ^= true;
			}

			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("Settings", ImVec2(149, 28)))
			{
				m_bMenuSettings ^= true;
			}
		}
		ImGui::End();

		DrawWindowAim();
		DrawWindowVisuals();
		DrawWindowHUD();
		DrawWindowUtility();
		DrawWindowConfig();
		DrawWindowSettings();
	}
}

void CMenuModule::DrawWindowAim()
{
	// Aim
	if (m_bMenuAim)
	{
		ImGui::SetNextWindowSize(ImVec2(500.0f, 600.0f), ImGuiCond_FirstUseEver);
		if (g_Config.cvars.menu_auto_resize)
		ImGui::Begin("Aim", &m_bMenuAim, ImGuiWindowFlags_AlwaysAutoResize);
		else
		ImGui::Begin("Aim", &m_bMenuAim);
		{
			if (ImGui::BeginTabBar("##tabs"))
			{
				// Recoil & Spread
				if (ImGui::BeginTabItem("Recoil & Spread"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Recoil & Spread");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("No Recoil");

					ImGui::Spacing();
					ImGui::Spacing();

					extern ConVar sc_no_recoil;
					extern ConVar sc_no_recoil_visual;

					if (ImGui::Checkbox("No Recoil", &g_Config.cvars.no_recoil))
					{
						sc_no_recoil.SetValue( !sc_no_recoil.GetBool() );
					}
					
					if (ImGui::Checkbox("No Recoil [Visual]", &g_Config.cvars.no_recoil_visual))
					{
						sc_no_recoil_visual.SetValue( !sc_no_recoil_visual.GetBool() );
					}

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}
			}
		}
	}
}

void CMenuModule::DrawWindowVisuals()
{
	// Visuals
	if (m_bMenuVisuals)
	{
		ImGui::SetNextWindowSize(ImVec2(527.0f, 600.0f), ImGuiCond_FirstUseEver);
		if (g_Config.cvars.menu_auto_resize)
		ImGui::Begin("Visuals", &m_bMenuVisuals, ImGuiWindowFlags_AlwaysAutoResize);
		else
		ImGui::Begin("Visuals", &m_bMenuVisuals);
		{
			if (ImGui::BeginTabBar("##tabs"))
			{
				// Render
				if (ImGui::BeginTabItem("Render"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Render");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Game");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("No Shake", &g_Config.cvars.no_shake); ImGui::SameLine();
					ImGui::Checkbox("No Fade", &g_Config.cvars.no_fade); ImGui::SameLine();
					ImGui::Checkbox("Remove FOV Cap", &g_Config.cvars.remove_fov_cap);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Draw Entities");

					ImGui::Spacing();
					ImGui::Spacing();

					static const char* draw_entities_items[] =
					{
						"0 - Default",
						"1 - Draw Bones",
						"2 - Draw Hitboxes",
						"3 - Draw Model & Hitboxes",
						"4 - Draw Hulls",
						"5 - Draw Players Bones",
						"6 - Draw Players Hitboxes"
					};

					ImGui::Combo(" ", &g_Config.cvars.draw_entities, draw_entities_items, IM_ARRAYSIZE(draw_entities_items));

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Light Map");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Override Lightmap", &g_Config.cvars.lightmap_override);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Lightmap Brightness", &g_Config.cvars.lightmap_brightness, 0.0f, 1.0f);

					ImGui::Spacing();

					ImGui::ColorEdit3("Lightmap Color", g_Config.cvars.lightmap_color);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("No Weapon Animations");

					ImGui::Spacing();
					ImGui::Spacing();

					static const char* no_weap_anim_items[] = { "0 - Off", "1 - All Animations", "2 - Take Animations" };

					ImGui::Combo("##no_weap_anims", &g_Config.cvars.no_weapon_anim, no_weap_anim_items, IM_ARRAYSIZE(no_weap_anim_items));

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// ESP
				if (ImGui::BeginTabItem("ESP"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("ESP");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable ESP", &g_Config.cvars.esp);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Outline Box", &g_Config.cvars.esp_box_outline); ImGui::SameLine();
					ImGui::Checkbox("Show Items", &g_Config.cvars.esp_show_items); ImGui::SameLine();
					ImGui::Checkbox("Ignore Unknown Entities", &g_Config.cvars.esp_ignore_unknown_ents);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Draw Entity Index", &g_Config.cvars.esp_box_index); ImGui::SameLine();
					ImGui::Text(" "); ImGui::SameLine();
					ImGui::Checkbox("Draw Distance", &g_Config.cvars.esp_box_distance);

					ImGui::Spacing();

					ImGui::Checkbox("Draw Player Health", &g_Config.cvars.esp_box_player_health); ImGui::SameLine();
					ImGui::Text(""); ImGui::SameLine();
					ImGui::Checkbox("Draw Player Armor", &g_Config.cvars.esp_box_player_armor);

					ImGui::Spacing();

					ImGui::Checkbox("Draw Entity Name", &g_Config.cvars.esp_box_entity_name); ImGui::SameLine();
					ImGui::Text("  "); ImGui::SameLine();
					ImGui::Checkbox("Draw Nicknames", &g_Config.cvars.esp_box_player_name);

					ImGui::Spacing();

					ImGui::Checkbox("Draw Skeleton", &g_Config.cvars.esp_skeleton); ImGui::SameLine();
					ImGui::Text("     "); ImGui::SameLine();
					ImGui::Checkbox("Draw Bones Name", &g_Config.cvars.esp_bones_name);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("ESP Colors");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Friend Color", g_Config.cvars.esp_friend_color);

					ImGui::Spacing();

					ImGui::ColorEdit3("Enemy Color", g_Config.cvars.esp_enemy_color);

					ImGui::Spacing();

					ImGui::ColorEdit3("Neutral Color", g_Config.cvars.esp_neutral_color);

					ImGui::Spacing();

					ImGui::ColorEdit3("Item Color", g_Config.cvars.esp_item_color);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					static const char* esp_process_items[] = { "0 - Everyone", "1 - Entities", "2 - Players" };
					ImGui::Combo("ESP Targets", &g_Config.cvars.esp_targets, esp_process_items, IM_ARRAYSIZE(esp_process_items));

					ImGui::Spacing();

					ImGui::Combo("Draw Skeleton Type", &g_Config.cvars.esp_skeleton_type, esp_process_items, IM_ARRAYSIZE(esp_process_items));

					ImGui::Spacing();

					static const char* esp_box_items[] = { "0 - Off", "1 - Default", "2 - Coal", "3 - Corner" };
					ImGui::Combo("Box Type", &g_Config.cvars.esp_box, esp_box_items, IM_ARRAYSIZE(esp_box_items));

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("ESP Distance", &g_Config.cvars.esp_distance, 1.0f, 10000.0f);
						
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("Box Alpha", &g_Config.cvars.esp_box_fill, 0, 255);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Chams
				if (ImGui::BeginTabItem("Chams"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Chams");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Chams", &g_Config.cvars.chams);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Players");

					ImGui::Spacing();
					ImGui::Spacing();

					static const char* chams_items[] = { "0 - Disable", "1 - Flat", "2 - Texture", "3 - Material" };
					ImGui::Checkbox("Chams Players Behind Wall", &g_Config.cvars.chams_players_wall);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("Chams Players", &g_Config.cvars.chams_players, 0, 3, chams_items[g_Config.cvars.chams_players]);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Chams Players Color", g_Config.cvars.chams_players_color);

					ImGui::Spacing();

					ImGui::ColorEdit3("Chams Players Wall Color", g_Config.cvars.chams_players_wall_color);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Entities");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Chams Entities Behind Wall", &g_Config.cvars.chams_entities_wall);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("Chams Entities", &g_Config.cvars.chams_entities, 0, 3, chams_items[g_Config.cvars.chams_entities]);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Chams Entities Color", g_Config.cvars.chams_entities_color);

					ImGui::Spacing();

					ImGui::ColorEdit3("Chams Entities Wall Color", g_Config.cvars.chams_entities_wall_color);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Items");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Chams Items Behind Wall", &g_Config.cvars.chams_items_wall);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("Chams Items", &g_Config.cvars.chams_items, 0, 3, chams_items[g_Config.cvars.chams_items]);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Chams Items Color", g_Config.cvars.chams_items_color);

					ImGui::Spacing();

					ImGui::ColorEdit3("Chams Items Wall Color", g_Config.cvars.chams_items_wall_color);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Glow
				if (ImGui::BeginTabItem("Glow"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Glow");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Glow", &g_Config.cvars.glow);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Optimize Glow Behind Wall", &g_Config.cvars.glow_optimize);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Players");

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					static const char* glow_items[] = { "0 - Disable", "1 - Glow Outline", "2 - Glow Shell", "3 - Ghost" };
					ImGui::Checkbox("Glow Players Behind Wall", &g_Config.cvars.glow_players_wall);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("Glow Players", &g_Config.cvars.glow_players, 0, 3, glow_items[g_Config.cvars.glow_players]);

					ImGui::Spacing();

					ImGui::SliderInt("Glow Players Width", &g_Config.cvars.glow_players_width, 0, 30);

					ImGui::Spacing();

					ImGui::ColorEdit3("Glow Players Color", g_Config.cvars.glow_players_color);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Entities");

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Glow Entities Behind Wall", &g_Config.cvars.glow_entities_wall);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("Glow Entities", &g_Config.cvars.glow_entities, 0, 3, glow_items[g_Config.cvars.glow_entities]);

					ImGui::Spacing();

					ImGui::SliderInt("Glow Entities Width", &g_Config.cvars.glow_entities_width, 0, 30);

					ImGui::Spacing();

					ImGui::ColorEdit3("Glow Entities Color", g_Config.cvars.glow_entities_color);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Items");

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Glow Items Behind Wall", &g_Config.cvars.glow_items_wall);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("Glow Items", &g_Config.cvars.glow_items, 0, 3, glow_items[g_Config.cvars.glow_items]);

					ImGui::Spacing();

					ImGui::SliderInt("Glow Items Width", &g_Config.cvars.glow_items_width, 0, 30);

					ImGui::Spacing();

					ImGui::ColorEdit3("Glow Items Color", g_Config.cvars.glow_items_color);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Dynamic Glow
				if (ImGui::BeginTabItem("Dynamic Glow"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Dynamic Glow");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Dyn. Glow Attach To Targets", &g_Config.cvars.dyn_glow_attach);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Self");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Dyn. Glow Self", &g_Config.cvars.dyn_glow_self);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Dyn. Glow Self Radius", &g_Config.cvars.dyn_glow_self_radius, 0.f, 4096.f);

					ImGui::Spacing();

					ImGui::SliderFloat("Dyn. Glow Self Decay", &g_Config.cvars.dyn_glow_self_decay, 0.f, 4096.f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Dyn. Glow Self Color", g_Config.cvars.dyn_glow_self_color);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Players");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Dyn. Glow Players", &g_Config.cvars.dyn_glow_players);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Dyn. Glow Players Radius", &g_Config.cvars.dyn_glow_players_radius, 0.f, 4096.f);

					ImGui::Spacing();

					ImGui::SliderFloat("Dyn. Glow Players Decay", &g_Config.cvars.dyn_glow_players_decay, 0.f, 4096.f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Dyn. Glow Players Color", g_Config.cvars.dyn_glow_players_color);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Entities");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Dyn. Glow Entities", &g_Config.cvars.dyn_glow_entities);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Dyn. Glow Entities Radius", &g_Config.cvars.dyn_glow_entities_radius, 0.f, 4096.f);

					ImGui::Spacing();

					ImGui::SliderFloat("Dyn. Glow Entities Decay", &g_Config.cvars.dyn_glow_entities_decay, 0.f, 4096.f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Dyn. Glow Entities Color", g_Config.cvars.dyn_glow_entities_color);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Items");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Dyn. Glow Items", &g_Config.cvars.dyn_glow_items);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Dyn. Glow Items Radius", &g_Config.cvars.dyn_glow_items_radius, 0.f, 4096.f);

					ImGui::Spacing();

					ImGui::SliderFloat("Dyn. Glow Items Decay", &g_Config.cvars.dyn_glow_items_decay, 0.f, 4096.f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Dyn. Glow Items Color", g_Config.cvars.dyn_glow_items_color);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Wallhack
				if (ImGui::BeginTabItem("Wallhack"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Wallhack");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Simple Wallhack", &g_Config.cvars.wallhack); ImGui::SameLine();
					ImGui::Checkbox("Lambert Wallhack", &g_Config.cvars.wallhack_white_walls);

					ImGui::Spacing();

					ImGui::Checkbox("Wireframe World", &g_Config.cvars.wallhack_wireframe); ImGui::SameLine();
					ImGui::Checkbox("Wireframe Models", &g_Config.cvars.wallhack_wireframe_models);

					ImGui::Spacing();

					ImGui::Checkbox("Negative", &g_Config.cvars.wallhack_negative);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Wireframe Line Width", &g_Config.cvars.wh_wireframe_width, 0.0f, 10.0f);

					ImGui::Spacing();

					ImGui::ColorEdit3("Wireframe Color", g_Config.cvars.wh_wireframe_color);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Misc (Skybox, Fog)
				if (ImGui::BeginTabItem("Misc."))
				{
					extern void ConCommand_ChangeSkybox(const CCommand &args);
					extern void ConCommand_ResetSkybox();

					extern const char* g_szSkyboxes[];
					extern int g_iSkyboxesSize;
					extern bool g_bMenuChangeSkybox;

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Skybox Changer");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Combo("Skybox Name", &g_Config.cvars.skybox, g_szSkyboxes, g_iSkyboxesSize))
					{
						g_bMenuChangeSkybox = true;

						ConCommand_ChangeSkybox(s_DummyCommand);

						g_bMenuChangeSkybox = false;
					}

					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Button("Reset Skybox"))
					{
						ConCommand_ResetSkybox();
					}
						
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Player's Push Direction");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();
						
					ImGui::Checkbox("Show Player's Push Direction", &g_Config.cvars.show_players_push_direction);

					ImGui::Spacing();

					ImGui::SliderFloat("Push Direction Length", &g_Config.cvars.push_direction_length, 0.0f, 256.0f);

					ImGui::Spacing();

					ImGui::SliderFloat("Push Direction Width", &g_Config.cvars.push_direction_width, 0.01f, 100.0f);

					ImGui::Spacing();

					ImGui::ColorEdit3("Push Direction Color", g_Config.cvars.push_direction_color);
						
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Custom HUD Color");

					ImGui::Spacing();

					ImGui::Separator();
						
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Remap HUD Color", &g_Config.cvars.remap_hud_color);

					ImGui::Spacing();

					ImGui::ColorEdit3("HUD Color", g_Config.cvars.hud_color);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Fog");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Fog", &g_Config.cvars.fog);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Fog Skybox", &g_Config.cvars.fog_skybox);

					ImGui::Spacing();

					ImGui::Checkbox("Disable Water Fog", &g_Config.cvars.remove_water_fog);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Fog Start", &g_Config.cvars.fog_start, 0.0f, 10000.0f);

					ImGui::Spacing();

					ImGui::SliderFloat("Fog End", &g_Config.cvars.fog_end, 0.0f, 10000.0f);

					ImGui::Spacing();

					ImGui::SliderFloat("Density", &g_Config.cvars.fog_density, 0.0f, 10.0f);

					ImGui::Spacing();

					ImGui::ColorEdit3("Color", g_Config.cvars.fog_color);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}
			}
		}
	}
}

void CMenuModule::DrawWindowHUD()
{
	// HUD 
	if (m_bMenuHud)
	{
		ImGui::SetNextWindowSize(ImVec2(463.0f, 510.0f), ImGuiCond_FirstUseEver);
		if (g_Config.cvars.menu_auto_resize)
			ImGui::Begin("HUD", &m_bMenuHud, ImGuiWindowFlags_AlwaysAutoResize);
		else
			ImGui::Begin("HUD", &m_bMenuHud);
		{
			if (ImGui::BeginTabBar("##tabs"))
			{
				// Speedometer
				if (ImGui::BeginTabItem("Speedometer"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Speedometer");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Button("Reset Color of Speedometer"))
					{
						g_Config.cvars.speed_color[0] = 100.f / 255.f;
						g_Config.cvars.speed_color[1] = 130.f / 255.f;
						g_Config.cvars.speed_color[2] = 200.f / 255.f;
					}

					ImGui::Spacing();
					ImGui::Spacing();
						
					ImGui::Checkbox("Show Speedometer", &g_Config.cvars.show_speed);

					ImGui::Spacing();

					ImGui::Checkbox("Show Jump's Speed", &g_Config.cvars.show_jumpspeed);

					ImGui::Spacing();

					ImGui::Checkbox("Store Vertical Speed", &g_Config.cvars.show_vertical_speed);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Jump's Speed: Fade Duration", &g_Config.cvars.jumpspeed_fade_duration, 0.1f, 2.0f);
						
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Speed Width Fraction", &g_Config.cvars.speed_width_fraction, 0.0f, 1.0f);

					ImGui::Spacing();

					ImGui::SliderFloat("Speed Height Fraction", &g_Config.cvars.speed_height_fraction, 0.0f, 1.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Speed Color", g_Config.cvars.speed_color);

					ImGui::Spacing();
					ImGui::Spacing();
						
					ImGui::Text("Legacy Speedometer");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Show Speedometer (Legacy)", &g_Config.cvars.show_speed_legacy);

					ImGui::Spacing();

					ImGui::Checkbox("Store Vertical Speed (Legacy)", &g_Config.cvars.show_vertical_speed_legacy);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Speed Width Fraction (Legacy)", &g_Config.cvars.speed_width_fraction_legacy, 0.0f, 1.0f);

					ImGui::Spacing();

					ImGui::SliderFloat("Speed Height Fraction (Legacy)", &g_Config.cvars.speed_height_fraction_legacy, 0.0f, 1.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit4("Speed Color (Legacy)", g_Config.cvars.speed_color_legacy);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Crosshair
				if (ImGui::BeginTabItem("Crosshair"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Crosshair");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Draw Crosshair", &g_Config.cvars.draw_crosshair);

					ImGui::Spacing();
						
					ImGui::Checkbox("Draw Crosshair Dot", &g_Config.cvars.draw_crosshair_dot);

					ImGui::Spacing();

					ImGui::Checkbox("Draw Crosshair Outline", &g_Config.cvars.draw_crosshair_outline);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("Crosshair Size", &g_Config.cvars.crosshair_size, 1, 50);
						
					ImGui::Spacing();

					ImGui::SliderInt("Crosshair Gap", &g_Config.cvars.crosshair_gap, 0, 50);
						
					ImGui::Spacing();

					ImGui::SliderInt("Crosshair Thickness", &g_Config.cvars.crosshair_thickness, 1, 50);
						
					ImGui::Spacing();

					ImGui::SliderInt("Crosshair Outline Thickness", &g_Config.cvars.crosshair_outline_thickness, 1, 50);
						
					ImGui::Spacing();
					ImGui::Spacing();
						
					ImGui::ColorEdit4("Crosshair Color", g_Config.cvars.crosshair_color);
						
					ImGui::Spacing();

					ImGui::ColorEdit4("Crosshair Outline Color", g_Config.cvars.crosshair_outline_color);
						
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Chat Colors
				if (ImGui::BeginTabItem("Chat Colors"))
				{
					extern void ConCommand_ChatColorsLoadPlayers();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Chat Colors");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Chat Colors", &g_Config.cvars.enable_chat_colors);

					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Button("Load Players List"))
					{
						ConCommand_ChatColorsLoadPlayers();
					}

					ImGui::Spacing();
						
					if (ImGui::Button("Reset Default Player Color"))
					{
						g_Config.cvars.player_name_color[0] = 0.6f;
						g_Config.cvars.player_name_color[1] = 0.75f;
						g_Config.cvars.player_name_color[2] = 1.0f;
					}

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Default Player Color", g_Config.cvars.player_name_color);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Rainbow Names");
						
					ImGui::Spacing();
					ImGui::Spacing();
						
					ImGui::SliderFloat("Rainbow Update Delay", &g_Config.cvars.chat_rainbow_update_delay, 0.0f, 0.5f);

					ImGui::Spacing();

					ImGui::SliderFloat("Rainbow Hue Delta", &g_Config.cvars.chat_rainbow_hue_delta, 0.0f, 0.5f);

					ImGui::Spacing();

					ImGui::SliderFloat("Rainbow Saturation", &g_Config.cvars.chat_rainbow_saturation, 0.0f, 1.0f);

					ImGui::Spacing();

					ImGui::SliderFloat("Rainbow Lightness", &g_Config.cvars.chat_rainbow_lightness, 0.0f, 1.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Custom Colors");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::ColorEdit3("Custom Color #1", g_Config.cvars.chat_color_one);

					ImGui::Spacing();

					ImGui::ColorEdit3("Custom Color #2", g_Config.cvars.chat_color_two);

					ImGui::Spacing();

					ImGui::ColorEdit3("Custom Color #3", g_Config.cvars.chat_color_three);

					ImGui::Spacing();

					ImGui::ColorEdit3("Custom Color #4", g_Config.cvars.chat_color_four);

					ImGui::Spacing();

					ImGui::ColorEdit3("Custom Color #5", g_Config.cvars.chat_color_five);

					ImGui::Spacing();

					ImGui::ColorEdit3("Custom Color #6", g_Config.cvars.chat_color_six);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Custom Vote Popup
				if (ImGui::BeginTabItem("Custom Vote Popup"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Custom Vote Popup");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Custom Vote Popup", &g_Config.cvars.vote_popup);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("VP: Width Size", &g_Config.cvars.vote_popup_width_size, 0, 1000);

					ImGui::Spacing();

					ImGui::SliderInt("VP: Height Size", &g_Config.cvars.vote_popup_height_size, 0, 1000);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("VP: Width Border Pixels", &g_Config.cvars.vote_popup_w_border_pix, 0, 100);

					ImGui::Spacing();

					ImGui::SliderInt("VP: Height Border Pixels", &g_Config.cvars.vote_popup_h_border_pix, 0, 100);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("VP: Width Fraction", &g_Config.cvars.vote_popup_width_frac, 0.0f, 1.0f);

					ImGui::Spacing();

					ImGui::SliderFloat("VP: Height Fraction", &g_Config.cvars.vote_popup_height_frac, 0.0f, 1.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}
			}
		}
	}
}

void CMenuModule::DrawWindowUtility()
{
	// Utility
	if (m_bMenuUtility)
	{
		ImGui::SetNextWindowSize(ImVec2(690.0f, 500.0f), ImGuiCond_FirstUseEver);
		if (g_Config.cvars.menu_auto_resize)
			ImGui::Begin("Utility", &m_bMenuUtility, ImGuiWindowFlags_AlwaysAutoResize);
		else
			ImGui::Begin("Utility", &m_bMenuUtility);
		{
			if (ImGui::BeginTabBar("##tabs"))
			{
				// Player
				if (ImGui::BeginTabItem("Player"))
				{
					extern void ConCommand_AutoSelfSink();
					extern void ConCommand_Freeze();
					extern void ConCommand_Freeze2();
					extern void ConCommand_DropEmptyWeapon();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Player");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Button("Selfsink"))
						ConCommand_AutoSelfSink();

					ImGui::Spacing();

					if (ImGui::Button("Freeze"))
						ConCommand_Freeze();

					ImGui::Spacing();
						
					if (ImGui::Button("Freeze #2"))
						ConCommand_Freeze2();

					ImGui::Spacing();

					if (ImGui::Button("Drop Empty Weapon"))
						ConCommand_DropEmptyWeapon();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Autojump", &g_Config.cvars.autojump);

					ImGui::Spacing();

					ImGui::Checkbox("Jumpbug", &g_Config.cvars.jumpbug);

					ImGui::Spacing();
						
					ImGui::Checkbox("Edgejump", &g_Config.cvars.edgejump);

					ImGui::Spacing();

					ImGui::Checkbox("Doubleduck", &g_Config.cvars.doubleduck);

					ImGui::Spacing();

					ImGui::Checkbox("Fastrun", &g_Config.cvars.fastrun);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Auto Ceil-Clipping", &g_Config.cvars.auto_ceil_clipping); ImGui::SameLine();
					ImGui::Checkbox("Tertiary Attack Glitch", &g_Config.cvars.tertiary_attack_glitch);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Rotate Dead Body", &g_Config.cvars.rotate_dead_body); ImGui::SameLine();
					ImGui::Checkbox("Quake Guns", &g_Config.cvars.quake_guns);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Lock View Angles");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Lock Pitch", &g_Config.cvars.lock_pitch);

					ImGui::Spacing();

					ImGui::SliderFloat("Lock Pitch: Angle", &g_Config.cvars.lock_pitch_angle, -179.999f, 180.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Lock Yaw", &g_Config.cvars.lock_yaw);

					ImGui::Spacing();

					ImGui::SliderFloat("Lock Yaw: Angle", &g_Config.cvars.lock_yaw_angle, 0.0f, 360.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Spinner");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Inclined Rotation", &g_Config.cvars.spin_pitch_angle);

					ImGui::Spacing();

					ImGui::SliderFloat("Inclined Rotation: Angle", &g_Config.cvars.spin_pitch_rotation_angle, -10.0f, 10.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Spin Yaw", &g_Config.cvars.spin_yaw_angle);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Spin Yaw: Angle", &g_Config.cvars.spin_yaw_rotation_angle, -10.0f, 10.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Strafer
				if (ImGui::BeginTabItem("Strafer"))
				{
					static const char* strafe_dir_items[] = { "0 - To the left", "1 - To the right", "2 - Best strafe", "3 - View angles" };
					static const char* strafe_type_items[] = { "0 - Max. acceleration", "1 - Max. angle", "2 - Max. deceleration", "3 - Const speed" };

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Strafer");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Strafer", &g_Config.cvars.strafe);

					ImGui::Spacing();

					ImGui::Checkbox("Ignore Ground", &g_Config.cvars.strafe_ignore_ground);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Combo("Strafe Direction", &g_Config.cvars.strafe_dir, strafe_dir_items, IM_ARRAYSIZE(strafe_dir_items));

					ImGui::Spacing();

					ImGui::Combo("Strafe Type", &g_Config.cvars.strafe_type, strafe_type_items, IM_ARRAYSIZE(strafe_type_items));

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Color Pulsator
				if (ImGui::BeginTabItem("Color Pulsator"))
				{
					extern void ConCommand_ResetColors();
					extern void ConCommand_SyncColors();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Color Pulsator");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Pulsator", &g_Config.cvars.color_pulsator);

					ImGui::Spacing();

					ImGui::Checkbox("Change Top Color", &g_Config.cvars.color_pulsator_top);

					ImGui::Spacing();

					ImGui::Checkbox("Change Bottom Color", &g_Config.cvars.color_pulsator_bottom);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Change Color Delay");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("  ", &g_Config.cvars.color_pulsator_delay, 0.1f, 2.5f);

					ImGui::Spacing();

					if (ImGui::Button("Reset Colors"))
						ConCommand_ResetColors();

					ImGui::SameLine();

					if (ImGui::Button("Sync. Colors"))
						ConCommand_SyncColors();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Fake Lag
				if (ImGui::BeginTabItem("Fake Lag"))
				{
					static const char* fakelag_type_items[] = { "0 - Dynamic", "1 - Maximum", "2 - Jitter", "3 - Break Lag Compensation" };
					static const char* fakelag_move_items[] = { "0 - Everytime", "1 - On Land", "2 - On Move", "3 - In Air" };

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Fake Lag");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Fake Lag", &g_Config.cvars.fakelag);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Adaptive Ex Interp", &g_Config.cvars.fakelag_adaptive_ex_interp);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderInt("Limit", &g_Config.cvars.fakelag_limit, 0, 256);

					ImGui::Spacing();

					ImGui::SliderFloat("Variance", &g_Config.cvars.fakelag_variance, 0.0f, 100.0f);

					ImGui::Spacing();

					ImGui::Combo("Fake Lag Type", &g_Config.cvars.fakelag_type, fakelag_type_items, IM_ARRAYSIZE(fakelag_type_items));

					ImGui::Spacing();

					ImGui::Combo("Fake Move Type", &g_Config.cvars.fakelag_move, fakelag_move_items, IM_ARRAYSIZE(fakelag_move_items));

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

                // Anti-AFK
				if (ImGui::BeginTabItem("Anti-AFK"))
				{
					static const char* antiafk_items[] =
					{
						"0 - Off",
						"1 - Step Forward & Back",
						"2 - Spam Gibme",
						"3 - Spam Kill",
						"4 - Walk Around & Spam Inputs",
						"5 - Walk Around",
						"6 - Go Right"
					};

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Anti-AFK");

					ImGui::Spacing(); 

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Combo("Mode", &g_Config.cvars.antiafk, antiafk_items, IM_ARRAYSIZE(antiafk_items));

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Anti-AFK Rotate Camera", &g_Config.cvars.antiafk_rotate_camera);

					ImGui::Spacing();

					ImGui::Checkbox("Anti-AFK Stay Within Range", &g_Config.cvars.antiafk_stay_within_range);
						
					ImGui::Spacing();

					ImGui::Checkbox("Anti-AFK Reset Stay Position", &g_Config.cvars.antiafk_reset_stay_pos);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("Rotation Angle", &g_Config.cvars.antiafk_rotation_angle, -7.0f, 7.0f);

					ImGui::Spacing();

					ImGui::SliderFloat("Stay Within Radius", &g_Config.cvars.antiafk_stay_radius, 10.0f, 500.0f);

					ImGui::Spacing();

					ImGui::SliderFloat("Stay Within Spread Angle", &g_Config.cvars.antiafk_stay_radius_offset_angle, 0.0f, 89.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Spammer
				if (ImGui::BeginTabItem("Spammer"))
				{
					extern void ConCommand_PrintSpamKeyWords(void);
					extern void ConCommand_PrintSpamTasks(void);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Key Spammer");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Hold Mode", &g_Config.cvars.keyspam_hold_mode);

					ImGui::Spacing();

					ImGui::Checkbox("Spam E", &g_Config.cvars.keyspam_e); ImGui::SameLine();
					ImGui::Checkbox("Spam Q", &g_Config.cvars.keyspam_q);

					ImGui::Spacing();

					ImGui::Checkbox("Spam W", &g_Config.cvars.keyspam_w); ImGui::SameLine();
					ImGui::Checkbox("Spam S", &g_Config.cvars.keyspam_s);

					ImGui::Spacing();

					ImGui::Checkbox("Spam CTRL", &g_Config.cvars.keyspam_ctrl);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Message Spammer");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Button("Show Spam Tasks"))
						ConCommand_PrintSpamTasks();

					ImGui::Spacing();

					if (ImGui::Button("Show Spam Keywords"))
						ConCommand_PrintSpamKeyWords();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Camera
				if (ImGui::BeginTabItem("Camera"))
				{
					extern void ConCommand_CamHack(void);
					extern void ConCommand_CamHackResetRoll(void);
					extern void ConCommand_CamHackReset(void);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Cam Hack");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Button("Toggle Cam Hack"))
						ConCommand_CamHack();

					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Button("Reset Roll Axis"))
						ConCommand_CamHackResetRoll();

					ImGui::Spacing();

					if (ImGui::Button("Reset Cam Hack"))
						ConCommand_CamHackReset();

					ImGui::Spacing();
					ImGui::Spacing();
							
					ImGui::Text("Speed Factor"); 
						
					ImGui::Spacing();

					ImGui::SliderFloat("CamHack: Speed Factor", &g_Config.cvars.camhack_speed_factor, 0.0f, 15.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Hide HUD", &g_Config.cvars.camhack_hide_hud);
						
					ImGui::Spacing();

					ImGui::Checkbox("Show Model", &g_Config.cvars.camhack_show_model);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("First-Person Roaming");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Enable First-Person Roaming", &g_Config.cvars.fp_roaming);

					ImGui::Spacing();

					ImGui::Checkbox("Draw Crosshair in Roaming", &g_Config.cvars.fp_roaming_draw_crosshair);

					ImGui::Spacing();

					ImGui::Checkbox("Lerp First-Person View", &g_Config.cvars.fp_roaming_lerp);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Lerp Value");

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::SliderFloat("FP Roaming: Lerp Value", &g_Config.cvars.fp_roaming_lerp_value, 0.001f, 1.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}

				// Misc (One Tick Exploit, Application Speed)
				if (ImGui::BeginTabItem("Misc."))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Other");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();
						
					ImGui::Checkbox("Ignore Different Map Versions", &g_Config.cvars.ignore_different_map_versions);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("One Tick Exploit");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("One Tick Exploit", &g_Config.cvars.one_tick_exploit);

					ImGui::Spacing();

					ImGui::Text("Lag Interval");
					ImGui::SliderInt("##one_tick_exploit_lag_interval", &g_Config.cvars.one_tick_exploit_lag_interval, 1, 256);
						
					ImGui::Text("Speedhack");
					ImGui::SliderFloat("##one_tick_exploit_speedhack", &g_Config.cvars.one_tick_exploit_speedhack, 0.01f, 1000000000.0f);
						
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Checkbox("Fast Crowbar", &g_Config.cvars.fast_crowbar);
					ImGui::Checkbox("Fast Crowbar [Auto Freeze]", &g_Config.cvars.fast_crowbar2);
					ImGui::Checkbox("Fast Medkit", &g_Config.cvars.fast_medkit);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Text("Application Speed");

					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Button("Reset App Speed"))
					{
						g_Config.cvars.application_speed = 1.0f;
					}

					ImGui::Spacing();

					ImGui::SliderFloat("Application Speed", &g_Config.cvars.application_speed, 0.1f, 50.0f);

					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::Separator();

					ImGui::Text("");
					ImGui::Spacing();

					ImGui::EndTabItem();
				}
			}
		}
	}
}

void CMenuModule::DrawWindowConfig()
{
	// Config
	if (m_bMenuConfig)
	{
		ImGui::SetNextWindowSize(ImVec2(300.0f, 250.0f), ImGuiCond_FirstUseEver);
		if (g_Config.cvars.menu_auto_resize)
		{
			ImGui::Begin("Config", &m_bMenuConfig, ImGuiWindowFlags_NoResize);
			ImGui::SetWindowSize(ImVec2(300.0f, 250.0f));
		}
		else
		{
			ImGui::Begin("Config", &m_bMenuConfig);
		}

		ImGui::Spacing();

		ImGui::Text("List of Configs");

		if (ImGui::BeginListBox("##configs_list", ImVec2(-FLT_MIN, 8 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (size_t i = 0; i < g_Config.configs.size(); i++)
			{
				bool bSelected = (g_Config.current_config.compare( g_Config.configs[i] ) == 0);

				if (ImGui::Selectable(g_Config.configs[i].c_str(), bSelected))
					g_Config.current_config = g_Config.configs[i];

				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndListBox();
		}

		if (ImGui::Button("Load"))
		{
			g_Config.Load();

			LoadMenuTheme();
			WindowStyle();
		}

		ImGui::SameLine();

		if (ImGui::Button("Save"))
			g_Config.Save();

		ImGui::SameLine();
			
		if (ImGui::Button("New"))
			g_Config.New();
			
		ImGui::SameLine();
			
		if (ImGui::Button("Delete"))
			g_Config.Remove();

		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::End();
	}
}

void CMenuModule::DrawWindowSettings()
{
	// Settings
	if (m_bMenuSettings)
	{
		ImGui::SetNextWindowSize(ImVec2(300.0f, 390.0f), ImGuiCond_FirstUseEver);
		if (g_Config.cvars.menu_auto_resize)
		{
			ImGui::Begin("Settings", &m_bMenuSettings, ImGuiWindowFlags_NoResize);
			ImGui::SetWindowSize(ImVec2(300.0f, 390.0f));
		}
		else
		{
			ImGui::Begin("Settings", &m_bMenuSettings);
		}

		{
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (60 / 2));
			ImGui::Text("Toggle Key");

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2.f) - (83.5f));
			if (ImGui::Button("Use Insert"))
				g_Config.cvars.toggle_button = 0x2D;

			ImGui::SameLine();

			if (ImGui::Button("Use Delete"))
				g_Config.cvars.toggle_button = 0x2E;

			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (69));
			if (ImGui::Button("Use Home"))
				g_Config.cvars.toggle_button = 0x24;

			ImGui::SameLine();

			if (ImGui::Button("Use End"))
				g_Config.cvars.toggle_button = 0x23;

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Separator();

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (30));
			ImGui::Text("Window Resize");

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (145 / 2));
			ImGui::Checkbox("Auto Resize", &g_Config.cvars.menu_auto_resize);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (87 / 2));
			ImGui::Text("Maps Soundcache");

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (135 / 2));
			ImGui::Checkbox("Save Soundcache", &g_Config.cvars.save_soundcache);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (20 / 2));
			ImGui::Text("Style");

			ImGui::Spacing();
			ImGui::Spacing();

			static const char* theme_items[] =
			{
				"Dark",
				"Light",
				"Classic",
				"Berserk",
				"Deep Dark",
				"Carbon",
				"Corporate Grey",
				"Grey",
				"Dark Light",
				"Soft Dark",
				"Gold & Black",
				"Monochrome",
				"Pink",
				"Half-Life",
				"Sven-Cope"
			};

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (130 / 2));
			ImGui::PushItemWidth(150);
			if (ImGui::Combo("", &g_Config.cvars.menu_theme, theme_items, IM_ARRAYSIZE(theme_items)))
			{
				LoadSavedStyle();

				LoadMenuTheme();
				WindowStyle();
			}
			ImGui::PopItemWidth();

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (35 / 2));
			ImGui::Text("Opacity");

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::PushItemWidth(150);
			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (130 / 2));
			ImGui::SliderFloat(" ", &g_Config.cvars.menu_opacity, 0.1f, 1.0f);

			ImGui::PopItemWidth();
			ImGui::Spacing();
			ImGui::Spacing();
		}
		ImGui::End();
	}
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

LRESULT CALLBACK HOOKED_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN && wParam == g_Config.cvars.toggle_button)
	{
		g_bMenuEnabled = !g_bMenuEnabled;

		if (g_bMenuEnabled)
		{
			extern void OnMenuOpen();

			OnMenuOpen();
		}
		else
		{
			extern void OnMenuClose();

			g_bMenuClosed = true;
			OnMenuClose();
		}

		return 0;
	}

	if (g_bMenuEnabled)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	}

	return CallWindowProc(hGameWndProc, hWnd, uMsg, wParam, lParam);
}

DECLARE_FUNC(BOOL, APIENTRY, HOOKED_wglSwapBuffers, HDC hdc)
{
	static bool bImGuiInitialized = false;

	if ( !bImGuiInitialized )
	{
		hGameWnd = WindowFromDC(hdc);
		hGameWndProc = (WNDPROC)SetWindowLong(hGameWnd, GWL_WNDPROC, (LONG)HOOKED_WndProc);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hGameWnd);
		ImGui_ImplOpenGL2_Init();

		ImGui::StyleColorsDark();
		SaveCurrentStyle();

		ImGuiIO &io = ImGui::GetIO();
		io.IniFilename = NULL;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

		bImGuiInitialized = true;
	}

	bool bMenuEnabled = g_bMenuEnabled;

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	g_MenuModule.Draw();

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	if (bMenuEnabled && !g_bMenuEnabled)
	{
		extern void OnMenuClose();

		g_bMenuClosed = true;
		OnMenuClose();
	}

	return ORIG_wglSwapBuffers(hdc);
}

DECLARE_FUNC(BOOL, WINAPI, HOOKED_SetCursorPos, int X, int Y)
{
	if (g_bMenuEnabled)
		return FALSE;

	return ORIG_SetCursorPos(X, Y);
}

//-----------------------------------------------------------------------------
// Menu feature impl
//-----------------------------------------------------------------------------

CMenuModule::CMenuModule()
{
	m_pfnwglSwapBuffers = NULL;
	m_pfnSetCursorPos = NULL;

	m_hwglSwapBuffers = 0;
	m_hSetCursorPos = 0;

	m_bThemeLoaded = false;
	m_bMenuSettings = false;
	m_bMenuConfig = false;
	m_bMenuAim = false;
	m_bMenuVisuals = false;
	m_bMenuHud = false;
	m_bMenuUtility = false;
}

bool CMenuModule::Load()
{
	m_pfnwglSwapBuffers = Sys_GetProcAddress(Sys_GetModuleHandle("opengl32.dll"), "wglSwapBuffers");
	m_pfnSetCursorPos = Sys_GetProcAddress(Sys_GetModuleHandle("user32.dll"), "SetCursorPos");

	if ( !m_pfnwglSwapBuffers )
	{
		Warning("Couldn't find function \"wglSwapBuffers\"\n");
		return false;
	}

	if ( !m_pfnSetCursorPos )
	{
		Warning("Couldn't find function \"SetCursorPos\"\n");
		return false;
	}

	if ( *(unsigned char *)m_pfnSetCursorPos == 0xE9 ) // JMP opcode, hooked by gameoverlayrenderer.dll
	{
		m_pfnSetCursorPos = MemoryUtils()->CalcAbsoluteAddress(m_pfnSetCursorPos);
	}

	if ( *(unsigned char *)m_pfnwglSwapBuffers == 0xE9 )
	{
		m_pfnwglSwapBuffers = MemoryUtils()->CalcAbsoluteAddress(m_pfnwglSwapBuffers);
	}

	return true;
}

void CMenuModule::PostLoad()
{
	m_hwglSwapBuffers = DetoursAPI()->DetourFunction( m_pfnwglSwapBuffers, HOOKED_wglSwapBuffers, GET_FUNC_PTR(ORIG_wglSwapBuffers) );
	m_hSetCursorPos = DetoursAPI()->DetourFunction( m_pfnSetCursorPos, HOOKED_SetCursorPos, GET_FUNC_PTR(ORIG_SetCursorPos) );
}

void CMenuModule::Unload()
{
	if ( hGameWnd && hGameWndProc )
	{
		SetWindowLong(hGameWnd, GWL_WNDPROC, (LONG)hGameWndProc);
	}

	DetoursAPI()->RemoveDetour( m_hwglSwapBuffers );
	DetoursAPI()->RemoveDetour( m_hSetCursorPos );
}