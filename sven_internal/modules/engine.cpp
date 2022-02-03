// Engine Module

#include "engine.h"

#include "../sdk.h"
#include "../config.h"
#include "../patterns.h"
#include "../interfaces.h"

#include "../game/console.h"
#include "../game/utils.h"

#include "../utils/signature_scanner.h"
#include "../utils/trampoline_hook.h"

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef qboolean (*Netchan_CanPacketFn)(netchan_t *);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

netchan_t *g_pNetchan = NULL;

bool bSendPacket = true;

//-----------------------------------------------------------------------------
// Init hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(Netchan_CanPacket_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

Netchan_CanPacketFn Netchan_CanPacket_Original = NULL;

//-----------------------------------------------------------------------------
// ConCommands, CVars..
//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(toggle, Toggle_Cmd, "toggle [cvar_name] [value #1] [value #2] [value #3].. - Toggle between values")
{
	int i;
	int argc = CMD_ARGC();

	if (argc > 3)
	{
		const char *pszCvar = CMD_ARGV(1);
		cvar_s *pCvar = g_pEngineFuncs->pfnGetCvarPointer(pszCvar);

		if (pCvar)
		{
			for (i = 2; i < argc; i++)
			{
				if ( !strcmp(pCvar->string, CMD_ARGV(i)) )
					break;
			}

			i++;

			if (i >= argc)
			{
				i = 2;
			}

			g_pEngineFuncs->pfnCvar_Set(pszCvar, CMD_ARGV(i));
		}
	}
	else
	{
		toggle.PrintUsage();
	}
}

CON_COMMAND(getang, "getang - Get view angles")
{
	Vector va;

	g_pEngineFuncs->GetViewAngles(va);

	Msg("View angles: %.3f %.3f %.3f", va.x, va.y, va.z);
}

CON_COMMAND(setang, "setang [pitch] [optional: yaw] [optional: roll] - Set view angles")
{
	if (CMD_ARGC() > 1)
	{
		Vector va;

		g_pEngineFuncs->GetViewAngles(va);

		va.x = strtof(CMD_ARGV(1), NULL);

		if (CMD_ARGC() > 2)
			va.y = strtof(CMD_ARGV(2), NULL);
		
		if (CMD_ARGC() > 3)
			va.z = strtof(CMD_ARGV(3), NULL);

		g_pEngineFuncs->SetViewAngles(va);
	}
	else
	{
		setang.PrintUsage();
	}
}

CON_COMMAND_FUNC(sc_load_config, ReloadConfig_Cmd, "sc_load_config - Load config from file sven_internal.ini")
{
	g_Config.Load();
}

CON_COMMAND_FUNC(sc_save_config, SaveConfig_Cmd, "sc_save_config - Save config to file sven_internal.ini")
{
	g_Config.Save();
}

CON_COMMAND(test, "test [entidx] - Retrieves an entity info")
{
	if (CMD_ARGC() >= 2)
	{
		int index = atoi(CMD_ARGV(1));

		cl_entity_s *pEntity = g_pEngineFuncs->GetEntityByIndex(index);

		if (pEntity)
		{
			Msg("Entity Pointer: %X\n", pEntity);

			if (pEntity->player)
			{
				Msg("Player Info Pointer: %X\n", g_pEngineStudio->PlayerInfo(index - 1));

				hud_player_info_t playerInfo;
				ZeroMemory(&playerInfo, sizeof(hud_player_info_s));

				g_pEngineFuncs->pfnGetPlayerInfo(index, &playerInfo);

				if (playerInfo.name && playerInfo.model && *playerInfo.model)
					Msg("Model: %s\n", playerInfo.model);

				Msg("Top Color: %d\n", playerInfo.topcolor);
				Msg("Bottom Color: %d\n", playerInfo.bottomcolor);
			}
			else if (pEntity->model && pEntity->model->name)
			{
				Msg("Model: %s\n", pEntity->model->name);
			}
		}
	}
	else
	{
		test.PrintUsage();
	}
};

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

qboolean Netchan_CanPacket_Hooked(netchan_t *chan)
{
	if (!bSendPacket)
		return 0;

	return Netchan_CanPacket_Original(chan);
}

//-----------------------------------------------------------------------------
// Init/release engine module
//-----------------------------------------------------------------------------

void InitEngineModule()
{
	//void *pNetchan_TransmitString = LookupForString(L"hw.dll", "%s:Outgoing");
	//void *pNetchan_Transmit = FindAddress(L"hw.dll", pNetchan_TransmitString);

	//pNetchan_Transmit = (void *)((BYTE *)pNetchan_Transmit - 0x75);

	void *pNetchan_CanPacket = FIND_PATTERN(L"hw.dll", Patterns::Hardware::Netchan_CanPacket);

	if (!pNetchan_CanPacket)
	{
		Sys_Error("'Netchan_CanPacket' failed initialization\n");
		return;
	}

	HOOK_FUNCTION(Netchan_CanPacket_Hook, pNetchan_CanPacket, Netchan_CanPacket_Hooked, Netchan_CanPacket_Original, Netchan_CanPacketFn);

	REGISTER_COMMAND("sc_help", PrintConsoleHelp);
}

void ShutdownEngineModule()
{
}