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

#include "titanic/moves/enter_exit_sec_class_mini_lift.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEnterExitSecClassMiniLift, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(StatusChangeMsg)
END_MESSAGE_MAP()

CEnterExitSecClassMiniLiftStatics *CEnterExitSecClassMiniLift::_statics;

void CEnterExitSecClassMiniLift::init() {
	_statics = new CEnterExitSecClassMiniLiftStatics();
}

void CEnterExitSecClassMiniLift::deinit() {
	delete _statics;
}

void CEnterExitSecClassMiniLift::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_statics->_viewName, indent);
	file->writeNumberLine(_statics->_state, indent);
	file->writeNumberLine(_roomNum, indent);

	CGameObject::save(file, indent);
}

void CEnterExitSecClassMiniLift::load(SimpleFile *file) {
	file->readNumber();
	_statics->_viewName = file->readString();
	_statics->_state = file->readNumber();
	_roomNum = file->readNumber();

	CGameObject::load(file);
}

bool CEnterExitSecClassMiniLift::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (compareRoomNameTo("2ndClassLobby")) {
		_statics->_viewName = getRoomNodeName() + ".W";
		changeView("SecClassLittleLift.Node 1.E");
		_statics->_state = 1;

		CPetControl *pet = getPetControl();
		if (pet) {
			pet->setRoomsRoomNum(_roomNum);
			pet->setRoomsSublevel(1);
		}
	} else if (compareRoomNameTo("SecClassLittleLift")) {
		if (_statics->_state == 1) {
			changeView(_statics->_viewName);
		}
	}

	return true;
}

bool CEnterExitSecClassMiniLift::StatusChangeMsg(CStatusChangeMsg *msg) {
	_statics->_state = msg->_newStatus;
	if (msg->_newStatus == 3)
		_statics->_state = 2;

	CPetControl *pet = getPetControl();
	if (pet)
		pet->setRoomsSublevel(_statics->_state);

	_cursorId = _statics->_state == 1 ? CURSOR_MOVE_FORWARD : CURSOR_INVALID;
	return true;
}

} // End of namespace Titanic
