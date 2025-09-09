#ifndef IKEYVALUES_H
#define IKEYVALUES_H

#include <common/interface.h>
#include "VGUI2.h"

namespace vgui
{
	class IKeyValues : public IBaseInterface
	{
	public:
		virtual void RegisterSizeofKeyValues(int size) = 0;
		virtual void *AllocKeyValuesMemory(int size) = 0;
		virtual void FreeKeyValuesMemory(void *pMem) = 0;
		virtual HKeySymbol GetSymbolForString(const char *name) = 0;
		virtual const char *GetStringForSymbol(HKeySymbol symbol) = 0;
		virtual void GetLocalizedFromANSI(const char *ansi, wchar_t *outBuf, int unicodeBufferSizeInBytes) = 0;
		virtual void GetANSIFromLocalized(const wchar_t *wchar, char *outBuf, int ansiBufferSizeInBytes) = 0;
		virtual void AddKeyValuesToMemoryLeakList(void *pMem, HKeySymbol name) = 0;
		virtual void RemoveKeyValuesFromMemoryLeakList(void *pMem) = 0;
	};
}

#define VGUI_KEYVALUES_INTERFACE_VERSION "KeyValues003"

#endif // IKEYVALUES_H
