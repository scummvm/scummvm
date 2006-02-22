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

void gfxDrawSprite(u8 *src4, u16 sw, u16 sh, u8 *dst4, s16 sx, s16 sy);

extern unsigned char *page0;
extern unsigned char *page1;
extern unsigned char *page2;
extern unsigned char *page3;

void init_video();

void convertGfx(u8 *source, u8 *dest, const u16 width, const u16 height);
void convertGfx2(u8 *source, u8 *dest, const u16 width, const u16 height);
void gfxCopyPage(u8 *source, u8 *dest);

void transformPaletteRange(u8 startColor, u8 numColor, s8 r, s8 g, s8 b);
void gfxFlipPage(void);

void gfxSpriteFunc1(u8 *ptr, u16 width, u16 height, u8 *page, s16 x, s16 y);
void gfxFillSprite(u8 *src4, u16 sw, u16 sh, u8 *dst4, s16 sx, s16 sy);

void gfxSpriteFunc2(u8 *spritePtr, s16 width, s16 height, u8 *maskPtr,
    s16 maskWidth, s16 maskHeight, u8 *bufferPtr, s16 x, s16 y, u8 maskIdx);

void gfxDrawLine(s16 x1, s16 y1, s16 x2, s16 y2, u8 color, u8 *page);
void gfxDrawPlainBox(s16 x1, s16 y1, s16 x2, s16 y2, u8 color);

void gfxResetPage(u8 *pagePtr);

s16 gfxGetBit(s16 x, s16 y, u8 *ptr, s16 width);

extern u8 page1Raw[320 * 200];
extern u8 page2Raw[320 * 200];
extern u8 page3Raw[320 * 200];

void gfxResetRawPage(u8 *pageRaw);
void gfxConvertSpriteToRaw(u8 *dest, u8 *source, u16 width, u16 height);
void gfxCopyRawPage(u8 *source, u8 * dest);
void gfxFlipRawPage(u8 *frontBuffer);
void drawSpriteRaw(u8 *spritePtr, u8 *maskPtr, s16 width, s16 height,
    u8 *page, s16 x, s16 y);
void gfxDrawPlainBoxRaw(s16 x1, s16 y1, s16 x2, s16 y2, u8 color, u8 *page);
void drawSpriteRaw2(u8 *spritePtr, u8 transColor, s16 width, s16 height,
    u8 *page, s16 x, s16 y);

#endif
