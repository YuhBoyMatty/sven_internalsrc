#ifndef IENGINEVGUI_H
#define IENGINEVGUI_H
#ifdef _WIN32
#pragma once
#endif

#include <common/interface.h>
#include "IPanel.h"

class TSteamProgress;
class TSteamError;

namespace vgui
{
	enum VGUIPANEL
	{
		PANEL_ROOT = 0 ,
		PANEL_CLIENTDLL ,
		PANEL_GAMEUIDLL
	};

	class IEngineVGui : public IBaseInterface
	{
	public:
		virtual vgui::IPanel* GetPanel( VGUIPANEL type ) = 0;
		
		// Sven Co-op specific
		virtual bool SteamRefreshLogin( const char *a1, boola2 ) = 0;
		virtual bool SteamProcessCall( bool *a1, TSteamProgress *a2, TSteamError *a3 ) = 0;
	};
}

extern vgui::IEngineVGui* g_pIEngineVGui;

#define VENGINE_VGUI_VERSION "VEngineVGui001"
#endif