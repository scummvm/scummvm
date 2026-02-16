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

bool XpLib::initWindow() {
	return false;
}

void XpLib::shutdownWindow() {

}

int16 XpLib::getEvent(int16 filter, uint32 *outData) {
	return 0;
}

int16 XpLib::peekEvent(int16 filter, uint32 *outData) {
	return 0;
}

void XpLib::unlinkEvent(int16 node) {

}

void XpLib::enqueueEvent(int16 node) {

}

void XpLib::pumpMessages() {

}

void XpLib::handleTimer(uint32 timerId) {

}

void XpLib::handleMouseMove(bool *mouseMoved) {

}

void XpLib::handleMouseButton(int16 down, int16 button) {

}

void XpLib::handleKey(int16 vkey, int16 down) {

}

void XpLib::postJoystickEvent(int16 source, int16 dx, int16 dy) {

}

void XpLib::postEvent(int16 type, uint32 data) {

}

bool XpLib::canDropEvent(int16 type) {
	return false;
}

int16 XpLib::setInactivityTimer(int16 seconds) {
	return 0;
}

int16 XpLib::setScreenSaverTimer(int16 time) {
	return 0;
}

void XpLib::activateScreenSaver() {

}

void XpLib::resetInactivity() {

}

bool XpLib::enableController() {
	return false;
}

void XpLib::disableController() {

}

void XpLib::enableMouse() {

}

void XpLib::disableMouse() {

}

int16 XpLib::getButtonState() {
	return 0;
}

} // End of namespace Bolt
