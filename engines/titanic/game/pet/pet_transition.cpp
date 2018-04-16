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

#include "titanic/game/pet/pet_transition.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/core/view_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPETTransition, CGameObject)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

void CPETTransition::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CPETTransition::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CPETTransition::EnterViewMsg(CEnterViewMsg *msg) {
	CPetControl *pet = getPetControl();

	if (compareRoomNameTo("1stClassLobby") && pet) {
		int elevatorNum = pet->getRoomsElevatorNum();
		CString nodeView = msg->_newView->getNodeViewName();

		if (nodeView == "Node 1.E") {
			pet->setRoomsElevatorNum((elevatorNum == 1 || elevatorNum == 2) ? 1 : 3);
		} else if (nodeView == "Node 1.W") {
			pet->setRoomsElevatorNum((elevatorNum == 1 || elevatorNum == 2) ? 2 : 4);
		}
	}

	return true;
}

} // End of namespace Titanic
