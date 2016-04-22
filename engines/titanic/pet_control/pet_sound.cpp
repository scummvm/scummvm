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
	// TODO

	return true;
}

bool CPetSound::reset() {
	CPetControl *pet = getPetControl();
	if (pet) {
		setName("PetSound", pet);
		_element.reset("PetVolChannels", pet, MODE_UNSELECTED);
		_slider1.reset("PetVolSlug");
		_slider2.reset("PetVolSlug");
		_slider3.reset("PetVolSlug");
		_slider4.reset("PetVolSlug");

		CPetSection *section = getPetSection();
		uint col = section->getColor(0);
		for (int idx = 0; idx < 4; ++idx)
			_text[idx].setColor(0, col);
	}

	return false;
}


} // End of namespace Titanic
