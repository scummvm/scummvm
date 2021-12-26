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

#include "common/system.h"
#include "chewy/events_base.h"

namespace Chewy {

EventsBase *g_eventsBase;

EventsBase::EventsBase(Graphics::Screen *screen, uint refreshRate) :
		_screen(screen) {
	g_eventsBase = this;
	addTimer(updateScreen, refreshRate);
}

EventsBase::EventsBase(uint refreshRate) : _screen(nullptr) {
	g_eventsBase = this;
	addTimer(updateScreen, refreshRate);
}

EventsBase::~EventsBase() {
	g_eventsBase = nullptr;
}

void EventsBase::checkTimers() {
	uint32 currTime = g_system->getMillis();

	for (TimerList::iterator it = _timers.begin(); it != _timers.end(); ++it) {
		TimerRecord &rec = *it;
		if (currTime >= rec._nextFrameTime) {
			rec._proc();
			rec._nextFrameTime = currTime + rec._interval;
		}
	}
}

void EventsBase::updateScreen() {
	if (g_eventsBase->_screen)
		g_eventsBase->_screen->update();
	else
		g_system->updateScreen();
}

void EventsBase::update() {
	// Brief pause to prevent 100% CPU usage
	g_system->delayMillis(10);

	// Check for any timers that have to be triggered
	checkTimers();

	// Process events
	processEvents();
}

#define MOUSE_MOVE \
	if (moveEvent.type != Common::EVENT_INVALID) { \
		handleEvent(moveEvent); \
		moveEvent.type = Common::EVENT_INVALID; \
	}

void EventsBase::processEvents() {
	Common::Event e;
	Common::Event moveEvent;

	while (g_system->getEventManager()->pollEvent(e)) {
		switch (e.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;

		case Common::EVENT_KEYDOWN:
		case Common::EVENT_KEYUP:
			MOUSE_MOVE;
			handleEvent(e);
			break;

		default:
			if (e.type == Common::EVENT_MOUSEMOVE) {
				// Mouse move events get cached so the engine isn't
				// spammed with multiple sequential move events
				moveEvent = e;
			} else {
				MOUSE_MOVE;
				handleEvent(e);
			}
			break;
		}
	}

	MOUSE_MOVE;
}

#undef MOUSE_MOVE

void EventsBase::handleEvent(const Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN) {
		_pendingKeyEvents.push(event);
	} else {
		_pendingEvents.push(event);
	}
}

void EventsBase::warpMouse(const Common::Point &newPos) {
	g_system->warpMouse(newPos.x, newPos.y);
}

void EventsBase::clearEvents() {
	processEvents();
	_pendingEvents.clear();
	_pendingKeyEvents.clear();
}

} // namespace Chewy
