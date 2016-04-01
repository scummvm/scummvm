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

#include "titanic/pet_control/pet_frame.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

static const PetArea PET_AREAS[6] = {
	PET_CONVERSATION, PET_INVENTORY, PET_REMOTE,
	PET_ROOMS, PET_SAVE, PET_5
};

CPetFrame::CPetFrame() : CPetSection() {
}

bool CPetFrame::setup(CPetControl *petControl) {
	if (setPetControl(petControl))
		return setup();
	return false;
}

bool CPetFrame::setup() {
	if (_petControl) {
		_background.setup("PetBackground", _petControl, MODE_UNSELECTED);
		_modeBackground.setup("PetModeBackground", _petControl, MODE_UNSELECTED);

		for (int idx = 0; idx < 5; ++idx) {
			CString resName = Common::String::format("PetMode%d", idx);
			_modeButtons[idx].setup(resName, _petControl, MODE_UNSELECTED);
		}

		for (int idx = 0; idx < 6; ++idx) {
			CString resName = Common::String::format("3Pettitle%d", idx);
			_titles[idx].setup(resName, _petControl, MODE_UNSELECTED);
		}

		for (int idx = 0; idx < 7; ++idx) {
			CString resName = Common::String::format("PetIndent%d", idx);
			_indent[idx].setup(resName, _petControl, MODE_UNSELECTED);
		}
	}

	return true;
}

bool CPetFrame::handleMessage(CMouseButtonDownMsg &msg) {
	for (int idx = 0; idx < 5; ++idx) {
		if (_modeButtons[idx].handleMessage(msg)) {
			_petControl->setArea(PET_AREAS[idx]);
			resetArea();
			_modeButtons[idx].setMode(MODE_SELECTED);
			return true;
		}
	}

	return false;
}

bool CPetFrame::isValid(CPetControl *petControl) {
	bool result = setPetControl(petControl);
	if (result) {
		_modeButtons[PET_AREAS[0]].setMode(MODE_UNSELECTED);
		_modeButtons[PET_AREAS[4]].setMode(MODE_SELECTED);
	}

	return result;
}

void CPetFrame::postLoad() {
	setup();
}

bool CPetFrame::setPetControl(CPetControl *petControl) {
	if (petControl) {
		_petControl = petControl;

		// Set the bounds of the individual elements
		_background.setBounds(Rect(20, 350, 620, 480));
		_modeBackground.setBounds(Rect(590, 365, 611, 467));

		Rect r(35, 373, 91, 429);
		for (int idx = 0, xp = 0; xp < 490; ++idx, xp += 70) {
			_indent[idx].setBounds(r);
			_indent[idx].translate(xp, 0);
		}

		r = Rect(590, 365, 606, 381);
		const int YLIST[] = { 7, 27, 45, 66, 84 };
		for (int idx = 0; idx < 5; ++idx) {
			_modeButtons[idx].setBounds(r);
			_modeButtons[idx].translate(0, YLIST[idx]);
		}
		_modeButtons[PET_AREAS[0]].setMode(MODE_SELECTED);
		
		const int XLIST[] = { 73, 54, 85, 109, 38, 71 };
		for (int idx = 0; idx < 6; ++idx) {
			_titles[idx].setBounds(Rect(0, 0, 110, 11));
			_titles[idx].translate(600 - XLIST[idx], 471);
		}
	}

	return true;
}

void CPetFrame::setArea(PetArea newArea) {
	resetArea();
	if (newArea < PET_6)
		_modeButtons[PET_AREAS[newArea]].setMode(MODE_SELECTED);
}

void CPetFrame::resetArea() {
	for (int idx = 0; idx < 6; ++idx)
		_modeButtons[idx].setMode(MODE_UNSELECTED);
}

void CPetFrame::drawFrame(CScreenManager *screenManager) {
	_background.draw(screenManager);
	_modeBackground.draw(screenManager);

	for (int idx = 0; idx < 5; ++idx)
		_modeButtons[idx].draw(screenManager);

	_titles[_petControl->_currentArea].draw(screenManager);
}

} // End of namespace Titanic
