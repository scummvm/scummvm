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

CMaitreD::CMaitreD() : CTrueTalkNPC(),
	_priorMusicName("z#40.wav"), _musicName("z#40.wav"), _unused5(0), _hasMusic(true),
	_musicSet(false), _fightFlag(false), _unused6(true), _savedFightFlag(false),
	_timerId(0), _defeated(false) {
}

void CMaitreD::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_unused5, indent);
	file->writeQuotedLine(_priorMusicName, indent);
	file->writeNumberLine(_hasMusic, indent);
	file->writeNumberLine(_musicSet, indent);
	file->writeQuotedLine(_musicName, indent);
	file->writeNumberLine(_fightFlag, indent);
	file->writeNumberLine(_unused6, indent);

	file->writeNumberLine(_defeated, indent);
	file->writeNumberLine(_savedFightFlag, indent);
	file->writeNumberLine(_timerId, indent);

	CTrueTalkNPC::save(file, indent);
}

void CMaitreD::load(SimpleFile *file) {
	file->readNumber();
	_unused5 = file->readNumber();
	_priorMusicName = file->readString();
	_hasMusic = file->readNumber();
	_musicSet = file->readNumber();
	_musicName = file->readString();
	_fightFlag = file->readNumber();
	_unused6 = file->readNumber();

	_defeated = file->readNumber();
	_savedFightFlag = file->readNumber();
	_timerId = file->readNumber();

	CTrueTalkNPC::load(file);

	// WORKAROUND: The back view of the MaitreD from close to the table is dodgy
	// in the original. And unneeded anyway, since he's also part of the background
	if (_name == "MaitreLoop03")
		_visible = false;
}

bool CMaitreD::RestaurantMusicChanged(CRestaurantMusicChanged *msg) {
	if (msg->_value.empty()) {
		_hasMusic = false;
	} else {
		_musicName = msg->_value;
		_hasMusic = _musicSet = true;
	}

	return true;
}

bool CMaitreD::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	if (msg->_action == 8) {
		_fightFlag = true;
		stopAnimTimer(_timerId);
		_timerId = startAnimTimer("MD Fight", 3500, 0);
	} else if (msg->_action == 9) {
		stopAnimTimer(_timerId);
		_timerId = 0;
	} else if (msg->_action == 10) {
		_fightFlag = false;
		_defeated = true;
		stopAnimTimer(_timerId);
		_timerId = 0;

		CMaitreDDefeatedMsg defeatedMsg;
		defeatedMsg.execute(findRoom());
	}

	return true;
}

bool CMaitreD::EnterViewMsg(CEnterViewMsg *msg) {
	setTalking(this, true, findView());
	_fightFlag = _savedFightFlag;

	if (_musicName != "STMusic" && (!_musicSet || _priorMusicName == _musicName))
		return true;

	// WORKAROUND: It's possible in the original to not have a music handler set
	// if you start and stop the phonograph, then save and restore the game
	if (!CMusicRoom::_musicHandler)
		return true;

	if (_musicName.contains("nasty ambient"))
		startTalking(this, 111, findView());
	else if (!CMusicRoom::_musicHandler->checkInstrument(SNAKE))
		startTalking(this, 114, findView());
	else if (!CMusicRoom::_musicHandler->checkInstrument(BASS))
		startTalking(this, 113, findView());
	else if (!CMusicRoom::_musicHandler->checkInstrument(PIANO))
		startTalking(this, 115, findView());
	else {
		startTalking(this, 110, findView());
		CMaitreDHappyMsg happyMsg;
		happyMsg.execute("MaitreD Left Arm");
		happyMsg.execute("MaitreD Right Arm");
	}

	_priorMusicName = _musicName;
	return true;
}

bool CMaitreD::LeaveViewMsg(CLeaveViewMsg *msg) {
	_savedFightFlag = _fightFlag;
	performAction(true);
	stopAnimTimer(_timerId);
	_timerId = 0;

	_fightFlag = false;
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
		if (_fightFlag)
			animMsg._value = 0;
		animMsg.execute(this, nullptr, MSGFLAG_SCAN);
	}

	return true;
}

bool CMaitreD::TimerMsg(CTimerMsg *msg) {
	if (msg->_action == "MD Fight") {
		if (_fightFlag && compareViewNameTo("1stClassRestaurant.MaitreD Node.N")) {
			startTalking(this, 131, findView());
		}
	} else {
		CTrueTalkNPC::TimerMsg(msg);
	}

	return true;
}

bool CMaitreD::TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg) {
	if (_fightFlag) {
		stopAnimTimer(_timerId);
		_timerId = 0;
	}

	CTrueTalkNPC::TrueTalkNotifySpeechStartedMsg(msg);
	return true;
}

bool CMaitreD::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	if (_fightFlag) {
		stopAnimTimer(_timerId);
		_timerId = startAnimTimer("MD Fight", 3000 + getRandomNumber(3000));
	}

	CTrueTalkNPC::TrueTalkNotifySpeechEndedMsg(msg);
	return true;
}

bool CMaitreD::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_fightFlag) {
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
