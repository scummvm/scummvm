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

#include "titanic/moves/move_player_to.h"
#include "titanic/game_manager.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMovePlayerTo, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

CMovePlayerTo::CMovePlayerTo() : CGameObject() {
}

void CMovePlayerTo::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_destination, indent);

	CGameObject::save(file, indent);
}

void CMovePlayerTo::load(SimpleFile *file) {
	file->readNumber();
	_destination = file->readString();

	CGameObject::load(file);
}

bool CMovePlayerTo::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CGameManager *gameManager = getGameManager();
	if (gameManager)
		changeView(_destination);

	return true;
}

bool CMovePlayerTo::ActMsg(CActMsg *msg) {
	_destination = msg->_action;
	return true;
}

} // End of namespace Titanic
