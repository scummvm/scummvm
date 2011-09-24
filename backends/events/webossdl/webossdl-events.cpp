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
#include "engines/engine.h"
#include "PDL.h"

/**
 * Construct a new WebOSSdlEventSource.
 */
WebOSSdlEventSource::WebOSSdlEventSource() :
		_gestureDown(false),
		_screenDownTime(0),
		_dragStartTime(0),
		_motionPtrIndex(-1),
		_dragDiffX(0), _dragDiffY(0),
		_dragging(false),
		_curX(0), _curY(0) {

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
	if (_gestureDown)
		event.kbd.flags |= Common::KBD_ALT;
}

/**
 * Before calling the original SDL implementation this method checks if the
 * gesture area is pressed down.
 *
 * @param ev	 The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleKeyDown(SDL_Event &ev, Common::Event &event) {
	// Handle gesture area tap.
	if (ev.key.keysym.sym == SDLK_WORLD_71) {
		_gestureDown = true;
		return true;
	}

	// Ensure that ALT key (Gesture down) is ignored when back or forward
	// gesture is detected. This is needed for WebOS 1 which releases the
	// gesture tap AFTER the backward gesture event and not BEFORE (Like
	// WebOS 2).
	if (ev.key.keysym.sym == 27 || ev.key.keysym.sym == 229) {
		 _gestureDown = false;
	}

	// handle virtual keyboard dismiss key
	if (ev.key.keysym.sym == 24) {
		int gblPDKVersion = PDL_GetPDKVersion();
		// check for correct PDK Version
		if (gblPDKVersion >= 300) {
			PDL_SetKeyboardState(PDL_FALSE);
			return true;
		}
	}

	// Call original SDL key handler.
	return SdlEventSource::handleKeyDown(ev, event);
}

/**
 * Before calling the original SDL implementation this method checks if the
 * gesture area has been released.
 *
 * @param ev	 The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleKeyUp(SDL_Event &ev, Common::Event &event) {
	// Handle gesture area tap.
	if (ev.key.keysym.sym == SDLK_WORLD_71) {
		_gestureDown = false;
		return true;
	}

	// handle virtual keyboard dismiss key
	if (ev.key.keysym.sym == 24) {
		int gblPDKVersion = PDL_GetPDKVersion();
		// check for correct PDK Version
		if (gblPDKVersion >= 300) {
			PDL_SetKeyboardState(PDL_FALSE);
			return true;
		}
	}

	// Call original SDL key handler.
	return SdlEventSource::handleKeyUp(ev, event);
}

/**
 * Handles mouse button press.
 *
 * @param ev	 The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	if (_motionPtrIndex == -1) {
		_motionPtrIndex = ev.button.which;
		_dragDiffX = 0;
		_dragDiffY = 0;
		_screenDownTime = g_system->getMillis();

		// Start dragging when pressing the screen shortly after a tap.
		if (g_system->getMillis() - _dragStartTime < 250) {
			_dragging = true;
			event.type = Common::EVENT_LBUTTONDOWN;
			processMouseEvent(event, _curX, _curY);
		}
	}
	return true;
}

/**
 * Handles mouse button release.
 *
 * @param ev	 The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	if (_motionPtrIndex == ev.button.which) {
		_motionPtrIndex = -1;

		int screenY = g_system->getOverlayHeight();
		
		// 60% of the screen height for menu dialog/keyboard
		if (ABS(_dragDiffY) >= ABS(screenY*0.6)) {
			if (_dragDiffY >= 0) {
				int gblPDKVersion = PDL_GetPDKVersion();
				// check for correct PDK Version
				if (gblPDKVersion >= 300) {
					PDL_SetKeyboardState(PDL_TRUE);
					return true;
				}
			} else {
				if (g_engine && !g_engine->isPaused()) {
					g_engine->openMainMenuDialog();
					return true;
				}
			}
		}

		// When drag mode was active then simply send a mouse up event
		if (_dragging) {
			event.type = Common::EVENT_LBUTTONUP;
			processMouseEvent(event, _curX, _curY);
			_dragging = false;
			return true;
		}

		// When mouse was moved 5 pixels or less then emulate a mouse button
		// click.
		if (ABS(_dragDiffX) < 6 && ABS(_dragDiffY) < 6) {
			int duration = g_system->getMillis() - _screenDownTime;

			// When screen was pressed for less than 500ms then emulate a
			// left mouse click.
			if (duration < 500) {
				event.type = Common::EVENT_LBUTTONUP;
				processMouseEvent(event, _curX, _curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_LBUTTONDOWN;
				_dragStartTime = g_system->getMillis();
			}

			// When screen was pressed for less than 1000ms then emulate a
			// right mouse click.
			else if (duration < 1000) {
				event.type = Common::EVENT_RBUTTONUP;
				processMouseEvent(event, _curX, _curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_RBUTTONDOWN;
			}

			// When screen was pressed for more than 1000ms then emulate a
			// middle mouse click.
			else {
				event.type = Common::EVENT_MBUTTONUP;
				processMouseEvent(event, _curX, _curY);
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
 * @param ev	 The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleMouseMotion(SDL_Event &ev, Common::Event &event) {
	if (ev.motion.which == _motionPtrIndex) {
		int screenX = g_system->getOverlayWidth();
		int screenY = g_system->getOverlayHeight();
		_curX = MIN(screenX, MAX(0, _curX + ev.motion.xrel));
		_curY = MIN(screenY, MAX(0, _curY + ev.motion.yrel));
		_dragDiffX += ev.motion.xrel;
		_dragDiffY += ev.motion.yrel;
		event.type = Common::EVENT_MOUSEMOVE;
		processMouseEvent(event, _curX, _curY);
	}
	return true;
}

#endif
