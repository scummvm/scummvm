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

#include "titanic/pet_control/pet_load_save.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/core/project_item.h"
#include "titanic/titanic.h"

namespace Titanic {

int CPetLoadSave::_savegameSlotNum;

bool CPetLoadSave::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetGlyph::setup(petControl, owner);
	_savegameSlotNum = -1;

	for (int idx = 0; idx < SAVEGAME_SLOTS_COUNT; ++idx) {
		Rect slotRect = getSlotBounds(idx);
		_slotNames[idx].setBounds(slotRect);
		_slotNames[idx].resize(3);
		_slotNames[idx].setMaxCharsPerLine(22);
		_slotNames[idx].setHasBorder(false);
		_slotNames[idx].setup();
	}

	Rect r1(0, 0, 68, 52);
	r1.moveTo(496, 388);
	_btnLoadSave.setBounds(r1);

	Rect r2(0, 0, 168, 78);
	r2.moveTo(309, 377);
	_gutter.setBounds(r2);
	return true;
}

bool CPetLoadSave::reset() {
	highlightChange();
	resetSlots();

	CPetControl *pet = getPetControl();
	if (pet) {
		_gutter.reset("PetSaveGutter", pet, MODE_UNSELECTED);
	}

	return true;
}

void CPetLoadSave::draw2(CScreenManager *screenManager) {
	_gutter.draw(screenManager);

	for (int idx = 0; idx < SAVEGAME_SLOTS_COUNT; ++idx)
		_slotNames[idx].draw(screenManager);

	_btnLoadSave.draw(screenManager);
}

bool CPetLoadSave::MouseButtonDownMsg(const Point &pt) {
	if (_btnLoadSave.MouseButtonDownMsg(pt))
		return true;

	checkSlotsHighlight(pt);
	return false;
}

bool CPetLoadSave::KeyCharMsg(int key) {
	switch (key) {
	case Common::KEYCODE_TAB:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		if (_savegameSlotNum != -1) {
			highlightSlot((_savegameSlotNum + 1) % 5);
			getPetControl()->makeDirty();
		}
		return true;

	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		if (_savegameSlotNum != -1) {
			int slotNum = --_savegameSlotNum;
			highlightSlot((slotNum == -1) ? SAVEGAME_SLOTS_COUNT - 1 : slotNum);
			getPetControl()->makeDirty();
		}
		return true;

	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		execute();
		return true;

	default:
		return false;
	}
}

Rect CPetLoadSave::getSlotBounds(int index) {
	return Rect(323, 376 + index * 16, 473, 392 + index * 16);
}

void CPetLoadSave::resetSlots() {
	for (int idx = 0; idx < SAVEGAME_SLOTS_COUNT; ++idx) {
		_slotNames[idx].setText(EMPTY);
		_slotInUse[idx] = false;

		// Try and open up the savegame for access
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(
			g_vm->getSaveStateName(idx));

		if (in) {
			// Read in the savegame header data
			CompressedFile file;
			file.open(in);

			TitanicSavegameHeader header;
			if (CProjectItem::readSavegameHeader(&file, header)) {
				_slotInUse[idx] = true;
				_slotNames[idx].setText(header._saveName);
			}

			file.close();
		}
	}

	highlightSlot(_savegameSlotNum);
}

void CPetLoadSave::highlightSlot(int index) {
	unhighlightSave(_savegameSlotNum);
	_savegameSlotNum = index;
	highlightChange();
	highlightSave(_savegameSlotNum);
}

void CPetLoadSave::highlightChange() {
	CPetSection *section = getPetSection();

	uint col = section ? section->getColor(3) : 0;
	for (int idx = 0; idx < SAVEGAME_SLOTS_COUNT; ++idx)
		_slotNames[idx].setLineColor(0, col);

	if (_savegameSlotNum != -1) {
		col = section ? section->getColor(4) : 0;
		_slotNames[_savegameSlotNum].setLineColor(0, col);
	}
}

bool CPetLoadSave::checkSlotsHighlight(const Point &pt) {
	for (int idx = 0; idx < SAVEGAME_SLOTS_COUNT; ++idx) {
		if (isSlotHighlighted(idx, pt)) {
			highlightSlot(idx);
			return true;
		}
	}

	return false;
}

bool CPetLoadSave::isSlotHighlighted(int index, const Point &pt) {
	Rect r = getSlotBounds(index);
	if (r.contains(pt)) {
		highlightSlot(index);
		return true;
	} else {
		return false;
	}
}

} // End of namespace Titanic
