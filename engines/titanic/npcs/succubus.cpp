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

#include "titanic/npcs/succubus.h"
#include "titanic/carry/carry.h"
#include "titanic/carry/chicken.h"
#include "titanic/core/view_item.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSuccUBus, CTrueTalkNPC)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(SubAcceptCCarryMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(PETDeliverMsg)
	ON_MESSAGE(PETReceiveMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(TrueTalkGetStateValueMsg)
	ON_MESSAGE(SignalObject)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(SUBTransition)
	ON_MESSAGE(SetChevRoomBits)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

bool CSuccUBus::_isOn;				// SuccUBus turned on
bool CSuccUBus::_motherBlocked;		// Bilge SuccUBus is blocked
bool CSuccUBus::_fuseboxOn;			// SuccUBus dial in fusebox is on

CSuccUBus::CSuccUBus() : CTrueTalkNPC() {
	_initialStartFrame = -1;
	_initialEndFrame = -1;
	_endingStartFrame = -1;
	_endingEndFrame = -1;
	_sendStartFrame = 68;
	_sendEndFrame = 168;
	_receiveStartFrame = 168;
	_receiveEndFrame = 248;
	_onStartFrame = 0;
	_onEndFrame = 0x0E;
	_offStartFrame = 0x0E;
	_offEndFrame = 27;
	_okStartFrame = 40;
	_okEndFrame = 68;
	_flagsComparison = RFC_LOCATION;
	_mailP = nullptr;
	_afterReceiveStartFrame = 0;
	_afterReceiveEndFrame = 0;
	_trayOutStartFrame = 224;
	_trayOutEndFrame = 248;
	_sendAction = SA_SENT;
	_signalFlag = false;
	_signalTarget = "NULL";
	_startFrame1 = 28;
	_endFrame1 = 40;
	_rect1 = Rect(82, 284, 148, 339);
	_field184 = 15;
	_mailPresent = false;
	_rect2 = Rect(0, 0, 240, 340);
	_sendLost = false;
	_soundHandle = -1;
	_isChicken = false;
	_isFeathers = false;
	_priorRandomVal1 = 0;
	_priorRandomVal2 = 0;
	_emptyStartFrame = 303;
	_emptyEndFrame = 312;
	_smokeStartFrame = 313;
	_smokeEndFrame = 325;
	_hoseStartFrame = 326;
	_hoseEndFrame = 347;
	_pumpingStartFrame = 348;
	_pumpingEndFrame = 375;
	_destRoomFlags = 1;
	_inProgress = false;
}

void CSuccUBus::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);

	file->writeNumberLine(_isOn, indent);
	file->writeNumberLine(_initialStartFrame, indent);
	file->writeNumberLine(_initialEndFrame, indent);
	file->writeNumberLine(_endingStartFrame, indent);
	file->writeNumberLine(_endingEndFrame, indent);
	file->writeNumberLine(_sendStartFrame, indent);
	file->writeNumberLine(_sendEndFrame, indent);
	file->writeNumberLine(_receiveStartFrame, indent);
	file->writeNumberLine(_receiveEndFrame, indent);
	file->writeNumberLine(_onStartFrame, indent);
	file->writeNumberLine(_onEndFrame, indent);
	file->writeNumberLine(_offStartFrame, indent);
	file->writeNumberLine(_offEndFrame, indent);
	file->writeNumberLine(_okStartFrame, indent);
	file->writeNumberLine(_okEndFrame, indent);
	file->writeNumberLine(_flagsComparison, indent);

	file->writeNumberLine(_motherBlocked, indent);
	file->writeNumberLine(_afterReceiveStartFrame, indent);
	file->writeNumberLine(_afterReceiveEndFrame, indent);
	file->writeNumberLine(_trayOutStartFrame, indent);
	file->writeNumberLine(_trayOutEndFrame, indent);
	file->writeNumberLine(_sendAction, indent);
	file->writeNumberLine(_signalFlag, indent);
	file->writeQuotedLine(_signalTarget, indent);
	file->writeNumberLine(_startFrame1, indent);
	file->writeNumberLine(_endFrame1, indent);
	file->writeNumberLine(_rect1.left, indent);
	file->writeNumberLine(_rect1.top, indent);
	file->writeNumberLine(_rect1.right, indent);
	file->writeNumberLine(_rect1.bottom, indent);
	file->writeNumberLine(_field184, indent);
	file->writeNumberLine(_mailPresent, indent);
	file->writeNumberLine(_rect2.left, indent);
	file->writeNumberLine(_rect2.top, indent);
	file->writeNumberLine(_rect2.right, indent);
	file->writeNumberLine(_rect2.bottom, indent);
	file->writeNumberLine(_sendLost, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_isChicken, indent);
	file->writeNumberLine(_isFeathers, indent);
	file->writeNumberLine(_priorRandomVal1, indent);
	file->writeNumberLine(_priorRandomVal2, indent);
	file->writeNumberLine(_emptyStartFrame, indent);
	file->writeNumberLine(_emptyEndFrame, indent);
	file->writeNumberLine(_smokeStartFrame, indent);
	file->writeNumberLine(_smokeEndFrame, indent);
	file->writeNumberLine(_hoseStartFrame, indent);
	file->writeNumberLine(_hoseEndFrame, indent);
	file->writeNumberLine(_pumpingStartFrame, indent);
	file->writeNumberLine(_pumpingEndFrame, indent);
	file->writeNumberLine(_destRoomFlags, indent);

	file->writeNumberLine(_fuseboxOn, indent);
	file->writeNumberLine(_inProgress, indent);
	file->writeNumberLine(_field104, indent);

	CTrueTalkNPC::save(file, indent);
}

void CSuccUBus::load(SimpleFile *file) {
	file->readNumber();

	_isOn = file->readNumber();
	_initialStartFrame = file->readNumber();
	_initialEndFrame = file->readNumber();
	_endingStartFrame = file->readNumber();
	_endingEndFrame = file->readNumber();
	_sendStartFrame = file->readNumber();
	_sendEndFrame = file->readNumber();
	_receiveStartFrame = file->readNumber();
	_receiveEndFrame = file->readNumber();
	_onStartFrame = file->readNumber();
	_onEndFrame = file->readNumber();
	_offStartFrame = file->readNumber();
	_offEndFrame = file->readNumber();
	_okStartFrame = file->readNumber();
	_okEndFrame = file->readNumber();
	_flagsComparison = (RoomFlagsComparison)file->readNumber();

	_motherBlocked = file->readNumber();
	_afterReceiveStartFrame = file->readNumber();
	_afterReceiveEndFrame = file->readNumber();
	_trayOutStartFrame = file->readNumber();
	_trayOutEndFrame = file->readNumber();
	_sendAction = (SuccUBusAction)file->readNumber();
	_signalFlag = file->readNumber();

	_signalTarget = file->readString();
	_startFrame1 = file->readNumber();
	_endFrame1 = file->readNumber();
	_rect1.left = file->readNumber();
	_rect1.top = file->readNumber();
	_rect1.right = file->readNumber();
	_rect1.bottom = file->readNumber();
	_field184 = file->readNumber();
	_mailPresent = file->readNumber();
	_rect2.left = file->readNumber();
	_rect2.top = file->readNumber();
	_rect2.right = file->readNumber();
	_rect2.bottom = file->readNumber();
	_sendLost = file->readNumber();
	_soundHandle = file->readNumber();
	_isChicken = file->readNumber();
	_isFeathers = file->readNumber();
	_priorRandomVal1 = file->readNumber();
	_priorRandomVal2 = file->readNumber();
	_emptyStartFrame = file->readNumber();
	_emptyEndFrame = file->readNumber();
	_smokeStartFrame = file->readNumber();
	_smokeEndFrame = file->readNumber();
	_hoseStartFrame = file->readNumber();
	_hoseEndFrame = file->readNumber();
	_pumpingStartFrame = file->readNumber();
	_pumpingEndFrame = file->readNumber();
	_destRoomFlags = file->readNumber();

	_fuseboxOn = file->readNumber();
	_inProgress = file->readNumber();
	_field104 = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CSuccUBus::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_inProgress)
		return true;

	Rect tempRect = _rect1;
	tempRect.translate(_bounds.left, _bounds.top);

	if (!_isOn) {
		CTurnOn onMsg;
		onMsg.execute(this);
		_isOn = true;
		return true;
	}

	if (_mailPresent && tempRect.contains(msg->_mousePos))
		return true;

	if (getRandomNumber(256) < 130) {
		_isOn = false;
		CTurnOff offMsg;
		offMsg.execute(this);
	} else {
		switch (getRandomNumber(2, &_priorRandomVal1)) {
		case 0:
			startTalking(this, 230055, findView());
			break;
		case 1:
			startTalking(this, 230067, findView());
			break;
		case 2:
			startTalking(this, 230045, findView());
			break;
		default:
			break;
		}
	}

	return true;
}

bool CSuccUBus::SubAcceptCCarryMsg(CSubAcceptCCarryMsg *msg) {
	if (!msg->_item)
		return false;

	CPetControl *pet = getPetControl();
	CCarry *item = dynamic_cast<CCarry *>(msg->_item);
	Rect tempRect = _rect2;
	tempRect.translate(_bounds.left, _bounds.top);
	uint roomFlags = pet ? pet->getRoomFlags() : 0;

	if (!_isOn || !pet || !item || !tempRect.contains(item->getControid())) {
		item->petAddToInventory();
	} else if (mailExists(roomFlags)) {
		petDisplayMessage(SUCCUBUS_DESCRIPTION);
		item->petAddToInventory();
	} else {
		petContainerRemove(item);
		pet->phonographAction("");

		CChicken *chicken = dynamic_cast<CChicken *>(item);
		bool chickenFlag = chicken ? chicken->_condiment == "None" : false;

		item->setVisible(false);
		if (_startFrame1 >= 0) {
			playSound(TRANSLATE("z#23.wav", "z#554.wav"));
			playMovie(_startFrame1, _endFrame1, 0);
		}

		if (!chickenFlag) {
			// Not chicken, or chicken with condiments
			_mailPresent = true;
			item->addMail(roomFlags);
			petSetArea(PET_REMOTE);
			petHighlightGlyph(16);
			CSUBTransition transMsg;
			transMsg.execute(this);
		} else {
			// Chicken without condiments was passed to SuccUBus, so it'll
			// eat it immediately
			if (_okStartFrame >= 0) {
				startTalking(this, 70219, findView());
				playMovie(_okStartFrame, _okEndFrame, 0);
			}

			if (_sendStartFrame >= 0) {
				playMovie(_sendStartFrame, _sendEndFrame, MOVIE_NOTIFY_OBJECT);
				_sendAction = SA_EATEN;
			}

			// WORKAROUND: The original had code below to return the chicken
			// to the dispensor after eaten, but since _fullViewName isn't
			// set, it didn't work. I've added code below in the else block
			// that replicates what happens when the parrot eats the chicken
			CViewItem *view = parseView(chicken->_fullViewName);
			if (view) {
				item->setPosition(item->_origPos);
				item->moveUnder(view);

				CSUBTransition transMsg;
				transMsg.execute(this);
			} else {
				CActMsg actMsg("Eaten");
				actMsg.execute(chicken);
				return false;
			}
		}
	}

	return true;
}

bool CSuccUBus::EnterViewMsg(CEnterViewMsg *msg) {
	if (getRandomNumber(4) == 0 && compareRoomNameTo("PromenadeDeck")) {
		CParrotSpeakMsg speakMsg("SuccUBus", "EnterView");
		speakMsg.execute("PerchedParrot");
	}

	petSetRemoteTarget();
	_mailP = nullptr;
	if (_initialStartFrame >= 0)
		loadFrame(_initialStartFrame);

	return true;
}

bool CSuccUBus::LeaveViewMsg(CLeaveViewMsg *msg) {
	petDisplayMessage(2, BLANK);
	if (_initialStartFrame >= 0)
		loadFrame(_initialStartFrame);
	else if (!_signalFlag && _onStartFrame >= 0)
		loadFrame(_onStartFrame);

	petClear();
	if (_soundHandle != -1) {
		stopSound(_soundHandle, 1);
		_soundHandle = -1;
	}

	if (_isOn) {
		_isOn = false;
		if (_offStartFrame >= 0)
			playSound(TRANSLATE("z#27.wav", "z#558.wav"), 100);

		if (_signalFlag)
			setVisible(false);
	}

	performAction(true, findView());
	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

bool CSuccUBus::PETDeliverMsg(CPETDeliverMsg *msg) {
	if (_inProgress)
		return true;

	if (!_isOn) {
		petDisplayMessage(2, SUCCUBUS_IS_IN_STANDBY);
		return true;
	}

	CPetControl *pet = getPetControl();
	if (!pet)
		return true;

	uint srcRoomFlags = pet->getRoomFlags();
	CGameObject *mailObject = findMail(srcRoomFlags);
	if (!mailObject) {
		// Nothing to send
		switch (getRandomNumber(2)) {
		case 0:
			startTalking(this, 70111, findView());
			break;
		case 1:
			startTalking(this, 70112, findView());
			break;
		case 2:
			startTalking(this, 70113, findView());
			break;
		default:
			break;
		}

		petDisplayMessage(2, NOTHING_IN_SUCCUBUS_TRAY);
	} else {
		_sendLost = false;

		CRoomFlags roomFlags = _destRoomFlags;
		if (!pet->isSuccUBusDest(roomFlags) || pet->getMailDestClass(roomFlags) < getPassengerClass()) {
			roomFlags = pet->getSpecialRoomFlags("BilgeRoom");
			_sendLost = true;
		}

		_isFeathers = mailObject->getName() == "Feathers";
		_isChicken = mailObject->getName() == "Chicken";
		_sendAction = SA_SENT;
		_mailPresent = false;
		_inProgress = true;
		lockMouse();

		if (_isFeathers) {
			// The feather has special handling to be rejected by the SuccUBus
			_sendLost = false;
			sendMail(srcRoomFlags, roomFlags);
			pet->phonographAction("");

			if (_okStartFrame >= 0) {
				playMovie(_okStartFrame, _okEndFrame, 0);
				startTalking(this, 230022, findView());
			}

			_sendAction = SA_FEATHERS;
			if (_sendStartFrame >= 0)
				playMovie(_sendStartFrame, _sendEndFrame, 0);

			if (_receiveStartFrame >= 0) {
				_mailP = mailObject;
				playMovie(_receiveStartFrame, _receiveEndFrame, MOVIE_NOTIFY_OBJECT);
			}

			if (_afterReceiveStartFrame >= 0) {
				playMovie(_afterReceiveStartFrame, _afterReceiveEndFrame, 0);
			}
		} else {
			// Send the mail to the destination
			sendMail(pet->getRoomFlags(), roomFlags);
			pet->phonographAction("");

			if (_okStartFrame >= 0) {
				playMovie(_okStartFrame, _okEndFrame, 0);
				startTalking(this, 230012, findView());
			}

			if (_sendStartFrame >= 0)
				playMovie(_sendStartFrame, _sendEndFrame, MOVIE_NOTIFY_OBJECT);
		}
	}

	return true;
}

bool CSuccUBus::PETReceiveMsg(CPETReceiveMsg *msg) {
	CPetControl *pet = getPetControl();

	if (_inProgress || !pet)
		return true;
	if (!_isOn) {
		petDisplayMessage(2, SUCCUBUS_IS_IN_STANDBY);
		return true;
	}

	uint petRoomFlags = pet->getRoomFlags();
	if (mailExists(petRoomFlags)) {
		// There's already something to send in the tray, so you can't receive
		switch (getRandomNumber(2)) {
		case 0:
			startTalking(this, 70080, findView());
			break;
		case 1:
			startTalking(this, 70081, findView());
			break;
		case 2:
			startTalking(this, 70082, findView());
			break;
		default:
			break;
		}
	} else {
		// When the SuccUBus dial in Titania's fusebox is on, then
		// any mail can be received by the SuccUBus in the bomb room.
		// Otherwise, only get mail sent to this specific SuccUBus
		CGameObject *mailObject = findMailByFlags(
			_fuseboxOn && compareRoomNameTo("Titania") ? RFC_TITANIA : _flagsComparison, petRoomFlags);

		if (!mailObject) {
			// No mail for this SuccUBus
			if (getRandomNumber(1) == 0) {
				startTalking(this, 70104, findView());
			} else {
				startTalking(this, 70105, findView());
			}

			playMovie(_emptyStartFrame, _emptyEndFrame, 0);
			playMovie(_smokeStartFrame, _smokeEndFrame, 0);
			petDisplayMessage(2, NOTHING_TO_DELIVER);
		} else {
			// Receive the mail addressed to this SuccUBus
			_mailP = mailObject;
			startTalking(this, 230004, findView());

			if (_receiveStartFrame >= 0) {
				_sendAction = SA_FEATHERS;
				_inProgress = true;
				lockMouse();
				playMovie(_receiveStartFrame, _receiveEndFrame, MOVIE_NOTIFY_OBJECT);
			}
		}
	}

	return true;
}

bool CSuccUBus::MovieEndMsg(CMovieEndMsg *msg) {
	CPetControl *pet = getPetControl();
	uint petRoomFlags = pet ? pet->getRoomFlags() : 0;

	if (msg->_endFrame == _offEndFrame) {
		if (_endingStartFrame >= 0)
			playSound(TRANSLATE("z#30.wav", "z#561.wav"), 100);

		if (_signalFlag) {
			_signalFlag = false;
			setVisible(false);
			CSignalObject signalMsg;
			signalMsg._numValue = 1;
			signalMsg.execute(_signalTarget);
		}
	}

	if (msg->_endFrame == _onEndFrame) {
		bool flag = false;

		if (pet && !mailExists(petRoomFlags)) {
			CGameObject *mailObject = _fuseboxOn && compareRoomNameTo("Titania") ?
				findMailByFlags(RFC_TITANIA, petRoomFlags) :
				findMailByFlags(_flagsComparison, petRoomFlags);

			if (mailObject) {
				switch (getRandomNumber(4)) {
				case 0:
					startTalking(this, 70094, findView());
					break;
				case 1:
					startTalking(this, 70095, findView());
					break;
				case 2:
					startTalking(this, 70096, findView());
					break;
				case 3:
					startTalking(this, 70098, findView());
					break;
				case 4:
					startTalking(this, 70099, findView());
					break;
				default:
					break;
				}
				flag = true;
			}
		}

		if (!_mailPresent && !flag) {
			stopSound(_soundHandle);
			_soundHandle = -1;

			switch (getRandomNumber(_motherBlocked ? 7 : 5, &_priorRandomVal2)) {
			case 2:
				startTalking(this, 230001, findView());
				break;
			case 3:
				startTalking(this, 230002, findView());
				break;
			case 4:
				startTalking(this, 230003, findView());
				break;
			case 5:
				startTalking(this, 230064, findView());
				break;
			case 6:
				startTalking(this, 230062, findView());
				break;
			case 7:
				startTalking(this, 230063, findView());
				break;
			default:
				break;
			}
		}
	}

	if (msg->_endFrame == _sendEndFrame) {
		if (_sendAction == SA_FEATHERS) {
			startTalking(this, 230022, findView());
		} else if (_sendAction == SA_EATEN) {
			startTalking(this, 230017, findView());
		} else if (_sendLost) {
			startTalking(this, 230019, findView());
			_sendLost = false;
		} else if (_isChicken) {
			startTalking(this, 230018, findView());
			_isChicken = false;
		} else {
			startTalking(this, 230013, findView());
		}

		if (_inProgress) {
			_inProgress = false;
			unlockMouse();
		}

		CSUBTransition transMsg;
		transMsg.execute(this);
	}

	if (msg->_endFrame == _receiveEndFrame) {
		// SuccUBus disgorged mail
		if (pet && _mailP) {
			_mailP->setMailDest(petRoomFlags);
		}

		_mailPresent = true;
		_mailP = nullptr;
		if (_inProgress) {
			_inProgress = false;
			unlockMouse();
		}

		CSUBTransition transMsg;
		transMsg.execute(this);
	}

	return true;
}

bool CSuccUBus::TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg) {
	if (msg->_stateNum == 1)
		msg->_stateVal = _isOn;

	return true;
}

bool CSuccUBus::SignalObject(CSignalObject *msg) {
	if (msg->_numValue == 1) {
		_signalTarget = msg->_strValue;
		_signalFlag = true;
		setVisible(true);
		CTurnOn onMsg;
		onMsg.execute(this);
	}

	return true;
}

bool CSuccUBus::TurnOn(CTurnOn *msg) {
	if (getRandomNumber(9) == 0) {
		CParrotSpeakMsg speakMsg("SuccUBus", "TurnOn");
		speakMsg.execute("PerchedParrot");
	}

	CPetControl *pet = getPetControl();
	if (pet) {
		if (!_signalFlag && _initialStartFrame >= 0) {
			playMovie(_initialStartFrame, _initialEndFrame, 0);
			playSound(TRANSLATE("z#30.wav", "z#561.wav"), 100);
		}

		if (_onStartFrame >= 0) {
			playMovie(_onStartFrame, _onEndFrame, MOVIE_NOTIFY_OBJECT);
			playSound(TRANSLATE("z#26.wav", "z#557.wav"), 100);
		}

		uint petRoomFlags = pet->getRoomFlags();
		if (mailExists(petRoomFlags) && _endFrame1 >= 0)
			// Mail canister present
			playMovie(_endFrame1, _endFrame1, 0);

		_isOn = true;
		CSUBTransition transMsg;
		transMsg.execute(this);

		setTalking(this, true, findView());
		petSetArea(PET_REMOTE);
		petHighlightGlyph(16);
	}

	return true;
}

bool CSuccUBus::TurnOff(CTurnOff *msg) {
	if (_soundHandle != -1) {
		stopSound(_soundHandle);
		_soundHandle = -1;
	}

	if (_offStartFrame >= 0) {
		playSound(TRANSLATE("z#27.wav", "z#558.wav"), 100);
		playMovie(_offStartFrame, _offEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	}

	if (!_signalFlag && _endingStartFrame >= 0)
		playMovie(_endingStartFrame, _endingEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);

	_isOn = false;
	performAction(true);
	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

bool CSuccUBus::SUBTransition(CSUBTransition *msg) {
	CPetControl *pet = getPetControl();

	if (pet) {
		uint petRoomFlags = pet->getRoomFlags();

		if (_isOn) {
			CGameObject *mailObject = findMail(petRoomFlags);
			if (mailObject)
				pet->phonographAction("Send");
			else
				pet->phonographAction("Receive");
		} else {
			if (pet->isSuccUBusRoom(petRoomFlags))
				pet->phonographAction("Record");
			else
				pet->phonographAction("");
		}
	}

	return true;
}

bool CSuccUBus::SetChevRoomBits(CSetChevRoomBits *msg) {
	if (_isOn) {
		_destRoomFlags = msg->_roomFlags;
		playSound(TRANSLATE("z#98.wav", "z#629.wav"), 100);
	}

	return true;
}

bool CSuccUBus::ActMsg(CActMsg *msg) {
	if (msg->_action == "EnableObject")
		// SuccUBus dial in fusebox was turned on
		_fuseboxOn = true;
	else if (msg->_action == "DisableObject")
		// SuccUBus dial in fusebox was turned off
		_fuseboxOn = false;

	return true;
}

bool CSuccUBus::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	CPetControl *pet = getPetControl();
	Rect tempRect = _rect1;
	tempRect.translate(_bounds.left, _bounds.top);

	if (_inProgress || !_isOn || !_mailPresent || !tempRect.contains(msg->_mousePos)
			|| !pet)
		return true;

	uint petRoomFlags = pet->getRoomFlags();
	CGameObject *mailObject = findMail(petRoomFlags);
	if (!mailObject)
		return true;

	petAddToCarryParcel(mailObject);
	CViewItem *view = getView();
	if (!view)
		return true;

	mailObject->moveUnder(view);
	mailObject->setPosition(Point(msg->_mousePos.x + mailObject->_bounds.width() / 2,
		msg->_mousePos.y + mailObject->_bounds.height() / 2));

	CVisibleMsg visibleMsg(true);
	visibleMsg.execute(mailObject);
	CPassOnDragStartMsg dragMsg;
	dragMsg._mousePos = msg->_mousePos;
	dragMsg._value3 = 1;
	dragMsg.execute(mailObject);

	if (!dragMsg._value4)
		msg->_dragItem = mailObject;

	loadFrame(_field184);
	_mailPresent = false;
	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

} // End of namespace Titanic
