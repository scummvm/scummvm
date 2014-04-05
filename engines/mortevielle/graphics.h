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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_GRAPHICS_H
#define MORTEVIELLE_GRAPHICS_H

#include "common/file.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Mortevielle {
class MortevielleEngine;

class PaletteManager {
private:
	void setPalette(const int *palette, uint idx, uint size);

public:
	void setDefaultPalette();
};

#define FONT_WIDTH 8
#define FONT_HEIGHT 6
#define FONT_NUM_CHARS 121

class GfxSurface : public Graphics::Surface {
private:
	int _xp, _yp;
	int _xSize, _ySize;
	int _lookupIndex, _lookupValue;
	bool _nibbleFlag;
	int _thickness;
	int _yInc, _yEnd, _xInc, _xEnd;

	byte nextNibble(const byte *&pSrc);
	byte nextByte(const byte *&pSrc, const byte *&pLookup);

	void majTtxTty();
	int desanalyse(const byte *&pSrc);
	void horizontal(const byte *&pSrc, byte *&pDest, const byte *&pLookup);
	void vertical(const byte *&pSrc, byte *&pDest, const byte *&pLookup);
	void decom11(const byte *&pSrc, byte *&pDest, const byte *&pLookup);
	void diag(const byte *&pSrc, byte *&pDest, const byte *&pLookup);
	void nextDecompPtr(byte *&pDest);
	void negXInc();
	void negYInc();
	bool TFP(int v);
	void TF1(byte *&pDest, int &v);
	void TF2(const byte *&pSrc, byte *&pDest, const byte *&pLookup, int &v);
public:
	// Specifies offset when drawing the image
	Common::Point _offset;
	// Transparency palette index
	int _transparency;

	~GfxSurface();

	void decode(const byte *pSrc);
};

class ScreenSurface: public Graphics::Surface {
private:
	MortevielleEngine *_vm;

	Common::List<Common::Rect> _dirtyRects;
	byte _fontData[FONT_NUM_CHARS * FONT_HEIGHT];

public:
	ScreenSurface(MortevielleEngine *vm);

	Common::Point _textPos;		// Original called xwhere/ywhere
	void readFontData(Common::File &f, int dataSize);
	Graphics::Surface lockArea(const Common::Rect &bounds);
	void updateScreen();
	void drawPicture(GfxSurface &surface, int x, int y);
	void copyFrom(Graphics::Surface &src, int x, int y);
	void writeCharacter(const Common::Point &pt, unsigned char ch, int palIndex);
	void drawBox(int x, int y, int dx, int dy, int col);
	void fillRect(int color, const Common::Rect &bounds);
	void clearScreen();
	void putxy(int x, int y) { _textPos = Common::Point(x, y); }
	void drawString(const Common::String &l, int command);
	int  getStringWidth(const Common::String &s);
	void drawLine(int x, int y, int xx, int yy, int coul);
	void drawRectangle(int x, int y, int dx, int dy);

	// TODO: Refactor code to remove this method, for increased performance
	void setPixel(const Common::Point &pt, int palIndex);
};

} // End of namespace Mortevielle

#endif
