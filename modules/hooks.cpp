#include <hl_sdk/engine/APIProxy.h>
#include <netchan.h>

#include <dbg.h>
#include <base_feature.h>

#include <IHooks.h>
#include <ISvenModAPI.h>
#include <IMemoryUtils.h>

#include <gl/GL.h>

#include "hooks.h"

#include "../patterns.h"
#include "../config.h"

#include "../game/drawing.h"

// Features
#include "../features/misc.h"
#include "../features/visual.h"
#include "../features/antiafk.h"
#include "../features/camhack.h"
#include "../features/chams.h"
#include "../features/strafer.h"
#include "../features/custom_vote_popup.h"
#include "../features/firstperson_roaming.h"
#include "../features/message_spammer.h"
#include "../features/keyspam.h"
#include "../features/skybox.h"
#include "../features/dynamic_glow.h"
#include "../features/chat_colors.h"

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK(void, __cdecl, IN_Move, float, usercmd_t *);

DECLARE_HOOK(qboolean, __cdecl, Netchan_CanPacket, netchan_t *);
DECLARE_HOOK(void, __cdecl, SPR_Set, VHSPRITE hPic, int r, int g, int b);

DECLARE_HOOK(void, APIENTRY, glBegin, GLenum);
DECLARE_HOOK(void, APIENTRY, glColor4f, GLfloat, GLfloat, GLfloat, GLfloat);

DECLARE_HOOK(void, __cdecl, V_RenderView);
DECLARE_HOOK(void, __cdecl, R_SetupFrame);

DECLARE_CLASS_HOOK(void, StudioRenderModel, CStudioModelRenderer *);

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern bool g_bMenuEnabled;
extern bool g_bMenuClosed;

extern int g_iChamsType;
extern bool g_bOverrideColor;

extern float g_flOverrideColor_R;
extern float g_flOverrideColor_G;
extern float g_flOverrideColor_B;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

bool bSendPacket = true;
float g_flClientDataLastUpdate = -1.f;

Vector g_oldviewangles(0.f, 0.f, 0.f);
Vector g_newviewangles(0.f, 0.f, 0.f);

static int s_iWaterLevel = 0;

//-----------------------------------------------------------------------------
// Hooks module feature
//-----------------------------------------------------------------------------

class CHooksModule : public CBaseFeature
{
public:
	CHooksModule();

	virtual bool Load();
	virtual void PostLoad();

	virtual void Unload();

private:
	void *m_pfnIN_Move;
	void *m_pfnNetchan_CanPacket;
	void *m_pfnSPR_Set;
	void *m_pfnglBegin;
	void *m_pfnglColor4f;
	void *m_pfnV_RenderView;
	void *m_pfnR_SetupFrame;

	DetourHandle_t m_hIN_Move;
	DetourHandle_t m_hNetchan_CanPacket;
	DetourHandle_t m_hSPR_Set;
	DetourHandle_t m_hglBegin;
	DetourHandle_t m_hglColor4f;
	DetourHandle_t m_hV_RenderView;
	DetourHandle_t m_hR_SetupFrame;

	DetourHandle_t m_hStudioRenderModel;
};

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

FORCEINLINE void RunClientMoveHooks(float frametime, usercmd_t *cmd, int active)
{
	g_Strafer.CreateMove(frametime, cmd, active);
	g_KeySpam.CreateMove(frametime, cmd, active);
	g_Misc.CreateMove(frametime, cmd, active);
	g_AntiAFK.CreateMove(frametime, cmd, active);
	g_CamHack.CreateMove(frametime, cmd, active);
	g_MessageSpammer.CreateMove(frametime, cmd, active);
}

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

static Vector s_lastViewAngles = { 0.0f, 0.0f, 0.0f };

void OnMenuOpen()
{
	g_pEngineFuncs->GetViewAngles(s_lastViewAngles);
}

void OnMenuClose()
{
}

//-----------------------------------------------------------------------------
// OpenGL hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC(void, APIENTRY, HOOKED_glBegin, GLenum mode) // wh
{
	if (g_Config.cvars.wallhack)
	{
		if (mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN) // humans and some objects
			glDepthRange(0, 0.25);
		else
			glDepthRange(0.5, 1);
	}

	if (g_Config.cvars.wallhack_negative)
	{
		if (mode == GL_POLYGON)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		}
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

	ORIG_glBegin(mode);
}

DECLARE_FUNC(void, APIENTRY, HOOKED_glColor4f, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
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

	ORIG_glColor4f(red, green, blue, alpha);
}

//-----------------------------------------------------------------------------
// Client hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC(void, __cdecl, HOOKED_IN_Move, float frametime, usercmd_t *cmd)
{
	if ( g_bMenuEnabled || g_bMenuClosed )
		return;

	g_pEngineFuncs->GetViewAngles( g_oldviewangles );

	ORIG_IN_Move(frametime, cmd);

	g_pEngineFuncs->GetViewAngles( g_newviewangles );
}

//-----------------------------------------------------------------------------
// Engine hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC(qboolean, __cdecl, HOOKED_Netchan_CanPacket, netchan_t *netchan)
{
	if ( !bSendPacket )
		return 0;

	return ORIG_Netchan_CanPacket(netchan);
}

int SpriteArray[] =
{
	0, 2, 1, 0, 0, 1, 1, 0, 1, 2,
	2, 0, 0, 0, 2, 1, 0, 0, 0, 0,
	0, 0, 2, 1, 0, 0, 0, 2, 0, 1,
	0, 0, 0, 2, 0, 2, 1, 0, 0, 0,
};

DECLARE_FUNC(void, __cdecl, HOOKED_SPR_Set, VHSPRITE hPic, int r, int g, int b)
{
	if ( g_Config.cvars.remap_hud_color )
	{
		r = int(g_Config.cvars.hud_color[0] * 255.f);
		g = int(g_Config.cvars.hud_color[1] * 255.f);
		b = int(g_Config.cvars.hud_color[2] * 255.f);
	}

	ORIG_SPR_Set(hPic, r, g, b);
}

DECLARE_FUNC(void, __cdecl, HOOKED_R_SetupFrame)
{
	ORIG_R_SetupFrame();

	if (s_iWaterLevel == WL_EYES)
	{
		float rgColor[3] = { 0.f, 0.f, 0.f };

		if (g_Config.cvars.remove_water_fog)
		{
			glDisable(GL_FOG);

			if (g_Config.cvars.fog)
				g_pTriangleAPI->Fog(rgColor, g_Config.cvars.fog_start, g_Config.cvars.fog_end, 0);
		}
		else if (g_Config.cvars.fog)
		{
			g_pTriangleAPI->Fog(rgColor, g_Config.cvars.fog_start, g_Config.cvars.fog_end, 0);
		}
	}
}

DECLARE_FUNC(void, __cdecl, HOOKED_V_RenderView)
{
	GLfloat glColor[] =
	{
		g_Config.cvars.fog_color[0] * 255.0f,
		g_Config.cvars.fog_color[1] * 255.0f,
		g_Config.cvars.fog_color[2] * 255.0f,
		//g_Config.cvars.fog_color[3] * 255.0f
	};

	if (g_Config.cvars.fog)
		g_pTriangleAPI->FogParams(g_Config.cvars.fog_density / 200.f, int(g_Config.cvars.fog_skybox));

	g_pTriangleAPI->Fog(glColor, g_Config.cvars.fog_start, g_Config.cvars.fog_end, int(g_Config.cvars.fog));

	ORIG_V_RenderView();

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

DECLARE_CLASS_FUNC(void, HOOKED_StudioRenderModel, CStudioModelRenderer *thisptr)
{
	bool bRenderHandled = false;

	// Calling many functions will take down our performance
	bRenderHandled = g_FirstPersonRoaming.StudioRenderModel();
	bRenderHandled = g_Chams.StudioRenderModel() || bRenderHandled;
	bRenderHandled = g_Visual.StudioRenderModel() || bRenderHandled;
	bRenderHandled = g_CamHack.StudioRenderModel() || bRenderHandled;

	if ( !bRenderHandled )
	{
		ORIG_StudioRenderModel(thisptr);
	}
}

//-----------------------------------------------------------------------------
// Client DLL hooks
//-----------------------------------------------------------------------------

HOOK_RESULT CClientHooks::HUD_VidInit(void)
{
	g_Visual.OnVideoInit();
	g_Drawing.OnVideoInit();
	g_Skybox.OnVideoInit();
	g_VotePopup.OnVideoInit();
	g_ChatColors.OnVideoInit();
	g_CamHack.OnVideoInit();
	g_AntiAFK.OnVideoInit();
	g_Misc.OnVideoInit();

	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_Redraw(float time, int intermission)
{
	g_DynamicGlow.OnHUDRedraw();

	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientHooks::HUD_UpdateClientData(int *changed, client_data_t *pcldata, float flTime)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_PlayerMove(playermove_t *ppmove, int server)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::IN_ActivateMouse(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::IN_DeactivateMouse(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::IN_MouseEvent(int mstate)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::IN_ClearStates(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::IN_Accumulate(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::CL_CreateMove(float frametime, usercmd_t *cmd, int active)
{
	static int s_nWaitFrames = 0;

	bSendPacket = true;

	if ( g_bMenuEnabled )
	{
		cmd->viewangles = s_lastViewAngles;
	}

	if ( g_bMenuClosed )
	{
		g_bMenuClosed = false;

		cmd->viewangles = s_lastViewAngles;
		g_pClientFuncs->IN_ClearStates();

		SetCursorPos(g_pUtils->GetScreenWidth() / 2, g_pUtils->GetScreenHeight() / 2);
	}

	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientHooks::CL_IsThirdPerson(int *thirdperson)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientHooks::KB_Find(kbutton_t **button, const char *name)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::CAM_Think(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::V_CalcRefdef(ref_params_t *pparams)
{
	s_iWaterLevel = pparams->waterlevel;

	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientHooks::HUD_AddEntity(int *visible, int type, cl_entity_t *ent, const char *modelname)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_CreateEntities(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_DrawNormalTriangles(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_DrawTransparentTriangles(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_StudioEvent(const mstudioevent_t *studio_event, const cl_entity_t *entity)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_PostRunCmd(local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_TxferLocalOverrides(entity_state_t *state, const clientdata_t *client)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_ProcessPlayerState(entity_state_t *dst, const entity_state_t *src)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_TxferPredictionData(entity_state_t *ps, const entity_state_t *pps, clientdata_t *pcd, const clientdata_t *ppcd, weapon_data_t *wd, const weapon_data_t *pwd)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::Demo_ReadBuffer(int size, unsigned const char *buffer)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientHooks::HUD_ConnectionlessPacket(int *valid_packet, netadr_t *net_from, const char *args, const char *response_buffer, int *response_buffer_size)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientHooks::HUD_GetHullBounds(int *hullnumber_exist, int hullnumber, float *mins, float *maxs)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_Frame(double time)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientHooks::HUD_Key_Event(int *process_key, int down, int keynum, const char *pszCurrentBinding)
{
	if ( g_bMenuEnabled && keynum != '`' ) // tilde
	{
		*process_key = 0;
		return HOOK_STOP;
	}

	g_VotePopup.OnKeyPress(down, keynum);

	if ( g_CamHack.IsEnabled() && g_CamHack.OnKeyPress(down, keynum) )
	{
		*process_key = 0;
		return HOOK_STOP;
	}

	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, TEMPENTITY **ppTempEntFree, TEMPENTITY **ppTempEntActive, int (*Callback_AddVisibleEntity)(cl_entity_t *pEntity), void (*Callback_TempEntPlaySound)(TEMPENTITY *pTemp, float damp))
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientHooks::HUD_GetUserEntity(cl_entity_t **ent, int index)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_DirectorMessage(unsigned char command, unsigned int firstObject, unsigned int secondObject, unsigned int flags)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientHooks::HUD_ChatInputPosition(int *x, int *y)
{
	return HOOK_CONTINUE;
}

//-----------------------------------------------------------------------------
// Client DLL post hooks
//-----------------------------------------------------------------------------

HOOK_RESULT CClientPostHooks::HUD_VidInit(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_Redraw(float time, int intermission)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientPostHooks::HUD_UpdateClientData(int *changed, client_data_t *pcldata, float flTime)
{
	if (*changed)
		g_flClientDataLastUpdate = flTime;

	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_PlayerMove(playermove_t *ppmove, int server)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::IN_ActivateMouse(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::IN_DeactivateMouse(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::IN_MouseEvent(int mstate)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::IN_ClearStates(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::IN_Accumulate(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::CL_CreateMove(float frametime, usercmd_t *cmd, int active)
{
	RunClientMoveHooks(frametime, cmd, active);

	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientPostHooks::CL_IsThirdPerson(int *thirdperson)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientPostHooks::KB_Find(kbutton_t **button, const char *name)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::CAM_Think(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::V_CalcRefdef(ref_params_t *pparams)
{
	g_CamHack.V_CalcRefdef(pparams);
	g_Misc.V_CalcRefdef(pparams);
	g_FirstPersonRoaming.V_CalcRefdef(pparams);

	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientPostHooks::HUD_AddEntity(int *visible, int type, cl_entity_t *ent, const char *modelname)
{
	g_DynamicGlow.OnAddEntityPost(*visible, type, ent, modelname);
	g_Misc.OnAddEntityPost(*visible, type, ent, modelname);

	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_CreateEntities(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_DrawNormalTriangles(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_DrawTransparentTriangles(void)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_StudioEvent(const mstudioevent_t *studio_event, const cl_entity_t *entity)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_PostRunCmd(local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_Misc.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);

	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_TxferLocalOverrides(entity_state_t *state, const clientdata_t *client)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_ProcessPlayerState(entity_state_t *dst, const entity_state_t *src)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_TxferPredictionData(entity_state_t *ps, const entity_state_t *pps, clientdata_t *pcd, const clientdata_t *ppcd, weapon_data_t *wd, const weapon_data_t *pwd)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::Demo_ReadBuffer(int size, unsigned const char *buffer)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientPostHooks::HUD_ConnectionlessPacket(int *valid_packet, netadr_t *net_from, const char *args, const char *response_buffer, int *response_buffer_size)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientPostHooks::HUD_GetHullBounds(int *hullnumber_exist, int hullnumber, float *mins, float *maxs)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_Frame(double time)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientPostHooks::HUD_Key_Event(int *process_key, int down, int keynum, const char *pszCurrentBinding)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, TEMPENTITY **ppTempEntFree, TEMPENTITY **ppTempEntActive, int (*Callback_AddVisibleEntity)(cl_entity_t *pEntity), void (*Callback_TempEntPlaySound)(TEMPENTITY *pTemp, float damp))
{
	return HOOK_CONTINUE;
}

HOOK_RESULT HOOK_RETURN_VALUE CClientPostHooks::HUD_GetUserEntity(cl_entity_t **ent, int index)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_DirectorMessage(unsigned char command, unsigned int firstObject, unsigned int secondObject, unsigned int flags)
{
	return HOOK_CONTINUE;
}

HOOK_RESULT CClientPostHooks::HUD_ChatInputPosition(int *x, int *y)
{
	return HOOK_CONTINUE;
}

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

CClientHooks g_ClientHooks;
CClientPostHooks g_ClientPostHooks;

//-----------------------------------------------------------------------------
// Hooks module feature impl
//-----------------------------------------------------------------------------

CHooksModule::CHooksModule()
{
	m_pfnNetchan_CanPacket = NULL;
	m_pfnglBegin = NULL;
	m_pfnglColor4f = NULL;
	m_pfnV_RenderView = NULL;
	m_pfnR_SetupFrame = NULL;

	m_hNetchan_CanPacket = 0;
	m_hglBegin = 0;
	m_hglColor4f = 0;
	m_hV_RenderView = 0;
	m_hR_SetupFrame = 0;
}

bool CHooksModule::Load()
{
	m_pfnglBegin = Sys_GetProcAddress(Sys_GetModuleHandle("opengl32.dll"), "glBegin");
	m_pfnglColor4f = Sys_GetProcAddress(Sys_GetModuleHandle("opengl32.dll"), "glColor4f");

	if ( !m_pfnglBegin )
	{
		Warning("Couldn't find function \"glBegin\"\n");
		return false;
	}

	if ( !m_pfnglColor4f )
	{
		Warning("Couldn't find function \"glColor4f\"\n");
		return false;
	}

	m_pfnIN_Move = MemoryUtils()->FindPattern( g_pModules->Client, Patterns::Client::IN_Move );

	if ( !m_pfnIN_Move )
	{
		Warning("Couldn't find function \"IN_Move\"\n");
		return false;
	}
	
	m_pfnNetchan_CanPacket = MemoryUtils()->FindPattern( g_pModules->Hardware, Patterns::Hardware::Netchan_CanPacket );

	if ( !m_pfnNetchan_CanPacket )
	{
		Warning("Couldn't find function \"Netchan_CanPacket\"\n");
		return false;
	}
	
	m_pfnV_RenderView = MemoryUtils()->FindPattern( g_pModules->Hardware, Patterns::Hardware::V_RenderView );

	if ( !m_pfnV_RenderView )
	{
		Warning("Couldn't find function \"V_RenderView\"\n");
		return false;
	}
	
	m_pfnR_SetupFrame = MemoryUtils()->FindPattern( g_pModules->Hardware, Patterns::Hardware::R_SetupFrame );

	if ( !m_pfnR_SetupFrame )
	{
		Warning("Couldn't find function \"R_SetupFrame\"\n");
		return false;
	}

	m_pfnSPR_Set = g_pEngineFuncs->SPR_Set;

	return true;
}

void CHooksModule::PostLoad()
{
	m_hIN_Move = DetoursAPI()->DetourFunction( m_pfnIN_Move, HOOKED_IN_Move, GET_FUNC_PTR(ORIG_IN_Move) );
	m_hNetchan_CanPacket = DetoursAPI()->DetourFunction( m_pfnNetchan_CanPacket, HOOKED_Netchan_CanPacket, GET_FUNC_PTR(ORIG_Netchan_CanPacket) );
	m_hSPR_Set = DetoursAPI()->DetourFunction( m_pfnSPR_Set, HOOKED_SPR_Set, GET_FUNC_PTR(ORIG_SPR_Set) );
	m_hglBegin = DetoursAPI()->DetourFunction( m_pfnglBegin, HOOKED_glBegin, GET_FUNC_PTR(ORIG_glBegin) );
	m_hglColor4f = DetoursAPI()->DetourFunction( m_pfnglColor4f, HOOKED_glColor4f, GET_FUNC_PTR(ORIG_glColor4f) );
	m_hV_RenderView = DetoursAPI()->DetourFunction( m_pfnV_RenderView, HOOKED_V_RenderView, GET_FUNC_PTR(ORIG_V_RenderView) );
	m_hR_SetupFrame = DetoursAPI()->DetourFunction( m_pfnR_SetupFrame, HOOKED_R_SetupFrame, GET_FUNC_PTR(ORIG_R_SetupFrame) );

	m_hStudioRenderModel = DetoursAPI()->DetourVirtualFunction( g_pStudioRenderer, 20, HOOKED_StudioRenderModel, GET_FUNC_PTR(ORIG_StudioRenderModel) );

	Hooks()->RegisterClientHooks( &g_ClientHooks );
	Hooks()->RegisterClientPostHooks( &g_ClientPostHooks );
}

void CHooksModule::Unload()
{
	DetoursAPI()->RemoveDetour( m_hIN_Move );
	DetoursAPI()->RemoveDetour( m_hNetchan_CanPacket );
	DetoursAPI()->RemoveDetour( m_hSPR_Set );
	DetoursAPI()->RemoveDetour( m_hglBegin );
	DetoursAPI()->RemoveDetour( m_hglColor4f );
	DetoursAPI()->RemoveDetour( m_hV_RenderView );
	DetoursAPI()->RemoveDetour( m_hR_SetupFrame );

	DetoursAPI()->RemoveDetour( m_hStudioRenderModel );

	Hooks()->UnregisterClientPostHooks( &g_ClientPostHooks );
	Hooks()->UnregisterClientHooks( &g_ClientHooks );
}

//-----------------------------------------------------------------------------
// Create singleton
//-----------------------------------------------------------------------------

CHooksModule g_HooksModule;