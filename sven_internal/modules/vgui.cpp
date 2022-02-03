// VGUI Module

#include "vgui.h"

#include "../sdk.h"
#include "../interfaces.h"

#include "../utils/vtable_hook.h"

#include "../game/utils.h"
#include "../game/drawing.h"

#include "../features/visual.h"
#include "../features/custom_vote_popup.h"

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef void (__thiscall *PaintTraverseFn)(void *thisptr, vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

screen_info_s g_ScreenInfo;

vgui::IPanel *g_pPanel = NULL;
vgui::ISurface *g_pSurface = NULL;
vgui::ILocalize *g_pLocalize = NULL;
vgui::ISchemeManager *g_pSchemeManager = NULL;

PaintTraverseFn PaintTraverse_Original = NULL;

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

void __fastcall PaintTraverse_Hooked(void *thisptr, int edx, vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	static vgui::VPANEL panel = 0;

	if (!panel)
	{
		const char *pszPanelName = g_pPanel->GetName(vguiPanel);

		if (!strcmp(pszPanelName, "StaticPanel"))
			panel = vguiPanel;
	}
	else if (panel == vguiPanel)
	{
		g_pSurface->GetScreenSize(g_ScreenInfo.width, g_ScreenInfo.height);

		if (g_pEngineFuncs->GetLocalPlayer())
		{
			g_Visual.Process();
			g_VotePopup.Draw();
		}
	}

	PaintTraverse_Original(thisptr, vguiPanel, forceRepaint, allowForce);
}

//-----------------------------------------------------------------------------
// Init/release VGUI module
//-----------------------------------------------------------------------------

void InitVGUIModule()
{
	HMODULE hVGUI2 = GetModuleHandle(L"vgui2.dll");
	HMODULE hHardware = GetModuleHandle(L"hw.dll");

	if (!hVGUI2)
	{
		Sys_Error("Failed to get vgui2's module\n");
		return;
	}
	
	if (!hHardware)
	{
		Sys_Error("Failed to get hw's module\n");
		return;
	}

	CreateInterfaceFn vgui2Factory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(hVGUI2, "CreateInterface"));
	CreateInterfaceFn hardwareFactory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(hHardware, "CreateInterface"));

	g_pPanel = reinterpret_cast<vgui::IPanel *>(vgui2Factory(VGUI_PANEL_INTERFACE_VERSION, NULL));
	g_pSurface = reinterpret_cast<vgui::ISurface *>(hardwareFactory(VGUI_SURFACE_INTERFACE_VERSION, NULL));
	g_pLocalize = reinterpret_cast<vgui::ILocalize *>(vgui2Factory(VGUI_LOCALIZE_INTERFACE_VERSION, NULL));
	g_pSchemeManager = reinterpret_cast<vgui::ISchemeManager *>(vgui2Factory(VGUI_SCHEME_INTERFACE_VERSION, NULL));

	PaintTraverse_Original = (PaintTraverseFn)CVTableHook::HookFunction(g_pPanel, PaintTraverse_Hooked, 41);

	g_Drawing.SetupFonts();

	g_Visual.Init();
}

void ShutdownVGUIModule()
{
}