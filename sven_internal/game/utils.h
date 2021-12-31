// Game Utils

#include "../sdk.h"
#include "../interfaces.h"

#include "utils/vgui2/ISurface.h"

// Register console stuff
#define REGISTER_COMMAND(command, func) g_pEngineFuncs->pfnAddCommand(command, func)
#define REGISTER_TOGGLE_COMMAND(command, key_down_func, key_up_func) g_pEngineFuncs->pfnAddCommand("+" command, key_down_func); g_pEngineFuncs->pfnAddCommand("-" command, key_up_func)
#define REGISTER_CVAR(cvar, default_value, flags) g_pEngineFuncs->pfnRegisterVariable(cvar, default_value, flags)

// Command arguments when use a console command
#define CMD_ARGC() g_pEngineFuncs->Cmd_Argc()
#define CMD_ARGV(arg) g_pEngineFuncs->Cmd_Argv(arg)

// Screen size
#define SCREEN_STRUCT int __width, __height; g_pSurface->GetScreenSize(__width, __height)
#define SCREEN_WIDTH __width
#define SCREEN_HEIGHT __height

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Console print macro & wrappers
#define M_Msg(msg, ...) g_pEngineFuncs->Con_Printf(msg, __VA_ARGS__)

void Msg(const char *pszMsg, ...);

template <typename... Args>
void T_Msg(const char *pszMsg, Args... args);

//-----------------------------------------------------------------------------
// Exports
//-----------------------------------------------------------------------------

extern double *dbRealtime;
extern vgui::ISurface *g_pSurface;

bool WorldToScreen(float *pflOrigin, float *pflVecScreen);
void ScreenToWorld(float *pflNDC, float *pflWorldOrigin);
void SetAnglesSilent(float *angles, struct usercmd_s *cmd);
void SetGameSpeed(double dbSpeed);
void SendPacket(bool bSend);

// some trash code
void EnableTertiaryAttackGlitch();
void DisableTertiaryAttackGlitch();
bool IsTertiaryAttackGlitchPatched();
bool IsTertiaryAttackGlitchInit();

void EnableTertiaryAttackGlitch_Server();
void DisableTertiaryAttackGlitch_Server();
bool IsTertiaryAttackGlitchPatched_Server();
bool IsTertiaryAttackGlitchInit_Server();

void InitUtils();

inline void ThrowError(const char *msg)
{
	MessageBoxA(NULL, msg, "FATAL ERROR", MB_OK | MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), 0);
}

const Vector VEC_HULL_MIN(-16.f, -16.f, -36.f);
const Vector VEC_HULL_MAX(16.f, 16.f, 36.f);
const Vector VEC_HUMAN_HULL_MIN(-16.f, -16.f, 0.f);
const Vector VEC_HUMAN_HULL_MAX(16.f, 16.f, 72.f);
const Vector VEC_HUMAN_HULL_DUCK(16.f, 16.f, 36.f);

const Vector VEC_VIEW(0.f, 0.f, 28.f);

const Vector VEC_DUCK_HULL_MIN(-16.f, -16.f, -18.f);
const Vector VEC_DUCK_HULL_MAX(16.f, 16.f, 18.f);
const Vector VEC_DUCK_VIEW(0.f, 0.f, 12.f);

const Vector VEC_DEAD_VIEW(0.f, 0.f, -8.f);