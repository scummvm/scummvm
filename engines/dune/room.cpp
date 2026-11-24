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

#include "dune/detection.h"
#include "dune/dune.h"
#include "dune/map_renderer.h"
#include "dune/sprite_sheet.h"

#include "common/debug.h"
#include "common/memstream.h"
#include "common/ptr.h"
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

void Room::add_gradient_section(int x0, int y0, int x1, int y1, Common::Array<uint16> &xs, int &row) {
	// A scene drawn with the wrong bank, or one that has been misread, can
	// name a line that runs off the screen; the edges it is stepped into are
	// only as tall as the screen is.
	const int height = (int)xs.size();
	auto put = [&](int x) {
		if (row >= 0 && row < height)
			xs[row] = (uint16)x;
		++row;
	};

	const int dx = ABS(x1 - x0), dy = ABS(y1 - y0);
	const int sx = x1 >= x0 ? 1 : -1, sy = y1 >= y0 ? 1 : -1;
	// 3E2F emits one horizontal endpoint, selected by the word-add carry.
	if (dy == 0) {
		const uint32 sum = (uint16)x0 + (uint32)(uint16)(x1 - x0);
		put(sum > 0xffff ? (uint16)sum : x0);
		return;
	}
	// 3E52 includes both endpoints for a vertical segment. The edge-table
	// cursor continues after that extra row, rather than overwriting it.
	if (dx == 0) {
		for (int i = 0; i <= dy; ++i)
			put(x0);
		return;
	}
	// 3E80 uses a half-major accumulator. Crucially 3EDF writes the OLD
	// X before each positive Y step. A generic line rasterizer's new X
	// changes run lengths and the continuing floor dither phase.
	const bool xMajor = dx > dy;
	const int major = MAX(dx, dy), minor = MIN(dx, dy);
	int error = major / 2;
	for (int i = 0; i < major; ++i) {
		error += minor;
		const bool diagonal = error >= major;
		if (diagonal)
			error -= major;
		if ((!xMajor || diagonal) && sy == 1)
			put(x0);
		if (xMajor || diagonal)
			x0 = (uint16)(x0 + sx);
	}
}

void Room::draw(byte *framebuffer, int index, bool drawPeople)
{
	assert(index < _count);
	_r->seek(_offsets[index]);

	const int position_markers = _r->readByte();
	_markers.clear();
	const Common::Array<byte> standing = drawPeople
		? _engine->peoplePresent(position_markers) : Common::Array<byte>();
	// 3D2F temporarily selects PERS; 3D80 restores the scene's bank. Keep
	// both decoded sheets alive rather than reopening them at every marker.
	SpriteSheet *sceneSheet = _engine->currentSpriteSheet;
	Common::ScopedPtr<SpriteSheet> peopleSheet;
	if (drawPeople)
		peopleSheet.reset(new SpriteSheet(_engine,
			_engine->_archive.openMember("PERS.HSQ"), false));
	_engine->_graphics.setFrameBuffer(framebuffer);

	debugC(2, kDebugGraphics, "scene %d: %d position markers", index, position_markers);

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

			int sprite_index = (cmd & 0x1ff) - 1;
			if (sprite_index <= 0) {
				// A place for a figure rather than a sprite of the scene.
				const Marker marker = { x, y, (byte)pal_offset, cmd };
				_markers.push_back(marker);
				const uint at = _markers.size() - 1;
				if (drawPeople && at < standing.size()) {
					const byte who = standing[standing.size() - 1 - at];
					if (who == 0xff)
						continue; // 3D21: an empty mark does not select PERS.
					_engine->currentSpriteSheet = peopleSheet.get();
					peopleSheet->applyPalette();
					_engine->drawStandingPerson(who,
						at, x, y, cmd, (byte)pal_offset);
					_engine->currentSpriteSheet = sceneSheet;
					// 3D2F/3D80: PERS supplies shared figure colours, then the
					// saved room bank restores its own ranges before drawing on.
					if (sceneSheet)
						sceneSheet->applyPalette();
				}
				continue;
			}

			// 0x3B9F hands 0xC22F the command itself with the bit that carries
			// the ninth of x taken out of it, and the palette offset in the
			// byte at CS:0xC21A beside it.  The offset used to be read and
			// thrown away, which left every figure and every thing in a room
			// in the colours of its own bank rather than the room's.
			_engine->drawSprite(sprite_index, x, y, cmd & ~0x0200, (byte)pal_offset);
		} else if ((cmd & 0x4000) == 0) {
			// Polygon.  The two gradients are eight bit fractions of a colour,
			// one for each pixel across a row and one for each row down; 0x3BFD
			// takes them out of the file and shifts each up by four.
			const byte polyFlags = (byte)(cmd >> 8);
			int16 h_gradiant = 16 * _r->readSByte();
			int16 v_gradiant = 16 * _r->readSByte();

			uint16_t start_x = _r->readUint16LE();
			uint16_t start_y = _r->readUint16LE();

			// Part 1
			uint16 x, y;
			uint16_t last_x = start_x & 0x3fff;
			uint16_t last_y = start_y;
			int downRow = start_y;
			do {
				x = _r->readUint16LE();
				y = _r->readUint16LE();
				add_gradient_section(last_x, last_y, x & 0x3fff, y, _polygon_side_down, downRow);
				last_x = x & 0x3fff;
				last_y = y;
			} while ((x & 0x4000) == 0);
			uint16 final_x = last_x;
			uint16 final_y = last_y;

			// Part 2
			last_x = start_x;
			last_y = start_y;
			int upRow = start_y;
			if ((x & 0x8000) == 0) {
				do {
					x = _r->readUint16LE();
					y = _r->readUint16LE();
					add_gradient_section(last_x, last_y, x & 0x3fff, y, _polygon_side_up, upRow);
					last_x = x & 0x3fff;
					last_y = y;
				} while ((x & 0x8000) == 0);
			}

			add_gradient_section(last_x, last_y, final_x, final_y, _polygon_side_up, upRow);

			// The colour is an eight bit fraction of its own: the byte the
			// command carries is the whole part and the row gradient is added
			// to it after every row.  The pattern that dithers the fill is an
			// exclusive-or shift register - 0x3C91 starts it at one where the
			// command asks for a pattern at all, and nought where it does not,
			// which fills flat - and 0x38E9 leaves it where it has got to, so
			// it runs on from row to row over the whole polygon.
			uint16 colour = (cmd & 0xff) << 8;
			uint16 pattern = (polyFlags & 0x3e) ? 1 : 0;
			const uint16 tap = (uint16)((polyFlags & 0x3e) << 8) | 2;
			// With bit 0 set the run is laid down from its right hand end, so
			// the gradient across it runs the other way.
			const bool leftwards = (polyFlags & 1) != 0;

			for (y = 0; y != final_y - start_y; ++y) {
				const int row = start_y + y;
				if (row < 0 || row >= (int)_polygon_side_down.size())
					break;

				// A row runs between the two edges and takes in both of them:
				// 0x3CC1 counts one more than the difference, and 0x3CBD puts
				// the smaller of the two first, whichever edge it came from.
				const int a = _polygon_side_up[row];
				const int b = _polygon_side_down[row];
				const int left = MIN(a, b);
				const int right = MAX(a, b);

				if (right >= 0 && left < 320) {
					// The original writes the whole run and lets what falls off
					// the screen go where it may; here it is cut short, so the
					// colour is carried over the pixels that were left out to
					// keep the gradient standing where it would have stood.
					const int first = MAX(left, 0);
					const int last = MIN(right, 319);
					const int skipped = leftwards ? right - last : first - left;
					const int count = last - first + 1;

					if (count > 0)
						pattern = _engine->_graphics.drawNoise(leftwards ? last : first, row, count,
															   colour + h_gradiant * skipped,
															   h_gradiant, pattern, tap, leftwards);
				}

				colour += v_gradiant;
			}
		} else if ((cmd & 0x4000)) {
			// A straight line between two points, in the colour the command's
			// low byte names.  0x3BC9 reads the four words and hands them to
			// the driver's line routine with every bit of its dash pattern
			// set, so the whole of it is drawn.  These were read and dropped,
			// which left the trim off the edges of several rooms.
			const int16 x0 = (int16)_r->readUint16LE();
			const int16 y0 = (int16)_r->readUint16LE();
			const int16 x1 = (int16)_r->readUint16LE();
			const int16 y1 = (int16)_r->readUint16LE();

			// 3BDF/3BE2 uses DNVGA's patterned line entry, just like the
			// survey. Non-axis lines step before plotting (omit the first
			// endpoint), unlike Graphics::drawLine's generic Bresenham loop.
			drawPatternedMapLine(framebuffer, 320, Common::Point(x0, y0), Common::Point(x1, y1),
					Common::Rect(0, 0, 320, 152), (byte)(cmd & 0xff), 0xffff);
		}
	}
}

} // End of namespace Dune
