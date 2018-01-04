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
#include "titanic/translation.h"

namespace Titanic {

static const PetArea PET_AREAS_EN[5] = {
	PET_CONVERSATION, PET_INVENTORY, PET_REMOTE,
	PET_ROOMS, PET_REAL_LIFE
};

static const PetArea PET_AREAS_DE[6] = {
	PET_CONVERSATION, PET_TRANSLATION, PET_INVENTORY, PET_REMOTE,
	PET_ROOMS, PET_REAL_LIFE
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

		for (uint idx = 0; idx < _petAreas.size(); ++idx) {
			CString resName = Common::String::format("PetMode%d", idx + 1);
			_modeButtons[idx].reset(resName, _petControl, MODE_SELECTED);
		}

		for (uint idx = 0; idx < ARRAYSIZE(_titles); ++idx) {
			CString resName = Common::String::format("3Pettitle%d", idx + 1);
			_titles[idx].setup(MODE_UNSELECTED, resName, _petControl);
		}

		for (int idx = 0; idx < TOTAL_GLYPHS; ++idx) {
			CString resName = Common::String::format("PetIndent%d", idx + 1);
			_squares[idx].reset(resName, _petControl, MODE_UNSELECTED);
		}
	}

	return true;
}

bool CPetFrame::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	for (uint idx = 0; idx < _petAreas.size(); ++idx) {
		if (_modeButtons[idx].MouseButtonUpMsg(msg->_mousePos)) {
			_petControl->setArea(_petAreas[idx]);
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
		_modeButtons[PET_CONVERSATION].setMode(MODE_UNSELECTED);
		_modeButtons[PET_REAL_LIFE].setMode(MODE_SELECTED);
	}

	return result;
}

void CPetFrame::postLoad() {
	reset();
}

bool CPetFrame::setPetControl(CPetControl *petControl) {
	if (petControl) {
		_petControl = petControl;

		// Set up the PET areas we'll have buttons for
		_petAreas.clear();
		if (g_language == Common::EN_ANY)
			_petAreas.assign(&PET_AREAS_EN[0], &PET_AREAS_EN[0] + 5);
		else
			_petAreas.assign(&PET_AREAS_DE[0], &PET_AREAS_DE[0] + 6);

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
		const int YLIST_EN[] = { 7, 27, 45, 66, 84 };
		const int YLIST_DE[] = { 0, 18, 36, 51, 67, 84 };
		_modeButtons.resize(_petAreas.size());
		for (uint idx = 0; idx < _modeButtons.size(); ++idx) {
			_modeButtons[idx].setBounds(r);
			_modeButtons[idx].translate(TRANSLATE(4, 0),
				TRANSLATE(YLIST_EN[idx], YLIST_DE[idx]));
		}
		setArea(PET_CONVERSATION);

		if (g_language == Common::EN_ANY) {
			const int XLIST_EN[] = { 73, 54, 85, 109, 38, 71 };
			for (uint idx = 0; idx < _petAreas.size(); ++idx) {
				_titles[idx].setBounds(Rect(0, 0, 110, 11));
				_titles[idx].translate(608 - XLIST_EN[idx], 471);
			}
		} else {
			for (uint idx = 0; idx < 7; ++idx) {
				_titles[idx].setBounds(Rect(0, 0, 110, 11));
				_titles[idx].translate(501, 469);
			}
		}
	}

	return true;
}

void CPetFrame::setArea(PetArea newArea) {
	resetArea();

	for (uint idx = 0; idx < _modeButtons.size(); ++idx) {
		if (_petAreas[idx] == newArea)
			_modeButtons[idx].setMode(MODE_SELECTED);
	}
}

void CPetFrame::resetArea() {
	for (uint idx = 0; idx < _modeButtons.size(); ++idx)
		_modeButtons[idx].setMode(MODE_UNSELECTED);
}

void CPetFrame::drawFrame(CScreenManager *screenManager) {
	_background.draw(screenManager);
	_modeBackground.draw(screenManager);

	for (uint idx = 0; idx < _modeButtons.size(); ++idx)
		_modeButtons[idx].draw(screenManager);

	_titles[_petControl->_currentArea].draw(screenManager);
}

void CPetFrame::drawSquares(CScreenManager *screenManager, int count) {
	count = CLIP(count, 0, TOTAL_GLYPHS);
	for (int idx = 0; idx < count; ++idx)
		_squares[idx].draw(screenManager);
}

} // End of namespace Titanic
