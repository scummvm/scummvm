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

#ifndef ULTIMA4_CONTROLLERS_REAGENTS_MENU_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_REAGENTS_MENU_CONTROLLER_H

#include "ultima/ultima4/controllers/menu_controller.h"

namespace Ultima {
namespace Ultima4 {

class Ingredients;

/**
 * Controller for the reagents menu used when mixing spells.  Fills
 * the passed in Ingredients with the selected reagents.
 */
class ReagentsMenuController : public MenuController {
public:
	ReagentsMenuController(Menu *menu, Ingredients *i, TextView *view) : MenuController(menu, view), _ingredients(i) { }

	/**
	 * Handles spell mixing for the Ultima V-style menu-system
	 */
	bool keyPressed(int key) override;

	void keybinder(KeybindingAction action) override;

private:
	Ingredients *_ingredients;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
