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

#include "common/system.h"
#include "ags/events.h"
#include "ags/globals.h"

namespace AGS {

EventsManager *g_events;

EventsManager::EventsManager() {
	g_events = this;
	_keys.resize(Common::KEYCODE_LAST);
	Common::fill(&_joystickAxis[0], &_joystickAxis[32], 0);
	Common::fill(&_joystickButton[0], &_joystickButton[32], 0);
}

EventsManager::~EventsManager() {
	g_events = nullptr;
}

void EventsManager::pollEvents() {
	Common::Event e;

	while (g_system->getEventManager()->pollEvent(e)) {
		switch (e.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			_G(want_exit) = true;
			_G(abort_engine) = true;
			_G(check_dynamic_sprites_at_exit) = false;
			return;

		case Common::EVENT_JOYAXIS_MOTION:
			assert(e.joystick.axis < 32);
			_joystickAxis[e.joystick.axis] = e.joystick.position;
			break;

		case Common::EVENT_JOYBUTTON_DOWN:
			assert(e.joystick.button < 32);
			_joystickButton[e.joystick.button] = true;
			break;

		case Common::EVENT_JOYBUTTON_UP:
			assert(e.joystick.button < 32);
			_joystickButton[e.joystick.button] = false;
			break;

		case Common::EVENT_KEYDOWN:
			updateKeys(e, true);
			_keyEvents.push(e);
			break;

		case Common::EVENT_KEYUP:
			updateKeys(e, false);
			break;

		default:
			if (e.type == Common::EVENT_MOUSEMOVE)
				_mousePos = Common::Point(e.mouse.x, e.mouse.y);

			// Add other event types to the pending events queue. If the event is a
			// mouse move and the prior one was also, then discard the prior one.
			// This'll help prevent too many mouse move events accumulating
			if (e.type == Common::EVENT_MOUSEMOVE && !_pendingEvents.empty() &&
			        _pendingEvents.back().type == Common::EVENT_MOUSEMOVE)
				_pendingEvents.back() = e;
			else
				_pendingEvents.push(e);
			break;
		}
	}
}

#if 0
bool EventsManager::keypressed() {
	pollEvents();
	return !_pendingKeys.empty();
}

#define EXTENDED_KEY_CODE ('\0')

Common::Event EventsManager::readKey() {
	pollEvents();
	return _pendingKeys.empty() ? Common::Event() : _pendingKeys.pop();
}
#endif

Common::Event EventsManager::readEvent() {
	pollEvents();
	return _pendingEvents.empty() ? Common::Event() : _pendingEvents.pop();
}

void EventsManager::warpMouse(const Common::Point &newPos) {
	g_system->warpMouse(newPos.x, newPos.y);
}

bool EventsManager::isModifierKey(const Common::KeyCode &keycode) const {
	return keycode == Common::KEYCODE_LCTRL || keycode == Common::KEYCODE_LALT
	       || keycode == Common::KEYCODE_RCTRL || keycode == Common::KEYCODE_RALT
	       || keycode == Common::KEYCODE_LSHIFT || keycode == Common::KEYCODE_RSHIFT
	       || keycode == Common::KEYCODE_LSUPER || keycode == Common::KEYCODE_RSUPER
	       || keycode == Common::KEYCODE_CAPSLOCK || keycode == Common::KEYCODE_NUMLOCK
	       || keycode == Common::KEYCODE_SCROLLOCK;
}

bool EventsManager::isExtendedKey(const Common::KeyCode &keycode) const {
	const Common::KeyCode EXTENDED_KEYS[] = {
		Common::KEYCODE_F1, Common::KEYCODE_F2, Common::KEYCODE_F3,
		Common::KEYCODE_F4, Common::KEYCODE_F5, Common::KEYCODE_F6,
		Common::KEYCODE_F7, Common::KEYCODE_F8, Common::KEYCODE_F9,
		Common::KEYCODE_F10, Common::KEYCODE_F11, Common::KEYCODE_F12,
		Common::KEYCODE_KP0, Common::KEYCODE_KP1, Common::KEYCODE_KP2,
		Common::KEYCODE_KP3, Common::KEYCODE_KP4, Common::KEYCODE_KP5,
		Common::KEYCODE_KP6, Common::KEYCODE_KP7, Common::KEYCODE_KP8,
		Common::KEYCODE_KP9, Common::KEYCODE_KP_PERIOD,
		Common::KEYCODE_INSERT, Common::KEYCODE_DELETE,
		Common::KEYCODE_HOME, Common::KEYCODE_END,
		Common::KEYCODE_PAGEUP, Common::KEYCODE_PAGEDOWN,
		Common::KEYCODE_LEFT, Common::KEYCODE_RIGHT,
		Common::KEYCODE_UP, Common::KEYCODE_DOWN,
		Common::KEYCODE_INVALID
	};

	for (const Common::KeyCode *kc = EXTENDED_KEYS;
	        *kc != Common::KEYCODE_INVALID; ++kc) {
		if (keycode == *kc)
			return true;
	}

	return false;
}

void EventsManager::updateKeys(const Common::Event &event, bool isDown) {
	_keyModifierFlags = event.kbd.flags;

	_keys[event.kbd.keycode] = isDown;
}

bool EventsManager::isKeyPressed(AGS3::eAGSKeyCode key) {
	pollEvents();

	Common::KeyCode kc[3];
	if (!ags_key_to_scancode(key, kc))
		return false;

	return (kc[0] != Common::KEYCODE_INVALID && _keys[kc[0]]) ||
		(kc[1] != Common::KEYCODE_INVALID && _keys[kc[1]]) ||
		(kc[2] != Common::KEYCODE_INVALID && _keys[kc[2]]);
}

bool EventsManager::ags_key_to_scancode(AGS3::eAGSKeyCode key, Common::KeyCode(&kc)[3]) {
	kc[0] = Common::KEYCODE_INVALID;
	kc[1] = Common::KEYCODE_INVALID;
	kc[2] = Common::KEYCODE_INVALID;
	Common::KeyCode sym = Common::KEYCODE_INVALID;

	// SDL sym codes happen to match small ASCII letters, so lowercase ours if necessary
	if (key >= AGS3::eAGSKeyCodeA && key <= AGS3::eAGSKeyCodeZ) {
		sym = static_cast<Common::KeyCode>(key - AGS3::eAGSKeyCodeA + Common::KEYCODE_a);
	}
	// Rest of the printable characters seem to match (and match ascii codes)
	else if (key >= AGS3::eAGSKeyCodeSpace && key <= AGS3::eAGSKeyCodeBackquote) {
		sym = static_cast<Common::KeyCode>(key);
	}

	if (sym != Common::KEYCODE_INVALID) {
		kc[0] = sym;
		return true;
	}

	// Other keys are mapped directly to scancode (based on [sonneveld]'s code)
	switch (key) {
	case AGS3::eAGSKeyCodeBackspace:
		kc[0] = Common::KEYCODE_BACKSPACE;
		return true;
	case AGS3::eAGSKeyCodeTab:
		kc[0] = Common::KEYCODE_TAB;
		return true;
	case AGS3::eAGSKeyCodeReturn:
		kc[0] = Common::KEYCODE_RETURN;
		kc[1] = Common::KEYCODE_KP_ENTER;
		return true;
	case AGS3::eAGSKeyCodeEscape:
		kc[0] = Common::KEYCODE_ESCAPE;
		return true;

	case AGS3::eAGSKeyCodeF1:
		kc[0] = Common::KEYCODE_F1;
		return true;
	case AGS3::eAGSKeyCodeF2:
		kc[0] = Common::KEYCODE_F2;
		return true;
	case AGS3::eAGSKeyCodeF3:
		kc[0] = Common::KEYCODE_F3;
		return true;
	case AGS3::eAGSKeyCodeF4:
		kc[0] = Common::KEYCODE_F4;
		return true;
	case AGS3::eAGSKeyCodeF5:
		kc[0] = Common::KEYCODE_F5;
		return true;
	case AGS3::eAGSKeyCodeF6:
		kc[0] = Common::KEYCODE_F6;
		return true;
	case AGS3::eAGSKeyCodeF7:
		kc[0] = Common::KEYCODE_F7;
		return true;
	case AGS3::eAGSKeyCodeF8:
		kc[0] = Common::KEYCODE_F8;
		return true;
	case AGS3::eAGSKeyCodeF9:
		kc[0] = Common::KEYCODE_F9;
		return true;
	case AGS3::eAGSKeyCodeF10:
		kc[0] = Common::KEYCODE_F10;
		return true;
	case AGS3::eAGSKeyCodeF11:
		kc[0] = Common::KEYCODE_F11;
		return true;
	case AGS3::eAGSKeyCodeF12:
		kc[0] = Common::KEYCODE_F12;
		return true;

	case AGS3::eAGSKeyCodeHome:
		kc[0] = Common::KEYCODE_KP7;
		kc[1] = Common::KEYCODE_HOME;
		return true;
	case AGS3::eAGSKeyCodeUpArrow:
		kc[0] = Common::KEYCODE_KP8;
		kc[1] = Common::KEYCODE_UP;
		return true;
	case AGS3::eAGSKeyCodePageUp:
		kc[0] = Common::KEYCODE_KP9;
		kc[1] = Common::KEYCODE_PAGEUP;
		return true;
	case AGS3::eAGSKeyCodeLeftArrow:
		kc[0] = Common::KEYCODE_KP4;
		kc[1] = Common::KEYCODE_LEFT;
		return true;
	case AGS3::eAGSKeyCodeNumPad5:
		kc[0] = Common::KEYCODE_KP5;
		return true;
	case AGS3::eAGSKeyCodeRightArrow:
		kc[0] = Common::KEYCODE_KP6;
		kc[1] = Common::KEYCODE_RIGHT;
		return true;
	case AGS3::eAGSKeyCodeEnd:
		kc[0] = Common::KEYCODE_KP1;
		kc[1] = Common::KEYCODE_END;
		return true;
	case AGS3::eAGSKeyCodeDownArrow:
		kc[0] = Common::KEYCODE_KP2;
		kc[1] = Common::KEYCODE_DOWN;
		return true;
	case AGS3::eAGSKeyCodePageDown:
		kc[0] = Common::KEYCODE_KP3;
		kc[1] = Common::KEYCODE_PAGEDOWN;
		return true;
	case AGS3::eAGSKeyCodeInsert:
		kc[0] = Common::KEYCODE_KP0;
		kc[1] = Common::KEYCODE_INSERT;
		return true;
	case AGS3::eAGSKeyCodeDelete:
		kc[0] = Common::KEYCODE_KP_PERIOD;
		kc[1] = Common::KEYCODE_DELETE;
		return true;

	case AGS3::eAGSKeyCodeLShift:
		kc[0] = Common::KEYCODE_LSHIFT;
		return true;
	case AGS3::eAGSKeyCodeRShift:
		kc[0] = Common::KEYCODE_RSHIFT;
		return true;
	case AGS3::eAGSKeyCodeLCtrl:
		kc[0] = Common::KEYCODE_LCTRL;
		return true;
	case AGS3::eAGSKeyCodeRCtrl:
		kc[0] = Common::KEYCODE_RCTRL;
		return true;
	case AGS3::eAGSKeyCodeLAlt:
		kc[0] = Common::KEYCODE_LALT;
		return true;
	case AGS3::eAGSKeyCodeRAlt:
		kc[0] = Common::KEYCODE_RALT;
		return true;

	default:
		return false;
	}
	return false;
}

AGS3::eAGSKeyCode EventsManager::scummvm_key_to_ags_key(const Common::Event &event) {
	if (event.type != Common::EVENT_KEYDOWN)
		return AGS3::eAGSKeyCodeNone;

	const Common::KeyCode sym = event.kbd.keycode;
	const uint16 mod = event.kbd.flags;

	// Ctrl and Alt combinations realign the letter code to certain offset
	if (sym >= Common::KEYCODE_a && sym <= Common::KEYCODE_z) {
		if ((mod & Common::KBD_CTRL) != 0) // align letters to code 1
			return static_cast<AGS3::eAGSKeyCode>(0 + (sym - Common::KEYCODE_a) + 1);
		else if ((mod & Common::KBD_ALT) != 0) // align letters to code 301
			return static_cast<AGS3::eAGSKeyCode>(AGS_EXT_KEY_SHIFT + (sym - Common::KEYCODE_a) + 1);
	}

	if (event.kbd.ascii >= 32 && event.kbd.ascii <= 127)
		return static_cast<AGS3::eAGSKeyCode>(event.kbd.ascii);

	// Remaining codes may match or not, but we use a big table anyway.
	// TODO: this is code by [sonneveld],
	// double check that we must use scan codes here, maybe can use sdl key (sym) too?
	switch (sym) {
	case Common::KEYCODE_BACKSPACE:
		return AGS3::eAGSKeyCodeBackspace;
	case Common::KEYCODE_TAB:
		return AGS3::eAGSKeyCodeTab;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		return AGS3::eAGSKeyCodeReturn;
	case Common::KEYCODE_ESCAPE:
		return AGS3::eAGSKeyCodeEscape;

	case Common::KEYCODE_F1:
		return AGS3::eAGSKeyCodeF1;
	case Common::KEYCODE_F2:
		return AGS3::eAGSKeyCodeF2;
	case Common::KEYCODE_F3:
		return AGS3::eAGSKeyCodeF3;
	case Common::KEYCODE_F4:
		return AGS3::eAGSKeyCodeF4;
	case Common::KEYCODE_F5:
		return AGS3::eAGSKeyCodeF5;
	case Common::KEYCODE_F6:
		return AGS3::eAGSKeyCodeF6;
	case Common::KEYCODE_F7:
		return AGS3::eAGSKeyCodeF7;
	case Common::KEYCODE_F8:
		return AGS3::eAGSKeyCodeF8;
	case Common::KEYCODE_F9:
		return AGS3::eAGSKeyCodeF9;
	case Common::KEYCODE_F10:
		return AGS3::eAGSKeyCodeF10;
	case Common::KEYCODE_F11:
		return AGS3::eAGSKeyCodeF11;
	case Common::KEYCODE_F12:
		return AGS3::eAGSKeyCodeF12;

	case Common::KEYCODE_KP7:
	case Common::KEYCODE_HOME:
		return AGS3::eAGSKeyCodeHome;
	case Common::KEYCODE_KP8:
	case Common::KEYCODE_UP:
		return AGS3::eAGSKeyCodeUpArrow;
	case Common::KEYCODE_KP9:
	case Common::KEYCODE_PAGEUP:
		return AGS3::eAGSKeyCodePageUp;
	case Common::KEYCODE_KP4:
	case Common::KEYCODE_LEFT:
		return AGS3::eAGSKeyCodeLeftArrow;
	case Common::KEYCODE_KP5:
		return AGS3::eAGSKeyCodeNumPad5;
	case Common::KEYCODE_KP6:
	case Common::KEYCODE_RIGHT:
		return AGS3::eAGSKeyCodeRightArrow;
	case Common::KEYCODE_KP1:
	case Common::KEYCODE_END:
		return AGS3::eAGSKeyCodeEnd;
	case Common::KEYCODE_KP2:
	case Common::KEYCODE_DOWN:
		return AGS3::eAGSKeyCodeDownArrow;
	case Common::KEYCODE_KP3:
	case Common::KEYCODE_PAGEDOWN:
		return AGS3::eAGSKeyCodePageDown;
	case Common::KEYCODE_KP0:
	case Common::KEYCODE_INSERT:
		return AGS3::eAGSKeyCodeInsert;
	case Common::KEYCODE_KP_PERIOD:
	case Common::KEYCODE_DELETE:
		return AGS3::eAGSKeyCodeDelete;

	default:
		return AGS3::eAGSKeyCodeNone;
	}

	return AGS3::eAGSKeyCodeNone;
}

} // namespace AGS
