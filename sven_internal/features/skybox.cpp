// Skybox

#include "skybox.h"

#include "../patterns.h"
#include "signature_scanner.h"
#include "trampoline_hook.h"

#include "../sdk.h"
#include "../config.h"
#include "../interfaces.h"

#include "../game/utils.h"
#include "../game/console.h"

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef int (*R_LoadSkyboxIntFn)(const char *pszSkyboxName);

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;

//-----------------------------------------------------------------------------
// Declare Hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(R_LoadSkyboxInt_Hook);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CSkybox g_Skybox;
R_LoadSkyboxIntFn R_LoadSkyboxInt_Original = NULL;

bool g_bMenuChangeSkybox = false;

static bool s_bLoadingSkybox = false;
static bool s_bSkyboxLoaded = false;

const char *g_szSkyboxes[] =
{
	"-",
	"desert",
	"2desert",
	"desnoon",
	"morning",
	"cliff",
	"dfcliff",
	"dustbowl",
	"sandstone",
	"sky_blu_",
	"sky16",
	"sky35",
	"sky45",
	"tornsky",
	"twildes",
	"crashsite",
	"doom1",
	"dusk",
	"fodrian",
	"night",
	"carnival",
	"theyh2",
	"theyh3",
	"thn",
	"forest512_",
	"tetris",
	"2vs",
	"ac_",
	"arcn",
	"black",
	"coliseum",
	"gmcity",
	"grassy",
	"toon",
	"parallax-errorlf256_",
	"necros-hell256_",
	"space",
	"hplanet",
	"vreality_sky",
	"neb1",
	"neb2b",
	"neb6",
	"neb7",
	"alien1",
	"alien2",
	"alien3",
	"xen8",
	"xen9",
	"xen10"
};

int g_iSkyboxesSize = int(sizeof(g_szSkyboxes) / sizeof(*g_szSkyboxes));

//-----------------------------------------------------------------------------
// ConCommands, CVars..
//-----------------------------------------------------------------------------

CON_COMMAND_FUNC(sc_change_skybox, ConCommand_ChangeSkybox, "sc_change_skybox [name] - Change skybox by given name")
{
	if (g_bMenuChangeSkybox || CMD_ARGC() > 1)
	{
		if (g_bMenuChangeSkybox && g_Config.cvars.skybox == 0)
		{
			Msg("Choose skybox name first\n");
			return;
		}

		const char *pszSkyboxName = g_bMenuChangeSkybox ? g_szSkyboxes[g_Config.cvars.skybox] : CMD_ARGV(1);

		if (*pszSkyboxName)
			g_Skybox.Replace(pszSkyboxName);
	}
	else
	{
		sc_change_skybox.PrintUsage();
	}
}

CON_COMMAND_FUNC(sc_reset_skybox, ConCommand_ResetSkybox, "sc_reset_skybox - Reset skybox to the default")
{
	if (g_Config.cvars.skybox != 0)
		g_Config.cvars.skybox = 0;

	g_Skybox.Reset();
}

CON_COMMAND(sc_change_skybox_color, "sc_change_skybox_color [r] [g] [b] - Change skybox's color (RGB: color range from 0 to 1)")
{
	if (CMD_ARGC() > 3)
	{
		float r = strtof(CMD_ARGV(1), NULL);
		float g = strtof(CMD_ARGV(2), NULL);
		float b = strtof(CMD_ARGV(3), NULL);

		g_pEngineClient->ChangeSkycolor(r, g, b);
	}
	else
	{
		sc_change_skybox_color.PrintUsage();
	}
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

int __cdecl R_LoadSkyboxInt_Hooked(const char *pszSkyboxName)
{
	int loaded = R_LoadSkyboxInt_Original(pszSkyboxName);

	if (s_bLoadingSkybox)
	{
		s_bSkyboxLoaded = loaded;
	}
	else
	{
		g_Skybox.SaveOriginalSkybox(pszSkyboxName);
	}

	return loaded;
}

//-----------------------------------------------------------------------------
// Think function
//-----------------------------------------------------------------------------

void CSkybox::Think()
{
	if (m_bSkyboxReplaced && g_pPlayerMove && g_pPlayerMove->movevars)
	{
		if (m_flNextThinkTime > g_pEngineFuncs->GetClientTime())
			return;
		else
			m_flNextThinkTime = g_pEngineFuncs->GetClientTime() + 0.25f;

		if ( memcmp( m_szSkyboxName, m_szCurrentSkyboxName, sizeof(m_szCurrentSkyboxName) ) )
		{
			s_bLoadingSkybox = true;

			g_pEngineClient->ChangeSkymap(m_szSkyboxName);

			s_bLoadingSkybox = false;

			if (!s_bSkyboxLoaded)
			{
				m_bSkyboxReplaced = false;

				*m_szSkyboxName = 0;
				*m_szCurrentSkyboxName = 0;
				*m_szOriginalSkyboxName = 0;
			}
			else
			{
				strcpy_s(m_szOriginalSkyboxName, sizeof(movevars_s::skyName), g_pPlayerMove->movevars->skyName);
				strcpy_s(m_szCurrentSkyboxName, sizeof(m_szCurrentSkyboxName), m_szSkyboxName);
			}
		}
	}
}

void CSkybox::Replace(const char *pszSkyboxName)
{
	strcpy_s(m_szSkyboxName, sizeof(m_szSkyboxName), pszSkyboxName);
	
	*m_szCurrentSkyboxName = 0;
	m_bSkyboxReplaced = true;
}

void CSkybox::Reset()
{
	if (m_bSkyboxReplaced && g_pPlayerMove && g_pPlayerMove->movevars && *m_szOriginalSkyboxName)
	{
		g_pEngineClient->ChangeSkymap(m_szOriginalSkyboxName);
	}

	*m_szSkyboxName = 0;
	*m_szOriginalSkyboxName = 0;
	*m_szCurrentSkyboxName = 0;

	m_bSkyboxReplaced = false;
}

void CSkybox::SaveOriginalSkybox(const char *pszSkyboxName)
{
	strcpy_s(m_szOriginalSkyboxName, sizeof(m_szOriginalSkyboxName), pszSkyboxName);
	strcpy_s(m_szCurrentSkyboxName, sizeof(m_szCurrentSkyboxName), pszSkyboxName);
}

//-----------------------------------------------------------------------------
// Init
//-----------------------------------------------------------------------------

void CSkybox::OnConfigLoad()
{
	if (g_Config.cvars.skybox > 0 && g_Config.cvars.skybox < g_iSkyboxesSize)
	{
		Replace(g_szSkyboxes[g_Config.cvars.skybox]);
	}
}

void CSkybox::OnVideoInit()
{
	m_flNextThinkTime = -1.0f;
}

void CSkybox::Init()
{
	HMODULE hHardwareDLL = GetModuleHandle(L"hw.dll");

	void *pR_LoadSkyboxInt = FindPattern(hHardwareDLL, Patterns::Hardware::R_LoadSkyboxInt);

	if (!pR_LoadSkyboxInt)
	{
		Sys_Error("'R_LoadSkyboxInt' failed initialization");
		return;
	}

	HOOK_FUNCTION(R_LoadSkyboxInt_Hook, pR_LoadSkyboxInt, R_LoadSkyboxInt_Hooked, R_LoadSkyboxInt_Original, R_LoadSkyboxIntFn);
}