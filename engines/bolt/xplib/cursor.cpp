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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

bool XpLib::initCoords() {
	return false;
}

void XpLib::shutdownCoords() {
}

bool XpLib::readCursor(uint16 *outButtons, int16 *outX, int16 *outY) {
	return false;
}

void XpLib::readJoystick(int16 *outX, int16 *outY) {
}

void XpLib::setCursorPos(int16 x, int16 y) {
}

void XpLib::setCursorImage(void *bitmap, int16 hotspotX, int16 hotspotY) {
}

void XpLib::setCursorColor(byte r, byte g, byte b) {
}

bool XpLib::showCursor() {
	return false;
}

void XpLib::hideCursor() {
}

void XpLib::updateCursorPosition() {
}

} // End of namespace Bolt
