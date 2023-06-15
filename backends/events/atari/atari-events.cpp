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

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE // atari-graphics.h's unordered_set

#include "backends/events/atari/atari-events.h"

#include <mint/osbind.h>

#include "backends/graphics/atari/atari-graphics.h"
#include "backends/platform/atari/osystem_atari.h"
#include "common/rect.h"

//! bit 0: rmb
//! bit 1: lmb
volatile uint8	g_atari_ikbd_mouse_buttons_state = 0;
volatile int16	g_atari_ikbd_mouse_delta_x = 0;
volatile int16	g_atari_ikbd_mouse_delta_y = 0;

#define SCANCODES_SIZE 256
volatile uint8	g_atari_ikbd_scancodes[SCANCODES_SIZE];
uint16			g_atari_ikbd_scancodes_mask = SCANCODES_SIZE-1;
volatile uint16	g_atari_ikbb_scancodes_head = 0;
static uint16	g_atari_ikbb_scancodes_tail = 0;


AtariEventSource::AtariEventSource() {
	_system = dynamic_cast<OSystem_Atari*>(g_system);
	assert(_system != nullptr);

	_KEYTAB *pKeyTables = (_KEYTAB*)Keytbl(KT_NOCHANGE, KT_NOCHANGE, KT_NOCHANGE);

	memcpy(_unshiftToAscii, pKeyTables->unshift, 128);
	memcpy(_shiftToAscii, pKeyTables->shift, 128);
	memcpy(_capsToAscii, pKeyTables->caps, 128);

	_scancodeToKeycode[0x01] = Common::KEYCODE_ESCAPE;
	_scancodeToKeycode[0x0e] = Common::KEYCODE_BACKSPACE;
	_scancodeToKeycode[0x0f] = Common::KEYCODE_TAB;
	_scancodeToKeycode[0x1c] = Common::KEYCODE_RETURN;
	_scancodeToKeycode[0x39] = Common::KEYCODE_SPACE;
	_scancodeToKeycode[0x3b] = Common::KEYCODE_F1;
	_scancodeToKeycode[0x3c] = Common::KEYCODE_F2;
	_scancodeToKeycode[0x3d] = Common::KEYCODE_F3;
	_scancodeToKeycode[0x3e] = Common::KEYCODE_F4;
	_scancodeToKeycode[0x3f] = Common::KEYCODE_F5;
	_scancodeToKeycode[0x40] = Common::KEYCODE_F6;
	_scancodeToKeycode[0x41] = Common::KEYCODE_F7;
	_scancodeToKeycode[0x42] = Common::KEYCODE_F8;
	_scancodeToKeycode[0x43] = Common::KEYCODE_F9;
	_scancodeToKeycode[0x44] = Common::KEYCODE_F10;
	_scancodeToKeycode[0x45] = Common::KEYCODE_PAGEUP;	// Eiffel only
	_scancodeToKeycode[0x46] = Common::KEYCODE_PAGEDOWN;	// Eiffel only
	_scancodeToKeycode[0x47] = Common::KEYCODE_HOME;
	_scancodeToKeycode[0x48] = Common::KEYCODE_UP;
	_scancodeToKeycode[0x4a] = Common::KEYCODE_KP_MINUS;
	_scancodeToKeycode[0x4b] = Common::KEYCODE_LEFT;
	_scancodeToKeycode[0x4c] = Common::KEYCODE_LMETA;
	_scancodeToKeycode[0x4d] = Common::KEYCODE_RIGHT;
	_scancodeToKeycode[0x4e] = Common::KEYCODE_KP_PLUS;
	_scancodeToKeycode[0x4f] = Common::KEYCODE_PAUSE;	// Eiffel only
	_scancodeToKeycode[0x50] = Common::KEYCODE_DOWN;
	_scancodeToKeycode[0x52] = Common::KEYCODE_INSERT;
	_scancodeToKeycode[0x53] = Common::KEYCODE_DELETE;
	_scancodeToKeycode[0x55] = Common::KEYCODE_END;	// Eiffel only
	_scancodeToKeycode[0x5b] = Common::KEYCODE_TILDE;	// Eiffel only
	_scancodeToKeycode[0x61] = Common::KEYCODE_F12;	// UNDO
	_scancodeToKeycode[0x62] = Common::KEYCODE_F11;	// HELP
	_scancodeToKeycode[0x63] = Common::KEYCODE_SLASH;	// KEYPAD /
	_scancodeToKeycode[0x64] = Common::KEYCODE_KP_DIVIDE;
	_scancodeToKeycode[0x65] = Common::KEYCODE_KP_MULTIPLY;
	_scancodeToKeycode[0x66] = Common::KEYCODE_KP_MULTIPLY;	// duplicate?
	_scancodeToKeycode[0x67] = Common::KEYCODE_7;	// KEYPAD 7
	_scancodeToKeycode[0x68] = Common::KEYCODE_8;	// KEYPAD 8
	_scancodeToKeycode[0x69] = Common::KEYCODE_9;	// KEYPAD 9
	_scancodeToKeycode[0x6a] = Common::KEYCODE_4;	// KEYPAD 4
	_scancodeToKeycode[0x6b] = Common::KEYCODE_5;	// KEYPAD 5
	_scancodeToKeycode[0x6c] = Common::KEYCODE_6;	// KEYPAD 6
	_scancodeToKeycode[0x6d] = Common::KEYCODE_1;	// KEYPAD 1
	_scancodeToKeycode[0x6e] = Common::KEYCODE_2;	// KEYPAD 2
	_scancodeToKeycode[0x6f] = Common::KEYCODE_3;	// KEYPAD 3
	_scancodeToKeycode[0x70] = Common::KEYCODE_0;	// KEYPAD 0
	_scancodeToKeycode[0x71] = Common::KEYCODE_KP_PERIOD;
	_scancodeToKeycode[0x72] = Common::KEYCODE_KP_ENTER;
}

bool AtariEventSource::pollEvent(Common::Event &event) {
	if (!_graphicsManager)
		return false;

	_system->update();

	if ((g_atari_ikbd_mouse_buttons_state & 0x01) && !_oldRmbDown) {
		event.type = Common::EVENT_RBUTTONDOWN;
		event.mouse = _graphicsManager->getMousePosition();
		_oldRmbDown = true;
		return true;
	}

	if (!(g_atari_ikbd_mouse_buttons_state & 0x01) && _oldRmbDown) {
		event.type = Common::EVENT_RBUTTONUP;
		event.mouse = _graphicsManager->getMousePosition();
		_oldRmbDown = false;
		return true;
	}

	if ((g_atari_ikbd_mouse_buttons_state & 0x02) && !_oldLmbDown) {
		event.type = Common::EVENT_LBUTTONDOWN;
		event.mouse = _graphicsManager->getMousePosition();
		_oldLmbDown = true;
		return true;
	}

	if (!(g_atari_ikbd_mouse_buttons_state & 0x02) && _oldLmbDown) {
		event.type = Common::EVENT_LBUTTONUP;
		event.mouse = _graphicsManager->getMousePosition();
		_oldLmbDown = false;
		return true;
	}

	if (g_atari_ikbd_mouse_delta_x != 0 || g_atari_ikbd_mouse_delta_y != 0) {
		const int deltaX = g_atari_ikbd_mouse_delta_x;
		const int deltaY = g_atari_ikbd_mouse_delta_y;

		g_atari_ikbd_mouse_delta_x = g_atari_ikbd_mouse_delta_y = 0;

		_graphicsManager->updateMousePosition(deltaX, deltaY);

		event.type = Common::EVENT_MOUSEMOVE;
		event.mouse = _graphicsManager->getMousePosition();
		event.relMouse = Common::Point(deltaX, deltaY);
		return true;
	}

	if (g_atari_ikbb_scancodes_head != g_atari_ikbb_scancodes_tail) {
		byte scancode = g_atari_ikbd_scancodes[g_atari_ikbb_scancodes_tail++];
		g_atari_ikbb_scancodes_tail &= SCANCODES_SIZE-1;

		bool pressed = !(scancode & 0x80);
		scancode &= 0x7f;

		if (scancode == 0x1d)
			_ctrlActive = pressed;

		if (scancode == 0x2a)
			_lshiftActive = pressed;

		if (scancode == 0x36)
			_rshiftActive = pressed;

		if (scancode == 0x38)
			_altActive = pressed;

		if (scancode == 0x3a && pressed)
			_capslockActive = !_capslockActive;

		// Eiffel only
		if (scancode == 0x59) {
			event.type = Common::EVENT_WHEELUP;
			event.mouse = _graphicsManager->getMousePosition();
			return true;
		}

		// Eiffel only
		if (scancode == 0x5a) {
			event.type = Common::EVENT_WHEELDOWN;
			event.mouse = _graphicsManager->getMousePosition();
			return true;
		}

		uint16 ascii;
		if (_lshiftActive || _rshiftActive) {
			ascii = _shiftToAscii[scancode];
		} else if (_capslockActive) {
			ascii = _capsToAscii[scancode];
		} else {
			ascii = _unshiftToAscii[scancode];
		}

		Common::KeyCode keycode = _scancodeToKeycode.getValOrDefault(scancode, Common::KEYCODE_INVALID);
		switch (keycode) {
		case Common::KEYCODE_BACKSPACE:
			ascii = Common::ASCII_BACKSPACE;
			break;
		case Common::KEYCODE_TAB:
			ascii = Common::ASCII_TAB;
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			ascii = Common::ASCII_RETURN;
			break;
		case Common::KEYCODE_ESCAPE:
			ascii = Common::ASCII_ESCAPE;
			break;
		case Common::KEYCODE_SPACE:
			ascii = Common::ASCII_SPACE;
			break;
		case Common::KEYCODE_F1:
			ascii = Common::ASCII_F1;
			break;
		case Common::KEYCODE_F2:
			ascii = Common::ASCII_F2;
			break;
		case Common::KEYCODE_F3:
			ascii = Common::ASCII_F3;
			break;
		case Common::KEYCODE_F4:
			ascii = Common::ASCII_F4;
			break;
		case Common::KEYCODE_F5:
			ascii = Common::ASCII_F5;
			break;
		case Common::KEYCODE_F6:
			ascii = Common::ASCII_F6;
			break;
		case Common::KEYCODE_F7:
			ascii = Common::ASCII_F7;
			break;
		case Common::KEYCODE_F8:
			ascii = Common::ASCII_F8;
			break;
		case Common::KEYCODE_F9:
			ascii = Common::ASCII_F9;
			break;
		case Common::KEYCODE_F10:
			ascii = Common::ASCII_F10;
			break;
		case Common::KEYCODE_F11:
			ascii = Common::ASCII_F11;
			break;
		case Common::KEYCODE_F12:
			ascii = Common::ASCII_F12;
			break;
		default:
			break;
		}

		if (ascii >= ' ' && ascii <= '~') {
			if (keycode == Common::KEYCODE_INVALID)
				keycode = _asciiToKeycode[ascii - ' '];
		}

		event.type = pressed ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
		event.kbd = Common::KeyState(keycode, ascii);
		event.kbd.flags |= _ctrlActive ? Common::KBD_CTRL : 0;
		event.kbd.flags |= _altActive ? Common::KBD_ALT : 0;
		event.kbd.flags |= (_lshiftActive || _rshiftActive) ? Common::KBD_SHIFT : 0;
		event.kbd.flags |= _capslockActive ? Common::KBD_CAPS : 0;

		return true;
	}

	return false;
}
