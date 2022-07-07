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
#include "chewy/events.h"
#include "chewy/globals.h"

namespace Chewy {

EventsManager *g_events;

EventsManager::EventsManager(Graphics::Screen *screen, uint refreshRate) : _screen(screen) {
	addTimer(updateScreen, refreshRate);
	g_events = this;
	init_timer_handler();
	_kbInfo._scanCode = Common::KEYCODE_INVALID;
}

EventsManager::~EventsManager() {
	g_events = nullptr;
}

void EventsManager::init_timer_handler() {
	_G(timer_int) = true;
	_G(timer_count) = 0;

	addTimer(timer_handler, (uint32)(1000 / 18.2));
}

void EventsManager::timer_handler() {
	if (!_G(timer_suspend))
		++_G(timer_count);
}

void EventsManager::checkTimers() {
	uint32 currTime = g_system->getMillis();

	for (TimerList::iterator it = _timers.begin(); it != _timers.end(); ++it) {
		TimerRecord &rec = *it;
		if (currTime >= rec._nextFrameTime) {
			rec._proc();
			rec._nextFrameTime = currTime + rec._interval;
		}
	}
}

void EventsManager::updateScreen() {
	if (g_events->_screen)
		g_events->_screen->update();
	else
		g_system->updateScreen();
}

void EventsManager::handleEvent(const Common::Event &event) {
	if (event.type >= Common::EVENT_MOUSEMOVE && event.type <= Common::EVENT_MBUTTONUP)
		handleMouseEvent(event);
	else if (event.type == Common::EVENT_KEYUP)
		handleKbdEvent(event);
}

void EventsManager::handleMouseEvent(const Common::Event &event) {
	_pendingEvents.push(event);

	_mousePos = event.mouse;
	bool isWheelEnabled = !_G(menu_display) && !_G(flags).InventMenu &&
		g_engine->canSaveAutosaveCurrently() &&
		_G(menu_item) >= CUR_WALK && _G(menu_item) <= CUR_TALK;

	// Different event types handling
	if (event.type != Common::EVENT_MOUSEMOVE)
		_G(minfo).button = 0;

	switch (event.type) {
	case Common::EVENT_LBUTTONUP:
		_G(minfo).button = 1;
		break;

	case Common::EVENT_RBUTTONUP:
		_G(minfo).button = 2;
		break;

	case Common::EVENT_WHEELUP:
		// Cycle backwards through cursors
		if (isWheelEnabled) {
			if (--_G(menu_item) < 0)
				_G(menu_item) = CUR_TALK;
			cursorChoice(_G(menu_item));
		}
		return;

	case Common::EVENT_WHEELDOWN:
		// Cycle forwards through cursors
		if (isWheelEnabled) {
			if (++_G(menu_item) > CUR_TALK)
				_G(menu_item) = CUR_WALK;
			cursorChoice(_G(menu_item));
		}
		return;

	case Common::EVENT_MBUTTONDOWN:
		// Toggle between walk and look cursor
		if (isWheelEnabled) {
			_G(menu_item) = (_G(menu_item) == CUR_WALK) ? CUR_LOOK : CUR_WALK;
			cursorChoice(_G(menu_item));
		}
		return;

	default:
		break;
	}

	// Set mouse position
	g_events->_mousePos.x = event.mouse.x;
	g_events->_mousePos.y = event.mouse.y;
}

void EventsManager::handleKbdEvent(const Common::Event &event) {
	_pendingKeyEvents.push(event);

	if (event.type == Common::EVENT_KEYUP) {
		_kbInfo._keyCode = event.kbd.ascii;
		_kbInfo._scanCode = event.kbd.keycode;
		if (event.kbd.flags & Common::KBD_ALT)
			_kbInfo._scanCode |= ALT;
	}
}

void EventsManager::delay(size_t time) {
	uint32 expiryTime = g_system->getMillis() + time;
	uint32 currTime;

	do {
		update();
		currTime = g_system->getMillis();
	} while (currTime < expiryTime);
}

void EventsManager::clearEvents() {
	processEvents();
	_pendingEvents.clear();
	_pendingKeyEvents.clear();

	_kbInfo._scanCode = Common::KEYCODE_INVALID;
	_kbInfo._keyCode = '\0';
	_G(minfo).button = 0;
}

void EventsManager::update() {
	// Brief pause to prevent 100% CPU usage
	g_system->delayMillis(10);

	// Check for any timers that have to be triggered
	checkTimers();

	// Process events
	processEvents();
}

#define MOUSE_MOVE                                 \
	if (moveEvent.type != Common::EVENT_INVALID) { \
		handleEvent(moveEvent);                    \
		moveEvent.type = Common::EVENT_INVALID;    \
	}

void EventsManager::processEvents() {
	Common::Event e;
	Common::Event moveEvent;

	while (g_system->getEventManager()->pollEvent(e)) {
		switch (e.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;

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
				return;
			}
			break;
		}
	}

	MOUSE_MOVE;
}

#undef MOUSE_MOVE

void EventsManager::warpMouse(const Common::Point &newPos) {
	_mousePos = newPos;
	g_system->warpMouse(newPos.x, newPos.y);
}

int16 EventsManager::getSwitchCode() {
	int16 switch_code = 0;

	if (_G(minfo).button == 2)
		switch_code = Common::KEYCODE_ESCAPE;
	else if (_G(minfo).button == 1)
		switch_code = Common::MOUSE_BUTTON_LEFT;
	else if (_G(minfo).button == 4)
		switch_code = Common::MOUSE_BUTTON_MIDDLE;

	if (g_events->_kbInfo._keyCode != 0)
		switch_code = (int16)g_events->_kbInfo._keyCode;

	// Virtual key, set when an item is taken from the inventory
	if (_hotkey != Common::KEYCODE_INVALID) {
		switch_code = _hotkey;
		_hotkey = Common::KEYCODE_INVALID;
	}

	return switch_code;
}

void delay(size_t time) {
	g_events->delay(time);
}

bool kbhit() {
	return g_events->_kbInfo._keyCode != 0;
}

char getch() {
	if (g_events->_kbInfo._keyCode) {
		char c = g_events->_kbInfo._keyCode;
		g_events->_kbInfo._keyCode = 0;
		g_events->_kbInfo._scanCode = 0;

		return c;
	}

	Common::Event e;
	while (!SHOULD_QUIT) {
		g_system->delayMillis(10);
		g_events->update();

		while (!g_system->getEventManager()->pollEvent(e) && !SHOULD_QUIT) {
			if (e.type == Common::EVENT_KEYUP)
				return e.kbd.ascii;
		}
	}

	return 0;
}

} // namespace Chewy
