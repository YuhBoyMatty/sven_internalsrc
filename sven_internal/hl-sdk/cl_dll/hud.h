#pragma once

#define HUD_ACTIVE 1
#define HUD_INTERMISSION 2

class CHudBase
{
public:
	struct
	{
		int x;
		int y;
	} m_pos;

	int m_type;
	int m_iFlags; // active, moving,

	virtual ~CHudBase() {}
	virtual bool Init() { return false; }
	virtual bool VidInit() { return false; }
	virtual bool Draw(float flTime) { return false; }
	virtual void Think() {}
	virtual void Reset() {}
	virtual void InitHUDData() {} // called every time a server is connected to
};

struct HUDLIST
{
	CHudBase *p;
	HUDLIST *pNext;
};

class CHud
{
public:
	HUDLIST *m_pHudList;
	// next class members are messed up, need to reverse them..
};