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

#include "engines/tinsel/psx_japan_font.h"

#include "engines/tinsel/tinsel.h"

namespace Tinsel {

// DW1 PSX Japanese font routines.
//
// DW1 PSX Japan uses font differently than all other platforms and versions.
// Instead of storing font glyphs as normal Tinsel images, the font glyphs are
// stored in MULTIBYT.FNT as a single bitmap. The format is native to the PSX.
// The Tinsel font resource defines all of the font glyphs as having no images.
// Instead, the executable sent the bitmap glyphs directly to the PSX GPU.
// The font glyphs were effectively drawn outside of the Tinsel graphics engine.
//
// MULTIBYT.FNT is an uncompressed 4 bpp bitmap with no header. The tricky part
// is supporting this in our engine without the Tinsel image resources that
// normally exist for each font glyph. We work around this limitation by using
// pseudo handles. This allows us to detect attempts to use font glyphs without
// significant changes to the engine. We effectively hook three operations:
//
// 1. Handle::GetFont() populates the character table with pseudo handles.
//    The upper bits contain a non-existent handle (0x1F0) that we use to
//    indicate a font glyph. The lower bits contain the character index.
// 2. Handle::GetImage() calls GetPsxJapanFontCharImage() when it detects a font
//    pseudo handle. This creates an IMAGE structure with the PSX values.
// 3. DrawObject() calls DrawPsxJapanFontChar() when it detects a font pseudo
//    handle. This draws the glyph from MULTIBYT.FNT using the character index
//    encoded in the pseudo handle.

#define PSX_JAPAN_FONT_FILE_NAME "MULTIBYT.FNT"

// Font pseudo handle mask. Encodes a non-existent handle index (0x1F0).
#define PSX_JAPAN_FONT_CHAR_MASK 0xF8000000

// Font glyphs are 18 x 17 pixels
#define FONT_WIDTH  18
#define FONT_HEIGHT 17

// MULTIBYT.FNT is a 256 x 816 pixel bitmap containing a grid of glyphs.
// Each glyph row contains 14 characters. The last 4 pixels are unused.
#define FONT_ROW_WIDTH 256
#define FONT_ROW_HEIGHT FONT_HEIGHT
#define FONT_CHARS_IN_ROW 14

// MULTIBYT.FNT contains three pixel values:
// 0  Transparent
// 1  Black
// 2  White
#define FONT_PIXEL_BLACK 1
#define FONT_PIXEL_WHITE 2
#define FONT_COLOR_BLACK 0
#define FONT_COLOR_WHITE 255

static Common::SeekableReadStream *g_MultiByteFont = nullptr;

/**
 * Open MULTIBYT.FNT during engine initialization.
 * Only call this function for DW1 PSX Japan.
 */
void OpenPsxJapanFont() {
	g_MultiByteFont = SearchMan.createReadStreamForMember(PSX_JAPAN_FONT_FILE_NAME);
	if (g_MultiByteFont == nullptr) {
		error("%s not found", PSX_JAPAN_FONT_FILE_NAME);
	}
}

/**
 * Close MULTIBYT.FNT (if open) during engine destruction.
 */
void ClosePsxJapanFont() {
	delete g_MultiByteFont;
	g_MultiByteFont = nullptr;
}

/**
 * Returns a SCNHANDLE for a DW1 PSX Japan font character.
 */
SCNHANDLE GetPsxJapanFontCharHandle(uint32 charIndex) {
	switch (charIndex) {
	case 10: // newline
	case 32: // space
		return 0;
	default:
		return (PSX_JAPAN_FONT_CHAR_MASK | charIndex);
	}
}

/**
 * Returns true if a SCNHANDLE is for a DW1 PSX Japan font character.
 */
bool IsPsxJapanFontChar(SCNHANDLE offset) {
	return ((offset & PSX_JAPAN_FONT_CHAR_MASK) == PSX_JAPAN_FONT_CHAR_MASK);
}

/**
 * Creates an IMAGE object for a DW1 PSX Japan font character.
 */
const IMAGE *GetPsxJapanFontCharImage(SCNHANDLE offset) {
	IMAGE *img = new IMAGE();
	memset(img, 0, sizeof(IMAGE));

	// All characters have the same width and height.
	img->imgWidth = FONT_WIDTH;
	img->imgHeight = FONT_HEIGHT;

	// Set the bitmap handle to the same pseudo handle as the image so that we
	// can detect it in DrawObject(). The DRAWOBJECT structure that is passed to
	// DrawObject() only contains the bitmap handle, not the image handle.
	img->hImgBits = offset;

	return img;
}

/**
 * Draws a DW1 PSX Japan font character from MULTIBYT.FNT.
 */
void DrawPsxJapanFontChar(DRAWOBJECT *pObj, uint8 *destP) {
	// Extract character index from pseudo handle
	const int charIndex = (pObj->hBits & 0x007fffffL);

	// Calculate character's position in MULTIBYT.FNT
	const int charRow = (charIndex / FONT_CHARS_IN_ROW) * (FONT_ROW_WIDTH / 2 * FONT_ROW_HEIGHT);
	const int charCol = (charIndex % FONT_CHARS_IN_ROW) * (FONT_WIDTH / 2);
	const int charUpperLeft = charRow + charCol;

	// Draw character from top to bottom, left to right
	const int height = pObj->height - pObj->botClip;
	const int width = pObj->width - pObj->rightClip;
	for (int y = pObj->topClip; y < height; y++) {
		// Read character row (9 bytes containing 18 pixels)
		const int charPos = charUpperLeft + (y * (FONT_ROW_WIDTH / 2));
		uint8 charRowBytes[FONT_WIDTH / 2];
		g_MultiByteFont->seek(charPos);
		g_MultiByteFont->read(charRowBytes, sizeof(charRowBytes));

		uint8 *rowDest = destP;
		for (int x = pObj->leftClip; x < width; x++) {
			// Bitmap format is 4 bits per pixel.
			// Lower nibble: left pixel
			// Upper nibble: right pixel
			uint8 b = charRowBytes[x / 2];
			uint8 pixel = (x & 1) ? (b >> 4) : (b & 0x0f);
			if (pixel == FONT_PIXEL_BLACK) {
				*rowDest = FONT_COLOR_BLACK;
			} else if (pixel == FONT_PIXEL_WHITE) {
				*rowDest = FONT_COLOR_WHITE;
			}
			rowDest++;
		}
		destP += SCREEN_WIDTH;
	}
}

} // End of namespace Tinsel
