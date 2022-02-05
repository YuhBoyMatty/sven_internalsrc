// Chat Colors

#pragma once

#include <stdint.h>

#include "../data_struct/hashtable.h"

class CChatColors
{
public:
	CChatColors();

	void Init();

public:
	void LoadPlayers();
	int *FindPlayerInList(int playerIndex);

private:
	enum token_state
	{
		TOKEN_READ_FAILED = -1,
		TOKEN_READ_OK = 0,
		TOKEN_READ_EMPTY_BUFFER,
		TOKEN_READ_STEAMID,
		TOKEN_READ_ASSIGNMENT,
		TOKEN_READ_TEAM_NUMBER
	};

	token_state ReadToken(uint64_t *steamID, int *teamnum, const char *pszBuffer);

private:
	CHashTable<uint64_t, int> m_HashTable;
};

extern CChatColors g_ChatColors;