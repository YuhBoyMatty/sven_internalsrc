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

private:
	void ESP();
	void DrawCrosshair();
	void ShowSpeed();
	void Lightmap();

	void ProcessBones();

private:
	DetourHandle_t m_hUserMsgHook_ScreenShake;
	DetourHandle_t m_hUserMsgHook_ScreenFade;

	int m_iScreenWidth;
	int m_iScreenHeight;
};

extern CVisual g_Visual;

#endif // VISUAL_H