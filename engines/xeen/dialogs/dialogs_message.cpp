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
#include "xeen/dialogs/dialogs_message.h"
#include "xeen/events.h"
#include "xeen/xeen.h"

namespace Xeen {

void MessageDialog::show(XeenEngine *vm, const Common::String &msg, MessageWaitType waitType) {
	MessageDialog *dlg = new MessageDialog(vm);
	dlg->execute(msg, waitType);
	delete dlg;
}

void MessageDialog::execute(const Common::String &msg, MessageWaitType waitType) {
	EventsManager &events = *_vm->_events;
	Windows &windows = *_vm->_windows;
	Window &w = windows[6];

	w.open();
	w.writeString(msg);
	w.update();

	switch (waitType) {
	case WT_FREEZE_WAIT:
		while (!_vm->shouldExit() && !events.isKeyMousePressed())
			events.pollEventsAndWait();

		events.clearEvents();
		break;

	case WT_ANIMATED_WAIT:
		if (g_vm->_locations->isActive() && (windows[11]._enabled || _vm->_mode == MODE_INTERACTIVE7)) {
			g_vm->_locations->wait();
			break;
		}
		// fall through

	case WT_NONFREEZED_WAIT:
		do {
			events.updateGameCounter();
			_vm->_interface->draw3d(true);

			events.wait(1);
			if (checkEvents(_vm))
				break;
		} while (!_vm->shouldExit() && !_buttonValue);
		break;

	case WT_LOC_WAIT:
		g_vm->_locations->wait();
		break;

	default:
		break;
	}

	w.close();
}

/*------------------------------------------------------------------------*/

void ErrorScroll::show(XeenEngine *vm, const Common::String &msg, MessageWaitType waitType) {
	Common::String s = Common::String::format("\x3""c\v010\t000%s", msg.c_str());
	MessageDialog::show(vm, s, waitType);
}

/*------------------------------------------------------------------------*/

void CantCast::show(XeenEngine *vm, int spellId, int componentNum) {
	CantCast *dlg = new CantCast(vm);
	dlg->execute(spellId, componentNum);
	delete dlg;
}

void CantCast::execute(int spellId, int componentNum) {
	EventsManager &events = *_vm->_events;
	Sound &sound = *_vm->_sound;
	Spells &spells = *_vm->_spells;
	Windows &windows = *_vm->_windows;
	Window &w = windows[6];
	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_FF;

	sound.playFX(21);
	w.open();
	w.writeString(Common::String::format(Res.NOT_ENOUGH_TO_CAST,
		Res.SPELL_CAST_COMPONENTS[componentNum - 1],
		spells._spellNames[spellId].c_str()
	));
	w.update();

	do {
		events.pollEventsAndWait();
	} while (!_vm->shouldExit() && !events.isKeyMousePressed());
	events.clearEvents();

	w.close();
	_vm->_mode = oldMode;
}

} // End of namespace Xeen
