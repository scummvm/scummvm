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

#include "ultima/ultima1/actions/move.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace Actions {

BEGIN_MESSAGE_MAP(Move, Action)
	ON_MESSAGE(MoveMsg)
END_MESSAGE_MAP()

bool Move::MoveMsg(CMoveMsg &msg) {
	Maps::Ultima1Map *map = getMap();
	
	if (map->_mapType == Maps::MAP_DUNGEON) {
		switch (msg._direction) {
		case Shared::Maps::DIR_LEFT:
			dungeonTurnLeft();
			break;
		case Shared::Maps::DIR_RIGHT:
			dungeonTurnRight();
			break;
		case Shared::Maps::DIR_DOWN:
			dungeonTurnAround();
			break;
		case Shared::Maps::DIR_UP:
			dungeonMoveForward();
			break;
		}
	} else {
		Shared::Maps::MapWidget *player = map->getPlayerWidget();
		assert(player);

		// Figure out the new position
		Point delta;
		switch (msg._direction) {
		case Shared::Maps::DIR_WEST:
			delta = Point(-1, 0);
			break;
		case Shared::Maps::DIR_EAST:
			delta = Point(1, 0);
			break;
		case Shared::Maps::DIR_NORTH:
			delta = Point(0, -1);
			break;
		case Shared::Maps::DIR_SOUTH:
			delta = Point(0, 1);
			break;
		}

		// Check if the player's widget type can move to the new position
		Point newPos = map->getDeltaPosition(delta);
		if (player->canMoveTo(newPos) == Shared::Maps::MapWidget::YES) {
			// Shift the viewport
			map->shiftViewport(delta);

			// Move to the new position
			player->moveTo(newPos);
			addInfoMsg(getRes()->DIRECTION_NAMES[msg._direction - 1]);
		} else {
			// Nope, so show a blocked message
			addInfoMsg(getRes()->BLOCKED);
			playFX(1);
		}
	}

	endOfTurn();
	return true;
}

void Move::dungeonTurnLeft() {
	Maps::Ultima1Map *map = getMap();

	switch (map->getDirection()) {
	case Shared::Maps::DIR_LEFT:
		map->setDirection(Shared::Maps::DIR_DOWN);
		break;
	case Shared::Maps::DIR_RIGHT:
		map->setDirection(Shared::Maps::DIR_UP);
		break;
	case Shared::Maps::DIR_DOWN:
		map->setDirection(Shared::Maps::DIR_RIGHT);
		break;
	case Shared::Maps::DIR_UP:
		map->setDirection(Shared::Maps::DIR_LEFT);
		break;
	default:
		break;
	}

	addInfoMsg(getGame()->_res->DUNGEON_MOVES[Shared::Maps::DIR_LEFT - 1]);
}

void Move::dungeonTurnRight() {
	Maps::Ultima1Map *map = getMap();

	switch (map->getDirection()) {
	case Shared::Maps::DIR_LEFT:
		map->setDirection(Shared::Maps::DIR_UP);
		break;
	case Shared::Maps::DIR_RIGHT:
		map->setDirection(Shared::Maps::DIR_DOWN);
		break;
	case Shared::Maps::DIR_DOWN:
		map->setDirection(Shared::Maps::DIR_LEFT);
		break;
	case Shared::Maps::DIR_UP:
		map->setDirection(Shared::Maps::DIR_RIGHT);
		break;
	default:
		break;
	}

	addInfoMsg(getGame()->_res->DUNGEON_MOVES[Shared::Maps::DIR_RIGHT - 1]);
}

void Move::dungeonTurnAround() {
	Maps::Ultima1Map *map = getMap();

	switch (map->getDirection()) {
	case Shared::Maps::DIR_LEFT:
		map->setDirection(Shared::Maps::DIR_RIGHT);
		break;
	case Shared::Maps::DIR_RIGHT:
		map->setDirection(Shared::Maps::DIR_LEFT);
		break;
	case Shared::Maps::DIR_DOWN:
		map->setDirection(Shared::Maps::DIR_UP);
		break;
	case Shared::Maps::DIR_UP:
		map->setDirection(Shared::Maps::DIR_DOWN);
		break;
	default:
		break;
	}

	addInfoMsg(getGame()->_res->DUNGEON_MOVES[Shared::Maps::DIR_DOWN - 1]);
}

void Move::dungeonMoveForward() {
	Maps::Ultima1Map *map = getMap();
	Point delta = map->getDirectionDelta();
	Shared::Maps::MapWidget *player = map->getPlayerWidget();
	assert(player);

	if (player->canMoveTo(map->getPosition() + delta) != Shared::Maps::MapWidget::NO) {
		map->setPosition(map->getPosition() + delta);
	} else {
		playFX(0);
	}

	addInfoMsg(getGame()->_res->DUNGEON_MOVES[Shared::Maps::DIR_UP - 1]);
}

} // End of namespace Actions
} // End of namespace Ultima1
} // End of namespace Ultima
