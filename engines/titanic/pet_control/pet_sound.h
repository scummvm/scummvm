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

#ifndef TITANIC_PET_SOUND_H
#define TITANIC_PET_SOUND_H

#include "titanic/pet_control/pet_glyphs.h"
#include "titanic/pet_control/pet_gfx_element.h"
#include "titanic/gfx/text_control.h"
#include "titanic/pet_control/pet_slider.h"

namespace Titanic {

class CPetRealLife;

class CPetSound : public CPetGlyph {
	enum SliderType {
		MASTER_SLIDER = 0, MUSIC_SLIDER = 1, PARROT_SLIDER = 2, SPEECH_SLIDER = 3
	};
private:
	CPetGfxElement _element;
	CPetSoundSlider _masterVolume;
	CPetSoundSlider _musicVolume;
	CPetSoundSlider _parrotVolume;
	CPetSoundSlider _speechVolume;
	CTextControl _textMasterVolume;
	CTextControl _textMusicVolume;
	CTextControl _textParrotVolume;
	CTextControl _textSpeechVolume;
	CPetSlider *_draggingSlider;
	SliderType _draggingSliderNum;
private:
	/**
	 * Called when a slider has changed
	 */
	void sliderChanged(double offset, SliderType sliderNum);
public:
	CPetSound();

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Reset the glyph
	 */
	bool reset() override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override;

	/**
	 * Called for mouse button down messages
	 */
	bool MouseButtonDownMsg(const Point &pt) override;

	/**
	 * Called when mouse drag starts
	 */
	bool MouseDragStartMsg(CMouseDragStartMsg *msg) override;

	/**
	 * Called during mouse drags
	 */
	bool MouseDragMoveMsg(CMouseDragMoveMsg *msg) override;

	/**
	 * Called when mouse drag ends
	 */
	bool MouseDragEndMsg(CMouseDragEndMsg *msg) override;

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	 * Highlight any currently highlighted element
	 */
	void highlightCurrent(const Point &pt) override {
		setSliders();
	}

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;

	/**
	 * Sets the positions of the volume sliders
	 */
	void setSliders();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_SOUND_H */
