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

#include "common/events.h"

namespace Common {

bool isMouseEvent(const Event &event) {
	return event.type == EVENT_LBUTTONDOWN
	        || event.type == EVENT_LBUTTONUP
	        || event.type == EVENT_RBUTTONDOWN
	        || event.type == EVENT_RBUTTONUP
	        || event.type == EVENT_MBUTTONDOWN
	        || event.type == EVENT_MBUTTONUP
	        || event.type == EVENT_X1BUTTONDOWN
	        || event.type == EVENT_X1BUTTONUP
	        || event.type == EVENT_X2BUTTONDOWN
	        || event.type == EVENT_X2BUTTONUP
	        || event.type == EVENT_WHEELDOWN
	        || event.type == EVENT_WHEELUP
	        || event.type == EVENT_MOUSEMOVE;
}

EventSource::~EventSource() {}

EventObserver::~EventObserver() {}

EventMapper::~EventMapper() {}

EventManager::~EventManager() {}

EventDispatcher::EventDispatcher() : _mapper(nullptr) {
}

EventDispatcher::~EventDispatcher() {
	for (List<SourceEntry>::iterator i = _sources.begin(); i != _sources.end(); ++i) {
		if (i->autoFree)
			delete i->source;
	}

	for (List<ObserverEntry>::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		if (i->autoFree)
			delete i->observer;
	}
}

void EventDispatcher::dispatch() {
	Event event;

	dispatchPoll();

	for (List<SourceEntry>::iterator i = _sources.begin(); i != _sources.end(); ++i) {
		while (i->source->pollEvent(event)) {
			// We only try to process the events via the setup event mapper, when
			// we have a setup mapper and when the event source allows mapping.
			if (i->source->allowMapping()) {
				assert(_mapper);

				// Backends may not produce directly action event types, those are meant
				// to be the output of the event mapper.
				assert(event.type != EVENT_CUSTOM_BACKEND_ACTION_START);
				assert(event.type != EVENT_CUSTOM_BACKEND_ACTION_END);
				assert(event.type != EVENT_CUSTOM_ENGINE_ACTION_START);
				assert(event.type != EVENT_CUSTOM_ENGINE_ACTION_END);


				List<Event> mappedEvents = _mapper->mapEvent(event);

				for (List<Event>::iterator j = mappedEvents.begin(); j != mappedEvents.end(); ++j) {
					const Event mappedEvent = *j;
					dispatchEvent(mappedEvent);
				}
			} else {
				dispatchEvent(event);
			}
		}
	}
}

void EventDispatcher::clearEvents() {
	Event event;

	for (List<SourceEntry>::iterator i = _sources.begin(); i != _sources.end(); ++i) {
		while (i->source->pollEvent(event)) {}
	}
}


void EventDispatcher::registerMapper(EventMapper *mapper) {
	_mapper = mapper;
}


void EventDispatcher::registerSource(EventSource *source, bool autoFree) {
	SourceEntry newEntry;

	newEntry.source = source;
	newEntry.autoFree = autoFree;

	_sources.push_back(newEntry);
}

void EventDispatcher::unregisterSource(EventSource *source) {
	for (List<SourceEntry>::iterator i = _sources.begin(); i != _sources.end(); ++i) {
		if (i->source == source) {
			if (i->autoFree)
				delete source;

			_sources.erase(i);
			return;
		}
	}
}

void EventDispatcher::registerObserver(EventObserver *obs, uint priority, bool autoFree, bool notifyPoll) {
	ObserverEntry newEntry;

	newEntry.observer = obs;
	newEntry.priority = priority;
	newEntry.autoFree = autoFree;
	newEntry.poll = notifyPoll;

	for (List<ObserverEntry>::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		if (i->priority < priority) {
			_observers.insert(i, newEntry);
			return;
		}
	}

	_observers.push_back(newEntry);
}

void EventDispatcher::unregisterObserver(EventObserver *obs) {
	for (List<ObserverEntry>::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		if (i->observer == obs) {
			if (i->autoFree)
				delete obs;

			_observers.erase(i);
			return;
		}
	}
}

void EventDispatcher::dispatchEvent(const Event &event) {
	for (List<ObserverEntry>::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		if (i->observer->notifyEvent(event))
			break;
	}
}

void EventDispatcher::dispatchPoll() {
	for (List<ObserverEntry>::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		if (i->poll)
			i->observer->notifyPoll();
	}
}

} // End of namespace Common
