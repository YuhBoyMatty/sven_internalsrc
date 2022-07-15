#include <string>

#include <ISvenModAPI.h>
#include <IClientPlugin.h>
#include <IVideoMode.h>

#include <convar.h>
#include <dbg.h>

#include <base_feature.h>

#include "config.h"
#include "game/utils.h"
#include "game/drawing.h"
#include "modules/patches.h"

// Features
#include "features/custom_vote_popup.h"
#include "features/skybox.h"
#include "features/visual.h"
#include "features/chat_colors.h"
#include "features/camhack.h"

static void SaveSoundcache();

extern bool g_bForceFreeze2;

//-----------------------------------------------------------------------------
// SvenMod's plugin
//-----------------------------------------------------------------------------

class CSvenInternal : public IClientPlugin
{
public:
	virtual api_version_s GetAPIVersion();

	virtual bool Load(CreateInterfaceFn pfnSvenModFactory, ISvenModAPI *pSvenModAPI, IPluginHelpers *pPluginHelpers);

	virtual void PostLoad(bool bGlobalLoad);

	virtual void Unload(void);

	virtual bool Pause(void);

	virtual void Unpause(void);

	virtual void GameFrame(client_state_t state, double frametime, bool bPostRunCmd);

	virtual PLUGIN_RESULT Draw(void);

	virtual PLUGIN_RESULT DrawHUD(float time, int intermission);

	virtual const char *GetName(void);

	virtual const char *GetAuthor(void);

	virtual const char *GetVersion(void);

	virtual const char *GetDescription(void);

	virtual const char *GetURL(void);

	virtual const char *GetDate(void);

	virtual const char *GetLogTag(void);

private:
	void InitFolders(ISvenModAPI *pSvenModAPI);

private:
	float m_flPlatTime;
};

//-----------------------------------------------------------------------------
// Implement plugin methods
//-----------------------------------------------------------------------------

api_version_s CSvenInternal::GetAPIVersion()
{
	return SVENMOD_API_VER;
}

bool CSvenInternal::Load(CreateInterfaceFn pfnSvenModFactory, ISvenModAPI *pSvenModAPI, IPluginHelpers *pPluginHelpers)
{
	BindApiToGlobals(pSvenModAPI);
	InitFolders(pSvenModAPI);

	if ( !LoadFeatures() )
	{
		Warning("[Sven Internal] Failed to initialize one of features\n");
		return false;
	}

	ConVar_Register();

	vmode_t *mode = g_pVideoMode->GetCurrentMode();

	g_ScreenInfo.width = mode->width;
	g_ScreenInfo.height = mode->height;

	g_Config.Init();
	g_Config.Load();

	g_Drawing.Init();
	g_Visual.ResetJumpSpeed();

	g_pEngineFuncs->ClientCmd("cl_timeout 999999;unbind F1;unbind F2;exec sven_internal.cfg");
	m_flPlatTime = g_pEngineFuncs->Sys_FloatTime();

	ConColorMsg({ 40, 255, 40, 255 }, "[Sven Internal] Successfully loaded\n");
	return true;
}

void CSvenInternal::PostLoad(bool bGlobalLoad)
{
	if (bGlobalLoad)
	{
		
	}
	else
	{
		
	}

	PostLoadFeatures();

	g_CamHack.Init();
}

void CSvenInternal::Unload(void)
{
	UnloadFeatures();

	ConVar_Unregister();
}

bool CSvenInternal::Pause(void)
{
	Warning("[Sven Internal] It is not allowed to pause the plugin\n");
	return false;
}

void CSvenInternal::Unpause(void)
{
}

void CSvenInternal::GameFrame(client_state_t state, double frametime, bool bPostRunCmd)
{
	if (bPostRunCmd)
	{
		if (g_pEngineFuncs->Sys_FloatTime() - m_flPlatTime >= 0.5f)
		{
			g_Config.UpdateConfigs();

			m_flPlatTime = g_pEngineFuncs->Sys_FloatTime();
		}
	}
	else
	{
		if (state >= CLS_CONNECTED)
		{
			SaveSoundcache();

			if (state == CLS_ACTIVE)
			{
				g_Skybox.Think();
				g_ChatColors.Think();

				static int wait_frames_TAGS = 0;

				wait_frames_TAGS++;

				if (wait_frames_TAGS >= 100)
				{
					if ( !IsTertiaryAttackGlitchInit_Server() )
					{
					#ifdef PLATFORM_WINDOWS
						HMODULE hServerDLL = Sys_GetModuleHandle("server.dll");
					#else
						HMODULE hServerDLL = Sys_GetModuleHandle("server.so");
					#endif

						if (hServerDLL)
						{
							extern void InitTertiaryAttackGlitch_Server(HMODULE hServerDLL);
							InitTertiaryAttackGlitch_Server(hServerDLL);
						}
					}

					wait_frames_TAGS = 0;
				}
			}
			else
			{
				g_bForceFreeze2 = false;
			}
		}
		else
		{
			g_bForceFreeze2 = false;
		}
	}
}

PLUGIN_RESULT CSvenInternal::Draw(void)
{
	g_Visual.Process();
	g_VotePopup.Draw();

	return PLUGIN_CONTINUE;
}

PLUGIN_RESULT CSvenInternal::DrawHUD(float time, int intermission)
{
	g_Visual.OnHUDRedraw(time);

	return PLUGIN_CONTINUE;
}

const char *CSvenInternal::GetName(void)
{
	return "Sven Internal";
}

const char *CSvenInternal::GetAuthor(void)
{
	return "Sw1ft / void";
}

const char *CSvenInternal::GetVersion(void)
{
	return "2.0.10";
}

const char *CSvenInternal::GetDescription(void)
{
	return "Provides various cheats and gameplay enhances";
}

const char *CSvenInternal::GetURL(void)
{
	return "https://github.com/sw1ft747/sven_internal";
}

const char *CSvenInternal::GetDate(void)
{
	return SVENMOD_BUILD_TIMESTAMP;
}

const char *CSvenInternal::GetLogTag(void)
{
	return "SVENINT";
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void CSvenInternal::InitFolders(ISvenModAPI *pSvenModAPI)
{
	std::string sDir = pSvenModAPI->GetBaseDirectory();

#ifdef PLATFORM_WINDOWS
	if ( !CreateDirectory((sDir + "\\sven_internal\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS )
	{
		Warning("[Sven Internal] Failed to create \"../sven_internal/\" directory\n");
	}

	if ( !CreateDirectory((sDir + "\\sven_internal\\config\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS )
	{
		Warning("[Sven Internal] Failed to create \"../sven_internal/config/\" directory\n");
	}

	if ( !CreateDirectory((sDir + "\\sven_internal\\message_spammer\\").c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS )
	{
		Warning("[Sven Internal] Failed to create \"../sven_internal/message_spammer/\" directory\n");
	}
#else
#error Implement Linux equivalent
#endif

	sDir.clear();
}

//-----------------------------------------------------------------------------
// Export the plugin's interface
//-----------------------------------------------------------------------------

EXPOSE_SINGLE_INTERFACE(CSvenInternal, IClientPlugin, CLIENT_PLUGIN_INTERFACE_VERSION);

//-----------------------------------------------------------------------------
// Weirdo function
//-----------------------------------------------------------------------------

#ifdef PLATFORM_WINDOWS
static void SetFilesAttributes(const char *m_szFdPath, DWORD dwAttribute)
{
	HANDLE hFile;
	WIN32_FIND_DATAA FileInformation;

	char m_szPath[MAX_PATH];
	char m_szFolderInitialPath[MAX_PATH];
	char wildCard[MAX_PATH] = "\\*.*";

	strcpy(m_szPath, m_szFdPath);
	strcpy(m_szFolderInitialPath, m_szFdPath);
	strcat(m_szFolderInitialPath, wildCard);

	hFile = ::FindFirstFileA(m_szFolderInitialPath, &FileInformation);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strcpy(m_szPath, m_szFdPath);
				strcat(m_szPath, "\\");
				strcat(m_szPath, FileInformation.cFileName);

			#pragma warning(push)
			#pragma warning(push)
			#pragma warning(disable: 26450)
			#pragma warning(disable: 4307)

				if (!(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (FileInformation.nFileSizeHigh * (MAXDWORD + 1)) + FileInformation.nFileSizeLow > 0)
				{
					//it is a file
					::SetFileAttributesA(m_szPath, dwAttribute);
				}

			#pragma warning(pop)
			#pragma warning(pop)
			}
		} while (::FindNextFileA(hFile, &FileInformation) == TRUE);

		::FindClose(hFile);

		//DWORD dwError = ::GetLastError();
		//if (dwError == ERROR_NO_MORE_FILES)
		//{
		//	//Attributes successfully changed
		//}
	}
}

static int sleep_frames = 0;
static char szSoundcacheDirectory[MAX_PATH] = { 0 };

static void SaveSoundcache()
{
	if ( !*szSoundcacheDirectory )
	{
		_snprintf(szSoundcacheDirectory, MAX_PATH, "%s\\svencoop_downloads\\maps\\soundcache\\", SvenModAPI()->GetBaseDirectory());
	}

	if (g_Config.cvars.save_soundcache)
	{
		sleep_frames++;

		if (sleep_frames >= 75)
		{
			SetFilesAttributes(szSoundcacheDirectory, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY);
			sleep_frames = 0;
		}
	}
}
#else
#error Implement Linux equivalent
#endif