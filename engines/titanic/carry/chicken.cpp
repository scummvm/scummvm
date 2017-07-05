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
	ON_MESSAGE(UseWithOtherMsg)
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

int CChicken::_temperature;

#define HOT_TEMPERATURE 120

CChicken::CChicken() : CCarry(), _condiment("None"),
		_greasy(true), _inactive(false), _timerId(0) {
}

void CChicken::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_greasy, indent);
	file->writeQuotedLine(_condiment, indent);
	file->writeNumberLine(_temperature, indent);
	file->writeNumberLine(_inactive, indent);
	file->writeNumberLine(_timerId, indent);

	CCarry::save(file, indent);
}

void CChicken::load(SimpleFile *file) {
	file->readNumber();
	_greasy = file->readNumber();
	_condiment = file->readString();
	_temperature = file->readNumber();
	_inactive = file->readNumber();
	_timerId = file->readNumber();

	CCarry::load(file);
}

bool CChicken::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	if (msg->_other->getName() == "Napkin") {
		if (_greasy || _condiment != "None") {
			CActMsg actMsg("Clean");
			actMsg.execute(this);
			petAddToInventory();
		} else {
			CShowTextMsg textMsg(CHICKEN_ALREADY_CLEAN);
			textMsg.execute("PET");
		}

		petAddToInventory();
	} else {
		CSauceDispensor *dispensor = dynamic_cast<CSauceDispensor *>(msg->_other);
		if (dispensor && _condiment == "None") {
			setVisible(false);
			CUse use(this);
			use.execute(msg->_other);
		} else {
			return CCarry::UseWithOtherMsg(msg);
		}
	}

	return true;
}

bool CChicken::UseWithCharMsg(CUseWithCharMsg *msg) {
	CSuccUBus *succubus = dynamic_cast<CSuccUBus *>(msg->_character);
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
		_condiment = "Tomato";
		loadFrame(4);
		_visibleFrame = 4;
	} else if (msg->_action == "Mustard") {
		_condiment = "Mustard";
		loadFrame(5);
		_visibleFrame = 5;
	} else if (msg->_action == "Bird") {
		_condiment = "Bird";
		loadFrame(2);
		_visibleFrame = 2;
	} else if (msg->_action == "None") {
		setVisible(false);
	} else if (msg->_action == "Clean") {
		_condiment = "None";
		loadFrame(3);
		_greasy = false;
		_visibleFrame = 3;
	} else if (msg->_action == "Dispense Chicken") {
		_condiment = "None";
		_inactive = false;
		_greasy = true;
		loadFrame(1);
		_visibleFrame = 1;
		_temperature = HOT_TEMPERATURE;
	} else if (msg->_action == "Hot") {
		_temperature = HOT_TEMPERATURE;
	} else if (msg->_action == "Eaten") {
		setVisible(false);
		petMoveToHiddenRoom();
		_inactive = true;
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
	// Check whether chicken has been mailed
	CGameObject *obj = getMailManFirstObject();
	while (obj && obj->getName() != "Chicken")
		obj = getMailManNextObject(obj);

	// If chicken is not mailed, and still hot, cool it down a bit
	if (!obj && _temperature > 0)
		--_temperature;

	// If chicken is cooled, refresh the inventory, just in case
	// the user had the hot chicken selected
	if (!_temperature) {
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
	if (_temperature > 0)
		msg->_isHot = true;

	if (_condiment == "Tomato") {
		msg->_condiment = 1;
	} else if (_condiment == "Mustard") {
		msg->_condiment = 2;
	} else if (_condiment == "Bird") {
		msg->_condiment = 3;
	}

	return true;
}

bool CChicken::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	if (_inactive) {
		showMouse();
		return true;
	} else {
		return CCarry::MouseDragEndMsg(msg);
	}
}

bool CChicken::PETObjectStateMsg(CPETObjectStateMsg *msg) {
	if (_temperature > 0)
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
