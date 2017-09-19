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
#include "titanic/game_manager.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBellBot, CTrueTalkNPC)
	ON_MESSAGE(OnSummonBotMsg)
	ON_MESSAGE(EnterViewMsg)
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
		_npcFlags |= NPCFLAG_MOVE_LOOP;
	} else {
		struct RoomWave {
			const char *_room;
			const char *_enSound;
			const char *_deSound;
		};
		static const RoomWave ROOM_WAVES[8] = {
			{ "EmbLobby",  "z#193.wav", "z#723.wav" },
			{ "PromenadeDeck", "z#191.wav", "z#721.wav" },
			{ "Arboretum", "z#195.wav", "z#725.wav" },
			{ "Frozen Arboretum", "z#195.wav", "z#725.wav" },
			{ "Bar", "z#194.wav", "z#724.wav" },
			{ "MusicRoom", "z#192.wav", "z#722.wav" },
			{ "MusicRoomLobby", "z#192.wav", "z#722.wav" },
			{ "1stClassRestaurant", "z#190.wav", "z#720.wav" }
		};

		int idx;
		for (idx = 0; idx < 8; ++idx) {
			if (compareRoomNameTo(ROOM_WAVES[idx]._room)) {
				playSound(TRANSLATE(ROOM_WAVES[idx]._enSound, ROOM_WAVES[idx]._deSound));
				break;
			}
		}
		if (idx == 8)
			playSound(TRANSLATE("z#147.wav", "z#703.wav"));

		sleep(2000);
		_npcFlags &= ~NPCFLAG_MOVE_LOOP;
	}

	getGameManager()->_gameState.setMode(GSMODE_CUTSCENE);
	playClip("Walk On", MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	movieEvent();
	_npcFlags |= NPCFLAG_MOVING;

	return true;
}

bool CBellBot::EnterViewMsg(CEnterViewMsg *msg) {
	// WORKAROUND: Calling bot in front of doors and then going through them
	// can leave it in the view. Detect this and properly remove him when
	// the player returns to that view
	if (!hasActiveMovie() && msg->_newView == getParent()
			&& getPetControl()->canSummonBot("BellBot"))
		petMoveToHiddenRoom();

	return true;
}

bool CBellBot::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_npcFlags & NPCFLAG_MOVING) {
		performAction(true);
		_npcFlags &= ~NPCFLAG_START_IDLING;
		CDismissBotMsg dismissMsg;
		dismissMsg.execute(this);
	}

	return true;
}

bool CBellBot::MovieEndMsg(CMovieEndMsg *msg) {
	if (!(_npcFlags & NPCFLAG_MOVING)) {
		CTrueTalkNPC::MovieEndMsg(msg);
	} else if (clipExistsByEnd("Walk On", msg->_endFrame)) {
		getGameManager()->_gameState.setMode(GSMODE_INTERACTIVE);
		setPosition(Point(80, 10));
		loadFrame(543);
		_npcFlags |= NPCFLAG_START_IDLING;
		if (_npcFlags & NPCFLAG_MOVE_LOOP) {
			startTalking(this, 157);
			_npcFlags &= ~NPCFLAG_MOVE_LOOP;
		}

		setTalking(this, true);
		petSetArea(PET_CONVERSATION);
	} else if (clipExistsByEnd("Walk Off", msg->_endFrame)) {
		CPutBotBackInHisBoxMsg boxMsg;
		boxMsg.execute(this);

		if (_npcFlags & NPCFLAG_MOVE_START)
			startAnimTimer("SummonDoorbot", 1500);
	} else {
		CTrueTalkNPC::MovieEndMsg(msg);
	}

	return true;
}

bool CBellBot::Use(CUse *msg) {
	CCarry *item = dynamic_cast<CCarry *>(msg->_item);
	assert(item);
	item->_npcUse = "Bellbot";
	return true;
}

bool CBellBot::DismissBotMsg(CDismissBotMsg *msg) {
	if (_npcFlags & NPCFLAG_MOVING) {
		playClip("Walk Off", MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		movieEvent();
		if (_npcFlags & NPCFLAG_START_IDLING) {
			_npcFlags &= ~NPCFLAG_START_IDLING;
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
		_npcFlags &= ~NPCFLAG_IDLING;
		CDismissBotMsg dismissMsg;
		dismissMsg.execute(this);
		break;
	}

	case 5:
		_npcFlags &= ~NPCFLAG_MOVE_START;
		playClip("Walk Off", MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
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
	_npcFlags &= ~NPCFLAG_START_IDLING;
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
		playClip("Mother Frame");
	else
		msg->_names = NAMES;

	return true;
}

bool CBellBot::TimerMsg(CTimerMsg *msg) {
	if (msg->_action == "SummonDoorbot") {
		CRoomItem *room = getRoom();
		if (room) {
			CSummonBotMsg botMsg;
			botMsg._npcName = "Doorbot";
			botMsg._value = 2;
			botMsg.execute(room);
		}

		_npcFlags &= ~NPCFLAG_MOVE_START;
	} else {
		CTrueTalkNPC::TimerMsg(msg);
	}

	return true;
}

bool CBellBot::TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg) {
	CPetControl *pet = getPetControl();
	bool isYourStateroom = pet ? pet->isFirstClassSuite() : false;

	if (msg->_stateNum == 7)
		msg->_stateVal = isYourStateroom ? 1 : 0;

	return true;
}

bool CBellBot::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	CTrueTalkNPC::TrueTalkNotifySpeechEndedMsg(msg);

	if (msg->_dialogueId == TRANSLATE(20991, 20997)) {
		petDismissBot("DoorBot");
		getGameManager()->unlockInputHandler();
	}

	return true;
}

} // End of namespace Titanic
