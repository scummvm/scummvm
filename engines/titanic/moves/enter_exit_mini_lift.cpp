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

#include "titanic/moves/enter_exit_mini_lift.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEnterExitMiniLift, CSGTNavigation)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

void CEnterExitMiniLift::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_destRoomNum, indent);

	CSGTNavigation::save(file, indent);
}

void CEnterExitMiniLift::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_destRoomNum = file->readNumber();

	CSGTNavigation::load(file);
}

bool CEnterExitMiniLift::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (compareRoomNameTo("SgtLobby")) {
		_statics->_destView = getRoomNodeName() + ".S";
		_statics->_destRoom = "SgtLobby";
		changeView("SGTLittleLift.Node 1.E");

		CPetControl *pet = getPetControl();
		if (pet)
			pet->setRoomsRoomNum(_destRoomNum);
	} else if (compareRoomNameTo("SGTLittleLift")) {
		if (_statics->_miniLiftFloor == 1) {
			changeView(_statics->_destView);
		}
	}

	return true;
}

bool CEnterExitMiniLift::EnterViewMsg(CEnterViewMsg *msg) {
	// Only show exit cursor when minilift is on the ground level
	_cursorId = _statics->_miniLiftFloor == 1 ? CURSOR_MOVE_FORWARD : CURSOR_INVALID;
	return true;
}

} // End of namespace Titanic
