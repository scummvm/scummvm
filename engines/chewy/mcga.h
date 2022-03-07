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

#ifndef CHEWY_MCGA_H
#define CHEWY_MCGA_H

namespace Chewy {

void init_mcga();

byte *get_dispoff();

void setScummVMPalette(const byte *palette, uint start, uint count);
void set_palette(const byte *palette);
void rastercol(int16 color, int16 r, int16 g, int16 b);
void setPartialPalette(const byte *palette, int16 startCol, int16 nr);

void clear_mcga();
uint8 getpix(int16 x, int16 y);
void line_mcga(int16 x1, int16 y1, int16 x2, int16 y2, int16 color);

void mem2mcga(const byte *ptr);

void map_spr_2screen(const byte *sptr, int16 x, int16 y);
void spr_save_mcga(byte *sptr, int16 x, int16 y, int16 width,
	int16 height, int16 scrWidth);
void spr_set_mcga(const byte *sptr, int16 x, int16 y, int16 scrWidth);
void mspr_set_mcga(byte *sptr, int16 x, int16 y, int16 scrWidth);
void zoom_set(byte *source, int16 x, int16 y, int16 xdiff,
    int16 ydiff, int16 scrWidth);

void putcxy(int16 x, int16 y, unsigned char c, int16 fgCol, int16 bgCol, int16 scrWidth);
void putz(unsigned char c, int16 fgCol, int16 bgCol, int16 scrWidth);
void vors();

void upd_scr();

} // namespace Chewy

#endif
