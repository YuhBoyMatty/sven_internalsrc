// OpenGL Module

#include "opengl.h"

#include "../sdk.h"

#include "../game/utils.h"

#include "../patterns.h"
#include "../utils/signature_scanner.h"
#include "../utils/trampoline_hook.h"

#include "../config.h"

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef void (APIENTRY *glBeginFn)(GLenum);
typedef void (APIENTRY *glColor4fFn)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (*V_RenderViewFn)(void);
typedef void (*R_SetupFrameFn)(void);

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;
extern ref_params_s * g_pRefParams;

extern int g_iChamsType;
extern bool g_bOverrideColor;

extern float g_flOverrideColor_R;
extern float g_flOverrideColor_G;
extern float g_flOverrideColor_B;

//-----------------------------------------------------------------------------
// Declare Hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(glBegin_Hook);
TRAMPOLINE_HOOK(glColor4f_Hook);
TRAMPOLINE_HOOK(V_RenderView_Hook);
TRAMPOLINE_HOOK(R_SetupFrame_Hook);

//-----------------------------------------------------------------------------
// Original Functions
//-----------------------------------------------------------------------------

glBeginFn glBegin_Original = NULL;
glColor4fFn glColor4f_Original = NULL;
V_RenderViewFn V_RenderView_Original = NULL;
R_SetupFrameFn R_SetupFrame_Original = NULL;

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

void APIENTRY glBegin_Hooked(GLenum mode) // wh
{
	if (g_Config.cvars.wallhack)
	{
		if (mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN) // humans and some objects
			glDepthRange(0, 0.25);
		else
			glDepthRange(0.5, 1);
	}

	if (g_Config.cvars.wallhack_white_walls)
	{
		if (mode == GL_POLYGON)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		}
	}

	if (g_Config.cvars.wallhack_wireframe)
	{
		if (mode == GL_POLYGON)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(g_Config.cvars.wh_wireframe_width);
			glColor3f(g_Config.cvars.wh_wireframe_color[0], g_Config.cvars.wh_wireframe_color[1], g_Config.cvars.wh_wireframe_color[2]);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	if (g_Config.cvars.wallhack_wireframe_models)
	{
		if (mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(1.0);
		}
		else if (!g_Config.cvars.wallhack_wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	glBegin_Original(mode);
}

void APIENTRY glColor4f_Hooked(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	if (g_bOverrideColor)
	{
		if (g_iChamsType == 2)
		{
			red *= g_flOverrideColor_R;
			green *= g_flOverrideColor_G;
			blue *= g_flOverrideColor_B;
		}
		else
		{
			red = g_flOverrideColor_R;
			green = g_flOverrideColor_G;
			blue = g_flOverrideColor_B;
		}
	}

	glColor4f_Original(red, green, blue, alpha);
}

//-----------------------------------------------------------------------------
// Game's render functions. Leave them here for a while
//-----------------------------------------------------------------------------

void R_SetupFrame_Hooked()
{
	R_SetupFrame_Original();

	if (g_pRefParams && g_pRefParams->waterlevel == 3)
	{
		float rgColor[3] = { 0.f, 0.f, 0.f };

		if (g_Config.cvars.remove_water_fog)
		{
			glDisable(GL_FOG);

			if (g_Config.cvars.fog)
				g_pEngineFuncs->pTriAPI->Fog(rgColor, g_Config.cvars.fog_start, g_Config.cvars.fog_end, 0);
		}
		else if (g_Config.cvars.fog)
		{
			g_pEngineFuncs->pTriAPI->Fog(rgColor, g_Config.cvars.fog_start, g_Config.cvars.fog_end, 0);
		}
	}
}

void V_RenderView_Hooked()
{
	GLfloat glColor[] =
	{
		g_Config.cvars.fog_color[0] * 255.0f,
		g_Config.cvars.fog_color[1] * 255.0f,
		g_Config.cvars.fog_color[2] * 255.0f,
		//g_Config.cvars.fog_color[3] * 255.0f
	};

	if (g_Config.cvars.fog)
		g_pEngineFuncs->pTriAPI->FogParams(g_Config.cvars.fog_density / 200.f, int(g_Config.cvars.fog_skybox));

	g_pEngineFuncs->pTriAPI->Fog(glColor, g_Config.cvars.fog_start, g_Config.cvars.fog_end, int(g_Config.cvars.fog));

	V_RenderView_Original();

	//if (g_Config.cvars.fog)
	//{
	//	GLfloat glColor[] =
	//	{
	//		g_Config.cvars.fog_color[0],
	//		g_Config.cvars.fog_color[1],
	//		g_Config.cvars.fog_color[2],
	//		g_Config.cvars.fog_color[3]
	//	};

	//	glEnable(GL_FOG);
	//	glFogi(GL_FOG_MODE, GL_EXP);
	//	glFogf(GL_FOG_DENSITY, g_Config.cvars.fog_density / 200.f);
	//	glFogfv(GL_FOG_COLOR, glColor);
	//}
}

//-----------------------------------------------------------------------------
// Init/release OpenGL module
//-----------------------------------------------------------------------------

void InitOpenGLModule()
{
	HMODULE hHardwareDLL = GetModuleHandle(L"hw.dll");

	void *pV_SetupFrame = FindPattern(hHardwareDLL, Patterns::Hardware::V_SetupFrame);

	if (!pV_SetupFrame)
	{
		Sys_Error("'V_SetupFrame' failed initialization\n");
		return;
	}

	void *pV_RenderView = FindPattern(hHardwareDLL, Patterns::Hardware::V_RenderView);

	if (!pV_RenderView)
	{
		Sys_Error("'V_RenderView' failed initialization\n");
		return;
	}

#pragma warning(push)
#pragma warning(disable: 6387)

	HOOK_FUNCTION(glBegin_Hook, GetProcAddress(GetModuleHandle(L"opengl32.dll"), "glBegin"), glBegin_Hooked, glBegin_Original, glBeginFn);
	HOOK_FUNCTION(glColor4f_Hook, GetProcAddress(GetModuleHandle(L"opengl32.dll"), "glColor4f"), glColor4f_Hooked, glColor4f_Original, glColor4fFn);

#pragma warning(pop)

	HOOK_FUNCTION(R_SetupFrame_Hook, pV_SetupFrame, R_SetupFrame_Hooked, R_SetupFrame_Original, R_SetupFrameFn);
	HOOK_FUNCTION(V_RenderView_Hook, pV_RenderView, V_RenderView_Hooked, V_RenderView_Original, V_RenderViewFn);
}

void ReleaseOpenGLModule()
{
}