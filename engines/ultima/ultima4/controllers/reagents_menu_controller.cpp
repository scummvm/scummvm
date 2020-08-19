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

#include "ultima/ultima4/controllers/reagents_menu_controller.h"
#include "ultima/ultima4/views/menu.h"
#include "ultima/ultima4/game/spell.h"

namespace Ultima {
namespace Ultima4 {

bool ReagentsMenuController::keyPressed(int key) {
	switch (key) {
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h': {
		// Select the corresponding reagent (if visible)
		Menu::MenuItemList::iterator mi = _menu->getById(key - 'a');
		if ((*mi)->isVisible()) {
			_menu->setCurrent(_menu->getById(key - 'a'));
			keyPressed(Common::KEYCODE_SPACE);
		}
		break;
	}

	default:
		return MenuController::keyPressed(key);
	}

	return true;
}

void ReagentsMenuController::keybinder(KeybindingAction action) {
	switch (action) {
	case KEYBIND_ESCAPE:
		_ingredients->revert();
		eventHandler->setControllerDone();
		break;

	case KEYBIND_UP:
		_menu->prev();
		break;

	case KEYBIND_DOWN:
		_menu->next();
		break;

	case KEYBIND_LEFT:
	case KEYBIND_RIGHT:
		if (_menu->isVisible()) {
			MenuItem *item = *_menu->getCurrent();

			// change whether or not it's selected
			item->setSelected(!item->isSelected());

			if (item->isSelected())
				_ingredients->addReagent((Reagent)item->getId());
			else
				_ingredients->removeReagent((Reagent)item->getId());
		}
		break;

	case KEYBIND_INTERACT:
		eventHandler->setControllerDone();
		break;

	default:
		break;
	}
}


} // End of namespace Ultima4
} // End of namespace Ultima
