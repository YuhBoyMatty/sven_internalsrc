#ifndef MISC_FEATURES_H
#define MISC_FEATURES_H

#ifdef _WIN32
#pragma once
#endif

#include <unordered_map>
#include <string>

#include <IDetoursAPI.h>
#include <base_feature.h>

typedef std::unordered_map<std::string, std::string> CmdMap;

class CMisc : public CBaseFeature
{
public:
	CmdMap m_OnTickCommands;

public:
	CMisc();
	~CMisc();

	virtual bool Load();
	virtual void PostLoad();

	virtual void Unload();

public:
	void CreateMove(float frametime, struct usercmd_s *cmd, int active);
	void V_CalcRefdef(struct ref_params_s *pparams);
	void HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed);
	void OnAddEntityPost(int is_visible, int type, struct cl_entity_s *ent, const char *modelname);
	void OnVideoInit();

private:
	void AutoJump(struct usercmd_s *cmd);
	void JumpBug(float frametime, struct usercmd_s *cmd);
	void EdgeJump(float frametime, struct usercmd_s *cmd);
	void DoubleDuck(struct usercmd_s *cmd);
	void FastRun(struct usercmd_s *cmd);
	void Spinner(struct usercmd_s *cmd);
	void Stick(struct usercmd_s *cmd);
	void OneTickExploit(struct usercmd_s *cmd);
	
	void AutoCeilClipping(struct usercmd_s *cmd);
	void FakeLag(float frametime);
	void AutoSelfSink();
	void TertiaryAttackGlitch();

	void ColorPulsator();

	void QuakeGuns_V_CalcRefdef();
	void QuakeGuns_HUD_PostRunCmd(struct local_state_s *to);

	void NoWeaponAnim_HUD_PostRunCmd(struct local_state_s *to);

private:
	void *m_pfnQueryPerformanceCounter;
	void *m_pfnNetchan_Transmit;
	void *m_pfnCHud__Think;
	
	DetourHandle_t m_hQueryPerformanceCounter;
	DetourHandle_t m_hNetchan_Transmit;
	DetourHandle_t m_hCHud__Think;

private:
	float m_flSpinPitchAngle;
	bool m_bSpinCanChangePitch;

	int m_iFakeLagCounter;
	int m_iOneTickExploitLagInterval;
};

extern CMisc g_Misc;

#endif // MISC_FEATURES_H