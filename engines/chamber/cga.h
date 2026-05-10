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

// HGA Constants
#define HGA_WIDTH 720
#define HGA_HEIGHT 348
#define HGA_BASE_SEG 0xB000
#define HGA_PAGE2_SEG 0xB800
#define HGA_NEXT_LINES_OFS 0x2000
#define HGA_BITS_PER_PIXEL 1
#define HGA_PIXELS_PER_BYTE (8 / HGA_BITS_PER_PIXEL)
#define HGA_BYTES_PER_LINE (HGA_WIDTH / HGA_PIXELS_PER_BYTE)
#define HGA_CALCXY_RAW(x, y) ( ((y) % 4) * HGA_NEXT_LINES_OFS + ((y) / 4) * HGA_BYTES_PER_LINE + (x) / HGA_PIXELS_PER_BYTE )
#define HGA_CENTERED_BASE_OFS HGA_CALCXY_RAW(40, 72)
#ifdef __386__
#define HGA_SCREENBUFFER ((byte*)(HGA_BASE_SEG * 16))
#define HGA_BACKBUFFER ((byte*)(HGA_PAGE2_SEG * 16))
#else
#if 0
#define HGA_SCREENBUFFER ((byte*)MK_FP(HGA_BASE_SEG, 0))
#define HGA_BACKBUFFER ((byte*)MK_FP(HGA_PAGE2_SEG, 0))


#define HGA_FONT_HEIGHT 6
#define frontbuffer HGA_SCREENBUFFER
#define backbuffer HGA_BACKBUFFER
#endif
#endif

// CGA Constants
#define CGA_WIDTH 320
#define CGA_HEIGHT 200
#define CGA_BASE_SEG 0xB800
#define CGA_ODD_LINES_OFS 0x2000
#define CGA_BITS_PER_PIXEL 2
#define CGA_PIXELS_PER_BYTE (8 / CGA_BITS_PER_PIXEL)
#define CGA_BYTES_PER_LINE (CGA_WIDTH / CGA_PIXELS_PER_BYTE)

extern byte SCREENBUFFER[0xFA00]; ///< CGA/HGA: 0xB800, EGA: 0xFA00 (64000)

#define CGA_FONT_HEIGHT 6

#define CGA_NEXT_LINE(offs) ((CGA_ODD_LINES_OFS ^ (offs)) + (((offs) & CGA_ODD_LINES_OFS) ? 0 : CGA_BYTES_PER_LINE))
#define CGA_PREV_LINE(offs) ((CGA_ODD_LINES_OFS ^ (offs)) - (((offs) & CGA_ODD_LINES_OFS) ? CGA_BYTES_PER_LINE : 0))

#define frontbuffer SCREENBUFFER
extern byte backbuffer[0xFA00]; ///< CGA/HGA: 0xB800, EGA: 0xFA00 (64000)

extern byte sprit_load_buffer[8192];

extern byte cga_pixel_flip[256];

extern byte char_draw_coords_x;
extern byte char_draw_coords_y;
extern byte *char_xlat_table;
extern byte string_ended;
extern byte char_draw_max_width;
extern byte char_draw_max_height;

void switchToTextMode(void);

void waitVBlank(void);


uint16 HGA_CalcXY(uint16 x, uint16 y);
uint16 HGA_CalcXY_p(uint16 x, uint16 y);
uint16 cga_CalcXY_p(uint16 x, uint16 y);

byte *cga_BackupImageReal(uint16 ofs, uint16 w, uint16 h);

void drawSpriteN(byte index, uint16 x, uint16 y, byte *target);
void drawSpriteNFlip(byte index, uint16 x, uint16 y, byte *target);

byte *loadSprite(byte index, byte *bank, byte *buffer, byte header_only);

} // End of namespace Chamber

#endif
