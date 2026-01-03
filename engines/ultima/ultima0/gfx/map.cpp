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

#include "ultima/ultima0/gfx/map.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Gfx {

void Map::draw(Graphics::ManagedSurface *s, bool showAsMap) {
	const auto &player = g_engine->_player;
	const auto &map = g_engine->_worldMap;

	s->clear();

	int x, y, x1, y1, w, h, grid;
	Common::Rect r;
	grid = showAsMap ? WORLD_MAP_SIZE : (g_engine->isEnhanced() ? 7 : 3);
	w = s->w / grid; h = s->h / grid;	// Get grid sizes

	for (x = 0; x < grid; x++) {
		for (y = 0; y < grid; y++) {
			r = Common::Rect(x * w, y * h, x * w + w - 1, y * h + h - 1);

			if (showAsMap) {
				// If map, not centered around us
				x1 = x, y1 = y;
			} else {
				// Which cell?
				x1 = player._worldPos.x - grid / 2 + x;
				y1 = player._worldPos.y - grid / 2 + y;
			}

			drawTile(s, r, map.read(x1, y1));

			// Draw us if we're there
			if (x1 == player._worldPos.x && y1 == player._worldPos.y)
				drawTile(s, r, WT_PLAYER);
		}
	}
}

#define HWColour(IDX)   color = IDX
#define	X(n)			(x1 + w * (n)/10)
#define	Y(n)			(y1 + h * (n)/10)
#define HWLine(X1, Y1, X2, Y2) s->drawLine(X1, Y1, X2, Y2, color)
#define BOX(x1,y1,x2,y2) { HWLine(X(x1),Y(y1),X(x2),Y(y1));HWLine(X(x1),Y(y1),X(x1),Y(y2));HWLine(X(x2),Y(y2),X(x2),Y(y1));HWLine(X(x2),Y(y2),X(x1),Y(y2)); }

void Map::drawTile(Graphics::ManagedSurface *s, const Common::Rect &r, int obj) {
	int x1 = r.left;
	int y1 = r.top;
	int w = r.width();
	int h = r.height();
	byte color = 0;

	// Decide on the object
	switch (obj) {
	case WT_SPACE:
		// Space does nothing at all
		break;

	case WT_MOUNTAIN:
		// Mountain the cracked effect
		HWColour(COL_MOUNTAIN);
		HWLine(X(2), Y(6), X(2), Y(10));
		HWLine(X(0), Y(8), X(2), Y(8));
		HWLine(X(2), Y(6), X(4), Y(6));
		HWLine(X(4), Y(6), X(4), Y(4));
		HWLine(X(2), Y(2), X(4), Y(4));
		HWLine(X(2), Y(2), X(2), Y(0));
		HWLine(X(2), Y(2), X(0), Y(2));
		HWLine(X(8), Y(4), X(4), Y(4));
		HWLine(X(8), Y(4), X(8), Y(0));
		HWLine(X(8), Y(2), X(10), Y(2));
		HWLine(X(6), Y(4), X(6), Y(8));
		HWLine(X(10), Y(8), X(6), Y(8));
		HWLine(X(8), Y(8), X(8), Y(10));
		break;

	case WT_TREE:
		// Tree is just a box
		HWColour(COL_TREE);
		BOX(3, 3, 7, 7);
		break;

	case WT_TOWN:
		// Town is 5 boxes
		HWColour(COL_TOWN);
		BOX(2, 2, 4, 4); BOX(4, 4, 6, 6); BOX(6, 6, 8, 8);
		BOX(6, 2, 8, 4); BOX(2, 6, 4, 8);
		break;

	case WT_DUNGEON:
		// Dungeon is a cross
		HWColour(COL_DUNGEON);
		HWLine(X(3), Y(3), X(7), Y(7));
		HWLine(X(7), Y(3), X(3), Y(7));
		break;

	case WT_BRITISH:
		// British castle
		HWColour(COL_BRITISH);
		HWLine(X(2), Y(2), X(8), Y(8));
		HWLine(X(8), Y(2), X(2), Y(8));
		BOX(0, 0, 10, 10);
		BOX(2, 2, 8, 8);
		break;

	case WT_PLAYER:
		// Player
		HWColour(COL_PLAYER);
		HWLine(X(4), Y(5), X(6), Y(5));
		HWLine(X(5), Y(4), X(5), Y(6));
		break;

	default:
		break;
	}
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
