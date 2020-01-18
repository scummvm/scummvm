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
	Point newPos = map->getPosition() + delta;
	if (transport->canMoveTo(newPos)) {
		// Shift the viewport
		map->shiftViewport(delta);

		// Move to the new position
		if (transport->moveTo(newPos))
			addStatusMsg(getRes()->DIRECTION_NAMES[msg._direction - 1]);
	} else {
		// Nope, so show a blocked message
		addStatusMsg(getRes()->BLOCKED);
	}

	return true;
}

} // End of namespace Actions
} // End of namespace Ultima1
} // End of namespace Ultima
