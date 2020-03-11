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

#include "backends/keymapper/input-watcher.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"

namespace Common {

InputWatcher::InputWatcher(EventDispatcher *eventDispatcher, Keymapper *keymapper) :
		_eventDispatcher(eventDispatcher),
		_keymapper(keymapper),
		_watching(false) {

}

void InputWatcher::startWatching() {
	assert(!_watching);
	assert(_hwInput.type == kHardwareInputTypeInvalid);

	_keymapper->setEnabled(false);
	_eventDispatcher->registerObserver(this, EventManager::kEventRemapperPriority, false);
	_watching = true;
}

void InputWatcher::stopWatching() {
	_keymapper->setEnabled(true);
	_eventDispatcher->unregisterObserver(this);
	_watching = false;
}

bool InputWatcher::isWatching() const {
	return _watching;
}

bool InputWatcher::notifyEvent(const Event &event) {
	assert(_watching);
	assert(_hwInput.type == kHardwareInputTypeInvalid);

	switch (event.type) {
		case EVENT_KEYDOWN:
		case EVENT_JOYBUTTON_DOWN:
		case EVENT_LBUTTONDOWN:
		case EVENT_RBUTTONDOWN:
		case EVENT_MBUTTONDOWN:
		case EVENT_X1BUTTONDOWN:
		case EVENT_X2BUTTONDOWN:
			return true;
		case EVENT_KEYUP:
		case EVENT_JOYBUTTON_UP:
		case EVENT_JOYAXIS_MOTION:
		case EVENT_LBUTTONUP:
		case EVENT_RBUTTONUP:
		case EVENT_MBUTTONUP:
		case EVENT_WHEELUP:
		case EVENT_WHEELDOWN:
		case EVENT_X1BUTTONUP:
		case EVENT_X2BUTTONUP:
		case EVENT_CUSTOM_BACKEND_HARDWARE:
			_hwInput = _keymapper->findHardwareInput(event);
			if (_hwInput.type != kHardwareInputTypeInvalid) {
				stopWatching();
			}
			return true;
		default:
			break;
	}

	return false;
}

HardwareInput InputWatcher::checkForCapturedInput() {
	HardwareInput hwInput = _hwInput;
	_hwInput = HardwareInput();
	return hwInput;
}

} // End of namespace Common
