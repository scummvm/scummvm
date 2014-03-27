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
	_buttonsDown = 0;
	_newKeys = 0;
	_enabledButtons = 0xFFFF;
	_cursorPos.x = 0;
	_cursorPos.y = 0;
	_prevCursorPos.x = 0;
	_prevCursorPos.y = 0;
	// TODO Not sure if this is still needed newTimer(40, 0, 0, Input_onTimer);
	initKeys();
}

void Input::processEvent(Common::Event event) {
	// TODO
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		handleKey(event.kbd.keycode, MOUSE_NONE, true);
		break;
	case Common::EVENT_KEYUP:
		handleKey(event.kbd.keycode, MOUSE_NONE, false);
		break;
	case Common::EVENT_MOUSEMOVE:
		_cursorPos.x = event.mouse.x;
		_cursorPos.y = event.mouse.y;
		break;
	case Common::EVENT_LBUTTONDOWN:
		handleMouseButton(MOUSE_BUTTON0, true);
		break;
	case Common::EVENT_LBUTTONUP:
		handleMouseButton(MOUSE_BUTTON0, false);
		break;
	case Common::EVENT_RBUTTONDOWN:
		handleMouseButton(MOUSE_BUTTON1, true);
		break;
	case Common::EVENT_RBUTTONUP:
		handleMouseButton(MOUSE_BUTTON1, false);
		break;
	default:
		break;
	}
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

void Input::initKeys() {
	// NOTE Skipped debugging keys of the original engine, not sure if used
	addKeyMapping(Common::KEYCODE_INVALID, MOUSE_BUTTON0, 0x01);
	addKeyMapping(Common::KEYCODE_RETURN, MOUSE_NONE, 0x01);
	addKeyMapping(Common::KEYCODE_INVALID, MOUSE_BUTTON1, 0x02);
	addKeyMapping(Common::KEYCODE_TAB, MOUSE_NONE, 0x04);
	addKeyMapping(Common::KEYCODE_INVALID, MOUSE_BUTTON1, 0x04);
	addKeyMapping(Common::KEYCODE_ESCAPE, MOUSE_NONE, 0x08);
	addKeyMapping(Common::KEYCODE_SPACE, MOUSE_NONE, 0x10);
	addKeyMapping(Common::KEYCODE_F1, MOUSE_NONE, 0x20);
	addKeyMapping(Common::KEYCODE_UP, MOUSE_NONE, 0x40);
	addKeyMapping(Common::KEYCODE_DOWN, MOUSE_NONE, 0x80);
	addKeyMapping(Common::KEYCODE_INVALID, MOUSE_BUTTON1, 0x80);
}

void Input::addKeyMapping(Common::KeyCode key, int mouseButton, uint bitMask) {
	KeyMapping keyMapping;
	keyMapping._key = key;
	keyMapping._mouseButton = mouseButton;
	keyMapping._bitMask = bitMask;
	keyMapping._down = false;
	_keyMap.push_back(keyMapping);
}

void Input::handleKey(Common::KeyCode key, int mouseButton, bool down) {
	for (KeyMap::iterator it = _keyMap.begin(); it != _keyMap.end(); ++it) {
		KeyMapping &keyMapping = *it;
		if ((keyMapping._key != Common::KEYCODE_INVALID && keyMapping._key == key) ||
			(keyMapping._mouseButton != MOUSE_NONE && keyMapping._mouseButton == mouseButton)) {
			if (down && !keyMapping._down) {
				_newKeys |= keyMapping._bitMask;
				keyMapping._down = true;
			} else if (!down)
				keyMapping._down = false;
		}
	}
	uint prevButtonStates = _buttonStates;
	_buttonStates |= _newKeys;
	_newKeys = 0;
	_newButtons = ~prevButtonStates & _buttonStates;
}

void Input::handleMouseButton(int mouseButton, bool down) {
	if (down)
		_buttonsDown |= mouseButton;
	else
		_buttonsDown &= ~mouseButton;
	handleKey(Common::KEYCODE_INVALID, mouseButton, down);
}

} // End of namespace Illusions
