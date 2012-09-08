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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "hopkins/events.h"

namespace Hopkins {

void Mouse::INSTALL_SOURIS() {
	// No implementation in original
}

void Mouse::souris_on() {
	souris_flag = true;

	if (mouse_linux) {
		souris_sizex = 52;
		souris_sizey = 32;
	} else {
		souris_sizex = 34;
		souris_sizey = 20;
	}

	ofset_souris_x = 0;
	ofset_souris_y = 0;

	if (!CASSE)
		souris_xy(300, 200);
	else
		souris_xy(150, 100);
}

void Mouse::souris_xy(int xp, int yp) {
	g_system->warpMouse(xp, yp);
}

void Mouse::souris_max() {
	// No implementation in original
}

void Mouse::hideCursor() {
}

void Mouse::showCursor() {
}

} // End of namespace Hopkins
