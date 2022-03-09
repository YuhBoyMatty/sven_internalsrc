// Game Utils

#include <regex>

#include "utils.h"
#include "mathlib.h"

#include "../libdasm/libdasm.h"

#include "../utils/signature_scanner.h"
#include "../utils/patcher.h"

#include "../game/console.h"
#include "../patterns.h"

#include "../modules/vgui.h"

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern playermove_s *g_pPlayerMove;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

static float forwardmove, sidemove, upmove; //backup for fixmove
static Vector vViewForward, vViewRight, vViewUp, vAimForward, vAimRight, vAimUp; //backup for fixmove

static float *s_flNextCmdTime = NULL;
static double *s_dbGameSpeed = NULL;

double *dbRealtime = NULL;

MEMORY_PATCHER(GaussTertiaryAttack);
MEMORY_PATCHER(MinigunTertiaryAttack);
MEMORY_PATCHER(HandGrenadeTertiaryAttack);
MEMORY_PATCHER(ShockRifleTertiaryAttack);

MEMORY_PATCHER(GaussTertiaryAttack_Server);
MEMORY_PATCHER(MinigunTertiaryAttack_Server);
MEMORY_PATCHER(HandGrenadeTertiaryAttack_Server);
MEMORY_PATCHER(ShockRifleTertiaryAttack_Server);
MEMORY_PATCHER(GluonGunTertiaryAttack_Server);

static bool s_bTertiaryAttackGlitchInitialized = false;
static bool s_bTertiaryAttackGlitchInitialized_Server = false;

static BYTE s_TertiaryAttackPatchedBytes[] =
{
	0xC3, // RETURN
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 // NOP
};

//-----------------------------------------------------------------------------
// ConCommands, CVars..
//-----------------------------------------------------------------------------

CON_COMMAND(disasm_addr, "disasm_addr [address]")
{
	if (CMD_ARGC() >= 2)
	{
		std::cmatch match;
		std::regex regex_address("^([a-zA-Z0-9]+.(dll|exe))(([+-]{1})([0-9abcdefABCDEF]+)){0,1}$");

		const char *pszAddress = CMD_ARGV(1);
		BYTE *pAddress = NULL;

		if (std::regex_search(pszAddress, match, regex_address))
		{
			//const wchar_t *pwcModuleName = CStringToWideCString(match[1].str().c_str());
			pAddress = (BYTE *)GetModuleHandleA(match[1].str().c_str());

			if (match[4].str().c_str() && match[5].str().c_str())
			{
				if (*match[4].str().c_str() == '+')
					pAddress = (BYTE *)((DWORD)pAddress + strtol(match[5].str().c_str(), NULL, 16));
				else
					pAddress = (BYTE *)((DWORD)pAddress - strtol(match[5].str().c_str(), NULL, 16));
			}

			//delete[] pwcModuleName;
		}
		else
		{
			pAddress = (BYTE *)strtol(pszAddress, NULL, 16);
		}

		if (IsBadReadPtr(pAddress, 15)) // longest instruction length
		{
			Msg("Failed to disassemble address. Memory access violation.\n");
		}
		else
		{
			INSTRUCTION instruction;

			int length = get_instruction(&instruction, pAddress, MODE_32);

			if (length)
			{
				Msg("======= Instruction Info =======\n\n");

				Msg("Address: %X\n\n", pAddress);
				Msg("Opcode: %X\nOpcode Length: %d\nOpcode Type: %d\n\n", instruction.opcode, length, instruction.type);
				Msg("Operand #1 Type: %d\nDisplacement Value: %X\nImmediate Value: %X\n\n", instruction.op1.type, instruction.op1.displacement, instruction.op1.immediate);
				Msg("Operand #2 Type: %d\nDisplacement Value: %X\nImmediate Value: %X\n\n", instruction.op2.type, instruction.op2.displacement, instruction.op2.immediate);
				Msg("Operand #3 Type: %d\nDisplacement Value: %X\nImmediate Value: %X\n\n", instruction.op3.type, instruction.op3.displacement, instruction.op3.immediate);

				Msg("======= Instruction Info =======\n\n");
			}
		}
	}
	else
	{
		disasm_addr.PrintUsage();
	}
}

CON_COMMAND(dump_ifaces, "dump_ifaces [module name] - Dumps all registered interfaces from the specified module")
{
	if (CMD_ARGC() > 1)
	{
		//const wchar_t *pwcModuleName = CStringToWideCString(CMD_ARGV(1));
		HMODULE hModule = GetModuleHandleA(CMD_ARGV(1));

		if (hModule)
		{
			CreateInterfaceFn pfnCreateInterface = (CreateInterfaceFn)GetProcAddress(hModule, "CreateInterface");

			if (pfnCreateInterface)
			{
				INSTRUCTION instruction;
				BYTE *pAddress = (BYTE *)pfnCreateInterface;

				int it = 1;
				int disassembledBytes = 0;
				
				while (disassembledBytes < 0x80)
				{
					int length = get_instruction(&instruction, pAddress, MODE_32);

					disassembledBytes += length;
					pAddress += length;

					if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
					{
						InterfaceReg *pInterfaceReg = *reinterpret_cast<InterfaceReg **>(instruction.op2.displacement);

						for (; pInterfaceReg != NULL; pInterfaceReg = pInterfaceReg->m_pNext)
						{
							Msg("[%d] %s (%X)\n", it++, pInterfaceReg->m_pName, pInterfaceReg->m_CreateFn());
						}

						break;
					}
				}
			}
			else
			{
				Msg("Function CreateInterface not found\n");
			}
		}
		else
		{
			Msg("No such module\n");
		}

		//delete[] pwcModuleName;
	}
	else
	{
		dump_ifaces.PrintUsage();
	}
}

CON_COMMAND(steamid_to_steamid64, "steamid_to_steamid64 [steamid] - Converts SteamID to SteamID64, apostrophes \"\" are required!")
{
	if (CMD_ARGC() > 1)
	{
		const char *pszSteamID = CMD_ARGV(1);

		std::cmatch match;
		std::regex regex_steamid("^STEAM_[0-5]:([01]):([0-9]+)$");

		if (std::regex_search(pszSteamID, match, regex_steamid))
		{
			uint64_t steamID = 76561197960265728; // base num

			uint64_t v1 = atoll(match[1].str().c_str());
			uint64_t v2 = atoll(match[2].str().c_str());

			steamID += v1 + v2 * 2;

			Msg("SteamID64: %llu\n", steamID);
			Msg("https://steamcommunity.com/profiles/%llu\n", steamID);
		}
		else
		{
			Msg("Invalid SteamID, did you forget to write SteamID with apostrophes? ( \"\" )\n");
		}
	}
	else
	{
		steamid_to_steamid64.PrintUsage();
	}
}

//-----------------------------------------------------------------------------
// Prints wrappers
//-----------------------------------------------------------------------------

void W_Msg(const char *pszMsg, ...)
{
	va_list args;

	va_start(args, pszMsg);
	g_pEngineFuncs->Con_Printf(pszMsg, args);
	va_end(args);
}

template<typename... Args>
void T_Msg(const char *pszMsg, Args... args)
{
	g_pEngineFuncs->Con_Printf(pszMsg, args...);
}

//-----------------------------------------------------------------------------
// char * to wchar_t *
//-----------------------------------------------------------------------------

const wchar_t *CStringToWideCString(const char *pszString)
{
	const size_t length = strlen(pszString) + 1;
	wchar_t *wcString = new wchar_t[length];

	mbstowcs(wcString, pszString, length);

	return wcString;
}

//-----------------------------------------------------------------------------
// Viewport transformations
//-----------------------------------------------------------------------------

bool WorldToScreen(float *pflOrigin, float *pflVecScreen)
{
	int iResult = g_pEngineFuncs->pTriAPI->WorldToScreen(pflOrigin, pflVecScreen);

	if (pflVecScreen[0] <= 1 && pflVecScreen[1] <= 1 && pflVecScreen[0] >= -1 && pflVecScreen[1] >= -1 && !iResult)
	{
		pflVecScreen[0] = (g_ScreenInfo.width / 2 * pflVecScreen[0]) + (pflVecScreen[0] + g_ScreenInfo.width / 2);
		pflVecScreen[1] = -(g_ScreenInfo.height / 2 * pflVecScreen[1]) + (pflVecScreen[1] + g_ScreenInfo.height / 2);

		return true;
	}

	return false;
}

void ScreenToWorld(float *pflNDC, float *pflWorldOrigin)
{
	g_pEngineFuncs->pTriAPI->ScreenToWorld(pflNDC, pflWorldOrigin);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static void FixMoveStart(struct usercmd_s *cmd)
{
	forwardmove = cmd->forwardmove;
	sidemove = cmd->sidemove;
	upmove = cmd->upmove;

	if (g_pPlayerMove->iuser1 == 0)
		g_pEngineFuncs->pfnAngleVectors(Vector(0.0f, cmd->viewangles.y, 0.0f), vViewForward, vViewRight, vViewUp);
	else
		g_pEngineFuncs->pfnAngleVectors(cmd->viewangles, vViewForward, vViewRight, vViewUp);
}

static void FixMoveEnd(struct usercmd_s *cmd)
{
	NormalizeAngles(cmd->viewangles);

	if (g_pPlayerMove->iuser1 == 0)
		g_pEngineFuncs->pfnAngleVectors(Vector(0.0f, cmd->viewangles.y, 0.0f), vAimForward, vAimRight, vAimUp);
	else
		g_pEngineFuncs->pfnAngleVectors(cmd->viewangles, vAimForward, vAimRight, vAimUp);

	Vector forwardmove_normalized = forwardmove * vViewForward;
	Vector sidemove_normalized = sidemove * vViewRight;
	Vector upmove_normalized = upmove * vViewUp;

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

void SetAnglesSilent(float *angles, struct usercmd_s *cmd)
{
	if ((g_pPlayerMove && g_pPlayerMove->movetype != MOVETYPE_WALK) || (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2 || cmd->buttons & IN_USE))
		return;

	FixMoveStart(cmd);

	if (g_pPlayerMove && g_pPlayerMove->waterlevel != 0)
	{
		cmd->viewangles[0] = 0.f;
	}
	else
	{
		cmd->viewangles[0] = angles[0];
	}

	cmd->viewangles[1] = angles[1];
	cmd->viewangles[2] = angles[2];

	FixMoveEnd(cmd);
}

//-----------------------------------------------------------------------------

void SetGameSpeed(double dbSpeed)
{
	*s_dbGameSpeed = dbSpeed * 1000.0;
}

void SendPacket(bool bSend)
{
	if (bSend)
		*s_flNextCmdTime = 0.0f;
	else
		*s_flNextCmdTime = FLT_MAX;
}

void EnableTertiaryAttackGlitch()
{
	GaussTertiaryAttack.Patch();
	MinigunTertiaryAttack.Patch();
	HandGrenadeTertiaryAttack.Patch();
	ShockRifleTertiaryAttack.Patch();
}

void DisableTertiaryAttackGlitch()
{
	GaussTertiaryAttack.Unpatch();
	MinigunTertiaryAttack.Unpatch();
	HandGrenadeTertiaryAttack.Unpatch();
	ShockRifleTertiaryAttack.Unpatch();
}

bool IsTertiaryAttackGlitchPatched()
{
	return GaussTertiaryAttack.IsPatched();
}

bool IsTertiaryAttackGlitchInit()
{
	return s_bTertiaryAttackGlitchInitialized;
}

void EnableTertiaryAttackGlitch_Server() // what's the point of loading 'server' library when you create a server????
{
	GaussTertiaryAttack_Server.Patch();
	MinigunTertiaryAttack_Server.Patch();
	HandGrenadeTertiaryAttack_Server.Patch();
	ShockRifleTertiaryAttack_Server.Patch();
	GluonGunTertiaryAttack_Server.Patch();
}

void DisableTertiaryAttackGlitch_Server()
{
	GaussTertiaryAttack_Server.Unpatch();
	MinigunTertiaryAttack_Server.Unpatch();
	HandGrenadeTertiaryAttack_Server.Unpatch();
	ShockRifleTertiaryAttack_Server.Unpatch();
	GluonGunTertiaryAttack_Server.Unpatch();
}

bool IsTertiaryAttackGlitchPatched_Server()
{
	return GaussTertiaryAttack_Server.IsPatched();
}

bool IsTertiaryAttackGlitchInit_Server()
{
	return s_bTertiaryAttackGlitchInitialized_Server;
}

//-----------------------------------------------------------------------------

static bool PatchInterp()
{
	DWORD dwProtection;
	HMODULE hHardwareDLL = GetModuleHandle(L"hw.dll");

	void *pPatchInterpString = FindString(hHardwareDLL, "cl_updaterate min");

	if (!pPatchInterpString)
	{
		//Sys_Error("'Patch Interp' failed initialization\n");
		Msg("'Patch Interp' failed initialization\n");
		return false;
	}

	BYTE *pPatchInterp = (BYTE *)FindAddress(hHardwareDLL, pPatchInterpString);

	if (!pPatchInterp)
	{
		//Sys_Error("'Patch Interp' failed initialization #2\n");
		Msg("'Patch Interp' failed initialization #2\n");
		return false;
	}

	// go back to PUSH opcode
	pPatchInterp -= 0x1;

	if (*pPatchInterp != 0x68) // check PUSH opcode
	{
		//Sys_Error("'Patch Interp' failed initialization #3\n");
		Msg("'Patch Interp' failed initialization #3\n");
		return false;
	}

	if (*(pPatchInterp - 0x1F) != 0x7A) // JP opcode
	{
		//Sys_Error("'Patch Interp' failed initialization #4\n");
		Msg("'Patch Interp' failed initialization #4\n");
		return false;
	}

	// Patch cl_updaterate min.
	VirtualProtect(pPatchInterp - 0x1F, sizeof(BYTE), PAGE_EXECUTE_READWRITE, &dwProtection);
	*(pPatchInterp - 0x1F) = 0xEB;
	//FlushInstructionCache(GetCurrentProcess(), pPatchInterp - 0x1F, sizeof(BYTE));
	VirtualProtect(pPatchInterp - 0x1F, sizeof(BYTE), dwProtection, &dwProtection);
	
	// Go to PUSH string 'cl_updaterate max...'
	pPatchInterp += 0x3F;

	if (*pPatchInterp != 0x68) // check PUSH opcode
	{
		//Sys_Error("'Patch Interp' failed initialization #5\n");
		Msg("'Patch Interp' failed initialization #5\n");
		return false;
	}

	if (*(pPatchInterp - 0x1F) != 0x7A) // JP opcode
	{
		//Sys_Error("'Patch Interp' failed initialization #6\n");
		Msg("'Patch Interp' failed initialization #6\n");
		return false;
	}

	// Patch cl_updaterate max.
	VirtualProtect(pPatchInterp - 0x1F, sizeof(BYTE), PAGE_EXECUTE_READWRITE, &dwProtection);
	*(pPatchInterp - 0x1F) = 0xEB;
	VirtualProtect(pPatchInterp - 0x1F, sizeof(BYTE), dwProtection, &dwProtection);
	
	// Go to PUSH string 'ex_interp forced up...'
	pPatchInterp += 0x62;

	if (*pPatchInterp != 0xB8) // check MOV, EAX ... opcode
	{
		//Sys_Error("'Patch Interp' failed initialization #7\n");
		Msg("'Patch Interp' failed initialization #7\n");
		return false;
	}

	if (*(pPatchInterp - 0x8) != 0x7D) // JNL opcode
	{
		//Sys_Error("'Patch Interp' failed initialization #8\n");
		Msg("'Patch Interp' failed initialization #8\n");
		return false;
	}

	// Patch ex_interp force up
	VirtualProtect(pPatchInterp - 0x8, sizeof(BYTE), PAGE_EXECUTE_READWRITE, &dwProtection);
	*(pPatchInterp - 0x8) = 0xEB;
	VirtualProtect(pPatchInterp - 0x8, sizeof(BYTE), dwProtection, &dwProtection);

	if (*(pPatchInterp + 0xD) != 0x7E) // JLE opcode
	{
		//Sys_Error("'Patch Interp' failed initialization #9\n");
		Msg("'Patch Interp' failed initialization #9\n");
		return false;
	}

	// Patch ex_interp force down
	VirtualProtect(pPatchInterp + 0xD, sizeof(BYTE), PAGE_EXECUTE_READWRITE, &dwProtection);
	*(pPatchInterp + 0xD) = 0xEB;
	VirtualProtect(pPatchInterp + 0xD, sizeof(BYTE), dwProtection, &dwProtection);

	return true;
}

static void InitTertiaryAttackPatch(CPatcher &patcher, void *pfnTertiaryAttack)
{
	INSTRUCTION instruction;

	get_instruction(&instruction, (BYTE *)pfnTertiaryAttack, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
	{
		get_instruction(&instruction, (BYTE *)pfnTertiaryAttack + 0x2, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_JMP && instruction.op1.type == OPERAND_TYPE_MEMORY)
		{
			patcher.Init(pfnTertiaryAttack, s_TertiaryAttackPatchedBytes, sizeof(s_TertiaryAttackPatchedBytes));
		}
		else
		{
			Sys_Error("'InitTertiaryAttackPatch' failed initialization #2\n");
		}
	}
	else
	{
		Sys_Error("'InitTertiaryAttackPatch' failed initialization #1\n");
	}
}

static void InitTertiaryAttackGlitch()
{
	INSTRUCTION instruction;
	DWORD *dwVTable[] = { NULL, NULL, NULL, NULL };

	auto clientDLL = GetModuleHandle(L"client.dll");

	if (clientDLL)
	{
		void *weapon_gauss = GetProcAddress(clientDLL, "weapon_gauss"); // vtable <- (byte *)weapon_gauss + 0x63
		void *weapon_minigun = GetProcAddress(clientDLL, "weapon_minigun"); // vtable <- (byte *)weapon_minigun + 0x63
		void *weapon_handgrenade = GetProcAddress(clientDLL, "weapon_handgrenade"); // vtable <- (byte *)weapon_handgrenade + 0x63
		void *weapon_shockrifle = GetProcAddress(clientDLL, "weapon_shockrifle"); // vtable <- (byte *)weapon_shockrifle + 0x67

		if (!weapon_gauss || !weapon_minigun || !weapon_handgrenade || !weapon_shockrifle)
			Sys_Error("InitTertiaryAttackGlitch: GetProcAddress failed\n");

		// weapon_gauss
		get_instruction(&instruction, (BYTE *)weapon_gauss + 0x63, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			dwVTable[0] = reinterpret_cast<DWORD *>(instruction.op2.immediate);
		else
			Sys_Error("'InitTertiaryAttackGlitch' failed initialization #1\n");
		
		// weapon_minigun
		get_instruction(&instruction, (BYTE *)weapon_minigun + 0x63, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			dwVTable[1] = reinterpret_cast<DWORD *>(instruction.op2.immediate);
		else
			Sys_Error("'InitTertiaryAttackGlitch' failed initialization #2\n");
		
		// weapon_handgrenade
		get_instruction(&instruction, (BYTE *)weapon_handgrenade + 0x63, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			dwVTable[2] = reinterpret_cast<DWORD *>(instruction.op2.immediate);
		else
			Sys_Error("'InitTertiaryAttackGlitch' failed initialization #3\n");
		
		// weapon_shockrifle
		get_instruction(&instruction, (BYTE *)weapon_shockrifle + 0x67, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			dwVTable[3] = reinterpret_cast<DWORD *>(instruction.op2.immediate);
		else
			Sys_Error("'InitTertiaryAttackGlitch' failed initialization #4\n");

		InitTertiaryAttackPatch(GaussTertiaryAttack, (void *)dwVTable[0][150]); // CBasePlayerWeapon::TertiaryAttack
		InitTertiaryAttackPatch(MinigunTertiaryAttack, (void *)dwVTable[1][150]);
		InitTertiaryAttackPatch(HandGrenadeTertiaryAttack, (void *)dwVTable[2][150]);
		InitTertiaryAttackPatch(ShockRifleTertiaryAttack, (void *)dwVTable[3][150]);

		s_bTertiaryAttackGlitchInitialized = true;
	}
	else
	{
		Sys_Error("InitTertiaryAttackGlitch: client module??\n");
	}
}

void InitTertiaryAttackGlitch_Server()
{
	INSTRUCTION instruction;
	DWORD *dwVTable[] = { NULL, NULL, NULL, NULL, NULL };

	auto serverDLL = GetModuleHandle(L"server.dll");

	if (serverDLL)
	{
		void *weapon_gauss = GetProcAddress(serverDLL, "weapon_gauss"); // vtable <- (byte *)weapon_gauss + 0x7B
		void *weapon_minigun = GetProcAddress(serverDLL, "weapon_minigun"); // vtable <- (byte *)weapon_minigun + 0x7B
		void *weapon_handgrenade = GetProcAddress(serverDLL, "weapon_handgrenade"); // vtable <- (byte *)weapon_handgrenade + 0x7B
		void *weapon_shockrifle = GetProcAddress(serverDLL, "weapon_shockrifle"); // vtable <- (byte *)weapon_shockrifle + 0x83
		void *weapon_egon = GetProcAddress(serverDLL, "weapon_egon"); // vtable <- (byte *)weapon_egon + 0x83

		if (!weapon_gauss || !weapon_minigun || !weapon_handgrenade || !weapon_shockrifle || !weapon_egon)
			Sys_Error("InitTertiaryAttackGlitch_Server: GetProcAddress failed\n");

		// weapon_gauss
		get_instruction(&instruction, (BYTE *)weapon_gauss + 0x7B, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			dwVTable[0] = reinterpret_cast<DWORD *>(instruction.op2.immediate);
		else
			Sys_Error("'InitTertiaryAttackGlitch_Server' failed initialization #1\n");

		// weapon_minigun
		get_instruction(&instruction, (BYTE *)weapon_minigun + 0x7B, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			dwVTable[1] = reinterpret_cast<DWORD *>(instruction.op2.immediate);
		else
			Sys_Error("'InitTertiaryAttackGlitch_Server' failed initialization #2\n");

		// weapon_handgrenade
		get_instruction(&instruction, (BYTE *)weapon_handgrenade + 0x7B, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			dwVTable[2] = reinterpret_cast<DWORD *>(instruction.op2.immediate);
		else
			Sys_Error("'InitTertiaryAttackGlitch_Server' failed initialization #3\n");

		// weapon_shockrifle
		get_instruction(&instruction, (BYTE *)weapon_shockrifle + 0x83, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			dwVTable[3] = reinterpret_cast<DWORD *>(instruction.op2.immediate);
		else
			Sys_Error("'InitTertiaryAttackGlitch_Server' failed initialization #4\n");
		
		// weapon_egon
		get_instruction(&instruction, (BYTE *)weapon_egon + 0x83, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			dwVTable[4] = reinterpret_cast<DWORD *>(instruction.op2.immediate);
		else
			Sys_Error("'InitTertiaryAttackGlitch_Server' failed initialization #5\n");

		InitTertiaryAttackPatch(GaussTertiaryAttack_Server, (void *)dwVTable[0][151]);
		InitTertiaryAttackPatch(MinigunTertiaryAttack_Server, (void *)dwVTable[1][151]);
		InitTertiaryAttackPatch(HandGrenadeTertiaryAttack_Server, (void *)dwVTable[2][151]);
		InitTertiaryAttackPatch(ShockRifleTertiaryAttack_Server, (void *)dwVTable[3][151]);
		InitTertiaryAttackPatch(GluonGunTertiaryAttack_Server, (void *)dwVTable[4][151]);

		s_bTertiaryAttackGlitchInitialized_Server = true;
	}
	else
	{
		Sys_Error("InitTertiaryAttackGlitch: server module??\n");
	}
}

// main func

void InitUtils()
{
	DWORD dwProtection;
	INSTRUCTION instruction;
	HMODULE hHardwareDLL = GetModuleHandle(L"hw.dll");

	if (!PatchInterp())
	{
		Msg("PATCH FAILURE\n");
		//return;
	}

	InitTertiaryAttackGlitch();

	void *pNextCmdTime = FindPattern(hHardwareDLL, Patterns::Hardware::flNextCmdTime);

	if (!pNextCmdTime)
	{
		Sys_Error("'flNextCmdTime' failed initialization\n");
		return;
	}

	void *pTextureLoadAddress = FindString(hHardwareDLL, "Texture load: %6.1fms");

	if (!pTextureLoadAddress)
	{
		Sys_Error("'Game speed up' failed initialization\n");
		return;
	}

	void *pTextureLoad = FindAddress(hHardwareDLL, pTextureLoadAddress);

	if (!pTextureLoad)
	{
		Sys_Error("'Game speed up' failed initialization #2\n");
		return;
	}

	// s_dbGameSpeed
	get_instruction(&instruction, (BYTE *)pTextureLoad - 0xA, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_FMUL && instruction.op1.type == OPERAND_TYPE_MEMORY)
	{
		s_dbGameSpeed = reinterpret_cast<double *>(instruction.op1.displacement);
	}
	else
	{
		Sys_Error("'Game speed up' failed initialization #3\n");
		return;
	}

	// s_flNextCmdTime
	get_instruction(&instruction, (BYTE *)pNextCmdTime, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_FSTP && instruction.op1.type == OPERAND_TYPE_MEMORY)
	{
		s_flNextCmdTime = reinterpret_cast<float *>(instruction.op1.displacement);
	}
	else
	{
		Sys_Error("'flNextCmdTime' failed initialization #2\n");
		return;
	}
	
	// dbRealtime
	get_instruction(&instruction, (BYTE *)g_pEngineFuncs->pNetAPI->SendRequest + 0x88, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_FLD && instruction.op1.type == OPERAND_TYPE_MEMORY)
	{
		dbRealtime = reinterpret_cast<double *>(instruction.op1.displacement);
	}
	else
	{
		Sys_Error("'dbRealtime' failed initialization\n");
		return;
	}

	VirtualProtect(s_dbGameSpeed, sizeof(double), PAGE_EXECUTE_READWRITE, &dwProtection);
}