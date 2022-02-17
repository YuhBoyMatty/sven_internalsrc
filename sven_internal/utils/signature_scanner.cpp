// Signature Scanner

#pragma once

#include "signature_scanner.h"

#include "../data_struct/hashtable.h"

//-----------------------------------------------------------------------------
// Structure declarations
//-----------------------------------------------------------------------------

struct moduleinfo_s
{
	void *pBaseOfDll;
	unsigned int SizeOfImage;
};

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

static CHashTable<HMODULE, moduleinfo_s> s_ModuleInfoHash(16);
static moduleinfo_s s_ModuleInfo;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

static bool RetrieveModuleInfo(HMODULE hModule, struct moduleinfo_s *pModInfo)
{
	struct moduleinfo_s *pHashEntry = NULL;

	if (pHashEntry = s_ModuleInfoHash.Find(hModule))
	{
		*pModInfo = *pHashEntry;
		return true;
	}

	MEMORY_BASIC_INFORMATION memInfo;

	IMAGE_DOS_HEADER *dos;
	IMAGE_NT_HEADERS *pe;
	IMAGE_FILE_HEADER *file;
	IMAGE_OPTIONAL_HEADER *opt;

	if (VirtualQuery(hModule, &memInfo, sizeof(MEMORY_BASIC_INFORMATION)))
	{
		if (memInfo.State != MEM_COMMIT || memInfo.Protect == PAGE_NOACCESS)
			return false;

		unsigned int dwAllocationBase = (unsigned int)memInfo.AllocationBase;
		pModInfo->pBaseOfDll = memInfo.AllocationBase;

		dos = reinterpret_cast<IMAGE_DOS_HEADER *>(dwAllocationBase);
		pe = reinterpret_cast<IMAGE_NT_HEADERS *>(dwAllocationBase + dos->e_lfanew);

		file = &pe->FileHeader;
		opt = &pe->OptionalHeader;

		if (dos->e_magic == IMAGE_DOS_SIGNATURE && pe->Signature == IMAGE_NT_SIGNATURE && opt->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC && file->Machine == IMAGE_FILE_MACHINE_I386)
		{
			pModInfo->SizeOfImage = opt->SizeOfImage;

			s_ModuleInfoHash.Insert(hModule, *pModInfo);

			return true;
		}
	}

	return false;
}

void *FindPattern(HMODULE hModule, struct pattern_s *pPattern, unsigned int offset /* = 0 */)
{
	if (RetrieveModuleInfo(hModule, &s_ModuleInfo))
	{
		unsigned long nLength = pPattern->length;
		unsigned char *pSignature = &pPattern->signature;

		unsigned char *pModuleBase = (unsigned char *)s_ModuleInfo.pBaseOfDll + offset;
		unsigned char *pModuleEnd = pModuleBase + s_ModuleInfo.SizeOfImage - nLength;

		while (pModuleBase < pModuleEnd)
		{
			bool bFound = true;

			for (register unsigned long i = 0; i < nLength; i++)
			{
				if (pSignature[i] != pPattern->ignorebyte && pSignature[i] != pModuleBase[i])
				{
					bFound = false;
					break;
				}
			}

			if (bFound)
				return (void *)pModuleBase;

			pModuleBase++;
		}
	}

	return NULL;
}

void *FindString(HMODULE hModule, const char *pszString, unsigned int offset /* = 0 */)
{
	if (RetrieveModuleInfo(hModule, &s_ModuleInfo))
	{
		unsigned long nLength = strlen(pszString);

		unsigned char *pModuleBase = (unsigned char *)s_ModuleInfo.pBaseOfDll + offset;
		unsigned char *pModuleEnd = pModuleBase + s_ModuleInfo.SizeOfImage - nLength;

		while (pModuleBase < pModuleEnd)
		{
			bool bFound = true;

			for (register unsigned long i = 0; i < nLength; i++)
			{
				if (pszString[i] != pModuleBase[i])
				{
					bFound = false;
					break;
				}
			}

			if (bFound)
				return (void *)pModuleBase;

			pModuleBase++;
		}
	}

	return NULL;
}

void *FindAddress(HMODULE hModule, void *pAddress, unsigned int offset /* = 0 */)
{
	if (RetrieveModuleInfo(hModule, &s_ModuleInfo))
	{
		unsigned char *pModuleBase = (unsigned char *)s_ModuleInfo.pBaseOfDll + offset;
		unsigned char *pModuleEnd = pModuleBase + s_ModuleInfo.SizeOfImage - sizeof(void *);

		while (pModuleBase < pModuleEnd)
		{
			if (*(unsigned long *)pModuleBase == (unsigned long)pAddress)
				return (void *)pModuleBase;

			pModuleBase++;
		}
	}

	return NULL;
}