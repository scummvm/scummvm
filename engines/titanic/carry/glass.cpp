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

#include "titanic/carry/glass.h"
#include "titanic/carry/chicken.h"
#include "titanic/game/sauce_dispensor.h"
#include "titanic/npcs/character.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CGlass, CCarry)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MouseDragEndMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

CGlass::CGlass() : CCarry(), _string6("None") {
}

void CGlass::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string6, indent);
	CCarry::save(file, indent);
}

void CGlass::load(SimpleFile *file) {
	file->readNumber();
	_string6 = file->readString();
	CCarry::load(file);
}

bool CGlass::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CSauceDispensor *dispensor = dynamic_cast<CSauceDispensor *>(msg->_other);
	CChicken *chicken = dynamic_cast<CChicken *>(msg->_other);

	if (dispensor && _string6 != "None") {
		CUse useMsg(this);
		useMsg.execute(dispensor);
	} else if (msg->_other->isEquals("Chicken") && _string6 != "None") {
		if (chicken->_string6 != "None") {
			if (!chicken->_field12C) {
				CActMsg actMsg(_string6);
				actMsg.execute("Chicken");
			}

			_string6 = "None";
			loadFrame(0);
			_visibleFrame = 0;
		}

		petAddToInventory();
	} else if (msg->_other->isEquals("Napkin") && _string6 != "None") {
		petAddToInventory();
		_string6 = "None";
		loadFrame(0);
		_visibleFrame = 0;
	} else {
		petAddToInventory();
	}

	return true;
}

bool CGlass::UseWithCharMsg(CUseWithCharMsg *msg) {
	if (msg->_character->isEquals("Barbot") && msg->_character->_visible) {
		CActMsg actMsg(_string6);
		setVisible(false);

		if (_string6 != "Bird")
			setPosition(_origPos);

		actMsg.execute(msg->_character);
	} else {
		petAddToInventory();
	}

	return true;
}

bool CGlass::ActMsg(CActMsg *msg) {
	if (msg->_action == "GoToPET") {
		setVisible(true);
		petAddToInventory();
	} else if (msg->_action == "Mustard") {
		_string6 = "Mustard";
		loadFrame(1);
		_visibleFrame = 1;
	} else if (msg->_action == "Tomato") {
		_string6 = "Tomato";
		loadFrame(2);
		_visibleFrame = 2;
	} else if (msg->_action == "Bird") {
		_string6 = "Bird";
		loadFrame(3);
		_visibleFrame = 3;
	} else if (msg->_action == "InTitilator") {
		_string6 = "None";
		loadFrame(0);
		_visibleFrame = 0;
	}

	return true;
}

bool CGlass::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	showMouse();
	if (msg->_dropTarget) {
		error("TODO: See what drop target is");
		CCharacter *npc = dynamic_cast<CCharacter *>(msg->_dropTarget);
		if (npc) {
			CUseWithCharMsg useMsg(npc);
			useMsg.execute(this);
		} else {
			CUseWithOtherMsg otherMsg(npc);
			otherMsg.execute(this);
		}
	} else if (compareViewNameTo(_fullViewName) && msg->_mousePos.y < 360) {
		setPosition(_origPos);
	} else {
		petAddToInventory();
	}

	return true;
}

bool CGlass::TurnOn(CTurnOn *msg) {
	setVisible(true);
	return true;
}

bool CGlass::TurnOff(CTurnOff *msg) {
	setVisible(false);
	return true;
}

} // End of namespace Titanic
