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

#include "common/osd_message_queue.h"
#include "common/system.h"

namespace Common {

DECLARE_SINGLETON(OSDMessageQueue);

OSDMessageQueue::OSDMessageQueue() : _lastUpdate(0) {
}

OSDMessageQueue::~OSDMessageQueue() {
	g_system->getEventManager()->getEventDispatcher()->unregisterSource(this);
}

void OSDMessageQueue::registerEventSource() {
	g_system->getEventManager()->getEventDispatcher()->registerSource(this, false);
}

void OSDMessageQueue::addMessage(const Common::U32String &msg) {
	_mutex.lock();
	_messages.push(msg);
	_mutex.unlock();
}

bool OSDMessageQueue::pollEvent(Common::Event &event) {
	_mutex.lock();
	if (!_messages.empty()) {
		uint t = g_system->getMillis();
		if (t - _lastUpdate >= kMinimumDelay) {
			_lastUpdate = t;
			Common::U32String msg = _messages.pop();
			g_system->displayMessageOnOSD(msg);
		}
	}
	_mutex.unlock();

	return false;
}

} // End of namespace Common
