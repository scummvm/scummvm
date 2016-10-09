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

#include "titanic/npcs/deskbot.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

static const char *const TALKING_NAMES[] = {
	"NeutralTalking", "HandFidget1", "HandFidget2", "LookingAround",
	"FriendlyTalking", "MoreRudeness", "HandUp", "TapFingers",
	"WaveOn", "WaveArmsAround", "HandsOverEdge"
};

static const char *const IDLE_NAMES[] = {
	"WaveOn", "HandFidget1", "HandFidget2", "TapFingers", "HandsOverEdge"
};

BEGIN_MESSAGE_MAP(CDeskbot, CTrueTalkNPC)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(TrueTalkTriggerActionMsg)
	ON_MESSAGE(NPCPlayTalkingAnimationMsg)
	ON_MESSAGE(NPCPlayIdleAnimationMsg)
	ON_MESSAGE(TrueTalkNotifySpeechStartedMsg)
	ON_MESSAGE(TrueTalkNotifySpeechEndedMsg)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

int CDeskbot::_v1;
int CDeskbot::_v2;

CDeskbot::CDeskbot() : CTrueTalkNPC(), _deskbotActive(false), _classNum(0) {
}

void CDeskbot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_deskbotActive, indent);
	file->writeNumberLine(_classNum, indent);

	CTrueTalkNPC::save(file, indent);
}

void CDeskbot::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_deskbotActive = file->readNumber();
	_classNum = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CDeskbot::TurnOn(CTurnOn *msg) {
	if (!_deskbotActive) {
		setVisible(true);
		playClip("BellRinging", 4);
		playSound("b#69.wav");
		petSetArea(PET_CONVERSATION);

		_npcFlags |= NPCFLAG_20000;
		_deskbotActive = true;
	}

	return true;
}

bool CDeskbot::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(false);
	_deskbotActive = false;
	_fieldC4 = 0;
	loadFrame(625);

	return true;
}

bool CDeskbot::ActMsg(CActMsg *msg) {
	if (msg->_action == "2ndClassUpgrade" && getPassengerClass() > 2) {
		startTalking(this, 140, findView());
	}

	return true;
}

bool CDeskbot::MovieEndMsg(CMovieEndMsg *msg) {
	bool flag = false;
	if (_npcFlags & NPCFLAG_10000) {
		if (_classNum) {
			petSetArea(PET_ROOMS);
			dec54();
			unlockMouse();
			playSound("z#47.wav");
			_classNum = false;
		}

		_npcFlags &= ~NPCFLAG_10000;
		flag = true;
	}

	if (_npcFlags & NPCFLAG_40000) {
		_deskbotActive = false;
		_npcFlags &= ~(NPCFLAG_40000 | NPCFLAG_20000);

		if (_npcFlags & NPCFLAG_80000) {
			CTurnOn turnOn;
			turnOn.execute("EmbBellbotTrigger");
			unlockMouse();
			changeView("EmbLobby.Node 4.N", "");
		} else if (_npcFlags & NPCFLAG_100000) {
			CTurnOn turnOn;
			turnOn.execute("EmbDoorBotTrigger");
			unlockMouse();
			changeView("EmbLobby.Node 4.N", "");
		}

		_npcFlags &= ~(NPCFLAG_80000 | NPCFLAG_100000);
		flag = true;
	}

	if (_npcFlags & NPCFLAG_20000) {
		_npcFlags &= ~(NPCFLAG_40000 | NPCFLAG_20000);
		endTalking(this, 1, findView());

		_npcFlags |= NPCFLAG_4;
		flag = true;
	}

	if (!flag)
		CTrueTalkNPC::MovieEndMsg(msg);

	return true;
}

bool CDeskbot::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_deskbotActive) {
		CTurnOff turnOff;
		turnOff.execute(this);
	}

	return true;
}

bool CDeskbot::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	switch (msg->_action) {
	case 19:
		inc54();
		lockMouse();
		petSetArea(PET_CONVERSATION);
		playClip("ReprogramPETInHand", 4);
		_npcFlags |= NPCFLAG_10000;
		_classNum = msg->_param1;

		switch (_classNum) {
		case 1:
			petDisplayMessage(UPGRADED_TO_FIRST_CLASS);
			setPassengerClass(_classNum);
			petReassignRoom(_classNum);
			break;
		case 2:
			petDisplayMessage(UPGRADED_TO_SECOND_CLASS);
			setPassengerClass(_classNum);
			petReassignRoom(_classNum);
			break;
		case 3:
			setPassengerClass(3);
			petReassignRoom(_classNum);
			break;
		default:
			break;
		}

	case 20:
		if (getPassengerClass() == 1) {
			CPetControl *petControl = getPetControl();
			if (petControl)
				petControl->changeLocationClass(4);
		}
		break;

	case 21:
		if (getPassengerClass() == 1) {
			CPetControl *petControl = getPetControl();
			if (petControl)
				petControl->changeLocationClass(3);
		}
		break;

	case 22:
		if (getPassengerClass() == 1) {
			CPetControl *petControl = getPetControl();
			if (petControl)
				petControl->changeLocationClass(2);
		}
		break;

	case 23:
		if (getPassengerClass() == 1) {
			CPetControl *petControl = getPetControl();
			if (petControl)
				petControl->changeLocationClass(1);
		}
		break;

	case 26:
		_npcFlags |= NPCFLAG_80000;
		CTurnOff turnOff;
		turnOff.execute(this);
		lockMouse();
		break;
	}

	return true;
}

bool CDeskbot::NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg) {
	if (msg->_value2 != 2)
		msg->_names = TALKING_NAMES;

	return true;
}

bool CDeskbot::NPCPlayIdleAnimationMsg(CNPCPlayIdleAnimationMsg *msg) {
	msg->_names = IDLE_NAMES;
	return true;
}

bool CDeskbot::TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg) {
	if (_npcFlags & NPCFLAG_40000)
		return true;

	CTrueTalkNPC::TrueTalkNotifySpeechStartedMsg(msg);
	switch (msg->_dialogueId) {
	case 41684:
	case 41686:
	case 41787:
	case 41788:
	case 41789:
		lockMouse();
		break;
	default:
		break;
	}

	return true;
}

bool CDeskbot::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	if (_npcFlags & NPCFLAG_40000)
		return true;

	CTurnOff turnOff;
	CTrueTalkNPC::TrueTalkNotifySpeechEndedMsg(msg);

	switch (msg->_dialogueId) {
	case 41684:
	case 41787:
	case 41788:
	case 41789:
		_npcFlags |= NPCFLAG_80000;
		turnOff.execute(this);

	case 41686:
		_npcFlags |= NPCFLAG_100000;
		turnOff.execute(this);
		break;

	default:
		break;
	}

	return true;
}

bool CDeskbot::TurnOff(CTurnOff *msg) {
	if (_deskbotActive) {
		stopMovie();
		performAction(1, findView());

		_npcFlags = (_npcFlags & ~(NPCFLAG_SPEAKING | NPCFLAG_2 | NPCFLAG_4)) | NPCFLAG_40000;
		playClip("Closing", 0x14);
	}

	return true;
}

} // End of namespace Titanic
