// Anti-AFK

#pragma once

class CAntiAFK
{
public:
	void Init();

	void CreateMove(float frametime, struct usercmd_s *cmd, int active);

private:
	void AntiAFK(struct usercmd_s *cmd);
};

extern CAntiAFK g_AntiAFK;