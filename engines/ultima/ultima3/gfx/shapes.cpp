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

#include "common/file.h"
#include "common/util.h"
#include "ultima/ultima3/gfx/shapes.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

constexpr int BYTES_PER_LINE = 4;

// Tiles scrolled one pixel at a time, and how many ticks pass between each
static const int SCROLL_TILES[4] = { 0, 33, 32, 34 };
static const byte SCROLL_PERIODS[4] = { 3, 2, 3, 2 };

// Pairs of scanlines swapped in a tile to make it flicker, and their periods
static const int FLICKER_TILES[3] = { 7, 6, 11 };
static const int FLICKER_LINES[3][2] = { { 1, 2 }, { 3, 4 }, { 2, 3 } };
static const byte FLICKER_PERIODS[3] = { 4, 3, 2 };

// Every few ticks the frames of the animated tiles are swapped for their
// alternates: tiles 16-30 with 64-78, and tile 12 with 79
constexpr byte SWAP_PERIOD = 10;

static int lineOffset(int line) {
	return (line & 1) * (SHAPE_BYTES / 2) + (line >> 1) * BYTES_PER_LINE;
}

void Shapes::load() {
	Common::File f;
	if (!f.open("SHAPES.ULT") || f.read(_data, sizeof(_data)) != sizeof(_data))
		error("Could not load SHAPES.ULT");

	for (int i = 0; i < 4; ++i)
		_scrollCounters[i] = SCROLL_PERIODS[i];
	for (int i = 0; i < 3; ++i)
		_flickerCounters[i] = FLICKER_PERIODS[i];
	_swapCounter = SWAP_PERIOD;
}

void Shapes::scrollTile(int index) {
	byte *t = tile(index);
	byte carry[BYTES_PER_LINE];
	memcpy(carry, t + lineOffset(15), BYTES_PER_LINE);

	for (int line = 0; line < SHAPE_SIZE; ++line) {
		byte old[BYTES_PER_LINE];
		byte *dest = t + lineOffset(line);

		memcpy(old, dest, BYTES_PER_LINE);
		memcpy(dest, carry, BYTES_PER_LINE);
		memcpy(carry, old, BYTES_PER_LINE);
	}
}

void Shapes::swapLines(int index, int line1, int line2) {
	byte *t = tile(index);
	byte temp[BYTES_PER_LINE];

	memcpy(temp, t + lineOffset(line1), BYTES_PER_LINE);
	memcpy(t + lineOffset(line1), t + lineOffset(line2), BYTES_PER_LINE);
	memcpy(t + lineOffset(line2), temp, BYTES_PER_LINE);
}

void Shapes::swapTiles(int index1, int index2, int count) {
	for (int i = 0; i < count * SHAPE_BYTES; ++i)
		SWAP(_data[index1 * SHAPE_BYTES + i], _data[index2 * SHAPE_BYTES + i]);
}

void Shapes::animate() {
	for (int i = 0; i < 4; ++i) {
		if (--_scrollCounters[i] == 0) {
			scrollTile(SCROLL_TILES[i]);
			_scrollCounters[i] = SCROLL_PERIODS[i];
		}
	}

	for (int i = 0; i < 3; ++i) {
		if (--_flickerCounters[i] == 0) {
			swapLines(FLICKER_TILES[i], FLICKER_LINES[i][0], FLICKER_LINES[i][1]);
			_flickerCounters[i] = FLICKER_PERIODS[i];
		}
	}

	if (--_swapCounter == 0) {
		swapTiles(16, 64, 15);
		swapTiles(12, 79, 1);
		_swapCounter = SWAP_PERIOD;
	}
}

void Shapes::drawTile(Graphics::ManagedSurface &dest, int x, int y, int index) const {
	if (index < 0 || index >= SHAPE_COUNT)
		return;

	const byte *t = &_data[index * SHAPE_BYTES];

	for (int line = 0; line < SHAPE_SIZE; ++line) {
		const byte *src = t + lineOffset(line);
		byte *destP = (byte *)dest.getBasePtr(x, y + line);

		for (int b = 0; b < BYTES_PER_LINE; ++b) {
			for (int p = 0; p < 4; ++p)
				*destP++ = (src[b] >> (6 - p * 2)) & 3;
		}
	}
}

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima
