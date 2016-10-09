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

#include "titanic/game/television.h"
#include "titanic/game/get_lift_eye2.h"
#include "titanic/core/project_item.h"
#include "titanic/carry/magazine.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CTelevision, CGameObject)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(PETUpMsg)
	ON_MESSAGE(PETDownMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(PETActivateMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(ShipSettingMsg)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(LightsMsg)
END_MESSAGE_MAP()

int CTelevision::_v1;
bool CTelevision::_turnOn;
int CTelevision::_v3;
int CTelevision::_v4;
int CTelevision::_v5;
int CTelevision::_v6;

CTelevision::CTelevision() : CBackground(), _fieldE0(1),
	_fieldE4(7), _isOn(false), _fieldEC(0), _soundHandle(0) {
}

void CTelevision::init() {
	_v1 = 531;
	_turnOn = true;
	_v3 = 0;
	_v4 = 27;
	_v5 = 1;
	_v6 = 1;
}

void CTelevision::deinit() {
}

void CTelevision::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_turnOn, indent);
	file->writeNumberLine(_isOn, indent);
	file->writeNumberLine(_v3, indent);
	file->writeNumberLine(_fieldEC, indent);
	file->writeNumberLine(_v4, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_v5, indent);
	file->writeNumberLine(_v6, indent);

	CBackground::save(file, indent);
}

void CTelevision::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_v1 = file->readNumber();
	_fieldE4 = file->readNumber();
	_turnOn = file->readNumber() != 0;
	_isOn = file->readNumber() != 0;
	_v3 = file->readNumber();
	_fieldEC = file->readNumber();
	_v4 = file->readNumber();
	_soundHandle = file->readNumber();
	_v5 = file->readNumber();
	_v6 = file->readNumber();

	CBackground::load(file);
}

bool CTelevision::LeaveViewMsg(CLeaveViewMsg *msg) {
	petClear();
	if (_isOn) {
		if (isSoundActive(_soundHandle))
			stopSound(_soundHandle, 0);

		loadFrame(622);
		stopMovie();
		setVisible(0);
		_isOn = false;

		if (compareRoomNameTo("CSGState")) {
			CVisibleMsg visibleMsg(true);
			visibleMsg.execute("Tellypic");
		}
	}

	return true;
}

bool CTelevision::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	if (msg->_season == "Autumn") {
		_v1 = 545;
		_v3 = 0;
	} else if (msg->_season == "Winter") {
		_v1 = 503;
		_v3 = 0;
	} else if (msg->_season == "Spring") {
		_v1 = 517;
		_v3 = 0;
	} else if (msg->_season == "Summer") {
		_v1 = 531;
		_v3 = 0;
	}

	return true;
}

bool CTelevision::EnterViewMsg(CEnterViewMsg *msg) {
	petSetArea(PET_REMOTE);
	petHighlightGlyph(GLYPH_TELEVISION_CONTROL);
	petSetRemoteTarget();
	setVisible(0);
	_fieldE0 = 1;

	return true;
}

static const int START_FRAMES[9] = { 0, 0, 56, 112, 168, 224, 280, 336, 392 };
static const int END_FRAMES[8] = { 0, 55, 111, 167, 223, 279, 335, 391 };

bool CTelevision::PETUpMsg(CPETUpMsg *msg) {
	if (msg->_name == "Television" && _isOn) {
		if (isSoundActive(_soundHandle))
			stopSound(_soundHandle, 0);

		_fieldE0 = _fieldE0 % _fieldE4 + 1;
		stopMovie();
		playMovie(START_FRAMES[_fieldE0], END_FRAMES[_fieldE0], 4);
	}

	return true;
}

bool CTelevision::PETDownMsg(CPETDownMsg *msg) {
	if (msg->_name == "Television" && _isOn) {
		if (isSoundActive(_soundHandle))
			stopSound(_soundHandle, 0);
		if (--_fieldE0 < 1)
			_fieldE0 += _fieldE4;

		_fieldE0 = _fieldE0 % _fieldE4 + 1;
		stopMovie();
		playMovie(START_FRAMES[_fieldE0], END_FRAMES[_fieldE0], 4);
	}

	return true;
}

bool CTelevision::StatusChangeMsg(CStatusChangeMsg *msg) {
	if (_isOn) {
		stopMovie();
		playMovie(0);
	}

	return true;
}

bool CTelevision::ActMsg(CActMsg *msg) {
	if (msg->_action == "TurnTVOnOff") {
		_isOn = !_isOn;
		if (_isOn) {
			setVisible(true);
			CStatusChangeMsg changeMsg;
			changeMsg.execute(this);
		} else {
			setVisible(_isOn);
			stopMovie();
		}
	}

	return true;
}

bool CTelevision::PETActivateMsg(CPETActivateMsg *msg) {
	if (msg->_name == "Television") {
		CVisibleMsg visibleMsg(_isOn);
		_isOn = !_isOn;

		if (_isOn) {
			setVisible(true);
			playMovie(0, 55, 0);
			_fieldE0 = 1;
		} else {
			stopMovie();
			if (isSoundActive(_soundHandle))
				stopSound(_soundHandle, 0);

			setVisible(false);
		}

		if (compareRoomNameTo("SGTState"))
			visibleMsg.execute("Tellypic");
	}

	return true;
}

bool CTelevision::MovieEndMsg(CMovieEndMsg *msg) {
	if (getRandomNumber(6) == 0) {
		CParrotSpeakMsg parrotMsg("Television", "");
		parrotMsg.execute("PerchedParrot");
	}

	if (_fieldE0 == 3 && compareRoomNameTo("SGTState") && !getPassengerClass()) {
		playSound("z#47.wav");
		_soundHandle = playSound("b#20.wav");
		CMagazine *magazine = dynamic_cast<CMagazine *>(getRoot()->findByName("Magazine"));

		if (magazine) {
			CPetControl *pet = getPetControl();
			uint roomFlags = pet->getRoomFlags();

			debugC(kDebugScripts, "Assigned room - %d", roomFlags);
			magazine->addMail(roomFlags);
			magazine->removeMail(roomFlags, roomFlags);
		}

		loadFrame(561);
	} else if (_fieldE0 == 2) {
		loadFrame(_v1);
	} else if (_fieldE0 == 4 && _v5) {
		if (_turnOn)
			loadFrame(502);
		else
			warning("There is currently nothing available for your viewing pleasure on this channel.");
	} else if (_fieldE0 == 5 && *CGetLiftEye2::_destObject != "NULL") {
		loadFrame(393 + _v4);
	} else {
		warning("There is currently nothing available for your viewing pleasure on this channel.");
	}

	return true;
}

bool CTelevision::ShipSettingMsg(CShipSettingMsg *msg) {
	_v4 = msg->_value;
	return true;
}

bool CTelevision::TurnOff(CTurnOff *msg) {
	_turnOn = false;
	return true;
}

bool CTelevision::TurnOn(CTurnOn *msg) {
	_turnOn = true;
	return true;
}

bool CTelevision::LightsMsg(CLightsMsg *msg) {
	CPetControl *pet = getPetControl();
	bool flag = false;

	if (pet)
		flag = pet->isRoom59706();

	if (msg->_flag2 || !flag)
		_turnOn = true;

	return true;
}

} // End of namespace Titanic
