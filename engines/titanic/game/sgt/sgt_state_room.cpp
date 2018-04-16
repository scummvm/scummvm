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
#include "titanic/translation.h"

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
	_statics->_bedhead = "Closed";
}

void CSGTStateRoom::deinit() {
	delete _statics;
}

CSGTStateRoom::CSGTStateRoom() : CBackground(), _isClosed(1),
	_displayFlag(true), _savedFrame(0), _savedIsClosed(true), _savedVisible(true) {
}

void CSGTStateRoom::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_statics->_bedhead, indent);
	file->writeQuotedLine(_statics->_bedfoot, indent);
	file->writeQuotedLine(_statics->_vase, indent);
	file->writeQuotedLine(_statics->_tv, indent);
	file->writeQuotedLine(_statics->_desk, indent);
	file->writeQuotedLine(_statics->_chestOfDrawers, indent);
	file->writeQuotedLine(_statics->_drawer, indent);
	file->writeQuotedLine(_statics->_armchair, indent);
	file->writeQuotedLine(_statics->_deskchair, indent);
	file->writeQuotedLine(_statics->_washstand, indent);
	file->writeQuotedLine(_statics->_basin, indent);
	file->writeQuotedLine(_statics->_toilet, indent);

	file->writeNumberLine(_isClosed, indent);
	file->writeNumberLine(_displayFlag, indent);
	file->writeNumberLine(_statics->_announcementFlag, indent);
	file->writeNumberLine(_statics->_roomFlags, indent);
	file->writeNumberLine(_savedFrame, indent);
	file->writeNumberLine(_savedIsClosed, indent);
	file->writeNumberLine(_savedVisible, indent);

	CBackground::save(file, indent);
}

void CSGTStateRoom::load(SimpleFile *file) {
	file->readNumber();
	_statics->_bedhead = file->readString();
	_statics->_bedfoot = file->readString();
	_statics->_vase = file->readString();
	_statics->_tv = file->readString();
	_statics->_desk = file->readString();
	_statics->_chestOfDrawers = file->readString();
	_statics->_drawer = file->readString();
	_statics->_armchair = file->readString();
	_statics->_deskchair = file->readString();
	_statics->_washstand = file->readString();
	_statics->_basin = file->readString();
	_statics->_toilet = file->readString();

	_isClosed = file->readNumber();
	_displayFlag = file->readNumber();
	_statics->_announcementFlag = file->readNumber();
	_statics->_roomFlags = file->readNumber();
	_savedFrame = file->readNumber();
	_savedIsClosed = file->readNumber();
	_savedVisible = file->readNumber();

	CBackground::load(file);
}

bool CSGTStateRoom::ActMsg(CActMsg *msg) {
	CPetControl *pet = getPetControl();
	uint roomFlags = pet->getRoomFlags();
	uint assignedRoom = pet->getAssignedRoomFlags();

	if (roomFlags != assignedRoom) {
		petDisplayMessage(NOT_YOUR_ASSIGNED_ROOM);
	} else if (_isClosed) {
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

	// WORKAROUND: Correctly show SGT furniture states in assigned stateroom
	// even when the user has already upgraded to 2nd or 1st class
	if (pet->isInAssignedRoom()) {
		loadFrame(_savedFrame);
		_isClosed = _savedIsClosed;
		setVisible(_savedVisible);

		if (isEquals("Desk") && _statics->_desk == "Closed")
			loadFrame(1);
	}

	if (isEquals("Drawer")) {
		petSetArea(PET_REMOTE);
		if (pet->isInAssignedRoom() && getPassengerClass() == 3
				&& _statics->_announcementFlag) {
			// Congratulations, you may have won an upgrade
			playSound(TRANSLATE("b#21.wav", "b#2.wav"));
			_statics->_announcementFlag = false;
		}

		_statics->_drawer = "Closed";
		setVisible(false);
		_isClosed = true;
	} else if (!pet->isInAssignedRoom()) {
		loadFrame(0);
		if (_displayFlag) {
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
		_savedFrame = getMovieFrame();
		_savedIsClosed = _isClosed;
		_savedVisible = _visible;
	}

	_statics->_roomFlags = roomFlags;
	return true;
}

} // End of namespace Titanic
