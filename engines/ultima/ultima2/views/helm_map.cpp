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

#include "ultima/ultima2/views/helm_map.h"
#include "ultima/ultima2/data/map.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

constexpr int COLOR_ICON = 3;

// Matches the screen offset used by every other full-screen point/line renderer
constexpr int VIEW_OFFSET = 16;

// Each world cell maps to a 4-wide x 2-tall block of plot points; the
// points lit within that block form a small per-terrain-category icon
static const byte ICON_GRASS[][2] = { {1, 0}, {3, 1} };
static const byte ICON_FOREST[][2] = { {1, 1}, {3, 0}, {1, 0}, {3, 1} };
static const byte ICON_MOUNTAIN_OR_SPRITE[][2] = { {0, 0}, {0, 1}, {2, 1}, {2, 0}, {1, 1}, {3, 0}, {1, 0}, {3, 1} };
static const byte ICON_SWAMP[][2] = { {1, 0}, {3, 0} };
static const byte ICON_ROAD[][2] = { {0, 0}, {0, 1}, {2, 0}, {2, 1} };
static const byte ICON_OTHER[][2] = { {1, 0}, {2, 0}, {2, 1}, {1, 1} };

HelmMap::HelmMap() : Shared::Gfx::View("HelmMap") {
	setBounds(TextRect(0, 0, 39, 19));
}

void HelmMap::plotIcon(Shared::Gfx::GfxSurface &s, int cellX, int cellY, Data::TileId tile) {
	const byte (*points)[2];
	int count;

	if (tile == Data::TILE_WATER) {
		return;
	} else if (tile == Data::TILE_MOUNTAIN || (tile >= Data::TILE_WALL && tile <= Data::TILE_COUNTER_END_LEFT)) {
		points = ICON_MOUNTAIN_OR_SPRITE;
		count = ARRAYSIZE(ICON_MOUNTAIN_OR_SPRITE);
	} else if (tile == Data::TILE_GRASS) {
		points = ICON_GRASS;
		count = ARRAYSIZE(ICON_GRASS);
	} else if (tile == Data::TILE_FOREST) {
		points = ICON_FOREST;
		count = ARRAYSIZE(ICON_FOREST);
	} else if (tile == Data::TILE_SWAMP) {
		points = ICON_SWAMP;
		count = ARRAYSIZE(ICON_SWAMP);
	} else if (tile == Data::TILE_ROAD) {
		points = ICON_ROAD;
		count = ARRAYSIZE(ICON_ROAD);
	} else {
		points = ICON_OTHER;
		count = ARRAYSIZE(ICON_OTHER);
	}

	int px = cellX * 4 + VIEW_OFFSET, py = cellY * 2 + VIEW_OFFSET;
	for (int i = 0; i < count; ++i)
		s.setPixel(px + points[i][0], py + points[i][1], COLOR_ICON);
}

void HelmMap::draw() {
	auto s = getSurface();
	s.clear();

	for (int y = 0; y < Data::MAP_HEIGHT; ++y) {
		for (int x = 0; x < Data::MAP_WIDTH; ++x)
			plotIcon(s, x, y, _G(map).tileAt(x, y));
	}
}

bool HelmMap::msgKeypress(const KeypressMessage &msg) {
	close();
	_G(logic)->resumeTurn();
	return true;
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
