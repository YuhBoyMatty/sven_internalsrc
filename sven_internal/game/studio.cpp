// Studio Model Renderer

#pragma comment(lib, "OpenGL32.lib")

#include <Windows.h>
#include <gl/GL.h>

#include "studio.h"

#include "../interfaces.h"
#include "../config.h"

#include "../utils/trampoline_hook.h"
#include "../utils/vtable_hook.h"

#include "../features/chams.h"
#include "../features/visual.h"
#include "../features/firstperson_roaming.h"

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef int (__thiscall *StudioDrawMonsterFn)(CStudioModelRenderer *, int, cl_entity_s *);
typedef void (__thiscall *StudioSetUpTransformFn)(CStudioModelRenderer *, int trivial_accept);
typedef void (__thiscall *StudioRenderModelFn)(CStudioModelRenderer *);
typedef void (__thiscall *StudioRenderFinal_HardwareFn)(CStudioModelRenderer *);

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern cvar_s *r_drawentities;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

bool bCallbackFinalRendering = true;

//-----------------------------------------------------------------------------
// Original Functions
//-----------------------------------------------------------------------------

StudioDrawMonsterFn StudioDrawMonster_Original = NULL;
StudioSetUpTransformFn StudioSetUpTransform_Original = NULL;
StudioRenderModelFn StudioRenderModel_Original = NULL;
StudioRenderFinal_HardwareFn StudioRenderFinal_Hardware_Original = NULL;

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

int __fastcall StudioDrawMonster_Hooked(CStudioModelRenderer *thisptr, int edx, int flags, cl_entity_s *pmonster)
{
	return StudioDrawMonster_Original(thisptr, flags, pmonster);
}

void __fastcall StudioSetUpTransform_Hooked(CStudioModelRenderer *thisptr, int edx, int trivial_accept)
{
	StudioSetUpTransform_Original(thisptr, trivial_accept);
}

void __fastcall StudioRenderModel_Hooked(CStudioModelRenderer *thisptr)
{
	if (g_Config.cvars.fp_roaming && thisptr->m_pCurrentEntity == g_FirstPersonRoaming.GetTargetPlayer())
		return;

	bool bRenderHandled = false;

	bCallbackFinalRendering = false;

	// Calling many functions will take down our performance
	bRenderHandled = g_Chams.StudioRenderModel();
	g_Visual.StudioRenderModel();

	bCallbackFinalRendering = true;

	if (!bRenderHandled)
	{
		if (g_Config.cvars.wallhack && r_drawentities->value >= 2.0f && r_drawentities->value <= 5.0f)
		{
			glDisable(GL_DEPTH_TEST);

			StudioRenderModel_Original(thisptr);

			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			StudioRenderModel_Original(thisptr);
		}
	}
}

void __fastcall StudioRenderFinal_Hardware_Hooked(CStudioModelRenderer *thisptr)
{
	StudioRenderFinal_Hardware_Original(thisptr);
}

//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------

void InitStudioDetours()
{
	//StudioDrawMonster_Original = (StudioDrawMonsterFn)CVTableHook::HookFunction(g_pStudioRenderer, StudioDrawMonster_Hooked, 4);
	//StudioSetUpTransform_Original = (StudioSetUpTransformFn)CVTableHook::HookFunction(g_pStudioRenderer, StudioSetUpTransform_Hooked, 6);
	StudioRenderModel_Original = (StudioRenderModelFn)CVTableHook::HookFunction(g_pStudioRenderer, StudioRenderModel_Hooked, 20);
	//StudioRenderFinal_Hardware_Original = (StudioRenderFinal_HardwareFn)CVTableHook::HookFunction(g_pStudioRenderer, StudioRenderFinal_Hardware_Hooked, 23);
}