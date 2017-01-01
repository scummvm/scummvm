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

bool CSuccUBus::_enabled;
int CSuccUBus::_v1;
int CSuccUBus::_v2;
int CSuccUBus::_v3;
int CSuccUBus::_v4;

CSuccUBus::CSuccUBus() : CTrueTalkNPC() {
	_startFrame8 = -1;
	_endFrame8 = -1;
	_startFrame11 = -1;
	_endFrame11 = -1;
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
	_field140 = 1;
	_mailP = nullptr;
	_afterReceiveStartFrame = 0;
	_afterReceiveEndFrame = 0;
	_trayOutStartFrame = 224;
	_trayOutEndFrame = 248;
	_field158 = 0;
	_field15C = 0;
	_string2 = "NULL";
	_startFrame1 = 28;
	_endFrame1 = 40;
	_rect1 = Rect(82, 284, 148, 339);
	_field184 = 15;
	_field188 = 0;
	_rect2 = Rect(0, 0, 240, 340);
	_sendLost = false;
	_soundHandle = -1;
	_isChicken = false;
	_isFeathers = false;
	_field1AC = 0;
	_field1B0 = 0;
	_emptyStartFrame = 303;
	_emptyEndFrame = 312;
	_smokeStartFrame = 313;
	_smokeEndFrame = 325;
	_field1C4 = 326;
	_field1C8 = 347;
	_field1CC = 348;
	_field1D0 = 375;
	_field1D4 = 1;
	_field1D8 = 0;
}

void CSuccUBus::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);

	file->writeNumberLine(_enabled, indent);
	file->writeNumberLine(_startFrame8, indent);
	file->writeNumberLine(_endFrame8, indent);
	file->writeNumberLine(_startFrame11, indent);
	file->writeNumberLine(_endFrame11, indent);
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
	file->writeNumberLine(_field140, indent);

	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_afterReceiveStartFrame, indent);
	file->writeNumberLine(_afterReceiveEndFrame, indent);
	file->writeNumberLine(_trayOutStartFrame, indent);
	file->writeNumberLine(_trayOutEndFrame, indent);
	file->writeNumberLine(_field158, indent);
	file->writeNumberLine(_field15C, indent);

	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_startFrame1, indent);
	file->writeNumberLine(_endFrame1, indent);
	file->writeNumberLine(_rect1.left, indent);
	file->writeNumberLine(_rect1.top, indent);
	file->writeNumberLine(_rect1.right, indent);
	file->writeNumberLine(_rect1.bottom, indent);
	file->writeNumberLine(_field184, indent);
	file->writeNumberLine(_field188, indent);
	file->writeNumberLine(_rect2.left, indent);
	file->writeNumberLine(_rect2.top, indent);
	file->writeNumberLine(_rect2.right, indent);
	file->writeNumberLine(_rect2.bottom, indent);
	file->writeNumberLine(_sendLost, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_isChicken, indent);
	file->writeNumberLine(_isFeathers, indent);
	file->writeNumberLine(_field1AC, indent);
	file->writeNumberLine(_field1B0, indent);
	file->writeNumberLine(_emptyStartFrame, indent);
	file->writeNumberLine(_emptyEndFrame, indent);
	file->writeNumberLine(_smokeStartFrame, indent);
	file->writeNumberLine(_smokeEndFrame, indent);
	file->writeNumberLine(_field1C4, indent);
	file->writeNumberLine(_field1C8, indent);
	file->writeNumberLine(_field1CC, indent);
	file->writeNumberLine(_field1D0, indent);
	file->writeNumberLine(_field1D4, indent);

	file->writeNumberLine(_v3, indent);
	file->writeNumberLine(_field1D8, indent);
	file->writeNumberLine(_field104, indent);

	CTrueTalkNPC::save(file, indent);
}

void CSuccUBus::load(SimpleFile *file) {
	file->readNumber();

	_enabled = file->readNumber();
	_startFrame8 = file->readNumber();
	_endFrame8 = file->readNumber();
	_startFrame11 = file->readNumber();
	_endFrame11 = file->readNumber();
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
	_field140 = file->readNumber();

	_v2 = file->readNumber();
	_afterReceiveStartFrame = file->readNumber();
	_afterReceiveEndFrame = file->readNumber();
	_trayOutStartFrame = file->readNumber();
	_trayOutEndFrame = file->readNumber();
	_field158 = file->readNumber();
	_field15C = file->readNumber();

	_string2 = file->readString();
	_startFrame1 = file->readNumber();
	_endFrame1 = file->readNumber();
	_rect1.left = file->readNumber();
	_rect1.top = file->readNumber();
	_rect1.right = file->readNumber();
	_rect1.bottom = file->readNumber();
	_field184 = file->readNumber();
	_field188 = file->readNumber();
	_rect2.left = file->readNumber();
	_rect2.top = file->readNumber();
	_rect2.right = file->readNumber();
	_rect2.bottom = file->readNumber();
	_sendLost = file->readNumber();
	_soundHandle = file->readNumber();
	_isChicken = file->readNumber();
	_isFeathers = file->readNumber();
	_field1AC = file->readNumber();
	_field1B0 = file->readNumber();
	_emptyStartFrame = file->readNumber();
	_emptyEndFrame = file->readNumber();
	_smokeStartFrame = file->readNumber();
	_smokeEndFrame = file->readNumber();
	_field1C4 = file->readNumber();
	_field1C8 = file->readNumber();
	_field1CC = file->readNumber();
	_field1D0 = file->readNumber();
	_field1D4 = file->readNumber();

	_v3 = file->readNumber();
	_field1D8 = file->readNumber();
	_field104 = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CSuccUBus::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_field1D8) {
		Rect tempRect = _rect1;
		tempRect.translate(_bounds.left, _bounds.top);

		if (!_enabled || (_field188 && tempRect.contains(msg->_mousePos))) {
			CTurnOn onMsg;
			onMsg.execute(this);
			_enabled = true;
		} else if (getRandomNumber(256) < 130) {
			_enabled = false;
			CTurnOff offMsg;
			offMsg.execute(this);
		} else {
			switch (getRandomNumber(2)) {
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

	if (!_enabled || !pet || !item || !tempRect.contains(item->getControid())) {
		item->petAddToInventory();
	} else if (mailExists(roomFlags)) {
		petDisplayMessage(SUCCUBUS_DESCRIPTION);
		item->petAddToInventory();
	} else {
		petContainerRemove(item);
		pet->phonographAction("");

		CChicken *chicken = dynamic_cast<CChicken *>(item);
		bool chickenFlag = chicken ? chicken->_string6 != "None" : false;

		item->setVisible(false);
		if (_startFrame1 >= 0) {
			playSound("z#23.wav");
			playMovie(_startFrame1, _endFrame1, 0);
		}

		if (!chickenFlag) {
			_field188 = 1;
			item->addMail(roomFlags);
			petSetArea(PET_REMOTE);
			petHighlightGlyph(16);
			CSUBTransition transMsg;
			transMsg.execute(this);
		} else {
			if (_okStartFrame >= 0) {
				startTalking(this, 70219, findView());
				playMovie(_okStartFrame, _okEndFrame, 0);
			}

			if (_sendStartFrame >= 0) {
				playMovie(_sendStartFrame, _sendEndFrame, MOVIE_NOTIFY_OBJECT);
				_field158 = 2;
			}

			CViewItem *view = parseView(chicken->_fullViewName);
			if (!view)
				return false;

			item->setPosition(item->_origPos);
			item->moveUnder(view);

			CSUBTransition transMsg;
			transMsg.execute(this);
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
	if (_startFrame8 >= 0)
		loadFrame(_startFrame8);

	return true;
}

bool CSuccUBus::LeaveViewMsg(CLeaveViewMsg *msg) {
	petDisplayMessage(2, BLANK);
	if (_startFrame8 >= 0)
		loadFrame(_startFrame8);
	else if (!_field15C && _onStartFrame >= 0)
		loadFrame(_onStartFrame);

	petClear();
	if (_soundHandle != -1) {
		stopSound(_soundHandle, 1);
		_soundHandle = -1;
	}

	if (_enabled) {
		_enabled = false;
		if (_offStartFrame >= 0)
			playSound("z#27.wav", 100);

		if (_field15C)
			setVisible(false);
	}

	performAction(true, findView());
	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

bool CSuccUBus::PETDeliverMsg(CPETDeliverMsg *msg) {
	if (_field1D8)
		return true;

	if (!_enabled) {
		petDisplayMessage(2, SUCCUBUS_IS_IN_STANDBY);
		return true;
	}

	CPetControl *pet = getPetControl();
	if (!pet)
		return true;

	uint destRoomFlags = pet->getRoomFlags();
	CGameObject *mailObject = findMail(destRoomFlags);
	if (!mailObject) {
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

		CRoomFlags roomFlags = _roomFlags;
		if (!pet->isSuccUBusDest(roomFlags) || pet->getMailDestClass(roomFlags) < getPassengerClass()) {
			roomFlags = pet->getSpecialRoomFlags("BilgeRoom");
			_sendLost = true;
		}

		_isFeathers = mailObject->getName() == "Feathers";
		_isChicken = mailObject->getName() == "Chicken";
		_field158 = 0;
		_field188 = 0;
		_field1D8 = 1;
		incTransitions();

		if (_isFeathers) {
			// The feather has special handling to be rejected by the SuccUBus
			_sendLost = false;
			sendMail(destRoomFlags, roomFlags);
			pet->phonographAction("");

			if (_okStartFrame >= 0) {
				playMovie(_okStartFrame, _okEndFrame, 0);
				startTalking(this, 230022, findView());
			}

			_field158 = 1;
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

	if (_field1D8 || !pet)
		return true;
	if (!_enabled) {
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
		CGameObject *mailObject = findMailByFlags(
			_v3 && compareRoomNameTo("Titania") ? 3 : _field140, petRoomFlags);
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
				_field158 = 1;
				_field1D8 = 1;
				incTransitions();
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
		if (_startFrame11 >= 0)
			playSound("z#30.wav", 100);

		if (_field15C) {
			_field15C = false;
			setVisible(false);
			CSignalObject signalMsg;
			signalMsg._numValue = 1;
			signalMsg.execute(_string2);
		}
	}

	if (msg->_endFrame == _onEndFrame) {
		bool flag = false;

		if (pet && !mailExists(petRoomFlags)) {
			CGameObject *mailObject = _v3 && compareRoomNameTo("Titania") ?
				findMailByFlags(3, petRoomFlags) :
				findMailByFlags(_field140, petRoomFlags);

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

		if (!_field188 && !flag) {
			stopSound(_soundHandle);
			_soundHandle = -1;

			switch (getRandomNumber(_v2 ? 7 : 5, &_field1B0)) {
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
		if (_field158 == 1) {
			startTalking(this, 230022, findView());
		} else if (_field158 == 2) {
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

		if (_field1D8) {
			_field1D8 = 0;
			decTransitions();
		}

		CSUBTransition transMsg;
		transMsg.execute(this);
	}

	if (msg->_endFrame == _receiveEndFrame) {
		// SuccUBus disgorged mail
		if (pet && _mailP) {
			_mailP->setMailDest(petRoomFlags);
		}

		_field188 = 1;
		_mailP = 0;
		if (_field1D8) {
			_field1D8 = 0;
			decTransitions();
		}

		CSUBTransition transMsg;
		transMsg.execute(this);
	}

	return true;
}

bool CSuccUBus::TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg) {
	if (msg->_stateNum == 1)
		msg->_stateVal = _enabled;

	return true;
}

bool CSuccUBus::SignalObject(CSignalObject *msg) {
	if (msg->_numValue == 1) {
		_string2 = msg->_strValue;
		_field15C = 1;
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
		if (!_field15C && _startFrame8 >= 0) {
			playMovie(_startFrame8, _endFrame8, 0);
			playSound("z#30.wav", 100);
		}

		if (_onStartFrame >= 0) {
			playMovie(_onStartFrame, _onEndFrame, MOVIE_NOTIFY_OBJECT);
			playSound("z#26.wav", 100);
		}

		uint petRoomFlags = pet->getRoomFlags();
		if (mailExists(petRoomFlags) && _endFrame1 >= 0)
			// Mail canister present
			playMovie(_endFrame1, _endFrame1, 0);

		_enabled = true;
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
		playSound("z#27.wav", 100);
		playMovie(_offStartFrame, _offEndFrame, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
	}

	if (!_field15C && _startFrame11 >= 0)
		playMovie(_startFrame11, _endFrame11, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);

	_enabled = false;
	performAction(true);
	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

bool CSuccUBus::SUBTransition(CSUBTransition *msg) {
	CPetControl *pet = getPetControl();

	if (pet) {
		uint petRoomFlags = pet->getRoomFlags();

		if (_enabled) {
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
	if (_enabled) {
		_roomFlags = msg->_roomFlags;
		playSound("z#98.wav", 100);
	}

	return true;
}

bool CSuccUBus::ActMsg(CActMsg *msg) {
	if (msg->_action == "EnableObject")
		_v3 = 1;
	else if (msg->_action == "DisableObject")
		_v3 = 0;

	return true;
}

bool CSuccUBus::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	CPetControl *pet = getPetControl();
	Rect tempRect = _rect1;
	tempRect.translate(_bounds.left, _bounds.top);

	if (_field1D8 || !_enabled || !_field188 || !tempRect.contains(msg->_mousePos)
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
	_field188 = 0;
	CSUBTransition transMsg;
	transMsg.execute(this);

	return true;
}

} // End of namespace Titanic
