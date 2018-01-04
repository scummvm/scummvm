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

#include "titanic/pet_control/pet_translation.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

CPetTranslation::CPetTranslation() {
	Rect rect1(32, 368, 586, 438);
	_message.setBounds(rect1);
	_message.resize(50);
	_message.setHasBorder(false);

	Rect rect2(32, 445, 586, 460);
	_tooltip.setBounds(rect2);
	_tooltip.setHasBorder(false);
}

bool CPetTranslation::setup(CPetControl *petControl) {
	if (petControl && setupControl(petControl))
		return reset();
	return false;
}

void CPetTranslation::draw(CScreenManager *screenManager) {
	_message.draw(screenManager);
	_tooltip.draw(screenManager);
}

bool CPetTranslation::setupControl(CPetControl *petControl) {
	if (petControl)
		_petControl = petControl;
	return true;
}

void CPetTranslation::clearTranslation() {
	_message.setup();
}

void CPetTranslation::addTranslation(const CString &str1, const CString &str2) {
	CString msg = CString::format("%s%s - %s%s",
		CTextControl::getColorText(0, 0, 0x80).c_str(), str1.c_str(),
		CTextControl::getColorText(0, 0, 0).c_str(), str2.c_str());
	_message.addLine(msg);
	_petControl->makeDirty();
}

} // End of namespace Titanic
