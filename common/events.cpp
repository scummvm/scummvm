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

#include "common/system.h"

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

class KeyboardRepeatEventSourceWrapper : public Common::EventSource {
public:
	KeyboardRepeatEventSourceWrapper(Common::EventSource *delegate) :
			_delegate(delegate),
			_keyRepeatTime(0) {
		assert(delegate);
	}

	// EventSource API
	bool pollEvent(Common::Event &event) override {
		uint32 time = g_system->getMillis(true);
		bool gotEvent = _delegate->pollEvent(event);

		if (gotEvent) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				// init continuous event stream
				_currentKeyDown = event.kbd;
				_keyRepeatTime = time + kKeyRepeatInitialDelay;
				break;

			case Common::EVENT_KEYUP:
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

			return false;
		}
	}

	bool allowMapping() const override {
		return _delegate->allowMapping();
	}

private:
	// for continuous events (keyDown)
	enum {
		kKeyRepeatInitialDelay = 400,
		kKeyRepeatSustainDelay = 100
	};

	Common::EventSource *_delegate;

	Common::KeyState _currentKeyDown;
	uint32 _keyRepeatTime;
};

EventSource *makeKeyboardRepeatingEventSource(EventSource *eventSource) {
	if (!eventSource) {
		return nullptr;
	}

	return new KeyboardRepeatEventSourceWrapper(eventSource);
}

} // End of namespace Common
