#pragma once

#include <os_types.h>

struct DMG_Font
{
	uint8_t width16[256];
	uint8_t bitmap16[256 * 32];
	uint8_t width8[256];
	uint8_t bitmap8[256 * 8];
};

bool DMG_IsSINTACFontV4(const char* fileName);
bool DMG_ReadSINTACFont(const char* fileName, DMG_Font* font);
bool DMG_WriteSINTACFont(const char* fileName, const DMG_Font* font);
