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

#include "mm/mm1/views_enh/map.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define BORDER_SIZE 1
#define TILE_W 10
#define TILE_H 8


Map::Map(UIElement *owner) : UIElement("Map", owner) {
}

void Map::draw() {
	MM1::Maps::Maps &maps = g_globals->_maps;
	MM1::Maps::Map &map = *maps._currentMap;
	Graphics::ManagedSurface s = getSurface();

	g_globals->_globalSprites.draw(&s, 15,
		Common::Point(BORDER_SIZE, BORDER_SIZE));

	assert((_bounds.width() - BORDER_SIZE * 2) % TILE_W == 0);
	assert((_bounds.height() - BORDER_SIZE * 2) % TILE_H == 0);
	int totalX = (_bounds.width() - BORDER_SIZE * 2) / TILE_W,
		totalY = (_bounds.height() - BORDER_SIZE * 2) / TILE_H;

	int currentX = maps._mapPos.x;
	int currentY = maps._mapPos.y;
	int xStart = CLIP(currentX - (totalX - 1) / 2, 0,
		MAP_W - totalX);
	int yStart = CLIP(currentY - (totalY - 1) / 2, 0,
		MAP_H - totalY);

	// Iterate through the cells
	for (int yp = BORDER_SIZE, mazeY = yStart + totalY - 1;
			mazeY >= yStart; yp += TILE_H, --mazeY) {
		for (int xp = BORDER_SIZE, mazeX = xStart;
				mazeX < (xStart + totalX); xp += TILE_W, ++mazeX) {
			byte visited = map._visited[mazeY * MAP_W + mazeX];

			if (!visited) {
				g_globals->_tileSprites.draw(&s, 1,
					Common::Point(xp, yp));
			} else {
				Common::Rect r(xp, yp, xp + TILE_W, yp + TILE_H);
				byte walls = map._walls[mazeY * MAP_W + mazeX];
				int wallsW = walls & 3;
				int wallsS = (walls >> 2) & 3;
				int wallsE = (walls >> 4) & 3;
				int wallsN = (walls >> 6) & 3;

				// Color visited cells in a color depending on the cell type,
				// and have a line on the edges representing walls vs doors
				if (visited == Maps::VISITED_NORMAL)
					s.fillRect(r, 0);
				else if (visited == Maps::VISITED_SPECIAL)
					s.fillRect(r, 0x10);
				else if (visited == Maps::VISITED_EXIT)
					s.fillRect(r, 0x20);
				else if (visited == Maps::VISITED_BUSINESS)
					s.fillRect(r, 0x30);

				if (wallsN != Maps::WALL_NONE)
					s.hLine(r.left, r.top, r.right - 1,
						wallsN == Maps::WALL_DOOR ? 128 : 255);
				if (wallsS != Maps::WALL_NONE)
					s.hLine(r.left, r.bottom - 1, r.right - 1,
						wallsS == Maps::WALL_DOOR ? 128 : 255);
				if (wallsE != Maps::WALL_NONE)
					s.vLine(r.right - 1, r.top, r.bottom - 1,
						wallsE == Maps::WALL_DOOR ? 128 : 255);
				if (wallsW != Maps::WALL_NONE)
					s.vLine(r.left, r.top, r.bottom - 1,
						wallsW == Maps::WALL_DOOR ? 128 : 255);

				// Draw arrow if it's the cell the party is on
				if (mazeX == maps._mapPos.x && mazeY == maps._mapPos.y) {
					int dir;
					switch (maps._forwardMask) {
					case Maps::DIRMASK_N: dir = 1; break;
					case Maps::DIRMASK_E: dir = 2; break;
					case Maps::DIRMASK_S: dir = 3; break;
					default: dir = 4; break;
					}

					g_globals->_globalSprites.draw(&s,
						dir, Common::Point(xp, yp));
				}
			}
		}
	}
}

} // namespace Views
} // namespace MM1
} // namespace MM
