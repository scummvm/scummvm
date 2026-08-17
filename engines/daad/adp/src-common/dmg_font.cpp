#include <dmg_font.h>
#include <os_file.h>
#include <os_lib.h>
#include <os_mem.h>

static const uint8_t SintacFontHeaderV3[16] = {
	'J', 'S', 'J', ' ', 'S', 'I', 'N', 'T', 'A', 'C', ' ', 'F', 'N', 'T', '3', 0
};

static const uint8_t SintacFontHeaderV4[16] = {
	'J', 'S', 'J', ' ', 'S', 'I', 'N', 'T', 'A', 'C', ' ', 'F', 'N', 'T', '4', 0
};

static const uint32_t SintacFontV3Size = 6672;
static const uint32_t SintacFontV4Size = 10768;

static bool ReadExact(File* file, void* buffer, uint64_t size)
{
	return File_Read(file, buffer, size) == size;
}

bool DMG_IsSINTACFontV4(const char* fileName)
{
	File* file = File_Open(fileName);
	if (file == 0)
		return false;

	uint64_t fileSize = File_GetSize(file);
	uint8_t header[16];
	bool ok = fileSize == SintacFontV4Size &&
		ReadExact(file, header, sizeof(header)) &&
		MemComp(header, (void*)SintacFontHeaderV4, sizeof(header)) == 0;

	File_Close(file);
	return ok;
}

static bool WriteExact(File* file, const void* buffer, uint64_t size)
{
	return File_Write(file, buffer, size) == size;
}

static uint16_t Pack8To16Bits(uint8_t bits)
{
	return (uint16_t)bits << 8;
}

static void PromoteVersion3ToVersion4(DMG_Font* font, const uint8_t* width8x16, const uint8_t* bitmap8x16)
{
	for (int glyph = 0; glyph < 256; glyph++)
	{
		uint8_t width = width8x16[glyph];
		if (width > 15)
			width = 15;
		font->width16[glyph] = width;

		const uint8_t* src = bitmap8x16 + glyph * 16;
		uint8_t* dst = font->bitmap16 + glyph * 32;
		for (int row = 0; row < 16; row++)
		{
			uint16_t packed = Pack8To16Bits(src[row]);
			dst[row * 2 + 0] = (uint8_t)(packed >> 8);
			dst[row * 2 + 1] = (uint8_t)(packed & 0xFF);
		}
	}
}

bool DMG_ReadSINTACFont(const char* fileName, DMG_Font* font)
{
	if (font == 0)
		return false;

	File* file = File_Open(fileName);
	if (file == 0)
		return false;

	uint64_t fileSize = File_GetSize(file);
	if (fileSize != SintacFontV3Size && fileSize != SintacFontV4Size)
	{
		File_Close(file);
		return false;
	}

	uint8_t header[16];
	if (!ReadExact(file, header, sizeof(header)))
	{
		File_Close(file);
		return false;
	}

	bool ok = false;
	if (MemComp(header, (void*)SintacFontHeaderV4, sizeof(header)) == 0 && fileSize == SintacFontV4Size)
	{
		ok =
			ReadExact(file, font->width16, sizeof(font->width16)) &&
			ReadExact(file, font->bitmap16, sizeof(font->bitmap16)) &&
			ReadExact(file, font->width8, sizeof(font->width8)) &&
			ReadExact(file, font->bitmap8, sizeof(font->bitmap8));
	}
	else if (MemComp(header, (void*)SintacFontHeaderV3, sizeof(header)) == 0 && fileSize == SintacFontV3Size)
	{
		uint8_t* legacyBuffer = Allocate<uint8_t>("SINTAC v3 font", 256 + 256 * 16, false);
		if (legacyBuffer == 0)
		{
			File_Close(file);
			return false;
		}
		uint8_t* width8x16 = legacyBuffer;
		uint8_t* bitmap8x16 = legacyBuffer + 256;
		ok =
			ReadExact(file, width8x16, 256) &&
			ReadExact(file, bitmap8x16, 256 * 16) &&
			ReadExact(file, font->width8, sizeof(font->width8)) &&
			ReadExact(file, font->bitmap8, sizeof(font->bitmap8));
		if (ok)
			PromoteVersion3ToVersion4(font, width8x16, bitmap8x16);
		Free(legacyBuffer);
	}

	File_Close(file);
	return ok;
}

bool DMG_WriteSINTACFont(const char* fileName, const DMG_Font* font)
{
	if (font == 0)
		return false;

	File* file = File_Create(fileName);
	if (file == 0)
		return false;

	bool ok =
		WriteExact(file, SintacFontHeaderV4, sizeof(SintacFontHeaderV4)) &&
		WriteExact(file, font->width16, sizeof(font->width16)) &&
		WriteExact(file, font->bitmap16, sizeof(font->bitmap16)) &&
		WriteExact(file, font->width8, sizeof(font->width8)) &&
		WriteExact(file, font->bitmap8, sizeof(font->bitmap8));

	File_Close(file);
	return ok;
}
