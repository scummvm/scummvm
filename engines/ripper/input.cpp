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
#include "common/system.h"

#include "ripper/detection.h"

namespace Ripper {

namespace {

uint16 altLetterScanCode(Common::KeyCode keycode) {
	switch (keycode) {
	case Common::KEYCODE_a: return 0x1e;
	case Common::KEYCODE_b: return 0x30;
	case Common::KEYCODE_c: return 0x2e;
	case Common::KEYCODE_d: return 0x20;
	case Common::KEYCODE_e: return 0x12;
	case Common::KEYCODE_f: return 0x21;
	case Common::KEYCODE_g: return 0x22;
	case Common::KEYCODE_h: return 0x23;
	case Common::KEYCODE_i: return 0x17;
	case Common::KEYCODE_j: return 0x24;
	case Common::KEYCODE_k: return 0x25;
	case Common::KEYCODE_l: return 0x26;
	case Common::KEYCODE_m: return 0x32;
	case Common::KEYCODE_n: return 0x31;
	case Common::KEYCODE_o: return 0x18;
	case Common::KEYCODE_p: return 0x19;
	case Common::KEYCODE_q: return 0x10;
	case Common::KEYCODE_r: return 0x13;
	case Common::KEYCODE_s: return 0x1f;
	case Common::KEYCODE_t: return 0x14;
	case Common::KEYCODE_u: return 0x16;
	case Common::KEYCODE_v: return 0x2f;
	case Common::KEYCODE_w: return 0x11;
	case Common::KEYCODE_x: return 0x2d;
	case Common::KEYCODE_y: return 0x15;
	case Common::KEYCODE_z: return 0x2c;
	default: return 0;
	}
}

char letterForAltScanCode(uint16 scanCode) {
	for (uint keycode = Common::KEYCODE_a; keycode <= Common::KEYCODE_z; ++keycode) {
		if (altLetterScanCode((Common::KeyCode)keycode) == scanCode)
			return 'A' + keycode - Common::KEYCODE_a;
	}
	return 0;
}

uint16 functionKeyCommand(Common::KeyCode keycode, byte flags) {
	if (keycode < Common::KEYCODE_F1 || keycode > Common::KEYCODE_F10)
		return 0;
	const uint functionIndex = keycode - Common::KEYCODE_F1;
	if ((flags & Common::KBD_ALT) != 0)
		return (0x68 + functionIndex) << 8;
	if ((flags & Common::KBD_CTRL) != 0)
		return (0x5e + functionIndex) << 8;
	if ((flags & Common::KBD_SHIFT) != 0)
		return (0x54 + functionIndex) << 8;
	return (0x3b + functionIndex) << 8;
}

} // End of anonymous namespace

InputManager::InputManager(Common::EventManager *eventManager) : _eventManager(eventManager) {
}

uint16 translateKeyToCommand(const Common::KeyState &key) {
	// PollInteractionAndResolveSelection at 0x13fc7 and
	// ServiceMediaPresentationTextControl at 0x17014 consume the DOS control
	// character values for the v1.05 replay, text, and auto-scroll shortcuts.
	if ((key.flags & Common::KBD_CTRL) != 0 &&
			key.keycode >= Common::KEYCODE_a && key.keycode <= Common::KEYCODE_z)
		return key.keycode - Common::KEYCODE_a + 1;

	// PollKeyboardCommand at 0x4d364 returns the BIOS AX command. Alt-letter
	// commands therefore carry the physical Set 1 scan code in the high byte
	// and zero in the low byte. InitializeDefaultSettingsBlob at 0x1eea2 uses
	// this representation for all retail Alt toolbar defaults, including Alt+R
	// as 0x1300. The same translation also preserves Alt+H for the web puzzle.
	if ((key.flags & Common::KBD_ALT) != 0) {
		const uint16 letterScanCode = altLetterScanCode(key.keycode);
		if (letterScanCode != 0)
			return letterScanCode << 8;
		if (key.keycode >= Common::KEYCODE_1 && key.keycode <= Common::KEYCODE_9)
			return (key.keycode - Common::KEYCODE_1 + 2) << 8;
		if (key.keycode == Common::KEYCODE_0)
			return 0x0b00;
	}

	const uint16 functionCommand = functionKeyCommand(key.keycode, key.flags);
	if (functionCommand != 0)
		return functionCommand;

	// ScummVM backends may publish navigation keys in both keycode and ascii
	// (for example, Right Arrow arrives with ascii 0x0113). Resolve mapped
	// non-printing keys first so they retain the BIOS command values consumed by
	// the original input paths.
	switch (key.keycode) {
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

Common::String formatKeyCommandLabel(uint16 command) {
	switch (command) {
	case 0x0008: return "BACKSPACE";
	case 0x0009: return "TAB";
	case 0x000d: return "ENTER";
	case 0x001b: return "ESC";
	case 0x4700: return "HOME";
	case 0x4800: return "UP";
	case 0x4900: return "PAGE UP";
	case 0x4b00: return "LEFT";
	case 0x4d00: return "RIGHT";
	case 0x4f00: return "END";
	case 0x5000: return "DOWN";
	case 0x5100: return "PAGE DOWN";
	case 0x5200: return "INSERT";
	case 0x5300: return "DELETE";
	default:
		break;
	}

	if ((command & 0xff00) == 0) {
		const byte character = command & 0xff;
		if (character >= 1 && character <= 26)
			return Common::String::format("CTRL+%c", 'A' + character - 1);
		if (character >= 0x20 && character <= 0x7e) {
			char printable = character;
			if (printable >= 'a' && printable <= 'z')
				printable -= 'a' - 'A';
			return Common::String(printable);
		}
	}

	const uint16 scanCode = command >> 8;
	const char altLetter = letterForAltScanCode(scanCode);
	if ((command & 0xff) == 0 && altLetter != 0)
		return Common::String::format("ALT+%c", altLetter);
	if ((command & 0xff) == 0 && scanCode >= 0x02 && scanCode <= 0x0a)
		return Common::String::format("ALT+%u", scanCode - 1);
	if (command == 0x0b00)
		return "ALT+0";
	if (scanCode >= 0x3b && scanCode <= 0x44)
		return Common::String::format("F%u", scanCode - 0x3a);
	if (scanCode >= 0x54 && scanCode <= 0x5d)
		return Common::String::format("SHIFT+F%u", scanCode - 0x53);
	if (scanCode >= 0x5e && scanCode <= 0x67)
		return Common::String::format("CTRL+F%u", scanCode - 0x5d);
	if (scanCode >= 0x68 && scanCode <= 0x71)
		return Common::String::format("ALT+F%u", scanCode - 0x67);
	return Common::String::format("0x%04X", command);
}

void InputManager::updateMousePosition(const Common::Event &event) {
	_mouseState.position = event.mouse;
	debugC(3, kDebugInput, "Ripper: mouse position x=%d y=%d",
		_mouseState.position.x, _mouseState.position.y);
}

void InputManager::warpMousePosition(const Common::Point &position) {
	_mouseState.position = position;
	g_system->warpMouse(position.x, position.y);
	debugC(3, kDebugInput, "Ripper: warped mouse position x=%d y=%d",
		position.x, position.y);
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
			const uint16 command = translateKeyToCommand(event.kbd);
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
