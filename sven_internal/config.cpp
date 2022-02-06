// Config

#include "config.h"

#include "../ini-parser/ini_parser.h"
#include "../features/skybox.h"
#include "../interfaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <type_traits>

//-----------------------------------------------------------------------------
// Make import/export easier
//-----------------------------------------------------------------------------

template <class T>
constexpr const char *ini_get_format_specifier(T var)
{
	if (std::is_same_v<T, int> || std::is_same_v<T, bool>)
		return "%s = %d\n";

	if (std::is_same_v<T, float>)
		return "%s = %.3f\n";

	if (std::is_same_v<T, DWORD>)
		return "%s = %X\n";
}

template <class T>
constexpr ini_field_type_t ini_get_fieldtype(T var)
{
	if (std::is_same_v<T, bool>)
		return INI_FIELD_BOOL;

	else if (std::is_same_v<T, int>)
		return INI_FIELD_INTEGER;

	else if (std::is_same_v<T, float>)
		return INI_FIELD_FLOAT;

	else if (std::is_same_v<T, DWORD>)
		return INI_FIELD_UINT32;
}

template <class T>
constexpr void ini_get_datatype(T &var, ini_datatype &datatype)
{
#pragma warning(push)
#pragma warning(disable: 4244)
	if (std::is_same_v<T, bool>)
		var = datatype.m_bool;

	else if (std::is_same_v<T, int>)
		var = datatype.m_int;

	else if (std::is_same_v<T, float>)
		var = datatype.m_float;

	else if (std::is_same_v<T, DWORD>)
		var = datatype.m_uint32;
#pragma warning(pop)
}

#define INI_EXPORT_BEGIN() FILE *file = fopen("sven_internal/sven_internal.ini", "w"); if (file) {
#define INI_EXPORT_END() fclose(file); }

#define INI_EXPORT_BEGIN_SECTION(section) fprintf(file, "[" section "]\n")
#define INI_EXPORT_END_SECTION() fprintf(file, "\n")
#define INI_EXPORT_VARIABLE(name, var) fprintf(file, ini_get_format_specifier(var), name, var)

#define INI_IMPORT_BEGIN() const char *pszSection = NULL; ini_datatype datatype; ini_data *data = (ini_data *)calloc(sizeof(ini_data), 1)
#define INI_IMPORT_END() ini_free_data(data, 1); return true
#define INI_IMPORT_BEGIN_SECTION(section) pszSection = section
#define INI_IMPORT_END_SECTION()

#define INI_IMPORT_VARIABLE_SET_RADIX(_radix) datatype.radix = _radix
#define INI_IMPORT_VARIABLE(name, var) \
	if (ini_read_data(data, pszSection, name, &datatype, ini_get_fieldtype(var))) \
		ini_get_datatype(var, datatype)

#define INI_IMPORT_PARSE_DATA() \
	if (!ini_parse_data("sven_internal/sven_internal.ini", data)) \
	{ \
		if (ini_get_last_error() == INI_MISSING_FILE) \
			g_pEngineFuncs->Con_Printf("Failed to parse the config file: Missing file sven_internal/sven_internal.ini to parse\n"); \
		else \
			g_pEngineFuncs->Con_Printf("Failed to parse the config file: Syntax error: %s in line %d\n", ini_get_last_error_msg(), ini_get_last_line()); \
			 \
		ini_free_data(data, 1); \
		return false; \
	}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CConfig g_Config;

//-----------------------------------------------------------------------------
// Implementations
//-----------------------------------------------------------------------------

bool CConfig::Load()
{
	INI_IMPORT_BEGIN();
	INI_IMPORT_PARSE_DATA();

	INI_IMPORT_BEGIN_SECTION("SETTINGS");
		INI_IMPORT_VARIABLE_SET_RADIX(16);
		INI_IMPORT_VARIABLE("ToggleButton", dwToggleButton);
	INI_IMPORT_END_SECTION();
		
	INI_IMPORT_BEGIN_SECTION("ESP");
		INI_IMPORT_VARIABLE("Enable", cvars.esp);
		INI_IMPORT_VARIABLE("Box", cvars.esp_box);
		INI_IMPORT_VARIABLE("Outline", cvars.esp_box_outline);
		INI_IMPORT_VARIABLE("Fill", cvars.esp_box_fill);
		INI_IMPORT_VARIABLE("ShowIndex", cvars.esp_box_index);
		INI_IMPORT_VARIABLE("ShowDistance", cvars.esp_box_distance);
		INI_IMPORT_VARIABLE("ShowPlayerHealth", cvars.esp_box_player_health);
		INI_IMPORT_VARIABLE("ShowPlayerArmor", cvars.esp_box_player_armor);
		INI_IMPORT_VARIABLE("ShowEntityName", cvars.esp_box_entity_name);
		INI_IMPORT_VARIABLE("ShowPlayerName", cvars.esp_box_player_name);
		INI_IMPORT_VARIABLE("Targets", cvars.esp_targets);
		INI_IMPORT_VARIABLE("ShowSkeleton", cvars.esp_skeleton);
		INI_IMPORT_VARIABLE("ShowBonesName", cvars.esp_bones_name);
		INI_IMPORT_VARIABLE("ShowSkeletonType", cvars.esp_skeleton_type);
		INI_IMPORT_VARIABLE("FriendColor_R", cvars.esp_friend_color[0]);
		INI_IMPORT_VARIABLE("FriendColor_G", cvars.esp_friend_color[1]);
		INI_IMPORT_VARIABLE("FriendColor_B", cvars.esp_friend_color[2]);
		INI_IMPORT_VARIABLE("EnemyColor_R", cvars.esp_enemy_color[0]);
		INI_IMPORT_VARIABLE("EnemyColor_G", cvars.esp_enemy_color[1]);
		INI_IMPORT_VARIABLE("EnemyColor_B", cvars.esp_enemy_color[2]);
		INI_IMPORT_VARIABLE("NeutralColor_R", cvars.esp_neutral_color[0]);
		INI_IMPORT_VARIABLE("NeutralColor_G", cvars.esp_neutral_color[1]);
		INI_IMPORT_VARIABLE("NeutralColor_B", cvars.esp_neutral_color[2]);
	INI_IMPORT_END_SECTION();
		
	INI_IMPORT_BEGIN_SECTION("WALLHACK");
		INI_IMPORT_VARIABLE("Wallhack", cvars.wallhack);
		INI_IMPORT_VARIABLE("WhiteWalls", cvars.wallhack_white_walls);
		INI_IMPORT_VARIABLE("Wireframe", cvars.wallhack_wireframe);
		INI_IMPORT_VARIABLE("WireframeModels", cvars.wallhack_wireframe_models);
		INI_IMPORT_VARIABLE("Wireframe_Width", cvars.wh_wireframe_width);
		INI_IMPORT_VARIABLE("Wireframe_R", cvars.wh_wireframe_color[0]);
		INI_IMPORT_VARIABLE("Wireframe_G", cvars.wh_wireframe_color[2]);
		INI_IMPORT_VARIABLE("Wireframe_B", cvars.wh_wireframe_color[1]);
	INI_IMPORT_END_SECTION();
	
	INI_IMPORT_BEGIN_SECTION("VISUAL");
		INI_IMPORT_VARIABLE("NoShake", cvars.no_shake);
		INI_IMPORT_VARIABLE("NoFade", cvars.no_fade);
		INI_IMPORT_VARIABLE("Crosshair", cvars.draw_crosshair);
		INI_IMPORT_VARIABLE("DrawEntities", cvars.draw_entities);
		INI_IMPORT_VARIABLE("ShowSpeed", cvars.show_speed);
		INI_IMPORT_VARIABLE("SpeedWidthFraction", cvars.speed_width_fraction);
		INI_IMPORT_VARIABLE("SpeedHeightFraction", cvars.speed_height_fraction);
		INI_IMPORT_VARIABLE("Speed_R", cvars.speed_color[0]);
		INI_IMPORT_VARIABLE("Speed_G", cvars.speed_color[1]);
		INI_IMPORT_VARIABLE("Speed_B", cvars.speed_color[2]);
		INI_IMPORT_VARIABLE("Speed_A", cvars.speed_color[3]);
		INI_IMPORT_VARIABLE("LightmapOverride", cvars.lightmap_override);
		INI_IMPORT_VARIABLE("LightmapOverrideBrightness", cvars.lightmap_brightness);
		INI_IMPORT_VARIABLE("LightmapOverride_R", cvars.lightmap_color[0]);
		INI_IMPORT_VARIABLE("LightmapOverride_G", cvars.lightmap_color[1]);
		INI_IMPORT_VARIABLE("LightmapOverride_B", cvars.lightmap_color[2]);
	INI_IMPORT_END_SECTION();

	INI_IMPORT_BEGIN_SECTION("CHAMS");
		INI_IMPORT_VARIABLE("Enable", cvars.chams);
		INI_IMPORT_VARIABLE("ChamsPlayers", cvars.chams_players);
		INI_IMPORT_VARIABLE("ChamsEntities", cvars.chams_entities);
		INI_IMPORT_VARIABLE("ChamsViewModel", cvars.chams_viewmodel);
		INI_IMPORT_VARIABLE("ChamsPlayersWall", cvars.chams_players_wall);
		INI_IMPORT_VARIABLE("ChamsEntitiesWall", cvars.chams_entities_wall);
		INI_IMPORT_VARIABLE("ChamsViewModelWall", cvars.chams_viewmodel_wall);
		INI_IMPORT_VARIABLE("ChamsPlayersColor_R", cvars.chams_players_color[0]);
		INI_IMPORT_VARIABLE("ChamsPlayersColor_G", cvars.chams_players_color[1]);
		INI_IMPORT_VARIABLE("ChamsPlayersColor_B", cvars.chams_players_color[2]);
		INI_IMPORT_VARIABLE("ChamsEntitiesColor_R", cvars.chams_entities_color[0]);
		INI_IMPORT_VARIABLE("ChamsEntitiesColor_G", cvars.chams_entities_color[1]);
		INI_IMPORT_VARIABLE("ChamsEntitiesColor_B", cvars.chams_entities_color[2]);
		INI_IMPORT_VARIABLE("ChamsViewModelColor_R", cvars.chams_viewmodel_color[0]);
		INI_IMPORT_VARIABLE("ChamsViewModelColor_G", cvars.chams_viewmodel_color[1]);
		INI_IMPORT_VARIABLE("ChamsViewModelColor_B", cvars.chams_viewmodel_color[2]);
		INI_IMPORT_VARIABLE("ChamsPlayersWallColor_R", cvars.chams_players_wall_color[0]);
		INI_IMPORT_VARIABLE("ChamsPlayersWallColor_G", cvars.chams_players_wall_color[1]);
		INI_IMPORT_VARIABLE("ChamsPlayersWallColor_B", cvars.chams_players_wall_color[2]);
		INI_IMPORT_VARIABLE("ChamsEntitiesWallColor_R", cvars.chams_entities_wall_color[0]);
		INI_IMPORT_VARIABLE("ChamsEntitiesWallColor_G", cvars.chams_entities_wall_color[1]);
		INI_IMPORT_VARIABLE("ChamsEntitiesWallColor_B", cvars.chams_entities_wall_color[2]);
		INI_IMPORT_VARIABLE("ChamsViewModelWallColor_R", cvars.chams_viewmodel_wall_color[0]);
		INI_IMPORT_VARIABLE("ChamsViewModelWallColor_G", cvars.chams_viewmodel_wall_color[1]);
		INI_IMPORT_VARIABLE("ChamsViewModelWallColor_B", cvars.chams_viewmodel_wall_color[2]);
	INI_IMPORT_END_SECTION();

	INI_IMPORT_BEGIN_SECTION("GLOW");
		INI_IMPORT_VARIABLE("Enable", cvars.glow);
		INI_IMPORT_VARIABLE("Optimize", cvars.glow_optimize);
		INI_IMPORT_VARIABLE("GlowPlayers", cvars.glow_players);
		INI_IMPORT_VARIABLE("GlowEntities", cvars.glow_entities);
		INI_IMPORT_VARIABLE("GlowViewModel", cvars.glow_viewmodel);
		INI_IMPORT_VARIABLE("GlowPlayersWidth", cvars.glow_players_width);
		INI_IMPORT_VARIABLE("GlowEntitiesWidth", cvars.glow_entities_width);
		INI_IMPORT_VARIABLE("GlowViewModelWidth", cvars.glow_viewmodel_width);
		INI_IMPORT_VARIABLE("GlowPlayersWall", cvars.glow_players_wall);
		INI_IMPORT_VARIABLE("GlowEntitiesWall", cvars.glow_entities_wall);
		INI_IMPORT_VARIABLE("GlowViewModelWall", cvars.glow_viewmodel_wall);
		INI_IMPORT_VARIABLE("GlowPlayersColor_R", cvars.glow_players_color[0]);
		INI_IMPORT_VARIABLE("GlowPlayersColor_G", cvars.glow_players_color[1]);
		INI_IMPORT_VARIABLE("GlowPlayersColor_B", cvars.glow_players_color[2]);
		INI_IMPORT_VARIABLE("GlowEntitiesColor_R", cvars.glow_entities_color[0]);
		INI_IMPORT_VARIABLE("GlowEntitiesColor_G", cvars.glow_entities_color[1]);
		INI_IMPORT_VARIABLE("GlowEntitiesColor_B", cvars.glow_entities_color[2]);
		INI_IMPORT_VARIABLE("GlowViewModelColor_R", cvars.glow_viewmodel_color[0]);
		INI_IMPORT_VARIABLE("GlowViewModelColor_G", cvars.glow_viewmodel_color[1]);
		INI_IMPORT_VARIABLE("GlowViewModelColor_B", cvars.glow_viewmodel_color[2]);
	INI_IMPORT_END_SECTION();

	INI_IMPORT_BEGIN_SECTION("STRAFE");
		INI_IMPORT_VARIABLE("Enable", cvars.strafe);
		INI_IMPORT_VARIABLE("IgnoreGround", cvars.strafe_ignore_ground);
		INI_IMPORT_VARIABLE("Direction", cvars.strafe_dir);
		INI_IMPORT_VARIABLE("Type", cvars.strafe_type);
	INI_IMPORT_END_SECTION();
	
	INI_IMPORT_BEGIN_SECTION("FAKELAG");
		INI_IMPORT_VARIABLE("Enable", cvars.fakelag);
		INI_IMPORT_VARIABLE("AdaptiveInterp", cvars.fakelag_adaptive_ex_interp);
		INI_IMPORT_VARIABLE("Type", cvars.fakelag_type);
		INI_IMPORT_VARIABLE("Move", cvars.fakelag_move);
		INI_IMPORT_VARIABLE("Limit", cvars.fakelag_limit);
		INI_IMPORT_VARIABLE("Variance", cvars.fakelag_variance);
	INI_IMPORT_END_SECTION();
	
	INI_IMPORT_BEGIN_SECTION("ANTIAFK");
		INI_IMPORT_VARIABLE("Type", cvars.antiafk);
		INI_IMPORT_VARIABLE("RotateCamera", cvars.antiafk_rotate_camera);
		INI_IMPORT_VARIABLE("StayWithinRange", cvars.antiafk_stay_within_range);
		INI_IMPORT_VARIABLE("RotationAngle", cvars.antiafk_rotation_angle);
		INI_IMPORT_VARIABLE("StayRadius", cvars.antiafk_stay_radius);
		INI_IMPORT_VARIABLE("StayRadiusOffsetAngle", cvars.antiafk_stay_radius_offset_angle);
	INI_IMPORT_END_SECTION();

	INI_IMPORT_BEGIN_SECTION("MISC");
		INI_IMPORT_VARIABLE("AutoJump", cvars.autojump);
		INI_IMPORT_VARIABLE("JumpBug", cvars.jumpbug);
		INI_IMPORT_VARIABLE("JumpBugMinHeight", cvars.jumpbug_min_height);
		INI_IMPORT_VARIABLE("DoubleDuck", cvars.doubleduck);
		INI_IMPORT_VARIABLE("FastRun", cvars.fastrun);
		INI_IMPORT_VARIABLE("QuakeGuns", cvars.quake_guns);
		INI_IMPORT_VARIABLE("TertiaryAttackGlitch", cvars.tertiary_attack_glitch);
		INI_IMPORT_VARIABLE("SaveSoundcache", cvars.save_soundcache);
		INI_IMPORT_VARIABLE("RotateDeadBody", cvars.rotate_dead_body);
		INI_IMPORT_VARIABLE("NoWeaponAnim", cvars.no_weapon_anim);
		INI_IMPORT_VARIABLE("ColorPulsator", cvars.color_pulsator);
		INI_IMPORT_VARIABLE("ColorPulsatorTop", cvars.color_pulsator_top);
		INI_IMPORT_VARIABLE("ColorPulsatorBottom", cvars.color_pulsator_bottom);
		INI_IMPORT_VARIABLE("ColorPulsatorDelay", cvars.color_pulsator_delay);
		INI_IMPORT_VARIABLE("Spinner", cvars.spinner);
		INI_IMPORT_VARIABLE("SpinnerRotatePitchAngle", cvars.spinner_rotate_pitch_angle);
		INI_IMPORT_VARIABLE("SpinnerPitchAngle", cvars.spinner_pitch_angle);
		INI_IMPORT_VARIABLE("SpinnerRotationPitchAngle", cvars.spinner_rotation_pitch_angle);
		INI_IMPORT_VARIABLE("SpinnerRotationYawAngle", cvars.spinner_rotation_yaw_angle);
		INI_IMPORT_VARIABLE("ApplicationSpeed", cvars.application_speed);
	INI_IMPORT_END_SECTION();

	INI_IMPORT_BEGIN_SECTION("KEYSPAM");
		INI_IMPORT_VARIABLE("HoldMode", cvars.keyspam_hold_mode);
		INI_IMPORT_VARIABLE("Spam_E", cvars.keyspam_e);
		INI_IMPORT_VARIABLE("Spam_W", cvars.keyspam_w);
		INI_IMPORT_VARIABLE("Spam_S", cvars.keyspam_s);
		INI_IMPORT_VARIABLE("Spam_Q", cvars.keyspam_q);
		INI_IMPORT_VARIABLE("Spam_CTRL", cvars.keyspam_ctrl);
	INI_IMPORT_END_SECTION();
	
	INI_IMPORT_BEGIN_SECTION("FOG");
		INI_IMPORT_VARIABLE("Enable", cvars.fog);
		INI_IMPORT_VARIABLE("FogSkybox", cvars.fog_skybox);
		INI_IMPORT_VARIABLE("RemoveInWater", cvars.remove_water_fog);
		INI_IMPORT_VARIABLE("Start", cvars.fog_start);
		INI_IMPORT_VARIABLE("End", cvars.fog_end);
		INI_IMPORT_VARIABLE("Density", cvars.fog_density);
		INI_IMPORT_VARIABLE("Fog_R", cvars.fog_color[0]);
		INI_IMPORT_VARIABLE("Fog_G", cvars.fog_color[1]);
		INI_IMPORT_VARIABLE("Fog_B", cvars.fog_color[2]);
	INI_IMPORT_END_SECTION();

	INI_IMPORT_BEGIN_SECTION("SKYBOX");
		INI_IMPORT_VARIABLE("Type", cvars.skybox);
	INI_IMPORT_END_SECTION();
	
	INI_IMPORT_BEGIN_SECTION("CHATCOLORS");
		INI_IMPORT_VARIABLE("PlayerName_R", cvars.player_name_color[0]);
		INI_IMPORT_VARIABLE("PlayerName_G", cvars.player_name_color[1]);
		INI_IMPORT_VARIABLE("PlayerName_B", cvars.player_name_color[2]);
		INI_IMPORT_VARIABLE("RainbowUpdateDelay", cvars.chat_rainbow_update_delay);
		INI_IMPORT_VARIABLE("RainbowHueDelta", cvars.chat_rainbow_hue_delta);
		INI_IMPORT_VARIABLE("RainbowSaturation", cvars.chat_rainbow_saturation);
		INI_IMPORT_VARIABLE("RainbowLightness", cvars.chat_rainbow_lightness);
		INI_IMPORT_VARIABLE("ColorOne_R", cvars.chat_color_one[0]);
		INI_IMPORT_VARIABLE("ColorOne_G", cvars.chat_color_one[1]);
		INI_IMPORT_VARIABLE("ColorOne_B", cvars.chat_color_one[2]);
		INI_IMPORT_VARIABLE("ColorTwo_R", cvars.chat_color_two[0]);
		INI_IMPORT_VARIABLE("ColorTwo_G", cvars.chat_color_two[1]);
		INI_IMPORT_VARIABLE("ColorTwo_B", cvars.chat_color_two[2]);
		INI_IMPORT_VARIABLE("ColorThree_R", cvars.chat_color_three[0]);
		INI_IMPORT_VARIABLE("ColorThree_G", cvars.chat_color_three[1]);
		INI_IMPORT_VARIABLE("ColorThree_B", cvars.chat_color_three[2]);
		INI_IMPORT_VARIABLE("ColorFour_R", cvars.chat_color_four[0]);
		INI_IMPORT_VARIABLE("ColorFour_G", cvars.chat_color_four[1]);
		INI_IMPORT_VARIABLE("ColorFour_B", cvars.chat_color_four[2]);
		INI_IMPORT_VARIABLE("ColorFive_R", cvars.chat_color_five[0]);
		INI_IMPORT_VARIABLE("ColorFive_G", cvars.chat_color_five[1]);
		INI_IMPORT_VARIABLE("ColorFive_B", cvars.chat_color_five[2]);
	INI_IMPORT_END_SECTION();

	INI_IMPORT_BEGIN_SECTION("CAMHACK");
		INI_IMPORT_VARIABLE("SpeedFactor", cvars.camhack_speed_factor);
		INI_IMPORT_VARIABLE("ShowModel", cvars.camhack_show_model);
	INI_IMPORT_END_SECTION();

	INI_IMPORT_BEGIN_SECTION("FPROAMING");
		INI_IMPORT_VARIABLE("Enable", cvars.fp_roaming);
		INI_IMPORT_VARIABLE("Crosshair", cvars.fp_roaming_draw_crosshair);
		INI_IMPORT_VARIABLE("Lerp", cvars.fp_roaming_lerp);
		INI_IMPORT_VARIABLE("LerpValue", cvars.fp_roaming_lerp_value);
	INI_IMPORT_END_SECTION();

	INI_IMPORT_BEGIN_SECTION("VOTEPOPUP");
		INI_IMPORT_VARIABLE("Enable", cvars.vote_popup);
		INI_IMPORT_VARIABLE("WidthSize", cvars.vote_popup_width_size);
		INI_IMPORT_VARIABLE("HeightSize", cvars.vote_popup_height_size);
		INI_IMPORT_VARIABLE("WidthBorderPixels", cvars.vote_popup_w_border_pix);
		INI_IMPORT_VARIABLE("HeightBorderPixels", cvars.vote_popup_h_border_pix);
		INI_IMPORT_VARIABLE("WidthFraction", cvars.vote_popup_width_frac);
		INI_IMPORT_VARIABLE("HeightFraction", cvars.vote_popup_height_frac);
	INI_IMPORT_END_SECTION();

	//INI_IMPORT_BEGIN_SECTION("AUTOVOTE");
	//	INI_IMPORT_VARIABLE("Mode", cvars.autovote_mode);
	//	INI_IMPORT_VARIABLE("UseOnCustomVotes", cvars.autovote_custom);
	//	INI_IMPORT_VARIABLE("IgnoreVoteFilter", cvars.autovote_ignore_filter);
	//INI_IMPORT_END_SECTION();
	
	INI_IMPORT_BEGIN_SECTION("AMS");
		INI_IMPORT_VARIABLE("MuteEverything", cvars.ams_mute_everything);
	INI_IMPORT_END_SECTION();

	// Callbacks
	g_Skybox.OnConfigLoad();

	INI_IMPORT_END();
}

void CConfig::Save()
{
	INI_EXPORT_BEGIN()

	INI_EXPORT_BEGIN_SECTION("SETTINGS");
		INI_EXPORT_VARIABLE("ToggleButton", dwToggleButton);
	INI_EXPORT_END_SECTION();
		
	INI_EXPORT_BEGIN_SECTION("ESP");
		INI_EXPORT_VARIABLE("Enable", cvars.esp);
		INI_EXPORT_VARIABLE("Box", cvars.esp_box);
		INI_EXPORT_VARIABLE("Outline", cvars.esp_box_outline);
		INI_EXPORT_VARIABLE("Fill", cvars.esp_box_fill);
		INI_EXPORT_VARIABLE("ShowIndex", cvars.esp_box_index);
		INI_EXPORT_VARIABLE("ShowDistance", cvars.esp_box_distance);
		INI_EXPORT_VARIABLE("ShowPlayerHealth", cvars.esp_box_player_health);
		INI_EXPORT_VARIABLE("ShowPlayerArmor", cvars.esp_box_player_armor);
		INI_EXPORT_VARIABLE("ShowEntityName", cvars.esp_box_entity_name);
		INI_EXPORT_VARIABLE("ShowPlayerName", cvars.esp_box_player_name);
		INI_EXPORT_VARIABLE("Targets", cvars.esp_targets);
		INI_EXPORT_VARIABLE("ShowSkeleton", cvars.esp_skeleton);
		INI_EXPORT_VARIABLE("ShowBonesName", cvars.esp_bones_name);
		INI_EXPORT_VARIABLE("ShowSkeletonType", cvars.esp_skeleton_type);
		INI_EXPORT_VARIABLE("FriendColor_R", cvars.esp_friend_color[0]);
		INI_EXPORT_VARIABLE("FriendColor_G", cvars.esp_friend_color[1]);
		INI_EXPORT_VARIABLE("FriendColor_B", cvars.esp_friend_color[2]);
		INI_EXPORT_VARIABLE("EnemyColor_R", cvars.esp_enemy_color[0]);
		INI_EXPORT_VARIABLE("EnemyColor_G", cvars.esp_enemy_color[1]);
		INI_EXPORT_VARIABLE("EnemyColor_B", cvars.esp_enemy_color[2]);
		INI_EXPORT_VARIABLE("NeutralColor_R", cvars.esp_neutral_color[0]);
		INI_EXPORT_VARIABLE("NeutralColor_G", cvars.esp_neutral_color[1]);
		INI_EXPORT_VARIABLE("NeutralColor_B", cvars.esp_neutral_color[2]);
	INI_EXPORT_END_SECTION();
		
	INI_EXPORT_BEGIN_SECTION("WALLHACK");
		INI_EXPORT_VARIABLE("Wallhack", cvars.wallhack);
		INI_EXPORT_VARIABLE("WhiteWalls", cvars.wallhack_white_walls);
		INI_EXPORT_VARIABLE("Wireframe", cvars.wallhack_wireframe);
		INI_EXPORT_VARIABLE("WireframeModels", cvars.wallhack_wireframe_models);
		INI_EXPORT_VARIABLE("Wireframe_Width", cvars.wh_wireframe_width);
		INI_EXPORT_VARIABLE("Wireframe_R", cvars.wh_wireframe_color[0]);
		INI_EXPORT_VARIABLE("Wireframe_G", cvars.wh_wireframe_color[2]);
		INI_EXPORT_VARIABLE("Wireframe_B", cvars.wh_wireframe_color[1]);
	INI_EXPORT_END_SECTION();
	
	INI_EXPORT_BEGIN_SECTION("VISUAL");
		INI_EXPORT_VARIABLE("NoShake", cvars.no_shake);
		INI_EXPORT_VARIABLE("NoFade", cvars.no_fade);
		INI_EXPORT_VARIABLE("Crosshair", cvars.draw_crosshair);
		INI_EXPORT_VARIABLE("DrawEntities", cvars.draw_entities);
		INI_EXPORT_VARIABLE("ShowSpeed", cvars.show_speed);
		INI_EXPORT_VARIABLE("SpeedWidthFraction", cvars.speed_width_fraction);
		INI_EXPORT_VARIABLE("SpeedHeightFraction", cvars.speed_height_fraction);
		INI_EXPORT_VARIABLE("Speed_R", cvars.speed_color[0]);
		INI_EXPORT_VARIABLE("Speed_G", cvars.speed_color[1]);
		INI_EXPORT_VARIABLE("Speed_B", cvars.speed_color[2]);
		INI_EXPORT_VARIABLE("Speed_A", cvars.speed_color[3]);
		INI_EXPORT_VARIABLE("LightmapOverride", cvars.lightmap_override);
		INI_EXPORT_VARIABLE("LightmapOverrideBrightness", cvars.lightmap_brightness);
		INI_EXPORT_VARIABLE("LightmapOverride_R", cvars.lightmap_color[0]);
		INI_EXPORT_VARIABLE("LightmapOverride_G", cvars.lightmap_color[1]);
		INI_EXPORT_VARIABLE("LightmapOverride_B", cvars.lightmap_color[2]);
	INI_EXPORT_END_SECTION();
	
	INI_EXPORT_BEGIN_SECTION("CHAMS");
		INI_EXPORT_VARIABLE("Enable", cvars.chams);
		INI_EXPORT_VARIABLE("ChamsPlayers", cvars.chams_players);
		INI_EXPORT_VARIABLE("ChamsEntities", cvars.chams_entities);
		INI_EXPORT_VARIABLE("ChamsViewModel", cvars.chams_viewmodel);
		INI_EXPORT_VARIABLE("ChamsPlayersWall", cvars.chams_players_wall);
		INI_EXPORT_VARIABLE("ChamsEntitiesWall", cvars.chams_entities_wall);
		INI_EXPORT_VARIABLE("ChamsViewModelWall", cvars.chams_viewmodel_wall);
		INI_EXPORT_VARIABLE("ChamsPlayersColor_R", cvars.chams_players_color[0]);
		INI_EXPORT_VARIABLE("ChamsPlayersColor_G", cvars.chams_players_color[1]);
		INI_EXPORT_VARIABLE("ChamsPlayersColor_B", cvars.chams_players_color[2]);
		INI_EXPORT_VARIABLE("ChamsEntitiesColor_R", cvars.chams_entities_color[0]);
		INI_EXPORT_VARIABLE("ChamsEntitiesColor_G", cvars.chams_entities_color[1]);
		INI_EXPORT_VARIABLE("ChamsEntitiesColor_B", cvars.chams_entities_color[2]);
		INI_EXPORT_VARIABLE("ChamsViewModelColor_R", cvars.chams_viewmodel_color[0]);
		INI_EXPORT_VARIABLE("ChamsViewModelColor_G", cvars.chams_viewmodel_color[1]);
		INI_EXPORT_VARIABLE("ChamsViewModelColor_B", cvars.chams_viewmodel_color[2]);
		INI_EXPORT_VARIABLE("ChamsPlayersWallColor_R", cvars.chams_players_wall_color[0]);
		INI_EXPORT_VARIABLE("ChamsPlayersWallColor_G", cvars.chams_players_wall_color[1]);
		INI_EXPORT_VARIABLE("ChamsPlayersWallColor_B", cvars.chams_players_wall_color[2]);
		INI_EXPORT_VARIABLE("ChamsEntitiesWallColor_R", cvars.chams_entities_wall_color[0]);
		INI_EXPORT_VARIABLE("ChamsEntitiesWallColor_G", cvars.chams_entities_wall_color[1]);
		INI_EXPORT_VARIABLE("ChamsEntitiesWallColor_B", cvars.chams_entities_wall_color[2]);
		INI_EXPORT_VARIABLE("ChamsViewModelWallColor_R", cvars.chams_viewmodel_wall_color[0]);
		INI_EXPORT_VARIABLE("ChamsViewModelWallColor_G", cvars.chams_viewmodel_wall_color[1]);
		INI_EXPORT_VARIABLE("ChamsViewModelWallColor_B", cvars.chams_viewmodel_wall_color[2]);
	INI_EXPORT_END_SECTION();

	INI_EXPORT_BEGIN_SECTION("GLOW");
		INI_EXPORT_VARIABLE("Enable", cvars.glow);
		INI_EXPORT_VARIABLE("Optimize", cvars.glow_optimize);
		INI_EXPORT_VARIABLE("GlowPlayers", cvars.glow_players);
		INI_EXPORT_VARIABLE("GlowEntities", cvars.glow_entities);
		INI_EXPORT_VARIABLE("GlowViewModel", cvars.glow_viewmodel);
		INI_EXPORT_VARIABLE("GlowPlayersWidth", cvars.glow_players_width);
		INI_EXPORT_VARIABLE("GlowEntitiesWidth", cvars.glow_entities_width);
		INI_EXPORT_VARIABLE("GlowViewModelWidth", cvars.glow_viewmodel_width);
		INI_EXPORT_VARIABLE("GlowPlayersWall", cvars.glow_players_wall);
		INI_EXPORT_VARIABLE("GlowEntitiesWall", cvars.glow_entities_wall);
		INI_EXPORT_VARIABLE("GlowViewModelWall", cvars.glow_viewmodel_wall);
		INI_EXPORT_VARIABLE("GlowPlayersColor_R", cvars.glow_players_color[0]);
		INI_EXPORT_VARIABLE("GlowPlayersColor_G", cvars.glow_players_color[1]);
		INI_EXPORT_VARIABLE("GlowPlayersColor_B", cvars.glow_players_color[2]);
		INI_EXPORT_VARIABLE("GlowEntitiesColor_R", cvars.glow_entities_color[0]);
		INI_EXPORT_VARIABLE("GlowEntitiesColor_G", cvars.glow_entities_color[1]);
		INI_EXPORT_VARIABLE("GlowEntitiesColor_B", cvars.glow_entities_color[2]);
		INI_EXPORT_VARIABLE("GlowViewModelColor_R", cvars.glow_viewmodel_color[0]);
		INI_EXPORT_VARIABLE("GlowViewModelColor_G", cvars.glow_viewmodel_color[1]);
		INI_EXPORT_VARIABLE("GlowViewModelColor_B", cvars.glow_viewmodel_color[2]);
	INI_EXPORT_END_SECTION();

	INI_EXPORT_BEGIN_SECTION("STRAFE");
		INI_EXPORT_VARIABLE("Enable", cvars.strafe);
		INI_EXPORT_VARIABLE("IgnoreGround", cvars.strafe_ignore_ground);
		INI_EXPORT_VARIABLE("Direction", cvars.strafe_dir);
		INI_EXPORT_VARIABLE("Type", cvars.strafe_type);
	INI_EXPORT_END_SECTION();
	
	INI_EXPORT_BEGIN_SECTION("FAKELAG");
		INI_EXPORT_VARIABLE("Enable", cvars.fakelag);
		INI_EXPORT_VARIABLE("AdaptiveInterp", cvars.fakelag_adaptive_ex_interp);
		INI_EXPORT_VARIABLE("Type", cvars.fakelag_type);
		INI_EXPORT_VARIABLE("Move", cvars.fakelag_move);
		INI_EXPORT_VARIABLE("Limit", cvars.fakelag_limit);
		INI_EXPORT_VARIABLE("Variance", cvars.fakelag_variance);
	INI_EXPORT_END_SECTION();
	
	INI_EXPORT_BEGIN_SECTION("ANTIAFK");
		INI_EXPORT_VARIABLE("Type", cvars.antiafk);
		INI_EXPORT_VARIABLE("RotateCamera", cvars.antiafk_rotate_camera);
		INI_EXPORT_VARIABLE("StayWithinRange", cvars.antiafk_stay_within_range);
		INI_EXPORT_VARIABLE("RotationAngle", cvars.antiafk_rotation_angle);
		INI_EXPORT_VARIABLE("StayRadius", cvars.antiafk_stay_radius);
		INI_EXPORT_VARIABLE("StayRadiusOffsetAngle", cvars.antiafk_stay_radius_offset_angle);
	INI_EXPORT_END_SECTION();

	INI_EXPORT_BEGIN_SECTION("MISC");
		INI_EXPORT_VARIABLE("AutoJump", cvars.autojump);
		INI_EXPORT_VARIABLE("JumpBug", cvars.jumpbug);
		INI_EXPORT_VARIABLE("JumpBugMinHeight", cvars.jumpbug_min_height);
		INI_EXPORT_VARIABLE("DoubleDuck", cvars.doubleduck);
		INI_EXPORT_VARIABLE("FastRun", cvars.fastrun);
		INI_EXPORT_VARIABLE("QuakeGuns", cvars.quake_guns);
		INI_EXPORT_VARIABLE("TertiaryAttackGlitch", cvars.tertiary_attack_glitch);
		INI_EXPORT_VARIABLE("SaveSoundcache", cvars.save_soundcache);
		INI_EXPORT_VARIABLE("RotateDeadBody", cvars.rotate_dead_body);
		INI_EXPORT_VARIABLE("NoWeaponAnim", cvars.no_weapon_anim);
		INI_EXPORT_VARIABLE("ColorPulsator", cvars.color_pulsator);
		INI_EXPORT_VARIABLE("ColorPulsatorTop", cvars.color_pulsator_top);
		INI_EXPORT_VARIABLE("ColorPulsatorBottom", cvars.color_pulsator_bottom);
		INI_EXPORT_VARIABLE("ColorPulsatorDelay", cvars.color_pulsator_delay);
		INI_EXPORT_VARIABLE("Spinner", cvars.spinner);
		INI_EXPORT_VARIABLE("SpinnerRotatePitchAngle", cvars.spinner_rotate_pitch_angle);
		INI_EXPORT_VARIABLE("SpinnerPitchAngle", cvars.spinner_pitch_angle);
		INI_EXPORT_VARIABLE("SpinnerRotationPitchAngle", cvars.spinner_rotation_pitch_angle);
		INI_EXPORT_VARIABLE("SpinnerRotationYawAngle", cvars.spinner_rotation_yaw_angle);
		INI_EXPORT_VARIABLE("ApplicationSpeed", cvars.application_speed);
	INI_EXPORT_END_SECTION();

	INI_EXPORT_BEGIN_SECTION("KEYSPAM");
		INI_EXPORT_VARIABLE("HoldMode", cvars.keyspam_hold_mode);
		INI_EXPORT_VARIABLE("Spam_E", cvars.keyspam_e);
		INI_EXPORT_VARIABLE("Spam_W", cvars.keyspam_w);
		INI_EXPORT_VARIABLE("Spam_S", cvars.keyspam_s);
		INI_EXPORT_VARIABLE("Spam_Q", cvars.keyspam_q);
		INI_EXPORT_VARIABLE("Spam_CTRL", cvars.keyspam_ctrl);
	INI_EXPORT_END_SECTION();
	
	INI_EXPORT_BEGIN_SECTION("FOG");
		INI_EXPORT_VARIABLE("Enable", cvars.fog);
		INI_EXPORT_VARIABLE("FogSkybox", cvars.fog_skybox);
		INI_EXPORT_VARIABLE("RemoveInWater", cvars.remove_water_fog);
		INI_EXPORT_VARIABLE("Start", cvars.fog_start);
		INI_EXPORT_VARIABLE("End", cvars.fog_end);
		INI_EXPORT_VARIABLE("Density", cvars.fog_density);
		INI_EXPORT_VARIABLE("Fog_R", cvars.fog_color[0]);
		INI_EXPORT_VARIABLE("Fog_G", cvars.fog_color[1]);
		INI_EXPORT_VARIABLE("Fog_B", cvars.fog_color[2]);
	INI_EXPORT_END_SECTION();

	INI_EXPORT_BEGIN_SECTION("SKYBOX");
		INI_EXPORT_VARIABLE("Type", cvars.skybox);
	INI_EXPORT_END_SECTION();
	
	INI_EXPORT_BEGIN_SECTION("CHATCOLORS");
		INI_EXPORT_VARIABLE("PlayerName_R", cvars.player_name_color[0]);
		INI_EXPORT_VARIABLE("PlayerName_G", cvars.player_name_color[1]);
		INI_EXPORT_VARIABLE("PlayerName_B", cvars.player_name_color[2]);
		INI_EXPORT_VARIABLE("RainbowUpdateDelay", cvars.chat_rainbow_update_delay);
		INI_EXPORT_VARIABLE("RainbowHueDelta", cvars.chat_rainbow_hue_delta);
		INI_EXPORT_VARIABLE("RainbowSaturation", cvars.chat_rainbow_saturation);
		INI_EXPORT_VARIABLE("RainbowLightness", cvars.chat_rainbow_lightness);
		INI_EXPORT_VARIABLE("ColorOne_R", cvars.chat_color_one[0]);
		INI_EXPORT_VARIABLE("ColorOne_G", cvars.chat_color_one[1]);
		INI_EXPORT_VARIABLE("ColorOne_B", cvars.chat_color_one[2]);
		INI_EXPORT_VARIABLE("ColorTwo_R", cvars.chat_color_two[0]);
		INI_EXPORT_VARIABLE("ColorTwo_G", cvars.chat_color_two[1]);
		INI_EXPORT_VARIABLE("ColorTwo_B", cvars.chat_color_two[2]);
		INI_EXPORT_VARIABLE("ColorThree_R", cvars.chat_color_three[0]);
		INI_EXPORT_VARIABLE("ColorThree_G", cvars.chat_color_three[1]);
		INI_EXPORT_VARIABLE("ColorThree_B", cvars.chat_color_three[2]);
		INI_EXPORT_VARIABLE("ColorFour_R", cvars.chat_color_four[0]);
		INI_EXPORT_VARIABLE("ColorFour_G", cvars.chat_color_four[1]);
		INI_EXPORT_VARIABLE("ColorFour_B", cvars.chat_color_four[2]);
		INI_EXPORT_VARIABLE("ColorFive_R", cvars.chat_color_five[0]);
		INI_EXPORT_VARIABLE("ColorFive_G", cvars.chat_color_five[1]);
		INI_EXPORT_VARIABLE("ColorFive_B", cvars.chat_color_five[2]);
	INI_EXPORT_END_SECTION();

	INI_EXPORT_BEGIN_SECTION("CAMHACK");
		INI_EXPORT_VARIABLE("SpeedFactor", cvars.camhack_speed_factor);
		INI_EXPORT_VARIABLE("ShowModel", cvars.camhack_show_model);
	INI_EXPORT_END_SECTION();
	
	INI_EXPORT_BEGIN_SECTION("FPROAMING");
		INI_EXPORT_VARIABLE("Enable", cvars.fp_roaming);
		INI_EXPORT_VARIABLE("Crosshair", cvars.fp_roaming_draw_crosshair);
		INI_EXPORT_VARIABLE("Lerp", cvars.fp_roaming_lerp);
		INI_EXPORT_VARIABLE("LerpValue", cvars.fp_roaming_lerp_value);
	INI_EXPORT_END_SECTION();

	INI_EXPORT_BEGIN_SECTION("VOTEPOPUP");
		INI_EXPORT_VARIABLE("Enable", cvars.vote_popup);
		INI_EXPORT_VARIABLE("WidthSize", cvars.vote_popup_width_size);
		INI_EXPORT_VARIABLE("HeightSize", cvars.vote_popup_height_size);
		INI_EXPORT_VARIABLE("WidthBorderPixels", cvars.vote_popup_w_border_pix);
		INI_EXPORT_VARIABLE("HeightBorderPixels", cvars.vote_popup_h_border_pix);
		INI_EXPORT_VARIABLE("WidthFraction", cvars.vote_popup_width_frac);
		INI_EXPORT_VARIABLE("HeightFraction", cvars.vote_popup_height_frac);
	INI_EXPORT_END_SECTION();

	//INI_EXPORT_BEGIN_SECTION("AUTOVOTE");
	//	INI_EXPORT_VARIABLE("Mode", cvars.autovote_mode);
	//	INI_EXPORT_VARIABLE("UseOnCustomVotes", cvars.autovote_custom);
	//	INI_EXPORT_VARIABLE("IgnoreVoteFilter", cvars.autovote_ignore_filter);
	//INI_EXPORT_END_SECTION();
	
	INI_EXPORT_BEGIN_SECTION("AMS");
		INI_EXPORT_VARIABLE("MuteEverything", cvars.ams_mute_everything);
	INI_EXPORT_END_SECTION();

	INI_EXPORT_END()
}