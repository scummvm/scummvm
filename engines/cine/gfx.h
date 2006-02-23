/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#ifndef CINE_GFX_H_
#define CINE_GFX_H_

void gfxDrawSprite(uint8 *src4, uint16 sw, uint16 sh, uint8 *dst4, int16 sx, int16 sy);

extern unsigned char *page0;
extern unsigned char *page1;
extern unsigned char *page2;
extern unsigned char *page3;

void init_video();

void convertGfx(uint8 *source, uint8 *dest, const uint16 width, const uint16 height);
void convertGfx2(uint8 *source, uint8 *dest, const uint16 width, const uint16 height);
void gfxCopyPage(uint8 *source, uint8 *dest);

void transformPaletteRange(uint8 startColor, uint8 numColor, int8 r, int8 g, int8 b);
void gfxFlipPage(void);

void gfxSpriteFunc1(uint8 *ptr, uint16 width, uint16 height, uint8 *page, int16 x, int16 y);
void gfxFillSprite(uint8 *src4, uint16 sw, uint16 sh, uint8 *dst4, int16 sx, int16 sy);

void gfxSpriteFunc2(uint8 *spritePtr, int16 width, int16 height, uint8 *maskPtr,
    int16 maskWidth, int16 maskHeight, uint8 *bufferPtr, int16 x, int16 y, uint8 maskIdx);

void gfxDrawLine(int16 x1, int16 y1, int16 x2, int16 y2, uint8 color, uint8 *page);
void gfxDrawPlainBox(int16 x1, int16 y1, int16 x2, int16 y2, uint8 color);

void gfxResetPage(uint8 *pagePtr);

int16 gfxGetBit(int16 x, int16 y, uint8 *ptr, int16 width);

extern uint8 page1Raw[320 * 200];
extern uint8 page2Raw[320 * 200];
extern uint8 page3Raw[320 * 200];

void gfxResetRawPage(uint8 *pageRaw);
void gfxConvertSpriteToRaw(uint8 *dest, uint8 *source, uint16 width, uint16 height);
void gfxCopyRawPage(uint8 *source, uint8 * dest);
void gfxFlipRawPage(uint8 *frontBuffer);
void drawSpriteRaw(uint8 *spritePtr, uint8 *maskPtr, int16 width, int16 height, uint8 *page, int16 x, int16 y);
void gfxDrawPlainBoxRaw(int16 x1, int16 y1, int16 x2, int16 y2, uint8 color, uint8 *page);
void drawSpriteRaw2(uint8 *spritePtr, uint8 transColor, int16 width, int16 height, uint8 *page, int16 x, int16 y);

#endif
