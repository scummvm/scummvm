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

#include "titanic/game/pet/pet_lift.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPETLift, CPETTransport)
	ON_MESSAGE(TransportMsg)
END_MESSAGE_MAP()

void CPETLift::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CPETTransport::save(file, indent);
}

void CPETLift::load(SimpleFile *file) {
	file->readNumber();
	CPETTransport::load(file);
}

bool CPETLift::TransportMsg(CTransportMsg *msg) {
	CPetControl *pet = getPetControl();
	if (msg->_value1 != 1)
		return false;

	int floorNum = -1;
	if (msg->_roomName == "TopOfWell") {
		floorNum = 1;
	} else if (msg->_roomName == "BottomOfWell") {
		floorNum = 39;
	} else if (msg->_roomName == "PlayersRoom" && pet) {
		floorNum = pet->getAssignedFloorNum();
		if (floorNum < 1 || floorNum > 39) {
			pet->petDisplayMessage(NO_ROOM_ASSIGNED);
			floorNum = -1;
		}
	}

	if (floorNum != -1) {
		int elevatorNum = pet ? pet->getRoomsElevatorNum() : 0;

		if ((elevatorNum == 2 || elevatorNum == 4) && floorNum > 27) {
			petDisplayMessage(ELEVATOR_NOT_BELOW_27);
		} else {
			CTrueTalkTriggerActionMsg triggerMsg(2, floorNum, 0);
			triggerMsg.execute("Liftbot");
		}
	}

	return true;
}

} // End of namespace Titanic
