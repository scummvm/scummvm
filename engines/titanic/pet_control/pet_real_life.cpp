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

#include "titanic/pet_control/pet_real_life.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

bool CPetRealLife::setup(CPetControl *petControl) {
	if (petControl && setupControl(petControl))
		return reset();
	return false;
}

bool CPetRealLife::reset() {
	return true;
}

void CPetRealLife::draw(CScreenManager *screenManager) {
	_petControl->drawSquares(screenManager, 4);
	_glyphs.draw(screenManager);
	_sub12.draw(screenManager);
}

bool CPetRealLife::setupControl(CPetControl *petControl) {
	if (petControl) {

	}

	return true;
}

bool CPetRealLife::isValid(CPetControl *petControl) {
	setupControl(petControl);
	return true;
}

} // End of namespace Titanic
