// Visual

#pragma once

class CVisual
{
public:
	void Init();
	void Process();
	
	bool StudioRenderModel();

private:
	void ESP();
	void DrawCrosshair();
	void ShowSpeed();
	void Lightmap();

	void ProcessBones();

private:
	int m_iScreenWidth = 1920;
	int m_iScreenHeight = 1080;
};

extern CVisual g_Visual;