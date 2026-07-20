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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/input.h"

#include "common/debug.h"

#include "ripper/detection.h"

namespace Ripper {

InputManager::InputManager(Common::EventManager *eventManager) : _eventManager(eventManager) {
}

uint16 InputManager::translateKey(const Common::KeyState &key) {
	// ScummVM backends may publish navigation keys in both keycode and ascii
	// (for example, Right Arrow arrives with ascii 0x0113). Resolve mapped
	// non-printing keys first so they retain the BIOS command values consumed by
	// the original input paths.
	switch (key.keycode) {
	case Common::KEYCODE_F1:
		return 0x3b00;
	case Common::KEYCODE_F2:
		return 0x3c00;
	case Common::KEYCODE_F3:
		return 0x3d00;
	case Common::KEYCODE_F4:
		return 0x3e00;
	case Common::KEYCODE_F5:
		return 0x3f00;
	case Common::KEYCODE_F6:
		return 0x4000;
	case Common::KEYCODE_F7:
		return 0x4100;
	case Common::KEYCODE_F8:
		return 0x4200;
	case Common::KEYCODE_F9:
		return 0x4300;
	case Common::KEYCODE_F10:
		return 0x4400;
	case Common::KEYCODE_HOME:
		return 0x4700;
	case Common::KEYCODE_UP:
		return 0x4800;
	case Common::KEYCODE_PAGEUP:
		return 0x4900;
	case Common::KEYCODE_LEFT:
		return 0x4b00;
	case Common::KEYCODE_RIGHT:
		return 0x4d00;
	case Common::KEYCODE_END:
		return 0x4f00;
	case Common::KEYCODE_DOWN:
		return 0x5000;
	case Common::KEYCODE_PAGEDOWN:
		return 0x5100;
	case Common::KEYCODE_INSERT:
		return 0x5200;
	case Common::KEYCODE_DELETE:
		return 0x5300;
	default:
		return key.ascii;
	}
}

void InputManager::updateMousePosition(const Common::Event &event) {
	_mouseState.position = event.mouse;
	debugC(3, kDebugInput, "Ripper: mouse position x=%d y=%d",
		_mouseState.position.x, _mouseState.position.y);
}

void InputManager::updateMouseButton(const Common::Event &event, uint16 button, bool pressed) {
	updateMousePosition(event);

	if (pressed) {
		if ((_mouseState.buttons & button) == 0)
			_mouseState.pressed |= button;
		_mouseState.buttons |= button;
	} else {
		if ((_mouseState.buttons & button) != 0)
			_mouseState.released |= button;
		_mouseState.buttons &= ~button;
	}

	debugC(3, kDebugInput,
		"Ripper: mouse button=%u down=%d buttons=0x%02x pressed=0x%02x released=0x%02x",
		button, pressed, _mouseState.buttons, _mouseState.pressed, _mouseState.released);
}

bool InputManager::pollEvents() {
	Common::Event event;
	bool quitRequested = false;

	while (_eventManager->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quitRequested = true;
			break;

		case Common::EVENT_KEYDOWN: {
			const uint16 command = translateKey(event.kbd);
			if (command != 0) {
				_pendingKeys.push(command);
				debugC(3, kDebugInput, "Ripper: queued keyboard command 0x%04x", command);
			}
			break;
		}

		case Common::EVENT_MOUSEMOVE:
			updateMousePosition(event);
			break;

		case Common::EVENT_LBUTTONDOWN:
			updateMouseButton(event, kMouseButtonLeft, true);
			break;
		case Common::EVENT_LBUTTONUP:
			updateMouseButton(event, kMouseButtonLeft, false);
			break;
		case Common::EVENT_RBUTTONDOWN:
			updateMouseButton(event, kMouseButtonRight, true);
			break;
		case Common::EVENT_RBUTTONUP:
			updateMouseButton(event, kMouseButtonRight, false);
			break;
		case Common::EVENT_MBUTTONDOWN:
			updateMouseButton(event, kMouseButtonMiddle, true);
			break;
		case Common::EVENT_MBUTTONUP:
			updateMouseButton(event, kMouseButtonMiddle, false);
			break;
		case Common::EVENT_WHEELUP:
			++_mouseState.wheel;
			debugC(3, kDebugInput, "Ripper: mouse wheel delta=%d", _mouseState.wheel);
			break;
		case Common::EVENT_WHEELDOWN:
			--_mouseState.wheel;
			debugC(3, kDebugInput, "Ripper: mouse wheel delta=%d", _mouseState.wheel);
			break;

		default:
			break;
		}
	}

	return quitRequested;
}

bool InputManager::hasPendingKey() const {
	return !_pendingKeys.empty();
}

uint16 InputManager::peekKey() const {
	return _pendingKeys.empty() ? 0 : _pendingKeys.front();
}

uint16 InputManager::consumeKey() {
	if (_pendingKeys.empty())
		return 0;

	const uint16 command = _pendingKeys.pop();
	debugC(3, kDebugInput, "Ripper: consumed keyboard command 0x%04x", command);
	return command;
}

void InputManager::drainKeys() {
	const int count = _pendingKeys.size();
	_pendingKeys.clear();
	// Blocking original loops, including the action-9 GC/CSH chooser at
	// 0x38871, drain after every empty poll. Keep that input ordering without
	// flooding normal diagnostics while the loop waits for a mouse selection.
	debugC(count == 0 ? 3 : 2, kDebugInput,
		"Ripper: drained %d pending keyboard commands", count);
}

void InputManager::discardMouseTransitions() {
	debugC(2, kDebugInput,
		"Ripper: discarded mouse transitions pressed=0x%02x released=0x%02x "
		"buttons=0x%02x wheel=%d",
		_mouseState.pressed, _mouseState.released, _mouseState.buttons, _mouseState.wheel);
	_mouseState.pressed = 0;
	_mouseState.released = 0;
	_mouseState.wheel = 0;
}

MouseState InputManager::publishMouseState() {
	const MouseState published = _mouseState;
	_mouseState.pressed = 0;
	_mouseState.released = 0;
	_mouseState.wheel = 0;
	return published;
}

} // End of namespace Ripper
