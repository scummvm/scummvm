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

#ifndef M4_GRAPHICS_GR_LINE_H
#define M4_GRAPHICS_GR_LINE_H

#include "m4/m4_types.h"

namespace M4 {

/**
 * Given starting and ending points on the Y axis, and the constant
 * X value, draws a line in the set color in the specified buffer 
 */
void gr_hline(Buffer *buf, int32 x1, int32 x2, int32 y);

/**
 * Given starting and ending points on the Y axis, and the constant
 * X value, draws a line in the given color on the live MCGA screen.
 */
void gr_vline(Buffer *buf, int32 x, int32 y1, int32 y2);
void gr_hline_xor(Buffer *buf, int32 x1, int32 x2, int32 y);
void gr_vline_xor(Buffer *buf, int32 x, int32 y1, int32 y2);
void gr_line(int32 x1, int32 y1, int32 x2, int32 y2, int32 color, Buffer *screen);

} // namespace M4

#endif
