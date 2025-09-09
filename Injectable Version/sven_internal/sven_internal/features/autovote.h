// Auto Vote

#pragma once

typedef int (__thiscall *MsgFunc_VoteMenuFn)(void *, int, int, int);

typedef int (*READ_BYTE_Fn)();
typedef char *(*READ_STRING_Fn)();

typedef void (__thiscall *CVotePopup__OpenFn)(void *);
typedef void (__thiscall *CVotePopup__CloseFn)(void *);

void InitAutoVote();