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

#include "titanic/game/get_lift_eye2.h"
#include "titanic/game/transport/lift.h"
#include "titanic/core/project_item.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CGetLiftEye2, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(VisibleMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

CString *CGetLiftEye2::_destObject;

void CGetLiftEye2::init() {
	_destObject = new CString();
}

void CGetLiftEye2::deinit() {
	delete _destObject;
}

void CGetLiftEye2::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(*_destObject, indent);
	CGameObject::save(file, indent);
}

void CGetLiftEye2::load(SimpleFile *file) {
	file->readNumber();
	*_destObject = file->readString();
	CGameObject::load(file);
}

bool CGetLiftEye2::ActMsg(CActMsg *msg) {
	*_destObject = msg->_action;
	setVisible(true);
	_cursorId = CURSOR_HAND;
	return true;
}

bool CGetLiftEye2::EnterRoomMsg(CEnterRoomMsg *msg) {
	CPetControl *pet = getPetControl();
	if (pet->getRoomsElevatorNum() == 4 && CLift::_hasHead && !CLift::_hasCorrectHead) {
		_cursorId = CURSOR_HAND;
		setVisible(true);
	} else {
		_cursorId = CURSOR_ARROW;
		setVisible(false);
	}

	return true;
}

bool CGetLiftEye2::VisibleMsg(CVisibleMsg *msg) {
	setVisible(true);
	_cursorId = CURSOR_HAND;
	return true;
}

bool CGetLiftEye2::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (checkPoint(msg->_mousePos, false, true)) {
		_cursorId = CURSOR_ARROW;
		setVisible(false);
		CActMsg actMsg("EyeNotHead");
		actMsg.execute(*_destObject);
		CPassOnDragStartMsg dragMsg(msg->_mousePos, 1);
		dragMsg.execute(*_destObject);

		msg->_dragItem = getRoot()->findByName(*_destObject);
		*_destObject = "NULL";
		return true;
	} else {
		return false;
	}
}

} // End of namespace Titanic
