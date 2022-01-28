// Config

#pragma once

#include <Windows.h>

//-----------------------------------------------------------------------------

class CConfig
{
public:
	bool Load();
	void Save();

	struct config_cvars
	{
		// ESP
		bool esp = true;
		int esp_box = 1;
		bool esp_box_outline = true;
		int esp_box_fill = 0;
		bool esp_box_index = false;
		bool esp_box_distance = true;
		bool esp_box_entity_name = true;
		bool esp_box_player_name = true;
		bool esp_skeleton = false;
		bool esp_bones_name = false;
		int esp_skeleton_type = 1;

		float esp_friend_color[3] = { 0.0f, 1.0f, 0.0f };
		float esp_enemy_color[3] = { 1.0f, 0.0f, 0.0f };
		float esp_neutral_color[3] = { 1.0f, 1.0f, 0.0f };

		// Visual
		bool lightmap_override = false;
		float lightmap_brightness = 1.0f;
		float lightmap_color[3] = { 1.0f, 1.0f, 1.0f };

		bool no_shake = false;
		bool no_fade = false;

		bool draw_crosshair = true;
		int draw_entities = 0; // enum

		bool show_speed = false;
		float speed_color[4] = { 1.0f, 0.75f, 0.0f, 0.882f };

		bool wallhack = false;
		bool wallhack_white_walls = false;
		bool wallhack_wireframe = false;
		bool wallhack_wireframe_models = false;

		float wh_wireframe_width = 1.5f;
		float wh_wireframe_color[3] = { 0.0f, 1.0f, 0.0f };

		// Chams
		bool chams = false;

		int chams_players = 0;
		bool chams_players_wall = true;
		float chams_players_color[3] = { 0.0f, 1.0f, 0.0f };
		float chams_players_wall_color[3] = { 1.0f, 0.0f, 0.0f };

		int chams_entities = 0;
		bool chams_entities_wall = true;
		float chams_entities_color[3] = { 0.0f, 1.0f, 0.0f };
		float chams_entities_wall_color[3] = { 1.0f, 0.0f, 0.0f };
		
		int chams_viewmodel = 0;
		bool chams_viewmodel_wall = true;
		float chams_viewmodel_color[3] = { 0.0f, 0.0f, 1.0f };
		float chams_viewmodel_wall_color[3] = { 1.0f, 0.0f, 0.0f };

		// Glow
		bool glow = false;
		bool glow_optimize = false;

		int glow_players = 0;
		bool glow_players_wall = true;
		int glow_players_width = 10;
		float glow_players_color[3] = { 0.0f, 1.0f, 0.0f };

		int glow_entities = 0;
		bool glow_entities_wall = true;
		int glow_entities_width = 10;
		float glow_entities_color[3] = { 1.0f, 0.0f, 0.0f };

		int glow_viewmodel = 0;
		bool glow_viewmodel_wall = true;
		int glow_viewmodel_width = 10;
		float glow_viewmodel_color[3] = { 1.0f, 0.0f, 0.0f };

		// Strafer
		bool strafe = true;
		bool strafe_ignore_ground = true;
		int strafe_dir = 3; // enum
		int strafe_type = 0; // enum

		// Fake Lag
		bool fakelag = false;
		bool fakelag_adaptive_ex_interp = false;
		int fakelag_limit = 64; // enum
		int fakelag_move = 0; // enum
		int fakelag_type = 0; // enum
		float fakelag_variance = 3.5f;
		
		// Anti-AFK
		int antiafk = 0; // enum
		float antiafk_rotation_angle = -0.7f;

		// Misc		funcs: autoselfsink
		bool autojump = true;
		bool jumpbug = false;
		bool doubleduck = false;
		bool fastrun = false;
		bool quake_guns = false;
		bool tertiary_attack_glitch = false;
		bool save_soundcache = false;
		int no_weapon_anim = 0; // enum
		float speedhack = 1.0f;
		float ltfxspeed = 0.0f;
		float app_speed = 1.0f;

		bool color_pulsator = false;
		bool color_pulsator_top = true;
		bool color_pulsator_bottom = true;
		float color_pulsator_delay = 0.5f;
		
		bool helicopter = false;
		float helicopter_pitch_angle = 0.0f;
		float helicopter_rotation_angle = 0.5f;

		// Key Spam
		bool keyspam_hold_mode = true;
		bool keyspam_e = false;
		bool keyspam_w = false;
		bool keyspam_s = false;
		bool keyspam_q = false;
		bool keyspam_ctrl = false;

		// Fog
		bool fog = false;
		float fog_density = 0.25f;
		float fog_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		// Skybox
		int skybox = 0;

		// Cam Hack		funcs: toggle camhack, reset
		float camhack_speed_factor = 1.0f;

		// First-Person Observer
		bool fp_roaming = true;
		bool fp_roaming_draw_crosshair = true;
		bool fp_roaming_lerp = true;
		float fp_roaming_lerp_value = 0.15f;

		// Auto Vote	funcs: reload filter
		int autovote_mode = 0; // enum
		bool autovote_custom = true;
		bool autovote_ignore_filter = false;

		// AMS			funcs: show (current) muted playerds
		bool ams_mute_everything = false;
	};

	config_cvars cvars;
	DWORD dwToggleButton = VK_INSERT;
};

//-----------------------------------------------------------------------------

extern CConfig g_Config;