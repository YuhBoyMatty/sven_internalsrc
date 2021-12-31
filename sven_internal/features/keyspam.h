// Key Spam

#pragma once

class CKeySpam
{
public:
	void Init();

	void CreateMove(float frametime, struct usercmd_s *cmd, int active);

private:
	void KeySpam();
};

extern CKeySpam g_KeySpam;