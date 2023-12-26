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

#ifndef M4_GRAPHICS_GR_DRAW_H
#define M4_GRAPHICS_GR_DRAW_H

#include "m4/m4_types.h"

namespace M4 {

/**
 * Given X, Y sets the value of the pixel at that position
 */
void buffer_put_pixel(Buffer *buf, int32 x, int32 y, byte c);

/**
 * Given X, Y returns value of pixel at that address on the screen
 */
byte buffer_get_pixel(Buffer *buf, int32 x, int32 y);

/**
 * Draws outside edge of retangle given home and size along both axis
 */
void buffer_draw_box(Buffer *buf, int32 x1, int32 y1, int32 x2, int32 y2, byte color);

/**
 * Draws a rectangle using xor
 */
void buffer_draw_box_xor(Buffer *buf, int32 x1, int32 y1, int32 x2, int32 y2);

/**
 * Fills an entire buffer with a single byte value.
 * @returns Returns True if successful, FALSE if buffer invalid.
*/
int32 buffer_fill(Buffer *target, byte value);

} // namespace M4

#endif
