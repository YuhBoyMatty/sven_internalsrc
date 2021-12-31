// Vectorial Strafer

#pragma once

class CStrafer
{
public:
	void Init();

	void CreateMove(float frametime, struct usercmd_s *cmd, int active);

private:
	void StrafeVectorial(struct usercmd_s *cmd);
};

extern CStrafer g_Strafer;