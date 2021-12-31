/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef USERMSG_H
#define USERMSG_H

#include "../sdk.h"

//--------------------------------------------------------------------------------------------------------------

typedef struct usermsg_s
{
	int iMsgId;
	unsigned int dwUnknown;
	char szMsg[16];
	usermsg_s *pNext;
	pfnUserMsgHook pfnHook;
} usermsg_t;

//--------------------------------------------------------------------------------------------------------------

pfnUserMsgHook HookUserMsg(const char *pszUserMessage, pfnUserMsgHook pfnHook);

void InitUserMsg();

//--------------------------------------------------------------------------------------------------------------

void BEGIN_READ( void *buf, int size );
int READ_CHAR( void );
int READ_BYTE( void );
int READ_SHORT( void );
int READ_WORD( void );
int READ_LONG( void );
float READ_FLOAT( void );
char* READ_STRING( void );
float READ_COORD( void );
float READ_ANGLE( void );
float READ_HIRESANGLE( void );
int READ_OK( void );

//--------------------------------------------------------------------------------------------------------------

class BufferWriter
{
public:
	BufferWriter();
	BufferWriter( unsigned char *buffer, int bufferLen );
	void Init( unsigned char *buffer, int bufferLen );

	void WriteByte( unsigned char data );
	void WriteLong( int data );
	void WriteString( const char *str );

	bool HasOverflowed();
	int GetSpaceUsed();

protected:
	unsigned char *m_buffer;
	int m_remaining;
	bool m_overflow;
	int m_overallLength;
};

//--------------------------------------------------------------------------------------------------------------

#endif // USERMSG_H