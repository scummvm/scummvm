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

#include "titanic/pet_control/pet_load.h"
#include "titanic/core/project_item.h"
#include "titanic/game_manager.h"
#include "titanic/main_game_window.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/titanic.h"

namespace Titanic {

bool CPetLoad::reset() {
	CPetLoadSave::reset();

	CPetControl *pet = getPetControl();
	if (pet) {
		setName("PetLoad", pet);
		_btnLoadSave.reset("PetLoadOut", pet, MODE_UNSELECTED);
		_btnLoadSave.reset("PetLoadIn", pet, MODE_SELECTED);
	}

	return true;
}

bool CPetLoad::MouseButtonUpMsg(const Point &pt) {
	if (_btnLoadSave.MouseButtonUpMsg(pt)) {
		execute();
		return true;
	} else {
		return false;
	}
}

void CPetLoad::getTooltip(CTextControl *text) {
	text->setText(LOAD_THE_GAME);
}

void CPetLoad::execute() {
	CPetControl *pet = getPetControl();

	if (_savegameSlotNum >= 0 && _slotInUse[_savegameSlotNum]) {
		CMainGameWindow *window = g_vm->_window;

		// WORKAROUND: Schedule the savegame to be loaded after frame rendering ends
		window->loadGame(_savegameSlotNum);
	} else if (pet) {
		pet->displayMessage(SELECT_GAME_TO_LOAD);
	}
}

} // End of namespace Titanic
