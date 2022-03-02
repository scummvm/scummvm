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
#include "chewy/timer.h"

namespace Chewy {

EventsManager *g_events;

EventsManager::EventsManager(Graphics::Screen *screen) : EventsBase(screen) {
	g_events = this;
	init_timer_handler();
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

void EventsManager::handleEvent(const Common::Event &event, bool updateOnButtonUp) {
	if (event.type >= Common::EVENT_MOUSEMOVE && event.type <= Common::EVENT_MBUTTONUP)
		handleMouseEvent(event, updateOnButtonUp);
	else if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP)
		handleKbdEvent(event);
}

void EventsManager::handleMouseEvent(const Common::Event &event, bool updateOnButtonUp) {
	_mousePos = event.mouse;
	bool isWheelEnabled = !_G(menu_display) && !_G(flags).InventMenu &&
		g_engine->canSaveAutosaveCurrently() &&
		_G(menu_item) >= CUR_WALK && _G(menu_item) <= CUR_TALK;

	// Different event types handling
	if (event.type != Common::EVENT_MOUSEMOVE)
		_G(minfo)._button = 0;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
		if (updateOnButtonUp && event.type == Common::EVENT_LBUTTONUP)
			_G(minfo)._button = 1;
		if (!updateOnButtonUp && event.type == Common::EVENT_LBUTTONDOWN)
			_G(minfo)._button = 1;
		break;

	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		if (updateOnButtonUp && event.type == Common::EVENT_RBUTTONUP)
			_G(minfo)._button = 2;
		if (!updateOnButtonUp && event.type == Common::EVENT_RBUTTONDOWN)
			_G(minfo)._button = 2;
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
	if (!_cursorMoveFl) {
		_cursorMoveFl = true;
		_G(minfo).x = event.mouse.x;
		_G(minfo).y = event.mouse.y;
	}
}

void EventsManager::handleKbdEvent(const Common::Event &event) {
	if (_kbInfo) {
		if (event.type == Common::EVENT_KEYDOWN) {
			_kbInfo->_keyCode = event.kbd.ascii;
			_kbInfo->scan_code = event.kbd.keycode;
			if (event.kbd.flags & Common::KBD_ALT)
				_kbInfo->scan_code |= ALT;
		} else if (event.type == Common::EVENT_KEYUP) {
			if (event.kbd.ascii == _kbInfo->_keyCode)
				_kbInfo->_keyCode = '\0';
		}
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
	if (_kbInfo) {
		_kbInfo->scan_code = Common::KEYCODE_INVALID;
		_kbInfo->_keyCode = '\0';
	}

	_G(minfo)._button = 0;
}

KbdInfo *EventsManager::setKbdInfo(KbdInfo *kbInfo) {
	KbdInfo *kb = _kbInfo;
	_kbInfo = kbInfo;
	return kb;
}

void EventsManager::setMousePos(const Common::Point &pt) {
	g_system->warpMouse(pt.x, pt.y);
	_mousePos = pt;
}

void delay(size_t time) {
	g_events->delay(time);
}

bool kbhit() {
	return g_events->_kbInfo && g_events->_kbInfo->_keyCode != 0;
}

char getch() {
	if (g_events->_kbInfo && g_events->_kbInfo->_keyCode) {
		char c = g_events->_kbInfo->_keyCode;
		g_events->_kbInfo->_keyCode = 0;
		g_events->_kbInfo->scan_code = 0;

		return c;
	}

	Common::Event e;
	while (!SHOULD_QUIT) {
		g_system->delayMillis(10);
		g_events->update();

		while (!g_system->getEventManager()->pollEvent(e) && !SHOULD_QUIT) {
			if (e.type == Common::EVENT_KEYDOWN)
				return e.kbd.ascii;
		}
	}

	return 0;
}

void putch(char c) {
	warning("STUB: putch()");
}

} // namespace Chewy
