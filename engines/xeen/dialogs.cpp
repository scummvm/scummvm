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
#include "xeen/events.h"
#include "xeen/resources.h"
#include "xeen/screen.h"
#include "xeen/xeen.h"

namespace Xeen {

/**
 * Saves the current list of buttons
 */
void ButtonContainer::saveButtons() {
	_savedButtons.push(_buttons);
	clearButtons();
}

/*
 * Clears the current list of defined buttons
 */
void ButtonContainer::clearButtons() {
	_buttons.clear();
}

void ButtonContainer::restoreButtons() {
	_buttons = _savedButtons.pop();
}

void ButtonContainer::addButton(const Common::Rect &bounds, int val, 
		SpriteResource *sprites) {
	_buttons.push_back(UIButton(bounds, val, sprites, true));
}

void ButtonContainer::addButton(const Common::Rect &bounds, int val) {
	_buttons.push_back(UIButton(bounds, val, nullptr, false));
}

void ButtonContainer::addPartyButtons(XeenEngine *vm) {
	for (uint idx = 0; idx < MAX_ACTIVE_PARTY; ++idx) {
		addButton(Common::Rect(CHAR_FACES_X[idx], 150, CHAR_FACES_X[idx] + 32, 182),
			Common::KEYCODE_F1 + idx);
	}
}

bool ButtonContainer::checkEvents(XeenEngine *vm) {
	EventsManager &events = *vm->_events;
	_buttonValue = 0;

	if (events._leftButton) {
		// Check whether any button is selected
		Common::Point pt = events._mousePos;

		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._bounds.contains(pt)) {
				events.debounceMouse();

				_buttonValue = _buttons[i]._value;
				return true;
			}
		}
	} else if (events.isKeyPending()) {
		Common::KeyState keyState;
		events.getKey(keyState);
		
		_buttonValue = keyState.keycode;
		if (_buttonValue == Common::KEYCODE_KP8)
			_buttonValue = Common::KEYCODE_UP;
		else if (_buttonValue == Common::KEYCODE_KP2)
			_buttonValue = Common::KEYCODE_DOWN;
		else if (_buttonValue == Common::KEYCODE_KP_ENTER)
			_buttonValue = Common::KEYCODE_RETURN;

		_buttonValue |= (keyState.flags << 8);
		if (_buttonValue)
			return true;
	}

	return false;
}


/**
* Draws the scroll in the background
*/
void ButtonContainer::doScroll(XeenEngine *vm, bool drawFlag, bool doFade) {
	Screen &screen = *vm->_screen;
	EventsManager &events = *vm->_events;

	if (vm->getGameID() != GType_Clouds) {
		if (doFade) {
			screen.fadeIn(2);
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
		Common::String name = Common::String::format("hand%02d.vga", i);
		hand[i] = new SpriteResource(name);
	}

	// Load marb vga files
	SpriteResource *marb[5];
	for (int i = 1; i < 5; ++i) {
		Common::String name = Common::String::format("marb%02d.vga", i);
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

			while (!vm->shouldQuit() && events.timeElapsed() == 0)
				events.pollEventsAndWait();

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

			while (!vm->shouldQuit() && events.timeElapsed() == 0)
				events.pollEventsAndWait();

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

/**
 * Draws the buttons onto the passed surface
 */
void ButtonContainer::drawButtons(XSurface *surface) {
	for (uint btnIndex = 0; btnIndex < _buttons.size(); ++btnIndex) {
		UIButton &btn = _buttons[btnIndex];
		if (btn._draw) {
			btn._sprites->draw(*surface, btnIndex * 2,
				Common::Point(btn._bounds.left, btn._bounds.top));
		}
	}
}

/*------------------------------------------------------------------------*/

void SettingsBaseDialog::showContents(SpriteResource &title1, bool waitFlag) {
	_vm->_events->pollEventsAndWait();
	checkEvents(_vm);
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
	doScroll(_vm, true, false);
	screen._windows[GAME_WINDOW].close();

	screen.loadBackground("marb.raw");
	screen._windows[0].writeString(CREDITS);
	doScroll(_vm, false, false);
	
	events.setCursor(0);
	screen._windows[0].update();
	clearButtons();

	// Wait for keypress
	while (!events.isKeyMousePressed())
		events.pollEventsAndWait();

	doScroll(_vm, true, false);
}

/*------------------------------------------------------------------------*/

void PleaseWait::show(XeenEngine *vm) {
	if (vm->_mode != MODE_0) {
		Window &w = vm->_screen->_windows[9];
		w.open();
		w.writeString(PLEASE_WAIT);
		w.update();
	}
}

} // End of namespace Xeen
