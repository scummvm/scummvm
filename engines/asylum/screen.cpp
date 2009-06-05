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
 */

#include "engines/engine.h"

#include "asylum/screen.h"

namespace Asylum {

Screen::Screen(OSystem *system) {
	initGraphics(SCREEN_WIDTH, SCREEN_DEPTH, true);

	_system = system;
	_frontBuf = _backBuf = NULL;
	_updatePalette = false;
	_fullRefresh = false;
}

Screen::~Screen() {
}

void clearScreen() {

}

void updateScreen() {

}

void updateRect(Common::Rect *r) {
}

void setPalette(uint8 * palette) {
}

void drawLine(int x0, int y0, int x1, int y1, uint8 colour) {
}

} // end of namespace Asylum
