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

namespace Ultima {
namespace Ultima4 {

unsigned int TimedEventMgr::_instances = 0;

TimedEventMgr::TimedEventMgr(int i) : _baseInterval(i), _locked(false) {
	/* start the SDL timer */
#ifdef TODO
	if (instances == 0) {
		if (u4_SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
			errorFatal("unable to init SDL: %s", SDL_GetError());
	}

	id = static_cast<void *>(SDL_AddTimer(i, &TimedEventMgr::callback, this));
#endif
	_instances++;
}

TimedEventMgr::~TimedEventMgr() {
#ifdef TODO
	SDL_RemoveTimer(static_cast<SDL_TimerID>(id));
	id = NULL;

	if (instances == 1)
		u4_SDL_QuitSubSystem(SDL_INIT_TIMER);
#endif
	if (_instances > 0)
		_instances--;
}

unsigned int TimedEventMgr::callback(unsigned int interval, void *param) {
#ifdef TODO
	Common::Event event;

	event.type = SDL_USEREVENT;
	event.user.code = 0;
	event.user.data1 = param;
	event.user.data2 = NULL;
	SDL_PushEvent(&event);
#endif
	return interval;
}

void TimedEventMgr::reset(unsigned int interval) {
	_baseInterval = interval;
	stop();
	start();
}

void TimedEventMgr::stop() {
#ifdef TODO
	if (id) {
		SDL_RemoveTimer(static_cast<SDL_TimerID>(id));
		id = NULL;
	}
#endif
}

void TimedEventMgr::start() {
#ifdef TODO
	if (!id)
		id = static_cast<void *>(SDL_AddTimer(baseInterval, &TimedEventMgr::callback, this));
#endif
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
