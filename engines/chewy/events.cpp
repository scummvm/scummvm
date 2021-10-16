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

#include "common/system.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/timer.h"

namespace Chewy {

EventsManager *g_events;

EventsManager::EventsManager(Graphics::Screen *screen) :
		EventsBase(screen) {
	g_events = this;
	init_timer_handler();
}

EventsManager::~EventsManager() {
	g_events = nullptr;
}


void EventsManager::init_timer_handler() {
	timer_int = true;
	timer_flag = false;
	timer_count = 0;

	addTimer(timer_handler, (uint32)(1000 / 18.2));
}

void EventsManager::timer_handler() {
	if (timer_flag == false) {
		timer_flag = true;
		if (!timer_suspend)
			++timer_count;
		timer_flag = false;
	}
}

void EventsManager::handleEvent(const Common::Event &event) {
	if (event.type >= Common::EVENT_MOUSEMOVE &&
		event.type <= Common::EVENT_MBUTTONUP)
		handleMouseEvent(event);
	else if (event.type == Common::EVENT_KEYDOWN)
		handleKbdEvent(event);
}

void EventsManager::handleMouseEvent(const Common::Event &event) {
	_mousePos = event.mouse;

	// Set mouse buttons
	minfo.button = 0;
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		minfo.button = 1;
		break;
	case Common::EVENT_RBUTTONDOWN:
		minfo.button = 2;
		break;
	default:
		break;
	}

	// Set mouse position
	if (cur_move != 1) {
		cur_move = 1;
		minfo.x = event.mouse.x;
		minfo.y = event.mouse.y;
	}
}

void EventsManager::handleKbdEvent(const Common::Event &event) {
	// TODO
}

void EventsManager::delay(size_t time) {
	uint32 expiryTime = g_system->getMillis() + time;
	uint32 currTime;

	do {
		update();
		currTime = g_system->getMillis();
	} while (currTime >= expiryTime);
}

} // namespace Chewy
