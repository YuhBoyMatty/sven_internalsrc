// Config

#pragma once

#include <Windows.h>

//-----------------------------------------------------------------------------

class CConfig
{
public:
	bool Load();
	void Save();

	struct config_cvars // why cvars lol
	{
		// ESP
		bool esp = true;
		int esp_box = 1;
		bool esp_box_outline = true;
		int esp_box_fill = 0;
		bool esp_box_index = false;
		bool esp_box_player_health = true;
		bool esp_box_player_armor = true;
		bool esp_box_distance = true;
		bool esp_box_entity_name = true;
		bool esp_box_player_name = true;
		bool esp_show_items = true;
		bool esp_ignore_unknown_ents = false;
		bool esp_skeleton = false;
		bool esp_bones_name = false;
		int esp_targets = 0;
		int esp_skeleton_type = 1;

		float esp_distance = 5000.0f;

		float esp_friend_color[3] = { 0.0f, 1.0f, 0.0f };
		float esp_enemy_color[3] = { 1.0f, 0.0f, 0.0f };
		float esp_neutral_color[3] = { 1.0f, 1.0f, 0.0f };
		float esp_item_color[3] = { 0.0f, 0.53f, 1.0f };

		// Visual
		bool lightmap_override = false;
		float lightmap_brightness = 1.0f;
		float lightmap_color[3] = { 1.0f, 1.0f, 1.0f };

		bool no_shake = false;
		bool no_fade = false;

		bool show_players_push_direction = false;
		float push_direction_length = 16.0f;
		float push_direction_width = 1.0f;
		float push_direction_color[3] = { 1.0f, 0.f, 0.f };

		bool draw_crosshair = true;
		bool draw_crosshair_dot = false;
		bool draw_crosshair_outline = true;
		int crosshair_size = 10;
		int crosshair_gap = 4;
		int crosshair_thickness = 2;
		int crosshair_outline_thickness = 1;
		float crosshair_outline_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float crosshair_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		int draw_entities = 0; // enum

		bool show_speed = false;
		bool show_vertical_speed = false;
		float speed_width_fraction = 0.5f;
		float speed_height_fraction = 0.88f;
		float speed_color[4] = { 1.0f, 0.75f, 0.0f, 0.882f };

		bool wallhack = false;
		bool wallhack_negative = false;
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
		
		int chams_items = 0;
		bool chams_items_wall = true;
		float chams_items_color[3] = { 0.0f, 0.0f, 1.0f };
		float chams_items_wall_color[3] = { 1.0f, 0.0f, 0.0f };

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
		float glow_entities_color[3] = { 0.0f, 0.0f, 1.0f };

		int glow_items = 0;
		bool glow_items_wall = true;
		int glow_items_width = 10;
		float glow_items_color[3] = { 0.0f, 0.0f, 1.0f };

		// Dynamic Glow
		bool dyn_glow_attach = false;

		bool dyn_glow_self = false;
		float dyn_glow_self_radius = 125.0f;
		float dyn_glow_self_decay = 0;
		float dyn_glow_self_color[3] = { 1.0f, 1.0f, 1.0f };

		bool dyn_glow_players = false;
		float dyn_glow_players_radius = 75.0f;
		float dyn_glow_players_decay = 0;
		float dyn_glow_players_color[3] = { 0.0f, 1.0f, 0.0f };
		
		bool dyn_glow_entities = false;
		float dyn_glow_entities_radius = 75.0f;
		float dyn_glow_entities_decay = 0;
		float dyn_glow_entities_color[3] = { 1.0f, 0.0f, 0.0f };
		
		bool dyn_glow_items = false;
		float dyn_glow_items_radius = 50.0f;
		float dyn_glow_items_decay = 0;
		float dyn_glow_items_color[3] = { 0.0f, 0.0f, 1.0f };

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
		bool antiafk_rotate_camera = true;
		bool antiafk_stay_within_range = true;
		bool antiafk_reset_stay_pos = true;
		float antiafk_stay_radius = 200.0f;
		float antiafk_stay_radius_offset_angle = 30.0f;
		float antiafk_rotation_angle = -0.7f;

		// Misc		funcs: autoselfsink
		bool autojump = true;
		bool jumpbug = false;
		bool doubleduck = false;
		bool fastrun = false;
		bool quake_guns = false;
		bool tertiary_attack_glitch = false;
		bool save_soundcache = false;
		bool rotate_dead_body = false;
		bool remove_fov_cap = false;
		bool auto_ceil_clipping = false;
		int no_weapon_anim = 0; // enum

		bool color_pulsator = false;
		bool color_pulsator_top = true;
		bool color_pulsator_bottom = true;
		float color_pulsator_delay = 0.5f;
		
		bool lock_pitch = false;
		bool lock_yaw = false;	
		float lock_pitch_angle = 0.0f;
		float lock_yaw_angle = 0.0f;

		bool spin_yaw_angle = false;
		bool spin_pitch_angle = false;
		float spin_yaw_rotation_angle = 0.5f;
		float spin_pitch_rotation_angle = 0.5f;

		float application_speed = 1.0f;

		// Key Spam
		bool keyspam_hold_mode = true;
		bool keyspam_e = false;
		bool keyspam_w = false;
		bool keyspam_s = false;
		bool keyspam_q = false;
		bool keyspam_ctrl = false;

		// Fog
		bool fog = false;
		bool remove_water_fog = false;
		bool fog_skybox = true;
		float fog_start = 0.0f;
		float fog_end = 1000.0f;
		float fog_density = 0.25f;
		float fog_color[3] = { 1.0f, 1.0f, 1.0f };

		// Skybox
		int skybox = 0;

		// Chat Colors
		bool enable_chat_colors = true;
		float player_name_color[3] = { 0.6f, 0.75f, 1.0f };
		float chat_rainbow_update_delay = 0.05f;
		float chat_rainbow_hue_delta = 0.015f;
		float chat_rainbow_saturation = 0.8f;
		float chat_rainbow_lightness = 0.5f;
		float chat_color_one[3] = { 0.25f, 0.25f, 1.0f };
		float chat_color_two[3] = { 1.0f, 0.25f, 0.25f };
		float chat_color_three[3] = { 1.0f, 1.0f, 0.25f };
		float chat_color_four[3] = { 0.25f, 1.0f, 0.25f };
		float chat_color_five[3] = { 1.0f, 0.57f, 0.0f };
		float chat_color_six[3] = { 1.0f, 1.0f, 1.0f };

		// Cam Hack		funcs: toggle camhack, reset
		bool camhack_show_model = true;
		float camhack_speed_factor = 1.0f;

		// First-Person Observer
		bool fp_roaming = true;
		bool fp_roaming_draw_crosshair = true;
		bool fp_roaming_lerp = true;
		float fp_roaming_lerp_value = 0.15f;

		// Auto Vote	funcs: reload filter
		//int autovote_mode = 0; // enum
		//bool autovote_custom = true;
		//bool autovote_ignore_filter = false;

		// Custom Vote Popup
		bool vote_popup = true;
		int vote_popup_width_size = 250;
		int vote_popup_height_size = 125;
		int vote_popup_w_border_pix = 12;
		int vote_popup_h_border_pix = 7;
		float vote_popup_width_frac = 0.015f;
		float vote_popup_height_frac = 0.37f;

		// AMS			funcs: show (current) muted playerds
		bool ams_mute_everything = false;
	};

	config_cvars cvars;
	DWORD dwToggleButton = VK_INSERT;
	bool ImGuiAutoResize = true;
	int theme = 0;
	float opacity = 1.0f;
};

//-----------------------------------------------------------------------------

extern CConfig g_Config;