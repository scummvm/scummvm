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

#include "titanic/npcs/bilge_succubus.h"
#include "titanic/carry/chicken.h"
#include "titanic/core/view_item.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBilgeSuccUBus, CSuccUBus)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(PETReceiveMsg)
	ON_MESSAGE(PETDeliverMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(SubAcceptCCarryMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(TrueTalkGetStateValueMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

CBilgeSuccUBus::CBilgeSuccUBus() : CSuccUBus(),
		_bilgeStartFrame1(-1), _bilgeEndFrame1(-1),
		_bilgeStartFrame2(-1), _bilgeEndFrame2(-1) {
}

void CBilgeSuccUBus::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_bilgeStartFrame1, indent);
	file->writeNumberLine(_bilgeEndFrame1, indent);
	file->writeNumberLine(_bilgeStartFrame2, indent);
	file->writeNumberLine(_bilgeEndFrame2, indent);

	CSuccUBus::save(file, indent);
}

void CBilgeSuccUBus::load(SimpleFile *file) {
	file->readNumber();
	_bilgeStartFrame1 = file->readNumber();
	_bilgeEndFrame1 = file->readNumber();
	_bilgeStartFrame2 = file->readNumber();
	_bilgeEndFrame2 = file->readNumber();

	CSuccUBus::load(file);
}

bool CBilgeSuccUBus::FrameMsg(CFrameMsg *msg) {
	return true;
}

bool CBilgeSuccUBus::PETReceiveMsg(CPETReceiveMsg *msg) {
	CPetControl *pet = getPetControl();

	if (_v2) {
		if (_startFrame4 >= 0)
			playMovie(_startFrame4, _endFrame4, MOVIE_GAMESTATE);
		if (_startFrame5 >= 0)
			playMovie(_startFrame5, _endFrame5, MOVIE_GAMESTATE);

		playSound("z#28.wav", 70);
	} else if (!_enabled) {
		petDisplayMessage(2, SUCCUBUS_IS_IN_STANDBY);
		return false;
	} else if (!pet) {
		return false;
	} else {
		uint roomFlags = pet->getRoomFlags();
		CGameObject *mailObject = findMailByFlags(
			_v3 && compareRoomNameTo("Titania") ? 3 : _field140,
			roomFlags);

		if (mailObject) {
			_mailP = mailObject;
			if (_startFrame4 >= 0)
				playMovie(_startFrame4, _endFrame4, MOVIE_GAMESTATE);
		} else {
			petDisplayMessage(2, NOTHING_TO_DELIVER);
		}
	}

	return true;
}

bool CBilgeSuccUBus::PETDeliverMsg(CPETDeliverMsg *msg) {
	CPetControl *pet = getPetControl();
	if (!_enabled || !pet)
		return true;

	uint petRoomFlags = pet->getRoomFlags();
	CGameObject *mailObject = findMail(petRoomFlags);

	if (!mailObject) {
		petDisplayMessage(2, NOTHING_IN_SUCCUBUS_TRAY);
		return true;
	}

	_field19C = 0;
	_mailP = mailObject;

	uint roomFlags = _roomFlags;
	if (!pet->testRooms5(roomFlags) ||
			getPassengerClass() > pet->getMailDest(roomFlags)) {
		roomFlags = pet->getSpecialRoomFlags("BilgeRoom");
		_field19C = 1;
	}

	_isChicken = mailObject->getName() == "Chicken";
	_isFeathers = mailObject->getName() == "Feathers";
	_field158 = 0;

	if (_v2) {
		if (_isFeathers) {
			startTalking(this, 230022);
			_field158 = 1;

			if (_startFrame3 >= 0)
				playMovie(_startFrame3, _endFrame3, MOVIE_NOTIFY_OBJECT);

			if (_bilgeStartFrame1 >= 0) {
				playMovie(_startFrame12, _endFrame12, MOVIE_GAMESTATE);
				playMovie(_bilgeStartFrame2, _bilgeEndFrame2, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
				playMovie(_bilgeStartFrame1, _bilgeEndFrame1, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
				inc54();
			}
		} else {
			startTalking(this, 230012);
			_field158 = 2;
			if (_startFrame3 >= 0)
				playMovie(_startFrame3, _endFrame3, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			if (_startFrame4 >= 0)
				playMovie(_startFrame4, _endFrame4, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			if (_startFrame5 >= 0)
				playMovie(_startFrame5, _endFrame5, MOVIE_GAMESTATE);
		}
	} else {
		if (_isFeathers) {
			startTalking(this, 230022);
			_field158 = 3;

			if (_startFrame3 >= 0)
				playMovie(_startFrame3, _endFrame3, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			if (_startFrame4 >= 0)
				playMovie(_startFrame4, _endFrame4, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			if (_startFrame5 >= 0)
				playMovie(_startFrame5, _endFrame5, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
		} else {
			removeMail(petRoomFlags, roomFlags);
			startTalking(this, 230012);
			if (_startFrame3 >= 0) {
				_field158 = 4;
				playMovie(_startFrame3, _endFrame3, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			}
		}
	}

	return true;
}

bool CBilgeSuccUBus::MovieEndMsg(CMovieEndMsg *msg) {
	CPetControl *pet = getPetControl();

	if (msg->_endFrame == _endFrame12) {
		if (_startFrame10 >= 0)
			playSound("z#27.wav");
	} else if (msg->_endFrame == _endFrame10) {
		if (_startFrame11 >= 0)
			playSound("z#30.wav");
	} else {
		if (_endFrame9 == _endFrame10 && pet) {
			if (_v2) {
				startTalking(this, getRandomNumber(1) ? 230062 : 230063);
			} else if (!findMail(pet->getRoomFlags())) {
				switch (getRandomNumber(4)) {
				case 0:
					startTalking(this, 230001);
					break;
				case 1:
					startTalking(this, 230002);
					break;
				case 2:
					startTalking(this, 230003);
					break;
				default:
					break;
				}
			}
		}

		if (msg->_endFrame == _endFrame3) {
			switch (_field158) {
			case 1:
				stopSound(_soundHandle);
				_soundHandle = playSound("z#3.wav");
				break;
			case 2:
				stopSound(_soundHandle);
				_soundHandle = playSound("z#12.wav");
				break;
			case 3:
				if (_isChicken) {
					startTalking(this, 230018);
					_isChicken = false;
				} else {
					startTalking(this, 230013);
				}
				break;
			case 4:
				startTalking(this, 230017);
				break;
			default:
				break;
			}

			CSUBTransition transMsg;
			transMsg.execute(this);

		} else if (msg->_endFrame == _bilgeEndFrame2) {
			playSound("z#25.wav", 70);
			playSound("z#24.wav", 70);

		} else if (msg->_endFrame == _endFrame4) {
			if (_mailP) {
				_mailP->petAddToInventory();
				CVisibleMsg visibleMsg(true);
				visibleMsg.execute(_mailP);

				_mailP = nullptr;
				petSetArea(PET_INVENTORY);

				CSUBTransition transMsg;
				transMsg.execute(this);
			}

		} else if (msg->_endFrame == _bilgeEndFrame1) {
			changeView("BilgeRoomWith.Node 1.N", "");
			_v2 = 0;
			resetMail();

			if (_mailP) {
				_mailP->petAddToInventory();
				CVisibleMsg visibleMsg(true);
				visibleMsg.execute(_mailP);

				_mailP = nullptr;
				petSetArea(PET_INVENTORY);
			}

			startTalking(this, 150);
			CBodyInBilgeRoomMsg bodyMsg;
			bodyMsg.execute("Service Elevator Entity");
			dec54();
			_field158 = 0;

		} else {
			_field158 = 0;
		}
	}

	return true;
}

bool CBilgeSuccUBus::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_enabled) {
		switch (getRandomNumber(4)) {
		case 0:
		case 4: {
			_enabled = false;
			CTurnOff offMsg;
			offMsg.execute(this);
			break;
		}

		case 1:
			startTalking(this, 230055);
			break;

		case 2:
			startTalking(this, 230067);
			break;

		case 3:
			startTalking(this, 230045);
			break;

		default:
			break;
		}
	} else {
		CTurnOn onMsg;
		onMsg.execute(this);
		_enabled = true;
	}

	return true;
}

bool CBilgeSuccUBus::SubAcceptCCarryMsg(CSubAcceptCCarryMsg *msg) {
	CPetControl *pet = getPetControl();
	if (!msg->_item)
		return false;

	CCarry *item = dynamic_cast<CCarry *>(msg->_item);
	if (!_enabled || !pet || !item) {
		item->petAddToInventory();
		return true;
	}

	uint petRoomFlags = pet->getRoomFlags();
	if (mailExists(petRoomFlags)) {
		petDisplayMessage(2, SUCCUBUS_SINGLE_DELIVERY);
		item->petAddToInventory();
		return true;
	}

	petContainerRemove(item);
	pet->phonographAction("");
	playSound("z#23.wav");

	CChicken *chicken = dynamic_cast<CChicken *>(item);
	bool chickenFlag = chicken ? chicken->_string6 == "None" : false;

	if (chickenFlag) {
		if (_startFrame2 >= 0) {
			startTalking(this, 70219);
			playMovie(_startFrame2, _endFrame2, 0);
		}

		if (_startFrame3 >= 0) {
			_field158 = 5;
			playMovie(_startFrame3, _endFrame3, MOVIE_NOTIFY_OBJECT);
		}

		CViewItem *view = parseView(item->_fullViewName);
		if (view) {
			item->setVisible(false);
			setPosition(item->_origPos);
			item->moveUnder(view);

			CSUBTransition transMsg;
			transMsg.execute(this);
		} else {
			return false;
		}
	} else {
		item->addMail(petRoomFlags);
		if (_startFrame2 >= 0)
			playMovie(_startFrame2, _endFrame2, 0);

		petSetArea(PET_REMOTE);
		CSUBTransition transMsg;
		transMsg.execute(this);
	}

	return true;
}

bool CBilgeSuccUBus::EnterViewMsg(CEnterViewMsg *msg) {
	petSetRemoteTarget();
	_mailP = nullptr;

	if (_startFrame8 >= 0)
		loadFrame(_startFrame8);

	return true;
}

bool CBilgeSuccUBus::LeaveViewMsg(CLeaveViewMsg *msg) {
	petDisplayMessage(2, BLANK);
	petClear();

	if (_soundHandle != -1) {
		stopSound(_soundHandle);
		_soundHandle = -1;
	}

	if (_enabled) {
		_enabled = false;
		if (_startFrame10 >= 0)
			playSound("z#27.wav");
	}

	performAction(true);
	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

bool CBilgeSuccUBus::TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg) {
	if (msg->_stateNum == 1)
		msg->_stateVal = _enabled;

	return true;
}

bool CBilgeSuccUBus::TurnOn(CTurnOn *msg) {
	CPetControl *pet = getPetControl();

	if (pet) {
		if (_startFrame9 >= 0) {
			playMovie(_startFrame9, _endFrame9, MOVIE_NOTIFY_OBJECT);
			playSound("z#26.wav");
		}

		if (mailExists(pet->getRoomFlags()) && _startFrame2 >= 0)
			playMovie(_startFrame2, _endFrame2, 0);

		_enabled = true;
		CSUBTransition transMsg;
		transMsg.execute(this);

		endTalking(this, true);
		petSetArea(PET_REMOTE);
		petHighlightGlyph(16);
	}

	return true;
}

bool CBilgeSuccUBus::TurnOff(CTurnOff *msg) {
	CPetControl *pet = getPetControl();

	if (pet && mailExists(pet->getRoomFlags()) && _startFrame12 >= 0)
		playMovie(_startFrame12, _endFrame12, MOVIE_NOTIFY_OBJECT);
	else if (_endFrame12 >= 0)
		playMovie(_endFrame12, _endFrame12, MOVIE_NOTIFY_OBJECT);

	if (_soundHandle != -1) {
		stopSound(_soundHandle);
		_soundHandle = -1;
	}

	if (_startFrame10 >= 0)
		playMovie(_startFrame10, _endFrame10, MOVIE_NOTIFY_OBJECT);

	_enabled = false;
	performAction(true);

	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

} // End of namespace Titanic
