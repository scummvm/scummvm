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
#include "titanic/translation.h"

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

CCarryParrot::CCarryParrot() : CCarry(), _parrotName("PerchedParrot"),
		_timerId(0), _freeCounter(0), _feathersFlag(false) {
}

void CCarryParrot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_parrotName, indent);
	file->writeNumberLine(_timerId, indent);
	file->writeNumberLine(_freeCounter, indent);
	file->writeNumberLine(_feathersFlag, indent);

	CCarry::save(file, indent);
}

void CCarryParrot::load(SimpleFile *file) {
	file->readNumber();
	_parrotName = file->readString();
	_timerId = file->readNumber();
	_freeCounter = file->readNumber();
	_feathersFlag = file->readNumber();

	CCarry::load(file);
}

bool CCarryParrot::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	CParrot::_state = PARROT_4;
	CActMsg actMsg("Shut");
	actMsg.execute("ParrotCage");

	return true;
}

bool CCarryParrot::TimerMsg(CTimerMsg *msg) {
	if (CParrot::_state == PARROT_1 || CParrot::_state == PARROT_4) {
		if (++_freeCounter >= 30) {
			CActMsg actMsg("FreeParrot");
			actMsg.execute(this);
		}
	}

	return true;
}

bool CCarryParrot::IsParrotPresentMsg(CIsParrotPresentMsg *msg) {
	msg->_isPresent = true;
	return true;
}

bool CCarryParrot::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_visible) {
		setVisible(false);
		_canTake = false;
		CParrot::_state = PARROT_ESCAPED;
	}

	return true;
}

bool CCarryParrot::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	stopMovie();

	if (msg->_mousePos.y >= 360) {
		petAddToInventory();
	} else if (compareViewNameTo("ParrotLobby.Node 1.N")) {
		if (msg->_mousePos.x >= 75 && msg->_mousePos.x <= 565 &&
				!CParrot::_takeOff && !CCage::_open) {
			setVisible(false);
			_canTake = false;
			CTreeItem *perchedParrot = findUnder(getRoot(), "PerchedParrot");
			detach();
			addUnder(perchedParrot);
			stopSoundChannel(true);

			CPutParrotBackMsg backMsg(msg->_mousePos.x);
			backMsg.execute(perchedParrot);
		} else {
			setVisible(false);
			_canTake = false;
			CParrot::_state = PARROT_ESCAPED;
			playSound(TRANSLATE("z#475.wav", "z#212.wav"));
			stopSoundChannel(true);
			moveUnder(findRoom());

			CActMsg actMsg("Shut");
			actMsg.execute("ParrotCage");
		}
	} else {
		CCharacter *character = dynamic_cast<CCharacter *>(msg->_dropTarget);
		if (character) {
			CUseWithCharMsg charMsg(character);
			charMsg.execute(this, nullptr, 0);
		} else {
			setVisible(false);
			_canTake = false;
			playSound(TRANSLATE("z#475.wav", "z#212.wav"));
			stopSoundChannel(true);
			moveUnder(findRoom());
		}
	}

	showMouse();
	return true;
}

bool CCarryParrot::PassOnDragStartMsg(CPassOnDragStartMsg *msg) {
	if (CParrot::_state != PARROT_MAILED) {
		moveToView();
		setPosition(Point(0, 0));
		setVisible(true);
		playClip("Pick Up", MOVIE_STOP_PREVIOUS);
		playClip("Flapping", MOVIE_REPEAT);

		stopTimer(_timerId);
		_timerId = addTimer(1000, 1000);

		_freeCounter = 0;
		CParrot::_state = PARROT_1;
		msg->_value3 = 1;

		return CCarry::PassOnDragStartMsg(msg);
	}

	CTrueTalkNPC *npc = dynamic_cast<CTrueTalkNPC *>(getRoot()->findByName(_parrotName));
	if (npc)
		startTalking(npc, 0x446BF);

	_canTake = false;
	CProximity prox(Audio::Mixer::kSpeechSoundType);
	playSound(TRANSLATE("z#475.wav", "z#212.wav"), prox);
	moveUnder(findRoom());
	CParrot::_state = PARROT_ESCAPED;

	msg->_value4 = 1;
	return true;
}

bool CCarryParrot::PreEnterViewMsg(CPreEnterViewMsg *msg) {
	loadSurface();
	CCarryParrot *parrot = dynamic_cast<CCarryParrot *>(getRoot()->findByName("CarryParrot"));
	if (parrot)
		parrot->_canTake = false;

	return true;
}

bool CCarryParrot::UseWithCharMsg(CUseWithCharMsg *msg) {
	CSuccUBus *succubus = dynamic_cast<CSuccUBus *>(msg->_character);
	if (succubus)
		CParrot::_state = PARROT_MAILED;

	return CCarry::UseWithCharMsg(msg);
}

bool CCarryParrot::ActMsg(CActMsg *msg) {
	if (msg->_action == "FreeParrot" && (CParrot::_state == PARROT_4 || CParrot::_state == PARROT_1)) {
		CTrueTalkNPC *npc = dynamic_cast<CTrueTalkNPC *>(getRoot()->findByName(_parrotName));
		if (npc)
			startTalking(npc, 0x446BF);

		setVisible(false);
		_canTake = false;

		if (CParrot::_state == PARROT_4) {
			playSound(TRANSLATE("z#475.wav", "z#212.wav"));

			if (!_feathersFlag) {
				CCarry *feathers = dynamic_cast<CCarry *>(getRoot()->findByName("Feathers"));
				if (feathers) {
					feathers->setVisible(true);
					feathers->petAddToInventory();
				}

				_feathersFlag = true;
			}

			CPetControl *pet = getPetControl();
			pet->removeFromInventory(this);
			pet->setAreaChangeType(1);
			moveUnder(getRoom());
		} else {
			CActMsg actMsg("Shut");
			actMsg.execute("ParrotCage");
		}

		CParrot::_state = PARROT_ESCAPED;
		stopAnimTimer(_timerId);
		_timerId = 0;
	}

	return true;
}

} // End of namespace Titanic
