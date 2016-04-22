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

#include "titanic/pet_control/pet_slider.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

CPetSlider::CPetSlider() {
	_field4 = 0;
	_field8 = 0;
	_field1C = 0;
	_field20 = 0;
	_field24 = 0;
	_field28 = 0;
	_field2C = 0;
	_field30 = 0;
	_field34 = 0;
}

void CPetSlider::initBounds(Rect *rect) {
	if (rect)
		*rect = _bounds2;
	_bounds2.clear();
}

void CPetSlider::proc8() {

}

void CPetSlider::proc9() {

}

void CPetSlider::proc10() {

}

void CPetSlider::proc11() {

}

void CPetSlider::proc12() {

}

void CPetSlider::proc13() {

}

void CPetSlider::proc14() {

}

void CPetSlider::proc15() {

}

void CPetSlider::proc16() {

}

void CPetSlider::proc17() {

}

void CPetSlider::proc18() {

}

/*------------------------------------------------------------------------*/

void CPetSoundSlider::setupBackground(const CString &name, CPetControl *petControl) {
	if (petControl) {
		_background = petControl->getHiddenObject(name);
	}
}

void CPetSoundSlider::setupThumb(const CString &name, CPetControl *petControl) {
	if (petControl) {
		_thumb = petControl->getHiddenObject(name);
	}
}

} // End of namespace Titanic
