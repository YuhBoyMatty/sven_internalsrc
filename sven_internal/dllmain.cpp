// Sven Internal

#include <Windows.h>
#include <direct.h>
#include <stdio.h>

#include "sdk.h"
#include "patterns.h"
#include "config.h"

#include "modules/engine.h"
#include "modules/client.h"
#include "modules/vgui.h"
#include "modules/opengl.h"
#include "modules/menu.h"
#include "modules/SDL2.h"

#include "game/utils.h"
#include "game/studio.h"
#include "game/usermsg.h"
#include "game/class_table.h"
#include "game/console.h"
#include "game/ammo.h"

#include "utils/signature_scanner.h"
#include "libdasm/libdasm.h"

// Show console for debugging or something
#define SHOW_CONSOLE 0

// Interfaces
cl_enginefunc_t *g_pEngineFuncs = NULL;
cl_clientfunc_t *g_pClientFuncs = NULL;
engine_studio_api_t *g_pEngineStudio = NULL;
r_studio_interface_t *g_pStudioAPI = NULL;
CStudioModelRenderer *g_pStudioRenderer = NULL;
IEngineClient *g_pEngineClient = NULL;

static char szSoundcacheDirectory[MAX_PATH] = { 0 };
static void MainThreadRoutine();

static bool InitInterfaces()
{
	INSTRUCTION instruction;

	void *pEngineFuncs = FIND_PATTERN(L"hw.dll", Patterns::Interfaces::EngineFuncs);

	if (!pEngineFuncs)
	{
		Sys_Error("'cl_enginefunc_t' failed initialization\n");
		return false;
	}

	void *pClientFuncs = FIND_PATTERN(L"hw.dll", Patterns::Interfaces::ClientFuncs);

	if (!pClientFuncs)
	{
		Sys_Error("'cl_clientfunc_t' failed initialization\n");
		return false;
	}

	void *pEngineStudio = FIND_PATTERN(L"hw.dll", Patterns::Interfaces::EngineStudio);

	if (!pEngineStudio)
	{
		Sys_Error("'engine_studio_api_t' failed initialization\n");
		return false;
	}

	void *pStudioAPI = pEngineStudio;

	// g_pEngineFuncs
	get_instruction(&instruction, (BYTE *)pEngineFuncs, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_PUSH && instruction.op1.type == OPERAND_TYPE_IMMEDIATE)
	{
		g_pEngineFuncs = reinterpret_cast<cl_enginefunc_t *>(instruction.op1.immediate);
	}
	else
	{
		Sys_Error("'cl_enginefunc_t' failed initialization #2\n");
		return false;
	}

	// g_pClientFuncs
	get_instruction(&instruction, (BYTE *)pClientFuncs + 0x15, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_REGISTER)
	{
		g_pClientFuncs = reinterpret_cast<cl_clientfunc_t *>(instruction.op1.displacement);
	}
	else
	{
		Sys_Error("'cl_clientfunc_t' failed initialization #2\n");
		return false;
	}

	// g_pEngineStudio
	get_instruction(&instruction, (BYTE *)pEngineStudio, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_PUSH && instruction.op1.type == OPERAND_TYPE_IMMEDIATE)
	{
		g_pEngineStudio = reinterpret_cast<engine_studio_api_t *>(instruction.op1.immediate);
	}
	else
	{
		Sys_Error("'engine_studio_api_t' failed initialization #2\n");
		return false;
	}

	// g_pStudioAPI
	get_instruction(&instruction, (BYTE *)pStudioAPI + 0x5, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_PUSH && instruction.op1.type == OPERAND_TYPE_IMMEDIATE)
	{
		g_pStudioAPI = reinterpret_cast<r_studio_interface_t *>(instruction.op1.immediate);
	}
	else
	{
		Sys_Error("'r_studio_interface_t' failed initialization\n");
		return false;
	}
	
	// g_pStudioRenderer
	get_instruction(&instruction, (BYTE *)g_pClientFuncs->HUD_GetStudioModelInterface + 0x26, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
	{
		g_pStudioRenderer = reinterpret_cast<CStudioModelRenderer *>(instruction.op2.immediate);
	}
	else
	{
		Sys_Error("'CStudioModelRenderer' failed initialization\n");
		return false;
	}

	CreateInterfaceFn hardwareFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandle(L"hw.dll"), "CreateInterface");

	if (!hardwareFactory)
	{
		Sys_Error("'IEngineClient' failed initialization\n");
		return false;
	}

	g_pEngineClient = reinterpret_cast<IEngineClient *>(hardwareFactory(SC_ENGINE_CLIENT_INTERFACE_VERSION, NULL));

	if (!g_pEngineClient)
	{
		Sys_Error("'IEngineClient' failed initialization #2\n");
		return false;
	}

	return true;
}

DWORD WINAPI MainThread(HMODULE hModule)
{
#if SHOW_CONSOLE
	FILE *file;
	AllocConsole();
	freopen_s(&file, "CONOUT$", "w", stdout);
#endif

	DWORD dwHWDLL = (DWORD)GetModuleHandle(L"hw.dll");

	if (dwHWDLL)
	{
		if (!InitInterfaces())
			goto FAILURE_EXIT;

		ConVar_Register();

		InitUtils();
		InitUserMsg();
		InitStudioDetours();
		InitWeaponsResource();

		InitSDL2Module();
		InitOpenGLModule();
		InitVGUIModule();
		InitEngineModule();
		InitClientModule();
		InitMenuModule();

		printf("Successfully loaded\n");

		Msg("Sven Internal Loaded\n");

		//g_pEngineFuncs->pfnClientCmd("cl_timeout 999999;rate 999999;cl_updaterate 1000;cl_cmdrate 1000;ex_interp 0.1"); // rate 50000; cl_updaterate 80; cl_cmdrate 205
		g_pEngineFuncs->pfnClientCmd("exec sven_internal.cfg");

		while (true)
		{
			MainThreadRoutine();
		}
	}
	else
	{
		printf("It is not a Half-Life game..\n");

	FAILURE_EXIT:
		printf("Exiting...\n");
		Sleep(3500);
	}

#if SHOW_CONSOLE
	if (file) fclose(file);
	FreeConsole();
#endif

	FreeLibraryAndExitThread(hModule, 0);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
		if (hThread) CloseHandle(hThread);
		break;
	}

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

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

static void MainThreadRoutine()
{
	Sleep(500);

	// Save soundcache to load faster
	static const char *szSoundcache = "\\svencoop_downloads\\maps\\soundcache\\";

	if (!*szSoundcacheDirectory && _getcwd(szSoundcacheDirectory, sizeof(szSoundcacheDirectory)))
	{
		memcpy(szSoundcacheDirectory + strlen(szSoundcacheDirectory), szSoundcache, strlen(szSoundcache));
		//g_pEngineFuncs->Con_Printf("%s\n", szSoundcacheDirectory);
	}

	if (g_Config.cvars.save_soundcache)
	{
		SetFilesAttributes(szSoundcacheDirectory, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY);
	}
	else
	{
		//SetFilesAttributes(szSoundcacheDirectory, FILE_ATTRIBUTE_NORMAL);
	}

	// server-side tertiary attack glitch
	if (!IsTertiaryAttackGlitchInit_Server())
	{
		HMODULE serverDLL = GetModuleHandle(L"server.dll");

		if (serverDLL)
		{
			extern void InitTertiaryAttackGlitch_Server();
			InitTertiaryAttackGlitch_Server();
		}
	}
}