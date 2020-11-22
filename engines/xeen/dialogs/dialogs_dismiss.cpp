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

#include "xeen/dialogs/dialogs_dismiss.h"
#include "xeen/party.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

void Dismiss::show(XeenEngine *vm) {
	Dismiss *dlg = new Dismiss(vm);
	dlg->execute();
	delete dlg;
}

void Dismiss::execute() {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	loadButtons();

	Window &w = windows[31];
	w.open();

	bool breakFlag = false;
	while (!_vm->shouldExit() && !breakFlag) {
		do {
			events.updateGameCounter();
			intf.draw3d(false, false);

			w.frame();
			w.fill();
			w.writeString(Res.DISMISS_WHOM);
			_iconSprites.draw(w, 0, Common::Point(225, 120));
			w.update();

			do {
				events.pollEventsAndWait();
				checkEvents(_vm);
			} while (!_vm->shouldExit() && !_buttonValue && events.timeElapsed() < 2);
		} while (!_vm->shouldExit() && !_buttonValue);

		if (_buttonValue >= Common::KEYCODE_F1 && _buttonValue <= Common::KEYCODE_F6) {
			_buttonValue -= Common::KEYCODE_F1;

			if (_buttonValue < (int)party._activeParty.size()) {
				if (party._activeParty.size() == 1) {
					w.close();
					ErrorScroll::show(_vm, Res.CANT_DISMISS_LAST_CHAR, WT_NONFREEZED_WAIT);
					w.open();
				} else if (party._activeParty[_buttonValue]._weapons.hasElderWeapon()) {
					w.close();
					ErrorScroll::show(_vm, Res.DELETE_CHAR_WITH_ELDER_WEAPON, WT_NONFREEZED_WAIT);
					w.open();
				} else {
					// Remove the character from the party
					party._activeParty.remove_at(_buttonValue);
					breakFlag = true;
				}
				break;
			}
		} else if (_buttonValue == Common::KEYCODE_ESCAPE) {
			breakFlag = true;
		}
	}

	w.close();
	intf.drawParty(true);
}

void Dismiss::loadButtons() {
	_iconSprites.load("esc.icn");
	addButton(Common::Rect(225, 120, 249, 140), Common::KEYCODE_ESCAPE, &_iconSprites);
	addButton(Common::Rect(16, 16, 48, 48), Common::KEYCODE_1);
	addButton(Common::Rect(117, 16, 149, 48), Common::KEYCODE_2);
	addButton(Common::Rect(16, 59, 48, 91), Common::KEYCODE_3);
	addButton(Common::Rect(117, 59, 149, 91), Common::KEYCODE_4);
}

} // End of namespace Xeen
