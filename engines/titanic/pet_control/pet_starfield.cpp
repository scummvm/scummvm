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

#include "titanic/pet_control/pet_starfield.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/star_control/star_control.h"

namespace Titanic {

CPetStarfield::CPetStarfield() : _flickerCtr(0), _photoOn(true),
		_hasReference(false), _rect1(22, 352, 598, 478) {
	_markerStates[0] = _markerStates[1] = _markerStates[2] = MS_BLANK;
}

bool CPetStarfield::setup(CPetControl *petControl) {
	if (petControl && setupControl(petControl))
		return reset();
	return false;
}

bool CPetStarfield::reset() {
	if (_petControl) {
		_imgStarfield.setup(MODE_UNSELECTED, "3PetStarField", _petControl);
		_imgPhoto.setup(MODE_UNSELECTED, "HomePhotoOnOff", _petControl);
		_btnSetDest.setup(MODE_UNSELECTED, "3PetSetDestin", _petControl);
		_btnSetDest.setup(MODE_SELECTED, "3PetSetDestin1", _petControl);
		_imgStarCtrl.setup(MODE_UNSELECTED, "3PetStarCtrl", _petControl);

		_leds[0].setup(MODE_UNSELECTED, "LEDOff1", _petControl);
		_leds[1].setup(MODE_UNSELECTED, "LEDOn1", _petControl);
		_leds[2].setup(MODE_UNSELECTED, "LEDOff2", _petControl);
		_leds[3].setup(MODE_UNSELECTED, "LEDOn2", _petControl);
		_leds[4].setup(MODE_UNSELECTED, "LEDOff3", _petControl);
		_leds[5].setup(MODE_UNSELECTED, "LEDOn3", _petControl);

		uint col = getColor(0);
		_text.setColor(col);
		_text.setLineColor(0, col);
	}

	return true;
}

void CPetStarfield::draw(CScreenManager *screenManager) {
	_petControl->drawSquares(screenManager, 2);

	_imgStarfield.draw(screenManager);
	if (_photoOn) {
		_imgPhoto.draw(screenManager);
	} else {
		_imgStarCtrl.draw(screenManager);
	}

	_btnSetDest.draw(screenManager);
	drawButton(_markerStates[0], 0, screenManager);
	drawButton(_markerStates[1], 2, screenManager);
	drawButton(_markerStates[2], 4, screenManager);
	_text.draw(screenManager);
}

bool CPetStarfield::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_petControl->_remoteTarget)
		return false;

	if (_imgStarfield.MouseButtonDownMsg(msg->_mousePos)) {
		CPETHelmetOnOffMsg helmetMsg;
		helmetMsg.execute(_petControl->_remoteTarget);
	} else if (_imgPhoto.MouseButtonDownMsg(msg->_mousePos)) {
		if (_hasReference) {
			_photoOn = !_photoOn;
			CPETPhotoOnOffMsg photoMsg;
			photoMsg.execute(_petControl->_remoteTarget);
		} else {
			_petControl->displayMessage(SUPPLY_GALACTIC_REFERENCE);
		}
	} else if (!_btnSetDest.MouseButtonDownMsg(msg->_mousePos)) {
		return markersMouseDown(msg);
	}

	return true;
}

bool CPetStarfield::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (!_petControl->_remoteTarget || !_btnSetDest.MouseButtonUpMsg(msg->_mousePos))
		return false;

	if (_petControl) {
		CStarControl *starControl = _petControl->getStarControl();

		if (starControl && starControl->canSetStarDestination()) {
			CPETSetStarDestinationMsg starfieldMsg;
			starfieldMsg.execute(_petControl->_remoteTarget);
			starControl->starDestinationSet();
		}
	}

	return true;
}

bool CPetStarfield::isValid(CPetControl *petControl) {
	return setupControl(petControl);
}

void CPetStarfield::load(SimpleFile *file, int param) {
	if (!param) {
		_photoOn = file->readNumber();
		_hasReference = file->readNumber();
	}
}

void CPetStarfield::postLoad() {
	reset();
}

void CPetStarfield::save(SimpleFile *file, int indent) {
	file->writeNumberLine(_photoOn, indent);
	file->writeNumberLine(_hasReference, indent);
}

bool CPetStarfield::setupControl(CPetControl *petControl) {
	if (petControl) {
		_petControl = petControl;

		Rect r(0, 0, 64, 64);
		r.translate(_rect1.left, _rect1.top);

		_imgStarfield.setBounds(r);
		_imgStarfield.translate(15, 23);
		_imgPhoto.setBounds(r);
		_imgPhoto.translate(85, 23);
		_imgStarCtrl.setBounds(r);
		_imgStarCtrl.translate(85, 23);

		r = Rect(0, 0, 34, 34);
		r.translate(468, 396);
		_leds[0].setBounds(r);
		_leds[1].setBounds(r);

		r.translate(36, 0);
		_leds[2].setBounds(r);
		_leds[3].setBounds(r);

		r.translate(36, 0);
		_leds[4].setBounds(r);
		_leds[5].setBounds(r);

		r = Rect(0, 0, 157, 51);
		r.translate(224, 33);
		r.translate(20, 350);
		_btnSetDest.setBounds(r);

		r = Rect(0, 0, 580, 15);
		r.translate(32, 445);
		_text.setBounds(r);
		_text.setHasBorder(false);
	}

	return true;
}

void CPetStarfield::drawButton(MarkerState state, int index, CScreenManager *screenManager) {
	int offset = (int)state;
	if (_flickerCtr < 4 && state == MS_FLICKERING)
		offset = 0;
	if (state == MS_HIGHLIGHTED)
		offset = 1;

	_leds[index + offset].draw(screenManager);
}

void CPetStarfield::setButtons(int matchIndex, bool isMarkerClose) {
	_markerStates[0] = MS_BLANK;
	_markerStates[1] = MS_BLANK;
	_markerStates[2] = MS_BLANK;

	if (matchIndex >= 0)
		_markerStates[0] = MS_HIGHLIGHTED;
	if (matchIndex >= 1)
		_markerStates[1] = MS_HIGHLIGHTED;
	if (matchIndex >= 2)
		_markerStates[2] = MS_HIGHLIGHTED;

	if (isMarkerClose) {
		if (matchIndex == -1)
			_markerStates[0] = MS_FLICKERING;
		if (matchIndex == 0)
			_markerStates[1] = MS_FLICKERING;
		if (matchIndex == 1)
			_markerStates[2] = MS_FLICKERING;
	}

	_flickerCtr = (_flickerCtr + 1) % 8;
}

void CPetStarfield::makePetDirty() {
	_petControl->makeDirty();
}

bool CPetStarfield::markersMouseDown(CMouseButtonDownMsg *msg) {
	if (markerMouseDown(0, msg, _leds[0].getRawBounds()))
		return true;
	if (markerMouseDown(1, msg, _leds[2].getRawBounds()))
		return true;
	if (markerMouseDown(2, msg, _leds[4].getRawBounds()))
		return true;

	return false;
}

bool CPetStarfield::markerMouseDown(int index, CMouseButtonDownMsg *msg, const Rect &rect) {
	if (!rect.contains(msg->_mousePos))
		return false;

	switch (_markerStates[index]) {
	case MS_FLICKERING:
		// Marker is flickering, so lock it in
		if (_petControl->_remoteTarget) {
			CPETStarFieldLockMsg lockMsg(1);
			lockMsg.execute(_petControl->_remoteTarget);
		}
		break;

	case MS_HIGHLIGHTED:
		// Marker is locked in. If the most recently locked marker
		// is clicked on, allow it to be unlocked
		if (index == 2 || _markerStates[index + 1] != MS_HIGHLIGHTED) {
			if (_petControl->_remoteTarget) {
				CPETStarFieldLockMsg lockMsg(0);
				lockMsg.execute(_petControl->_remoteTarget);
			}
		}
		break;

	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
