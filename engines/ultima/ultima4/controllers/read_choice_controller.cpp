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

#include "ultima/ultima4/controllers/read_choice_controller.h"

namespace Ultima {
namespace Ultima4 {


ReadChoiceController::ReadChoiceController(const Common::String &choices) :
		WaitableController<int>(-1) {
	_choices = choices;
}

bool ReadChoiceController::keyPressed(int key) {
	// Common::isUpper() accepts 1-byte characters, yet the modifier keys
	// (ALT, SHIFT, ETC) produce values beyond 255
	if ((key <= 0x7F) && (Common::isUpper(key)))
		key = tolower(key);

	_value = key;

	if (_choices.empty() || _choices.findFirstOf(_value) < _choices.size()) {
		// If the value is printable, display it
		if (!Common::isSpace(key))
			g_screen->screenMessage("%c", toupper(key));
		doneWaiting();
		return true;
	}

	return false;
}

void ReadChoiceController::keybinder(KeybindingAction action) {
	if (action == KEYBIND_ESCAPE && _choices.contains('\x1B')) {
		_value = 27;
		doneWaiting();
	} else {
		WaitableController<int>::keybinder(action);
	}
}


char ReadChoiceController::get(const Common::String &choices, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	ReadChoiceController ctrl(choices);
	eh->pushController(&ctrl);
	return ctrl.waitFor();
}

} // End of namespace Ultima4
} // End of namespace Ultima
