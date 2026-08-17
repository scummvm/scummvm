#include <vid_font.h>

#include <dmg.h>
#include <dmg_font.h>
#include <ddb.h>
#include <ddb_scr.h>   // charset[], charWidth[]
#include <os_file.h>
#include <os_mem.h>

bool SCR_LoadCharset(uint8_t* ptr, const char* filename)
{
	File* file = File_Open(filename, ReadOnly);
	if (file == 0)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
		return false;
	}
	if (File_GetSize(file) != 2176)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		File_Close(file);
		return false;
	}
	if (!File_Seek(file, 128) || File_Read(file, ptr, 2048) != 2048)
	{
		DDB_SetError(DDB_ERROR_READING_FILE);
		File_Close(file);
		return false;
	}
	File_Close(file);
	VID_ActivateCharset();
	return true;
}

bool SCR_LoadSINTACFont(const char* filename)
{
	// Decode the font into the shared DMG temporary buffer: DMG_Font is ~10.5K,
	// too big for the tight stack budget, and it is only needed briefly to copy
	// the glyphs into the global charset.
	if (!DMG_ReserveTemporaryBuffer(sizeof(DMG_Font)))
		return false;
	DMG_Font* font = (DMG_Font*)DMG_GetTemporaryBufferBase();
	if (font == 0 || !DMG_ReadSINTACFont(filename, font))
		return false;

#if HAS_HIRES_FONT
	if (VID_StoreFont2X(font, filename))
	{
		VID_ActivateCharset();
		return true;
	}
#endif

	MemCopy(charset, font->bitmap8, sizeof(font->bitmap8));
	MemCopy(charWidth, font->width8, sizeof(font->width8));
	VID_ActivateCharset();
	return true;
}
