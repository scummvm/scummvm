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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GOB_DRAW_H
#define GOB_DRAW_H

#include "gob/video.h"

namespace Gob {

#define SPRITES_COUNT 50

#define RENDERFLAG_NOINVALIDATE      0x0001
#define RENDERFLAG_CAPTUREPUSH       0x0002
#define RENDERFLAG_COLLISIONS        0x0004
#define RENDERFLAG_CAPTUREPOP        0x0008
#define RENDERFLAG_USEDELTAS         0x0010
#define RENDERFLAG_UNKNOWN           0x0080
#define RENDERFLAG_NOBLITINVALIDATED 0x0200
#define RENDERFLAG_SKIPOPTIONALTEXT  0x0400
#define RENDERFLAG_FROMSPLIT         0x0800
#define RENDERFLAG_DOUBLECOORDS      0x1000

class Draw {
public:
	struct FontToSprite {
		int8 sprite;
		int8 base;
		int8 width;
		int8 height;
		FontToSprite() : sprite(0), base(0), width(0), height(0) {}
	};

	int16 _renderFlags;

	int16 _fontIndex;
	int16 _spriteLeft;
	int16 _spriteTop;
	int16 _spriteRight;
	int16 _spriteBottom;
	int16 _destSpriteX;
	int16 _destSpriteY;
	int16 _backColor;
	int16 _frontColor;
	int16 _transparency;

	int16 _sourceSurface;
	int16 _destSurface;

	char _letterToPrint;
	const char *_textToPrint;

	int16 _backDeltaX;
	int16 _backDeltaY;

	FontToSprite _fontToSprite[4];
	Video::FontDesc *_fonts[8];

	Common::Array<SurfaceDescPtr> _spritesArray;

	int16 _invalidatedCount;
	int16 _invalidatedTops[30];
	int16 _invalidatedLefts[30];
	int16 _invalidatedRights[30];
	int16 _invalidatedBottoms[30];

	bool _noInvalidated;
	// Don't blit invalidated rects when in video mode 5 or 7
	bool _noInvalidated57;
	bool _paletteCleared;
	bool _applyPal;

	SurfaceDescPtr _backSurface;
	SurfaceDescPtr _frontSurface;

	int16 _unusedPalette1[18];
	int16 _unusedPalette2[16];
	Video::Color _vgaPalette[256];
	Video::Color _vgaSmallPalette[16];

	// 0 (00b): No cursor
	// 1 (01b): Cursor would be on _backSurface
	// 2 (10b): Cursor would be on _frontSurface
	// 3 (11b): Cursor would be on _backSurface and _frontSurface
	uint8 _showCursor;
	int16 _cursorIndex;
	int16 _transparentCursor;
	uint32 _cursorTimeKey;

	int16 _cursorX;
	int16 _cursorY;
	int16 _cursorWidth;
	int16 _cursorHeight;

	int16 _cursorHotspotXVar;
	int16 _cursorHotspotYVar;

	SurfaceDescPtr _cursorSprites;
	SurfaceDescPtr _cursorSpritesBack;
	SurfaceDescPtr _scummvmCursor;

	int16 _cursorAnim;
	int8 _cursorAnimLow[40];
	int8 _cursorAnimHigh[40];
	int8 _cursorAnimDelays[40];

	int16 _palLoadData1[4];
	int16 _palLoadData2[4];

	int16 _needAdjust;
	int16 _scrollOffsetY;
	int16 _scrollOffsetX;

	void invalidateRect(int16 left, int16 top, int16 right, int16 bottom);
	void blitInvalidated();
	void setPalette();
	void clearPalette();

	void dirtiedRect(int16 surface, int16 left, int16 top, int16 right, int16 bottom);
	void dirtiedRect(SurfaceDescPtr surface, int16 left, int16 top, int16 right, int16 bottom);

	void initSpriteSurf(int16 index, int16 width, int16 height, int16 flags);
	void freeSprite(int16 index) {
		assert(index < SPRITES_COUNT);
		_spritesArray[index].reset();
	}
	void adjustCoords(char adjust, int16 *coord1, int16 *coord2);
	void adjustCoords(char adjust, uint16 *coord1, uint16 *coord2) {
		adjustCoords(adjust, (int16 *) coord1, (int16 *) coord2);
	}
	int stringLength(const char *str, int16 fontIndex);
	void drawString(const char *str, int16 x, int16 y, int16 color1, int16 color2,
			int16 transp, SurfaceDesc &dest, Video::FontDesc *font);
	void printTextCentered(int16 id, int16 left, int16 top, int16 right,
			int16 bottom, const char *str, int16 fontIndex, int16 color);
	int32 getSpriteRectSize(int16 index);
	void forceBlit(bool backwards = false);

	static const int16 _wobbleTable[360];
	void wobble(SurfaceDesc &surfDesc);

	virtual void initScreen() = 0;
	virtual void closeScreen() = 0;
	virtual void blitCursor() = 0;
	virtual void animateCursor(int16 cursor) = 0;
	virtual void printTotText(int16 id) = 0;
	virtual void spriteOperation(int16 operation) = 0;

	Draw(GobEngine *vm);
	virtual ~Draw();

protected:
	GobEngine *_vm;
};

class Draw_v1 : public Draw {
public:
	virtual void initScreen();
	virtual void closeScreen();
	virtual void blitCursor();
	virtual void animateCursor(int16 cursor);
	virtual void printTotText(int16 id);
	virtual void spriteOperation(int16 operation);

	Draw_v1(GobEngine *vm);
	virtual ~Draw_v1() {}
};

class Draw_v2 : public Draw_v1 {
public:
	virtual void initScreen();
	virtual void closeScreen();
	virtual void blitCursor();
	virtual void animateCursor(int16 cursor);
	virtual void printTotText(int16 id);
	virtual void spriteOperation(int16 operation);

	Draw_v2(GobEngine *vm);
	virtual ~Draw_v2() {}

private:
	uint8 _mayorWorkaroundStatus;
};

class Draw_Bargon: public Draw_v2 {
public:
	virtual void initScreen();

	Draw_Bargon(GobEngine *vm);
	virtual ~Draw_Bargon() {}
};

class Draw_Fascin: public Draw_v2 {
public:
	virtual void initScreen();

	Draw_Fascin(GobEngine *vm);
	virtual ~Draw_Fascin() {}
};

// Draw operations

#define DRAW_BLITSURF	0
#define DRAW_PUTPIXEL	1
#define DRAW_FILLRECT	2
#define DRAW_DRAWLINE	3
#define DRAW_INVALIDATE	4
#define DRAW_LOADSPRITE	5
#define DRAW_PRINTTEXT	6
#define DRAW_DRAWBAR 7
#define DRAW_CLEARRECT	8
#define DRAW_FILLRECTABS 9
#define DRAW_DRAWLETTER	10

} // End of namespace Gob

#endif // GOB_DRAW_H
