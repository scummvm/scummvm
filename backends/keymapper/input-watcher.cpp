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

#ifdef ENABLE_KEYMAPPER

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"

namespace Common {

InputWatcher::InputWatcher(EventDispatcher *eventDispatcher, Keymapper *keymapper) :
		_eventDispatcher(eventDispatcher),
		_keymapper(keymapper),
		_watching(false),
		_hwInput(nullptr) {

}

void InputWatcher::startWatching() {
	assert(!_watching);
	assert(!_hwInput);

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
	assert(!_hwInput);

	switch (event.type) {
		case EVENT_KEYDOWN:
			return true;
		case EVENT_KEYUP:
		case EVENT_CUSTOM_BACKEND_HARDWARE:
			_hwInput = _keymapper->findHardwareInput(event);
			if (_hwInput) {
				stopWatching();
			}
			return true;
		default:
			break;
	}

	return false;
}

const HardwareInput *InputWatcher::checkForCapturedInput() {
	const HardwareInput *hwInput = _hwInput;
	_hwInput = nullptr;
	return hwInput;
}

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
