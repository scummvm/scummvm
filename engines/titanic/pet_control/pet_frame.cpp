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
	PET_ROOMS, PET_REAL_LIFE, PET_STARFIELD
};

CPetFrame::CPetFrame() : CPetSection() {
}

bool CPetFrame::setup(CPetControl *petControl) {
	if (setPetControl(petControl))
		return reset();
	return false;
}

bool CPetFrame::reset() {
	if (_petControl) {
		_background.reset("PetBackground", _petControl, MODE_UNSELECTED);
		_modeBackground.reset("PetModeBackground", _petControl, MODE_UNSELECTED);

		for (int idx = 0; idx < 5; ++idx) {
			CString resName = Common::String::format("PetMode%d", idx + 1);
			_modeButtons[idx].reset(resName, _petControl, MODE_SELECTED);
		}

		for (int idx = 0; idx < 6; ++idx) {
			CString resName = Common::String::format("3Pettitle%d", idx + 1);
			_titles[idx].reset(resName, _petControl, MODE_UNSELECTED);
		}

		for (int idx = 0; idx < TOTAL_GLYPHS; ++idx) {
			CString resName = Common::String::format("PetIndent%d", idx + 1);
			_squares[idx].reset(resName, _petControl, MODE_UNSELECTED);
		}
	}

	return true;
}

bool CPetFrame::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	for (int idx = 0; idx < 5; ++idx) {
		if (_modeButtons[idx].MouseButtonUpMsg(msg->_mousePos)) {
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
	reset();
}

bool CPetFrame::setPetControl(CPetControl *petControl) {
	if (petControl) {
		_petControl = petControl;

		// Set the bounds of the individual elements
		_background.setBounds(Rect(20, 350, 620, 480));
		_modeBackground.setBounds(Rect(590, 365, 611, 467));

		// Squares used for holding glyphs in various tabs
		Rect r(35, 373, 91, 429);
		for (int idx = 0, xp = 0; idx < TOTAL_GLYPHS; ++idx, xp += 70) {
			_squares[idx].setBounds(r);
			_squares[idx].translate(xp, 0);
		}

		// Draw the mode buttons vertically on the right edge of the PET
		r = Rect(590, 365, 606, 381);
		const int YLIST[] = { 7, 27, 45, 66, 84 };
		for (int idx = 0; idx < 5; ++idx) {
			_modeButtons[idx].setBounds(r);
			_modeButtons[idx].translate(4, YLIST[idx]);
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
	if (newArea < PET_TRANSLATION)
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

void CPetFrame::drawSquares(CScreenManager *screenManager, int count) {
	count = CLIP(count, 0, TOTAL_GLYPHS);
	for (int idx = 0; idx < count; ++idx)
		_squares[idx].draw(screenManager);
}

} // End of namespace Titanic
