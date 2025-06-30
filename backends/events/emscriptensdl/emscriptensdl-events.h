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

#if !defined(BACKEND_EVENTS_EMSCRIPTEN_H) && !defined(DISABLE_DEFAULT_EVENTMANAGER)
#define BACKEND_EVENTS_EMSCRIPTEN_H

#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/emscripten/emscripten.h"
#include "common/events.h"

/**
 * SDL Events manager for Emscripten
 */
class EmscriptenSdlEventSource : public SdlEventSource {
public:
	/**
	 * Gets and processes SDL events.
	 */
	bool pollEvent(Common::Event &event) override {
	
		bool ret_value = SdlEventSource::pollEvent(event);
		if (event.type != Common::EVENT_QUIT && event.type != Common::EVENT_RETURN_TO_LAUNCHER) {	
			// yield to the browser and process timers  
			// (after polling the events to ensure synchronous event processing)
			g_system->delayMillis(0);
		}
		return ret_value;
	};
};

#endif /* BACKEND_EVENTS_EMSCRIPTEN_H */
