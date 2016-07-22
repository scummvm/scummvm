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
#include "titanic/pet_control/pet_control.h"
#include "titanic/core/project_item.h"
#include "titanic/game_manager.h"

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

void CPetLoad::getTooltip(CPetText *text) {
	text->setText("Load the game.");
}

void CPetLoad::execute() {
	CPetControl *pet = getPetControl();

	if (_savegameSlotNum >= 0 && _slotInUse[_savegameSlotNum]) {
		CProjectItem *project = pet ? pet->getRoot() : nullptr;
		CGameManager *gameManager = project ? project->getGameManager() : nullptr;

		if (project && gameManager) {
			pet->displayMessage("Loading the selected game, please wait.");

			gameManager->destroyTreeItem();
			gameManager->initBounds();
			project->loadGame(_savegameSlotNum);
		}
	} else if (pet) {
		pet->displayMessage("You must select a game to load first.");
	}
}

} // End of namespace Titanic
