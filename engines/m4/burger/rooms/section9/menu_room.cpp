/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/burger/rooms/section9/menu_room.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_series.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void MenuRoom::init() {
	_buttonNum = -1;
	_highlightedButton = -1;
	_activeButton = -1;
	_flag = false;
}


void MenuRoom::daemon() {
	if (_G(kernel).trigger == kCALLED_EACH_LOOP) {
		if (player_commands_allowed())
			buttonsFrame();

	} else {
		_G(kernel).continue_handling_trigger = true;
	}
}

void MenuRoom::setButtons(const MenuButtonDef *btns, int count) {
	_buttons.resize(count);
	for (int i = 0; i < count; ++i)
		_buttons[i] = btns[i];
}

void MenuRoom::drawButton(int index) {
	if (index < 0 || index >= (int)_buttons.size())
		error_show(FL, 'Burg', "draw_button which?");

	MenuButton &btn = _buttons[index];
	assert(btn._state >= BTNSTATE_DISABLED && btn._state <= BTNSTATE_PRESSED);
	int frame[4] = {
		btn._frame_disabled, btn._frame_enabled,
		btn._frame_highlighted, btn._frame_pressed
	};
	btn._machine = series_show(_menuName, 0, 0, -1, -1, frame[btn._state], 100, btn._x1, btn._y1);
}

void MenuRoom::drawButtons() {
	for (uint i = 0; i < _buttons.size(); ++i)
		drawButton(i);
}

void MenuRoom::setButtonState(int index, ButtonState newState) {
	if (index >= 0 && index < (int)_buttons.size()) {
		MenuButton &btn = _buttons[index];
		if (btn._state != BTNSTATE_DISABLED && newState != btn._state) {
			terminateMachineAndNull(btn._machine);
			btn._state = newState;
			drawButton(index);
		}
	} else if (index != -1) {
		// LOL: A fine example of original authors sense of humour
		term_message("ooga booga %d", index);
	}
}

void MenuRoom::buttonsFrame() {
	bool newFlag = false;

	_highlightedButton = is_mouse_over_a_button();

	if (_buttonNum == -1) {
		if (_highlightedButton == -1) {
			setButtonState(_activeButton, BTNSTATE_ENABLED);
		} else if (_highlightedButton != _activeButton) {
			setButtonState(_activeButton, BTNSTATE_ENABLED);
			setButtonState(_highlightedButton, BTNSTATE_HIGHLIGHTED);
		}

		_activeButton = _highlightedButton;
	} else {
		setButtonState(_buttonNum, _buttonNum == _highlightedButton ? BTNSTATE_PRESSED : BTNSTATE_ENABLED);
	}

	if (_G(MouseState).ButtonState) {
		_flag = true;
		if (_buttonNum == -1)
			_buttonNum = _highlightedButton;

		if (_buttonNum != -1) {
			setButtonState(_buttonNum, _buttonNum == _highlightedButton ? BTNSTATE_PRESSED : BTNSTATE_ENABLED);
		}
	} else if (_flag) {
		_flag = false;
		newFlag = true;
	}

	if (newFlag) {
		_G(events).clearMouseStateEvent();

		if (_highlightedButton == _buttonNum && _buttonNum != -1) {
			term_message("Button pressed: %d", _highlightedButton);

			const MenuButton &btn = _buttons[_highlightedButton];
			if (btn._state != BTNSTATE_DISABLED) {
				digi_play(_clickName, 2, 255, -1);
				kernel_trigger_dispatch_now(btn._trigger);
				setButtonState(_buttonNum, BTNSTATE_HIGHLIGHTED);
			}
		}

		_buttonNum = -1;
	}
}

int32 MenuRoom::is_mouse_over_a_button() const {
	for (uint idx = 0; idx < _buttons.size(); ++idx) {
		const MenuButton &btn = _buttons[idx];

		if (_G(MouseState).CursorColumn >= btn._x1 && _G(MouseState).CursorColumn <= btn._x2 &&
				_G(MouseState).CursorRow >= btn._y1 && _G(MouseState).CursorRow <= btn._y2) {
			return idx;
		}
	}

	// Mouse not over any button
	return NO_BUTTONS_HILITED;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
