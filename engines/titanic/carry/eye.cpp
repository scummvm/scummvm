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

#include "titanic/carry/eye.h"
#include "titanic/game/head_slot.h"
#include "titanic/game/light.h"
#include "titanic/game/television.h"
#include "titanic/game/transport/lift.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEye, CHeadPiece)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(PETGainedObjectMsg)
	ON_MESSAGE(PassOnDragStartMsg)
END_MESSAGE_MAP()

CEye::CEye() : CHeadPiece(), _eyeFlag(false) {
}

void CEye::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_eyeFlag, indent);
	CHeadPiece::save(file, indent);
}

void CEye::load(SimpleFile *file) {
	file->readNumber();
	_eyeFlag = file->readNumber();
	CHeadPiece::load(file);
}


bool CEye::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CHeadSlot *slot = dynamic_cast<CHeadSlot *>(msg->_other);
	if (slot) {
		petMoveToHiddenRoom();
		_flag = true;
		CAddHeadPieceMsg headMsg(getName());

		if (headMsg._value != "NULL")
			headMsg.execute(isEquals("Eye1") ? "Eye1Slot" : "Eye2Slot");
	} else if (msg->_other->isEquals("LiftbotWithoutHead")) {
		CPetControl *pet = getPetControl();
		if (!CLift::_hasHead && pet->getRoomsElevatorNum() == 4) {
			_eyeFlag = true;
			setPosition(_origPos);
			setVisible(false);
			CActMsg actMsg1(getName());
			actMsg1.execute("GetLiftEye");

			CTelevision::_eyeFlag = true;
			CActMsg actMsg2("AddWrongHead");
			actMsg2.execute("FaultyLiftbot");
		}
	} else {
		return CCarry::UseWithOtherMsg(msg);
	}

	return true;
}

bool CEye::UseWithCharMsg(CUseWithCharMsg *msg) {
	CLift *lift = dynamic_cast<CLift *>(msg->_character);
	if (lift && lift->getName() == "Well") {
		CPetControl *pet = getPetControl();
		if (!CLift::_hasHead && pet->getRoomsElevatorNum() == 4) {
			_eyeFlag = true;
			setPosition(_origPos);
			setVisible(false);

			CActMsg actMsg1(getName());
			actMsg1.execute("GetLiftEye");
			CActMsg actMsg2("AddWrongHead");
			actMsg2.execute(msg->_character);
		}

		return true;
	} else {
		return CHeadPiece::UseWithCharMsg(msg);
	}
}

bool CEye::ActMsg(CActMsg *msg) {
	if (msg->_action == "BellbotGetLight") {
		setVisible(true);
		petAddToInventory();
		playSound(TRANSLATE("z#47.wav", "z#578.wav"));

		CActMsg actMsg("Eye Removed");
		actMsg.execute("1stClassState", CLight::_type,
			MSGFLAG_CLASS_DEF | MSGFLAG_SCAN);
	} else {
		_eyeFlag = false;

		CActMsg actMsg("LoseHead");
		actMsg.execute("FaultyLiftbot");
	}

	return true;
}

bool CEye::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	if (isEquals("Eye1"))
		CTelevision::_channel4Glyph = false;

	return CHeadPiece::PETGainedObjectMsg(msg);
}

bool CEye::PassOnDragStartMsg(CPassOnDragStartMsg *msg) {
	setVisible(true);
	if (_eyeFlag)
		CTelevision::_eyeFlag = false;
	else if (isEquals("Eye1"))
		CTelevision::_channel4Glyph = false;

	return CHeadPiece::PassOnDragStartMsg(msg);
}

} // End of namespace Titanic
