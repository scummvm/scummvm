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

#include "titanic/npcs/maitre_d.h"
#include "titanic/core/room_item.h"
#include "titanic/sound/music_room.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMaitreD, CTrueTalkNPC)
	ON_MESSAGE(RestaurantMusicChanged)
	ON_MESSAGE(TrueTalkTriggerActionMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(NPCPlayTalkingAnimationMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(TrueTalkNotifySpeechStartedMsg)
	ON_MESSAGE(TrueTalkNotifySpeechEndedMsg)
	ON_MESSAGE(LoadSuccessMsg)
	ON_MESSAGE(TextInputMsg)
	ON_MESSAGE(TriggerNPCEvent)
END_MESSAGE_MAP()

int CMaitreD::_v1;

CMaitreD::CMaitreD() : CTrueTalkNPC(),
	_string2("z#40.wav"), _string3("z#40.wav"), _field108(0), _field118(1),
	_field11C(0), _field12C(0), _field130(1), _field134(0), _timerId(0) {
}

void CMaitreD::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field108, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);

	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_field134, indent);
	file->writeNumberLine(_timerId, indent);

	CTrueTalkNPC::save(file, indent);
}

void CMaitreD::load(SimpleFile *file) {
	file->readNumber();
	_field108 = file->readNumber();
	_string2 = file->readString();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_string3 = file->readString();
	_field12C = file->readNumber();
	_field130 = file->readNumber();

	_v1 = file->readNumber();
	_field134 = file->readNumber();
	_timerId = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CMaitreD::RestaurantMusicChanged(CRestaurantMusicChanged *msg) {
	if (msg->_value.empty()) {
		_field118 = 0;
	} else {
		_string3 = msg->_value;
		_field118 = _field11C = 1;
	}

	return true;
}

bool CMaitreD::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	if (msg->_action == 8) {
		_field12C = 1;
		stopAnimTimer(_timerId);
		_timerId = startAnimTimer("MD Fight", 3500, 0);
	} else if (msg->_action == 9) {
		stopAnimTimer(_timerId);
		_timerId = 0;
	} else if (msg->_action == 10) {
		_field12C = 0;
		_v1 = 1;
		stopAnimTimer(_timerId);
		_timerId = 0;

		CMaitreDDefeatedMsg defeatedMsg;
		defeatedMsg.execute(findRoom());
	}

	return true;
}

bool CMaitreD::EnterViewMsg(CEnterViewMsg *msg) {
	endTalking(this, true, findView());
	_field12C = _field134;

	if (_string3 == "STMusic" && (!_field11C || _string2 == _string3))
		return true;

	if (_string3.contains("nasty ambient"))
		startTalking(this, 111, findView());
	else if (!CMusicRoom::_musicHandler->checkSound(1))
		startTalking(this, 114, findView());
	else if (!CMusicRoom::_musicHandler->checkSound(3))
		startTalking(this, 113, findView());
	else if (!CMusicRoom::_musicHandler->checkSound(2))
		startTalking(this, 115, findView());
	else {
		startTalking(this, 110, findView());
		CMaitreDHappyMsg happyMsg;
		happyMsg.execute("MaitreD Left Arm");
		happyMsg.execute("MaitreD Right Arm");
	}

	return true;
}

bool CMaitreD::LeaveViewMsg(CLeaveViewMsg *msg) {
	_field134 = _field12C;
	performAction(true);
	stopAnimTimer(_timerId);
	_timerId = 0;

	_field12C = 0;
	return true;
}

bool CMaitreD::NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg) {
	static const char *const NAMES[] = {
		"Talking0", "Talking1", "Talking2", "Talking3", "Talking4",
		"Talking5", "Talking6", "Talking7", nullptr
	};

	if (msg->_value2 != 2) {
		msg->_names = NAMES;

		CAnimateMaitreDMsg animMsg;
		if (_field12C)
			animMsg._value = 0;
		animMsg.execute(this);
	}

	return true;
}

bool CMaitreD::TimerMsg(CTimerMsg *msg) {
	if (msg->_action == "MD Fight") {
		if (_field12C && compareViewNameTo("1stClassRestaurant.MaitreD Node.N")) {
			startTalking(this, 131, findView());
		}
	} else {
		CTrueTalkNPC::TimerMsg(msg);
	}

	return true;
}

bool CMaitreD::TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg) {
	if (_field12C) {
		stopAnimTimer(_timerId);
		_timerId = 0;
	}

	CTrueTalkNPC::TrueTalkNotifySpeechStartedMsg(msg);
	return true;
}

bool CMaitreD::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	if (_field12C) {
		stopAnimTimer(_timerId);
		_timerId = startAnimTimer("MD Fight", 3000 + getRandomNumber(3000));
	}

	CTrueTalkNPC::TrueTalkNotifySpeechEndedMsg(msg);
	return true;
}

bool CMaitreD::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_field12C) {
		_timerId = startAnimTimer("MD Fight", 3000 + getRandomNumber(3000));
	}

	return true;
}

bool CMaitreD::TextInputMsg(CTextInputMsg *msg) {
	CTrueTalkNPC::processInput(msg, findView());
	return true;
}

bool CMaitreD::TriggerNPCEvent(CTriggerNPCEvent *msg) {
	startTalking(this, msg->_value, findView());
	return true;
}

} // End of namespace Titanic
