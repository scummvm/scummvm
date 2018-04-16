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
	CPetControl *pet = getPetControl();
	CString viewStr = msg->_newView->getNodeViewName();
	CString tempStr;

	if (compareRoomNameTo("TopOfWell")) {
		tempStr = msg->_newView->getNodeViewName();
		int wellEntry = 0;

		if (tempStr == "Node 25.N")
			wellEntry = 1;
		else if (tempStr == "Node 24.SE")
			wellEntry = 2;
		else if (tempStr == "Node 26.N")
			wellEntry = 3;
		else if (tempStr == "Node 27.N")
			wellEntry = 4;

		if (wellEntry != 0)
			petSetRoomsWellEntry(wellEntry);
	} else if (compareRoomNameTo("1stClassLobby")) {
		int roomNum = 0;

		if (viewStr == "Node 2.N")
			roomNum = 1;
		else if (viewStr == "Node 3.N")
			roomNum = 2;
		else if (viewStr == "Node 4.N")
			roomNum = 3;
		else if (viewStr == "Node 5.N")
			roomNum = 1;
		else if (viewStr == "Node 6.N")
			roomNum = 2;
		else if (viewStr == "Node 7.N")
			roomNum = 3;

		if (pet) {
			pet->setRoomsRoomNum(roomNum);
			pet->resetRoomsHighlight();

			int elevatorNum = pet->getRoomsElevatorNum();
			int wellEntry = 0;

			if (viewStr == "Node 10.S")
				wellEntry = (elevatorNum == 1 || elevatorNum == 2) ? 1 : 3;
			else if (viewStr == "Node 9.S")
				wellEntry = (elevatorNum == 1 || elevatorNum == 2) ? 2 : 4;

			if (wellEntry)
				petSetRoomsWellEntry(wellEntry);
		}
	} else if (compareRoomNameTo("2ndClassLobby")) {
		int roomNum = 0;

		if (viewStr == "Node 3.N")
			roomNum = 1;
		else if (viewStr == "Node 4.N")
			roomNum = 2;
		else if (viewStr == "Node 5.N")
			roomNum = 3;
		else if (viewStr == "Node 6.N")
			roomNum = 4;

		if (pet) {
			pet->setRoomsRoomNum(roomNum);
			pet->resetRoomsHighlight();

			int elevatorNum = pet->getRoomsElevatorNum();
			int wellEntry = 0;

			if (viewStr == "Node 8.S")
				wellEntry = (elevatorNum == 1 || elevatorNum == 2) ? 1 : 3;
			else if (viewStr == "Node 1.S")
				wellEntry = (elevatorNum == 1 || elevatorNum == 2) ? 2 : 4;

			if (wellEntry)
				petSetRoomsWellEntry(wellEntry);
		}
	} else if (compareRoomNameTo("SecClassLittleLift")) {
		if (pet && viewStr == "Node 1.N")
			pet->resetRoomsHighlight();
	} else  if (compareRoomNameTo("SGTLittleLift")) {
		if (pet && viewStr == "Node 1.N")
			pet->resetRoomsHighlight();
	} else if (compareRoomNameTo("SgtLobby")) {
		int roomNum = 0;

		if (viewStr == "Node 4.N")
			roomNum = 1;
		else if (viewStr == "Node 5.N")
			roomNum = 2;
		else if (viewStr == "Node 6.N")
			roomNum = 3;
		else if (viewStr == "Node 7.N")
			roomNum = 4;
		else if (viewStr == "Node 8.N")
			roomNum = 5;
		else if (viewStr == "Node 9.N")
			roomNum = 6;

		if (pet) {
			pet->setRoomsSublevel(1);
			pet->setRoomsRoomNum(roomNum);
			pet->resetRoomsHighlight();

			if (viewStr == "Node 1.S")
				pet->setRoomsWellEntry(pet->getRoomsElevatorNum());
		}
	} else if (compareRoomNameTo("BottomOfWell")) {
		if (viewStr == "Node 10.E")
			petSetRoomsWellEntry(3);
		else if (viewStr == "Node 11.W")
			petSetRoomsWellEntry(1);
	}

	return true;
}

bool CPETPosition::LeaveViewMsg(CLeaveViewMsg *msg) {
	CPetControl *pet = getPetControl();
	CString oldView = msg->_oldView->getFullViewName();
	CString newView = msg->_newView->getFullViewName();

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
		} else if (oldView == "BottomOfWell.Node 10.E") {
			pet->setRoomsElevatorNum(3);
		} else if (oldView == "BottomOfWell.Node 11.W") {
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
