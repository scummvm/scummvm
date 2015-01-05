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

#include "common/scummsys.h"
#include "xeen/dialogs_error.h"
#include "xeen/events.h"
#include "xeen/xeen.h"

namespace Xeen {

void ErrorScroll::show(XeenEngine *vm, const Common::String &msg, ErrorWaitType waitType) {
	ErrorScroll *dlg = new ErrorScroll(vm);
	dlg->execute(msg, waitType);
	delete dlg;
}

void ErrorScroll::execute(const Common::String &msg, ErrorWaitType waitType) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Window &w = screen._windows[6];

	Common::String s = Common::String::format("\x03c\v010\t000%s", msg);
	w.open();
	w.writeString(s);
	w.update();

	switch (waitType) {
	case WT_FREEZE_WAIT:
		while (!_vm->shouldQuit() && !events.isKeyPending())
			events.pollEventsAndWait();

		events.clearEvents();
		break;
	case WT_3:
		if (w._enabled || _vm->_mode == MODE_17) {
			warning("TODO: sub_26D8F");
			break;
		}
		// Deliberate fall-through
	case WT_NONFREEZED_WAIT:
		do {
			events.updateGameCounter();
			_vm->_interface->draw3d(true);

			events.wait(1, true);
			if (checkEvents(_vm))
				break;
		} while (!_vm->shouldQuit() && !_buttonValue);
		break;
	case WT_2:
		warning("TODO: sub_26D8F");
		break;
	default:
		break;
	}
}

} // End of namespace Xeen
