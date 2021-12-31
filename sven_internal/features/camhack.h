// Cam Hack

#pragma once

//-----------------------------------------------------------------------------

typedef void (*GetViewInfoFn)(float *, float *, float *, float *);

//-----------------------------------------------------------------------------

class CCamHack
{
public:
	void Init();

	void CreateMove(float frametime, struct usercmd_s *cmd, int active);
	void V_CalcRefdef(struct ref_params_s *pparams);
};

extern CCamHack g_CamHack;