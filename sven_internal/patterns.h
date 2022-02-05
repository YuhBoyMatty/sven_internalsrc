// Patterns

#pragma once

#include "utils/patterns_base.h"

namespace Patterns
{
	namespace Interfaces
	{
		INIT_PATTERN(EngineFuncs);
		INIT_PATTERN(ClientFuncs);
		INIT_PATTERN(EngineStudio);
	}

	namespace Hardware
	{
		INIT_PATTERN(flNextCmdTime);

		INIT_PATTERN(Netchan_CanPacket);

		INIT_PATTERN(V_RenderView);
		INIT_PATTERN(V_SetupFrame);
		INIT_PATTERN(R_LoadSkyboxInt);
	}

	namespace Client
	{
		INIT_PATTERN(CVotePopup__MsgFunc_VoteMenu);
		INIT_PATTERN(READ_BYTE);
		INIT_PATTERN(READ_STRING);
		
		INIT_PATTERN(CHudBaseTextBlock__Print);

		INIT_PATTERN(CVoiceBanMgr__SaveState);
		INIT_PATTERN(CVoiceBanMgr__SetPlayerBan);
		INIT_PATTERN(CVoiceBanMgr__InternalFindPlayerSquelch);

		INIT_PATTERN(CVoiceStatus__IsPlayerBlocked);
		INIT_PATTERN(CVoiceStatus__SetPlayerBlockedState);
		INIT_PATTERN(CVoiceStatus__UpdateServerState);

		INIT_PATTERN(HACK_GetPlayerUniqueID);

		INIT_PATTERN(GetClientColor);

		INIT_PATTERN(WeaponsResource__GetFirstPos);
	}

	namespace GameOverlay
	{
		INIT_PATTERN(SetCursorPos_Hook);
		INIT_PATTERN(ValveUnhookFunc);
	}
}