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
 */

#include "startrek/console.h"
#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::pollEvents(bool queueEvents) {
	Common::Event event;
	TrekEvent trekEvent;
	while (_eventMan->pollEvent(event)) {
		trekEvent.mouse = event.mouse;
		trekEvent.kbd = event.kbd;

		switch (event.type) {
		case Common::EVENT_QUIT:
			_system->quit();
			break;

		case Common::EVENT_MOUSEMOVE:
			if (queueEvents) {
				trekEvent.type = TREKEVENT_MOUSEMOVE;
				addEventToQueue(trekEvent);
			}

			// WORKAROUND: this improves the responsiveness of the mouse.
			_system->updateScreen();
			_system->delayMillis(10);
			break;

		case Common::EVENT_LBUTTONDOWN:
			if (queueEvents) {
				trekEvent.type = TREKEVENT_LBUTTONDOWN;
				addEventToQueue(trekEvent);
			}
			break;

		case Common::EVENT_RBUTTONDOWN:
			if (queueEvents) {
				trekEvent.type = TREKEVENT_RBUTTONDOWN;
				addEventToQueue(trekEvent);
			}
			break;

		case Common::EVENT_KEYDOWN:
			if (queueEvents) {
				trekEvent.type = TREKEVENT_KEYDOWN;
				addEventToQueue(trekEvent);
			}
			break;

		default:
			break;
		}
	}
}

void StarTrekEngine::waitForNextTick(bool queueEvents) {
	pollEvents(queueEvents);

	uint nextFrame = _frameStartMillis + 1000 / 18.206;
	uint millis = _system->getMillis();

	if (millis < nextFrame)
		_system->delayMillis(nextFrame - millis);

	_clockTicks++;
	_frameStartMillis = nextFrame;

	if (queueEvents) {
		TrekEvent tickEvent;
		tickEvent.type = TREKEVENT_TICK;
		tickEvent.tick = _clockTicks;
		addEventToQueue(tickEvent);
	}
}

void StarTrekEngine::initializeEventsAndMouse() {
	_mouseMoveEventInQueue = false;
	_tickEventInQueue = false;
	_frameStartMillis = _system->getMillis();

	// TODO: mouse
}

bool StarTrekEngine::getNextEvent(TrekEvent *e, bool poll) {
	while (poll && _eventQueue.empty()) {
		pollEvents(true);

		// Check for tick event
		uint nextFrame = _frameStartMillis + 1000 / 18.206;
		uint millis = _system->getMillis();

		if (millis >= nextFrame) {
			_clockTicks++;
			_frameStartMillis = millis;

			TrekEvent tickEvent;
			tickEvent.type = TREKEVENT_TICK;
			tickEvent.tick = _clockTicks;
			addEventToQueue(tickEvent);
		}

		if (!_eventQueue.empty())
			break;

		// Still no events; wait a 60th of a second before checking for events again
		int delay = 1000 / 60;
		millis = _system->getMillis();
		if (millis + delay > nextFrame)
			delay = nextFrame - millis;
		if (delay > 0)
			_system->delayMillis(delay);
	}

	if (_eventQueue.empty())
		return false;
	*e = _eventQueue.front();
	return true;
}

void StarTrekEngine::removeNextEvent() {
	if (_eventQueue.empty())
		return;

	const TrekEvent &e = _eventQueue.front();

	if (e.type == TREKEVENT_MOUSEMOVE)
		_mouseMoveEventInQueue = false;
	if (e.type == TREKEVENT_TICK)
		_tickEventInQueue = false;

	_eventQueue.pop_front();
}

bool StarTrekEngine::popNextEvent(TrekEvent *e, bool poll) {
	if (!getNextEvent(e, poll))
		return false;

	removeNextEvent();
	return true;
}

void StarTrekEngine::addEventToQueue(const TrekEvent &e) {
	if (e.type == TREKEVENT_MOUSEMOVE && _mouseMoveEventInQueue) {
		// Only allow one mouse move event at once
		for (Common::List<TrekEvent>::iterator i = _eventQueue.begin(); i != _eventQueue.end(); ++i) {
			if (i->type == TREKEVENT_MOUSEMOVE) {
				*i = e;
				return;
			}
		}

		error("Couldn't find mouse move event in eventQueue");
	}

	if (e.type == TREKEVENT_TICK) {
		// Only allow one tick event at once
		if (_tickEventInQueue)
			return;
		_tickEventInQueue = true;
	}

	if (e.type == TREKEVENT_MOUSEMOVE)
		_mouseMoveEventInQueue = true;

	assert(_eventQueue.size() < 0x40);
	_eventQueue.push_back(e);
}

} // End of namespace StarTrek
