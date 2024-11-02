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


#include "freescape/freescape.h"

namespace Freescape {

EventManagerWrapper::EventManagerWrapper(Common::EventManager *delegate) :
		_delegate(delegate),
		_keyRepeatTime(0),
		_currentActionDown(kActionNone) {
	assert(delegate);
}

bool EventManagerWrapper::pollEvent(Common::Event &event) {
	uint32 time = g_system->getMillis(true);
	bool gotEvent = _delegate->pollEvent(event);

	if (gotEvent) {
		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (event.customType == kActionEscape)
				break;
			_currentActionDown = event.customType;
			_keyRepeatTime = time + kKeyRepeatInitialDelay;
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			if (event.customType == kActionEscape)
				break;
			if (event.customType == _currentActionDown) {
				// Only stop firing events if it's the current key
				_currentActionDown = kActionNone;
			}
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd == Common::KEYCODE_ESCAPE || event.kbd == Common::KEYCODE_F5)
				break;

			// init continuous event stream
			_currentKeyDown = event.kbd;
			_keyRepeatTime = time + kKeyRepeatInitialDelay;
			break;

		case Common::EVENT_KEYUP:
			if (event.kbd == Common::KEYCODE_ESCAPE || event.kbd == Common::KEYCODE_F5)
				break;

			if (event.kbd.keycode == _currentKeyDown.keycode) {
				// Only stop firing events if it's the current key
				_currentKeyDown.keycode = Common::KEYCODE_INVALID;
			}
			break;

		default:
			break;
		}

		return true;
	} else {
		// Check if event should be sent again (keydown)
		if (_currentKeyDown.keycode != Common::KEYCODE_INVALID && _keyRepeatTime <= time) {
			// fire event
			event.type = Common::EVENT_KEYDOWN;
			event.kbdRepeat = true;
			event.kbd = _currentKeyDown;
			_keyRepeatTime = time + kKeyRepeatSustainDelay;
			return true;
		}
		if (_currentActionDown != kActionNone && _keyRepeatTime <= time) {
			event.type = Common::EVENT_CUSTOM_ENGINE_ACTION_START;
			event.kbdRepeat = true;
			event.customType = _currentActionDown;
			_keyRepeatTime = time + kKeyRepeatSustainDelay;
			return true;

		}

		return false;
	}
}

void EventManagerWrapper::purgeKeyboardEvents() {
	_delegate->purgeKeyboardEvents();
	_currentKeyDown.keycode = Common::KEYCODE_INVALID;
	_currentActionDown = kActionNone;
}

void EventManagerWrapper::purgeMouseEvents() {
	_delegate->purgeMouseEvents();
}

void EventManagerWrapper::pushEvent(Common::Event &event) {
	_delegate->pushEvent(event);
}

void EventManagerWrapper::clearExitEvents() {
	_delegate->resetQuit();
	//_delegate->resetReturnToLauncher();

}

} // namespace Freescape
