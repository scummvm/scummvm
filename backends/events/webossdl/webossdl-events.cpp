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

#include "common/scummsys.h"
#include <stdio.h>

#ifdef WEBOS

#include "backends/events/webossdl/webossdl-events.h"

static int mouseButton = 0;

// WebOS devices only have a shift key and a ctrl key. There is also an alt
// key (the orange key) but this is already processed by WebOS to change the 
// mode of the keys so ScummVM must not use this key as a modifier.
void WebOSSdlEventSource::SDLModToOSystemKeyFlags(SDLMod mod, Common::Event &event) {
        event.kbd.flags = 0;

        if (mod & KMOD_SHIFT)
                event.kbd.flags |= Common::KBD_SHIFT;
        if (mod & KMOD_CTRL)
                event.kbd.flags |= Common::KBD_CTRL;
}

// Capture Shift and Control to decide the active mouse button
bool WebOSSdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {
	int sym = ev.key.keysym.sym;
	switch (ev.type) {
		case SDL_KEYDOWN:
			if (sym == 304) {
				mouseButton = 1;
				return true;
			}
			else if (sym == 305) {
				mouseButton = 2;
				return true;
			}
			break;
		case SDL_KEYUP:
			if (sym == 304 || sym == 305) {
				mouseButton = 0;
				return true;
			}
			break;
	}

	// Invoke parent implementation of this method
	return SdlEventSource::remapKey(ev, event);
}

bool WebOSSdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	// Simulate right mouse button
	if (ev.button.button == SDL_BUTTON_LEFT && mouseButton == 1) {
		event.type = Common::EVENT_RBUTTONDOWN;
		fillMouseEvent(event, ev.button.x, ev.button.y);
		printf("rbutton down\n");
		return true;
	}

	// Simulate middle mouse button
	if (ev.button.button == SDL_BUTTON_LEFT && mouseButton == 2) {
		event.type = Common::EVENT_MBUTTONDOWN;
		fillMouseEvent(event, ev.button.x, ev.button.y);
		printf("rbutton down\n");
		return true;
	}

	// Invoke parent implementation of this method
	return SdlEventSource::handleMouseButtonDown(ev, event);
}

bool WebOSSdlEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	// Simulate right mouse button
	if (ev.button.button == SDL_BUTTON_LEFT && mouseButton == 1) {
		event.type = Common::EVENT_RBUTTONUP;
		fillMouseEvent(event, ev.button.x, ev.button.y);
		return true;
	}

	// Simulate right mouse button
	if (ev.button.button == SDL_BUTTON_LEFT && mouseButton == 2) {
		event.type = Common::EVENT_MBUTTONUP;
		fillMouseEvent(event, ev.button.x, ev.button.y);
		return true;
	}

	// Invoke parent implementation of this method
	return SdlEventSource::handleMouseButtonUp(ev, event);
}

#endif
