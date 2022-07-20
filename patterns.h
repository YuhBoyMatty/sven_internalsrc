#ifndef PATTERNS_H
#define PATTERNS_H

#ifdef _WIN32
#pragma once
#endif

#include <memutils/patterns.h>

namespace Patterns
{
	namespace Hardware
	{
		EXTERN_PATTERN(flNextCmdTime);

		EXTERN_PATTERN(Netchan_CanPacket);
		EXTERN_PATTERN(Netchan_Transmit);

		EXTERN_PATTERN(V_RenderView);
		EXTERN_PATTERN(R_SetupFrame);
		EXTERN_PATTERN(R_LoadSkyboxInt);

		EXTERN_PATTERN(CRC_MapFile);
		EXTERN_PATTERN(UserInfo_Offset);
	}

	namespace Client
	{
		EXTERN_PATTERN(IN_Move);

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

		EXTERN_PATTERN(ScaleColors);
		EXTERN_PATTERN(ScaleColors_RGBA);

		EXTERN_PATTERN(CHud__Think);

		EXTERN_PATTERN(V_PunchAxis);

		EXTERN_PATTERN(WeaponsResource__GetFirstPos);
	}

	namespace GameOverlay
	{
		EXTERN_PATTERN(SetCursorPos_Hook);
		EXTERN_PATTERN(ValveUnhookFunc);
	}
}

#endif // PATTERNS_H