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

#include "m4/graphics/gr_draw.h"
#include "m4/graphics/gr_line.h"
#include "m4/graphics/gr_pal.h"

namespace M4 {

void buffer_put_pixel(Buffer *buf, int32 x, int32 y, byte c) {
	*(buf->data + x + (y * buf->stride)) = (uint8)c;
}

byte buffer_get_pixel(Buffer *buf, int32 x, int32 y) {
	return *(buf->data + x + (y * buf->stride));
}

void buffer_draw_box(Buffer *buf, int32 x1, int32 y1, int32 x2, int32 y2, byte color) {
	gr_color_set(color);
	gr_hline(buf, x1, x2, y1);
	gr_hline(buf, x1, x2, y2);
	gr_vline(buf, x1, y1, y2);
	gr_vline(buf, x2, y1, y2);
}

void buffer_draw_box_xor(Buffer *buf, int32 x1, int32 y1, int32 x2, int32 y2) {
	gr_hline_xor(buf, x1, x2, y1);
	gr_hline_xor(buf, x1, x2, y2);
	gr_vline_xor(buf, x1, y1 + 1, y2 - 1);
	gr_vline_xor(buf, x2, y1 + 1, y2 - 1);
}

int32 buffer_fill(Buffer *target, byte value) {
	if (!target)
		return false;
	if (!target->data)
		return false;

	memset(target->data, value, target->stride * target->h);
	return true;
}

} // namespace M4
