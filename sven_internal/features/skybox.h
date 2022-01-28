// Skybox

#pragma once

class CSkybox
{
public:
	void Init();
	void OnConfigLoad();

	void Think();

	void Replace(const char *pszSkyboxName);
	void Reset();

	void SaveOriginalSkybox(const char *pszSkyboxName);

private:
	bool m_bSkyboxReplaced = false;

	char m_szSkyboxName[128] = { 0 };
	char m_szCurrentSkyboxName[128] = { 0 };
	char m_szOriginalSkyboxName[128] = { 0 };
};

extern CSkybox g_Skybox;