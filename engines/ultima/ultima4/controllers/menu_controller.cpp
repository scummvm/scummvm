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
#include "ultima/ultima4/views/menu.h"
#include "ultima/ultima4/views/textview.h"

namespace Ultima {
namespace Ultima4 {

MenuController::MenuController(Menu *menu, TextView *view) :
		WaitableController<void *>(nullptr) {
	this->_menu = menu;
	this->_view = view;
}

void MenuController::setActive() {
	MetaEngine::setKeybindingMode(KBMODE_MENU);
}

void MenuController::keybinder(KeybindingAction action) {
	bool cursorOn = _view->getCursorEnabled();

	if (cursorOn)
		_view->disableCursor();

	switch (action) {
	case KEYBIND_UP:
		_menu->prev();
		break;
	case KEYBIND_DOWN:
		_menu->next();
		break;
	case KEYBIND_LEFT:
	case KEYBIND_RIGHT:
	case KEYBIND_INTERACT: {
		MenuEvent::Type menuAction = MenuEvent::ACTIVATE;

		if (action == KEYBIND_LEFT)
			menuAction = MenuEvent::DECREMENT;
		else if (action == KEYBIND_RIGHT)
			menuAction = MenuEvent::INCREMENT;
		_menu->activateItem(-1, menuAction);
		break;
	}
	default:
		break;
	}

	_menu->show(_view);

	if (cursorOn)
		_view->enableCursor();
	_view->update();

	if (_menu->getClosed())
		doneWaiting();
}

bool MenuController::keyPressed(int key) {
	bool handled = true;
	bool cursorOn = _view->getCursorEnabled();

	if (cursorOn)
		_view->disableCursor();

	handled = _menu->activateItemByShortcut(key, MenuEvent::ACTIVATE);

	_menu->show(_view);

	if (cursorOn)
		_view->enableCursor();
	_view->update();

	if (_menu->getClosed())
		doneWaiting();

	return handled;
}

bool MenuController::mousePressed(const Common::Point &mousePos) {
	bool cursorOn = _view->getCursorEnabled();

	if (cursorOn)
		_view->disableCursor();

	_menu->activateItemAtPos(_view, mousePos);

	_menu->show(_view);

	if (cursorOn)
		_view->enableCursor();
	_view->update();

	if (_menu->getClosed())
		doneWaiting();

	return true;
}

} // End of namespace Ultima4
} // End of namespace Ultima
