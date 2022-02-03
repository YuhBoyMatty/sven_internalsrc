// First-Person Roaming

#pragma once

#include "../sdk.h"

class CFirstPersonRoaming
{
public:
	CFirstPersonRoaming();

	bool StudioRenderModel();

	void V_CalcRefdef(struct ref_params_s *pparams);
	cl_entity_s *GetTargetPlayer();

private:
	void GetPlayerViewAngles(Vector &vOutput);

private:
	int m_iTarget;
	int m_iSpectatorMode;

	cl_entity_s *m_pTarget;
	Vector m_vPrevAngles;
};

extern CFirstPersonRoaming g_FirstPersonRoaming;