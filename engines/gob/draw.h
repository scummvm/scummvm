/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */
#ifndef GOB_DRAW_H
#define GOB_DRAW_H

#include "gob/video.h"

namespace Gob {

#define RENDERFLAG_NOINVALIDATE	1
#define RENDERFLAG_CAPTUREPUSH	2
#define RENDERFLAG_CAPTUREPOP	8
#define RENDERFLAG_USEDELTAS 	0x10

class Draw {
public:
	struct FontToSprite {
		int8 sprite;
		int8 base;
		int8 width;
		int8 height;
		FontToSprite() : sprite(0), base(0), width(0), height() {}
	};

	int16 _fontIndex;
	int16 _spriteLeft;
	int16 _spriteTop;
	int16 _spriteRight;
	int16 _spriteBottom;
	int16 _destSpriteX;
	int16 _destSpriteY;
	int16 _backColor;
	int16 _frontColor;
	char _letterToPrint;
	FontToSprite _fontToSprite[4];
	int16 _destSurface;
	int16 _sourceSurface;
	int16 _renderFlags;
	int16 _backDeltaX;
	int16 _backDeltaY;
	Video::FontDesc *_fonts[4];
	char *_textToPrint;
	int16 _transparency;
	Video::SurfaceDesc *_spritesArray[50];
	Video::SurfaceDesc *_sprites1[50];
	Video::SurfaceDesc *_sprites2[50];
	Video::SurfaceDesc *_sprites3[50];
	uint16 _spritesWidths[50];

	int16 _invalidatedCount;
	int16 _invalidatedTops[30];
	int16 _invalidatedLefts[30];
	int16 _invalidatedRights[30];
	int16 _invalidatedBottoms[30];

	int8 _noInvalidated;
//	int8 doFullFlip; // Never used?!?
	int8 _paletteCleared;

	int16 _cursorIndex;
	int16 _transparentCursor;
	uint32 _cursorTimeKey;

	Video::SurfaceDesc *_backSurface;
	Video::SurfaceDesc *_frontSurface;

	int16 _unusedPalette1[18];
	int16 _unusedPalette2[16];
	Video::Color _vgaPalette[256];
	Video::Color _vgaSmallPalette[16];

	int16 _cursorX;
	int16 _cursorY;
	int16 _cursorWidth;
	int16 _cursorHeight;

	int16 _cursorXDeltaVar;
	int16 _cursorYDeltaVar;

	Video::SurfaceDesc *_cursorSprites;
	Video::SurfaceDesc *_cursorBack;
	int16 _cursorAnim;
	int8 _cursorAnimLow[40];
	int8 _cursorAnimHigh[40];
	int8 _cursorAnimDelays[40];
	int8 _applyPal;

	int16 _palLoadData1[4];
	int16 _palLoadData2[4];
		
	void invalidateRect(int16 left, int16 top, int16 right, int16 bottom);
	void blitInvalidated(void);
	void setPalette(void);
	void clearPalette(void);
	void blitCursor(void);

	void spriteOperation(int16 operation);
	void animateCursor(int16 cursor);
	void printText(void);

	void freeSprite(int16 index);
	void adjustCoords(int16 *coord1, int16 *coord2, char adjust);
	void initBigSprite(int16 index, int16 height, int16 width, int16 flags);

	Draw(GobEngine *vm);

protected:
	GobEngine *_vm;
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

}				// End of namespace Gob

#endif	/* __DRAW_H */
