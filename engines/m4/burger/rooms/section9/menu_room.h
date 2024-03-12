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

#ifndef M4_BURGER_ROOMS_SECTION9_MENU_ROOM_H
#define M4_BURGER_ROOMS_SECTION9_MENU_ROOM_H

#include "common/array.h"
#include "m4/wscript/ws_machine.h"
#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

constexpr int NO_BUTTONS_HILITED = -1;

enum ButtonState { BTNSTATE_DISABLED = 0, BTNSTATE_ENABLED = 1, BTNSTATE_HIGHLIGHTED = 2, BTNSTATE_PRESSED = 3 };

struct MenuButtonDef {
	int32 _x1;
	int32 _y1;
	int32 _x2;
	int32 _y2;
	int32 _frame_disabled;
	int32 _frame_enabled;
	int32 _frame_highlighted;
	int32 _frame_pressed;
	ButtonState _state;
	int32 _trigger;
};

struct MenuButton : public MenuButtonDef {
	machine *_machine = nullptr;

	MenuButton() : MenuButtonDef() {}
	MenuButton(const MenuButtonDef &def) : MenuButtonDef(def) {}
};

/**
 * Base class used for the menu rooms 901 and 903
 */
class MenuRoom : public Rooms::Room {
private:
	const char *_menuName;
	const char *_clickName;
	int _buttonNum = -1;
	int _highlightedButton = -1;
	int _activeButton = -1;
	bool _flag = false;

	/**
	 * Handles button processing once a frame
	 */
	void buttonsFrame();

	/**
	 * Returns the index of the button currently under the mouse, if any
	 */
	int32 is_mouse_over_a_button() const;

protected:
	Common::Array<MenuButton> _buttons;

	/**
	 * Set the display buttons
	 */
	void setButtons(const MenuButtonDef *btns, int count);

	/**
	 * Draws a button
	 */
	void drawButton(int index);

	/**
	 * Draws all the buttons
	 */
	void drawButtons();

	/**
	 * Changes a button's state
	 */
	void setButtonState(int index, ButtonState newState);

	/**
	 * Resets the selected button
	 */
	void resetSelectedButton() {
		_activeButton = -1;
	}
public:
	MenuRoom(const char *menuName, const char *clickName) : Rooms::Room(),
		_menuName(menuName), _clickName(clickName) {}
	virtual ~MenuRoom() {}

	void init() override;
	void daemon() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
