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
#include "titanic/titanic.h"
#include "common/config-manager.h"

namespace Titanic {

CPetSound::CPetSound() : CPetGlyph(), _draggingSlider(nullptr), _draggingSliderNum(MASTER_SLIDER) {
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

	_speechVolume.setOrientation(ORIENTATION_HORIZONTAL);
	_speechVolume.setBounds(Rect(17, 60, 147, 75));
	_speechVolume.setSlidingBounds(Rect(35, 65, 127, 71));
	_speechVolume.setThumbSize(Point(25, 15));
	_speechVolume.translate(Point(415, 376));

	_element.setBounds(Rect(0, 0, 165, 77));
	_element.translate(Point(415, 376));

	Rect rect(0, 0, 88, 16);
	rect.translate(320, 376);
	_textMasterVolume.setBounds(rect);
	_textMasterVolume.resize(3);
	_textMasterVolume.setHasBorder(false);
	_textMasterVolume.setText(MASTER_VOLUME);

	rect.translate(0, 20);
	_textMusicVolume.setBounds(rect);
	_textMusicVolume.resize(3);
	_textMusicVolume.setHasBorder(false);
	_textMusicVolume.setText(MUSIC_VOLUME);

	rect.translate(0, 20);
	_textParrotVolume.setBounds(rect);
	_textParrotVolume.resize(3);
	_textParrotVolume.setHasBorder(false);
	_textParrotVolume.setText(PARROT_VOLUME);

	rect.translate(0, 20);
	_textSpeechVolume.setBounds(rect);
	_textSpeechVolume.resize(3);
	_textSpeechVolume.setHasBorder(false);
	_textSpeechVolume.setText(SPEECH_VOLUME);

	return true;
}

bool CPetSound::reset() {
	CPetControl *pet = getPetControl();
	if (pet) {
		setName("PetSound", pet);
		_element.reset("PetVolChannels", pet, MODE_UNSELECTED);
		_musicVolume.setupThumb2("PetVolSlug", pet);
		_masterVolume.setupThumb2("PetVolSlug", pet);
		_parrotVolume.setupThumb2("PetVolSlug", pet);
		_speechVolume.setupThumb2("PetVolSlug", pet);

		CPetSection *section = getPetSection();
		uint col = section->getColor(0);
		_textMusicVolume.setLineColor(0, col);
		_textMasterVolume.setLineColor(0, col);
		_textParrotVolume.setLineColor(0, col);
		_textSpeechVolume.setLineColor(0, col);
	}

	return false;
}

void CPetSound::setSliders() {
	// Get the mute settings
	bool muteAll = ConfMan.hasKey("mute") ? ConfMan.getBool("mute") : false;
	bool musicMute = muteAll || (ConfMan.hasKey("music_mute") && ConfMan.getBool("music_mute"));
	bool sfxMute = muteAll || (ConfMan.hasKey("sfx_mute") && ConfMan.getBool("sfx_mute"));
	bool speechMute = muteAll || (ConfMan.hasKey("speech_mute") && ConfMan.getBool("speech_mute"));

	// Get the volume levels
	uint musicVol = musicMute ? 0 : MIN(255, ConfMan.getInt("music_volume"));
	uint parrotVol = sfxMute ? 0 : MIN(255, ConfMan.getInt("sfx_volume"));
	uint speechVol = speechMute ? 0 : MIN(255, ConfMan.getInt("speech_volume"));
	uint masterVol = MAX(musicVol, MAX(parrotVol, speechVol));

	const double FACTOR = 1.0 / 255.0;
	_masterVolume.setSliderOffset(masterVol * FACTOR);
	_musicVolume.setSliderOffset(musicVol * FACTOR);
	_parrotVolume.setSliderOffset(parrotVol * FACTOR);
	_speechVolume.setSliderOffset(speechVol * FACTOR);
}

void CPetSound::sliderChanged(double offset, SliderType sliderNum) {
	uint newVol = (uint)(offset * 255.0);

	switch (sliderNum) {
	case MASTER_SLIDER:
		ConfMan.setBool("music_mute", false);
		ConfMan.setBool("sfx_mute", false);
		ConfMan.setBool("sfx_mute", false);
		ConfMan.setInt("music_volume", newVol);
		ConfMan.setInt("sfx_volume", newVol);
		ConfMan.setInt("speech_volume", newVol);

		_musicVolume.setSliderOffset(newVol * 0.01);
		_parrotVolume.setSliderOffset(newVol * 0.01);
		_speechVolume.setSliderOffset(newVol * 0.01);
		break;
	case MUSIC_SLIDER:
		ConfMan.setBool("music_mute", false);
		ConfMan.setInt("music_volume", newVol);
		break;
	case PARROT_SLIDER:
		ConfMan.setBool("sfx_mute", false);
		ConfMan.setInt("sfx_volume", newVol);
		break;
	case SPEECH_SLIDER:
		ConfMan.setBool("speech_mute", false);
		ConfMan.setInt("speech_volume", newVol);
		break;
	default:
		return;
	}

	ConfMan.setBool("mute", false);
	g_vm->syncSoundSettings();
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
	for (int idx = MASTER_SLIDER; idx <= SPEECH_SLIDER; ++idx) {
		CPetSlider *slider = sliders[idx];
		const bool isLeft = rectLeft.contains(pt);
		const bool isRight = rectRight.contains(pt);
		double offset = 0.0;

		if (isLeft) {
			slider->stepPosition(-1);
			offset = slider->getOffsetPixels();
		} else if (isRight) {
			slider->stepPosition(1);
			offset = slider->getOffsetPixels();
		}

		if (isLeft || isRight) {
			sliderChanged(offset, (SliderType)idx);
			return true;
		}

		// Move to next slider row
		rectLeft.translate(0, 20);
		rectRight.translate(0, 20);
	}

	return false;
}

bool CPetSound::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (_masterVolume.resetThumbFocus()) {
		_draggingSlider = &_masterVolume; 
		getOwner()->startDragging(this, msg);
		_draggingSliderNum = MASTER_SLIDER;
		return true;
	} else if (_musicVolume.resetThumbFocus()) {
		_draggingSlider = &_musicVolume;
		getOwner()->startDragging(this, msg);
		_draggingSliderNum = MUSIC_SLIDER;
		return true;
	} else if (_parrotVolume.resetThumbFocus()) {
		_draggingSlider = &_parrotVolume;
		getOwner()->startDragging(this, msg);
		_draggingSliderNum = PARROT_SLIDER;
		return true;
	} else if (_speechVolume.resetThumbFocus()) {
		_draggingSlider = &_speechVolume;
		getOwner()->startDragging(this, msg);
		_draggingSliderNum = SPEECH_SLIDER;
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

	// Flush the changed settings
	ConfMan.flushToDisk();

	bool result = _draggingSlider->MouseDragEndMsg(msg->_mousePos);
	getOwner()->endDragging();

	return result;
}

bool CPetSound::MouseButtonUpMsg(const Point &pt) {
	SliderType sliderNum = MASTER_SLIDER;
	CPetSlider *slider = nullptr;

	if (_masterVolume.MouseButtonUpMsg(pt)) {
		sliderNum = MASTER_SLIDER;
		slider = &_masterVolume;
	} else if (_musicVolume.MouseButtonUpMsg(pt)) {
		sliderNum = MUSIC_SLIDER;
		slider = &_musicVolume;
	} else if (_parrotVolume.MouseButtonUpMsg(pt)) {
		sliderNum = PARROT_SLIDER;
		slider = &_parrotVolume;
	} else if (_speechVolume.MouseButtonUpMsg(pt)) {
		sliderNum = SPEECH_SLIDER;
		slider = &_speechVolume;
	} else {
		return false;
	}

	double offset = slider->getOffsetPixels();
	sliderChanged(offset, sliderNum);
	return true;
}

void CPetSound::getTooltip(CTextControl *text) {
	text->setText(CHANGE_VOLUME_SETTINGS);
}

} // End of namespace Titanic
