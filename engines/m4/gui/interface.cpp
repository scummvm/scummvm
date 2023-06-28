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

#include "m4/gui/interface.h"
#include "m4/globals.h"

namespace M4 {

#define _GI(X) _G(interface).X

static void sub1();

void interface_init(int arrow, int wait, int look, int grab, int use) {
	_GI(arrow) = arrow;
	_GI(wait) = wait;
	_GI(look) = look;
	_GI(grab) = grab;
	_GI(use) = use;

	mouse_set_sprite(wait);

	_GI(gameInterfaceBuff) = new GrBuff(_GI(x2) - _GI(x1), _GI(y2) - _GI(y1));
	sub1();

	mouse_set_sprite(arrow);
}

void interface_shutdown() {
	// TODO
}

static void sub1() {

}

} // End of namespace M4
