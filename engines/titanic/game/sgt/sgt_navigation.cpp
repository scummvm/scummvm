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

#include "titanic/game/sgt/sgt_navigation.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSGTNavigation, CGameObject)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CSGTNavigationStatics *CSGTNavigation::_statics;

void CSGTNavigation::init() {
	_statics = new CSGTNavigationStatics();
}

void CSGTNavigation::deinit() {
	delete _statics;
}

void CSGTNavigation::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_statics->_miniLiftFloor, indent);
	file->writeQuotedLine(_statics->_destView, indent);
	file->writeQuotedLine(_statics->_destRoom, indent);

	CGameObject::save(file, indent);
}

void CSGTNavigation::load(SimpleFile *file) {
	file->readNumber();
	_statics->_miniLiftFloor = file->readNumber();
	_statics->_destView = file->readString();
	_statics->_destRoom = file->readString();

	CGameObject::load(file);
}

bool CSGTNavigation::StatusChangeMsg(CStatusChangeMsg *msg) {
	CPetControl *pet = getPetControl();

	if (isEquals("SGTLL")) {
		static const int FRAMES[7] = { 0, 149, 112, 74, 0, 36, 74 };
		_statics->_miniLiftFloor = msg->_newStatus;
		if (pet->getRoomsSublevel() != _statics->_miniLiftFloor) {
			changeView("SGTLittleLift.Node 1.N");
		}

		int startVal = pet->getRoomsSublevel();
		if (startVal > _statics->_miniLiftFloor)
			playMovie(FRAMES[startVal], FRAMES[_statics->_miniLiftFloor], MOVIE_WAIT_FOR_FINISH);
		else
			playMovie(FRAMES[startVal + 3], FRAMES[_statics->_miniLiftFloor + 3], MOVIE_WAIT_FOR_FINISH);

		_cursorId = _statics->_miniLiftFloor != 1 ? CURSOR_MOVE_FORWARD : CURSOR_INVALID;

		pet->setRoomsSublevel(_statics->_miniLiftFloor);
		pet->resetRoomsHighlight();
	}

	return true;
}

bool CSGTNavigation::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (compareRoomNameTo("SgtLobby")) {
		_statics->_destView = getRoomNodeName();
		_statics->_destRoom = "SgtLobby";
		changeView("SGTState.Node 1.S");
	} else if (compareRoomNameTo("SGTLittleLift")) {
		if (_statics->_miniLiftFloor != 1) {
			_statics->_destRoom = "SGTLittleLift";
			changeView("SGTState.Node 1.S");
		}
	} else if (compareRoomNameTo("SGTState")) {
		if (_statics->_destRoom == "SgtLobby") {
			if (compareViewNameTo("SGTState.Node 2.N")) {
				changeView("SGTState.Node 1.N");
				_statics->_destView += ".S";
			} else {
				_statics->_destView += ".N";
			}

			changeView(_statics->_destView);
		} else if (_statics->_destRoom == "SGTLittleLift") {
			if (compareViewNameTo("SGTState.Node 1.S")) {
				changeView("SGTLittleLift.Node 1.N");
			} else {
				changeView("SGTState.Node 1.N");
				changeView("SGTLittleLift.Node 1.S");
			}
		}
	}

	return true;
}

bool CSGTNavigation::EnterViewMsg(CEnterViewMsg *msg) {
	if (isEquals("SGTLL")) {
		static const int FRAMES[3] = { 0, 36, 74 };
		CPetControl *pet = getPetControl();
		loadFrame(FRAMES[pet->getRoomsSublevel() - 1]);
	}

	return true;
}

} // End of namespace Titanic
