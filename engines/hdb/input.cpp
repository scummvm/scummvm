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

#include "hdb/hdb.h"

namespace HDB {

bool Input::init() {
	_stylusDown = false;

	warning("STUB: Input::init: Set the default key values");

	_mouseX = kScreenWidth / 2;
	_mouseY = kScreenHeight / 2;
	_lastMouseX = _mouseX;
	_lastMouseY = _mouseY;

	return true;
}

void Input::setButtons(uint16 b) {
	warning("STUB: Input: setButtons required");
}

uint16 Input::getButtons() {
	warning("STUB: Input: getButtons required");
	return 0;
}

void Input::stylusDown(int x, int y) {
	warning("STUB: Input: stylusDown required");
}

void stylusUp(int x, int y) {
	warning("STUB: Input: stylusUp required");
}

void stylusMove(int x, int y) {
	warning("STUB: Input: stylusMove required");
}

}

