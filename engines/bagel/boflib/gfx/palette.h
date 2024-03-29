
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_BOFLIB_GFX_PALETTE_H
#define BAGEL_BOFLIB_GFX_PALETTE_H

#include "graphics/screen.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

#define PAL_DEFAULT 0x0000
#define PAL_ANIMATED 0x0001
#define PAL_EXPLICIT 0x0002

typedef ULONG RGBCOLOR;
struct HPALETTE {
	byte _data[PALETTE_SIZE];
	SHORT _numColors;
	HPALETTE(SHORT numColors = PALETTE_COUNT);
};

struct PALETTEENTRY {
	byte peRed;
	byte peGreen;
	byte peBlue;
	byte peFlags;
};
struct LOGPALETTE {
	SHORT palNumEntries;
	SHORT palVersion;
	PALETTEENTRY palPalEntry[1];
};

extern HPALETTE CreatePalette(const LOGPALETTE *pal);

#define RGB(r, g, b) ((RGBCOLOR)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))

typedef struct bofRGBQUAD {
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} BOFRGBQUAD;

#define GetRed(rgb) ((BYTE)((rgb)&0x000000FF))
#define GetGreen(rgb) ((BYTE)(((rgb) >> 8) & 0x000000FF))
#define GetBlue(rgb) ((BYTE)(((rgb) >> 16) & 0x000000FF))

class CBofPalette : public CBofError, public CBofObject {
protected:
	HPALETTE _palette;

	static CBofPalette *m_pSharedPalette;
	static CHAR m_szSharedPalFile[MAX_FNAME];

	/**
	 * Deletes internal palette info
	 */
	VOID ReleasePalette();

public:
	/**
	 * Default constructor
	 */
	CBofPalette();

	/**
	 * Constructor that loads a palette from a file
	 * @param pszFileName       filename
	 */
	CBofPalette(const CHAR *pszFileName);

	/**
	 * Constructor that takes in an existing palette
	 */
	CBofPalette(const HPALETTE &hPalette);

	/**
	 * Loads palette from specified bitmap-file
	 * @param pszFileName   Name of file to load palette from
	 * @param nFlags        Flags for animation, etc...
	 * @return              Error return Code
	 */
	ERROR_CODE LoadPalette(const CHAR *pszFileName, USHORT nFlags = PAL_DEFAULT);

	ERROR_CODE CreateDefault(USHORT nFlags = PAL_DEFAULT);

	UBYTE GetNearestIndex(RGBCOLOR cColor);

	RGBCOLOR GetColor(UBYTE nIndex);
	VOID AnimateEntry(UBYTE nIndex, RGBCOLOR cColor);

	VOID AnimateToPalette(CBofPalette *pSrcPal);

	/**
	 * Assignes specified palette to this CBofPalette
	 * @param hPal      Handle to windows palette
	 */
	VOID SetPalette(const HPALETTE &hPalette);

	const HPALETTE &GetPalette() const {
		return _palette;
	}

	const byte *GetData() const {
		return _palette._data;
	}

	virtual ~CBofPalette();

	/**
	 * Create a new palette based on current palette
	 */
	CBofPalette *CopyPalette();

	/**
	 * initialize static class fields
	 */
	static void initialize();

	/**
	 * Called only in response to "SHAREDPAL=filename" in a script file
	 * @param pszFileName       Palette filename
	 */
	static ERROR_CODE SetSharedPalette(const CHAR *pszFileName);

	/**
	 * Returns the current shared palette
	 * @return      Pointer to shared palette
	 */
	static CBofPalette *GetSharedPalette();
};

} // namespace Bagel

#endif
