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

#include "illusions/input.h"

namespace Illusions {

Input::Input() {
	_buttonStates = 0;
	_newButtons = 0;
	// TODO? _buttonsDown = 0;
	// TODO? _unk6 = 0;
	_enabledButtons = 0xFFFF;
	_cursorPos.x = 0;
	_cursorPos.y = 0;
	_prevCursorPos.x = 0;
	_prevCursorPos.y = 0;
	// TODO Not sure if this is still needed newTimer(40, 0, 0, Input_onTimer);
}

void Input::processEvent(Common::Event event) {
	// TODO
}

bool Input::pollButton(uint buttons) {
	if (lookButtonStates(buttons)) {
		_buttonStates &= ~buttons;
		return true;
	}
	return false;
}

bool Input::lookButtonStates(uint buttons) {
	return (buttons & (_buttonStates & _enabledButtons)) != 0;
}

bool Input::lookNewButtons(uint buttons) {
	return (buttons & (_newButtons & _enabledButtons)) != 0;
}

void Input::setButtonState(uint buttons) {
	_buttonStates |= _enabledButtons & buttons;
}

void Input::discardButtons(uint buttons) {
	_buttonStates &= ~buttons;
}

void Input::activateButton(uint buttons) {
	_enabledButtons |= buttons;
	_buttonStates &= ~buttons;
}

void Input::deactivateButton(uint buttons) {
	_enabledButtons &= ~buttons;
}

Common::Point Input::getCursorPosition() {
	return _cursorPos;
}

void Input::setCursorPosition(Common::Point mousePos) {
	_prevCursorPos = _cursorPos = mousePos;
}

Common::Point Input::getCursorDelta() {
	Common::Point deltaPos;
	deltaPos.x = _prevCursorPos.x - _cursorPos.x;
	deltaPos.y = _prevCursorPos.y - _cursorPos.y;
	_prevCursorPos = _cursorPos;
	return deltaPos;
}

} // End of namespace Illusions
