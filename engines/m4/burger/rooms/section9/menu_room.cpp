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

void MenuRoom::setButtons(const MenuButtonDef *btns, int count) {
	_buttons.resize(count);
	for (int i = 0; i < count; ++i)
		_buttons[i] = btns[i];
}

void MenuRoom::drawButton(int index) {
	if (index < 0 || index >= (int)_buttons.size())
		error_show(FL, 'Burg', "draw_button which?");

	MenuButton &btn = _buttons[index];
	assert(btn._state >= BTNSTATE_0 && btn._state <= BTNSTATE_3);
	int frame[4] = { btn._frame0, btn._frame1, btn._frame2, btn._frame3 };
	btn._machine = series_show_(_menuName, 0, 0, -1, -1, frame[btn._state], 100, btn._x1, btn._y1);
}

void MenuRoom::setButtonState(int index, ButtonState newState) {
	if (index >= 0 && index < (int)_buttons.size()) {
		MenuButton &btn = _buttons[index];
		if (btn._state != BTNSTATE_0 && newState != btn._state) {
			TerminateMachineAndNull(btn._machine);
			btn._state = newState;
			drawButton(index);
		}
	} else if (index != -1) {
		// LOL: A fine example of original authors sense of humour
		term_message("ooga booga %d", index);
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
