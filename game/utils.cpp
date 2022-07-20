// Game Utils

#include "utils.h"

#include <regex>

#include <convar.h>
#include <interface.h>
#include <sys.h>
#include <dbg.h>

#include <hl_sdk/engine/APIProxy.h>

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

static float forwardmove, sidemove, upmove; //backup for fixmove
static Vector vViewForward, vViewRight, vViewUp, vAimForward, vAimRight, vAimUp; //backup for fixmove

float *g_flNextCmdTime = NULL;
double *g_dbGameSpeed = NULL;
double *dbRealtime = NULL;

screen_info_s g_ScreenInfo;

//-----------------------------------------------------------------------------
// ConCommands, CVars..
//-----------------------------------------------------------------------------

CON_COMMAND(sc_steamid_to_steam64id, "Converts Steam ID to Steam64 ID, apostrophes \"\" are required")
{
	if (args.ArgC() > 1)
	{
		const char *pszSteamID = args[1];

		std::cmatch match;
		std::regex regex_steamid("^STEAM_[0-5]:([01]):([0-9]+)$");

		if (std::regex_search(pszSteamID, match, regex_steamid))
		{
			uint64_t steamID = 76561197960265728; // base num

			uint64_t v1 = atoll(match[1].str().c_str());
			uint64_t v2 = atoll(match[2].str().c_str());

			steamID += v1 + v2 * 2;

			Msg("Steam64 ID: %llu\n", steamID);
			Msg("https://steamcommunity.com/profiles/%llu\n", steamID);
		}
		else
		{
			Msg("Invalid SteamID, did you forget to write SteamID with apostrophes? ( \"\" )\n");
		}
	}
	else
	{
		ConMsg("Usage:  sc_steamid_to_steam64id <steamid>\n");
	}
}

CON_COMMAND(getang, "Prints current view angles")
{
	Vector va;
	g_pEngineFuncs->GetViewAngles(va);

	Msg("View Angles: %.6f %.6f %.6f\n", VectorExpand(va));
}

CON_COMMAND(setang, "Sets view angles")
{
	if (args.ArgC() >= 2)
	{
		Vector va;
		float x, y, z;

		x = atof(args[1]);

		g_pEngineFuncs->GetViewAngles(va);

		if (args.ArgC() >= 3)
		{
			y = atof(args[2]);

			if (args.ArgC() >= 4)
			{
				z = atof(args[3]);
			}
			else
			{
				z = va.z;
			}
		}
		else
		{
			y = va.y;
			z = va.z;
		}

		va.x = x;
		va.y = y;
		va.z = z;

		g_pEngineFuncs->SetViewAngles( va );
	}
	else
	{
		Msg("Usage:  setang <x> <optional: y> <optional: z>\n");
	}
}

//-----------------------------------------------------------------------------
// char * to wchar_t *
//-----------------------------------------------------------------------------

const wchar_t *UTIL_CStringToWideCString(const char *pszString)
{
	const size_t length = strlen(pszString) + 1;
	wchar_t *wcString = new wchar_t[length];

	mbstowcs(wcString, pszString, length);

	return wcString;
}

//-----------------------------------------------------------------------------
// Viewport transformations
//-----------------------------------------------------------------------------

bool UTIL_WorldToScreen(float *pflOrigin, float *pflVecScreen)
{
	int iResult = g_pTriangleAPI->WorldToScreen(pflOrigin, pflVecScreen);

	if (!iResult && pflVecScreen[0] <= 1 && pflVecScreen[1] <= 1 && pflVecScreen[0] >= -1 && pflVecScreen[1] >= -1)
	{
		pflVecScreen[0] = (g_ScreenInfo.width / 2 * pflVecScreen[0]) + (pflVecScreen[0] + g_ScreenInfo.width / 2);
		pflVecScreen[1] = -(g_ScreenInfo.height / 2 * pflVecScreen[1]) + (pflVecScreen[1] + g_ScreenInfo.height / 2);

		return true;
	}

	return false;
}

void UTIL_ScreenToWorld(float *pflNDC, float *pflWorldOrigin)
{
	g_pTriangleAPI->ScreenToWorld(pflNDC, pflWorldOrigin);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static void FixMoveStart(struct usercmd_s *cmd)
{
	forwardmove = cmd->forwardmove;
	sidemove = cmd->sidemove;
	upmove = cmd->upmove;

	if (g_pPlayerMove->iuser1 == 0)
		g_pEngineFuncs->AngleVectors(Vector(0.0f, cmd->viewangles.y, 0.0f), vViewForward, vViewRight, vViewUp);
	else
		g_pEngineFuncs->AngleVectors(cmd->viewangles, vViewForward, vViewRight, vViewUp);
}

static void FixMoveEnd(struct usercmd_s *cmd)
{
	NormalizeAngles(cmd->viewangles);

	if (g_pPlayerMove->iuser1 == 0)
		g_pEngineFuncs->AngleVectors(Vector(0.0f, cmd->viewangles.y, 0.0f), vAimForward, vAimRight, vAimUp);
	else
		g_pEngineFuncs->AngleVectors(cmd->viewangles, vAimForward, vAimRight, vAimUp);

	Vector forwardmove_normalized = vViewForward * forwardmove;
	Vector sidemove_normalized = vViewRight * sidemove;
	Vector upmove_normalized = vViewUp * upmove;

	cmd->forwardmove = DotProduct(forwardmove_normalized, vAimForward) + DotProduct(sidemove_normalized, vAimForward) + DotProduct(upmove_normalized, vAimForward);
	cmd->sidemove = DotProduct(forwardmove_normalized, vAimRight) + DotProduct(sidemove_normalized, vAimRight) + DotProduct(upmove_normalized, vAimRight);
	cmd->upmove = DotProduct(forwardmove_normalized, vAimUp) + DotProduct(sidemove_normalized, vAimUp) + DotProduct(upmove_normalized, vAimUp);

	Vector vMove(cmd->forwardmove, cmd->sidemove, cmd->upmove);
	float flSpeed = sqrtf(vMove.x * vMove.x + vMove.y * vMove.y), flYaw;
	Vector vecMove, vecRealView(cmd->viewangles);
	VectorAngles(vMove, vecMove);
	flYaw = (cmd->viewangles.y - vecRealView.y + vecMove.y) * static_cast<float>(M_PI) / 180.0f;

	cmd->forwardmove = cosf(flYaw) * flSpeed;

	if (cmd->viewangles.x >= 90.f || cmd->viewangles.x <= -90.f)
		cmd->forwardmove *= -1;

	cmd->sidemove = sinf(flYaw) * flSpeed;
}

void UTIL_SetAnglesSilent(float *angles, struct usercmd_s *cmd)
{
	FixMoveStart(cmd);

	cmd->viewangles[0] = angles[0];
	cmd->viewangles[1] = angles[1];
	cmd->viewangles[2] = angles[2];

	FixMoveEnd(cmd);
}

//-----------------------------------------------------------------------------

void UTIL_SetGameSpeed(double dbSpeed)
{
	*g_dbGameSpeed = dbSpeed * 1000.0;
}

void UTIL_SendPacket(bool bSend)
{
	if (bSend)
		*g_flNextCmdTime = 0.0f;
	else
		*g_flNextCmdTime = FLT_MAX;
}