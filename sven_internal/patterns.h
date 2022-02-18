// Patterns

#pragma once

#include "utils/patterns_base.h"

namespace Patterns
{
	namespace Interfaces
	{
		EXTERN_PATTERN(EngineFuncs);
		EXTERN_PATTERN(ClientFuncs);
		EXTERN_PATTERN(EngineStudio);
	}

	namespace Hardware
	{
		EXTERN_PATTERN(flNextCmdTime);

		EXTERN_PATTERN(Netchan_CanPacket);

		EXTERN_PATTERN(V_RenderView);
		EXTERN_PATTERN(V_SetupFrame);
		EXTERN_PATTERN(R_LoadSkyboxInt);
	}

	namespace Client
	{
		EXTERN_PATTERN(CVotePopup__MsgFunc_VoteMenu);
		EXTERN_PATTERN(READ_BYTE);
		EXTERN_PATTERN(READ_STRING);
		
		EXTERN_PATTERN(CHudBaseTextBlock__Print);

		EXTERN_PATTERN(CVoiceBanMgr__SaveState);
		EXTERN_PATTERN(CVoiceBanMgr__SetPlayerBan);
		EXTERN_PATTERN(CVoiceBanMgr__InternalFindPlayerSquelch);

		EXTERN_PATTERN(CVoiceStatus__IsPlayerBlocked);
		EXTERN_PATTERN(CVoiceStatus__SetPlayerBlockedState);
		EXTERN_PATTERN(CVoiceStatus__UpdateServerState);

		EXTERN_PATTERN(HACK_GetPlayerUniqueID);

		EXTERN_PATTERN(GetClientColor);

		EXTERN_PATTERN(CHud__Think);

		EXTERN_PATTERN(WeaponsResource__GetFirstPos);
	}

	namespace GameOverlay
	{
		EXTERN_PATTERN(SetCursorPos_Hook);
		EXTERN_PATTERN(ValveUnhookFunc);
	}
}