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
#include "ultima/shared/maps/dungeon_widget.h"
#include "ultima/shared/maps/dungeon_creature.h"
#include "ultima/shared/maps/map_widget.h"
#include "ultima/shared/maps/map.h"
#include "ultima/shared/early/game.h"

namespace Ultima {
namespace Shared {

EMPTY_MESSAGE_MAP(ViewportDungeon, Shared::Gfx::VisualItem);

void ViewportDungeon::draw() {
	// Get a surface reference and clear it's contents
	DungeonSurface s = getSurface();
	s.clear();

	// Get the position delta for the facing direction, and the cells to the left and right of that
	Maps::Map *map = getGame()->getMap();
	Point delta = map->getDirectionDelta();

	Point leftDelta, rightDelta;
	switch (map->getDirection()) {
	case Maps::DIR_LEFT:
		leftDelta.y = 1;
		rightDelta.y = -1;
		break;
	case Maps::DIR_RIGHT:
		leftDelta.y = -1;
		rightDelta.y = 1;
		break;
	case Maps::DIR_UP:
		leftDelta.x = -1;
		rightDelta.x = 1;
		break;
	case Maps::DIR_DOWN:
		leftDelta.x = 1;
		rightDelta.x = -1;
		break;
	default:
		break;
	}

	Maps::MapTile tile, deltaTile, leftTile, rightTile, backTile;
	Point currentPos = map->getPosition();
	map->getTileAt(currentPos, &tile);
	map->getTileAt(currentPos + delta, &deltaTile);

	bool isDoor = tile.isDoor();
	bool isWall = tile.isWallOrSecretDoor();
	int distance = distanceToOccupiedCell(delta);

	// If stuck in a wall, draw it and exit
	if (isWall) {
		s.drawWall(0);
		return;
	}

	Point backDelta(delta.x * distance, delta.y * distance),
		currDelta(delta.x * distance, delta.y * distance);

	if (isDoor && deltaTile.isWallOrDoorway()) {
		s.drawWall(0);
	} else {
		if (isDoor)
			s.drawDoorway(0);

		// These are essentially boolean flags with an extra 'initial' state
		byte endingLeft = 100, endingRight = 100;
		byte leftFlag = 100, rightFlag = 100;
		byte priorLeftFlag = 100, priorRightFlag = 100;

		for (int index = distance; index; --index) {
			currDelta -= delta;
			Point pt = currentPos + currDelta;
			
			if (!isDoor || index > 1) {
				map->getTileAt(pt + leftDelta, &leftTile);
				map->getTileAt(pt + rightDelta, &rightTile);
				leftFlag = leftTile.isSolid();
				rightFlag = rightTile.isSolid();
				if (index == distance) {
					endingLeft = leftFlag;
					endingRight = rightFlag;
				} else {
					if (leftFlag != priorLeftFlag)
						s.drawLeftEdge(index);
					if (rightFlag != priorRightFlag)
						s.drawRightEdge(index);
				}

				drawLeftCell(index, leftTile);
				drawRightCell(index, rightTile);
			}

			drawCell(index, pt + delta);
			priorLeftFlag = leftFlag;
			priorRightFlag = rightFlag;
		}

		if (!isDoor)
			drawCell(0, currentPos);

		map->getTileAt(currentPos + delta, &backTile);
		if (distance < 5 && isMonsterBlocking(currentPos + backDelta) && backTile.isDoor()) {
			map->getTileAt(currentPos + leftDelta, &leftTile);
			map->getTileAt(currentPos + rightDelta, &rightTile);

			drawLeftCell(distance + 1, leftTile);
			drawRightCell(distance + 1, rightTile);

			map->getTileAt(currentPos + leftDelta, &leftTile);
			if (!leftTile.isSolid())
				s.drawLeftEdge(distance);

			if (!rightTile.isSolid())
				s.drawRightEdge(distance);
		} else {
			if (endingLeft)
				s.drawLeftEdge(distance);
			if (endingRight)
				s.drawRightEdge(distance);
		}
	}

	Maps::DungeonWidget *widget = dynamic_cast<Maps::DungeonWidget *>(tile._widget);
	if (isDoor && widget) {
		widget->draw(s, 0);
	}
}

uint ViewportDungeon::distanceToOccupiedCell(const Point &delta) {
	Point d = delta;
	uint distance;
	for (distance = 1; !isCellOccupied(d); ++distance, d.x += delta.x, d.y += delta.y) {}

	return MIN(distance, (uint)5);
}

bool ViewportDungeon::isCellOccupied(const Point &delta) {
	Maps::Map *map = getGame()->getMap();
	Point pt = map->getPosition() + delta;
	
	Maps::MapTile tile;
	map->getTileAt(pt, &tile);
	if (tile.isWallOrDoorway())
		return true;

	return isMonsterBlocking(pt);
}

bool ViewportDungeon::isMonsterBlocking(const Point &pt) {
	Maps::MapTile tile;
	getGame()->getMap()->getTileAt(pt, &tile);
	Maps::DungeonCreature *monster = dynamic_cast<Maps::DungeonCreature *>(tile._widget);
	return monster != nullptr && monster->isBlockingView();
}

void ViewportDungeon::drawCell(uint distance, const Point &pt) {
	Game *game = getGame();
	DungeonSurface s = getSurface();
	Maps::Map *map = game->getMap();

	Maps::MapTile tile;
	map->getTileAt(pt, &tile);

	// TODO: This currently contains dungeon cell types specific to Ultima 1
	Maps::DungeonCreature *monster = dynamic_cast<Maps::DungeonCreature *>(tile._widget);
	if (monster) {
		// Draw a monster
		if (tile.isWallOrDoorway())
			s.drawWall(distance);
		if (tile._tileId == 7)
			// Ladder down
			s.drawWidget(27, distance + 1, game->_edgeColor);
		if (tile._tileId == 6)
			// Ladder up
			s.drawWidget(26, distance + 1, game->_edgeColor);

		monster->draw(s, distance);
	} else {
		switch (tile._tileId) {
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
			if (map->getDirection() == Maps::DIR_UP || map->getDirection() == Maps::DIR_DOWN) {
				s.drawLadderDownFaceOn(distance + 1);
			} else {
				s.drawLadderDownSideOn(distance + 1);
			}
			break;
		case 7:
			// Ladder up
			if (map->getDirection() == Maps::DIR_UP || map->getDirection() == Maps::DIR_DOWN) {
				s.drawLadderUpFaceOn(distance + 1);
			} else {
				s.drawLadderUpSideOn(distance + 1);
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

	// Draw any item at that distance
	Maps::DungeonWidget *widget = dynamic_cast<Maps::DungeonWidget *>(tile._widget);
	if (widget)
		widget->draw(s, distance);
}

void ViewportDungeon::drawLeftCell(uint distance, const Maps::MapTile &tile) {
	DungeonSurface s = getSurface();

	if (tile.isDoor())
		s.drawLeftDoor(distance);
	else if (tile.isWallOrSecretDoor())
		s.drawLeftWall(distance);
	else
		s.drawLeftBlank(distance);
}

void ViewportDungeon::drawRightCell(uint distance, const Maps::MapTile &tile) {
	DungeonSurface s = getSurface();

	if (tile.isDoor())
		s.drawRightDoor(distance);
	else if (tile.isWallOrSecretDoor())
		s.drawRightWall(distance);
	else
		s.drawRightBlank(distance);
}

} // End of namespace Shared
} // End of namespace Ultima
