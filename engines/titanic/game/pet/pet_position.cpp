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

#include "titanic/game/pet/pet_position.h"
#include "titanic/core/view_item.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPETPosition, CGameObject)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

void CPETPosition::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CPETPosition::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CPETPosition::EnterRoomMsg(CEnterRoomMsg *msg) {
	if (msg->_newRoom->getName() == "EmbLobby") {
		CPetControl *pet = getPetControl();
		if (pet)
			pet->setRoomsFloorNum(1);
	}

	return true;
}

bool CPETPosition::EnterViewMsg(CEnterViewMsg *msg) {
	// TODO
	return true;
}

bool CPETPosition::LeaveViewMsg(CLeaveViewMsg *msg) {
	CPetControl *pet = getPetControl();
	CString oldView = msg->_oldView->getName();
	CString newView = msg->_newView->getName();

	if (pet && newView == "Lift.Node 1.N") {
		int elevatorNum = pet->getRoomsElevatorNum();

		if (oldView == "TopOfWell.Node 25.N") {
			pet->setRoomsFloorNum(1);
			pet->setRoomsElevatorNum(1);
			return true;
		} else if (oldView == "TopOfWell.Node 24.SE") {
			pet->setRoomsFloorNum(1);
			pet->setRoomsElevatorNum(2);
			return true;
		} else if (oldView == "TopOfWell.Node 26.N") {
			pet->setRoomsFloorNum(1);
			pet->setRoomsElevatorNum(3);
			return true;
		} else if (oldView == "TopOfWell.Node 27.N") {
			pet->setRoomsFloorNum(1);
			pet->setRoomsElevatorNum(4);
			return true;
		} else if (oldView == "1stClassLobby.Node 10.S") {
			switch (elevatorNum) {
			case 1:
				pet->setRoomsFloorNum(1);
				pet->setRoomsElevatorNum(1);
				break;
			case 2:
				pet->setRoomsElevatorNum(1);
				break;
			default:
				pet->setRoomsElevatorNum(3);
				break;
			}
			return true;
		} else if (oldView == "1stClassLobby.Node 9.S") {
			switch (elevatorNum) {
			case 1:
			case 2:
				pet->setRoomsElevatorNum(2);
				break;
			default:
				pet->setRoomsElevatorNum(4);
				break;
			}
			return true;
		} else if (oldView == "2ndClassLobby.Node 8.S") {
			switch (elevatorNum) {
			case 1:
			case 2:
				pet->setRoomsElevatorNum(1);
				break;
			default:
				pet->setRoomsElevatorNum(3);
				break;
			}
			return true;
		} else if (oldView == "2ndClassLobby.Node 1.S") {
			switch (elevatorNum) {
			case 1:
			case 2:
				pet->setRoomsElevatorNum(2);
				break;
			default:
				pet->setRoomsElevatorNum(4);
				break;
			}
			return true;
		} else if (oldView == "SgtLobby.Node 1.S") {
			return true;
		} else if (oldView == "BottomOfWell.Node 10.E" || oldView == "BottomOfWell.Node 11.W") {
			pet->setRoomsElevatorNum(1);
			return true;
		}
	}

	CRoomItem *newRoom = msg->_newView->findRoom();
	if (newRoom) {
		CString roomName = newRoom->getName();
		if (roomName == "1stClassLobby" || roomName == "2ndClassLobby" || roomName == "SgtLobby") {
			if (pet)
				pet->resetRoomsHighlight();
		}
	}

	return true;
}


} // End of namespace Titanic
