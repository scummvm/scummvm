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

#include "titanic/npcs/parrot.h"
#include "titanic/core/project_item.h"
#include "titanic/carry/carry.h"
#include "titanic/carry/chicken.h"
#include "titanic/game_manager.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CParrot, CTrueTalkNPC)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(TrueTalkTriggerActionMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(ParrotSpeakMsg)
	ON_MESSAGE(NPCPlayTalkingAnimationMsg)
	ON_MESSAGE(NPCPlayIdleAnimationMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(MovieFrameMsg)
	ON_MESSAGE(PutParrotBackMsg)
	ON_MESSAGE(PreEnterViewMsg)
	ON_MESSAGE(PanningAwayFromParrotMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(TrueTalkNotifySpeechStartedMsg)
	ON_MESSAGE(TrueTalkNotifySpeechEndedMsg)
END_MESSAGE_MAP()

bool CParrot::_eatingChicken;
bool CParrot::_takeOff;
bool CParrot::_unused;
ParrotState CParrot::_state;
bool CParrot::_coreReplaced;

CParrot::CParrot() : CTrueTalkNPC() {
	_unused1 = 0;
	_carryParrot = "CarryParrot";
	_canDrag = true;
	_unused2 = 25;
	_lastSpeakTime = 0;
	_newXp = 73;
	_newXc = 58;
	_triedEatChicken = false;
	_eatOffsetX = 0;
	_panTarget = nullptr;

	_assetName = "z454.dlg";
	_assetNumber = 0x13880;
}

void CParrot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_assetNumber, indent);

	file->writeQuotedLine(_assetName, indent);
	file->writeNumberLine(_unused1, indent);
	file->writeNumberLine(_eatingChicken, indent);
	file->writeNumberLine(_takeOff, indent);
	file->writeNumberLine(_unused, indent);

	file->writeQuotedLine(_carryParrot, indent);
	file->writeNumberLine(_canDrag, indent);
	file->writeNumberLine(_unused2, indent);
	file->writeNumberLine(_lastSpeakTime, indent);
	file->writeNumberLine(_newXp, indent);
	file->writeNumberLine(_newXc, indent);
	file->writeNumberLine(_triedEatChicken, indent);
	file->writeNumberLine(_eatOffsetX, indent);
	file->writeNumberLine(_state, indent);
	file->writeNumberLine(_coreReplaced, indent);

	CTrueTalkNPC::save(file, indent);
}

void CParrot::load(SimpleFile *file) {
	file->readNumber();
	_assetNumber = file->readNumber();

	_assetName = file->readString();
	_unused1 = file->readNumber();
	_eatingChicken = file->readNumber();
	_takeOff = file->readNumber();
	_unused = file->readNumber();

	_carryParrot = file->readString();
	_canDrag = file->readNumber();
	_unused2 = file->readNumber();
	_lastSpeakTime = file->readNumber();
	_newXp = file->readNumber();
	_newXc = file->readNumber();
	_triedEatChicken = file->readNumber();
	_eatOffsetX = file->readNumber();
	_state = (ParrotState)file->readNumber();
	_coreReplaced = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CParrot::ActMsg(CActMsg *msg) {
	if (msg->_action == "Chicken") {
		// Nothing to do
	} else if (msg->_action == "CarryParrotLeftView") {
		if (!_takeOff)
			setEatingChicken(false);
	} else if (msg->_action == "StartChickenDrag") {
		if (_state == PARROT_IN_CAGE) {
			stopMovie();
			startTalking(this, 280275, findView());
			_triedEatChicken = false;
		}
	} else if (msg->_action == "EnteringFromTOW" &&
			(_state == PARROT_IN_CAGE || _state == PARROT_ESCAPED)) {
		if (_takeOff) {
			_state = PARROT_ESCAPED;
		} else {
			setVisible(true);
			CTreeItem *cageBar = getRoot()->findByName("CageBar");
			detach();
			attach(cageBar);

			_state = PARROT_IN_CAGE;
			CActMsg actMsg1("OpenNow");
			actMsg1.execute("ParrotCage");
			CActMsg actMsg2("GainParrot");
			actMsg2.execute("ParrotLobbyController");
		}
	}

	return true;
}

bool CParrot::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!(_npcFlags & NPCFLAG_TAKE_OFF) && _speechCounter == 0) {
		CTrueTalkTriggerActionMsg triggerMsg(280250, 280250, 1);
		triggerMsg.execute(this);
	}

	return true;
}

bool CParrot::MovieEndMsg(CMovieEndMsg *msg) {
	if ((_npcFlags & NPCFLAG_TAKE_OFF) && clipExistsByEnd("Take Off", msg->_endFrame)) {
		setVisible(false);
		moveUnder(findRoom());
		stopMovie();

		CActMsg actMsg1("LoseParrot");
		actMsg1.execute("ParrotLobbyController");

		if (_panTarget) {
			CActMsg actMsg2("PanAwayFromParrot");
			actMsg2.execute(_panTarget);
			_panTarget = nullptr;
		} else {
			CActMsg actMsg2("Shut");
			actMsg2.execute("ParrotCage");
		}

		_npcFlags &= ~NPCFLAG_TAKE_OFF;
		_state = PARROT_ESCAPED;
	} else if (_npcFlags & NPCFLAG_MOVING) {
		if (_npcFlags & NPCFLAG_MOVE_START) {
			// Parrot is about to loop
			_npcFlags = (_npcFlags & ~NPCFLAG_MOVE_START) | NPCFLAG_MOVE_LOOP;
			if (_npcFlags & NPCFLAG_MOVE_LEFT) {
				playClip("Walk Left Loop", MOVIE_NOTIFY_OBJECT);
				movieEvent(236);
			} else {
				playClip("Walk Right Loop", MOVIE_NOTIFY_OBJECT);
			}
		} else if (_npcFlags & NPCFLAG_MOVE_LOOP) {
			// In progress movement loop
			int xp = _bounds.left + _bounds.width() / 2;

			if (_npcFlags & NPCFLAG_MOVE_LEFT) {
				if ((xp - _newXc) > 32) {
					setPosition(Point(_bounds.left - 40, _bounds.top));
					playClip("Walk Left Loop", MOVIE_NOTIFY_OBJECT);
					movieEvent(236);
				} else {
					setPosition(Point(_bounds.left - 10, _bounds.top));
					playClip("Walk Left Outro", MOVIE_NOTIFY_OBJECT);
					_npcFlags = (_npcFlags & ~NPCFLAG_MOVE_LOOP) | NPCFLAG_MOVE_FINISH;
				}
			} else {
				if ((_newXc - xp) > 32) {
					playClip("Walk Right Loop", MOVIE_NOTIFY_OBJECT);
					movieEvent(244);
				} else {
					playClip("Walk Right Outro", MOVIE_NOTIFY_OBJECT);
					_npcFlags = (_npcFlags & ~NPCFLAG_MOVE_LOOP) | NPCFLAG_MOVE_FINISH;
				}
			}
		} else if (_npcFlags & NPCFLAG_MOVE_FINISH) {
			// Finishing movement
			loadFrame(0);
			if (_npcFlags & NPCFLAG_MOVE_LEFT)
				setPosition(Point(_bounds.left - 30, _bounds.top));
			else
				setPosition(Point(_bounds.left + 14, _bounds.top));

			_npcFlags &= ~(NPCFLAG_MOVING | NPCFLAG_MOVE_FINISH | NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT);
			CTrueTalkNPC::MovieEndMsg(msg);
		}
	} else if (_npcFlags & NPCFLAG_CHICKEN_OUTSIDE_CAGE) {
		Point pt = getMousePos();
		if (pt.x > 70 || pt.y < 90 || pt.y > 280) {
			stopMovie();
			loadFrame(0);
			_npcFlags &= ~NPCFLAG_CHICKEN_OUTSIDE_CAGE;
		}

		if (clipExistsByEnd("Walk Left Loop", msg->_endFrame)) {
			playClip("Lean Over To Chicken", MOVIE_NOTIFY_OBJECT);
			setPosition(Point(_bounds.left - 55, _bounds.top));
			_eatOffsetX = (-100 - _bounds.left) / 5;
			movieEvent(261);
			movieEvent(262);
			movieEvent(265);
			movieEvent(268);
			movieEvent(271);
			return true;

		} else if (clipExistsByEnd("Lean Over To Chicken", msg->_endFrame)) {
			// WORKAROUND: Do what the original obviously intended but got
			// wrong.. only flag chicken as eaten if it's still being dragged
			CTreeItem *dragItem = getGameManager()->_dragItem;
			CCarry *chicken = dynamic_cast<CCarry *>(dragItem);

			if (chicken)
				playClip("Eat Chicken");
			playClip("Eat Chicken 2", MOVIE_NOTIFY_OBJECT);

			if (chicken) {
				setEatingChicken(true);

				CTrueTalkTriggerActionMsg actionMsg;
				actionMsg._action = 280266;
				actionMsg._param2 = 1;
				actionMsg.execute(this);

				CActMsg actMsg("Eaten");
				actMsg.execute(chicken);
			}

			_npcFlags &= ~NPCFLAG_CHICKEN_OUTSIDE_CAGE;
			return true;
		}
	}

	if (clipExistsByEnd("Eat Chicken 2", msg->_endFrame)) {
		// Parrot has finished eating Chicken
		setEatingChicken(false);

		if (_takeOff) {
			// Perch has been taken, so take off
			loadMovie(TRANSLATE("z168.avi", "z191.avi"), false);
			playClip("Take Off", MOVIE_NOTIFY_OBJECT);
			setPosition(Point(20, 10));
			_npcFlags |= NPCFLAG_TAKE_OFF;
		} else {
			// Resetting back to standing
			_npcFlags &= ~(NPCFLAG_MOVING | NPCFLAG_MOVE_START | NPCFLAG_MOVE_LOOP
				| NPCFLAG_MOVE_FINISH | NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT);
			_npcFlags |= NPCFLAG_MOVE_END;
			stopMovie();
			loadFrame(0);
			setPosition(Point(-90, _bounds.top));
		}
	} else {
		return CTrueTalkNPC::MovieEndMsg(msg);
	}

	return true;
}

bool CParrot::EnterViewMsg(CEnterViewMsg *msg) {
	static const char *const NAMES[] = {
		"Talking0", "Talking1", "Talking2", "Talking3", "Talking4",
		"Talking5", "Talking6", "Talking7", nullptr
	};

	if (_state == PARROT_IN_CAGE) {
		setPosition(Point(_newXp, _bounds.top));
		_canDrag = true;
		_npcFlags &= ~(NPCFLAG_MOVING | NPCFLAG_MOVE_START | NPCFLAG_MOVE_LOOP
			| NPCFLAG_MOVE_FINISH | NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT | NPCFLAG_MOVE_END);
		loadFrame(0);
		setTalking(this, true, findView());

		if (_speechCounter > 0) {
			playRandomClip(NAMES, MOVIE_NOTIFY_OBJECT);
		} else {
			startTalking(this, 280258, findView());
		}

		petSetArea(PET_CONVERSATION);
		_triedEatChicken = false;
		_npcFlags |= NPCFLAG_START_IDLING;
	}

	return true;
}

bool CParrot::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	if (_state != PARROT_MAILED) {
		CViewItem *view = msg->_param2 ? findView() : nullptr;
		startTalking(this, msg->_action, view);
	}

	return true;
}

bool CParrot::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (_canDrag && _state == PARROT_IN_CAGE && checkPoint(msg->_mousePos, false, true)) {
		setVisible(false);
		CRoomItem *room = findRoom();

		moveUnder(room);
		startTalking(this, 280129);
		performAction(true);

		CCarry *item = dynamic_cast<CCarry *>(getRoot()->findByName(_carryParrot));
		if (item) {
			item->_canTake = true;
			CPassOnDragStartMsg passMsg;
			passMsg._mousePos = msg->_mousePos;
			passMsg.execute(item);
			msg->_dragItem = item;

			CActMsg actMsg("LoseParrot");
			actMsg.execute("ParrotLobbyController");
			return true;
		}
	}

	return false;
}

bool CParrot::LeaveViewMsg(CLeaveViewMsg *msg) {
	performAction(true);
	_npcFlags &= ~NPCFLAG_START_IDLING;

	return true;
}

bool CParrot::ParrotSpeakMsg(CParrotSpeakMsg *msg) {
	static const char *const ROOM_NAMES[] = {
		"SGTState", "SGTLittleLift", "SecClassLittleLift", "SecClassState",
		"Lift", "ServiceElevator", "Dome", "Home", "MoonEmbLobby", nullptr
	};

	if (!stateGetParrotMet() || _state == PARROT_MAILED || compareViewNameTo("Titania.Node 18.N"))
		return true;

	// Check for rooms not to speak in
	for (const char *const *s = &ROOM_NAMES[0]; *s; ++s) {
		if (isEquals(*s))
			return true;
	}

	// Don't have the parrot speak too often
	if ((getTicksCount() - _lastSpeakTime) < 20000 || _speechCounter)
		return true;

	playSound(TRANSLATE("z#475.wav", "z#212.wav"), 50);

	if (msg->_target == "Bomb") {
		startTalking("PerchedParrot", 280236);
	} else if (msg->_target == "Announcements") {
		startTalking("PerchedParrot", 280263);
	} else if (msg->_target == "Television") {
		startTalking("PerchedParrot", 280264);
	} else if (msg->_target == "Barbot") {
		if (msg->_action == "AskForDrink")
			startTalking("PerchedParrot", 280262);
	} else if (msg->_target == "SuccUBus") {
		if (msg->_action == "TurnOn")
			startTalking("PerchedParrot", 80161);
		else if (msg->_action == "EnterView")
			startTalking("PerchedParrot", 80159);
	} else if (msg->_target == "Cellpoints") {
		if (getRandomNumber(2) == 0) {
			switch (getRandomNumber(2)) {
			case 0:
				startTalking("PerchedParrot", 80193);
				break;
			case 1:
				startTalking("PerchedParrot", 80197);
				break;
			case 2:
				startTalking("PerchedParrot", 80198);
				break;
			default:
				break;
			}
		} else if (msg->_action == "DoorBot") {
			startTalking("PerchedParrot", 80195);
		} else if (msg->_action == "DeskBot") {
			startTalking("PerchedParrot", 80194);
		} else if (msg->_action == "BarBot") {
			startTalking("PerchedParrot", 80191);
		} else if (msg->_action == "BellBot") {
			startTalking("PerchedParrot", 80192);
		} else if (msg->_action == "LiftBot") {
			startTalking("PerchedParrot", 80196);
		}
	}

	_lastSpeakTime = getTicksCount();
	return true;
}

bool CParrot::NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg) {
	static const char *const NAMES[] = {
		"Talking0", "Talking1", "Talking2", "Talking3", "Talking4",
		"Talking5", "Talking6", "Talking7", nullptr
	};

	if (!(_npcFlags & (NPCFLAG_MOVING | NPCFLAG_MOVE_START | NPCFLAG_MOVE_LOOP | NPCFLAG_MOVE_FINISH
			| NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT | NPCFLAG_MOVE_END))
			&& _visible && _state == PARROT_IN_CAGE) {
		if (compareViewNameTo("ParrotLobby.Node 1.N"))
			msg->_names = NAMES;
	}

	return true;
}

bool CParrot::NPCPlayIdleAnimationMsg(CNPCPlayIdleAnimationMsg *msg) {
	static const char *const NAMES[] = {
		"Idle0", "Idle1", "Peck At Feet", "Peck At Feet Left",
		"Peck At Feet Right", nullptr
	};

	if (!(_npcFlags & (NPCFLAG_MOVING | NPCFLAG_MOVE_START | NPCFLAG_MOVE_LOOP | NPCFLAG_MOVE_FINISH
			| NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT | NPCFLAG_MOVE_END))
			&& _visible && _state == PARROT_IN_CAGE && compareViewNameTo("ParrotLobby.Node 1.N")) {
		CGameObject *dragItem = getDraggingObject();
		if (!dragItem || dragItem->getName() == "Chicken") {
			if (!_coreReplaced || getRandomNumber(3) != 0) {
				if (getRandomNumber(1)) {
					startTalking(this, 280267, findView());
				} else {
					msg->_names = NAMES;
				}
			} else {
				int id = -1;
				switch (getParrotResponse()) {
				case 0:
					id = 280107;
					break;
				case 1:
					id = 280106;
					break;
				case 2:
					id = 280115;
					break;
				case 3:
					id = 280114;
					break;
				case 4:
					id = 280113;
					break;
				case 5:
					id = 280112;
					break;
				case 6:
					id = 280111;
					break;
				case 7:
					id = 280110;
					break;
				case 8:
					id = 280109;
					break;
				case 9:
					id = 280108;
					break;
				case 10:
					id = 280105;
					break;
				case 11:
					id = 280000;
					break;
				default:
					break;
				}

				if (id != -1)
					startTalking(this, id, findView());

				CActMsg actMsg("FlashCore");
				actMsg.execute("PerchCoreHolder");
			}
		}
	}

	return true;
}

bool CParrot::FrameMsg(CFrameMsg *msg) {
	if (!compareViewNameTo("ParrotLobby.Node 1.N"))
		return false;
	if (_state != PARROT_IN_CAGE)
		return true;

	Point pt = getMousePos();
	CGameObject *dragObject = getDraggingObject();
	int xp = _bounds.left + _bounds.width() / 2;
	bool chickenFlag = false;

	if ((_npcFlags & NPCFLAG_MOVE_END) && !hasActiveMovie()) {
		_newXc =  _newXp + _bounds.width() / 2;
		int xDiff = ABS(xp - _newXc);

		if (xDiff < 64) {
			if (_panTarget) {
				CActMsg actMsg("PanAwayFromParrot");
				actMsg.execute(_panTarget);
				_panTarget = nullptr;
			}

			_npcFlags &= ~(NPCFLAG_MOVING | NPCFLAG_MOVE_START | NPCFLAG_MOVE_LOOP
				| NPCFLAG_MOVE_FINISH | NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT | NPCFLAG_MOVE_END);
			return true;
		}

		// WORKAROUND: Prevent panning away from stalling if Parrot was moving
		_npcFlags &= ~NPCFLAG_MOVING;
	} else {
		if (dragObject)
			chickenFlag = dragObject && dragObject->isEquals("Chicken");

		if (_npcFlags & NPCFLAG_CHICKEN_OUTSIDE_CAGE) {
			if (!chickenFlag || pt.x > 70 || pt.y < 90 || pt.y > 280) {
				// A start of eating the chicken outside the cage has to be
				// aborted because the chicken has been moved out of range
				stopMovie();
				loadFrame(0);
				setPosition(Point(-90, _bounds.top));
				_npcFlags &= ~NPCFLAG_CHICKEN_OUTSIDE_CAGE;
			}
		} else {
			if (!chickenFlag)
				return false;
		}

		_newXc = CLIP((int)pt.x, 230, 480);
	}

	if ((_npcFlags & NPCFLAG_MOVING) || hasActiveMovie())
		return true;

	if (ABS(_newXc - xp) > 64) {
		_npcFlags |= NPCFLAG_MOVING | NPCFLAG_MOVE_START;

		if (_newXc >= xp) {
			// WORKAROUND: Original did not properly reset the eating chicken
			// flag when the player turns away from the cage
			setEatingChicken(false);

			setPosition(Point(_bounds.left + 30, _bounds.top));
			_npcFlags |= NPCFLAG_MOVE_RIGHT;
			playClip("Walk Right Intro", MOVIE_NOTIFY_OBJECT);
		} else {
			_npcFlags |= NPCFLAG_MOVE_LEFT;
			playClip("Walk Left Intro", MOVIE_NOTIFY_OBJECT);
		}
	} else if (chickenFlag && pt.y >= 90 && pt.y <= 280 && !_triedEatChicken) {
		CParrotTriesChickenMsg triesMsg;
		triesMsg.execute(dragObject);

		CTrueTalkTriggerActionMsg triggerMsg;
		int &action = triggerMsg._action;
		switch (triesMsg._condiment) {
		case 1:
			action = triesMsg._isHot ? 280034 : 280056;
			break;
		case 2:
			action = triesMsg._isHot ? 280033 : 280055;
			break;
		case 3:
			action = triesMsg._isHot ? 280032 : 280054;
			break;
		default:
			action = triesMsg._isHot ? 280266 : 280053;
			break;
		}

		if (action == 280266) {
			if (pt.x < 75) {
				// Parrot needs to reach outside the cage
				_npcFlags |= NPCFLAG_CHICKEN_OUTSIDE_CAGE;
				playClip("Walk Left Intro", MOVIE_STOP_PREVIOUS);
				playClip("Walk Left Loop", MOVIE_NOTIFY_OBJECT);
				movieEvent(236);
				chickenFlag = false;
			} else if ((pt.x - xp) > 15) {
				_npcFlags |= NPCFLAG_PECKING;
				playClip("Peck At Feet Right", MOVIE_NOTIFY_OBJECT);
				movieEvent(170);
			} else if ((xp - pt.x) > 15) {
				_npcFlags |= NPCFLAG_PECKING;
				playClip("Peck At Feet Left", MOVIE_NOTIFY_OBJECT);
				movieEvent(142);
			} else {
				_npcFlags |= NPCFLAG_PECKING;
				playClip("Peck At Feet", MOVIE_NOTIFY_OBJECT);
				movieEvent(157);
			}
		}

		if (chickenFlag) {
			triggerMsg._param2 = 1;
			triggerMsg.execute(this);
			_triedEatChicken = true;
		}
	}

	return true;
}

bool CParrot::MovieFrameMsg(CMovieFrameMsg *msg) {
	if (_npcFlags & NPCFLAG_PECKING) {
		// Whoopsy, the Parrot got your chicken
		CCarry *chicken = dynamic_cast<CCarry *>(findUnder(getRoot(), "Chicken"));
		if (chicken) {
			CActMsg actMsg("Eaten");
			actMsg.execute(chicken);
		}

		_npcFlags &= ~NPCFLAG_PECKING;
	}

	switch (msg->_frameNumber) {
	case 244:
		setPosition(Point(_bounds.left + 45, _bounds.top));
		break;
	case 261:
	case 262:
	case 265:
	case 268:
	case 271:
		setPosition(Point(_bounds.left + _eatOffsetX, _bounds.top));
		break;
	default:
		break;
	}

	return true;
}

bool CParrot::PutParrotBackMsg(CPutParrotBackMsg *msg) {
	const char *const NAMES[] = {
		"Talking0", "Talking1", "Talking2", "Talking3", "Talking4",
		"Talking5", "Talking6", "Talking7", nullptr
	};

	int xp = CLIP(msg->_value, 230, 480);
	setVisible(true);
	moveToView();
	_state = PARROT_IN_CAGE;

	setPosition(Point(xp - _bounds.width() / 2, _bounds.top));
	playRandomClip(NAMES, MOVIE_NOTIFY_OBJECT);

	CActMsg actMsg("GainParrot");
	actMsg.execute("ParrotLobbyController");

	return true;
}

bool CParrot::PreEnterViewMsg(CPreEnterViewMsg *msg) {
	if (_state == PARROT_IN_CAGE) {
		loadMovie(TRANSLATE("z167.avi", "z190.avi"), false);
		loadFrame(0);
	}

	return true;
}

bool CParrot::PanningAwayFromParrotMsg(CPanningAwayFromParrotMsg *msg) {
	if (_state != PARROT_IN_CAGE) {
		CActMsg actMsg("PanAwayFromParrot");
		actMsg.execute(msg->_target);
		_panTarget = nullptr;
	} else if (_takeOff) {
		_panTarget = msg->_target;
		loadMovie(TRANSLATE("z168.avi", "z191.avi"), false);
		stopMovie();
		playClip("Take Off", MOVIE_NOTIFY_OBJECT);
		_npcFlags |= NPCFLAG_TAKE_OFF;
	} else {
		_npcFlags |= NPCFLAG_MOVE_END;
		_panTarget = msg->_target;
		stopMovie();
	}

	return true;
}

bool CParrot::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	if (_state == PARROT_IN_CAGE)
		startTalking(this, 280259);

	return true;
}

bool CParrot::TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg) {
	// WORKAROUND: Fix parrot freezing up if you drag the chicken whilst
	// he's still returning to the center from a prior chicken drag
	if (_npcFlags & (NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT))
		_npcFlags &= ~(NPCFLAG_MOVING | NPCFLAG_MOVE_LEFT | NPCFLAG_MOVE_RIGHT);

	return CTrueTalkNPC::TrueTalkNotifySpeechStartedMsg(msg);
}

bool CParrot::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	if (msg->_dialogueId == 80022) {
		// WORKAROUND: End of parrot speech after having fixed Titania
		unlockMouse();
		changeView("Titania.Node 18.N", "");
	}

	return CTrueTalkNPC::TrueTalkNotifySpeechEndedMsg(msg);
}

void CParrot::setEatingChicken(bool eating) {
	_eatingChicken = eating;
	CStatusChangeMsg statusMsg;
	statusMsg._newStatus = eating ? 0 : 1;
	statusMsg.execute("PerchCoreHolder");
}

} // End of namespace Titanic
