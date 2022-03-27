#ifndef VISUAL_H
#define VISUAL_H

#ifdef _WIN32
#pragma once
#endif

#include <base_feature.h>
#include <IHooks.h>

class CVisual : public CBaseFeature
{
public:
	CVisual();

	virtual bool Load();
	virtual void PostLoad();

	virtual void Unload();

public:
	void Process();
	bool StudioRenderModel();

	void OnVideoInit();
	void OnHUDRedraw(float flTime);

	void ResetJumpSpeed();

private:
	void ESP();
	void DrawCrosshair();
	void ShowSpeed();
	void Lightmap();

	void ProcessBones();

private:
	float m_flTime;

	float m_flPrevTime;
	float m_flFadeTime;
	float m_flJumpSpeed;

	int m_clFadeFrom[3];

	bool m_bOnGround;

	DetourHandle_t m_hUserMsgHook_ScreenShake;
	DetourHandle_t m_hUserMsgHook_ScreenFade;

	int m_iScreenWidth;
	int m_iScreenHeight;
};

extern CVisual g_Visual;

#endif // VISUAL_H