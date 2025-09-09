// Player Utils

#include "player_utils.h"

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern extra_player_info_t *g_pPlayerExtraInfo;

//-----------------------------------------------------------------------------
// Util functions
//-----------------------------------------------------------------------------

float GetPlayerHealth(int index)
{
	return *(float *)((unsigned char *)g_pPlayerExtraInfo + sizeof(extra_player_info_t) * index + 0x42);
}

float GetPlayerArmor(int index)
{
	return *(float *)((unsigned char *)g_pPlayerExtraInfo + sizeof(extra_player_info_t) * index + 0x46);
}