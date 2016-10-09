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

#include "titanic/npcs/bellbot.h"
#include "titanic/carry/carry.h"
#include "titanic/core/room_item.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBellBot, CTrueTalkNPC)
	ON_MESSAGE(OnSummonBotMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(Use)
	ON_MESSAGE(DismissBotMsg)
	ON_MESSAGE(TrueTalkTriggerActionMsg)
	ON_MESSAGE(MovieFrameMsg)
	ON_MESSAGE(PutBotBackInHisBoxMsg)
	ON_MESSAGE(NPCPlayIdleAnimationMsg)
	ON_MESSAGE(NPCPlayTalkingAnimationMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(TrueTalkGetStateValueMsg)
	ON_MESSAGE(TrueTalkNotifySpeechEndedMsg)
END_MESSAGE_MAP()

CBellBot::CBellBot() : CTrueTalkNPC(), _field108(0) {
}

void CBellBot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field108, indent);

	CTrueTalkNPC::save(file, indent);
}

void CBellBot::load(SimpleFile *file) {
	file->readNumber();
	_field108 = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CBellBot::OnSummonBotMsg(COnSummonBotMsg *msg) {
	if (msg->_value == 1) {
		_npcFlags |= NPCFLAG_40000;
	} else {
		static const char *const ROOM_WAVES[8][2] = {
			{ "EmbLobby",  "z#193.wav" },
			{ "PromenadeDeck", "z#191.wav" },
			{ "Arboretum", "z#195.wav" },
			{ "Frozen Arboretum", "z#195.wav" },
			{ "Bar", "z#194.wav" },
			{ "MusicRoom", "z#192.wav" },
			{ "MusicRoomLobby", "z#192.wav" },
			{ "1stClassRestaurant", "z#190.wav" }
		};

		int idx;
		for (idx = 0; idx < 8; ++idx) {
			if (compareRoomNameTo(ROOM_WAVES[idx][0])) {
				playSound(ROOM_WAVES[idx][1]);

			}
		}
		if (idx == 8)
			playSound("z#147.wav");

		sleep(2000);
		_npcFlags &= ~NPCFLAG_40000;
	}

	playClip("Walk On", MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
	movieEvent();
	_npcFlags |= NPCFLAG_10000;

	return true;
}

bool CBellBot::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_npcFlags & NPCFLAG_10000) {
		performAction(1);
		_npcFlags &= ~NPCFLAG_4;
		CDismissBotMsg dismissMsg;
		dismissMsg.execute(this);
	}

	return true;
}

bool CBellBot::MovieEndMsg(CMovieEndMsg *msg) {
	if (!(_npcFlags & NPCFLAG_10000)) {
		CTrueTalkNPC::MovieEndMsg(msg);
	} else if (clipExistsByEnd("Walk On", msg->_endFrame)) {
		setPosition(Point(80, 10));
		loadFrame(543);
		_npcFlags |= NPCFLAG_4;
		if (_npcFlags & NPCFLAG_40000) {
			startTalking(this, 157);
			_npcFlags &= ~NPCFLAG_40000;
		}

		endTalking(this, true);
		petSetArea(PET_CONVERSATION);
	} else if (clipExistsByEnd("Walk Off", msg->_endFrame)) {
		CPutBotBackInHisBoxMsg boxMsg;
		boxMsg.execute(this);

		if (_npcFlags & NPCFLAG_20000)
			startAnimTimer("SummonDoorbot", 1500);
	} else {
		CTrueTalkNPC::MovieEndMsg(msg);
	}

	return true;
}

bool CBellBot::Use(CUse *msg) {
	dynamic_cast<CCarry *>(msg->_item)->_string1 = "Bellbot";
	return true;
}

bool CBellBot::DismissBotMsg(CDismissBotMsg *msg) {
	if (_npcFlags & NPCFLAG_10000) {
		playClip("Walk Off", MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
		if (_npcFlags & NPCFLAG_4) {
			_npcFlags &= ~NPCFLAG_4;
			performAction(true);
		} else {
			performAction(false);
		}

		CActMsg actMsg("BellbotDismissed");
		actMsg.execute("BotIdleSummons");
	}

	return true;
}

bool CBellBot::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	switch (msg->_action) {
	case 1:
	case 28: {
		_npcFlags &= ~NPCFLAG_2;
		CDismissBotMsg dismissMsg;
		dismissMsg.execute(this);
		break;
	}

	case 5:
		_npcFlags &= ~NPCFLAG_20000;
		playClip("Walk Off", MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
		movieEvent();
		break;

	case 17: {
		CActMsg actMsg("ThrowTVDownWell");
		actMsg.execute("ThrowTVDownWellControl");
		break;
	}

	case 29: {
		CActMsg actMsg("BellbotGetLight");
		actMsg.execute("BellbotGetLightCutScene");
		startTalking(this, 158);
		break;
	}

	default:
		break;
	}

	return true;
}

bool CBellBot::MovieFrameMsg(CMovieFrameMsg *msg) {
	if (clipExistsByStart("Walk Off", msg->_frameNumber)
			|| clipExistsByStart("Walk On", msg->_frameNumber)) {
		setPosition(Point(20, 10));
	}

	return true;
}

bool CBellBot::PutBotBackInHisBoxMsg(CPutBotBackInHisBoxMsg *msg) {
	petMoveToHiddenRoom();
	_npcFlags &= ~NPCFLAG_4;
	return true;
}

bool CBellBot::NPCPlayIdleAnimationMsg(CNPCPlayIdleAnimationMsg *msg) {
	static const char *const NAMES[] = {
		"Sway Side To Side", "Hit Head", "Hands On Hips", "Sway",
		"Hand Wave", "Slow Sway", "Lean Backwards",
		"Sway Side To Side 2", "Bob Up And Down", nullptr
	};

	msg->_names = NAMES;
	return true;
}

bool CBellBot::NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg) {
	static const char *const NAMES[] = {
		"Hand On Hip Talking", "Hand On Hip Talking", "Hand On Hip Talking",
		"Sway Side To Side", "Lean Forward", "Hit Head", "Confidential Talking",
		"Hands On Hips", "Hands On Hips", "Hands On Hips", "Sway", "Laugh",
		"Hand Wave", "Slow Sway", "Lean Backwards", "Sway Side To Side 2",
		"Bob Up And Down", "Elbow In Hand", "Elbow In Hand", "Elbow In Hand",
		nullptr
	};

	if (msg->_value2 == 2)
		playClip("Mother Frame", 0);
	else
		msg->_names = NAMES;

	return true;
}

bool CBellBot::TimerMsg(CTimerMsg *msg) {
	if (msg->_action == "SummonDoorbot") {
		CTrueTalkNPC::TimerMsg(msg);
	} else {
		CRoomItem *room = getRoom();
		if (room) {
			CSummonBotMsg botMsg;
			botMsg._npcName = "Doorbot";
			botMsg._value = 2;
			botMsg.execute(room);
		}

		_npcFlags &= ~NPCFLAG_20000;
	}

	return true;
}

bool CBellBot::TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg) {
	CPetControl *pet = getPetControl();
	bool flag = pet ? pet->isRoom59706() : false;

	if (msg->_stateNum == 7)
		msg->_stateVal = flag ? 1 : 0;

	return true;
}

bool CBellBot::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	CTrueTalkNPC::TrueTalkNotifySpeechEndedMsg(msg);

	if (msg->_dialogueId == 20991)
		petDismissBot("DoorBot");

	return true;
}

} // End of namespace Titanic
