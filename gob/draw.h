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

typedef struct Draw_FontToSprite {
	int8 sprite;
	int8 base;
	int8 width;
	int8 height;
} Draw_FontToSprite;

extern int16 draw_fontIndex;
extern int16 draw_spriteLeft;
extern int16 draw_spriteTop;
extern int16 draw_spriteRight;
extern int16 draw_spriteBottom;
extern int16 draw_destSpriteX;
extern int16 draw_destSpriteY;
extern int16 draw_backColor;
extern int16 draw_frontColor;
extern char draw_letterToPrint;
extern Draw_FontToSprite draw_fontToSprite[4];
extern int16 draw_destSurface;
extern int16 draw_sourceSurface;
extern int16 draw_renderFlags;
extern int16 draw_backDeltaX;
extern int16 draw_backDeltaY;
extern FontDesc *draw_fonts[4];
extern char *draw_textToPrint;
extern int16 draw_transparency;
extern SurfaceDesc *draw_spritesArray[50];

extern int16 draw_invalidatedCount;
extern int16 draw_invalidatedTops[30];
extern int16 draw_invalidatedLefts[30];
extern int16 draw_invalidatedRights[30];
extern int16 draw_invalidatedBottoms[30];

extern int8 draw_noInvalidated;
extern int8 draw_doFullFlip;
extern int8 draw_paletteCleared;

extern int16 draw_cursorIndex;
extern int16 draw_transparentCursor;

extern SurfaceDesc *draw_backSurface;
extern SurfaceDesc *draw_frontSurface;

extern int16 draw_unusedPalette1[18];
extern int16 draw_unusedPalette2[16];
extern Color draw_vgaPalette[256];
extern Color draw_vgaSmallPalette[16];

extern int16 draw_cursorX;
extern int16 draw_cursorY;
extern int16 draw_cursorWidth;
extern int16 draw_cursorHeight;

extern int16 draw_cursorXDeltaVar;
extern int16 draw_cursorYDeltaVar;

extern SurfaceDesc *draw_cursorSprites;
extern SurfaceDesc *draw_cursorBack;
extern int16 draw_cursorAnim;
extern int8 draw_cursorAnimLow[40];
extern int8 draw_cursorAnimHigh[40];
extern int8 draw_cursorAnimDelays[40];
extern int8 draw_applyPal;

void draw_invalidateRect(int16 left, int16 top, int16 right, int16 bottom);
void draw_blitInvalidated(void);
void draw_setPalette(void);
void draw_clearPalette(void);
void draw_blitCursor(void);

void draw_spriteOperation(int16 operation);
void draw_animateCursor(int16 cursor);
void draw_interPalLoad(void);
void draw_printText(void);
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
