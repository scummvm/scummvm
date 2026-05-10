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

#include "common/osd_message_queue.h"
#include "common/system.h"

#include "graphics/surface.h"

namespace Common {

DECLARE_SINGLETON(OSDMessageQueue);

OSDMessageQueue::OSDQueueEntry::OSDQueueEntry(const Common::U32String &msg)
	: _text(new Common::U32String(msg)), _image(nullptr) {}

OSDMessageQueue::OSDQueueEntry::OSDQueueEntry(const Graphics::Surface *surface)
	: _text(nullptr), _image(nullptr) {
	if (surface) {
		_image = new Graphics::Surface();
		_image->copyFrom(*surface);
	}
}

OSDMessageQueue::OSDQueueEntry::~OSDQueueEntry() {
	if (_text)
		delete _text;
	if (_image) {
		_image->free();
		delete _image;
	}
}

OSDMessageQueue::OSDMessageQueue() : _lastUpdate(0), _iconWasShown(false) {
}

OSDMessageQueue::~OSDMessageQueue() {
	g_system->getEventManager()->getEventDispatcher()->unregisterSource(this);
	_mutex.lock();
	while (!_messages.empty()) {
		OSDQueueEntry *entry = _messages.pop();
		delete entry;
	}
	_mutex.unlock();
}

void OSDMessageQueue::registerEventSource() {
	g_system->getEventManager()->getEventDispatcher()->registerSource(this, false);
}

void OSDMessageQueue::addMessage(const Common::U32String &msg) {
	_mutex.lock();
	_messages.push(new OSDQueueEntry(msg));
	_mutex.unlock();
}

void OSDMessageQueue::addImage(const Graphics::Surface *surface) {
	_mutex.lock();
	_messages.push(new OSDQueueEntry(surface));
	_mutex.unlock();
}

bool OSDMessageQueue::pollEvent(Common::Event &event) {
	_mutex.lock();

	uint t = g_system->getMillis(true);

	if (!_messages.empty()) {
		if (t - _lastUpdate >= kMinimumDelay) {
			_lastUpdate = t;

			OSDQueueEntry *entry = _messages.pop();

			if (entry->_image) {
				g_system->displayActivityIconOnOSD(entry->_image);
				_iconWasShown = true;
			} else if (entry->_text) {
				g_system->displayMessageOnOSD(*(entry->_text));
			}

			delete entry;
		}
	} else if (t - _lastUpdate >= kIconCleanupDelay) {
		// If there are no messages, but the last message was an icon that was shown, clear it after a delay
		if (_iconWasShown) {
			g_system->displayActivityIconOnOSD(nullptr);
			_iconWasShown = false;
		}
	}
	_mutex.unlock();

	return false;
}

} // End of namespace Common
