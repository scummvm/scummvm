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

#include "titanic/game/light_switch.h"
#include "titanic/game/light.h"
#include "titanic/game/television.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLightSwitch, CBackground)
	ON_MESSAGE(PETUpMsg)
	ON_MESSAGE(PETDownMsg)
	ON_MESSAGE(PETLeftMsg)
	ON_MESSAGE(PETRightMsg)
	ON_MESSAGE(PETActivateMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(EnterRoomMsg)
END_MESSAGE_MAP()

bool CLightSwitch::_flag;

CLightSwitch::CLightSwitch() : CBackground(),
		_fieldE0(0), _fieldE4(0), _turnOnTV(false) {
}

void CLightSwitch::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_flag, indent);
	file->writeNumberLine(_turnOnTV, indent);

	CBackground::save(file, indent);
}

void CLightSwitch::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_flag = file->readNumber();
	_turnOnTV = file->readNumber();

	CBackground::load(file);
}

bool CLightSwitch::PETUpMsg(CPETUpMsg *msg) {
	if (msg->_name == "Light") {
		CLightsMsg lightsMsg(true, true, false, false);
		lightsMsg.execute("1stClassState", CLight::_type, MSGFLAG_SCAN);

		if (_turnOnTV)
			CTelevision::_turnOn = true;
	}

	return true;
}

bool CLightSwitch::PETDownMsg(CPETDownMsg *msg) {
	if (msg->_name == "Light") {
		CLightsMsg lightsMsg(false, false, true, true);
		lightsMsg.execute("1stClassState", CLight::_type, MSGFLAG_SCAN);

		if (_turnOnTV)
			CTelevision::_turnOn = true;
	}

	return true;
}

bool CLightSwitch::PETLeftMsg(CPETLeftMsg *msg) {
	if (msg->_name == "Light") {
		CLightsMsg lightsMsg(false, true, true, false);
		lightsMsg.execute("1stClassState", CLight::_type, MSGFLAG_SCAN);

		if (_turnOnTV)
			CTelevision::_turnOn = true;
	}

	return true;
}

bool CLightSwitch::PETRightMsg(CPETRightMsg *msg) {
	if (msg->_name == "Light") {
		CLightsMsg lightsMsg(true, false, false, true);
		lightsMsg.execute("1stClassState", CLight::_type, MSGFLAG_SCAN);

		if (_turnOnTV)
			CTelevision::_turnOn = true;
	}

	return true;
}

bool CLightSwitch::PETActivateMsg(CPETActivateMsg *msg) {
	if (msg->_name == "Light") {
		if (_flag) {
			CTurnOff offMsg;
			offMsg.execute("1stClassState", CLight::_type, MSGFLAG_CLASS_DEF | MSGFLAG_SCAN);

		} else {
			CTurnOn onMsg;
			onMsg.execute("1stClassState", CLight::_type, MSGFLAG_CLASS_DEF | MSGFLAG_SCAN);
			_flag = false;
			if (_turnOnTV)
				CTelevision::_turnOn = false;
		}
	}

	return true;
}

bool CLightSwitch::EnterViewMsg(CEnterViewMsg *msg) {
	petSetRemoteTarget();
	return true;
}

bool CLightSwitch::LeaveViewMsg(CLeaveViewMsg *msg) {
	petClear();
	return true;
}

bool CLightSwitch::EnterRoomMsg(CEnterRoomMsg *msg) {
	_flag = true;
	CPetControl *pet = getPetControl();
	if (pet)
		_turnOnTV = pet->isFirstClassSuite();

	return true;
}

} // End of namespace Titanic
