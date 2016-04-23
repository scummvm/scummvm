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

namespace Titanic {

CPetSound::CPetSound() : CPetGlyph(), _field198(0), _field19C(0) {
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
		_textMusicVolume.setColor(0, col);
		_textMasterVolume.setColor(0, col);
		_textParrotVolume.setColor(0, col);
		_textSpeechVolume.setColor(0, col);
	}

	return false;
}


} // End of namespace Titanic
