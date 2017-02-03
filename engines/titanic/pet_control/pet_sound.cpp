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

#include "titanic/pet_control/pet_sound.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/pet_control/pet_real_life.h"
#include "titanic/game_manager.h"

namespace Titanic {

CPetSound::CPetSound() : CPetGlyph(), _draggingSlider(nullptr), _draggingSliderNum(0) {
}

bool CPetSound::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetGlyph::setup(petControl, owner);

	_masterVolume.setOrientation(ORIENTATION_HORIZONTAL);
	_masterVolume.setBounds(Rect(17, 0, 147, 15));
	_masterVolume.setSlidingBounds(Rect(35, 5, 127, 11));
	_masterVolume.setThumbSize(Point(25, 15));
	_masterVolume.translate(Point(415, 376));

	_musicVolume.setOrientation(ORIENTATION_HORIZONTAL);
	_musicVolume.setBounds(Rect(17, 20, 147, 35));
	_musicVolume.setSlidingBounds(Rect(35, 25, 127, 31));
	_musicVolume.setThumbSize(Point(25, 15));
	_musicVolume.translate(Point(415, 376));

	_parrotVolume.setOrientation(ORIENTATION_HORIZONTAL);
	_parrotVolume.setBounds(Rect(17, 40, 147, 55));
	_parrotVolume.setSlidingBounds(Rect(35, 45, 127, 51));
	_parrotVolume.setThumbSize(Point(25, 15));
	_parrotVolume.translate(Point(415, 376));

	_parrotVolume.setOrientation(ORIENTATION_HORIZONTAL);
	_parrotVolume.setBounds(Rect(17, 60, 147, 75));
	_parrotVolume.setSlidingBounds(Rect(35, 65, 127, 71));
	_parrotVolume.setThumbSize(Point(25, 15));
	_parrotVolume.translate(Point(415, 376));

	_element.setBounds(Rect(0, 0, 165, 77));
	_element.translate(Point(415, 376));

	Rect rect(0, 0, 88, 16);
	rect.translate(320, 376);
	_textMasterVolume.setBounds(rect);
	_textMasterVolume.resize(3);
	_textMasterVolume.setHasBorder(false);
	_textMasterVolume.setText("Master volume");

	rect.translate(0, 20);
	_textMusicVolume.setBounds(rect);
	_textMusicVolume.resize(3);
	_textMusicVolume.setHasBorder(false);
	_textMusicVolume.setText("Music volume");

	rect.translate(0, 20);
	_textParrotVolume.setBounds(rect);
	_textParrotVolume.resize(3);
	_textParrotVolume.setHasBorder(false);
	_textParrotVolume.setText("Parrot volume");

	rect.translate(0, 20);
	_textSpeechVolume.setBounds(rect);
	_textSpeechVolume.resize(3);
	_textSpeechVolume.setHasBorder(false);
	_textSpeechVolume.setText("Speech volume");

	return true;
}

bool CPetSound::reset() {
	CPetControl *pet = getPetControl();
	if (pet) {
		setName("PetSound", pet);
		_element.reset("PetVolChannels", pet, MODE_UNSELECTED);
		_musicVolume.reset("PetVolSlug");
		_masterVolume.reset("PetVolSlug");
		_parrotVolume.reset("PetVolSlug");
		_speechVolume.reset("PetVolSlug");

		CPetSection *section = getPetSection();
		uint col = section->getColor(0);
		_textMusicVolume.setLineColor(0, col);
		_textMasterVolume.setLineColor(0, col);
		_textParrotVolume.setLineColor(0, col);
		_textSpeechVolume.setLineColor(0, col);
	}

	return false;
}

void CPetSound::draw2(CScreenManager *screenManager) {
	_element.draw(screenManager);

	_musicVolume.draw(screenManager);
	_masterVolume.draw(screenManager);
	_parrotVolume.draw(screenManager);
	_speechVolume.draw(screenManager);

	_textMusicVolume.draw(screenManager);
	_textMasterVolume.draw(screenManager);
	_textParrotVolume.draw(screenManager);
	_textSpeechVolume.draw(screenManager);
}

bool CPetSound::MouseButtonDownMsg(const Point &pt) {
	if (_musicVolume.checkThumb(pt) || _masterVolume.checkThumb(pt) ||
		_speechVolume.checkThumb(pt))
		return true;

	if (_parrotVolume.checkThumb(pt)) {
		CPetControl *pet = getPetControl();
		if (pet)
			pet->playSound(2);

		return true;
	}

	Rect rectLeft(0, 0, 10, 11);
	Rect rectRight(0, 0, 10, 11);
	rectLeft.translate(415, 379);
	rectRight.translate(567, 378);

	CPetSlider *sliders[4] = { &_masterVolume, &_musicVolume, &_parrotVolume, &_speechVolume };
	for (int idx = 0; idx < 4; ++idx) {
		CPetSlider *slider = sliders[idx];
		bool isLeft = rectLeft.contains(pt);
		bool isRight = rectRight.contains(pt);
		double offset;

		if (isLeft) {
			slider->stepPosition(-1);
			offset = slider->getOffsetPixels();
		} else if (isRight) {
			slider->stepPosition(1);
			offset = slider->getOffsetPixels();
		}

		if (isLeft || isRight) {
			sliderChanged(offset, idx);
			return true;
		}

		// Move to next slider row
		rectLeft.translate(0, 20);
		rectRight.translate(0, 20);
	}

	return false;
}

void CPetSound::sliderChanged(double offset, int sliderNum) {
	CPetControl *pet = getPetControl();
	if (!pet)
		return;

	CGameManager *gameManager = pet->getGameManager();
	if (!gameManager)
		return;

	QSoundManager &soundManager = gameManager->_sound._soundManager;
	double percent = offset * 100.0;

	switch (sliderNum) {
	case 0:
		soundManager.setMasterPercent(percent);
		break;
	case 1:
		soundManager.setMusicPercent(percent);
		break;
	case 2:
		soundManager.setParrotPercent(percent);
		break;
	case 3:
		soundManager.setSpeechPercent(percent);
		break;
	default:
		break;
	}
}

bool CPetSound::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (_musicVolume.resetThumbFocus()) {
		_draggingSlider = &_musicVolume;
		getOwner()->startDragging(this, msg);
		_draggingSliderNum = 0;
		return true;
	} else if (_masterVolume.resetThumbFocus()) {
		_draggingSlider = &_masterVolume;
		getOwner()->startDragging(this, msg);
		_draggingSliderNum = 1;
		return true;
	} else if (_parrotVolume.resetThumbFocus()) {
		_draggingSlider = &_parrotVolume;
		getOwner()->startDragging(this, msg);
		_draggingSliderNum = 2;
		return true;
	} else if (_speechVolume.resetThumbFocus()) {
		_draggingSlider = &_speechVolume;
		getOwner()->startDragging(this, msg);
		_draggingSliderNum = 3;
		return true;
	}

	_draggingSlider = nullptr;
	return false;
}

bool CPetSound::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	if (!_draggingSlider)
		return false;

	if (_draggingSlider->MouseDragMoveMsg(msg->_mousePos)) {
		double offset = _draggingSlider->getOffsetPixels();
		sliderChanged(offset, _draggingSliderNum);
		getPetControl()->makeDirty();
		return true;
	}

	return false;
}

bool CPetSound::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	if (!_draggingSlider)
		return false;

	_draggingSlider->MouseDragEndMsg(msg->_mousePos);
	getOwner()->endDragging();

	return false;
}

bool CPetSound::MouseButtonUpMsg(const Point &pt) {
	int sliderNum = 0;
	CPetSlider *slider = nullptr;

	if (_musicVolume.MouseButtonUpMsg(pt)) {
		sliderNum = 0;
		slider = &_musicVolume;
	} else if (_masterVolume.MouseButtonUpMsg(pt)) {
		sliderNum = 1;
		slider = &_masterVolume;
	} else if (_parrotVolume.MouseButtonUpMsg(pt)) {
		sliderNum = 2;
		slider = &_parrotVolume;
	} else if (_speechVolume.MouseButtonUpMsg(pt)) {
		sliderNum = 3;
		slider = &_speechVolume;
	} else {
		return false;
	}

	double offset = slider->getOffsetPixels();
	sliderChanged(offset, sliderNum);
	return true;
}

void CPetSound::getTooltip(CTextControl *text) {
	text->setText("Change the volume settings.");
}

} // End of namespace Titanic
