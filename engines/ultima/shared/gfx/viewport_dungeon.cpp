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

void ViewportDungeon::draw() {
	// Get a surface reference and clear it's contents
	DungeonSurface s = getSurface();
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

	Point currentPos = map->getPosition();
	bool isDoor = map->isDoor(map->getPosition());
	bool isWall = map->isWallOrSecretDoor(currentPos);
	int distance = distanceToOccupiedCell(delta);

	// If stuck in a wall, draw it and exit
	if (isWall) {
		s.drawWall(0);
		return;
	}

	Point backDelta(delta.x * distance, delta.y * distance),
		currDelta(delta.x * distance, delta.y * distance);

	if (isDoor && map->isWallOrDoorway(map->getPosition() + delta)) {
		s.drawWall(0);
	} else {
		if (isDoor)
			s.drawDoorway(0);

		bool endingLeft = false, endingRight = false;
		bool var5 = 0, var6 = 0;
		byte var7 = 0, var8 = 0;
		for (int index = distance; distance; --distance) {
			currDelta -= delta;
			
			if (!isDoor || distance > 1) {
				// todo
			}


		}

		if (isDoor)
			drawCell(0, currentPos);

		if (isMonsterBlocking(currentPos + backDelta) && map->isDoor(currentPos + backDelta) && distance < 5) {
			drawLeftCell(distance + 1, currentPos + leftDelta);
			drawRightCell(distance + 1, currentPos + rightDelta);

			if (!map->isSolid(currentPos + leftDelta))
				s.drawLeftEdge(distance);
			if (!map->isSolid(currentPos + rightDelta))
				s.drawRightEdge(distance);
		} else {
			if (endingLeft)
				s.drawLeftEdge(distance);
			if (endingRight)
				s.drawRightEdge(distance);
		}
	}

	if (isDoor) {

	}

	// TODO
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

void ViewportDungeon::drawCell(uint distance, const Point &pt) {
	Game *game = getGame();
	DungeonSurface s = getSurface();
	Map *map = getMap();

	MapTile tile;
	map->getTileAt(pt, &tile);

	if (tile._widget) {
		// Draw a monster
		if (map->isWallOrDoorway(pt))
			s.drawWall(distance);
		if (tile._tileNum == 7)
			// Ladder down
			s.drawWidget(27, distance, game->_edgeColor);
		if (tile._tileNum == 6)
			// Ladder up
			s.drawWidget(26, distance, game->_edgeColor);

		static_cast<DungeonMonster *>(tile._widget)->draw(s, distance);
	} else {
		switch (tile._tileNum) {
		case 1:
		case 2:
			// Wall or secret door
			s.drawWall(distance);
			break;
		case 3:
			// Doorway
			s.drawDoorway(distance);
			break;
		case 6:
			// Ladder down
			if (map->_direction == DIR_UP || map->_direction == DIR_DOWN) {
				s.drawLadderDownFaceOn(distance);
			} else {
				s.drawLadderDownSideOn(distance);
			}
			break;
		case 7:
			// Ladder up
			if (map->_direction == DIR_UP || map->_direction == DIR_DOWN) {
				s.drawLadderUpFaceOn(distance);
			} else {
				s.drawLadderUpSideOn(distance);
			}
			break;
		case 8:
			// Beams
			s.drawBeams(distance);
			break;
		default:
			break;
		}
	}
}

void ViewportDungeon::drawLeftCell(uint distance, const Point &pt) {
	DungeonSurface s = getSurface();
	Map *map = getMap();

	if (map->isDoor(pt))
		s.drawLeftDoor(distance);
	else if (map->isWallOrSecretDoor(pt))
		s.drawLeftWall(distance);
	else
		s.drawLeftBlank(distance);
}

void ViewportDungeon::drawRightCell(uint distance, const Point &pt) {
	DungeonSurface s = getSurface();
	Map *map = getMap();

	if (map->isDoor(pt))
		s.drawRightDoor(distance);
	else if (map->isWallOrSecretDoor(pt))
		s.drawRightWall(distance);
	else
		s.drawRightBlank(distance);
}

} // End of namespace Shared
} // End of namespace Ultima
