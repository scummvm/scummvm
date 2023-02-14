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

#ifndef CHAMBER_cga_H
#define CHAMBER_cga_H

namespace Chamber {

#define CGA_WIDTH 320
#define CGA_HEIGHT 200
#define CGA_BASE_SEG 0xB800
#define CGA_ODD_LINES_OFS 0x2000
#define CGA_BITS_PER_PIXEL 2
#define CGA_PIXELS_PER_BYTE (8 / CGA_BITS_PER_PIXEL)
#define CGA_BYTES_PER_LINE (CGA_WIDTH / CGA_PIXELS_PER_BYTE)

extern byte CGA_SCREENBUFFER[0x4000];

#define CGA_FONT_HEIGHT 6

#define CGA_NEXT_LINE(offs) ((CGA_ODD_LINES_OFS ^ (offs)) + (((offs) & CGA_ODD_LINES_OFS) ? 0 : CGA_BYTES_PER_LINE))
#define CGA_PREV_LINE(offs) ((CGA_ODD_LINES_OFS ^ (offs)) - (((offs) & CGA_ODD_LINES_OFS) ? CGA_BYTES_PER_LINE : 0))

#define frontbuffer CGA_SCREENBUFFER
extern byte backbuffer[0x4000];

extern byte sprit_load_buffer[1290];

extern byte cga_pixel_flip[256];

extern byte char_draw_coords_x;
extern byte char_draw_coords_y;
extern byte *char_xlat_table;
extern byte string_ended;
extern byte char_draw_max_width;
extern byte char_draw_max_height;

void switchToGraphicsMode(void);
void switchToTextMode(void);

void waitVBlank(void);
void cga_blitToScreen(int16 dx, int16 dy, int16 w, int16 h);
void cga_blitToScreen(int16 ofs, int16 w, int16 h);

void cga_ColorSelect(byte csel);
void cga_BackBufferToRealFull(void);
void cga_RealBufferToBackFull(void);
void cga_SwapRealBackBuffer(void);

void cga_SwapScreenRect(byte *pixels, uint16 w, uint16 h, byte *screen, uint16 ofs);

uint16 cga_CalcXY(uint16 x, uint16 y);
uint16 cga_CalcXY_p(uint16 x, uint16 y);

void cga_CopyScreenBlock(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs);

byte *cga_BackupImage(byte *source, uint16 ofs, uint16 w, uint16 h, byte *buffer);
byte *cga_BackupImageReal(uint16 ofs, uint16 w, uint16 h);

void cga_RestoreImage(byte *buffer, byte *target);
void cga_RefreshImageData(byte *buffer);
void cga_RestoreBackupImage(byte *target);

void cga_Blit(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);
void cga_BlitAndWait(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);
void cga_FillAndWait(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs);

void cga_DrawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target);
void cga_DrawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target);
uint16 cga_DrawHLineWithEnds(uint16 bmask, uint16 bpix, byte color, uint16 l, byte *target, uint16 ofs);

void cga_PrintChar(byte c, byte *target);

void cga_BlitScratchBackSprite(uint16 sprofs, uint16 w, uint16 h, byte *screen, uint16 ofs);
void cga_BlitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs);

void cga_BlitSprite(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);
void cga_BlitSpriteFlip(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);

void cga_BlitSpriteBak(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs, byte *backup, byte mask);

void drawSprite(byte *sprite, byte *screen, uint16 ofs);
void drawSpriteFlip(byte *sprite, byte *screen, uint16 ofs);

void drawSpriteN(byte index, uint16 x, uint16 y, byte *target);
void drawSpriteNFlip(byte index, uint16 x, uint16 y, byte *target);

void backupAndShowSprite(byte index, byte x, byte y);

byte *loadSprite(byte index, byte *bank, byte *buffer, byte header_only);

byte *loadSprit(byte index);
byte *loadPersSprit(byte index);

void cga_AnimLiftToUp(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 x, uint16 y);
void cga_AnimLiftToDown(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);
void cga_AnimLiftToLeft(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);
void cga_AnimLiftToRight(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs);

void cga_HideScreenBlockLiftToUp(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs);
void cga_HideScreenBlockLiftToDown(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs);
void cga_HideScreenBlockLiftToLeft(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs);
void cga_HideScreenBlockLiftToRight(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs);

void cga_HideShatterFall(byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs);

void cga_TraceLine(uint16 sx, uint16 ex, uint16 sy, uint16 ey, byte *source, byte *target);

void cga_ZoomImage(byte *pixels, byte w, byte h, byte nw, byte nh, byte *target, uint16 ofs);
void cga_AnimZoomIn(byte *pixels, byte w, byte h, byte *target, uint16 ofs);

void cga_ZoomInplaceXY(byte *pixels, byte w, byte h, byte nw, byte nh, uint16 x, uint16 y, byte *target);

} // End of namespace Chamber

#endif
