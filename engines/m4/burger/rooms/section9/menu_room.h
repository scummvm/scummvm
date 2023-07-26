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

enum ButtonState { BTNSTATE_0 = 0, BTNSTATE_1 = 1, BTNSTATE_2 = 2, BTNSTATE_3 = 3 };

struct MenuButtonDef {
	int32 _x1;
	int32 _y1;
	int32 _x2;
	int32 _y2;
	int32 _frame0;
	int32 _frame1;
	int32 _frame2;
	int32 _frame3;
	ButtonState _state;
	int32 _val10;
};

struct MenuButton : public MenuButtonDef {
	machine *_machine = nullptr;

	MenuButton() : MenuButtonDef() {}
	MenuButton(const MenuButtonDef &def) : MenuButtonDef(def) {}
};

class MenuRoom : public Rooms::Room {
protected:
	Common::Array<MenuButton> _buttons;
	const char *_menuName;
	const char *_clickName;
	int _val2 = -1;
	int _val3 = -1;
	int _val4 = -1;

	/**
	 * Set the display buttons
	 */
	void setButtons(const MenuButtonDef *btns, int count);

	/**
	 * Draws a button
	 */
	void drawButton(int index);

	/**
	 * Changes a button's state
	 */
	void setButtonState(int index, ButtonState newState);

	/**
	 * Returns the index of the button currently under the mouse, if any
	 */
	int32 is_mouse_over_a_button() const;

public:
	MenuRoom(int roomNum, const char *menuName, const char *clickName) :
		Rooms::Room(roomNum), _menuName(menuName), _clickName(clickName) {}
	virtual ~MenuRoom() {}
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
