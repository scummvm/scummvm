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

#include "ultima/ultima4/events/timed_event_mgr.h"
#include "ultima/ultima4/gfx/screen.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

TimedEventMgr::TimedEventMgr(int baseInterval) :
		_baseInterval(baseInterval), _lastTickTime(0), _locked(false) {
}

void TimedEventMgr::poll() {
	uint32 time = g_system->getMillis();
	if (time >= (_lastTickTime + _baseInterval)) {
		_lastTickTime = time;
		tick();

		g_screen->update();
	}
}

void TimedEventMgr::reset(uint interval) {
	_baseInterval = interval;
}

/*-------------------------------------------------------------------*/

TimedEvent::TimedEvent(TimedEvent::Callback cb, int i, void *d) :
	_callback(cb),
	_data(d),
	_interval(i),
	_current(0) {
}

TimedEvent::Callback TimedEvent::getCallback() const {
	return _callback;
}

void *TimedEvent::getData() {
	return _data;
}

void TimedEvent::tick() {
	if (++_current >= _interval) {
		(*_callback)(_data);
		_current = 0;
	}
}

bool TimedEventMgr::isLocked() const {
	return _locked;
}

void TimedEventMgr::add(TimedEvent::Callback theCallback, int interval, void *data) {
	_events.push_back(new TimedEvent(theCallback, interval, data));
}

TimedEventMgr::List::iterator TimedEventMgr::remove(List::iterator i) {
	if (isLocked()) {
		_deferredRemovals.push_back(*i);
		return i;
	} else {
		delete *i;
		return _events.erase(i);
	}
}

void TimedEventMgr::remove(TimedEvent *event) {
	List::iterator i;
	for (i = _events.begin(); i != _events.end(); i++) {
		if ((*i) == event) {
			remove(i);
			break;
		}
	}
}

void TimedEventMgr::remove(TimedEvent::Callback theCallback, void *data) {
	List::iterator i;
	for (i = _events.begin(); i != _events.end(); i++) {
		if ((*i)->getCallback() == theCallback && (*i)->getData() == data) {
			remove(i);
			break;
		}
	}
}

void TimedEventMgr::tick() {
	List::iterator i;
	lock();

	for (i = _events.begin(); i != _events.end(); i++)
		(*i)->tick();

	unlock();

	// Remove events that have been deferred for removal
	for (i = _deferredRemovals.begin(); i != _deferredRemovals.end(); i++)
		_events.remove(*i);
}

void TimedEventMgr::lock() {
	_locked = true;
}

void TimedEventMgr::unlock() {
	_locked = false;
}


} // End of namespace Ultima4
} // End of namespace Ultima
