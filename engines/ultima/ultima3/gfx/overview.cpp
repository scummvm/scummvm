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

#include "ultima/ultima3/gfx/overview.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

constexpr int SCREEN_OFFSET = 0x20;
constexpr int STEPS_PER_FLICKER = 2;
constexpr char GLYPH_PARTY = '*';

// The pixels of a square that the party flickers in, in the order they're changed
static const int FLICKER_X[4] = { 0, 1, 1, 0 };
static const int FLICKER_Y[4] = { 0, 1, 0, 1 };

void Overview::plot(int x, int y, int value) {
	_pixels[y * MAP_SIZE * 2 + x] ^= value;
}

void Overview::showMap() {
	const Data::Savegame &save = _G(savegame);
	memset(_pixels, 0, sizeof(_pixels));

	// Each square of the map becomes a block of four pixels with a shape
	// depending on what's in it
	for (int y = 0; y < MAP_SIZE; ++y) {
		for (int x = 0; x < MAP_SIZE; ++x) {
			byte cell = _G(map).cell(x, y);
			int px = x * 2, py = y * 2;

			switch (cell) {
			case 0:
				break;
			case 4:
				plot(px + 1, py + (x & 1), 1);
				break;
			case 8:
				plot(px + 1, py + 1, 1);
				break;
			case 0x0C:
				plot(px + 1, py, 1);
				plot(px + 1, py + 1, 1);
				break;
			case 0x10:
			case 0x8C:
				plot(px, py, 3);
				plot(px, py + 1, 3);
				plot(px + 1, py, 3);
				plot(px + 1, py + 1, 3);
				break;
			case 0x20:
				plot(px, py, 3);
				plot(px, py + 1, 3);
				break;
			default:
				plot(px, py, 3);
				plot(px + 1, py, 3);
				break;
			}
		}
	}

	_partyX = save._posX;
	_partyY = save._posY;
	_step = 0;
	memset(_lit, 0, sizeof(_lit));
	_type = WORLD;
}

void Overview::showLevel() {
	const Data::Savegame &save = _G(savegame);

	for (int y = 0; y < DUNGEON_SIZE; ++y) {
		for (int x = 0; x < DUNGEON_SIZE; ++x) {
			byte tile = _G(dungeon).tile(save._dungeonLevel, x, y);
			byte glyph;

			switch (tile) {
			case 0xC0: glyph = 1; break;
			case 0xA0: glyph = 2; break;
			case 0x80: glyph = 3; break;
			case 0x30: glyph = 4; break;
			case 0x20: glyph = 5; break;
			case 0x10: glyph = 6; break;
			case 0: glyph = 0; break;
			default: glyph = '?'; break;
			}

			_glyphs[y * DUNGEON_SIZE + x] = glyph;
		}
	}

	_partyX = save._posX;
	_partyY = save._posY;
	_step = 0;
	_type = LEVEL;
}

void Overview::tick() {
	if (_type == NONE)
		return;

	++_step;
	if (_type == WORLD && _step % STEPS_PER_FLICKER == 0) {
		int which = (_step / STEPS_PER_FLICKER) & 3;
		_lit[which] = !_lit[which];
	}
}

void Overview::draw(Graphics::Views::GfxSurface &s) const {
	if (_type == WORLD) {
		for (int y = 0; y < MAP_SIZE * 2; ++y) {
			for (int x = 0; x < MAP_SIZE * 2; ++x) {
				int value = _pixels[y * MAP_SIZE * 2 + x];

				// The flickering shows up as the opposite of what's there
				for (int i = 0; i < 4; ++i) {
					if (_lit[i] && x == _partyX * 2 + FLICKER_X[i] && y == _partyY * 2 + FLICKER_Y[i])
						value ^= 3;
				}

				if (value)
					s.setPixel(SCREEN_OFFSET + x, SCREEN_OFFSET + y, value);
			}
		}
	} else if (_type == LEVEL) {
		const int column = SCREEN_OFFSET / 8, row = SCREEN_OFFSET / 8;

		for (int y = 0; y < DUNGEON_SIZE; ++y) {
			for (int x = 0; x < DUNGEON_SIZE; ++x)
				s.writeChar(Common::Point(column + x, row + y), _glyphs[y * DUNGEON_SIZE + x]);
		}

		bool shown = (_step / (STEPS_PER_FLICKER * 2)) & 1;
		s.writeChar(Common::Point(column + _partyX, row + _partyY), shown ? GLYPH_PARTY : ' ');
	}
}

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima
