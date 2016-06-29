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

#include "titanic/carry/chicken.h"
#include "titanic/game/sauce_dispensor.h"
#include "titanic/npcs/succubus.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CChicken, CCarry)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(VisibleMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(PETGainedObjectMsg)
	ON_MESSAGE(ParrotTriesChickenMsg)
	ON_MESSAGE(MouseDragEndMsg)
	ON_MESSAGE(PETObjectStateMsg)
	ON_MESSAGE(PETLostObjectMsg)
END_MESSAGE_MAP()

int CChicken::_v1;

CChicken::CChicken() : CCarry(), _string6("None"),
		_field12C(1), _field13C(0), _timerId(0) {
}

void CChicken::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeQuotedLine(_string6, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeNumberLine(_timerId, indent);

	CCarry::save(file, indent);
}

void CChicken::load(SimpleFile *file) {
	file->readNumber();
	_field12C = file->readNumber();
	_string6 = file->readString();
	_v1 = file->readNumber();
	_field13C = file->readNumber();
	_timerId = file->readNumber();

	CCarry::load(file);
}

bool CChicken::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	if (msg->_other->getName() == "Napkin") {
		if (_field12C || _string6 == "None") {
			CActMsg actMsg("Clean");
			actMsg.execute(this);
			petAddToInventory();
		} else {
			CShowTextMsg textMsg("The chicken is already clean.");
			textMsg.execute("PET");
		}

		petAddToInventory();
	} else {
		CSauceDispensor *dispensor = static_cast<CSauceDispensor *>(msg->_other);
		if (!dispensor || _string6 == "None") {
			return CCarry::UseWithOtherMsg(msg);
		} else {
			setVisible(false);
			CUse use(this);
			use.execute(msg->_other);
		}
	}

	return true;
}

bool CChicken::UseWithCharMsg(CUseWithCharMsg *msg) {
	CSuccUBus *succubus = static_cast<CSuccUBus *>(msg->_character);
	if (succubus) {
		setPosition(Point(330, 300));
		CSubAcceptCCarryMsg acceptMsg;
		acceptMsg._item = this;
		acceptMsg.execute(succubus);
	} else {
		petAddToInventory();
	}

	return true;
}

bool CChicken::ActMsg(CActMsg *msg) {
	if (msg->_action == "GoToPET") {
		setVisible(true);
		petAddToInventory();
	} else if (msg->_action == "Tomato") {
		_string6 = "Tomato";
		loadFrame(4);
		_visibleFrame = 4;
	} else if (msg->_action == "Mustard") {
		_string6 = "Mustard";
		loadFrame(5);
		_visibleFrame = 5;
	} else if (msg->_action == "Bird") {
		_string6 = "Bird";
		loadFrame(2);
		_visibleFrame = 2;
	} else if (msg->_action == "None") {
		setVisible(false);
	} else if (msg->_action == "Clean") {
		_string6 = "None";
		loadFrame(3);
		_field12C = 0;
		_visibleFrame = 3;
	}
	else if (msg->_action == "Dispense Chicken") {
		_string6 = "None";
		_field13C = 0;
		_field12C = 1;
		loadFrame(1);
		_visibleFrame = 1;
		_v1 = 120;
	} else if (msg->_action == "Hot") {
		_v1 = 120;
	} else if (msg->_action == "Eaten") {
		setVisible(false);
		petMoveToHiddenRoom();
		_field13C = 1;
	}

	return true;
}

bool CChicken::VisibleMsg(CVisibleMsg *msg) {
	setVisible(msg->_visible);
	if (msg->_visible)
		loadFrame(_visibleFrame);

	return true;
}

bool CChicken::TimerMsg(CTimerMsg *msg) {
	CGameObject *obj = getMailManFirstObject();
	while (obj && obj->getName() != "Chicken")
		obj = getMailManNextObject(obj);

	bool flag = false;
	if (obj) {
		flag = _v1;
	} else if (_v1 > 0) {
		--_v1;
		flag = _v1;
	}

	if (flag) {
		petInvChange();
		stopTimer(_timerId);
	}

	return true;
}

bool CChicken::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	stopTimer(_timerId);
	_timerId = addTimer(1000, 1000);
	return true;
}

bool CChicken::ParrotTriesChickenMsg(CParrotTriesChickenMsg *msg) {
	if (_v1 > 0)
		msg->_value1 = 1;

	if (_string6 == "Tomato") {
		msg->_value2 = 1;
	} else if (_string6 == "Mustard") {
		msg->_value2 = 2;
	} else if (_string6 == "Bird") {
		msg->_value2 = 3;
	}

	return true;
}

bool CChicken::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	if (_field13C)
		return true;
	else
		return CCarry::MouseDragEndMsg(msg);
}

bool CChicken::PETObjectStateMsg(CPETObjectStateMsg *msg) {
	if (_v1 > 0)
		msg->_value = 2;

	return true;
}

bool CChicken::PETLostObjectMsg(CPETLostObjectMsg *msg) {
	if (compareViewNameTo("ParrotLobby.Node 1.N")) {
		CActMsg actMsg("StartChickenDrag");
		actMsg.execute("PerchedParrot");
	}

	return true;
}

} // End of namespace Titanic
