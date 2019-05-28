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
#include "titanic/translation.h"

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
		_sneezing2StartFrame(-1), _sneezing2EndFrame(-1),
		_sneezing1StartFrame(-1), _sneezing1EndFrame(-1) {
}

void CBilgeSuccUBus::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_sneezing2StartFrame, indent);
	file->writeNumberLine(_sneezing2EndFrame, indent);
	file->writeNumberLine(_sneezing1StartFrame, indent);
	file->writeNumberLine(_sneezing1EndFrame, indent);

	CSuccUBus::save(file, indent);
}

void CBilgeSuccUBus::load(SimpleFile *file) {
	file->readNumber();
	_sneezing2StartFrame = file->readNumber();
	_sneezing2EndFrame = file->readNumber();
	_sneezing1StartFrame = file->readNumber();
	_sneezing1EndFrame = file->readNumber();

	CSuccUBus::load(file);
}

bool CBilgeSuccUBus::FrameMsg(CFrameMsg *msg) {
	return true;
}

bool CBilgeSuccUBus::PETReceiveMsg(CPETReceiveMsg *msg) {
	CPetControl *pet = getPetControl();

	if (_motherBlocked) {
		// Mother hasn't yet been unblocked, so don't receive anything
		if (_receiveStartFrame >= 0)
			playMovie(_receiveStartFrame, _receiveEndFrame, MOVIE_WAIT_FOR_FINISH);
		if (_afterReceiveStartFrame >= 0)
			playMovie(_afterReceiveStartFrame, _afterReceiveEndFrame, MOVIE_WAIT_FOR_FINISH);

		playSound(TRANSLATE("z#28.wav", "z#559.wav"), 70);
	} else if (!_isOn) {
		petDisplayMessage(2, SUCCUBUS_IS_IN_STANDBY);
		return false;
	} else if (!pet) {
		return false;
	} else {
		uint roomFlags = pet->getRoomFlags();
		CGameObject *mailObject = findMailByFlags(
			_fuseboxOn && compareRoomNameTo("Titania") ? RFC_TITANIA : _flagsComparison,
			roomFlags);

		if (mailObject) {
			startTalking(this, 230004);
			_mailP = mailObject;
			if (_receiveStartFrame >= 0)
				playMovie(_receiveStartFrame, _receiveEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		} else {
			petDisplayMessage(2, NOTHING_TO_DELIVER);
		}
	}

	return true;
}

bool CBilgeSuccUBus::PETDeliverMsg(CPETDeliverMsg *msg) {
	CPetControl *pet = getPetControl();
	if (!_isOn || !pet)
		return true;

	uint petRoomFlags = pet->getRoomFlags();
	CGameObject *mailObject = findMail(petRoomFlags);

	if (!mailObject) {
		petDisplayMessage(2, NOTHING_IN_SUCCUBUS_TRAY);
		return true;
	}

	_sendLost = false;
	_mailP = mailObject;

	uint roomFlags = _destRoomFlags;
	if (!pet->isSuccUBusDest(roomFlags) || getPassengerClass() > pet->getMailDestClass(roomFlags)) {
		roomFlags = pet->getSpecialRoomFlags("BilgeRoom");
		_sendLost = true;
	}

	_isChicken = mailObject->getName() == "Chicken";
	_isFeathers = mailObject->getName() == "Feathers";
	_sendAction = SA_SENT;

	if (_motherBlocked) {
		if (_isFeathers) {
			startTalking(this, 230022);
			_sendAction = SA_FEATHERS;

			if (_sendStartFrame >= 0)
				playMovie(_sendStartFrame, _sendEndFrame, MOVIE_NOTIFY_OBJECT);

			if (_sneezing2StartFrame >= 0) {
				playMovie(_trayOutStartFrame, _trayOutEndFrame, MOVIE_WAIT_FOR_FINISH);
				playMovie(_sneezing1StartFrame, _sneezing1EndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
				playMovie(_sneezing2StartFrame, _sneezing2EndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
				lockMouse();
			}
		} else {
			startTalking(this, 230012);
			_sendAction = SA_EATEN;
			if (_sendStartFrame >= 0)
				playMovie(_sendStartFrame, _sendEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			if (_receiveStartFrame >= 0)
				playMovie(_receiveStartFrame, _receiveEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			if (_afterReceiveStartFrame >= 0)
				playMovie(_afterReceiveStartFrame, _afterReceiveEndFrame, MOVIE_WAIT_FOR_FINISH);
		}
	} else {
		if (_isFeathers) {
			startTalking(this, 230022);
			_sendAction = SA_BILGE_FEATHERS;

			if (_sendStartFrame >= 0)
				playMovie(_sendStartFrame, _sendEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			if (_receiveStartFrame >= 0)
				playMovie(_receiveStartFrame, _receiveEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			if (_afterReceiveStartFrame >= 0)
				playMovie(_afterReceiveStartFrame, _afterReceiveEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		} else {
			sendMail(petRoomFlags, roomFlags);
			startTalking(this, 230012);
			if (_sendStartFrame >= 0) {
				_sendAction = SA_BILGE_SENT;
				playMovie(_sendStartFrame, _sendEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			}
		}
	}

	return true;
}

bool CBilgeSuccUBus::MovieEndMsg(CMovieEndMsg *msg) {
	CPetControl *pet = getPetControl();

	if (msg->_endFrame == _trayOutEndFrame) {
		if (_offStartFrame >= 0)
			playSound(TRANSLATE("z#27.wav", "z#558.wav"));
	} else if (msg->_endFrame == _offEndFrame) {
		if (_endingStartFrame >= 0)
			playSound(TRANSLATE("z#30.wav", "z#561.wav"));
	} else {
		if (msg->_endFrame == _onEndFrame && pet) {
			if (_motherBlocked) {
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

		} else if (msg->_endFrame == _sendEndFrame) {
			switch (_sendAction) {
			case SA_EATEN:
				stopSound(_soundHandle, 1);
				_soundHandle = playSound(TRANSLATE("z#3.wav", "z#539.wav"), 1);
				break;
			case SA_BILGE_FEATHERS:
				stopSound(_soundHandle);
				_soundHandle = playSound(TRANSLATE("z#12.wav", "z#532.wav"));
				break;
			case SA_BILGE_SENT:
				if (_isChicken) {
					startTalking(this, 230018);
					_isChicken = false;
				} else {
					startTalking(this, 230013);
				}
				break;
			case SA_BILGE_EATEN:
				startTalking(this, 230017);
				break;
			default:
				break;
			}

			CSUBTransition transMsg;
			transMsg.execute(this);

		} else if (msg->_endFrame == _receiveEndFrame) {
			if (_mailP) {
				_mailP->petAddToInventory();
				CVisibleMsg visibleMsg(true);
				visibleMsg.execute(_mailP);

				_mailP = nullptr;
				petSetArea(PET_INVENTORY);

				CSUBTransition transMsg;
				transMsg.execute(this);
			}

		} else if (msg->_endFrame == _sneezing1EndFrame) {
			playSound(TRANSLATE("z#25.wav", "z#556.wav"), 70);
			playSound(TRANSLATE("z#24.wav", "z#555.wav"), 70);

		} else if (msg->_endFrame == _sneezing2EndFrame) {
			changeView("BilgeRoomWith.Node 1.N", "");
			_motherBlocked = false;
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
			unlockMouse();
			_sendAction = SA_SENT;

		} else {
			_sendAction = SA_SENT;
		}
	}

	return true;
}

bool CBilgeSuccUBus::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_isOn) {
		switch (getRandomNumber(4)) {
		case 0:
		case 4: {
			_isOn = false;
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
		_isOn = true;
	}

	return true;
}

bool CBilgeSuccUBus::SubAcceptCCarryMsg(CSubAcceptCCarryMsg *msg) {
	CPetControl *pet = getPetControl();
	if (!msg->_item)
		return false;

	CCarry *item = dynamic_cast<CCarry *>(msg->_item);
	if (!_isOn || !pet || !item) {
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
	item->setVisible(false);
	playSound(TRANSLATE("z#23.wav", "z#554.wav"));

	CChicken *chicken = dynamic_cast<CChicken *>(item);
	bool chickenFlag = chicken ? chicken->_condiment == "None" : false;

	if (chickenFlag) {
		if (_okStartFrame >= 0) {
			startTalking(this, 70219);
			playMovie(_okStartFrame, _okEndFrame, 0);
		}

		if (_sendStartFrame >= 0) {
			_sendAction = SA_BILGE_EATEN;
			playMovie(_sendStartFrame, _sendEndFrame, MOVIE_NOTIFY_OBJECT);
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
		if (_okStartFrame >= 0)
			playMovie(_okStartFrame, _okEndFrame, 0);

		petSetArea(PET_REMOTE);
		CSUBTransition transMsg;
		transMsg.execute(this);
	}

	return true;
}

bool CBilgeSuccUBus::EnterViewMsg(CEnterViewMsg *msg) {
	petSetRemoteTarget();
	_mailP = nullptr;

	if (_initialStartFrame >= 0)
		loadFrame(_initialStartFrame);

	return true;
}

bool CBilgeSuccUBus::LeaveViewMsg(CLeaveViewMsg *msg) {
	petDisplayMessage(2, BLANK);
	petClear();

	if (_soundHandle != -1) {
		stopSound(_soundHandle);
		_soundHandle = -1;
	}

	if (_isOn) {
		_isOn = false;
		if (_offStartFrame >= 0)
			playSound(TRANSLATE("z#27.wav", "z#558.wav"));
	}

	performAction(true);
	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

bool CBilgeSuccUBus::TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg) {
	if (msg->_stateNum == 1)
		msg->_stateVal = _isOn;

	return true;
}

bool CBilgeSuccUBus::TurnOn(CTurnOn *msg) {
	CPetControl *pet = getPetControl();

	if (pet) {
		if (_onStartFrame >= 0) {
			playMovie(_onStartFrame, _onEndFrame, MOVIE_NOTIFY_OBJECT);
			playSound(TRANSLATE("z#26.wav", "z#557.wav"));
		}

		if (mailExists(pet->getRoomFlags()) && _okStartFrame >= 0)
			playMovie(_okStartFrame, _okEndFrame, 0);

		_isOn = true;
		CSUBTransition transMsg;
		transMsg.execute(this);

		setTalking(this, true);
		petSetArea(PET_REMOTE);
		petHighlightGlyph(16);
	}

	return true;
}

bool CBilgeSuccUBus::TurnOff(CTurnOff *msg) {
	CPetControl *pet = getPetControl();

	if (pet && mailExists(pet->getRoomFlags()) && _trayOutStartFrame >= 0)
		playMovie(_trayOutStartFrame, _trayOutEndFrame, MOVIE_NOTIFY_OBJECT);
	else if (_trayOutEndFrame >= 0)
		playMovie(_trayOutEndFrame, _trayOutEndFrame, MOVIE_NOTIFY_OBJECT);

	if (_soundHandle != -1) {
		stopSound(_soundHandle);
		_soundHandle = -1;
	}

	if (_offStartFrame >= 0)
		playMovie(_offStartFrame, _offEndFrame, MOVIE_NOTIFY_OBJECT);

	_isOn = false;
	performAction(true);

	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

} // End of namespace Titanic
