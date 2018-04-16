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

#include "titanic/moves/enter_sec_class_state.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEnterSecClassState, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CEnterSecClassState::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_mode, indent);
	file->writeNumberLine(_soundHandle, indent);

	CGameObject::save(file, indent);
}

void CEnterSecClassState::load(SimpleFile *file) {
	file->readNumber();
	_mode = file->readNumber();
	_soundHandle = file->readNumber();

	CGameObject::load(file);
}

bool CEnterSecClassState::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (getPassengerClass() > 2) {
		playSound(TRANSLATE("b#105.wav", "b#84.wav"));
		petDisplayMessage(1, CLASS_NOT_PERMITTED_IN_AREA);
	} else if (!compareRoomNameTo("SecClassLittleLift") || _mode == 2)  {
		CActMsg actMsg(getFullViewName().deleteRight(2) + ".S");
		actMsg.execute("SecClassRoomLeaver");
		changeView("secClassState.Node 01.N");
	}

	return true;
}

bool CEnterSecClassState::StatusChangeMsg(CStatusChangeMsg *msg) {
	stopSound(_soundHandle);

	if (msg->_newStatus == _mode || (_mode == 2 && msg->_newStatus == 3)) {
		if (_mode == 2) {
			_soundHandle = queueSound(TRANSLATE("b#36.wav", "b#15.wav"), _soundHandle);
		} else {
			_soundHandle = queueSound(TRANSLATE("b#31.wav", "b#10.wav"), _soundHandle);
		}
		if (msg->_newStatus == 3)
			msg->_newStatus = 2;
	} else {
		changeView("SecClassLittleLift.Node 1.N");
		if (msg->_newStatus == 1) {
			_soundHandle = queueSound(TRANSLATE("b#32.wav", "b#11.wav"), _soundHandle);
		} else if (msg->_newStatus == 2) {
			_soundHandle = queueSound(TRANSLATE("b#25.wav", "b#4.wav"), _soundHandle);
		} else if (msg->_newStatus == 3) {
			_soundHandle = queueSound(TRANSLATE("b#33.wav", "b#12.wav"), _soundHandle);
			msg->_newStatus = 2;
		}
	}

	if (msg->_newStatus != 3) {
		if (msg->_newStatus == 2 && _mode == 1)
			playMovie(0, 10, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		else if (msg->_newStatus == 1)
			playMovie(11, 21, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	}

	_cursorId = msg->_newStatus == 2 ? CURSOR_MOVE_FORWARD : CURSOR_INVALID;
	_mode = msg->_newStatus;
	return true;
}

bool CEnterSecClassState::MovieEndMsg(CMovieEndMsg *msg) {
	CPetControl *pet = getPetControl();
	if (pet) {
		pet->setRoomsSublevel(_mode);
		pet->resetRoomsHighlight();
	}

	return true;
}

} // End of namespace Titanic
