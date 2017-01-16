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

#include "titanic/game/light.h"
#include "titanic/game/television.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLight, CBackground)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(LightsMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(EnterRoomMsg)
END_MESSAGE_MAP()

CLight::CLight() : CBackground(), _unused1(0), _upRight(false),
	_upLeft(false), _downLeft(false), _downRight(false), _unused2(0),
	_unused3(0), _eyePresent(false) {
}

void CLight::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_unused1, indent);
	file->writeNumberLine(_upRight, indent);
	file->writeNumberLine(_upLeft, indent);
	file->writeNumberLine(_downLeft, indent);
	file->writeNumberLine(_downRight, indent);
	file->writeNumberLine(_unused2, indent);
	file->writeNumberLine(_unused3, indent);
	file->writeNumberLine(_eyePresent, indent);

	CBackground::save(file, indent);
}

void CLight::load(SimpleFile *file) {
	file->readNumber();
	_unused1 = file->readNumber();
	_upRight = file->readNumber();
	_upLeft = file->readNumber();
	_downLeft = file->readNumber();
	_downRight = file->readNumber();
	_unused2 = file->readNumber();
	_unused3 = file->readNumber();
	_eyePresent = file->readNumber();

	CBackground::load(file);
}

bool CLight::TurnOff(CTurnOff *msg) {
	setVisible(false);
	return true;
}

bool CLight::LightsMsg(CLightsMsg *msg) {
	if ((msg->_upLeft && _upLeft) || (msg->_downLeft && _downLeft)
			|| (msg->_upRight && _upRight) || (msg->_downRight && _downRight)) {
		setVisible(true);
	} else {
		setVisible(false);
	}

	return true;
}

bool CLight::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	// WORKAROUND: Original code doesn't seem to do anything
	return true;
}

bool CLight::TurnOn(CTurnOn *msg) {
	setVisible(true);
	return true;
}

bool CLight::StatusChangeMsg(CStatusChangeMsg *msg) {
	CPetControl *pet = getPetControl();
	bool flag = pet ? pet->isRoom59706() : false;

	if (_eyePresent && flag) {
		petDisplayMessage(1, LIGHT_IS_LOOSE);
		playSound("z#144.wav", 70);
	} else {
		petDisplayMessage(1, LUMI_GLOW_LIGHTS);
		playSound("z#62.wav", 70);
	}

	return true;
}

bool CLight::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CPetControl *pet = getPetControl();
	bool flag = pet ? pet->isRoom59706() : false;

	if (_eyePresent && flag) {
		petDisplayMessage(1, LIGHT_IS_LOOSE);
		playSound("z#144.wav", 70);
	} else {
		petDisplayMessage(1, LUMI_GLOW_LIGHTS);
		playSound("z#62.wav", 70);
	}

	return true;
}

bool CLight::ActMsg(CActMsg *msg) {
	if (msg->_action == "Eye Removed")
		_eyePresent = false;

	return true;
}

bool CLight::EnterRoomMsg(CEnterRoomMsg *msg) {
	CPetControl *pet = getPetControl();
	setVisible(true);

	if (isEquals("6WTL")) {
		CLightsMsg lightsMsg(1, 1, 1, 1);
		lightsMsg.execute("1stClassState", CLight::_type, MSGFLAG_SCAN);

		bool flag = pet ? pet->isRoom59706() : false;
		if (flag)
			CTelevision::_turnOn = true;
	}

	return true;
}

} // End of namespace Titanic
