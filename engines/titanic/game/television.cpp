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
#include "titanic/carry/magazine.h"
#include "titanic/core/project_item.h"
#include "titanic/debugger.h"
#include "titanic/game/get_lift_eye2.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/translation.h"

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

int CTelevision::_seasonFrame;
bool CTelevision::_turnOn;
int CTelevision::_seasonUnused;
int CTelevision::_eyeFloorNum;
bool CTelevision::_channel4Glyph;
bool CTelevision::_eyeFlag;

CTelevision::CTelevision() : CBackground(), _channelNum(1),
	_channelsCount(7), _isOn(false), _unused(0), _soundHandle(0) {
}

void CTelevision::init() {
	_seasonFrame = 531;
	_turnOn = true;
	_seasonUnused = 0;
	_eyeFloorNum = 27;
	_channel4Glyph = true;
	_eyeFlag = true;
}

void CTelevision::deinit() {
}

void CTelevision::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_channelNum, indent);
	file->writeNumberLine(_seasonFrame, indent);
	file->writeNumberLine(_channelsCount, indent);
	file->writeNumberLine(_turnOn, indent);
	file->writeNumberLine(_isOn, indent);
	file->writeNumberLine(_seasonUnused, indent);
	file->writeNumberLine(_unused, indent);
	file->writeNumberLine(_eyeFloorNum, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_channel4Glyph, indent);
	file->writeNumberLine(_eyeFlag, indent);

	CBackground::save(file, indent);
}

void CTelevision::load(SimpleFile *file) {
	file->readNumber();
	_channelNum = file->readNumber();
	_seasonFrame = file->readNumber();
	_channelsCount = file->readNumber();
	_turnOn = file->readNumber() != 0;
	_isOn = file->readNumber() != 0;
	_seasonUnused = file->readNumber();
	_unused = file->readNumber();
	_eyeFloorNum = file->readNumber();
	_soundHandle = file->readNumber();
	_channel4Glyph = file->readNumber();
	_eyeFlag = file->readNumber();

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
		_seasonFrame = 545;
		_seasonUnused = 0;
	} else if (msg->_season == "Winter") {
		_seasonFrame = 503;
		_seasonUnused = 0;
	} else if (msg->_season == "Spring") {
		_seasonFrame = 517;
		_seasonUnused = 0;
	} else if (msg->_season == "Summer") {
		_seasonFrame = 531;
		_seasonUnused = 0;
	}

	return true;
}

bool CTelevision::EnterViewMsg(CEnterViewMsg *msg) {
	petSetArea(PET_REMOTE);
	petHighlightGlyph(GLYPH_TELEVISION_CONTROL);
	petSetRemoteTarget();
	setVisible(0);
	_channelNum = 1;

	return true;
}

static const int START_FRAMES[9] = { 0, 0, 56, 112, 168, 224, 280, 336, 392 };
static const int END_FRAMES[8] = { 0, 55, 111, 167, 223, 279, 335, 391 };

bool CTelevision::PETUpMsg(CPETUpMsg *msg) {
	if (msg->_name == "Television" && _isOn) {
		if (isSoundActive(_soundHandle))
			stopSound(_soundHandle, 0);

		_channelNum = (_channelNum % _channelsCount) + 1;
		stopMovie();
		playMovie(START_FRAMES[_channelNum], END_FRAMES[_channelNum], MOVIE_NOTIFY_OBJECT);
	}

	return true;
}

bool CTelevision::PETDownMsg(CPETDownMsg *msg) {
	if (msg->_name == "Television" && _isOn) {
		if (isSoundActive(_soundHandle))
			stopSound(_soundHandle, 0);
		if (--_channelNum < 1)
			_channelNum += _channelsCount;

		stopMovie();
		playMovie(START_FRAMES[_channelNum], END_FRAMES[_channelNum], MOVIE_NOTIFY_OBJECT);
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
			_channelNum = 1;
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

	if (_channelNum == 3 && compareRoomNameTo("SGTState") && getPassengerClass() == THIRD_CLASS) {
		// WORKAROUND: The original allowed the magazine to be "won" multiple times. We
		// now search for magazine within the room (which is it's initial, hidden location).
		// That way, when it's 'Won', it's no longer present and can't be won again
		CMagazine *magazine = dynamic_cast<CMagazine *>(findRoom()->findByName("Magazine"));

		if (magazine) {
			// You may be a winner
			CProximity prox1, prox2;
			prox1._soundType = prox2._soundType = Audio::Mixer::kSpeechSoundType;
			playSound(TRANSLATE("z#47.wav", "z#578.wav"), prox1);
			_soundHandle = playSound(TRANSLATE("b#20.wav", "b#1.wav"), prox2);

			// Get the room flags for the SGT floor we're on
			CPetControl *pet = getPetControl();
			uint roomFlags = pet->getRoomFlags();

			// Send the magazine to the SuccUBus
			debugC(DEBUG_INTERMEDIATE, kDebugScripts, "Assigned room - %d", roomFlags);
			magazine->addMail(roomFlags);
			magazine->sendMail(roomFlags, roomFlags);

			loadFrame(561);
		} else {
			petDisplayMessage(NOTHING_ON_CHANNEL);
		}
	} else if (_channelNum == 2) {
		loadFrame(_seasonFrame);
	} else if (_channelNum == 4 && _channel4Glyph) {
		if (_turnOn)
			loadFrame(502);
		else
			petDisplayMessage(NOTHING_ON_CHANNEL);
	} else if (_channelNum == 5 && *CGetLiftEye2::_destObject != "NULL") {
		loadFrame(393 + _eyeFloorNum);
	} else {
		petDisplayMessage(NOTHING_ON_CHANNEL);
	}

	return true;
}

bool CTelevision::ShipSettingMsg(CShipSettingMsg *msg) {
	_eyeFloorNum = msg->_value;
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
	bool isYourStateroom = false;

	if (pet)
		isYourStateroom = pet->isFirstClassSuite();

	if (msg->_topLeft || !isYourStateroom)
		_turnOn = true;

	return true;
}

} // End of namespace Titanic
