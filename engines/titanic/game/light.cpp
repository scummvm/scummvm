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

CLight::CLight() : CBackground(), _fieldE0(0), _fieldE4(0),
	_fieldE8(0), _fieldEC(0), _fieldF0(0), _fieldF4(0),
	_fieldF8(0), _fieldFC(0) {
}

void CLight::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_fieldEC, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_fieldF4, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeNumberLine(_fieldFC, indent);

	CBackground::save(file, indent);
}

void CLight::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_fieldE8 = file->readNumber();
	_fieldEC = file->readNumber();
	_fieldF0 = file->readNumber();
	_fieldF4 = file->readNumber();
	_fieldF8 = file->readNumber();
	_fieldFC = file->readNumber();

	CBackground::load(file);
}

bool CLight::TurnOff(CTurnOff *msg) {
	setVisible(false);
	return true;
}

bool CLight::LightsMsg(CLightsMsg *msg) {
	if ((msg->_flag2 && _fieldE8) || (msg->_flag3 && _fieldEC)
			|| (msg->_flag1 && _fieldE4) || (msg->_flag4 && _fieldF0)) {
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

	if (_fieldFC == 1 && flag) {
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

	if (_fieldFC == 1 && flag) {
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
		_fieldFC = 0;

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
