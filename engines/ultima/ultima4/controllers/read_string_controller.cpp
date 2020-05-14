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

#include "ultima/ultima4/controllers/read_string_controller.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/gfx/screen.h"

namespace Ultima {
namespace Ultima4 {

ReadStringController::ReadStringController(int maxlen, int screenX, int screenY,
		const Common::String &accepted_chars) : WaitableController<Common::String>("") {
	_maxLen = maxlen;
	_screenX = screenX;
	_screenY = screenY;
	_view = nullptr;
	_accepted = accepted_chars;
}

ReadStringController::ReadStringController(int maxlen, TextView *view,
		const Common::String &accepted_chars) : WaitableController<Common::String>("") {
	_maxLen = maxlen;
	_screenX = view->getCursorX();
	_screenY = view->getCursorY();
	_view = view;
	_accepted = accepted_chars;
}

bool ReadStringController::keyPressed(int key) {
	int valid = true, len = _value.size();
	size_t pos = Common::String::npos;

	if (key < 0x80)
		pos = _accepted.findFirstOf(key);

	if (pos != Common::String::npos) {
		if (key == Common::KEYCODE_BACKSPACE) {
			if (len > 0) {
				/* remove the last character */
				_value.erase(len - 1, 1);

				if (_view) {
					_view->textAt(_screenX + len - 1, _screenY, " ");
					_view->setCursorPos(_screenX + len - 1, _screenY, true);
				} else {
					g_screen->screenHideCursor();
					g_screen->screenTextAt(_screenX + len - 1, _screenY, " ");
					g_screen->screenSetCursorPos(_screenX + len - 1, _screenY);
					g_screen->screenShowCursor();
				}
			}
		} else if (key == '\n' || key == '\r') {
			doneWaiting();
		} else if (len < _maxLen) {
			/* add a character to the end */
			_value += key;

			if (_view) {
				_view->textAt(_screenX + len, _screenY, "%c", key);
			} else {
				g_screen->screenHideCursor();
				g_screen->screenTextAt(_screenX + len, _screenY, "%c", key);
				g_screen->screenSetCursorPos(_screenX + len + 1, _screenY);
				g_context->_col = len + 1;
				g_screen->screenShowCursor();
			}
		}
	} else {
		valid = false;
	}

	return valid || KeyHandler::defaultHandler(key, nullptr);
}

Common::String ReadStringController::get(int maxlen, int screenX, int screenY, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	ReadStringController ctrl(maxlen, screenX, screenY);
	eh->pushController(&ctrl);
	return ctrl.waitFor();
}

Common::String ReadStringController::get(int maxlen, TextView *view, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	ReadStringController ctrl(maxlen, view);
	eh->pushController(&ctrl);
	return ctrl.waitFor();
}

} // End of namespace Ultima4
} // End of namespace Ultima
