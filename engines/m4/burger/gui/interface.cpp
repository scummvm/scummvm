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

#include "m4/burger/gui/interface.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_series.h"
#include "m4/burger/burger_globals.h"

namespace M4 {
namespace Burger {
namespace GUI {

void Interface::init(int arrow, int wait, int look, int grab, int use) {
	_arrow = arrow;
	_wait = wait;
	_look = look;
	_grab = grab;
	_use = use;

	_sprite = series_load("999intr", 22, nullptr);
	if (_sprite != 22)
		error_show(FL, 'SLF!');


	mouse_set_sprite(wait);

	_gameInterfaceBuff = new GrBuff(_x2 - _x1, _y2 - _y1);
	setup();

	mouse_set_sprite(arrow);
}

Interface::~Interface() {
	delete _gameInterfaceBuff;
	delete _interfaceBox;
	delete _inventory;
	delete _textField;
	delete _btnTake;
	delete _btnManipulate;
	delete _btnHandle;
	delete _btnAbductFail;
	delete _btnMenu;
	delete _btnScrollLeft;
	delete _btnScrollRight;
}

void Interface::show() {
	error("TODO: Interface::show");
}

void Interface::setup() {
	_interfaceBox = new InterfaceBox(RectClass(0, 0, SCREEN_WIDTH - 1, 105));
	_inventory = new GUI::Inventory(RectClass(188, 22, 539, 97), _sprite, 9, 1, 39, 75, 3);
	_textField = new TextField(200, 1, 450, 21);
	_btnTake = new ButtonClass(RectClass(60, 35, 92, 66), "take", 4, 3, 3, 4, 5);
	_btnManipulate = new ButtonClass(RectClass(105, 35, 137, 66), "manipulate", 7, 6, 6, 7, 8);
	_btnHandle = new ButtonClass(RectClass(15, 35, 47, 66), "handle", 5, 0, 0, 1, 2);

	_interfaceBox->add(_btnTake);
	_interfaceBox->add(_btnManipulate);
	_interfaceBox->add(_btnHandle);


	if (_G(executing) == WHOLE_GAME) {
		_btnAbductFail = new ButtonClass(RectClass(580, 10, 620, 69), "abductfail", 10, 69, 69, 70, 71);
		_btnMenu = new ButtonClass(RectClass(582, 70, 619, 105), "menu", 11, 76, 76, 77, 78);
		_interfaceBox->add(_btnAbductFail);
		_interfaceBox->add(_btnMenu);

	} else {
		_btnAbductFail = new ButtonClass(RectClass(580, 22, 620, 75), "abductfail", 10, 69, 69, 70, 71);
		_interfaceBox->add(_btnAbductFail);
	}

	_btnScrollLeft = new ButtonClass(RectClass(168, 22, 188, 97), "scroll left", 8, 59, 60, 61, 62);
	_btnScrollRight = new ButtonClass(RectClass(539, 22, 559, 97), "scroll right", 9, 63, 64, 65, 66);
	_interfaceBox->add(_btnScrollLeft);
	_interfaceBox->add(_btnScrollRight);
}

} // namespace GUI
} // namespace Burger
} // namespace M4
