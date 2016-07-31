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

#include "titanic/pet_control/pet_save.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

bool CPetSave::reset() {
	CPetLoadSave::reset();
	
	CPetControl *pet = getPetControl();
	if (pet) {
		setName("PetSave", pet);
		_btnLoadSave.reset("PetSaveOut", pet, MODE_UNSELECTED);
		_btnLoadSave.reset("PetSaveIn", pet, MODE_SELECTED);
	}

	return true;
}

bool CPetSave::MouseButtonUpMsg(const Point &pt) {
	if (_btnLoadSave.MouseButtonUpMsg(pt)) {
		execute();
		resetSlots();
		return true;
	} else {
		return false;
	}
}

void CPetSave::highlightCurrent(const Point &pt) {
	resetSlots();
	highlightSave(_savegameSlotNum);
}

void CPetSave::getTooltip(CPetText *text) {
	text->setText("Save the game.");
}

void CPetSave::highlightSave(int index) {
	warning("TODO: CPetSave::highlightSave");
}

void CPetSave::unhighlightSave(int index) {
	warning("TODO: CPetSave::unhighlightSave");
}

void CPetSave::execute() {
	warning("TODO: CPetSave::execute");
}

} // End of namespace Titanic
