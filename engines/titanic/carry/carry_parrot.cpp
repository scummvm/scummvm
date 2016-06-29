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

#include "titanic/carry/carry_parrot.h"
#include "titanic/core/project_item.h"
#include "titanic/core/room_item.h"
#include "titanic/game/cage.h"
#include "titanic/npcs/parrot.h"
#include "titanic/npcs/succubus.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCarryParrot, CCarry)
	ON_MESSAGE(PETGainedObjectMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(IsParrotPresentMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(MouseDragEndMsg)
	ON_MESSAGE(PassOnDragStartMsg)
	ON_MESSAGE(PreEnterViewMsg)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

CCarryParrot::CCarryParrot() : CCarry(), _string6("PerchedParrot"),
		_timerId(0), _field13C(0), _field140(false), _field144(10),
		_field148(25), _field14C(0), _field150(8) {
}

void CCarryParrot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string6, indent);
	file->writeNumberLine(_timerId, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeNumberLine(_field140, indent);

	CCarry::save(file, indent);
}

void CCarryParrot::load(SimpleFile *file) {
	file->readNumber();
	_string6 = file->readString();
	_timerId = file->readNumber();
	_field13C = file->readNumber();
	_field140 = file->readNumber();

	CCarry::load(file);
}

bool CCarryParrot::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	CParrot::_v4 = 4;
	CActMsg actMsg("Shut");
	actMsg.execute("ParrotCage");

	return true;
}

bool CCarryParrot::TimerMsg(CTimerMsg *msg) {
	if (CParrot::_v4 == 1 || CParrot::_v4 == 4) {
		if (++_field13C >= 30) {
			CActMsg actMsg("FreeParrot");
			actMsg.execute(this);
		}
	}

	return true;
}

bool CCarryParrot::IsParrotPresentMsg(CIsParrotPresentMsg *msg) {
	msg->_value = true;
	return true;
}

bool CCarryParrot::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_visible) {
		setVisible(false);
		_fieldE0 = 0;
		CParrot::_v4 = 2;
	}

	return true;
}

bool CCarryParrot::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	stopMovie();

	if (msg->_mousePos.y >= 360) {
		petAddToInventory();
		return true;
	}

	if (compareViewNameTo("ParrotLobby.Node 1.N")) {
		if (msg->_mousePos.x >= 75 && msg->_mousePos.x <= 565 &&
				!CParrot::_v2 && !CCage::_v2) {
			setVisible(false);
			_fieldE0 = 0;
			CTreeItem *perchedParrot = findUnder(getRoot(), "PerchedParrot");
			detach();
			addUnder(perchedParrot);
			sound8(true);

			CPutParrotBackMsg backMsg(msg->_mousePos.x);
			backMsg.execute(perchedParrot);
		} else {
			setVisible(false);
			_fieldE0 = 0;
			CParrot::_v4 = 2;
			playSound("z#475.wav", 100, 0, 0);
			sound8(true);
			moveUnder(findRoom());

			CActMsg actMsg("Shut");
			actMsg.execute("ParrotCage");
		}
	} else {
		CCharacter *character = static_cast<CCharacter *>(msg->_dropTarget);
		if (character) {
			CUseWithCharMsg charMsg(character);
			charMsg.execute(this, nullptr, 0);
		} else {
			setVisible(false);
			_fieldE0 = 0;
			playSound("z#475.wav", 100, 0, 0);
			sound8(true);
			moveUnder(findRoom());
		}
	}

	return true;
}

bool CCarryParrot::PassOnDragStartMsg(CPassOnDragStartMsg *msg) {
	if (CParrot::_v4 != 3) {
		moveToView();
		setPosition(Point(0, 0));
		setVisible(true);
		playClip("Pick Up", 2);
		playClip("Flapping", 1);

		stopTimer(_timerId);
		_timerId = addTimer(1000, 1000);

		_field13C = 0;
		CParrot::_v4 = 1;
		msg->_value3 = 1;

		return CCarry::PassOnDragStartMsg(msg);
	}

	CTrueTalkNPC *npc = static_cast<CTrueTalkNPC *>(getRoot()->findByName(_string6));
	if (npc)
		startTalking(npc, 0x446BF);

	_fieldE0 = 0;
	playSound("z#475.wav", 100, 0, 0);
	moveUnder(findRoom());
	msg->_value4 = 1;

	return true;
}

bool CCarryParrot::PreEnterViewMsg(CPreEnterViewMsg *msg) {
	loadSurface();
	CCarryParrot *parrot = static_cast<CCarryParrot *>(getRoot()->findByName("CarryParrot"));
	if (parrot)
		parrot->_fieldE0 = 0;

	return true;
}

bool CCarryParrot::UseWithCharMsg(CUseWithCharMsg *msg) {
	CSuccUBus *succubus = static_cast<CSuccUBus *>(msg->_character);
	if (succubus)
		CParrot::_v4 = 3;

	return CCarry::UseWithCharMsg(msg);
}

bool CCarryParrot::ActMsg(CActMsg *msg) {
	if (msg->_action == "FreeParrot" && (CParrot::_v4 == 4 || CParrot::_v4 == 1)) {
		CTrueTalkNPC *npc = static_cast<CTrueTalkNPC *>(getRoot()->findByName(_string6));
		if (npc)
			startTalking(npc, 0x446BF);

		setVisible(false);
		_fieldE0 = 0;

		if (CParrot::_v4 == 4) {
			CActMsg actMsg("Shut");
			actMsg.execute("ParrotCage");
		} else {
			playSound("z#475.wav", 100, 0, 0);

			if (!_field140) {
				CCarry *feathers = static_cast<CCarry *>(getRoot()->findByName("Feathers"));
				if (feathers) {
					feathers->setVisible(true);
					feathers->petAddToInventory();
				}

				_field140 = true;
			}

			getPetControl()->removeFromInventory(this);
			getPetControl()->setAreaChangeType(1);
			moveUnder(getRoom());
		}

		CParrot::_v4 = 2;
		stopTimer(_timerId);
		_timerId = 0;
	}

	return true;
}

} // End of namespace Titanic
