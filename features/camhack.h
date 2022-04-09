#ifndef CAMHACK_H
#define CAMHACK_H

#ifdef _WIN32
#pragma once
#endif

#include <math/vector.h>

class CCamHack
{
public:
	CCamHack();

	void Init();

public:
	void Enable();
	void Disable();

	bool IsEnabled() const;

public:
	void SetFirstPersonState(bool bEnable);
	void SetThirdPersonState(bool bEnable);

	bool MustEnableFirstPerson() const;
	bool MustEnableThirdPerson() const;

	void ResetRollAxis();
	void ResetOrientation();

	// Callbacks
	bool StudioRenderModel();

	void OnVideoInit();

	bool OnKeyPress(int down, int keynum);

	void CreateMove(float frametime, struct usercmd_s *cmd, int active);
	void V_CalcRefdef(struct ref_params_s *pparams);

public:
	float m_flSavedPitchAngle;

	Vector m_vecCameraOrigin;
	Vector m_vecCameraAngles;

	Vector m_vecViewAngles;

private:
	bool m_bEnabled;

	bool m_bChangeCameraState;
	bool m_bChangeToThirdPerson;
};

inline bool CCamHack::IsEnabled() const { return m_bEnabled; }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern CCamHack g_CamHack;

#endif // CAMHACK_H