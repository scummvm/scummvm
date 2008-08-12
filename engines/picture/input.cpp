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
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/input.h"
#include "picture/palette.h"
#include "picture/resource.h"

namespace Picture {

Input::Input(PictureEngine *vm) : _vm(vm) {

	_mouseX = 0;
	_mouseY = 0;
	_mousePosDelta = 0;
	_mouseCounter = 0;
	_mouseButtonPressedFlag = false;
	_mouseButton = 0;
	_mouseDisabled = 0;

	_leftButtonDown = false;
	_rightButtonDown = false;

}

Input::~Input() {
}

void Input::update() {

	Common::Event event;
	Common::EventManager *eventMan = _vm->_system->getEventManager();
	while (eventMan->pollEvent(event)) {
	switch (event.type) {
		case Common::EVENT_KEYDOWN:

			// FIXME: This is just for debugging
			switch (event.kbd.keycode) {
			case Common::KEYCODE_F6:
				_vm->savegame("toltecs.001");
				break;
			case Common::KEYCODE_F9:
				_vm->loadgame("toltecs.001");
				break;
			default:
				break;
			}

			break;
		case Common::EVENT_QUIT:
			// FIXME: Find a nicer way to quit
			g_system->quit();
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_leftButtonDown = true;
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_leftButtonDown = false;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_rightButtonDown = true;
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_rightButtonDown = false;
			break;
		default:
			break;
		}
	}
	
	if (_mouseDisabled == 0) {
		_mousePosDelta = _mousePosDelta + _mouseY - _mouseX;
		
		if (_mouseCounter > 0)
			_mouseCounter--;

		byte mouseButtons = 0;
		if (_leftButtonDown)
			mouseButtons |= 1;
		if (_rightButtonDown)
			mouseButtons |= 2;

		if (mouseButtons != 0) {
			if (!_mouseButtonPressedFlag) {
				_mouseButton = mouseButtons;
				if (_mouseCounter != 0)
					_mouseButton |= 0x80;
				_mouseCounter = 30; // maybe TODO
				_mouseButtonPressedFlag = true;
			} else {
				_mouseButton = 0;
			}
		} else {
			_mouseButtonPressedFlag = false;
			_mouseButton = 0;
		}

	}

}

int16 Input::getMouseDeltaStuff(int16 divisor) {
	return ABS(_mousePosDelta % divisor);
}

} // End of namespace Picture
