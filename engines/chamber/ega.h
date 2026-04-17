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

#ifndef CHAMBER_EGA_H
#define CHAMBER_EGA_H

#include "graphics/surface.h"

namespace Chamber {

// EGA screen dimensions (same as CGA)
#define EGA_WIDTH  320
#define EGA_HEIGHT 200

// EGA uses 1 byte per pixel (CLUT8 linear, no interlacing)
#define EGA_BYTES_PER_LINE EGA_WIDTH
#define EGA_SCREEN_SIZE    (EGA_WIDTH * EGA_HEIGHT)

// Planar background: 4 planes × 8000 bytes
#define EGA_PLANAR_SIZE    (EGA_WIDTH / 8 * EGA_HEIGHT)   // 8000 bytes per plane

extern byte *ega_screen;     // aliases CGA_SCREENBUFFER (frontbuffer)
extern byte *ega_backbuffer; // aliases backbuffer

// Standard EGA 16-color palette (RGB, used for screen and cursor)
extern const byte EGA_PALETTE[16 * 3];

// CGA palette 1 high-intensity → EGA color index mapping
extern const byte cga_to_ega_color[4];

// --- screen management ---
void ega_switchToGraphicsMode();
void ega_blitToScreen(int16 x, int16 y, int16 w, int16 h);
void ega_BackBufferToRealFull();
void ega_RealBufferToBackFull();
void ega_SwapRealBackBuffer();

// --- offset helpers ---
uint16 ega_CalcXY(uint16 x, uint16 y);
uint16 ega_CalcXY_p(uint16 x, uint16 y);

// --- memory / blit ---
void ega_CopyScreenBlock(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs);
void ega_SwapScreenRect(byte *pixels, uint16 w, uint16 h, byte *screen, uint16 ofs);

byte *ega_BackupImage(byte *source, uint16 ofs, uint16 w, uint16 h, byte *buffer);
void  ega_RestoreImage(byte *buffer, byte *target);
void  ega_RefreshImageData(byte *buffer);
void  ega_RestoreBackupImage(byte *target);

void ega_Blit(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);
void ega_BlitAndWait(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);
void ega_Fill(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs);
void ega_FillAndWait(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs);

void ega_BlitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs);

// --- sprite blitters ---
// EGA sprites are decoded to CLUT8 (1 byte/pixel); pixel 0 = transparent
void ega_BlitSprite(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);
void ega_BlitSpriteFlip(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);
void ega_BlitSpriteBak(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs, byte *backup, byte mask);

void ega_BlitScratchBackSprite(uint16 sprofs, uint16 w, uint16 h, byte *screen, uint16 ofs);

// --- draw primitives ---
void ega_DrawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target);
void ega_DrawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target);
uint16 ega_DrawHLineWithEnds(uint16 bmask, uint16 bpix, byte color, uint16 l, byte *target, uint16 ofs);

void ega_PrintChar(byte c, byte *target);

// --- resource loader ---
Graphics::Surface *ega_loadFond(const char *filename);

// --- background restore ---
// Saved clean copy of decoded FOND.EGA for room background restoration
extern byte ega_fond_clean[EGA_SCREEN_SIZE];
void ega_drawBackground(byte *target);

} // End of namespace Chamber

#endif // CHAMBER_EGA_H
