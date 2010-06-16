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
 *
 * $URL$
 * $Id$
 *
 */

#if !defined(BACKEND_EVENTS_SDL_H) && !defined(DISABLE_DEFAULT_EVENTMANAGER)
#define BACKEND_EVENTS_SDL_H

#include "backends/events/default/default-events.h"

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

class SdlEventManager : public DefaultEventManager {
public:
	SdlEventManager(Common::EventSource *boss);
	~SdlEventManager();

	virtual bool pollSdlEvent(Common::Event &event);

	void resetKeyboadEmulation(int16 x_max, int16 y_max);

protected:
	virtual void preprocessEvents(SDL_Event *event) {}

	// Keyboard mouse emulation.  Disabled by fingolfin 2004-12-18.
	// I am keeping the rest of the code in for now, since the joystick
	// code (or rather, "hack") uses it, too.
	struct KbdMouse {
		int16 x, y, x_vel, y_vel, x_max, y_max, x_down_count, y_down_count;
		uint32 last_time, delay_time, x_down_time, y_down_time;
	};

	KbdMouse _km;

	// Scroll lock state - since SDL doesn't track it
	bool _scrollLock;
	
	// joystick
	SDL_Joystick *_joystick;

	virtual bool dispatchSDLEvent(SDL_Event &ev, Common::Event &event);

	// Handlers for specific SDL events, called by pollEvent.
	// This way, if a backend inherits fromt the SDL backend, it can
	// change the behavior of only a single event, without having to override all
	// of pollEvent.
	virtual bool handleKeyDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleKeyUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseMotion(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyAxisMotion(SDL_Event &ev, Common::Event &event);

	virtual void fillMouseEvent(Common::Event &event, int x, int y); // overloaded by CE backend
	void toggleMouseGrab();

	void handleKbdMouse();
	virtual bool remapKey(SDL_Event &ev, Common::Event &event);

	int _lastScreenID;
};

#endif
