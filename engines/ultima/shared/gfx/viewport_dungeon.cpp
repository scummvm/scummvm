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

#include "ultima/shared/gfx/viewport_dungeon.h"
#include "ultima/shared/core/monsters.h"
#include "ultima/shared/core/map.h"
#include "ultima/shared/early/game.h"

namespace Ultima {
namespace Shared {

EMPTY_MESSAGE_MAP(ViewportDungeon, Shared::Gfx::VisualItem);

const byte WALL_ARRAY_X[] = { 0, 72, 108, 126, 135, 144 };
const byte WALL_ARRAY_Y[] = { 0, 36, 54, 63, 68, 72 };

void ViewportDungeon::draw() {
	// Get a surface reference and clear it's contents
	Gfx::VisualSurface s = getSurface();
	s.clear();

	// Get the position delta for the cells to the left and right of the player's position
	Map *map = getMap();
	Point rightDelta, leftDelta, delta;
	switch (map->_direction) {
	case DIR_LEFT:
		leftDelta.y = 1;
		rightDelta.y = -1;
		delta.x = -1;
		break;
	case DIR_RIGHT:
		leftDelta.y = -1;
		rightDelta.y = 1;
		delta.x = 1;
		break;
	case DIR_UP:
		leftDelta.x = -1;
		rightDelta.x = 1;
		delta.y = -1;
		break;
	case DIR_DOWN:
		leftDelta.x = 1;
		rightDelta.x = -1;
		delta.y = 1;
		break;
	}

	bool isDoor = map->isDoor(map->getPosition());
	bool isWall = map->isWallOrSecretDoor(map->getPosition());
	int distance = distanceToOccupiedCell(delta);

	if (isWall) {
		drawWall(0);
		return;
	}

}

uint ViewportDungeon::distanceToOccupiedCell(const Point &delta) {
	Point d = delta;
	uint distance;
	for (distance = 1; !isCellOccupied(d); ++distance, d.x += ABS(d.x), d.y += ABS(d.y)) {}

	return MIN(distance, (uint)5);
}

bool ViewportDungeon::isCellOccupied(const Point &delta) {
	Map *map = getMap();
	Point pt = map->getPosition() + delta;
	if (map->isWallOrDoorway(pt))
		return true;

	return isMonsterBlocking(pt);
}

bool ViewportDungeon::isMonsterBlocking(const Point &pt) {
	MapTile tile;
	getMap()->getTileAt(pt, &tile);
	DungeonMonster *monster = dynamic_cast<DungeonMonster *>(tile._widget);
	return monster != nullptr && monster->isBlockingView();
}

void ViewportDungeon::drawWall(uint distance) {
	Game *game = getGame();
	Gfx::VisualSurface s = getSurface();
	int offsetX = !distance ? 8 : 0, offsetY = !distance ? 8 : 0;

	if (distance <= 5) {
		s.hLine(WALL_ARRAY_X[distance] + 16 + offsetX, WALL_ARRAY_Y[distance] + 8 + offsetY,
			303 - WALL_ARRAY_X[distance] - offsetX, game->_edgeColor);
		s.hLine(WALL_ARRAY_X[distance] + 16 + offsetX, 15 - WALL_ARRAY_Y[distance] - offsetY,
			303 - WALL_ARRAY_X[distance] - offsetX, game->_edgeColor);
	}
}

} // End of namespace Shared
} // End of namespace Ultima
