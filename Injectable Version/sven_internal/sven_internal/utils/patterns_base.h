#pragma once

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define DEFINE_PATTERN(name, signature) struct tpattern_s<get_pattern_length(signature), (unsigned char)0x2A> __pattern_##name(signature); \
	struct pattern_s *name = reinterpret_cast<struct pattern_s *>(&(__pattern_##name))

#define DEFINE_PATTERN_IGNORE_BYTE(name, signature, ignorebyte) struct tpattern_s<get_pattern_length(signature), (unsigned char)ignorebyte> __pattern_##name(signature); \
	struct pattern_s *name = reinterpret_cast<struct pattern_s *>(&(__pattern_##name))

#define EXTERN_PATTERN(name) extern struct pattern_s *name

#define REPLACE_PATTERN(dest, src) dest = src

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static constexpr int hex_to_decimal_table[] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static inline int hex_to_decimal_fast(char *hex)
{
	int result = 0;

	while (*hex && result >= 0)
	{
		result = (result << 4) | hex_to_decimal_table[*hex++];
	}

	return result;
}

static inline constexpr unsigned int get_pattern_length(const char *pszPattern)
{
	unsigned int nCount = 0;

	while (*pszPattern)
	{
		char symbol = *pszPattern;

		if (symbol != ' ')
		{
			++nCount;

			if (symbol != '?')
				++pszPattern;
		}

		++pszPattern;
	}

	return nCount;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<int bytesCount, unsigned char ignoreByte>
struct tpattern_s
{
	constexpr tpattern_s(const char *pszPattern) : signature(), length(bytesCount), ignorebyte(ignoreByte)
	{
		int nLength = 0;

		while (*pszPattern)
		{
			char symbol = *pszPattern;

			if (symbol != ' ')
			{
				if (symbol != '?')
				{
					char byte[3] = { 0 };

					byte[0] = pszPattern[0];
					byte[1] = pszPattern[1];

					//signature[nLength] = static_cast<unsigned char>(strtoul(byte, NULL, 16));
					signature[nLength] = static_cast<unsigned char>(hex_to_decimal_fast(byte));

					++pszPattern;
				}
				else
				{
					signature[nLength] = ignorebyte;
				}

				++nLength;
			}

			++pszPattern;
		}
	}

	int length;
	unsigned char ignorebyte;
	unsigned char signature[bytesCount];
};

struct pattern_s
{
	unsigned int length;
	unsigned char ignorebyte;
	unsigned char signature;
};