/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
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
	typedef struct FontToSprite {
		int8 sprite;
		int8 base;
		int8 width;
		int8 height;
		FontToSprite() : sprite(0), base(0), width(0), height() {}
	} FontToSprite;

	int16 fontIndex;
	int16 spriteLeft;
	int16 spriteTop;
	int16 spriteRight;
	int16 spriteBottom;
	int16 destSpriteX;
	int16 destSpriteY;
	int16 backColor;
	int16 frontColor;
	char letterToPrint;
	FontToSprite fontToSprite[4];
	int16 destSurface;
	int16 sourceSurface;
	int16 renderFlags;
	int16 backDeltaX;
	int16 backDeltaY;
	Video::FontDesc *fonts[4];
	char *textToPrint;
	int16 transparency;
	Video::SurfaceDesc *spritesArray[50];

	int16 invalidatedCount;
	int16 invalidatedTops[30];
	int16 invalidatedLefts[30];
	int16 invalidatedRights[30];
	int16 invalidatedBottoms[30];

	int8 noInvalidated;
//	int8 doFullFlip; // Never used?!?
	int8 paletteCleared;

	int16 gcursorIndex;
	int16 transparentCursor;
	uint32 cursorTimeKey;

	Video::SurfaceDesc *backSurface;
	Video::SurfaceDesc *frontSurface;

	int16 unusedPalette1[18];
	int16 unusedPalette2[16];
	Video::Color vgaPalette[256];
	Video::Color vgaSmallPalette[16];

	int16 cursorX;
	int16 cursorY;
	int16 cursorWidth;
	int16 cursorHeight;

	int16 cursorXDeltaVar;
	int16 cursorYDeltaVar;

	Video::SurfaceDesc *cursorSprites;
	Video::SurfaceDesc *cursorBack;
	int16 cursorAnim;
	int8 cursorAnimLow[40];
	int8 cursorAnimHigh[40];
	int8 cursorAnimDelays[40];
	int8 applyPal;

	int16 palLoadData1[4];
	int16 palLoadData2[4];
		
	void invalidateRect(int16 left, int16 top, int16 right, int16 bottom);
	void blitInvalidated(void);
	void setPalette(void);
	void clearPalette(void);
	void blitCursor(void);

	void spriteOperation(int16 operation);
	void animateCursor(int16 cursor);
	void interPalLoad(void);
	void printText(void);

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
