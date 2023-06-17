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

#include "common/textconsole.h"
#include "m4/graphics/gr_pal.h"

namespace M4 {

uint8 gr_pal_get_ega_color(uint8 myColor) {
	error("TODO");
}

uint8 *gr_color_createInverseTable(RGB8 *pal, uint8 bitDepth, int begin_color, int end_color) {
	error("TODO: gr_color_createInverseTable");
}

void gr_color_create_ipl5(uint8 *inverseColorTable, char *fname, int room_num) {
	error("TODO: gr_color_create_ipl5");
}

uint8 *gr_color_load_ipl5(const char *filename, uint8 *inverseColors) {
	error("TODO: gr_color_load_ipl5");
}

void gr_color_set(int32 c) {
	error("TODO: gr_color_set");
}

byte gr_color_get_current() {
	error("TODO: gr_color_get_current");
}

} // namespace M4
