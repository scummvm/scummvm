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
#include "xeen/resources.h"

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


/**
* Draws the scroll in the background
*/
void Dialog::doScroll(bool drawFlag, bool doFade) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;

	if (_vm->getGameID() != GType_Clouds) {
		if (doFade) {
			_vm->_screen->fadeIn(2);
		}
		return;
	}

	const int SCROLL_L[8] = { 29, 23, 15, 251, 245, 233, 207, 185 };
	const int SCROLL_R[8] = { 165, 171, 198, 218, 228, 245, 264, 281 };

	saveButtons();
	clearButtons();
	screen.saveBackground();

	// Load hand vga files
	SpriteResource *hand[16];
	for (int i = 0; i < 16; ++i) {
		Common::String name = Common::String::format("hand%02u.vga", i);
		hand[i] = new SpriteResource(name);
	}

	// Load marb vga files
	SpriteResource *marb[5];
	for (int i = 1; i < 5; ++i) {
		Common::String name = Common::String::format("marb%02u.vga");
		marb[i] = new SpriteResource(name);
	}

	if (drawFlag) {
		for (int i = 22; i > 0; --i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i > 0 && i <= 14) {
				hand[i - 1]->draw(screen, 0);
			}
			else {
				// TODO: Check '800h'.. horizontal reverse maybe?
				hand[14]->draw(screen, 0, Common::Point(SCROLL_L[i - 14], 0));
				marb[15]->draw(screen, 0, Common::Point(SCROLL_R[i - 14], 0));
			}

			if (i <= 20) {
				marb[i / 5]->draw(screen, i % 5);
			}

			while (!_vm->shouldQuit() && _vm->_events->timeElapsed() == 0)
				_vm->_events->pollEventsAndWait();

			screen._windows[0].update();
			if (i == 0 && doFade)
				screen.fadeIn(2);
		}
	} else {
		for (int i = 0; i < 22 && !events.isKeyMousePressed(); ++i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i < 14) {
				hand[i]->draw(screen, 0);
			} else {
				// TODO: Check '800h'.. horizontal reverse maybe?
				hand[14]->draw(screen, 0, Common::Point(SCROLL_L[i - 7], 0));
				marb[15]->draw(screen, 0, Common::Point(SCROLL_R[i - 7], 0));
			}

			if (i < 20) {
				marb[i / 5]->draw(screen, i % 5);
			}

			while (!_vm->shouldQuit() && _vm->_events->timeElapsed() == 0)
				_vm->_events->pollEventsAndWait();

			screen._windows[0].update();
			if (i == 0 && doFade)
				screen.fadeIn(2);
		}
	}

	if (drawFlag) {
		hand[0]->draw(screen, 0);
		marb[0]->draw(screen, 0);
	}
	else {
		screen.restoreBackground();
	}

	screen._windows[0].update();
	restoreButtons();

	// Free resources
	for (int i = 1; i < 5; ++i)
		delete marb[i];
	for (int i = 0; i < 16; ++i)
		delete hand[i];
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
