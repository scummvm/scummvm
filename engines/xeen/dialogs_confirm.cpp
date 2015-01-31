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

#include "xeen/dialogs_confirm.h"
#include "xeen/xeen.h"

namespace Xeen {

bool ConfirmDialog::show(XeenEngine *vm, const Common::String &msg, int v2) {
	ConfirmDialog *dlg = new ConfirmDialog(vm);
	bool result = dlg->execute(msg, v2);
	delete dlg;

	return result;
}

bool ConfirmDialog::execute(const Common::String &msg, int v2) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	SpriteResource confirmSprites;
	confirmSprites.load("confirm.icn");

	addButton(Common::Rect(129, 112, 153, 132), Common::KEYCODE_y, &confirmSprites);
	addButton(Common::Rect(185, 112, 209, 132), Common::KEYCODE_n, &confirmSprites);

	Window &w = screen._windows[v2 ? 21 : 22];
	w.open();

	if (v2) {
		confirmSprites.draw(screen._windows[21], 0, Common::Point(129, 112));
		confirmSprites.draw(screen._windows[21], 2, Common::Point(185, 112));

		_buttons[0]._bounds.moveTo(129, 112);
		_buttons[1]._bounds.moveTo(185, 112);
	} else if (v2 & 0x80) {
		clearButtons();
	} else {
		confirmSprites.draw(screen._windows[22], 0, Common::Point(120, 133));
		confirmSprites.draw(screen._windows[22], 2, Common::Point(176, 133));

		_buttons[0]._bounds.moveTo(120, 133);
		_buttons[1]._bounds.moveTo(176, 133);
	}

	w.writeString(msg);
	w.update();

	bool result = false;
	while (!_vm->shouldQuit()) {
		while (!events.isKeyMousePressed())
			events.pollEventsAndWait();

		if ((v2 & 0x80) || _buttonValue == Common::KEYCODE_ESCAPE ||
			_buttonValue == Common::KEYCODE_y)
			break;
		else if (_buttonValue == Common::KEYCODE_y) {
			result = true;
			break;
		}
	}

	w.close();
	return result;
}

} // End of namespace Xeen
