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
#include "xeen/dialogs.h"
#include "xeen/resdata.h"

namespace Xeen {

/**
 * Saves the current list of buttons
 */
void Dialog::saveButtons() {
	_savedButtons.push(_buttons);
}

/*
 * Clears the current list of defined buttons
 */
void Dialog::clearButtons() {
	_buttons.clear();
}

void Dialog::restoreButtons() {
	_buttons = _savedButtons.pop();
}

void Dialog::addButton(const Common::Rect &bounds, char c, SpriteResource *sprites, bool draw = true) {
	_buttons.push_back(DialogButton(bounds, c, sprites, draw));
}

void Dialog::checkEvents() {
	EventsManager &events = *_vm->_events;
	events.pollEventsAndWait();

	if (events._leftButton) {
		// Check whether any button is selected
		events.debounceMouse();
		Common::Point pt = events._mousePos;

		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._bounds.contains(pt)) {
				_key = _buttons[i]._c;
				return;
			}
		}
	} else if (events.isKeyPending()) {
		Common::KeyState keyState;
		events.getKey(keyState);
		if (keyState.ascii >= 32 && keyState.ascii <= 127) {
			_key = keyState.ascii;
			return;
		}
	}
}

/*------------------------------------------------------------------------*/

void SettingsBaseDialog::showContents(SpriteResource &title1, bool waitFlag) {
	checkEvents();
}

/*------------------------------------------------------------------------*/

void CreditsScreen::show(XeenEngine *vm) {
	CreditsScreen *dlg = new CreditsScreen(vm);
	dlg->execute();
	delete dlg;
}

void CreditsScreen::execute() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	
	// Handle drawing the credits screen
	doScroll(true, false);
	screen._windows[28].close();

	screen.loadBackground("marb.raw");
	screen._windows[0].writeString(CREDITS);
	doScroll(false, false);
	
	events.setCursor(0);
	screen._windows[0].update();
	clearButtons();

	// Wait for keypress
	while (!events.isKeyMousePressed())
		events.pollEventsAndWait();

	doScroll(true, false);
}

} // End of namespace Xeen
