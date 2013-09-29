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
#include "engines/engine.h"
#include "backends/touchmapper/touchmapper.h"
#include "common/config-manager.h"

namespace Common {

Touchmapper::Touchmapper(EventManager *evtMgr) {
	if (ConfMan.hasKey("direct_input") && ConfMan.getBool("direct_input"))
		inputMode = kDirectInput;
	else
		inputMode = kTouchpadMode;

	_dragAndDrop = false;
}

Touchmapper::~Touchmapper() {
}

List<Event> Touchmapper::mapEvent(const Event &ev, EventSource *source) {
	List<Event> events;
	Event mappedEvent;

	// FIXME: Maybe check something else instead!
	if (g_engine && !g_engine->isPaused()) {
		if (inputMode == kDirectInput) {
			switch (ev.type) {
				case EVENT_FINGERMOVE:
					g_system->warpMouse(ev.finger[0].position.x, ev.finger[0].position.y);
					break;
				case EVENT_SINGLETAP:
					g_system->warpMouse(ev.finger[0].position.x, ev.finger[0].position.y);
					mappedEvent.type = EVENT_LBUTTONUP;
					break;
				case EVENT_TAPDOWN:
					g_system->warpMouse(ev.finger[0].position.x, ev.finger[0].position.y);
					mappedEvent.type = EVENT_LBUTTONDOWN;
					break;
				case EVENT_SIMULATE_MBUTTONUP:
					g_system->warpMouse(ev.finger[0].position.x, ev.finger[0].position.y);
					mappedEvent.type = EVENT_MBUTTONUP;
					break;
				case EVENT_SIMULATE_MBUTTONDOWN:
					g_system->warpMouse(ev.finger[0].position.x, ev.finger[0].position.y);
					mappedEvent.type = EVENT_MBUTTONDOWN;
					break;
				case EVENT_SIMULATE_RBUTTONUP:
					g_system->warpMouse(ev.finger[0].position.x, ev.finger[0].position.y);
					mappedEvent.type = EVENT_RBUTTONUP;
					break;
				case EVENT_SIMULATE_RBUTTONDOWN:
					g_system->warpMouse(ev.finger[0].position.x, ev.finger[0].position.y);
					mappedEvent.type = EVENT_RBUTTONDOWN;
					break;
			}
		} else if (inputMode == kTouchpadMode) {
			int mousex;
			int mousey;

			mousex = g_system->getEventManager()->getMousePos().x;
			mousey = g_system->getEventManager()->getMousePos().y;

			if (!_dragAndDrop) {
				switch (ev.type) {
					case EVENT_FINGERMOVE:
					case EVENT_TAPDOWN:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						break;
					case EVENT_DOUBLETAP:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						mappedEvent.type = EVENT_LBUTTONDOWN;
						_dragAndDrop = true;
						break;
					case EVENT_SINGLETAP:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						mappedEvent.type = EVENT_LBUTTONUP;
						break;
					case EVENT_SIMULATE_LBUTTONDOWN:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						mappedEvent.type = EVENT_LBUTTONDOWN;
						break;
					case EVENT_SIMULATE_MBUTTONUP:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						mappedEvent.type = EVENT_MBUTTONUP;
						break;
					case EVENT_SIMULATE_MBUTTONDOWN:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						mappedEvent.type = EVENT_MBUTTONDOWN;
						break;
					case EVENT_SIMULATE_RBUTTONUP:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						mappedEvent.type = EVENT_RBUTTONUP;
						break;
					case EVENT_SIMULATE_RBUTTONDOWN:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						mappedEvent.type = EVENT_RBUTTONDOWN;
						break;
				}
			} else {
				switch (ev.type) {
					case EVENT_FINGERMOVE:
					case EVENT_TAPDOWN:
					case EVENT_SINGLETAP:
					case EVENT_SIMULATE_LBUTTONDOWN:
					case EVENT_SIMULATE_MBUTTONUP:
					case EVENT_SIMULATE_MBUTTONDOWN:
					case EVENT_SIMULATE_RBUTTONUP:
					case EVENT_SIMULATE_RBUTTONDOWN:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						break;
					case EVENT_DOUBLETAP:
						g_system->warpMouse(mousex + ev.finger[0].deltax, mousey + ev.finger[0].deltay);
						mappedEvent.type = EVENT_LBUTTONUP;
						_dragAndDrop = false;
						break;
				}
			}
		}
	} else {
		// In GUI
		switch (ev.type) {
			case EVENT_SIMULATE_RBUTTONUP:
			case EVENT_SIMULATE_MBUTTONUP:
				mappedEvent.type = EVENT_SINGLETAP;
				break;
			case EVENT_SIMULATE_LBUTTONDOWN:
			case EVENT_SIMULATE_MBUTTONDOWN:
			case EVENT_SIMULATE_RBUTTONDOWN:
				break;
		}
	}

	// if it didn't get mapped, just pass it through
	if (mappedEvent.type == EVENT_INVALID)
		mappedEvent = ev;

	events.push_back(mappedEvent);
	return events;
}

} // End of namespace Common
