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

#include "ultima/ultima4/controllers/menu_controller.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/game/menu.h"
#include "ultima/ultima4/game/textview.h"

namespace Ultima {
namespace Ultima4 {

MenuController::MenuController(Menu *menu, TextView *view) :
		WaitableController<void *>(nullptr) {
	this->_menu = menu;
	this->_view = view;
}

bool MenuController::keyPressed(int key) {
	bool handled = true;
	bool cursorOn = _view->getCursorEnabled();

	if (cursorOn)
		_view->disableCursor();

	switch (key) {
	case Common::KEYCODE_UP:
		_menu->prev();
		break;
	case Common::KEYCODE_DOWN:
		_menu->next();
		break;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_RETURN: {
		MenuEvent::Type action = MenuEvent::ACTIVATE;

		if (key == Common::KEYCODE_LEFT)
			action = MenuEvent::DECREMENT;
		else if (key == Common::KEYCODE_RIGHT)
			action = MenuEvent::INCREMENT;
		_menu->activateItem(-1, action);
	}
	break;
	default:
		handled = _menu->activateItemByShortcut(key, MenuEvent::ACTIVATE);
	}

	_menu->show(_view);

	if (cursorOn)
		_view->enableCursor();
	_view->update();

	if (_menu->getClosed())
		doneWaiting();

	return handled;
}

} // End of namespace Ultima4
} // End of namespace Ultima
