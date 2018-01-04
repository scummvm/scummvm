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

#include "titanic/moves/move_player_in_parrot_room.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMovePlayerInParrotRoom, CMovePlayerTo)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MovementMsg)
END_MESSAGE_MAP()

CMovePlayerInParrotRoom::CMovePlayerInParrotRoom() : CMovePlayerTo() {
}

void CMovePlayerInParrotRoom::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CMovePlayerTo::save(file, indent);
}

void CMovePlayerInParrotRoom::load(SimpleFile *file) {
	file->readNumber();
	CMovePlayerTo::load(file);
}

bool CMovePlayerInParrotRoom::ActMsg(CActMsg *msg) {
	if (msg->_action == "PanAwayFromParrot") {
		unlockMouse();
		changeView(_destination);
	}

	return true;
}

bool CMovePlayerInParrotRoom::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	lockMouse();
	CPanningAwayFromParrotMsg awayMsg(this);
	awayMsg.execute("PerchedParrot");
	return true;
}

bool CMovePlayerInParrotRoom::MovementMsg(CMovementMsg *msg) {
	if (msg->_movement == TURN_RIGHT)
		msg->_posToUse = Point(600, 180);
	return false;
}

} // End of namespace Titanic
