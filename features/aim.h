#ifndef AIM_H
#define AIM_H

#ifdef _WIN32
#pragma once
#endif

#include <hl_sdk/common/usercmd.h>
#include <hl_sdk/common/ref_params.h>

#include <base_feature.h>

//-----------------------------------------------------------------------------
// Aim Feature
//-----------------------------------------------------------------------------

class CAim : public CBaseFeature
{
public:
	virtual bool Load();
	virtual void PostLoad();

	virtual void Unload();

public:
	CAim();

	void CreateMove(float frametime, usercmd_t *cmd, int active);

	void Pre_V_CalcRefdef(ref_params_t *pparams);
	void Post_V_CalcRefdef(ref_params_t *pparams);

private:
	void NoRecoil(usercmd_t *cmd);

private:
	void *m_pfnV_PunchAxis;

	Vector m_vecPunchAngle;
	Vector m_vecEVPunchAngle;
};

extern CAim g_Aim;

#endif // AIM_H