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

#include "titanic/npcs/liftbot.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLiftBot, CTrueTalkNPC)
	ON_MESSAGE(TextInputMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(TrueTalkTriggerActionMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(TrueTalkGetStateValueMsg)
	ON_MESSAGE(NPCPlayTalkingAnimationMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

bool CLiftBot::_flag;
bool CLiftBot::_enabled;

CLiftBot::CLiftBot() : CTrueTalkNPC(), _field108(1) {
}

void CLiftBot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_enabled, indent);

	CTrueTalkNPC::save(file, indent);
}

void CLiftBot::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	_field108 = file->readNumber();
	_enabled = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CLiftBot::TextInputMsg(CTextInputMsg *msg) {
	CPetControl *pet = getPetControl();
	if (_enabled || pet->getRoomsElevatorNum() != 4) {
		if (getName() == "LiftBot") {
			CViewItem *view = findView();
			processInput(msg, view);
		}
	}

	return true;
}

bool CLiftBot::EnterViewMsg(CEnterViewMsg *msg) {
	CPetControl *pet = getPetControl();
	if (!_enabled && pet->getRoomsElevatorNum() == 4) {
		loadFrame(700);
	} else if (!_flag && getName() == "LiftBot") {
		// First time meeting the LiftBot
		CViewItem *view = findView();
		setTalking(this, true, view);
		petSetArea(PET_CONVERSATION);
		_flag = 1;
	}

	return true;
}

bool CLiftBot::EnterRoomMsg(CEnterRoomMsg *msg) {
	_flag = 0;
	changeView("Lift.Node 1.W", "");
	return true;
}

bool CLiftBot::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	if (msg->_action == 2 && msg->_param1 != _field108) {
		CStatusChangeMsg statusMsg(_field108, msg->_param1, false);
		statusMsg.execute("Well");

		_field108 = msg->_param1;
	}

	return true;
}

bool CLiftBot::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	if (getName() == "LiftBot")
		performAction(false);

	return true;
}

bool CLiftBot::TurnOff(CTurnOff *msg) {
	_enabled = false;
	return true;
}

bool CLiftBot::TurnOn(CTurnOn *msg) {
	_enabled = true;
	if (!_flag) {
		if (isEquals("LiftBotTalking")) {
			setTalking(this, MOVIE_REPEAT, findView());
			petSetArea(PET_CONVERSATION);
			_flag = true;
		}
	}

	return true;
}

bool CLiftBot::LeaveViewMsg(CLeaveViewMsg *msg) {
	return true;
}

bool CLiftBot::TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg) {
	if (msg->_stateNum == 4) {
		CPetControl *pet = getPetControl();
		if (pet)
			msg->_stateVal = pet->getAssignedFloorNum();
	} else if (msg->_stateNum == 5) {
		msg->_stateVal = _field108;
	} else if (msg->_stateNum == 6) {
		CPetControl *pet = getPetControl();
		if (pet)
			msg->_stateVal = pet->getRoomsElevatorNum();
	} else {
		msg->_stateVal = _field108;
	}

	return true;
}

bool CLiftBot::NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg) {
	static const char *const NAMES[] = {
		"Groaning", "Groaning 2", "Talking 1", "Talking 2", "Talking 3",
		"Happy Talking", "Complaining", "Aggressive", "Explaining",
		"Happy Talking 2", "Happy Talking 3", "Happy Talking 4",
		"Confidential", nullptr
	};

	if (msg->_value2 == 2)
		playClip("At Rest");
	else
		msg->_names = NAMES;
	return true;
}

bool CLiftBot::ActMsg(CActMsg *msg) {
	if (msg->_action == "ActivateLift") {
		_enabled = true;
		CViewItem *view = findView();
		setTalking(this, true, view);
		startTalking(this, 155, view);
	} else if (msg->_action == "LiftArrive") {
		CViewItem *view = findView();
		startTalking(this, 156, view);
	}

	return true;
}

} // End of namespace Titanic
