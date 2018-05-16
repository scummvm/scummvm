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

#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::pollSystemEvents() {
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
			trekEvent.type = TREKEVENT_MOUSEMOVE;
			addEventToQueue(trekEvent);
			break;

		case Common::EVENT_LBUTTONDOWN:
			// TODO: what happens when mouse click is outside normal screen bounds?
			// (apparently this can happen)
			trekEvent.type = TREKEVENT_LBUTTONDOWN;
			addEventToQueue(trekEvent);
			break;

		case Common::EVENT_RBUTTONDOWN:
			trekEvent.type = TREKEVENT_RBUTTONDOWN;
			addEventToQueue(trekEvent);
			break;

		case Common::EVENT_KEYDOWN:
			trekEvent.type = TREKEVENT_KEYDOWN;
			addEventToQueue(trekEvent);
			break;

		default:
			break;
		}
	}

	// TODO: check for events other than "tick" more often.
	if (_eventQueue.empty()) {
		int delay = 1000/18.206 - (_system->getMillis() - _frameStartMillis);

		_clockTicks++;
		if (delay < 0)
			debug(5, "Late frame");
		while (delay < 0) { // Check if we're behind...
			delay += 1000/18.206;
			_clockTicks++;
		}
		_system->delayMillis(delay);

		_frameStartMillis = _system->getMillis();

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

/**
 * Returns false if there is no event waiting.
 */
bool StarTrekEngine::getNextEvent(TrekEvent *e) {
	pollSystemEvents(); // TODO: put this somewhere else?

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

bool StarTrekEngine::popNextEvent(TrekEvent *e) {
	if (!getNextEvent(e))
		return false;

	removeNextEvent();
	return true;
}

void StarTrekEngine::addEventToQueue(const TrekEvent &e) {
	if (e.type == TREKEVENT_MOUSEMOVE && _mouseMoveEventInQueue) {
		// Only allow one mouse move event at once
		for (Common::List<TrekEvent>::iterator i = _eventQueue.begin(); i!=_eventQueue.end(); i++) {
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

}
