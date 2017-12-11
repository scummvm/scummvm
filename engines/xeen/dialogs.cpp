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
		addButton(Common::Rect(Res.CHAR_FACES_X[idx], 150, Res.CHAR_FACES_X[idx] + 32, 182),
			Common::KEYCODE_F1 + idx);
	}
}

bool ButtonContainer::checkEvents(XeenEngine *vm) {
	EventsManager &events = *vm->_events;
	Windows &windows = *_vm->_windows;
	_buttonValue = 0;

	if (events._leftButton) {
		// Check whether any button is selected
		Common::Point pt = events._mousePos;

		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._bounds.contains(pt)) {
				events.debounceMouse();

				_buttonValue = _buttons[i]._value;
				break;
			}
		}

		if (!_buttonValue && Common::Rect(8, 8, 224, 135).contains(pt)) {
			_buttonValue = 1;
			return true;
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

		_buttonValue |= (keyState.flags & ~Common::KBD_CAPS) << 16;
	}

	if (_buttonValue) {
		// Check for a button matching the selected _buttonValue
		Window &win = windows[39];
		for (uint btnIndex = 0; btnIndex < _buttons.size(); ++btnIndex) {
			UIButton &btn = _buttons[btnIndex];
			if (btn._draw && btn._value == _buttonValue) {
				// Found the correct button
				// Draw button depressed
				btn._sprites->draw(0, btnIndex * 2 + 1,
					Common::Point(btn._bounds.left, btn._bounds.top));
				win.setBounds(btn._bounds);
				win.update();

				// Slight delay
				events.updateGameCounter();
				events.wait(2);

				// Redraw button in it's original non-depressed form
				btn._sprites->draw(0, btnIndex * 2,
					Common::Point(btn._bounds.left, btn._bounds.top));
				win.setBounds(btn._bounds);
				win.update();
				break;
			}
		}

		return true;
	}

	return false;
}

void ButtonContainer::drawButtons(XSurface *surface) {
	for (uint btnIndex = 0; btnIndex < _buttons.size(); ++btnIndex) {
		UIButton &btn = _buttons[btnIndex];
		if (btn._draw) {
			btn._sprites->draw(*surface, btnIndex * 2,
				Common::Point(btn._bounds.left, btn._bounds.top));
		}
	}
}

bool ButtonContainer::doScroll(bool rollUp, bool fadeIn) {
	if (_vm->_files->_isDarkCc) {
		return Cutscenes::doScroll(rollUp, fadeIn);
	} else {
		saveButtons();
		clearButtons();
		bool result = Cutscenes::doScroll(rollUp, fadeIn);
		restoreButtons();
		return result;
	}
}

void ButtonContainer::loadStrings(const Common::String &name) {
	File f(name);
	_textStrings.clear();
	while (f.pos() < f.size())
		_textStrings.push_back(f.readString());
	f.close();
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
	Windows &windows = *_vm->_windows;
	EventsManager &events = *_vm->_events;

	// Handle drawing the credits screen
	doScroll(true, false);
	windows[GAME_WINDOW].close();

	screen.loadBackground("marb.raw");
	windows[0].writeString(Res.CREDITS);
	doScroll(false, false);

	events.setCursor(0);
	windows[0].update();
	clearButtons();

	// Wait for keypress
	while (!events.isKeyMousePressed())
		events.pollEventsAndWait();

	doScroll(true, false);
}

/*------------------------------------------------------------------------*/

void PleaseWait::show(XeenEngine *vm) {
	Windows &windows = *vm->_windows;
	Window &w = windows[9];

	if (vm->_mode != MODE_0) {
		w.open();
		w.writeString(Res.PLEASE_WAIT);
		w.update();
	}
}

} // End of namespace Xeen
