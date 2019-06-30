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

#include "xeen/dialogs/dialogs_exchange.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

void ExchangeDialog::show(XeenEngine *vm, Character *&c, int &charIndex) {
	ExchangeDialog *dlg = new ExchangeDialog(vm);
	dlg->execute(c, charIndex);
	delete dlg;
}

void ExchangeDialog::execute(Character *&c, int &charIndex) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	loadButtons();

	Window &w = windows[31];
	w.open();
	w.writeString(Res.EXCHANGE_WITH_WHOM);
	_iconSprites.draw(w, 0, Common::Point(225, 120));
	w.update();

	while (!_vm->shouldExit()) {
		events.pollEventsAndWait();
		checkEvents(_vm);

		if (_buttonValue >= Common::KEYCODE_F1 && _buttonValue <= Common::KEYCODE_F6) {
			_buttonValue -= Common::KEYCODE_F1;
			if (_buttonValue < (int)party._activeParty.size()) {
				SWAP(party._activeParty[charIndex], party._activeParty[_buttonValue]);

				charIndex = _buttonValue;
				c = &party._activeParty[charIndex];
				break;
			}
		} else if (_buttonValue == Common::KEYCODE_ESCAPE) {
			break;
		}
	}

	w.close();
	intf.drawParty(true);
	intf.highlightChar(charIndex);
}

void ExchangeDialog::loadButtons() {
	_iconSprites.load("esc.icn");
	addButton(Common::Rect(225, 120, 249, 245), Common::KEYCODE_ESCAPE, &_iconSprites);
	addButton(Common::Rect(16, 16, 48, 48), Common::KEYCODE_1);
	addButton(Common::Rect(117, 16, 149, 48), Common::KEYCODE_2);
	addButton(Common::Rect(16, 59, 48, 91), Common::KEYCODE_3);
	addButton(Common::Rect(117, 59, 149, 91), Common::KEYCODE_4);
}

} // End of namespace Xeen
