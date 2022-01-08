// Menu Module

#pragma comment(lib, "OpenGL32")

#include "menu.h"

#include <Windows.h>
#include <gl/GL.h>

#include "../sdk.h"
#include "../interfaces.h"
#include "../game/utils.h"

#include "../config.h"
#include "../patterns.h"
#include "../utils/trampoline_hook.h"
#include "../utils/signature_scanner.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl2.h"

//-----------------------------------------------------------------------------

typedef BOOL (APIENTRY *wglSwapBuffersFn)(HDC);
typedef BOOL (WINAPI *SetCursorPosFn)(int, int);

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;

static HWND hGameWnd;
static WNDPROC hGameWndProc;

bool g_bMenuEnabled = false;
bool g_bMenuClosed = false;

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
// Menu
//-----------------------------------------------------------------------------

void ShowMainMenu()
{
	ImGui::GetIO().MouseDrawCursor = g_bMenuEnabled;

	if (g_bMenuEnabled)
	{
		ImGui::SetNextWindowSize(ImVec2(500.0f, 650.0f), ImGuiCond_FirstUseEver);

		ImGui::Begin("Sven Internal", &g_bMenuEnabled, ImGuiWindowFlags_MenuBar /*| ImGuiWindowFlags_AlwaysAutoResize*/);
		{
			ImGui::BeginMenuBar();
			{
				SYSTEMTIME SysTime;
				GetLocalTime(&SysTime);

				ImGui::Text("Time: %02d:%02d:%02d", SysTime.wHour, SysTime.wMinute, SysTime.wSecond); ImGui::SameLine(200.0f);
				ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}
			ImGui::EndMenuBar();

			// Config
			if (ImGui::CollapsingHeader("Config"))
			{
				ImGui::Separator();
				ImGui::Text("Save & Load Config");

				if (ImGui::Button("Load"))
					g_Config.Load();

				ImGui::SameLine();
					
				if (ImGui::Button("Save"))
					g_Config.Save();

				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("Toggle Key");

				if (ImGui::Button("Use Insert"))
					g_Config.dwToggleButton = 0x2D;

				ImGui::SameLine();
				
				if (ImGui::Button("Use Delete"))
					g_Config.dwToggleButton = 0x2E;

				if (ImGui::Button("Use Home"))
					g_Config.dwToggleButton = 0x24;

				ImGui::SameLine();

				if (ImGui::Button("Use End"))
					g_Config.dwToggleButton = 0x23;

				ImGui::Text("");
			}

			// ESP
			if (ImGui::CollapsingHeader("ESP"))
			{
				ImGui::Separator();

				ImGui::Checkbox("Enable ESP", &g_Config.cvars.esp);
				ImGui::Checkbox("Outline Box", &g_Config.cvars.esp_box_outline);
				ImGui::Checkbox("Draw Index", &g_Config.cvars.esp_box_index); ImGui::SameLine();
				ImGui::Checkbox("Draw Distance", &g_Config.cvars.esp_box_distance);
				ImGui::Checkbox("Draw Entity Name", &g_Config.cvars.esp_box_entity_name); ImGui::SameLine();
				ImGui::Checkbox("Draw Nicknames", &g_Config.cvars.esp_box_player_name);
				ImGui::Checkbox("Draw Skeleton", &g_Config.cvars.esp_skeleton); ImGui::SameLine();
				ImGui::Checkbox("Draw Bones Name", &g_Config.cvars.esp_bones_name);

				ImGui::Text("");
				ImGui::Text("ESP Colors");

				ImGui::ColorEdit3("Friend Color", g_Config.cvars.esp_friend_color);
				ImGui::ColorEdit3("Enemy Color", g_Config.cvars.esp_enemy_color);
				ImGui::ColorEdit3("Neutral Color", g_Config.cvars.esp_neutral_color);
				
				ImGui::Text("");

				static const char *esp_box_items[] = { "0 - Off", "1 - Default", "2 - Coal", "3 - Corner" };
				ImGui::Combo("Box Type", &g_Config.cvars.esp_box, esp_box_items, IM_ARRAYSIZE(esp_box_items));

				ImGui::Text("");
				
				static const char *esp_skeleton_items[] = { "0 - Everyone", "1 - Entities", "2 - Players" };
				ImGui::Combo("Draw Skeleton Type", &g_Config.cvars.esp_skeleton_type, esp_skeleton_items, IM_ARRAYSIZE(esp_skeleton_items));

				ImGui::Text("");

				ImGui::SliderInt("Box Fill Alpha", &g_Config.cvars.esp_box_fill, 0, 255);

				ImGui::Text("");
			}

			// Visual
			if (ImGui::CollapsingHeader("Visual"))
			{
				ImGui::Separator();

				ImGui::Checkbox("No Shake", &g_Config.cvars.no_shake); ImGui::SameLine();
				ImGui::Checkbox("No Fade", &g_Config.cvars.no_fade);
				ImGui::Checkbox("Draw Crosshair", &g_Config.cvars.draw_crosshair);

				ImGui::Text("");

				static const char *draw_entities_items[] =
				{
					"0 - Default",
					"1 - Draw Bones",
					"2 - Draw Hitboxes",
					"3 - Draw Model & Hitboxes",
					"4 - Draw Hulls",
					"5 - Draw Players Bones"
				};

				ImGui::Combo("Draw Entities", &g_Config.cvars.draw_entities, draw_entities_items, IM_ARRAYSIZE(draw_entities_items));
				
				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("Speed");

				ImGui::Checkbox("Show Speed", &g_Config.cvars.show_speed);
				ImGui::ColorEdit4("Speed Color", g_Config.cvars.speed_color);
				
				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("Light Map");

				ImGui::Checkbox("Override Lightmap", &g_Config.cvars.lightmap_override);
				ImGui::SliderFloat("Lightmap Brightness", &g_Config.cvars.lightmap_brightness, 0.0f, 1.0f);
				ImGui::ColorEdit3("Lightmap Color", g_Config.cvars.lightmap_color);

				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("Wallhack");

				ImGui::Checkbox("Default", &g_Config.cvars.wallhack); ImGui::SameLine();
				ImGui::Checkbox("White Walls", &g_Config.cvars.wallhack_white_walls);
				ImGui::Checkbox("Wireframe", &g_Config.cvars.wallhack_wireframe); ImGui::SameLine();
				ImGui::Checkbox("Wireframe Models", &g_Config.cvars.wallhack_wireframe_models);

				ImGui::Text("");

				ImGui::SliderFloat("Wireframe Line Width", &g_Config.cvars.wh_wireframe_width, 0.0f, 10.0f);
				ImGui::ColorEdit3("Wireframe Color", g_Config.cvars.wh_wireframe_color);

				ImGui::Text("");
			}

			// Chams
			if (ImGui::CollapsingHeader("Chams"))
			{
				static const char *chams_items[] = { "0 - Disable", "1 - Flat", "2 - Texture", "3 - Material" };

				ImGui::Separator();
				ImGui::Checkbox("Enable Chams", &g_Config.cvars.chams);
				ImGui::Text("");

				ImGui::Separator();
				ImGui::Text("Players");

				ImGui::Checkbox("Chams Players Behind Wall", &g_Config.cvars.chams_players_wall);
				ImGui::SliderInt("Chams Players", &g_Config.cvars.chams_players, 0, 3, chams_items[g_Config.cvars.chams_players]);
				ImGui::ColorEdit3("Chams Players Color", g_Config.cvars.chams_players_color);
				ImGui::ColorEdit3("Chams Players Wall Color", g_Config.cvars.chams_players_wall_color);

				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("Entities");

				ImGui::Checkbox("Chams Entities Behind Wall", &g_Config.cvars.chams_entities_wall);
				ImGui::SliderInt("Chams Entities", &g_Config.cvars.chams_entities, 0, 3, chams_items[g_Config.cvars.chams_entities]);
				ImGui::ColorEdit3("Chams Entities Color", g_Config.cvars.chams_entities_color);
				ImGui::ColorEdit3("Chams Entities Wall Color", g_Config.cvars.chams_entities_wall_color);
				
				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("View Model");

				ImGui::Checkbox("Chams View Model Behind Wall", &g_Config.cvars.chams_viewmodel_wall);
				ImGui::SliderInt("Chams View Model", &g_Config.cvars.chams_viewmodel, 0, 3, chams_items[g_Config.cvars.chams_viewmodel]);
				ImGui::ColorEdit3("Chams View Model Color", g_Config.cvars.chams_viewmodel_color);
				ImGui::ColorEdit3("Chams View Model Wall Color", g_Config.cvars.chams_viewmodel_wall_color);
				
				ImGui::Text("");
			}
			
			// Glow
			if (ImGui::CollapsingHeader("Glow"))
			{
				static const char *glow_items[] = { "0 - Disable", "1 - Glow Outline", "2 - Glow Shell", "3 - Ghost" };

				ImGui::Separator();
				ImGui::Checkbox("Enable Glow", &g_Config.cvars.glow);
				ImGui::Checkbox("Optimize Glow Behind Wall", &g_Config.cvars.glow_optimize);
				ImGui::Text("");

				ImGui::Separator();
				ImGui::Text("Players");

				ImGui::Checkbox("Glow Players Behind Wall", &g_Config.cvars.glow_players_wall);
				ImGui::SliderInt("Glow Players", &g_Config.cvars.glow_players, 0, 3, glow_items[g_Config.cvars.glow_players]);
				ImGui::SliderInt("Glow Players Width", &g_Config.cvars.glow_players_width, 0, 30);
				ImGui::ColorEdit3("Glow Players Color", g_Config.cvars.glow_players_color);

				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("Entities");

				ImGui::Checkbox("Glow Entities Behind Wall", &g_Config.cvars.glow_entities_wall);
				ImGui::SliderInt("Glow Entities", &g_Config.cvars.glow_entities, 0, 3, glow_items[g_Config.cvars.glow_entities]);
				ImGui::SliderInt("Glow Entities Width", &g_Config.cvars.glow_entities_width, 0, 30);
				ImGui::ColorEdit3("Glow Entities Color", g_Config.cvars.glow_entities_color);

				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("View Model");

				ImGui::Checkbox("Glow View Model Behind Wall", &g_Config.cvars.glow_viewmodel_wall);
				ImGui::SliderInt("Glow View Model", &g_Config.cvars.glow_viewmodel, 0, 3, glow_items[g_Config.cvars.glow_viewmodel]);
				ImGui::SliderInt("Glow View Model Width", &g_Config.cvars.glow_viewmodel_width, 0, 30);
				ImGui::ColorEdit3("Glow View Model Color", g_Config.cvars.glow_viewmodel_color);

				ImGui::Text("");
			}
			
			// Strafe
			if (ImGui::CollapsingHeader("Strafer"))
			{
				ImGui::Separator();

				ImGui::Checkbox("Enable Strafer", &g_Config.cvars.strafe);
				ImGui::Checkbox("Ignore Ground", &g_Config.cvars.strafe_ignore_ground);

				static const char *strafe_dir_items[] = { "0 - To the left", "1 - To the right", "2 - Best strafe", "3 - View angles" };
				ImGui::Combo("Strafe Direction", &g_Config.cvars.strafe_dir, strafe_dir_items, IM_ARRAYSIZE(strafe_dir_items));
				
				static const char *strafe_type_items[] = { "0 - Max. acceleration", "1 - Max. angle", "2 - Max. deceleration", "3 - Const speed" };
				ImGui::Combo("Strafe Type", &g_Config.cvars.strafe_type, strafe_type_items, IM_ARRAYSIZE(strafe_type_items));
				
				ImGui::Text("");
			}

			// Misc
			if (ImGui::CollapsingHeader("Misc"))
			{
				extern void ConCommand_AutoSelfSink();
				extern void ConCommand_Freeze();
				extern void ConCommand_DropEmptyWeapons();

				ImGui::Separator();

				if (ImGui::Button("Selfsink"))
					ConCommand_AutoSelfSink();
				
				if (ImGui::Button("Freeze"))
					ConCommand_Freeze();
				
				if (ImGui::Button("Drop Empty Weapons"))
					ConCommand_DropEmptyWeapons();

				ImGui::Checkbox("Autojump", &g_Config.cvars.autojump); ImGui::SameLine();
				ImGui::Checkbox("Jumpbug", &g_Config.cvars.jumpbug); ImGui::SameLine();
				ImGui::Checkbox("Doubleduck", &g_Config.cvars.doubleduck); ImGui::SameLine();
				ImGui::Checkbox("Fastrun", &g_Config.cvars.fastrun);
				ImGui::Checkbox("Quake Guns", &g_Config.cvars.quake_guns);
				ImGui::Checkbox("Tertiary Attack Glitch", &g_Config.cvars.tertiary_attack_glitch);
				ImGui::Checkbox("Save Soundcache", &g_Config.cvars.save_soundcache);

				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("Color Pulsator");

				extern void ConCommand_ResetColors();
				extern void ConCommand_SyncColors();

				if (ImGui::Button("Reset Colors"))
					ConCommand_ResetColors();
				
				if (ImGui::Button("Sync. Colors"))
					ConCommand_SyncColors();

				ImGui::Checkbox("Enable Pulsator", &g_Config.cvars.color_pulsator);
				ImGui::Checkbox("Change Top Color", &g_Config.cvars.color_pulsator_top);
				ImGui::Checkbox("Change Bottom Color", &g_Config.cvars.color_pulsator_bottom);

				ImGui::Text("");

				ImGui::SliderFloat("Change Color Delay", &g_Config.cvars.color_pulsator_delay, 0.1f, 2.5f);

				ImGui::Text("");
				ImGui::Separator();
				ImGui::Text("Speedhack Methods");

				if (ImGui::Button("Reset Speedhack"))
					g_Config.cvars.speedhack = 1.0f;

				ImGui::SliderFloat("Speedhack", &g_Config.cvars.speedhack, 0.0f, 30.0f);
				
				ImGui::Text("");

				if (ImGui::Button("Reset LTFX Speed"))
					g_Config.cvars.ltfxspeed = 0.0f;

				ImGui::SliderFloat("LTFX Speed", &g_Config.cvars.ltfxspeed, -5.0f, 5.0f);
				
				ImGui::Text("");

				if (ImGui::Button("Reset Application Speed"))
					g_Config.cvars.app_speed = 1.0f;

				ImGui::SliderFloat("Application Speed", &g_Config.cvars.app_speed, 0.1f, 30.0f);

				ImGui::Text("");
				ImGui::Separator();

				ImGui::Checkbox("Enable Helicopter", &g_Config.cvars.helicopter);
				ImGui::SliderFloat("Helicopter Pitch Angle", &g_Config.cvars.helicopter_pitch_angle, -180.0f, 180.0f);
				ImGui::SliderFloat("Helicopter Rotation Angle", &g_Config.cvars.helicopter_rotation_angle, -10.0f, 10.0f);

				ImGui::Text("");
				ImGui::Separator();

				static const char *no_weap_anim_items[] = { "0 - Off", "1 - All Animations", "2 - Take Animations" };
				ImGui::Combo("No Weapon Animations", &g_Config.cvars.no_weapon_anim, no_weap_anim_items, IM_ARRAYSIZE(no_weap_anim_items));

				ImGui::Text("");
			}
			
			// Fake Lag
			if (ImGui::CollapsingHeader("Fake Lag"))
			{
				ImGui::Separator();

				ImGui::Checkbox("Enable Fake Lag", &g_Config.cvars.fakelag);
				ImGui::Checkbox("Adaptive Ex Interp", &g_Config.cvars.fakelag_adaptive_ex_interp);

				ImGui::Text("");

				ImGui::SliderInt("Limit", &g_Config.cvars.fakelag_limit, 0, 256);
				ImGui::SliderFloat("Variance", &g_Config.cvars.fakelag_variance, 0.0f, 100.0f);

				ImGui::Text("");

				static const char *fakelag_type_items[] = { "0 - Dynamic", "1 - Maximum", "2 - Jitter", "3 - Break Lag Compensation" };
				ImGui::Combo("Fake Lag Type", &g_Config.cvars.fakelag_type, fakelag_type_items, IM_ARRAYSIZE(fakelag_type_items));
				
				static const char *fakelag_move_items[] = { "0 - Everytime", "1 - On Land", "2 - On Move", "3 - In Air" };
				ImGui::Combo("Fake Move Type", &g_Config.cvars.fakelag_move, fakelag_move_items, IM_ARRAYSIZE(fakelag_move_items));
				
				ImGui::Text("");
			}
			
			// Anti-AFK
			if (ImGui::CollapsingHeader("Anti-AFK"))
			{
				ImGui::Separator();

				static const char *antiafk_items[] = { "0 - Off", "1 - Step Forward & Back", "2 - Spam Gibme", "3 - Walk Around & Spam Inputs", "4 - Walk Around", "5 - Go Right" };
				ImGui::Combo("Mode", &g_Config.cvars.antiafk, antiafk_items, IM_ARRAYSIZE(antiafk_items));
				ImGui::SliderFloat("Rotation Angle", &g_Config.cvars.antiafk_rotation_angle, -7.0f, 7.0f);
				
				ImGui::Text("");
			}

			// Keyspam
			if (ImGui::CollapsingHeader("Key Spam"))
			{
				ImGui::Separator();

				ImGui::Checkbox("Hold Mode", &g_Config.cvars.keyspam_hold_mode);

				ImGui::Checkbox("Spam E", &g_Config.cvars.keyspam_e); ImGui::SameLine();
				ImGui::Checkbox("Spam Q", &g_Config.cvars.keyspam_q);

				ImGui::Checkbox("Spam W", &g_Config.cvars.keyspam_w); ImGui::SameLine();
				ImGui::Checkbox("Spam S", &g_Config.cvars.keyspam_s);

				ImGui::Checkbox("Spam CTRL", &g_Config.cvars.keyspam_ctrl);

				ImGui::Text("");
			}
			
			// Fog
			if (ImGui::CollapsingHeader("Fog"))
			{
				ImGui::Separator();

				ImGui::Checkbox("Enable Fog", &g_Config.cvars.fog);

				ImGui::Text("");

				ImGui::SliderFloat("Density", &g_Config.cvars.fog_density, 0.0f, 10.0f);

				ImGui::ColorEdit4("Color", g_Config.cvars.fog_color);

				ImGui::Text("");
			}
			
			// Cam Hack
			if (ImGui::CollapsingHeader("Cam Hack"))
			{
				extern void ConCommand_CamHack(void);
				extern void ConCommand_CamHackResetRoll(void);
				extern void ConCommand_CamHackReset(void);

				ImGui::Separator();

				if (ImGui::Button("Toggle Cam Hack"))
					ConCommand_CamHack();
				
				if (ImGui::Button("Reset Roll Axis"))
					ConCommand_CamHackResetRoll();
				
				if (ImGui::Button("Reset Cam Hack"))
					ConCommand_CamHackReset();

				ImGui::SliderFloat("Speed Factor", &g_Config.cvars.camhack_speed_factor, 0.0f, 15.0f);
				
				ImGui::Text("");
			}
			
			// First-Person Roaming
			if (ImGui::CollapsingHeader("First-Person Roaming"))
			{
				ImGui::Separator();

				ImGui::Checkbox("Enable First-Person Roaming", &g_Config.cvars.fp_roaming);
				ImGui::Checkbox("Draw Crosshair in Roaming", &g_Config.cvars.fp_roaming_draw_crosshair);
				ImGui::Checkbox("Lerp First-Person View", &g_Config.cvars.fp_roaming_lerp);

				ImGui::SliderFloat("Lerp Value", &g_Config.cvars.fp_roaming_lerp_value, 0.001f, 1.0f);
				
				ImGui::Text("");
			}
			
			// Auto Vote
			if (ImGui::CollapsingHeader("Auto Vote"))
			{
				extern void LoadVoteFilter();

				ImGui::Separator();

				if (ImGui::Button("Reload Filter"))
					LoadVoteFilter();
				
				ImGui::Checkbox("Use on Custom Votes", &g_Config.cvars.autovote_custom);
				ImGui::Checkbox("Ignore Filter", &g_Config.cvars.autovote_ignore_filter);

				static const char *autovote_items[] = { "Off", "Vote 'Yes'", "Vote 'No'" };
				ImGui::Combo("Vote Mode", &g_Config.cvars.autovote_mode, autovote_items, IM_ARRAYSIZE(autovote_items));
				
				ImGui::Text("");
			}

			// Message Spammer
			if (ImGui::CollapsingHeader("Message Spammer"))
			{
				extern void ConCommand_PrintSpamKeyWords(void);
				extern void ConCommand_PrintSpamTasks(void);

				ImGui::Separator();

				if (ImGui::Button("Show Spam Tasks"))
					ConCommand_PrintSpamTasks();

				if (ImGui::Button("Show Spam Keywords"))
					ConCommand_PrintSpamKeyWords();
				
				ImGui::Text("");
			}

			// Advanced Mute System
			if (ImGui::CollapsingHeader("Advanced Mute System"))
			{
				extern void ConCommand_ShowMutedPlayers(void);
				extern void ConCommand_ShowCurrentMutedPlayers(void);

				ImGui::Separator();

				if (ImGui::Button("Show Muted Players"))
					ConCommand_ShowMutedPlayers();

				ImGui::SameLine();
				
				if (ImGui::Button("Show Current Muted Players"))
					ConCommand_ShowCurrentMutedPlayers();
				
				ImGui::Checkbox("Mute Everything", &g_Config.cvars.ams_mute_everything);
				
				ImGui::Text("");
			}
		}
		ImGui::End();
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

	if (!ValveUnhookFunc)
	{
		ValveUnhookFunc = (ValveUnhookFuncFn)FIND_PATTERN(L"gameoverlayrenderer.dll", Patterns::GameOverlay::ValveUnhookFunc);

		if (!ValveUnhookFunc)
			ThrowError("'ValveUnhookFunc' failed initialization\n");
	}

	SetCursorPos_GameOverlay = (SetCursorPosFn)FIND_PATTERN(L"gameoverlayrenderer.dll", Patterns::GameOverlay::SetCursorPos_Hook);

	if (!SetCursorPos_GameOverlay)
		ThrowError("'SetCursorPos_GameOverlay' failed initialization\n");

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

	__INITIALIZED__ = true;
}

void ReleaseMenuModule()
{
	if (!__INITIALIZED__)
		return;
	
}