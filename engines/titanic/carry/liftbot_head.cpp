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

#include "titanic/carry/liftbot_head.h"
#include "titanic/game/transport/lift.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLiftbotHead, CCarry)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

CLiftbotHead::CLiftbotHead() : CCarry(), _flag(false) {
}

void CLiftbotHead::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	CCarry::save(file, indent);
}

void CLiftbotHead::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	CCarry::load(file);
}

bool CLiftbotHead::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	if (msg->_other->getName() == "LiftbotWithoutHead") {
		CPetControl *pet = getPetControl();
		if (!CLift::_hasHead && pet->getRoomsElevatorNum() == 4) {
			_flag = true;
			CActMsg actMsg("AddRightHead");
			actMsg.execute("FaultyLiftbot");
			setVisible(false);
		} else {
			petAddToInventory();
		}

		return true;
	} else {
		return CCarry::UseWithOtherMsg(msg);
	}
}

bool CLiftbotHead::UseWithCharMsg(CUseWithCharMsg *msg) {
	CLift *lift = dynamic_cast<CLift *>(msg->_character);
	if (lift) {
		CPetControl *pet = getPetControl();
		if (lift->isEquals("Well") && !CLift::_hasHead && pet->getRoomsElevatorNum() == 4) {
			_flag = true;
			CActMsg actMsg("AddRightHead");
			actMsg.execute(lift);
			setVisible(false);

			return true;
		}
	}

	return CCarry::UseWithCharMsg(msg);
}

bool CLiftbotHead::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkStartDragging(msg)) {
		return false;
	} else if (compareViewNameTo("BottomOfWell.Node 8.N")) {
		changeView("BottomOfWell.Node 13.N");
		moveToView();

		CActMsg actMsg("LiftbotHeadTaken");
		actMsg.execute("BOWLiftbotHeadMonitor");

		return CCarry::MouseDragStartMsg(msg);
	} else if (_flag) {
		_flag = false;
		CActMsg actMsg("LoseHead");
		actMsg.execute("FaultyLiftbot");
	}

	return CCarry::MouseDragStartMsg(msg);
}

} // End of namespace Titanic
