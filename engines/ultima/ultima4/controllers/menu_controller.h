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

#ifndef ULTIMA4_CONTROLLERS_MENU_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_MENU_CONTROLLER_H

#include "ultima/ultima4/controllers/controller.h"

namespace Ultima {
namespace Ultima4 {

class Menu;
class TextView;

/**
 * This class controls a menu.  The value field of WaitableController
 * isn't used.
 */
class MenuController : public WaitableController<void *> {
public:
	MenuController(Menu *menu, TextView *view);

	/**
	 * Called when a controller is made active
	 */
	void setActive() override;

	/**
	 * Handles keybinder actions
	 */
	void keybinder(KeybindingAction action) override;

	/**
	 * Key was pressed
	 */
	bool keyPressed(int key) override;

	/**
	 * Mouse button was pressed
	 */
	bool mousePressed(const Common::Point &mousePos) override;
protected:
	Menu *_menu;
	TextView *_view;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
