// Weapon Resource

#pragma once

#include "../sdk.h"

#define MAX_WEAPON_POSITIONS 25

class WeaponsResource
{
private:
	WEAPON *rgSlots[MAX_WEAPON_SLOTS][MAX_WEAPON_POSITIONS];	// The slots currently in use by weapons.  The value is a pointer to the weapon;  if it's NULL, no weapon is there
	char pad[544];
	int	riAmmo[MAX_AMMO_TYPES];							// count of each ammo type

public:

	WEAPON *GetWeaponSlot(int slot, int pos)
	{
		return rgSlots[slot][pos];
	}

	void LoadWeaponSprites(WEAPON *wp);
	void LoadAllWeaponSprites();
	WEAPON *GetFirstPos(int iSlot);
	void SelectSlot(int iSlot, int fAdvance, int iDirection);
	WEAPON *GetNextActivePos(int iSlot, int iSlotPos);

	int HasAmmo(WEAPON *p);

	///// AMMO /////
	AMMO GetAmmo(int iId)
	{
		return iId;
	}

	void SetAmmo(int iId, int iCount)
	{
		riAmmo[iId] = iCount;
	}

	int CountAmmo(int iId);

	HSPRITE *GetAmmoPicFromWeapon(int iAmmoId, Rect &rect);

	// User methods
	void IterateWeapons(void (*pfnCallback)(WEAPON *pWeapon, bool bHasAmmo, int nAmmo, int nAmmo2));
};

void InitWeaponsResource();

extern WeaponsResource *g_pWR;