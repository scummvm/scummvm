/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MORTEVIELLE_GRAPHICS_H
#define MORTEVIELLE_GRAPHICS_H

#include "common/file.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Mortevielle {

class PaletteManager {
public:
	void setPalette(const int *palette, uint idx, uint size);
	void setDefaultPalette();
};

#define FONT_WIDTH 8
#define FONT_HEIGHT 6
#define FONT_NUM_CHARS 121

class GfxSurface: public Graphics::Surface {
private:
	int _xp, _yp;
	int _xSize, _ySize, _var12;
	int _var14, _lookupIndex, _lookupValue;
	bool _nibbleFlag;
	int _thickness;
	int _yInc, _yEnd, _xInc, _xEnd;
	int _width, _height;

	void majTtxTty();
	byte suiv(const byte *&pSrc);
	byte csuiv(const byte *&pSrc, const byte *&pLookup);
	int desanalyse(const byte *&pSrc);
	void horizontal(const byte *&pSrc, byte *&pDest, const byte *&pLookup);
	void vertical(const byte *&pSrc, byte *&pDest, const byte *&pLookup);
	void decom11(const byte *&pSrc, byte *&pDest, const byte *&pLookup);
	void diag(const byte *&pSrc, byte *&pDest, const byte *&pLookup);
	void increments(byte *&pDest);
	void NIH();
	void NIV();
	bool TFP(int v);
	void TF1(byte *&pDest, int &v);
	void TF2(const byte *&pSrc, byte *&pDest, const byte *&pLookup, int &v);
public:
	// Specifies offset when drawing the image
	Common::Point _offset;
public:
	~GfxSurface();

	void decode(const byte *pSrc);
};

class ScreenSurface: public Graphics::Surface {
private:
	Common::List<Common::Rect> _dirtyRects;
	byte _fontData[FONT_NUM_CHARS * FONT_HEIGHT];
public:
	void readFontData(Common::File &f, int dataSize);
	Graphics::Surface lockArea(const Common::Rect &bounds);
	void updateScreen();
	void drawPicture(GfxSurface &surface, int x, int y);
	void writeCharacter(const Common::Point &pt, unsigned char ch, int palIndex);
	void drawBox(int x, int y, int dx, int dy, int col);
	void fillBox(int colour, const Common::Rect &bounds, int patt);
	void clearScreen();

	// TODO: Refactor code to remove this method, for increased performance
	void setPixel(const Common::Point &pt, int palIndex);
};

} // End of namespace Mortevielle

#endif
