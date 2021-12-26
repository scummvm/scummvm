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

#include <nds.h>

#include "backends/platform/ds/keyboard.h"
#include "common/system.h"

namespace DS {

Keyboard::Keyboard(Common::EventDispatcher *eventDispatcher) :
	_eventDispatcher(eventDispatcher),
	_lastKey(NOKEY),
	_visible(false) {
	_eventDispatcher->registerSource(this, false);
}

Keyboard::~Keyboard() {
	_eventDispatcher->unregisterSource(this);
}

void Keyboard::init(int layer, int mapBase, int tileBase, bool mainDisplay) {
	keyboardInit(nullptr, layer, BgType_Text4bpp, BgSize_T_256x512, mapBase, tileBase, mainDisplay, true);
}

void Keyboard::show() {
	keyboardShow();
	_visible = true;
}

void Keyboard::hide() {
	keyboardHide();
	_visible = false;
}

bool Keyboard::mapKey(int key, Common::KeyState &ks) {
	switch (key) {
	case DVK_BACKSPACE:
		ks = Common::KeyState(Common::KEYCODE_BACKSPACE, Common::ASCII_BACKSPACE);
		break;
	case DVK_TAB:
		ks = Common::KeyState(Common::KEYCODE_TAB, Common::ASCII_TAB);
		break;
	case DVK_ENTER:
		ks = Common::KeyState(Common::KEYCODE_RETURN, Common::ASCII_RETURN);
		break;
	case DVK_SPACE:
		ks = Common::KeyState(Common::KEYCODE_SPACE, Common::ASCII_SPACE);
		break;
	case DVK_MENU:
		ks = Common::KeyState(Common::KEYCODE_MENU);
		break;
	case DVK_SHIFT:
		ks = Common::KeyState(Common::KEYCODE_LSHIFT);
		break;
	case DVK_CAPS:
		ks = Common::KeyState(Common::KEYCODE_CAPSLOCK);
		break;
	case DVK_CTRL:
		ks = Common::KeyState(Common::KEYCODE_LCTRL);
		break;
	case DVK_UP:
		ks = Common::KeyState(Common::KEYCODE_UP);
		break;
	case DVK_RIGHT:
		ks = Common::KeyState(Common::KEYCODE_RIGHT);
		break;
	case DVK_DOWN:
		ks = Common::KeyState(Common::KEYCODE_DOWN);
		break;
	case DVK_LEFT:
		ks = Common::KeyState(Common::KEYCODE_LEFT);
		break;
	case DVK_FOLD:
		ks = Common::KeyState(Common::KEYCODE_ESCAPE, Common::ASCII_ESCAPE);
		break;
	case DVK_ALT:
		ks = Common::KeyState(Common::KEYCODE_LALT);
		break;
	default:
		if (key < Common::KEYCODE_SPACE || key > Common::KEYCODE_z)
			return false;
		ks = Common::KeyState((Common::KeyCode)tolower(key), key);
		break;
	}

	return true;
}

bool Keyboard::pollEvent(Common::Event &event) {
	if (!_visible)
		return false;

	int key = keyboardUpdate();
	if (key == _lastKey)
		return false;

	if (key == NOKEY) {
		event.type = Common::EVENT_KEYUP;
		if (!mapKey(_lastKey, event.kbd))
			return false;
	} else {
		event.type = Common::EVENT_KEYDOWN;
		if (!mapKey(key, event.kbd))
			return false;
	}

	_lastKey = key;

	// TODO: Handle flags

	return true;
}

} // End of namespace DS
