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

#include "ultima/ultima4/controllers/alpha_action_controller.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/gfx/screen.h"

namespace Ultima {
namespace Ultima4 {

bool AlphaActionController::keyPressed(int key) {
	if (Common::isLower(key))
		key = toupper(key);

	if (key >= 'A' && key <= toupper(_lastValidLetter)) {
		_value = key - 'A';
		doneWaiting();
	} else {
		g_screen->screenMessage("\n%s", _prompt.c_str());
		g_screen->update();
		return KeyHandler::defaultHandler(key, nullptr);
	}

	return true;
}

void AlphaActionController::keybinder(KeybindingAction action) {
	if (action == KEYBIND_ESCAPE) {
		g_screen->screenMessage("\n");
		_value = -1;
		doneWaiting();
	}
}

int AlphaActionController::get(char lastValidLetter, const Common::String &prompt, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	AlphaActionController ctrl(lastValidLetter, prompt);
	eh->pushController(&ctrl);
	return ctrl.waitFor();
}

} // End of namespace Ultima4
} // End of namespace Ultima
