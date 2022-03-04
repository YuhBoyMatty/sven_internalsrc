// Menu Module

#pragma comment(lib, "OpenGL32")

#include "menu.h"
#include "client.h"

#include <Windows.h>
#include <gl/GL.h>

#include "../sdk.h"
#include "../interfaces.h"
#include "../game/utils.h"

#include "../config.h"
#include "../patterns.h"
#include "../utils/trampoline_hook.h"
#include "../utils/signature_scanner.h"
#include "../utils/styles.h"

#include "imgui_impl_win32.h"
#include "imgui_impl_opengl2.h"

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef BOOL (APIENTRY *wglSwapBuffersFn)(HDC);
typedef BOOL (WINAPI *SetCursorPosFn)(int, int);

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

static HWND hGameWnd;
static WNDPROC hGameWndProc;

bool g_bMenuEnabled = false;
bool g_bMenuClosed = false;
static bool ThemeLoaded = false;;
bool Settings = false;
bool Visuals = false;
bool Hud = false;
bool Utility = false;
bool Ams = false;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(wglSwapBuffers_Hook);
TRAMPOLINE_HOOK(SetCursorPos_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

wglSwapBuffersFn wglSwapBuffers_Original = NULL;
SetCursorPosFn SetCursorPos_Original = NULL;
SetCursorPosFn SetCursorPos_GameOverlay = NULL;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// Restores window style
void WindowStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();

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

ImVec4 FramerateColor(ImVec4)
{
	if (ImGui::GetIO().Framerate < 30.f)
	{
		return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		return ImVec4(0.0f, 1.0f, 0.1f, 1.0f);
	}
}

//-----------------------------------------------------------------------------
// Menu
//-----------------------------------------------------------------------------

void ShowMainMenu()
{
	if (!ThemeLoaded) 
	{
		LoadTheme();
		WindowStyle();
		ThemeLoaded = true;
	}

	ImGui::GetIO().MouseDrawCursor = g_bMenuEnabled;
	ImGui::GetStyle().Alpha = g_Config.opacity;

	if (g_bMenuEnabled)
	{
		// Main Window
		ImGui::SetNextWindowSize(ImVec2(250.0f, 290.0f), ImGuiCond_FirstUseEver);
		if (g_Config.ImGuiAutoResize)
		{
			ImGui::Begin("Sven Internal", &g_bMenuEnabled, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImGui::SetWindowSize(ImVec2(250.0f, 290.0f));
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
			ImGui::TextColored(FramerateColor(ImVec4()), "%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (16));
			ImGui::Text("General");

			ImGui::Spacing();

			// Main Buttons
			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("Visuals", ImVec2(149, 28)))
			{
				Visuals ^= true;
			}

			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("HUD", ImVec2(149, 28)))
			{
				Hud ^= true;
			}

			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("Utility", ImVec2(149, 28)))
			{
				Utility ^= true;
			}

			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("Advanced Mute System", ImVec2(149, 28)))
			{
				Ams ^= true;
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (66));
			if (ImGui::Button("Settings", ImVec2(149, 28)))
			{
				Settings ^= true;
			}
		}
		ImGui::End();

		// Visuals
		if (Visuals)
		{
			ImGui::SetNextWindowSize(ImVec2(527.0f, 600.0f), ImGuiCond_FirstUseEver);
			if (g_Config.ImGuiAutoResize)
			ImGui::Begin("Visuals", &Visuals, ImGuiWindowFlags_AlwaysAutoResize);
			else
			ImGui::Begin("Visuals", &Visuals);
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
							"5 - Draw Players Bones"
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

						ImGui::Combo("   ", &g_Config.cvars.no_weapon_anim, no_weap_anim_items, IM_ARRAYSIZE(no_weap_anim_items));


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

						ImGui::Checkbox("Glow Items Behind Wall", &g_Config.cvars.glow_entities_wall);

						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::SliderInt("Glow Items", &g_Config.cvars.glow_entities, 0, 3, glow_items[g_Config.cvars.glow_entities]);

						ImGui::Spacing();

						ImGui::SliderInt("Glow Items Width", &g_Config.cvars.glow_entities_width, 0, 30);

						ImGui::Spacing();

						ImGui::ColorEdit3("Glow Items Color", g_Config.cvars.glow_entities_color);

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
						extern void ConCommand_ChangeSkybox();
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

							ConCommand_ChangeSkybox();

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

						ImGui::Separator();

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

		// HUD 
		if (Hud)
		{
			ImGui::SetNextWindowSize(ImVec2(463.0f, 510.0f), ImGuiCond_FirstUseEver);
			if (g_Config.ImGuiAutoResize)
				ImGui::Begin("HUD", &Hud, ImGuiWindowFlags_AlwaysAutoResize);
			else
				ImGui::Begin("HUD", &Hud);
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

						ImGui::Checkbox("Show Speedometer", &g_Config.cvars.show_speed);

						ImGui::Spacing();

						ImGui::Checkbox("Store Vertical Speed", &g_Config.cvars.show_vertical_speed);

						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::SliderFloat("Speed Width Fraction", &g_Config.cvars.speed_width_fraction, 0.0f, 1.0f);

						ImGui::Spacing();

						ImGui::SliderFloat("Speed Height Fraction", &g_Config.cvars.speed_height_fraction, 0.0f, 1.0f);

						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::ColorEdit4("Speed Color", g_Config.cvars.speed_color);

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

		// Utility
		if (Utility)
		{
			ImGui::SetNextWindowSize(ImVec2(690.0f, 500.0f), ImGuiCond_FirstUseEver);
			if (g_Config.ImGuiAutoResize)
				ImGui::Begin("Utility", &Utility, ImGuiWindowFlags_AlwaysAutoResize);
			else
				ImGui::Begin("Utility", &Utility);
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
						ImGui::Spacing();

						ImGui::Checkbox("Enable Jumpbug", &g_Config.cvars.jumpbug); ImGui::SameLine();
						ImGui::Checkbox("Doubleduck", &g_Config.cvars.doubleduck); ImGui::SameLine();
						ImGui::Checkbox("Fastrun", &g_Config.cvars.fastrun);

						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::Checkbox("Quake Guns", &g_Config.cvars.quake_guns); ImGui::SameLine();
						ImGui::Checkbox("Tertiary Attack Glitch", &g_Config.cvars.tertiary_attack_glitch); ImGui::SameLine();
						ImGui::Checkbox("Rotate Dead Body", &g_Config.cvars.rotate_dead_body);

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

						ImGui::SliderFloat("   ", &g_Config.cvars.color_pulsator_delay, 0.1f, 2.5f);

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
							"3 - Walk Around & Spam Inputs",
							"4 - Walk Around",
							"5 - Go Right"
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
						ImGui::Spacing();

						ImGui::SliderFloat("Rotation Angle", &g_Config.cvars.antiafk_rotation_angle, -7.0f, 7.0f);

						ImGui::Spacing();

						ImGui::SliderFloat("Stay Within Radius", &g_Config.cvars.antiafk_stay_radius, 25.0f, 500.0f);

						ImGui::Spacing();

						ImGui::SliderFloat("Stay Within Offset Angle", &g_Config.cvars.antiafk_stay_radius_offset_angle, 0.0f, 89.0f);

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

						ImGui::SliderFloat("   ", &g_Config.cvars.camhack_speed_factor, 0.0f, 15.0f);

						ImGui::Spacing();
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

						ImGui::SliderFloat("   ", &g_Config.cvars.fp_roaming_lerp_value, 0.001f, 1.0f);

						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::Separator();

						ImGui::Text("");
						ImGui::Spacing();

						ImGui::EndTabItem();
					}

					// Misc (Spinner, Application Speed)
					if (ImGui::BeginTabItem("Misc."))
					{
						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::Text("Spinner");

						ImGui::Spacing();

						ImGui::Separator();

						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::Checkbox("Enable Spinner", &g_Config.cvars.spinner);

						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::SliderFloat("Set Pitch Angle", &g_Config.cvars.spinner_pitch_angle, -180.0f, 180.0f);

						ImGui::Spacing();

						ImGui::SliderFloat("Yaw Angle Rotation", &g_Config.cvars.spinner_rotation_yaw_angle, -10.0f, 10.0f);

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::Checkbox("Rotate Pitch Angle", &g_Config.cvars.spinner_rotate_pitch_angle);

						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::SliderFloat("Pitch Angle Rotation", &g_Config.cvars.spinner_rotation_pitch_angle, -10.0f, 10.0f);

						ImGui::Spacing();
						ImGui::Spacing();

						ImGui::Separator();

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

		// Advanced Mute System
		if (Ams)
		{
			ImGui::SetNextWindowSize(ImVec2(250.0f, 260.0f), ImGuiCond_FirstUseEver);
			if (g_Config.ImGuiAutoResize)
			{
				ImGui::Begin("Advanced Mute System", &Ams, ImGuiWindowFlags_AlwaysAutoResize);
			}
			else
			{
				ImGui::Begin("Advanced Mute System", &Ams);
			}
			extern void ConCommand_ShowMutedPlayers(void);
			extern void ConCommand_ShowCurrentMutedPlayers(void);

			ImGui::Spacing();
			ImGui::Spacing();

			if (ImGui::Button("Show Muted Players"))
				ConCommand_ShowMutedPlayers();

			ImGui::Spacing();

			if (ImGui::Button("Show Current Muted Players"))
				ConCommand_ShowCurrentMutedPlayers();

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Checkbox("Mute Everything", &g_Config.cvars.ams_mute_everything);

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Separator();

			ImGui::Text("");
			ImGui::Spacing();

			ImGui::EndTabItem();
		}

		//Settings
		if (Settings)
		{
			ImGui::SetNextWindowSize(ImVec2(300.0f, 460.0f), ImGuiCond_FirstUseEver);
			if (g_Config.ImGuiAutoResize)
			{
				ImGui::Begin("Settings", &Settings, ImGuiWindowFlags_NoResize);
				ImGui::SetWindowSize(ImVec2(300.0f, 460.0f));
			}
			else
			{
			    ImGui::Begin("Settings", &Settings);
		    }

			{
				ImGui::Spacing();

				ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (110 / 2));
				ImGui::Text("Save & Load Config");

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (85 / 2));
				if (ImGui::Button("Load"))
					g_Config.Load();
				    LoadTheme();
					WindowStyle();

				ImGui::SameLine();

				if (ImGui::Button("Save"))
					g_Config.Save();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (60 / 2));
				ImGui::Text("Toggle Key");

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2.f) - (83.5f));
				if (ImGui::Button("Use Insert"))
					g_Config.dwToggleButton = 0x2D;

				ImGui::SameLine();

				if (ImGui::Button("Use Delete"))
					g_Config.dwToggleButton = 0x2E;

				ImGui::Spacing();

				ImGui::SameLine((ImGui::GetContentRegionAvail().x / 2) - (69));
				if (ImGui::Button("Use Home"))
					g_Config.dwToggleButton = 0x24;

				ImGui::SameLine();

				if (ImGui::Button("Use End"))
					g_Config.dwToggleButton = 0x23;

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
				ImGui::Checkbox("Always AutoResize", &g_Config.ImGuiAutoResize);

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
				if (ImGui::Combo("", &g_Config.theme, theme_items, IM_ARRAYSIZE(theme_items)))
				{
					LoadSavedStyle();

					LoadTheme();
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
				ImGui::SliderFloat(" ", &g_Config.opacity, 0.1f, 1.0f);

				ImGui::PopItemWidth();
				ImGui::Spacing();
				ImGui::Spacing();
			}
			ImGui::End();
		}
	}
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

LRESULT CALLBACK HOOK_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN && wParam == g_Config.dwToggleButton)
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

		return false;
	}

	if (g_bMenuEnabled)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	}

	return CallWindowProc(hGameWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL APIENTRY wglSwapBuffers_Hooked(HDC hdc)
{
	static bool bImGuiInitialized = false;

	if (!bImGuiInitialized)
	{
		hGameWnd = WindowFromDC(hdc);
		hGameWndProc = (WNDPROC)SetWindowLong(hGameWnd, GWL_WNDPROC, (LONG)HOOK_WndProc);

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

	ShowMainMenu();

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	if (bMenuEnabled && !g_bMenuEnabled)
	{
		extern void OnMenuClose();

		g_bMenuClosed = true;
		OnMenuClose();
	}

	return wglSwapBuffers_Original(hdc);
}

BOOL WINAPI SetCursorPos_Hooked(int X, int Y)
{
	if (g_bMenuEnabled)
		return FALSE;
	else
		return SetCursorPos_GameOverlay(X, Y);

	return SetCursorPos_Original(X, Y);
}

//-----------------------------------------------------------------------------
// Init/release menu module
//-----------------------------------------------------------------------------

void InitMenuModule()
{
#pragma warning(push)
#pragma warning(disable: 6387)

	using ValveUnhookFuncFn = void (__cdecl *)(void *, bool); // void *pAddress, bool unknown
	static ValveUnhookFuncFn ValveUnhookFunc = NULL;

	HMODULE hGameOverlayDLL = GetModuleHandle(L"gameoverlayrenderer.dll");

	if (!ValveUnhookFunc)
	{
		ValveUnhookFunc = (ValveUnhookFuncFn)FindPattern(hGameOverlayDLL, Patterns::GameOverlay::ValveUnhookFunc);

		if (!ValveUnhookFunc)
			Sys_Error("'ValveUnhookFunc' failed initialization\n");
	}

	SetCursorPos_GameOverlay = (SetCursorPosFn)FindPattern(hGameOverlayDLL, Patterns::GameOverlay::SetCursorPos_Hook);

	if (!SetCursorPos_GameOverlay)
		Sys_Error("'SetCursorPos_GameOverlay' failed initialization\n");

	void *pwglSwapBuffers = GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers");
	void *pSetCursorPos = GetProcAddress(GetModuleHandle(L"user32.dll"), "SetCursorPos");

	if (*(BYTE *)pwglSwapBuffers == 0xE9) // opcode JMP
		ValveUnhookFunc(pwglSwapBuffers, false);

	if (*(BYTE *)pSetCursorPos == 0xE9) // opcode JMP
		ValveUnhookFunc(pSetCursorPos, false);

	// I could just hook SetCursorPos_GameOverlay lol
	HOOK_FUNCTION(SetCursorPos_Hook, pSetCursorPos, SetCursorPos_Hooked, SetCursorPos_Original, SetCursorPosFn);
	HOOK_FUNCTION(wglSwapBuffers_Hook, pwglSwapBuffers, wglSwapBuffers_Hooked, wglSwapBuffers_Original, wglSwapBuffersFn);

#pragma warning(pop)
}

void ShutdownMenuModule()
{
}