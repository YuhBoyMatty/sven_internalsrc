// VGUI Module

#include "vgui.h"

#include "../sdk.h"
#include "../interfaces.h"

#include "../utils/vtable_hook.h"

#include "../game/drawing.h"
#include "../features/visual.h"

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

vgui::IPanel *g_pPanel = NULL;
vgui::ISurface *g_pSurface = NULL;

PaintTraverseFn PaintTraverse_Original = NULL;

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

void __fastcall PaintTraverse_Hooked(void *thisptr, int edx, vgui::IPanel *vguiPanel, bool forceRepaint, bool allowForce)
{
	static vgui::IPanel *panel = NULL;

	if (!panel)
	{
		const char *pszPanelName = g_pPanel->GetName(vguiPanel);

		if (!strcmp(pszPanelName, "StaticPanel"))
			panel = vguiPanel;
	}
	else if (panel == vguiPanel && g_pEngineFuncs->GetLocalPlayer())
	{
		g_Visual.Process();
	}

	PaintTraverse_Original(thisptr, vguiPanel, forceRepaint, allowForce);
}

//-----------------------------------------------------------------------------
// Init/release VGUI module
//-----------------------------------------------------------------------------

void InitVGUIModule()
{
	CreateInterfaceFn vgui2Factory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(GetModuleHandle(L"vgui2.dll"), "CreateInterface"));
	CreateInterfaceFn hardwareFactory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(GetModuleHandle(L"hw.dll"), "CreateInterface"));

	g_pPanel = reinterpret_cast<vgui::IPanel *>(vgui2Factory(VGUI_PANEL_INTERFACE_VERSION, NULL));
	g_pSurface = reinterpret_cast<vgui::ISurface *>(hardwareFactory(VGUI_SURFACE_INTERFACE_VERSION, NULL));

	PaintTraverse_Original = (PaintTraverseFn)CVTableHook::HookFunction(g_pPanel, PaintTraverse_Hooked, 41);

	g_Drawing.SetupFonts();

	g_Visual.Init();
}

void ReleaseVGUIModule()
{
}