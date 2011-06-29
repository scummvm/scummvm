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
 */

#ifdef WEBOS

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "common/scummsys.h"
#include "common/system.h"
#include "sys/time.h"
#include "time.h"

#include "backends/events/webossdl/webossdl-events.h"
#include "gui/message.h"

// Inidicates if gesture area is pressed down or not.
static bool gestureDown = false;

// The timestamp when screen was pressed down.
static int screenDownTime = 0;

// The timestamp when a possible drag operation was triggered.
static int dragStartTime = 0;

// The index of the motion pointer.
static int motionPtrIndex = -1;

// The maximum horizontal motion during dragging (For tap recognition).
static int dragDiffX = 0;

// The maximum vertical motion during dragging (For tap recognition).
static int dragDiffY = 0;

// Indicates if we are in drag mode.
static bool dragging = false;

// The current mouse position on the screen.
static int curX = 0, curY = 0;

// The time (seconds after 1/1/1970) when program started.
static time_t programStartTime = time(0);

/**
 * Returns the number of passed milliseconds since program start.
 *
 * @return The number of passed milliseconds.
 */
static time_t getMillis()
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (time(0) - programStartTime) * 1000 + tv.tv_usec / 1000;
}

/**
 * WebOS devices only have a Shift key and a CTRL key. There is also an Alt
 * key (the orange key) but this is already processed by WebOS to change the
 * mode of the keys so ScummVM must not use this key as a modifier. Instead
 * pressing down the gesture area is used as Alt key.
 *
 * @param mod   The pressed key modifier as detected by SDL.
 * @param event The ScummVM event to setup.
 */
void WebOSSdlEventSource::SDLModToOSystemKeyFlags(SDLMod mod,
		Common::Event &event) {
	event.kbd.flags = 0;

	if (mod & KMOD_SHIFT)
		event.kbd.flags |= Common::KBD_SHIFT;
	if (mod & KMOD_CTRL)
		event.kbd.flags |= Common::KBD_CTRL;

	// Holding down the gesture area emulates the ALT key
	if (gestureDown)
		event.kbd.flags |= Common::KBD_ALT;
}

/**
 * Before calling the original SDL implementation this method checks if the
 * gesture area is pressed down.
 *
 * @param ev    The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleKeyDown(SDL_Event &ev, Common::Event &event) {
	// Handle gesture area tap.
	if (ev.key.keysym.sym == SDLK_WORLD_71) {
		gestureDown = true;
		return true;
	}

	// Ensure that ALT key (Gesture down) is ignored when back or forward
	// gesture is detected. This is needed for WebOS 1 which releases the
	// gesture tap AFTER the backward gesture event and not BEFORE (Like
	// WebOS 2).
	if (ev.key.keysym.sym == 27 || ev.key.keysym.sym == 229) {
	    gestureDown = false;
	}

	// Call original SDL key handler.
	return SdlEventSource::handleKeyDown(ev, event);
}

/**
 * Before calling the original SDL implementation this method checks if the
 * gesture area has been released.
 *
 * @param ev    The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleKeyUp(SDL_Event &ev, Common::Event &event) {
	// Handle gesture area tap.
	if (ev.key.keysym.sym == SDLK_WORLD_71) {
		gestureDown = false;
		return true;
	}

	// Call original SDL key handler.
	return SdlEventSource::handleKeyUp(ev, event);
}

/**
 * Handles mouse button press.
 *
 * @param ev    The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	if (motionPtrIndex == -1) {
		motionPtrIndex = ev.button.which;
		dragDiffX = 0;
		dragDiffY = 0;
		screenDownTime = getMillis();

		// Start dragging when pressing the screen shortly after a tap.
		if (getMillis() - dragStartTime < 250) {
			dragging = true;
			event.type = Common::EVENT_LBUTTONDOWN;
			fillMouseEvent(event, curX, curY);
		}
	}
	return true;
}

/**
 * Handles mouse button release.
 *
 * @param ev    The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	if (motionPtrIndex == ev.button.which) {
		motionPtrIndex = -1;

		// When drag mode was active then simply send a mouse up event
		if (dragging)
		{
			event.type = Common::EVENT_LBUTTONUP;
			fillMouseEvent(event, curX, curY);
			dragging = false;
			return true;
		}

		// When mouse was moved 5 pixels or less then emulate a mouse button
		// click.
		if (ABS(dragDiffX) < 6 && ABS(dragDiffY) < 6)
		{
			int duration = getMillis() - screenDownTime;

			// When screen was pressed for less than 500ms then emulate a
			// left mouse click.
			if (duration < 500) {
				event.type = Common::EVENT_LBUTTONUP;
				fillMouseEvent(event, curX, curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_LBUTTONDOWN;
				dragStartTime = getMillis();
			}

			// When screen was pressed for less than 1000ms then emulate a
			// right mouse click.
			else if (duration < 1000) {
				event.type = Common::EVENT_RBUTTONUP;
				fillMouseEvent(event, curX, curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_RBUTTONDOWN;
			}

			// When screen was pressed for more than 1000ms then emulate a
			// middle mouse click.
			else {
				event.type = Common::EVENT_MBUTTONUP;
				fillMouseEvent(event, curX, curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_MBUTTONDOWN;
			}

		}
	}
	return true;
}

/**
 * Handles mouse motion.
 *
 * @param ev    The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleMouseMotion(SDL_Event &ev, Common::Event &event) {
	if (ev.motion.which == motionPtrIndex) {
		int screenX = g_system->getWidth();
		int screenY = g_system->getHeight();
		curX = MIN(screenX, MAX(0, curX + ev.motion.xrel));
		curY = MIN(screenY, MAX(0, curY + ev.motion.yrel));
		dragDiffX += ev.motion.xrel;
		dragDiffY += ev.motion.yrel;
		event.type = Common::EVENT_MOUSEMOVE;
		fillMouseEvent(event, curX, curY);
	}
	return true;
}

#endif
