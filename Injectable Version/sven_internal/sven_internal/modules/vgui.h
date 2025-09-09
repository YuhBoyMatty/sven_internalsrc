// VGUI Module

#pragma once

#include "utils/vgui2/include/IPanel.h"
#include "utils/vgui2/include/ISurface.h"
#include "utils/vgui2/include/ILocalize.h"
#include "utils/vgui2/include/IScheme.h"

//-----------------------------------------------------------------------------
// Struct declarations
//-----------------------------------------------------------------------------

struct screen_info_s
{
	int width;
	int height;
};

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern screen_info_s g_ScreenInfo;

extern vgui::IPanel *g_pPanel;
extern vgui::ISurface *g_pSurface;
extern vgui::ILocalize *g_pLocalize;
extern vgui::ISchemeManager *g_pSchemeManager;

inline vgui::IPanel *panel()
{
	return g_pPanel;
}

inline vgui::ISurface *surface()
{
	return g_pSurface;
}

inline vgui::ILocalize *localize()
{
	return g_pLocalize;
}

inline vgui::ISchemeManager *schememanager()
{
	return g_pSchemeManager;
}

//-----------------------------------------------------------------------------
// Init/shutdown VGUI module
//-----------------------------------------------------------------------------

void InitVGUIModule();

void ShutdownVGUIModule();