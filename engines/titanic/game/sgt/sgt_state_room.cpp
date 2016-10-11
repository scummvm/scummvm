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

#include "titanic/game/sgt/sgt_state_room.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSGTStateRoom, CBackground)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(VisibleMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(LeaveRoomMsg)
END_MESSAGE_MAP()

CSGTStateRoomStatics *CSGTStateRoom::_statics;

void CSGTStateRoom::init() {
	_statics = new CSGTStateRoomStatics();
	_statics->_v1 = "Closed";
}

void CSGTStateRoom::deinit() {
	delete _statics;
}

CSGTStateRoom::CSGTStateRoom() : CBackground(), _fieldE0(1),
	_fieldE4(1), _fieldE8(0), _fieldEC(1), _fieldF0(1) {
}

void CSGTStateRoom::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_statics->_v1, indent);
	file->writeQuotedLine(_statics->_v2, indent);
	file->writeQuotedLine(_statics->_v3, indent);
	file->writeQuotedLine(_statics->_v4, indent);
	file->writeQuotedLine(_statics->_v5, indent);
	file->writeQuotedLine(_statics->_v6, indent);
	file->writeQuotedLine(_statics->_v7, indent);
	file->writeQuotedLine(_statics->_v8, indent);
	file->writeQuotedLine(_statics->_v9, indent);
	file->writeQuotedLine(_statics->_v10, indent);
	file->writeQuotedLine(_statics->_v11, indent);
	file->writeQuotedLine(_statics->_v12, indent);

	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_statics->_v13, indent);
	file->writeNumberLine(_statics->_v14, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_fieldEC, indent);
	file->writeNumberLine(_fieldF0, indent);

	CBackground::save(file, indent);
}

void CSGTStateRoom::load(SimpleFile *file) {
	file->readNumber();
	_statics->_v1 = file->readString();
	_statics->_v2 = file->readString();
	_statics->_v3 = file->readString();
	_statics->_v4 = file->readString();
	_statics->_v5 = file->readString();
	_statics->_v6 = file->readString();
	_statics->_v7 = file->readString();
	_statics->_v8 = file->readString();
	_statics->_v9 = file->readString();
	_statics->_v10 = file->readString();
	_statics->_v11 = file->readString();
	_statics->_v12 = file->readString();

	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_statics->_v13 = file->readNumber();
	_statics->_v14 = file->readNumber();
	_fieldE8 = file->readNumber();
	_fieldEC = file->readNumber();
	_fieldF0 = file->readNumber();

	CBackground::load(file);
}

bool CSGTStateRoom::ActMsg(CActMsg *msg) {
	CPetControl *pet = getPetControl();
	uint roomFlags = pet->getRoomFlags();
	uint assignedRoom = pet->getAssignedRoomFlags();

	if (roomFlags != assignedRoom) {
		petDisplayMessage(NOT_YOUR_ASSIGNED_ROOM);
	} else if (_fieldE0) {
		CTurnOn onMsg;
		onMsg.execute(this);
	} else {
		CTurnOff offMsg;
		offMsg.execute(this);
	}

	return true;
}

bool CSGTStateRoom::VisibleMsg(CVisibleMsg *msg) {
	setVisible(msg->_visible);
	return true;
}

bool CSGTStateRoom::EnterRoomMsg(CEnterRoomMsg *msg) {
	CPetControl *pet = getPetControl();
	uint roomFlags = pet->getRoomFlags();
	uint assignedRoom = pet->getAssignedRoomFlags();

	if (roomFlags == assignedRoom) {
		loadFrame(_fieldE8);
		_fieldE0 = _fieldEC;
		setVisible(_fieldF0);

		if (isEquals("Desk") && _statics->_v5 == "Closed")
			loadFrame(1);
	}

	if (isEquals("Drawer")) {
		petSetArea(PET_REMOTE);
		if (roomFlags == assignedRoom && getPassengerClass() == 3
				&& _statics->_v13) {
			playSound("b#21.wav");
			_statics->_v13 = 0;
		}

		_statics->_v7 = "Closed";
		setVisible(false);
		_fieldE0 = true;
	} else if (roomFlags != assignedRoom) {
		loadFrame(0);
		if (_fieldE4) {
			setVisible(true);
			if (isEquals("Desk"))
				loadFrame(1);
		} else {
			setVisible(false);
		}
	}

	return true;
}

bool CSGTStateRoom::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	CPetControl *pet = getPetControl();
	uint roomFlags = pet->getRoomFlags();
	uint assignedRoom = pet->getAssignedRoomFlags();

	if (roomFlags == assignedRoom) {
		_fieldE8 = getMovieFrame();
		_fieldEC = _fieldE0;
		_fieldF0 = _visible;
	}

	_statics->_v14 = roomFlags;
	return true;
}

} // End of namespace Titanic
