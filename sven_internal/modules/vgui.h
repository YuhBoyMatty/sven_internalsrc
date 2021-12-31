// VGUI Module

#pragma once

#include "utils/vgui2/IPanel.h"
#include "utils/vgui2/ISurface.h"

//-----------------------------------------------------------------------------

typedef void (__thiscall *PaintTraverseFn)(void *thisptr, vgui::IPanel *vguiPanel, bool forceRepaint, bool allowForce);

//-----------------------------------------------------------------------------

void InitVGUIModule();

void ReleaseVGUIModule();