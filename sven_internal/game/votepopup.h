// Vote Popup

#ifndef VOTEPOPUP_H
#define VOTEPOPUP_H

#ifdef _WIN32
#pragma once
#endif

#pragma comment(lib, "vgui.lib")

#include <VGUI_Panel.h>
#include <VGUI_Label.h>

enum VoteType
{
	VOTEKILL = 1,
	VOTEKICK,
	VOTEBAN,
	VOTEMAP
};

using namespace vgui;

class CTransparentPanel : public Panel
{
private:
	int	m_iTransparency;
public:
	CTransparentPanel(int iTrans, int x,int y,int wide,int tall) : Panel(x,y,wide,tall)
	{
		m_iTransparency = iTrans;
	}

	virtual void paintBackground();
};

class CMenuPanel : public CTransparentPanel
{
private:
	CMenuPanel *m_pNextMenu;
	int			m_iMenuID;
	int			m_iRemoveMe;
	int			m_iIsActive;
	float		m_flOpenTime;
public:
	CMenuPanel(int iRemoveMe, int x,int y,int wide,int tall) : CTransparentPanel(100, x,y,wide,tall)
	{
		Reset();
		m_iRemoveMe = iRemoveMe;
	}

	CMenuPanel(int iTrans, int iRemoveMe, int x,int y,int wide,int tall) : CTransparentPanel(iTrans, x,y,wide,tall)
	{
		Reset();
		m_iRemoveMe = iRemoveMe;
	}

	virtual void Reset( void );

	void SetNextMenu( CMenuPanel *pNextPanel )
	{
		if (m_pNextMenu)
			m_pNextMenu->SetNextMenu( pNextPanel );
		else
			m_pNextMenu = pNextPanel;
	}

	void SetMenuID( int iID )
	{
		m_iMenuID = iID;
	}

	void SetActive( int iState )
	{
		m_iIsActive = iState;
	}

	virtual void Open( void );

	virtual void Close( void );

	int			ShouldBeRemoved() { return m_iRemoveMe; };
	CMenuPanel* GetNextMenu() { return m_pNextMenu; };
	int			GetMenuID() { return m_iMenuID; };
	int			IsActive() { return m_iIsActive; };
	float		GetOpenTime() { return m_flOpenTime; };

	// Numeric input
	virtual bool SlotInput( int iSlot );
	virtual void SetActiveInfo( int iInput );
};

class CVotePopup : public CMenuPanel
{
public:
	virtual void Update();
	virtual bool MsgFunc_VoteMenu(int a1, int a2, int a3, int a4, int a5, int a6);

public:
	char pad[18];
	Label label;
};

struct CVoteInfo
{
	VoteType m_type = static_cast<VoteType>(-1);

	char *m_pszVoteMessage = NULL;

	char *m_pszVoteYes = NULL;
	char *m_pszVoteNo = NULL;
};

#endif