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
#include "titanic/translation.h"

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

CLight::CLight() : CBackground(), _unused1(0), _topRight(false),
	_topLeft(false), _bottomLeft(false), _bottomRight(false), _unused2(0),
	_unused3(0), _eyePresent(false) {
}

void CLight::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_unused1, indent);
	file->writeNumberLine(_topRight, indent);
	file->writeNumberLine(_topLeft, indent);
	file->writeNumberLine(_bottomLeft, indent);
	file->writeNumberLine(_bottomRight, indent);
	file->writeNumberLine(_unused2, indent);
	file->writeNumberLine(_unused3, indent);
	file->writeNumberLine(_eyePresent, indent);

	CBackground::save(file, indent);
}

void CLight::load(SimpleFile *file) {
	file->readNumber();
	_unused1 = file->readNumber();
	_topRight = file->readNumber();
	_topLeft = file->readNumber();
	_bottomLeft = file->readNumber();
	_bottomRight = file->readNumber();
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
	// WORKAROUND: Since solving the puzzle to get Titania's eye explicitly
	// requires referring to the "broken light", don't allow the top left
	// light to be turned on in the player's stateroom until it's gotten/fixed
	if (msg->_topLeft && _topLeft) {
		bool showFlag = !getPetControl()->isFirstClassSuite() ||
			getRoom()->findByName("Eye1") == nullptr;
		setVisible(showFlag);
	} else if ((msg->_bottomLeft && _bottomLeft) || (msg->_topRight && _topRight) ||
			(msg->_bottomRight && _bottomRight)) {
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
	bool isYourStateroom = pet ? pet->isFirstClassSuite() : false;

	if (_eyePresent && isYourStateroom) {
		petDisplayMessage(1, LIGHT_IS_LOOSE);
		playSound(TRANSLATE("z#144.wav", "z#700.wav"), 70);
	} else {
		petDisplayMessage(1, LUMI_GLOW_LIGHTS);
		playSound(TRANSLATE("z#62.wav", "z#593.wav"), 70);
	}

	return true;
}

bool CLight::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CPetControl *pet = getPetControl();
	bool isYourStateroom = pet ? pet->isFirstClassSuite() : false;

	if (_eyePresent && isYourStateroom) {
		petDisplayMessage(1, LIGHT_IS_LOOSE);
		playSound(TRANSLATE("z#144.wav", "z#700.wav"), 70);
	} else {
		petDisplayMessage(1, LUMI_GLOW_LIGHTS);
		playSound(TRANSLATE("z#62.wav", "z#593.wav"), 70);
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
		CLightsMsg lightsMsg(true, true, true, true);
		lightsMsg.execute("1stClassState", CLight::_type, MSGFLAG_SCAN);

		bool isYourStateroom = pet ? pet->isFirstClassSuite() : false;
		if (isYourStateroom)
			CTelevision::_turnOn = true;
	}

	return true;
}

} // End of namespace Titanic
