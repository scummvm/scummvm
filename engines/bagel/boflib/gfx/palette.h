
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
#include "bagel/boflib/error.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

#define PAL_DEFAULT 0x0000
#define PAL_ANIMATED 0x0001
#define PAL_EXPLICIT 0x0002

typedef uint32 RGBCOLOR;
struct HPALETTE {
	byte _data[PALETTE_SIZE];
	int16 _numColors;
	HPALETTE(int16 numColors = PALETTE_COUNT);
};

struct PALETTEENTRY {
	byte peRed;
	byte peGreen;
	byte peBlue;
	byte peFlags;
};
struct LOGPALETTE {
	int16 palNumEntries;
	int16 palVersion;
	PALETTEENTRY palPalEntry[1];
};

#define RGB(r, g, b) ((RGBCOLOR)(((byte)(r) | ((uint16)((byte)(g)) << 8)) | (((uint32)(byte)(b)) << 16)))

struct BOFRGBQUAD {
	byte rgbBlue;
	byte rgbGreen;
	byte rgbRed;
	byte rgbReserved;
};

#define GetRed(rgb) ((byte)((rgb) & 0x000000FF))
#define GetGreen(rgb) ((byte)(((rgb) >> 8) & 0x000000FF))
#define GetBlue(rgb) ((byte)(((rgb) >> 16) & 0x000000FF))

class CBofPalette : public CBofError, public CBofObject {
protected:
	HPALETTE _palette;

	static CBofPalette *_pSharedPalette;
	static char _szSharedPalFile[MAX_FNAME];

	/**
	 * Deletes internal palette info
	 */
	void ReleasePalette();

public:
	/**
	 * Default constructor
	 */
	CBofPalette();

	/**
	 * Constructor that loads a palette from a file
	 * @param pszFileName       filename
	 */
	CBofPalette(const char *pszFileName);

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
	ErrorCode loadPalette(const char *pszFileName, uint16 nFlags = PAL_DEFAULT);

	ErrorCode createDefault(uint16 nFlags = PAL_DEFAULT);

	byte getNearestIndex(RGBCOLOR cColor);

	RGBCOLOR getColor(byte nIndex);

	/**
	 * Assigns specified palette to this CBofPalette
	 * @param hPalette      Handle to windows palette
	 */
	void setPalette(const HPALETTE &hPalette);

	const HPALETTE &getPalette() const {
		return _palette;
	}

	const byte *getData() const {
		return _palette._data;
	}

	void setData(const byte* colors) {
		memcpy(_palette._data, colors, PALETTE_SIZE);
	}

	virtual ~CBofPalette();

	/**
	 * Create a new palette based on current palette
	 */
	CBofPalette *copyPalette();

	/**
	 * initialize static class fields
	 */
	static void initialize();

	/**
	 * Called only in response to "SHAREDPAL=filename" in a script file
	 * @param pszFileName       Palette filename
	 */
	static ErrorCode setSharedPalette(const char *pszFileName);

	/**
	 * Returns the current shared palette
	 * @return      Pointer to shared palette
	 */
	static CBofPalette *getSharedPalette();
};

} // namespace Bagel

#endif
