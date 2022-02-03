// Custom Vote Popup

#pragma once

typedef enum
{
	VOTEKILL = 1,
	VOTEKICK,
	VOTEBAN,
	VOTEMAP
} votetype_t;

class CUserVotePopup
{
public:
	CUserVotePopup();

	void Init();

public:
	void Draw();

	bool OnVoteStart(const char *pszUserMsg, int iSize, void *pBuffer);
	bool OnVoteEnd(const char *pszUserMsg, int iSize, void *pBuffer);

	void OnHUDInit();
	void OnVideoInit();
	void OnKeyPress(int bKeyDown, int nKey);

private:
	votetype_t m_voteType;

	wchar_t m_wszVoteTarget[128];
	wchar_t m_wszVoteMessage[128];
	wchar_t m_wszVoteYes[128];
	wchar_t m_wszVoteNo[128];
	
	char m_szVoteTarget[128];

	bool m_bShowPopup;
	bool m_bVoteStarted;
};

extern CUserVotePopup g_VotePopup;