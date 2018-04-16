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

#include "titanic/moves/enter_exit_first_class_state.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEnterExitFirstClassState, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

void CEnterExitFirstClassState::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(g_vm->_stateRoomExitView, indent);
	CGameObject::save(file, indent);
}

void CEnterExitFirstClassState::load(SimpleFile *file) {
	file->readNumber();
	g_vm->_stateRoomExitView = file->readString();
	CGameObject::load(file);
}

bool CEnterExitFirstClassState::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	switch (getPassengerClass()) {
	case 1:
		if (compareRoomNameTo("1stClassLobby")) {
			// Entering room, so save where you were and change to stateroom
			g_vm->_stateRoomExitView = getRoomNodeName() + ".E";
			changeView("1stClassState.Node 1.S");
		} else if (compareRoomNameTo("1stClassState")) {
			// Return to where you entered room from
			changeView(g_vm->_stateRoomExitView);
		}
		break;

	case 2:
		petDisplayMessage(1, ROOM_RESERVED_FOR_FIRST_CLASS);
		break;

	default:
		petDisplayMessage(NO_LOSERS);
		break;
	}

	return true;
}

} // End of namespace Titanic
