// Signature Scanner

#pragma once

#include <Windows.h>

#include "patterns_base.h"

void *FindPattern(HMODULE hModule, struct pattern_s *pPattern, unsigned int offset = 0);
void *FindString(HMODULE hModule, const char *pszString, unsigned int offset = 0);
void *FindAddress(HMODULE hModule, void *pAddress, unsigned int offset = 0);