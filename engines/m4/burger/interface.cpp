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

#include "m4/burger/interface.h"
#include "m4/globals.h"

namespace M4 {
namespace Burger {

void Interface::init(int arrow, int wait, int look, int grab, int use) {
	_arrow = arrow;
	_wait = wait;
	_look = look;
	_grab = grab;
	_use = use;

	mouse_set_sprite(wait);

	_gameInterfaceBuff = new GrBuff(_x2 - _x1, _y2 - _y1);
	setup();

	mouse_set_sprite(arrow);
}

Interface::~Interface() {
	// TODO
}

void Interface::show() {
	error("TODO: Interface::show");
}

void Interface::setup() {
	_interfaceBox = new InterfaceBox(RectClass(0, 0, SCREEN_WIDTH - 1, 105));
}

} // namespace Burger
} // namespace M4
