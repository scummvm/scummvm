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
#include "ultima/ultima1/core/map.h"
#include "ultima/ultima1/core/transports.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace Actions {

BEGIN_MESSAGE_MAP(Move, Action)
	ON_MESSAGE(MoveMsg)
END_MESSAGE_MAP()

bool Move::MoveMsg(CMoveMsg &msg) {
	Ultima1Map *map = getMap();
	
	if (map->_mapType == MAP_DUNGEON) {
		switch (msg._direction) {
		case Shared::DIR_LEFT:
			dungeonTurnLeft();
			break;
		case Shared::DIR_RIGHT:
			dungeonTurnRight();
			break;
		case Shared::DIR_DOWN:
			dungeonTurnAround();
			break;
		case Shared::DIR_UP:
			dungeonMoveForward();
			break;
		}
	} else {
		WidgetTransport *transport = map->_currentTransport;

		// Figure out the new position
		Point delta;
		switch (msg._direction) {
		case Shared::DIR_LEFT:
			delta = Point(-1, 0);
			break;
		case Shared::DIR_RIGHT:
			delta = Point(1, 0);
			break;
		case Shared::DIR_UP:
			delta = Point(0, -1);
			break;
		case Shared::DIR_DOWN:
			delta = Point(0, 1);
			break;
		}

		// Check if the given transport type can move to the new position
		Point newPos = map->getDeltaPosition(delta);
		if (transport->canMoveTo(newPos)) {
			// Shift the viewport
			map->shiftViewport(delta);

			// Move to the new position
			if (transport->moveTo(newPos))
				addStatusMsg(getRes()->DIRECTION_NAMES[msg._direction - 1]);
		} else {
			// Nope, so show a blocked message
			addStatusMsg(getRes()->BLOCKED);
			playFX(1);
		}
	}

	return true;
}

void Move::dungeonTurnLeft() {
	Ultima1Map *map = getMap();

	switch (map->_direction) {
	case Shared::DIR_LEFT:
		map->_direction = Shared::DIR_DOWN;
		break;
	case Shared::DIR_RIGHT:
		map->_direction = Shared::DIR_UP;
		break;
	case Shared::DIR_DOWN:
		map->_direction = Shared::DIR_RIGHT;
		break;
	case Shared::DIR_UP:
		map->_direction = Shared::DIR_LEFT;
		break;
	}
}

void Move::dungeonTurnRight() {
	Ultima1Map *map = getMap();

	switch (map->_direction) {
	case Shared::DIR_LEFT:
		map->_direction = Shared::DIR_UP;
		break;
	case Shared::DIR_RIGHT:
		map->_direction = Shared::DIR_DOWN;
		break;
	case Shared::DIR_DOWN:
		map->_direction = Shared::DIR_LEFT;
		break;
	case Shared::DIR_UP:
		map->_direction = Shared::DIR_RIGHT;
		break;
	}
}

void Move::dungeonTurnAround() {
	Ultima1Map *map = getMap();

	switch (map->_direction) {
	case Shared::DIR_LEFT:
		map->_direction = Shared::DIR_RIGHT;
		break;
	case Shared::DIR_RIGHT:
		map->_direction = Shared::DIR_LEFT;
		break;
	case Shared::DIR_DOWN:
		map->_direction = Shared::DIR_UP;
		break;
	case Shared::DIR_UP:
		map->_direction = Shared::DIR_DOWN;
		break;
	}
}

void Move::dungeonMoveForward() {
	Ultima1Map *map = getMap();
	WidgetTransport *transport = map->_currentTransport;
	Point delta = map->getDirectionDelta();

	if (transport->canMoveTo(map->getPosition() + delta)) {
		map->setPosition(map->getPosition() + delta);
	} else {
		playFX(0);
	}
}

} // End of namespace Actions
} // End of namespace Ultima1
} // End of namespace Ultima
