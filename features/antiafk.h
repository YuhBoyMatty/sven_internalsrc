#ifndef ANTIAFK_H
#define ANTIAFK_H

#ifdef _WIN32
#pragma once
#endif

#include <math/vector.h>

class CAntiAFK
{
public:
	CAntiAFK();

	void Reset();

	void OnVideoInit();
	void CreateMove(float frametime, struct usercmd_s *cmd, int active);

private:
	void AntiAFK(struct usercmd_s *cmd);
	void WalkAround(struct usercmd_s *cmd, int &delay, int &movement_button, const int delay_count);
	void RotateCamera();

	void OnRevive();
	void OnDie();

private:
	bool m_bDead;
	bool m_bComingBackToAFKPoint;

	Vector2D m_vecAFKPoint; // 2D point only
	float m_flComingBackStartTime;
};

extern CAntiAFK g_AntiAFK;

#endif // ANTIAFK_H