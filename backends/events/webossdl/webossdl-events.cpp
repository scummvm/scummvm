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

// Disable system overrides to allow the use of system headers
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <string>

#include "common/scummsys.h"
#include "common/system.h"
#include "common/translation.h"
#include "sys/time.h"
#include "time.h"

#include "backends/events/webossdl/webossdl-events.h"
#include "gui/message.h"
#include "engines/engine.h"
#include "PDL.h"

using std::string;

// Indicates whether the device is multitouch-capable
#ifdef MULTITOUCH
static const bool multitouch = true;
#else
static const bool multitouch = false;
#endif

// Indicates if gesture area is pressed down or not.
static bool gestureDown = false;

// Indicates if we're in touchpad mode or tap-to-move mode.
static bool touchpadMode = true;

// Indicates if we're in automatic drag mode.
static bool autoDragMode = false;

// The timestamp when screen was pressed down, per finger.
static int screenDownTime[3] = {0, 0, 0};

// Tracks which fingers are currently touching the screen.
static bool fingerDown[3] = {false, false, false};

// The timestamp when a possible drag operation was triggered.
static int dragStartTime = 0;

// The horizontal distance per finger from touch to release.
static int dragDiffX[3] = {0, 0, 0};

// The vertical distance per finger from touch to release.
static int dragDiffY[3] = {0, 0, 0};

// Indicates if we are in drag mode.
static bool dragging = false;

// The current mouse position on the screen.
static int curX = 0, curY = 0;

// The time (seconds after 1/1/1970) when program started.
static time_t programStartTime = time(0);

// Time in millis to wait before loading a queued event
static const int queuedInputEventDelay = 250;

// Time to execute queued event
static long queuedEventTime = 0;

// Indicates if dragging should be enabled when firing the queued event
static bool queuedDragEvent = false;

// An event to be processed after the next poll tick
static Common::Event queuedInputEvent;

// Indicates if a click should be executed when the first finger is lifted
static bool doClick = true;

/**
 * Initialize a new WebOSSdlEventSource.
 */
WebOSSdlEventSource::WebOSSdlEventSource() {
	queuedInputEvent.type = (Common::EventType)0;
}

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
 * Before calling the original SDL implementation, this method loads in
 * queued events.
 *
 * @param event The ScummVM event
 */
bool WebOSSdlEventSource::pollEvent(Common::Event &event) {
	long curTime = getMillis();

	// Move the queued event into the event if it's time
	if (queuedInputEvent.type != (Common::EventType)0 && curTime >= queuedEventTime) {
		event = queuedInputEvent;
		queuedInputEvent.type = (Common::EventType)0;
		if (queuedDragEvent) {
			dragging = true;
			processMouseEvent(event, curX, curY);
			queuedDragEvent = false;
		}
		return true;
	}

	return SdlEventSource::pollEvent(event);
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
 * @param ev    The SDL event
 * @param event The ScummVM event.
 * @return True if event was processed, false if not.
 */
bool WebOSSdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	dragDiffX[ev.button.which] = 0;
	dragDiffY[ev.button.which] = 0;
	fingerDown[ev.button.which] = true;
	screenDownTime[ev.button.which] = getMillis();

	if (ev.button.which == 0) {
		// Do a click when the finger lifts unless we leave the range
		doClick = true;
		// Queue up dragging if auto-drag mode is on
		if (autoDragMode) {
			queuedDragEvent = true;
			queuedInputEvent.type = Common::EVENT_LBUTTONDOWN;
			queuedEventTime = getMillis() + 500;
		}
		// Turn drag mode on instantly for a double-tap
		else if (getMillis() - dragStartTime < 400) {
			dragging = true;
			event.type = Common::EVENT_LBUTTONDOWN;
			processMouseEvent(event, curX, curY);
		}
		// If we're not in touchpad mode, move the cursor to the tap
		if (!touchpadMode) {
			int screenX = g_system->getOverlayWidth();
			int screenY = g_system->getOverlayHeight();
			curX = MIN(screenX, MAX(0, 0 + ev.motion.x));
			curY = MIN(screenY, MAX(0, 0 + ev.motion.y));
			event.type = Common::EVENT_MOUSEMOVE;
			processMouseEvent(event, curX, curY);
		}
		// Watch for a double-tap-triggered drag
		dragStartTime = getMillis();
	}
	// Kill any queued drag event if a second finger goes down
	else if (ev.button.which == 1) {
		if (queuedDragEvent) {
			queuedInputEvent.type = (Common::EventType)0;
			queuedDragEvent = false;
		}
		doClick = false;
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
	// Only react if the finger hasn't been virtually lifted already
	if (fingerDown[ev.button.which]) {
		// No matter what, if it's the first finger that's lifted when
		// we're dragging, just lift the mouse button.
		if (ev.button.which == 0 && dragging) {
			event.type = Common::EVENT_LBUTTONUP;
			processMouseEvent(event, curX, curY);
			dragging = false;
		}
		// Use a different control set if multitouch is enabled.
		else if (multitouch) {
			// If it was the first finger and the click hasn't been
			// canceled, it's a click.
			if (ev.button.which == 0 && doClick &&
					!fingerDown[1] && !fingerDown[2]) {
				event.type = Common::EVENT_LBUTTONUP;
				processMouseEvent(event, curX, curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_LBUTTONDOWN;
				if (queuedDragEvent) {
					queuedDragEvent = false;
					queuedInputEvent.type = (Common::EventType)0;
				}
			}

			// If the first finger's down and the second taps, it's a
			// right mouse click.
			else if (ev.button.which == 1 &&
					fingerDown[0] && fingerDown[1] && !fingerDown[2]) {
				event.type = Common::EVENT_RBUTTONUP;
				processMouseEvent(event, curX, curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_RBUTTONDOWN;
			}

			// If two fingers are down and a third taps, it's a middle
			// click -- but lift the second finger so it doesn't register
			// as a right click.
			else if (ev.button.which == 2 &&
					fingerDown[0] && fingerDown[1]) {
				event.type = Common::EVENT_MBUTTONUP;
				processMouseEvent(event, curX, curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_MBUTTONDOWN;
				fingerDown[1] = false;
			}
		}
		else if (ev.button.which == 0 && doClick &&
				!fingerDown[1] && !fingerDown[2]) {
			int duration = getMillis() - screenDownTime[0];

			// When screen was pressed for less than 500ms then emulate a
			// left mouse click.
			if (duration < 500) {
				event.type = Common::EVENT_LBUTTONUP;
				processMouseEvent(event, curX, curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_LBUTTONDOWN;
			}

			// When screen was pressed for less than 1000ms then emulate a
			// right mouse click.
			else if (duration < 1000) {
				event.type = Common::EVENT_RBUTTONUP;
				processMouseEvent(event, curX, curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_RBUTTONDOWN;
			}

			// When screen was pressed for more than 1000ms then emulate a
			// middle mouse click.
			else {
				event.type = Common::EVENT_MBUTTONUP;
				processMouseEvent(event, curX, curY);
				g_system->getEventManager()->pushEvent(event);
				event.type = Common::EVENT_MBUTTONDOWN;
			}
		}
		// Officially lift the finger that was raised.
		fingerDown[ev.button.which] = false;	
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
	if (fingerDown[ev.motion.which]) {
		dragDiffX[ev.motion.which] += ev.motion.xrel;
		dragDiffY[ev.motion.which] += ev.motion.yrel;
		int screenX = g_system->getOverlayWidth();
		int screenY = g_system->getOverlayHeight();

		switch (ev.motion.which) {
			case 0:
				// If our dragDiff goes > 3 pixels in either direction, kill
				// the future click and any queued drag event.
				if (doClick && ABS(dragDiffX[0]) > 2 &&
						ABS(dragDiffY[0]) > 2) {
					doClick = false;
					if (queuedDragEvent) {
						queuedInputEvent.type = (Common::EventType)0;
						queuedDragEvent = false;
					}
				}
				// If only one finger is on the screen and moving, that's
				// the mouse pointer.
				if (!fingerDown[1] && !fingerDown[2]) {
					if (touchpadMode) {
						curX = MIN(screenX, MAX(0, curX + ev.motion.xrel));
						curY = MIN(screenY, MAX(0, curY + ev.motion.yrel));
					} else {
						curX = MIN(screenX, MAX(0, 0 + ev.motion.x));
						curY = MIN(screenY, MAX(0, 0 + ev.motion.y));
					}
					event.type = Common::EVENT_MOUSEMOVE;
					processMouseEvent(event, curX, curY);
				}
				break;
			case 1:
				// Check for a two-finger swipe
				if (fingerDown[0] && !fingerDown[2]) {
					// Check for a vertical 20% swipe
					if (ABS(dragDiffY[0]) > screenY * 0.2 &&
							ABS(dragDiffY[1]) > screenY * 0.2) {
						// Virtually lift fingers so we don't get repeat triggers
						fingerDown[0] = fingerDown[1] = false;
						if (dragDiffY[0] < 0 && dragDiffY[1] < 0) {
							// A swipe up triggers the keyboard, if it exists
							int gblPDKVersion = PDL_GetPDKVersion();
							if (gblPDKVersion >= 300)
								PDL_SetKeyboardState(PDL_TRUE);
						} else if (dragDiffY[0] > 0 && dragDiffY[1] > 0){
							// A swipe down triggers the menu
							if (g_engine && !g_engine->isPaused())
								g_engine->openMainMenuDialog();
						}
						return true;
					}
					// Check for a horizontal 15% swipe
					if (ABS(dragDiffX[0]) > screenX * 0.15 &&
							ABS(dragDiffX[1]) > screenX * 0.15) {
						// Virtually lift fingers so we don't get repeat triggers
						fingerDown[0] = fingerDown[1] = false;
						if (dragDiffX[0] < 0 && dragDiffX[1] < 0) {
							// A swipe left presses escape
							event.type = Common::EVENT_KEYDOWN;
							queuedInputEvent.type = Common::EVENT_KEYUP;
							event.kbd.flags = queuedInputEvent.kbd.flags = 0;
							event.kbd.keycode = queuedInputEvent.kbd.keycode = Common::KEYCODE_ESCAPE;
							event.kbd.ascii = queuedInputEvent.kbd.ascii = Common::ASCII_ESCAPE;
							queuedEventTime = getMillis() + queuedInputEventDelay;
						} else if (dragDiffX[0] > 0 && dragDiffX[1] > 0) {
							// A swipe right toggles touchpad mode
							touchpadMode = !touchpadMode;
							g_system->showMouse(touchpadMode);
							string dialogMsg(_("Touchpad mode is now "));
							dialogMsg += (touchpadMode ? _("ON") : _("OFF"));
							dialogMsg += ".\n";
							dialogMsg += _("Swipe two fingers to the right to toggle.");
							GUI::TimedMessageDialog dialog(dialogMsg.c_str(), 1500);
							dialog.runModal();
						}
						return true;
					}
				}
				break;
			case 2:
				// Check for a three-finger horizontal 15% swipe right
				if (fingerDown[0] && fingerDown[1] && 
						dragDiffX[0] > screenX * 0.15 &&
						dragDiffX[1] > screenX * 0.15 &&
						dragDiffX[2] > screenX * 0.15) {
					// Virtually lift fingers so we don't get repeat triggers
					fingerDown[0] = fingerDown[1] = fingerDown[2] = false;
					// Toggle Auto-drag mode
					autoDragMode = !autoDragMode;
					string dialogMsg(_("Auto-drag mode is now "));
					dialogMsg += (autoDragMode ? _("ON") : _("OFF"));
					dialogMsg += ".\n";
					dialogMsg += _("Swipe three fingers to the right to toggle.");
					GUI::TimedMessageDialog dialog(dialogMsg.c_str(), 1500);
					dialog.runModal();
					return true;
				}
		}	
	}
	return true;
}

#endif
