#pragma once

#include <os_types.h>

static inline bool IsAlphaNumeric(uint8_t ch)
{
	return (ch >= 'A' && ch <= 'Z') || 
	       (ch >= 'a' && ch <= 'z') || 
	       (ch >= '0' && ch <= '9') || 
		   (ch >= 21 && ch <= 31);			// Spanish DAAD letters
}

static inline bool IsSpace(uint8_t ch)
{
	return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

static inline bool IsDelimiter(uint8_t ch)
{
	return ch == '.' || ch == ';' || ch == ':' || ch == '"' || ch == '\'' || ch == ',';
}

static inline uint8_t ToUpper(uint8_t ch)
{
	static char UpperSpanish[17] = "\x10\x11\x12\x13\x14" "AEIOUNNCCUU";

	if (ch >= 'a' && ch <= 'z')
		return ch - 32;
	if (ch >= 16 && ch <= 31)
		return UpperSpanish[ch-16];

	return ch;
}

static inline uint8_t ToLower(uint8_t ch)
{
	if (ch >= 'A' && ch <= 'Z')
		return ch + 32;
	if (ch == 0x1B || ch == 0x1D || ch == 0x1F)
		return ch - 1;
	return ch;
}

extern uint8_t DDB_Char2ISO[128];
extern uint8_t DDB_ISO2Char[256];
extern const char* DDB_CharToUTF8(uint8_t ch);
