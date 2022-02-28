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


class CHudBaseTextBlock : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	// CHudBase::Think - not overriden
	// CHudBase::Reset - not overriden
	void InitHUDData() override;
	virtual void FlushText();
};

class CHudSayText : public CHudBaseTextBlock
{
public:
	bool Init() override;
	// CHudBaseTextBlock::VidInit() - not overriden
	bool Draw(float flTime) override;
	// CHudBase::Think - not overriden
	// CHudBase::Reset - not overriden
	// CHudBaseTextBlock::InitHUDData - not overriden
	// CHudBaseTextBlock::FlushText - not overriden
	virtual float GetScrollTime();
	// @messageType: 0 - client, 1 - server, Should be more? I didn't go further
	virtual void PrintText(int messageType, const char* pszBuf, int iBufSize, int clientIndex);
	virtual void UpdatePos();
	
	bool MsgFunc_SayText(const char *pszName, int iSize, void *pbuf);
	void SayTextPrint(const char *pszBuf, int iBufSize, int clientIndex = -1);
	void EnsureTextFitsInOneLineAndWrapIfHaveTo(int line);
	friend class CHudSpectator;

private:
	// FIXME: validate offsets
	struct cvar_s *m_HUD_saytext;
	struct cvar_s *m_HUD_saytext_time;
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