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

#include "titanic/game/transport/service_elevator.h"
#include "titanic/core/room_item.h"
#include "titanic/npcs/doorbot.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CServiceElevator, CTransport)
	ON_MESSAGE(BodyInBilgeRoomMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(ServiceElevatorMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(ServiceElevatorFloorRequestMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(OpeningCreditsMsg)
END_MESSAGE_MAP()

bool CServiceElevator::_v1;
int CServiceElevator::_v2;
int CServiceElevator::_v3;

CServiceElevator::CServiceElevator() : CTransport(),
	_fieldF8(0), _soundHandle1(0), _timerId(0), _soundHandle2(0) {
}

void CServiceElevator::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_v3, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeNumberLine(_soundHandle1, indent);
	file->writeNumberLine(_timerId, indent);
	file->writeNumberLine(_soundHandle2, indent);

	CTransport::save(file, indent);
}

void CServiceElevator::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_v3 = file->readNumber();
	_fieldF8 = file->readNumber();
	_soundHandle1 = file->readNumber();
	_timerId = file->readNumber();
	_soundHandle2 = file->readNumber();

	CTransport::load(file);
}

bool CServiceElevator::BodyInBilgeRoomMsg(CBodyInBilgeRoomMsg *msg) {
	_v2 = true;
	_string1 = "BilgeRoomWith.Node 2.N";
	return true;
}

bool CServiceElevator::EnterViewMsg(CEnterViewMsg *msg) {
	petShow();
	return true;
}

bool CServiceElevator::ServiceElevatorMsg(CServiceElevatorMsg *msg) {
	switch (msg->_value) {
	case 1:
	case 2:
	case 3: {
		switch (msg->_value) {
		case 1:
			_v3 = 0;
			break;
		case 2:
			_v3 = 1;
			break;
		case 3:
			_v3 = 2;
			break;
		default:
			break;
		}

		CServiceElevatorFloorRequestMsg requestMsg;
		requestMsg.execute(this);
		break;
	}

	case 4:
		if (!_string1.empty()) {
			if (_string1 == "DeepSpace") {
				disableMouse();
				_soundHandle1 = playSound(TRANSLATE("z#413.wav", "z#157.wav"), 50);
				_timerId = addTimer(1, 1000, 500);
			} else {
				changeView(_string1);
			}
		}
		break;

	case 5:
		// Reaching destination floor
		_fieldF8 = false;
		_fieldDC = _v3;
		loadSound(TRANSLATE("z#423.wav", "z#168.wav"));
		stopSound(_soundHandle2);
		_soundHandle2 = playSound(TRANSLATE("z#423.wav", "z#168.wav"), 80);

		switch (_fieldDC) {
		case 0:
			_string1 = "DeepSpace";
			_string2 = TRANSLATE("a#2.wav", "a#54.wav");
			queueSound(TRANSLATE("z#416.wav", "z#160.wav"), _soundHandle2, 50);
			break;

		case 1:
			_string1 = _v2 ? "BilgeRoomWith.Node 2.N" : "BilgeRoom.Node 1.N";
			queueSound(TRANSLATE("z#421.wav", "z#165.wav"), _soundHandle2, 50);
			break;

		case 2:
			_string1 = _v1 ?  "MoonEmbLobby.Node 1.NE" : "EmbLobby.Node 1.NE";
			queueSound(TRANSLATE("z#411.wav", "z#155.wav"), _soundHandle2, 50);
			break;

		default:
			break;
		}

		enableMouse();
		if (findRoom()->findByName("Doorbot"))
			addTimer(3, 3000, 0);
		break;

	default:
		break;
	}

	return true;
}

bool CServiceElevator::TimerMsg(CTimerMsg *msg) {
	CDoorbot *doorbot = dynamic_cast<CDoorbot *>(findRoom()->findByName("Doorbot"));

	switch (msg->_actionVal) {
	case 0:
	case 1:
		if (!isSoundActive(_soundHandle1)) {
			stopAnimTimer(_timerId);
			if (msg->_actionVal == 0) {
				// Elevator in motion after pressing button
				_fieldF8 = true;
				CServiceElevatorFloorChangeMsg changeMsg(_fieldDC, _v3);
				changeMsg.execute(getRoom(), nullptr, MSGFLAG_SCAN);
				_soundHandle2 = playSound(TRANSLATE("z#424.wav", "z#169.wav"));

				if (doorbot) {
					CActMsg actMsg("DoorbotPlayerPressedTopButton");
					actMsg.execute(doorbot);
				}
			} else {
				// Finished playing message for bottom/middle floor disabled
				enableMouse();
				if (doorbot) {
					CActMsg actMsg;
					if (_v3 == 0)
						actMsg._action = "DoorbotPlayerPressedBottomButton";
					else if (_v3 == 1)
						actMsg._action = "DoorbotPlayerPressedMiddleButton";

					actMsg.execute(doorbot);
				}
			}
		}
		break;

	case 3: {
		CActMsg actMsg("DoorbotReachedEmbLobby");
		actMsg.execute(doorbot);
		break;
	}

	default:
		break;
	}

	return true;
}

bool CServiceElevator::ServiceElevatorFloorRequestMsg(CServiceElevatorFloorRequestMsg *msg) {
	disableMouse();
	CDoorbot *doorbot = dynamic_cast<CDoorbot *>(findRoom()->findByName("Doorbot"));

	if (doorbot && _v3 == 0) {
		_soundHandle1 = playSound(TRANSLATE("z#415.wav", "z#159.wav"), 50);
		_timerId = addTimer(1, 1000, 500);
	} else if (doorbot && _v3 == 1) {
		_soundHandle1 = playSound(TRANSLATE("z#417.wav", "z#161.wav"), 50);
		_timerId = addTimer(1, 1000, 500);
	} else if (_fieldDC == _v3) {
		switch (_v3) {
		case 0:
			_soundHandle1 = playSound(TRANSLATE("z#415.wav", "z#159.wav"), 50);
			break;
		case 1:
			_soundHandle1 = playSound(TRANSLATE("z#420.wav", "z#164.wav"), 50);
			break;
		case 2:
			_soundHandle1 = playSound(TRANSLATE("z#410.wav", "z#154.wav"), 50);
			break;
		default:
			break;
		}

		_timerId = addTimer(1, 1000, 500);
	} else {
		switch (_v3) {
		case 0:
			_soundHandle1 = playSound(TRANSLATE("z#414.wav", "z#158.wav"), 50);
			break;
		case 1:
			_soundHandle1 = playSound(_fieldDC ? TRANSLATE("z#419.wav", "z#163.wav")
				: TRANSLATE("z#418.wav", "z#162.wav"), 50);
			break;
		case 2:
			_soundHandle1 = playSound(TRANSLATE("z#409.wav", "z#153.wav"), 50);
			break;
		default:
			break;
		}

		_timerId = addTimer(0, 1000, 500);
	}

	return true;
}

bool CServiceElevator::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	CDoorbot *doorbot = dynamic_cast<CDoorbot *>(findRoom()->findByName("Doorbot"));

	if (doorbot) {
		CPutBotBackInHisBoxMsg boxMsg(0);
		boxMsg.execute("Doorbot");
		doorbot->performAction(false);
		enableMouse();
	}

	return true;
}

bool CServiceElevator::OpeningCreditsMsg(COpeningCreditsMsg *msg) {
	_v1 = false;
	_string1 = "EmbLobby.Node 1.NE";
	return true;
}

} // End of namespace Titanic
