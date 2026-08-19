/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dune/room.h"
#include "dune/dune.h"

#include "common/debug.h"
#include "common/memstream.h"
#include "common/util.h"

namespace Dune {

byte hi(uint16 w) {
	return (w & 0xff00) >> 8;
}

byte lo(uint16 w) {
	return (w & 0x00ff);
}

Room::Room(DuneEngine *e, Resource r)
	: _engine(e), _r(r)
{
	uint16 offset_0 = _r->readUint16LE();

	_count = offset_0 / 2;
	_offsets.reserve(_count);
	_offsets.push_back(offset_0);
	for (int i = 1; i != _count; ++i) {
		_offsets.push_back(_r->readUint16LE());
	}

	_polygon_side_down.resize(200);
	_polygon_side_up.resize(200);

	_offsets.push_back(_r->size());
}

void Room::add_gradient_section(int x0, int y0, int x1, int y1, Common::Array<uint16> &xs) {
	int dx = abs(x1-x0);
	int sx = x0 < x1 ? 1 : -1;

	int dy = -abs(y1-y0);
	int sy = y0 < y1 ? 1 : -1;

	xs[y0] = x0;

	int err = dx+dy;
	if (dx == 0) {
		while (y0 != y1+sy) {
			xs[y0] = x0;
			y0 += sy;
		}
		return;
	}
	while (y0 != y1) {
		int e2 = 2*err;
		bool y_stepped = false;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
			y_stepped = true;
		}
		if (y_stepped) {
			xs[y0] = x0;
		}
	}
}

void Room::draw(byte *framebuffer, int index)
{
	assert(index < _count);
	_r->seek(_offsets[index]);

	int position_markers = _r->readByte();

	debug("Position markers: %d", position_markers);

	for (;;) {
		uint16_t cmd = _r->readUint16LE();

		if (cmd == 0xffff) {
			// global pal offset = 0;
			break;
		}

		if ((cmd & 0x8000) == 0) {
			// Draw sprite
			uint16 x          = _r->readByte() + ((cmd & 0x0200) ? 256 : 0);
			uint16 y          = _r->readByte();
			uint16 pal_offset = _r->readByte();

			byte       flags = (cmd >> 8);
			int sprite_index = (cmd & 0x1ff) - 1;
			if (sprite_index <= 0) {
				// Marker
				debug("MARKER: %04x: (%3x, %02x, %02x)", cmd, x, y, pal_offset);
				// Call draw_sprite from PERS.HSQ
				continue;
			}

			_engine->drawSprite(sprite_index, x, y, flags >> 1);
		} else if ((cmd & 0x4000) == 0) {
			// Polygon

			// TODO:
			int16 h_gradiant = 16 * _r->readSByte();
			int16 v_gradiant = 16 * _r->readSByte();

			uint16_t start_x = _r->readUint16LE();
			uint16_t start_y = _r->readUint16LE();

			// Part 1
			uint16 x, y;
			uint16_t last_x = start_x & 0x3fff;
			uint16_t last_y = start_y;
			do {
				x = _r->readUint16LE();
				y = _r->readUint16LE();
				add_gradient_section(last_x, last_y, x & 0x3fff, y, _polygon_side_down);
				last_x = x & 0x3fff;
				last_y = y;
			} while ((x & 0x4000) == 0);
			uint16 final_x = last_x;
			uint16 final_y = last_y;

			// Part 2
			last_x = start_x;
			last_y = start_y;
			if ((x & 0x8000) == 0) {
				do {
					x = _r->readUint16LE();
					y = _r->readUint16LE();
					add_gradient_section(last_x, last_y, x & 0x3fff, y, _polygon_side_up);
					last_x = x & 0x3fff;
					last_y = y;
				} while ((x & 0x8000) == 0);
			}

			add_gradient_section(last_x, last_y, final_x, final_y, _polygon_side_up);

			for (y = 0; y != final_y - start_y; ++y) {
				int w = _polygon_side_down[start_y + y] - _polygon_side_up[start_y + y];
				if (w) {
					_engine->_graphics.drawNoise(_polygon_side_up[y], start_y + y, w, (cmd & 0xff) << 8, 0, 0, 2);
				}
			}
		} else if ((cmd & 0x4000)) {
			// TODO: Draw line
			uint16 t0 = _r->readUint16LE();
			uint16 t1 = _r->readUint16LE();
			uint16 t2 = _r->readUint16LE();
			uint16 t3 = _r->readUint16LE();
		}
	}
}

} // End of namespace Dune
