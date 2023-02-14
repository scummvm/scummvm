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

#ifndef CHAMBER_PRINT_H
#define CHAMBER_PRINT_H

namespace Chamber {

extern byte *cur_str_end;

extern byte draw_x;
extern byte draw_y;

extern byte chars_color_bonw[];
extern byte chars_color_bonc[];
extern byte chars_color_wonb[];
extern byte chars_color_wonc[];

void printStringCentered(byte *str, byte *target);
byte *printStringPadded(byte *str, byte *target);

void drawMessage(byte *msg, byte *target);

void cga_DrawTextBox(byte *msg, byte *target);

void calcStringSize(byte *str, uint16 *w, uint16 *n);

} // End of namespace Chamber

#endif
