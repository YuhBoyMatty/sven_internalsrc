// Weapon Resource

#include "ammo.h"

#include "../game/utils.h"
#include "../libdasm/libdasm.h"
#include "../utils/signature_scanner.h"
#include "../patterns.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

WeaponsResource *g_pWR = NULL;

//-----------------------------------------------------------------------------
// Implementations
//-----------------------------------------------------------------------------

int WeaponsResource::HasAmmo( WEAPON *p )
{
	if ( !p )
		return false;

	// weapons with no max ammo can always be selected
	if ( p->iMax1 == -1 )
		return true;

	return (p->iAmmoType == -1) || p->iClip > 0 || CountAmmo(p->iAmmoType) || CountAmmo(p->iAmmo2Type) || ( p->iFlags & WEAPON_FLAGS_SELECTONEMPTY );
}

int WeaponsResource::CountAmmo( int iId ) 
{ 
	if ( iId < 0 )
		return 0;

	return riAmmo[iId];
}

void WeaponsResource::IterateWeapons( void (*pfnCallback)(WEAPON *pWeapon, bool bHasAmmo, int nAmmo, int nAmmo2) )
{
	for (int i = 0; i < MAX_WEAPON_SLOTS; ++i)
	{
		for (int j = 0; j < MAX_WEAPON_POSITIONS; ++j)
		{
			WEAPON *pWeapon = GetWeaponSlot(i, j);

			if (pWeapon)
			{
				pfnCallback(pWeapon, HasAmmo(pWeapon), CountAmmo(pWeapon->iAmmoType), CountAmmo(pWeapon->iAmmo2Type));
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Find gWR singleton
//-----------------------------------------------------------------------------

void InitWeaponsResource()
{
	INSTRUCTION instruction;

	void *pWeaponsResource__GetFirstPos = FIND_PATTERN(L"client.dll", Patterns::Client::WeaponsResource__GetFirstPos);

	if (!pWeaponsResource__GetFirstPos)
		Sys_Error("'WeaponsResource' failed initialization #1\n");
	
	get_instruction(&instruction, (BYTE *)pWeaponsResource__GetFirstPos + 0xB, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_ADD && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
		g_pWR = reinterpret_cast<WeaponsResource *>(instruction.op2.immediate);
	else
		Sys_Error("'WeaponsResource' failed initialization #2\n");
}