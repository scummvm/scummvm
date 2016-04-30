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

#include "titanic/pet_control/pet_nav_helmet.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

CPetNavHelmet::CPetNavHelmet() :
		_field98(0), _field9C(0), _fieldA0(0), _field18C(0),
		_field20C(1), _field210(0), _rect1(22, 352, 598, 478) {
}

bool CPetNavHelmet::setup(CPetControl *petControl) {
	if (petControl && setupControl(petControl))
		return reset();
	return false;
}

bool CPetNavHelmet::reset() {
	if (_petControl) {
		_val1.setup(MODE_UNSELECTED, "3PetStarField", _petControl);
		_val2.setup(MODE_UNSELECTED, "HomePhotoOnOff", _petControl);
		_val3.setup(MODE_UNSELECTED, "3PetSetDestin", _petControl);
		_val3.setup(MODE_SELECTED, "3PetSetDestin1", _petControl);
		_val4.setup(MODE_UNSELECTED, "3PetStarCtrl", _petControl);
		
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

void CPetNavHelmet::draw(CScreenManager *screenManager) {
	_petControl->drawSquares(screenManager, 2);

	if (_field20C) {
		_val2.draw(screenManager);
	} else {
		_val4.draw(screenManager);
	}

	_val3.draw(screenManager);
	drawButton(_field98, 0, screenManager);
	drawButton(_field9C, 2, screenManager);
	drawButton(_fieldA0, 4, screenManager);
	_text.draw(screenManager);
}

bool CPetNavHelmet::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return false;
}

bool CPetNavHelmet::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	return false;
}

bool CPetNavHelmet::isValid(CPetControl *petControl) {
	return setupControl(petControl);
}

void CPetNavHelmet::load(SimpleFile *file, int param) {
	if (!param) {
		_field20C = file->readNumber();
		_field210 = file->readNumber();
	}
}

void CPetNavHelmet::postLoad() {
	reset();
}

void CPetNavHelmet::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(_field20C, indent);
	file->writeNumberLine(_field210, indent);
}

bool CPetNavHelmet::setupControl(CPetControl *petControl) {
	if (petControl) {
		_petControl = petControl;

		Rect r(0, 0, 64, 64);
		r.translate(_rect1.left, _rect1.top);

		_val1.setBounds(r);
		_val1.translate(15, 23);
		_val2.setBounds(r);
		_val2.translate(85, 23);
		_val4.setBounds(r);
		_val4.translate(85, 23);

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
		_val3.setBounds(r);

		r = Rect(0, 0, 580, 15);
		r.translate(32, 445);
		_text.setBounds(r);
		_text.setHasBorder(false);
	}

	return true;
}

void CPetNavHelmet::drawButton(int offset, int index, CScreenManager *screenManager) {
	if (_field18C < 4 && (offset / 3) == 1)
		--offset;
	if (offset == 2)
		offset = 1;

	_leds[index + offset].draw(screenManager);
}

} // End of namespace Titanic
