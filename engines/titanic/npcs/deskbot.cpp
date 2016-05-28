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

namespace Titanic {

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

CDeskbot::CDeskbot() : CTrueTalkNPC(), _deskbotActive(false), _field10C(0) {
}

void CDeskbot::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_deskbotActive, indent);
	file->writeNumberLine(_field10C, indent);

	CTrueTalkNPC::save(file, indent);
}

void CDeskbot::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_deskbotActive = file->readNumber();
	_field10C = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CDeskbot::TurnOn(CTurnOn *msg) {
	if (!_deskbotActive) {
		setVisible(true);
		playClip("BellRinging", 4);
		playSound("b#69.wav");
		setPetArea(PET_CONVERSATION);

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
		if (_field10C) {
			setPetArea(PET_ROOMS);
			dec54();
			unlockMouse();
			playSound("z#47.wav", 100, 0, 0);
			_field10C = false;
		}

		_npcFlags &= ~NPCFLAG_10000;
		flag = true;
	}

	bool flag = false;
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
	// TODO
	return true;
}

bool CDeskbot::NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::NPCPlayIdleAnimationMsg(CNPCPlayIdleAnimationMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	// TODO
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
