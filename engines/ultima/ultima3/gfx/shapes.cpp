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
#include "ultima/ultima3/gfx/pixels.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

constexpr int BYTES_PER_LINE = 4;
constexpr int SHAPE_BYTES = 64;

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

// The tile of the moon gate, whose rings of color are cycled in VGA
constexpr int GATE_TILE = 34;
constexpr int GATE_RINGS = 5;

// In VGA the flickering only affects the middle of a scanline
constexpr int VGA_FLICKER_START = 4;
constexpr int VGA_FLICKER_WIDTH = 8;

static int lineOffset(int line) {
	return (line & 1) * (SHAPE_BYTES / 2) + (line >> 1) * BYTES_PER_LINE;
}

void Shapes::load() {
	Common::File f;

	switch (_G(renderMode)) {
	case Data::RENDER_EGA: {
		byte data[SHAPE_COUNT * SHAPE_PIXELS / 2];
		if (!f.open("shapes.ega") || f.read(data, sizeof(data)) != sizeof(data))
			error("Could not load shapes.ega");

		unpack4bpp(data, _pixels, sizeof(_pixels));
		break;
	}

	case Data::RENDER_VGA:
		if (!f.open("shapes.vga") || f.read(_pixels, sizeof(_pixels)) != sizeof(_pixels))
			error("Could not load shapes.vga");
		break;

	default: {
		byte data[SHAPE_COUNT * SHAPE_BYTES];
		if (!f.open("SHAPES.ULT") || f.read(data, sizeof(data)) != sizeof(data))
			error("Could not load SHAPES.ULT");

		for (int index = 0; index < SHAPE_COUNT; ++index) {
			for (int line = 0; line < SHAPE_SIZE; ++line)
				unpack2bpp(&data[index * SHAPE_BYTES + lineOffset(line)],
					tile(index) + line * SHAPE_SIZE, SHAPE_SIZE);
		}
		break;
	}
	}

	for (int i = 0; i < 4; ++i)
		_scrollCounters[i] = SCROLL_PERIODS[i];
	for (int i = 0; i < 3; ++i)
		_flickerCounters[i] = FLICKER_PERIODS[i];
	_swapCounter = SWAP_PERIOD;
}

void Shapes::scrollTile(int index) {
	byte *t = tile(index);
	byte carry[SHAPE_SIZE];
	memcpy(carry, t + (SHAPE_SIZE - 1) * SHAPE_SIZE, SHAPE_SIZE);

	for (int line = 0; line < SHAPE_SIZE; ++line) {
		byte old[SHAPE_SIZE];
		byte *dest = t + line * SHAPE_SIZE;

		memcpy(old, dest, SHAPE_SIZE);
		memcpy(dest, carry, SHAPE_SIZE);
		memcpy(carry, old, SHAPE_SIZE);
	}
}

void Shapes::cycleRings(int index) {
	byte *t = tile(index);

	// Each ring is an arch, with the outer one starting at the top left. They
	// each take the color of the ring outside them
	byte color = t[1 * SHAPE_SIZE + 3];

	for (int ring = GATE_RINGS - 1; ring >= 0; --ring) {
		int x = 3 + ring;
		byte next = t[14 * SHAPE_SIZE + x];
		int width = 2 * (GATE_RINGS - 1 - ring) + 1;
		int rows = 13 - ring;
		int pos = 14 * SHAPE_SIZE + x;

		for (int i = 0; i < rows; ++i, pos -= SHAPE_SIZE) {
			t[pos] = color;
			t[pos + width] = color;
		}
		for (int i = 0; i <= width; ++i)
			t[pos + i] = color;

		color = next;
	}
}

void Shapes::swapLines(int index, int line1, int line2) {
	byte *t = tile(index);
	int start = 0, width = SHAPE_SIZE;

	if (_G(renderMode) == Data::RENDER_VGA) {
		start = VGA_FLICKER_START;
		width = VGA_FLICKER_WIDTH;
	}

	for (int i = 0; i < width; ++i)
		SWAP(t[line1 * SHAPE_SIZE + start + i], t[line2 * SHAPE_SIZE + start + i]);
}

void Shapes::swapTiles(int index1, int index2, int count) {
	for (int i = 0; i < count * SHAPE_PIXELS; ++i)
		SWAP(_pixels[index1 * SHAPE_PIXELS + i], _pixels[index2 * SHAPE_PIXELS + i]);
}

void Shapes::animate() {
	for (int i = 0; i < 4; ++i) {
		if (--_scrollCounters[i] == 0) {
			if (SCROLL_TILES[i] == GATE_TILE && _G(renderMode) == Data::RENDER_VGA)
				cycleRings(GATE_TILE);
			else
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

	const byte *t = &_pixels[index * SHAPE_PIXELS];

	for (int line = 0; line < SHAPE_SIZE; ++line)
		memcpy(dest.getBasePtr(x, y + line), t + line * SHAPE_SIZE, SHAPE_SIZE);
}

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima
