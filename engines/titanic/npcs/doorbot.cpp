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

#include "titanic/npcs/doorbot.h"
#include "titanic/core/room_item.h"
#include "titanic/debugger.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CDoorbot, CTrueTalkNPC)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(OnSummonBotMsg)
	ON_MESSAGE(TrueTalkTriggerActionMsg)
	ON_MESSAGE(DoorbotNeededInHomeMsg)
	ON_MESSAGE(DoorbotNeededInElevatorMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(NPCPlayTalkingAnimationMsg)
	ON_MESSAGE(NPCPlayIdleAnimationMsg)
	ON_MESSAGE(PutBotBackInHisBoxMsg)
	ON_MESSAGE(DismissBotMsg)
	ON_MESSAGE(MovieFrameMsg)
	ON_MESSAGE(TrueTalkNotifySpeechEndedMsg)
	ON_MESSAGE(TextInputMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

int CDoorbot::_v1;
int CDoorbot::_v2;

CDoorbot::CDoorbot() : CTrueTalkNPC() {
	_introMovieNum = 0;
	_timerId = 0;
	_field110 = 0;
	_field114 = 0;
}

void CDoorbot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);

	file->writeNumberLine(_introMovieNum, indent);
	file->writeNumberLine(_timerId, indent);
	file->writeNumberLine(_field110, indent);
	file->writeNumberLine(_field114, indent);

	CTrueTalkNPC::save(file, indent);
}

void CDoorbot::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();

	_introMovieNum = file->readNumber();
	_timerId = file->readNumber();
	_field110 = file->readNumber();
	_field114 = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CDoorbot::MovieEndMsg(CMovieEndMsg *msg) {
	debugC(DEBUG_DETAILED, kDebugScripts, "CDoorbot MovieEndMsg flags=%x v=%d, start=%d, end=%d",
		_npcFlags, _introMovieNum, msg->_startFrame, msg->_endFrame);

	if (_npcFlags & NPCFLAG_DOORBOT_INTRO) {
		switch (_introMovieNum) {
		case 3:
			startTalking(this, 221482);
			_introMovieNum = 4;
			break;

		case 6:
			if (clipExistsByEnd("Cloak On", msg->_endFrame)) {
				petShow();
				petDecAreaLocks();
				stateSetSoundMakerAllowed(true);
				changeView("ServiceElevator.Node 1.S");
				changeView("ServiceElevator.Node 1.N");
			}
			break;

		case 7:
			startTalking(this, 221467);
			_introMovieNum = 8;
			break;

		case 9:
			if (msg->_endFrame == 949)
				startTalking(this, 221468);
			break;

		case 11:
			changeView("ServiceElevator.Node 1.S");
			changeView("MoonEmbLobby.Node 1.NE");
			break;

		default:
			break;
		}

		CTrueTalkNPC::MovieEndMsg(msg);
	} else if (_npcFlags & NPCFLAG_MOVE_LEFT) {
		if (clipExistsByEnd("Cloak Off", msg->_endFrame)) {
			_npcFlags = (_npcFlags & ~NPCFLAG_DOORBOT_IN_HOME) | NPCFLAG_START_IDLING;
			setTalking(this, false);
			startTalking(this, 221474);
			_npcFlags |= NPCFLAG_DOORBOT_INTRO;
			_introMovieNum = 0;
		} else if (clipExistsByEnd("Cloak On", msg->_endFrame)) {
			petShow();
			stateSetSoundMakerAllowed(true);
			changeView("ServiceElevator.Node 1.S");
		} else {
			CTrueTalkNPC::MovieEndMsg(msg);
		}
	} else if (_npcFlags & NPCFLAG_MOVE_END) {
		if (clipExistsByEnd("Whizz On Left", msg->_endFrame)
				|| clipExistsByEnd("Whizz On Right", msg->_endFrame)) {
			setPosition(Point((600 - _bounds.width()) / 2 + 18, 42));
			loadFrame(0);
			setTalking(this, true);
			_npcFlags |= NPCFLAG_START_IDLING;
			petSetArea(PET_CONVERSATION);
		} else if (clipExistsByEnd("Whizz Off Left", msg->_endFrame)
				|| clipExistsByEnd("Whizz Off Right", msg->_endFrame)) {
			CPutBotBackInHisBoxMsg boxMsg;
			boxMsg.execute(this);
			if (_npcFlags & NPCFLAG_SUMMON_BELLBOT)
				startAnimTimer("SummonBellbot", 1500);
		} else {
			CTrueTalkNPC::MovieEndMsg(msg);
		}
	} else {
		CTrueTalkNPC::MovieEndMsg(msg);
	}

	return true;
}

bool CDoorbot::OnSummonBotMsg(COnSummonBotMsg *msg) {
	struct RoomWave {
		const char *_room;
		const char *_enSound;
		const char *_deSound;
	};
	const RoomWave ROOM_WAVES[8] = {
		{ "EmbLobby", "z#186.wav", "z#716.wav" },
		{ "PromenadeDeck", "z#184.wav", "z#714.wav" },
		{ "Arboretum", "z#188.wav", "z#718.wav" },
		{ "Frozen Arboretum", "z#188.wav", "z#718.wav" },
		{ "Bar", "z#187.wav", "z#717.wav" },
		{ "MusicRoom", "z#185.wav", "z#715.wav" },
		{ "MusicRoomLobby", "z#185.wav", "z#715.wav" },
		{ "1stClassRestaurant", "z#183.wav", "z#713.wav" },
	};

	if (msg->_value != -1) {
		int idx;
		for (idx = 0; idx < 8; ++idx) {
			if (compareRoomNameTo(ROOM_WAVES[idx]._room)) {
				playSound(TRANSLATE(ROOM_WAVES[idx]._enSound, ROOM_WAVES[idx]._deSound));
				break;
			}
		}
		if (idx == 8)
			playSound(TRANSLATE("z#146.wav", "z#702.wav"));

		sleep(2000);
	}

	playClip(getRandomNumber(1) ? "Whizz On Left" : "Whizz On Right",
		MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	movieEvent();
	_npcFlags |= NPCFLAG_MOVE_END;

	return true;
}

bool CDoorbot::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	switch (msg->_action) {
	case 3:
		playClip("Cloak On", MOVIE_NOTIFY_OBJECT);
		break;

	case 4:
		_npcFlags = (_npcFlags & ~NPCFLAG_IDLING) | NPCFLAG_SUMMON_BELLBOT;
		playClip("Whizz Off Left", MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		break;

	case 28: {
		_npcFlags &= ~(NPCFLAG_IDLING | NPCFLAG_START_IDLING);
		CDismissBotMsg dismissMsg;
		dismissMsg.execute(this);
		break;
	}

	default:
		break;
	}

	return true;
}

bool CDoorbot::DoorbotNeededInHomeMsg(CDoorbotNeededInHomeMsg *msg) {
	moveToView();
	setPosition(Point(90, 42));
	_npcFlags = NPCFLAG_MOVE_LEFT;

	stopMovie();
	playClip("Cloak Off", MOVIE_NOTIFY_OBJECT);

	_npcFlags |= NPCFLAG_DOORBOT_IN_HOME;
	return true;
}

bool CDoorbot::DoorbotNeededInElevatorMsg(CDoorbotNeededInElevatorMsg *msg) {
	moveToView("ServiceElevator.Node 1.N");
	setPosition(Point(100, 42));

	if (_npcFlags & NPCFLAG_DOORBOT_INTRO) {
		_introMovieNum = 7;
		_npcFlags |= NPCFLAG_MOVE_RIGHT;
		loadFrame(797);
	} else {
		_npcFlags = 0;
		if (msg->_value)
			setTalking(this, true);
	}

	return true;
}

bool CDoorbot::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (!(_npcFlags & NPCFLAG_DOORBOT_INTRO) && (_npcFlags & NPCFLAG_MOVE_END)) {
		performAction(true);
		_npcFlags &= ~NPCFLAG_START_IDLING;
	}

	return true;
}

bool CDoorbot::TimerMsg(CTimerMsg *msg) {
	if (msg->_action == "NPCIdleAnim") {
		return CTrueTalkNPC::TimerMsg(msg);
	} else if (_npcFlags & NPCFLAG_DOORBOT_INTRO) {
		_timerId = 0;

		switch (msg->_actionVal) {
		case 0:
			startTalking(this, 221475);
			break;

		case 1:
			startTalking(this, 221476);
			break;

		case 2:
			startTalking(this, 221477);
			break;

		case 3:
			playClip("DoubleTake Start");
			playClip("DoubleTake End");
			playClip("DoubleTake Start");
			playClip("DoubleTake End", MOVIE_NOTIFY_OBJECT);
			_introMovieNum = 3;
			break;

		case 4:
			startTalking(this, 221483);
			lockInputHandler();
			_field114 = true;
			break;

		case 5:
			lockInputHandler();
			mouseDisableControl();
			_field114 = true;
			startTalking(this, 221485);
			break;

		case 6:
			// Start dragging photograph to PET
			CMouseButtonDownMsg::generate();
			mouseSetPosition(Point(200, 430), 2500);
			_timerId = addTimer(7, 2500, 0);
			break;

		case 7:
			// Drop photograph in PET
			CMouseButtonUpMsg::generate();
			startTalking(this, 221486);
			mouseEnableControl();
			unlockInputHandler();
			_field114 = false;
			disableMouse();
			break;

		default:
			break;
		}
	} else if (msg->_action == "SummonBellbot") {
		CRoomItem *room = getRoom();
		if (room) {
			CSummonBotMsg botMsg;
			botMsg._npcName = "Bellbot";
			botMsg.execute(room);
		}

		_npcFlags &= ~NPCFLAG_SUMMON_BELLBOT;
	}

	return true;
}

bool CDoorbot::NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg) {
	static const char *const NAMES1[] = {
		"Mutter Aside", "Rub Chin", "Drunken Eye Roll", "Drunken Head Move",
		"Look down and mutter",  "Look side to side", "Gesture forward and around",
		"Arms behind back", "Look down", "Rolling around", "Hold face",
		"Touch chin", "Cross hands in front", nullptr
	};
	static const char *const NAMES2[] = {
		"SE Talking 1", "SE Talking 2", "SE Talking 3", "SE Talking 4",
		nullptr
	};
	static const char *const NAMES3[] = {
		"SE Ask For Help", nullptr
	};

	if (msg->_value2 != 2) {
		if (_npcFlags & NPCFLAG_MOVE_RIGHT) {
			switch (_introMovieNum) {
			case 8:
			case 10:
				msg->_names = NAMES2;
				break;
			case 9:
				msg->_names = NAMES3;
				_introMovieNum = 10;
				break;
			default:
				break;
			}
		} else if (_npcFlags & (NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_END)) {
			msg->_names = NAMES1;
		}
	}

	return true;
}

bool CDoorbot::NPCPlayIdleAnimationMsg(CNPCPlayIdleAnimationMsg *msg) {
	static const char *const NAMES[] = {
		"Hand swivel", "Prompt Push", "Eye Roll", "Say something", nullptr
	};

	if (!(_npcFlags & (NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT))
			&& (_npcFlags & NPCFLAG_MOVE_END))
		msg->_names = NAMES;

	return true;
}

bool CDoorbot::PutBotBackInHisBoxMsg(CPutBotBackInHisBoxMsg *msg) {
	petMoveToHiddenRoom();
	_npcFlags &= ~(NPCFLAG_START_IDLING | NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT | NPCFLAG_DOORBOT_INTRO);
	if (msg->_value)
		performAction(true);

	return true;
}

bool CDoorbot::DismissBotMsg(CDismissBotMsg *msg) {
	if (_npcFlags & NPCFLAG_MOVE_END) {
		playClip(getRandomNumber(1) ? "Whizz Off Left" : "Whizz Off Right",
			MOVIE_STOP_PREVIOUS | MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		movieEvent();

		if (_npcFlags & NPCFLAG_START_IDLING) {
			_npcFlags &= ~NPCFLAG_START_IDLING;
			performAction(true);
		} else {
			performAction(false);
		}

		CActMsg actMsg("DoorbotDismissed");
		actMsg.execute("BotIdleSummons");
	}

	return true;
}

bool CDoorbot::MovieFrameMsg(CMovieFrameMsg *msg) {
	if (clipExistsByStart("Whizz Off Left", msg->_frameNumber)
			|| clipExistsByStart("Whizz On Left", msg->_frameNumber)) {
		setPosition(Point(20, 42));
	} else if (clipExistsByStart("Whizz Off Right", msg->_frameNumber)
			|| clipExistsByStart("Whizz On Right", msg->_frameNumber)) {
		setPosition(Point(620 - _bounds.width(), 42));
	}

	return true;
}

bool CDoorbot::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	CTrueTalkNPC::TrueTalkNotifySpeechEndedMsg(msg);

	if (_npcFlags & NPCFLAG_DOORBOT_INTRO) {
		// Initial speech by Doorbot in 
		switch (msg->_dialogueId - TRANSLATE(10552, 10563)) {
		case 0:
			playClip("SE Try Buttons", MOVIE_NOTIFY_OBJECT);
			_introMovieNum = 9;
			break;

		case 1:
			enableMouse();
			break;

		case 5:
			playClip("SE Move To Right", MOVIE_NOTIFY_OBJECT);
			_introMovieNum = 11;
			break;

		case 7:
			stopAnimTimer(_timerId);
			_timerId = addTimer(0, 2500, 0);
			break;

		case 8:
			petShow();
			petSetArea(PET_CONVERSATION);
			petIncAreaLocks();
			stopAnimTimer(_timerId);
			_timerId = addTimer(1, 1000, 0);
			break;

		case 9:
			enableMouse();
			_introMovieNum = 1;
			stopAnimTimer(_timerId);
			_timerId = addTimer(2, 10000, 0);
			break;

		case 10:
			if (_introMovieNum == 1) {
				stopAnimTimer(_timerId);
				_timerId = addTimer(2, getRandomNumber(5000) + 5000, 0);
			}
			break;

		case 11:
		case 12:
			disableMouse();
			startTalking(this, 221480);
			break;

		case 13:
			startTalking(this, 221481);
			break;

		case 14:
			stopAnimTimer(_timerId);
			_timerId = 0;
			if (_field110 == 2) {
				playClip("Cloak On", MOVIE_NOTIFY_OBJECT);
				_introMovieNum = 6;
			} else {
				_timerId = addTimer(3, 2000, 0);
			}
			break;

		case 15: {
			CActMsg actMsg("BecomeGettable");
			actMsg.execute("Photograph");
			enableMouse();
			stopAnimTimer(_timerId);
			_timerId = addTimer(4, 5000, 0);
			break;
		}

		case 16:
			// Start moving cursor to photograph
			mouseDisableControl();
			mouseSetPosition(Point(600, 250), 2500);
			_timerId = addTimer(6, 2500, 0);
			break;

		case 17:
			if (_field110 != 2) {
				stopAnimTimer(_timerId);
				_timerId = addTimer(5, 3000, 0);
			}
			break;

		case 18:
			mouseSetPosition(Point(200, 430), 2500);
			_timerId = addTimer(7, 3000, 0);
			break;

		case 19:
			playClip("Cloak On", MOVIE_NOTIFY_OBJECT);
			_introMovieNum = 6;
			break;

		default:
			break;
		}
	}

	return true;
}

bool CDoorbot::TextInputMsg(CTextInputMsg *msg) {
	if (!(_npcFlags & NPCFLAG_DOORBOT_INTRO))
		return CTrueTalkNPC::TextInputMsg(msg);

	if (_introMovieNum == 1) {
		stopAnimTimer(_timerId);
		_introMovieNum = 2;
		_timerId = 0;

		if (msg->_input == "yes" || msg->_input == "yeah"
				|| msg->_input == "yea" || msg->_input == "yup"
				|| msg->_input == "yep" || msg->_input == "sure"
				|| msg->_input == "alright" || msg->_input == "all right"
				|| msg->_input == "ok") {
			startTalking(this, 221479);
		} else {
			startTalking(this, 221478);
		}
	}

	return true;
}

bool CDoorbot::EnterViewMsg(CEnterViewMsg *msg) {
	if ((_npcFlags & NPCFLAG_DOORBOT_INTRO) && _introMovieNum == 7)
		playClip("SE Move And Turn", MOVIE_NOTIFY_OBJECT);
	else if (!compareRoomNameTo("ServiceElevator") && msg->_newView == getParent() && getPetControl()->canSummonBot("DoorBot")) {
		// WORKAROUND: Calling bot in front of doors and then going through them
		// can leave it in the view. Detect this and properly remove him when
		// the player returns to that view
		petMoveToHiddenRoom();
	}

	return true;
}

bool CDoorbot::ActMsg(CActMsg *msg) {
	debugC(DEBUG_DETAILED, kDebugScripts, "CDoorbot ActMsg action=%s v108=%d v110=%d v114=%d",
		msg->_action.c_str(), _introMovieNum, _field110, _field114);

	if (msg->_action == "DoorbotPlayerPressedTopButton") {
		disableMouse();
		startTalking(this, 221471);
	} else if (msg->_action == "DoorbotPlayerPressedMiddleButton") {
		startTalking(this, 221470);
	} else if (msg->_action == "DoorbotPlayerPressedBottomButton") {
		startTalking(this, 221469);
	} else if (msg->_action == "DoorbotReachedEmbLobby") {
		startTalking(this, 221472);
	} else if (msg->_action == "PlayerPicksUpPhoto") {
		_field110 = 1;
		if (!_field114 && _introMovieNum == 4) {
			stopAnimTimer(_timerId);
			_timerId = 0;
			_introMovieNum = 5;
			startTalking(this, 221484);
		}
	} else if (msg->_action == "PlayerPutsPhotoInPet") {
		_field110 = 2;
		if (!_field114 && _introMovieNum == 5) {
			stopAnimTimer(_timerId);
			_timerId = 0;
			startTalking(this, 221486);
			disableMouse();
		}
	}

	return true;
}

} // End of namespace Titanic
