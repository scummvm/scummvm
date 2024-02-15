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

#include "common/scummsys.h"

#if defined(NINTENDO_SWITCH)

#include <math.h>

#include "backends/platform/sdl/switch/switch.h"
#include "backends/events/switchsdl/switchsdl-events.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/platform/sdl/sdl.h"
#include "engines/engine.h"

#include "common/util.h"
#include "common/events.h"
#include "common/config-manager.h"

SwitchEventSource::SwitchEventSource() {
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		_finger[i].id = -1;
		_finger[i].timeLastDown = 0;
		_finger[i].lastX = 0;
		_finger[i].lastY = 0;
		_finger[i].lastDownX = 0;
		_finger[i].lastDownY = 0;
	}
	_multiFingerDragging = DRAG_NONE;

	for (int i = 0; i < 2; i++) {
		_simulatedClickStartTime[i] = 0;
	}
}

bool SwitchEventSource::pollEvent(Common::Event &event) {
	((DefaultTimerManager *) g_system->getTimerManager())->handler();
	finishSimulatedMouseClicks();
	return SdlEventSource::pollEvent(event);
}

void SwitchEventSource::preprocessEvents(SDL_Event *event) {

	// Supported touch gestures:
	// left mouse click: single finger short tap
	// right mouse click: second finger short tap while first finger is still down
	// pointer motion: single finger drag
	if (event->type == SDL_FINGERDOWN || event->type == SDL_FINGERUP || event->type == SDL_FINGERMOTION) {
		// touchpad_mouse_mode off: use direct touch control of pointer
		// touchpad_mouse_mode on: use indirect touch control
		// where the finger can be somewhere else than the pointer and still move it
		switch (event->type) {
		case SDL_FINGERDOWN:
			//debug(0, "down[%li]: %i %i", event->tfinger.fingerId, (int) event->tfinger.x, (int) event->tfinger.y);
			preprocessFingerDown(event);
			break;
		case SDL_FINGERUP:
			//debug(0, "up[%li]: %i %i", event->tfinger.fingerId, (int) event->tfinger.x, (int) event->tfinger.y);
			preprocessFingerUp(event);
			break;
		case SDL_FINGERMOTION:
			//debug(0, "mov[%li]: %i %i", event->tfinger.fingerId, (int) event->tfinger.x, (int) event->tfinger.y);
			preprocessFingerMotion(event);
			break;
		}
	}
}

void SwitchEventSource::preprocessFingerDown(SDL_Event *event) {
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

	int x = _mouseX;
	int y = _mouseY;

	if (!ConfMan.getBool("touchpad_mouse_mode")) {
		convertTouchXYToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
	}

	// make sure each finger is not reported down multiple times
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[i].id == id) {
			_finger[i].id = -1;
		}
	}

	// we need the timestamps to decide later if the user performed a short tap (click)
	// or a long tap (drag)
	// we also need the last coordinates for each finger to keep track of dragging
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[i].id == -1) {
			_finger[i].id = id;
			_finger[i].timeLastDown = event->tfinger.timestamp;
			_finger[i].lastDownX = event->tfinger.x;
			_finger[i].lastDownY = event->tfinger.y;
			_finger[i].lastX = x;
			_finger[i].lastY = y;
			break;
		}
	}
}

void SwitchEventSource::preprocessFingerUp(SDL_Event *event) {
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[i].id >= 0) {
			numFingersDown++;
		}
	}

	int x = _mouseX;
	int y = _mouseY;

	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[i].id == id) {
			_finger[i].id = -1;
			if (!_multiFingerDragging) {
				if ((event->tfinger.timestamp - _finger[i].timeLastDown) <= MAX_TAP_TIME) {
					// short (<MAX_TAP_TIME ms) tap is interpreted as right/left mouse click depending on # fingers already down
					// but only if the finger hasn't moved since it was pressed down by more than MAX_TAP_MOTION_DISTANCE pixels
					float xrel = ((event->tfinger.x * (float) TOUCHSCREEN_WIDTH) - (_finger[i].lastDownX * (float) TOUCHSCREEN_WIDTH));
					float yrel = ((event->tfinger.y * (float) TOUCHSCREEN_HEIGHT) - (_finger[i].lastDownY * (float) TOUCHSCREEN_HEIGHT));
					float maxRSquared = (float) (MAX_TAP_MOTION_DISTANCE * MAX_TAP_MOTION_DISTANCE);
					if ((xrel * xrel + yrel * yrel) < maxRSquared) {
						if (numFingersDown == 2 || numFingersDown == 1) {
							uint8 simulatedButton = 0;
							if (numFingersDown == 2) {
								simulatedButton = SDL_BUTTON_RIGHT;
								// need to raise the button later
								_simulatedClickStartTime[1] = event->tfinger.timestamp;
							} else if (numFingersDown == 1) {
								simulatedButton = SDL_BUTTON_LEFT;
								// need to raise the button later
								_simulatedClickStartTime[0] = event->tfinger.timestamp;
								if (!ConfMan.getBool("touchpad_mouse_mode")) {
									convertTouchXYToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
								}
							}

							event->type = SDL_MOUSEBUTTONDOWN;
							event->button.button = simulatedButton;
							event->button.x = x;
							event->button.y = y;
						}
					}
				}
			} else if (numFingersDown == 1) {
				// when dragging, and the last finger is lifted, the drag is over
				if (!ConfMan.getBool("touchpad_mouse_mode")) {
					convertTouchXYToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
				}
				uint8 simulatedButton = 0;
				if (_multiFingerDragging == DRAG_THREE_FINGER)
					simulatedButton = SDL_BUTTON_RIGHT;
				else {
					simulatedButton = SDL_BUTTON_LEFT;
				}
				event->type = SDL_MOUSEBUTTONUP;
				event->button.button = simulatedButton;
				event->button.x = x;
				event->button.y = y;
				_multiFingerDragging = DRAG_NONE;
			}
		}
	}
}

void SwitchEventSource::preprocessFingerMotion(SDL_Event *event) {
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[i].id >= 0) {
			numFingersDown++;
		}
	}

	if (numFingersDown >= 1) {
		int x = _mouseX;
		int y = _mouseY;
		int xMax = _graphicsManager->getWindowWidth() - 1;
		int yMax = _graphicsManager->getWindowHeight() - 1;

		if (!ConfMan.getBool("touchpad_mouse_mode")) {
			convertTouchXYToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
		} else {
			// for relative mode, use the pointer speed setting
			float speedFactor = 1.0;

			switch (ConfMan.getInt("kbdmouse_speed")) {
			case 0:
				speedFactor = 0.25;
				break;
			case 1:
				speedFactor = 0.5;
				break;
			case 2:
				speedFactor = 0.75;
				break;
			case 3:
				speedFactor = 1.0;
				break;
			case 4:
				speedFactor = 1.25;
				break;
			case 5:
				speedFactor = 1.5;
				break;
			case 6:
				speedFactor = 1.75;
				break;
			case 7:
				speedFactor = 2.0;
				break;
			default:
				speedFactor = 1.0;
			}

			// convert touch events to relative mouse pointer events
			// Whenever an SDL_event involving the mouse is processed,
			// _mouseX/Y are truncated from subpixel precision to regular pixel precision.
			// Therefore, there's no need here to deal with subpixel precision in _mouseX/Y.
			x = (_mouseX + (event->tfinger.dx * 1.25 * speedFactor * xMax));
			y = (_mouseY + (event->tfinger.dy * 1.25 * speedFactor * yMax));
		}

		x = CLIP(x, 0, xMax);
		y = CLIP(y, 0, yMax);

		// update the current finger's coordinates so we can track it later
		for (int i = 0; i < MAX_NUM_FINGERS; i++) {
			if (_finger[i].id == id) {
				_finger[i].lastX = x;
				_finger[i].lastY = y;
			}
		}

		// If we are starting a multi-finger drag, start holding down the mouse button
		if (numFingersDown >= 2) {
			if (!_multiFingerDragging) {
				// only start a multi-finger drag if at least two fingers have been down long enough
				int numFingersDownLong = 0;
				for (int i = 0; i < MAX_NUM_FINGERS; i++) {
					if (_finger[i].id >= 0) {
						if (event->tfinger.timestamp - _finger[i].timeLastDown > MAX_TAP_TIME) {
							numFingersDownLong++;
						}
					}
				}
				if (numFingersDownLong >= 2) {
					// starting drag, so push mouse down at current location
					// or location of "oldest" finger depending on mode
					int mouseDownX = _mouseX;
					int mouseDownY = _mouseY;
					if (!ConfMan.getBool("touchpad_mouse_mode")) {
						for (int i = 0; i < MAX_NUM_FINGERS; i++) {
							if (_finger[i].id == id) {
								uint32 earliestTime = _finger[i].timeLastDown;
								for (int j = 0; j < MAX_NUM_FINGERS; j++) {
									if (_finger[j].id >= 0 && (i != j) ) {
										if (_finger[j].timeLastDown < earliestTime) {
											mouseDownX = _finger[j].lastX;
											mouseDownY = _finger[j].lastY;
											earliestTime = _finger[j].timeLastDown;
										}
									}
								}
								break;
							}
						}
					}
					uint8 simulatedButton = 0;
					if (numFingersDownLong == 2) {
						simulatedButton = SDL_BUTTON_LEFT;
						_multiFingerDragging = DRAG_TWO_FINGER;
					} else {
						simulatedButton = SDL_BUTTON_RIGHT;
						_multiFingerDragging = DRAG_THREE_FINGER;
					}
					SDL_Event ev;
					ev.type = SDL_MOUSEBUTTONDOWN;
					ev.button.button = simulatedButton;
					ev.button.x = mouseDownX;
					ev.button.y = mouseDownY;
					SDL_PushEvent(&ev);
				}
			}
		}

		//check if this is the "oldest" finger down (or the only finger down), otherwise it will not affect mouse motion
		bool updatePointer = true;
		if (numFingersDown > 1) {
			for (int i = 0; i < MAX_NUM_FINGERS; i++) {
				if (_finger[i].id == id) {
					for (int j = 0; j < MAX_NUM_FINGERS; j++) {
						if (_finger[j].id >= 0 && (i != j) ) {
							if (_finger[j].timeLastDown < _finger[i].timeLastDown) {
								updatePointer = false;
							}
						}
					}
				}
			}
		}
		if (updatePointer) {
			event->type = SDL_MOUSEMOTION;
			event->motion.x = x;
			event->motion.y = y;
		}
	}
}

void SwitchEventSource::convertTouchXYToGameXY(float touchX, float touchY, int *gameX, int *gameY) {
	int screenH = _graphicsManager->getWindowHeight();
	int screenW = _graphicsManager->getWindowWidth();

	const int dispW = TOUCHSCREEN_WIDTH;
	const int dispH = TOUCHSCREEN_HEIGHT;

	int x, y, w, h;
	float sx, sy;
	float ratio = (float)screenW / (float)screenH;

	h = dispH;
	w = h * ratio;

	x = (dispW - w) / 2;
	y = (dispH - h) / 2;

	sy = (float)h / (float)screenH;
	sx = (float)w / (float)screenW;

	// Find touch coordinates in terms of screen pixels
	float dispTouchX = (touchX * (float)dispW);
	float dispTouchY = (touchY * (float)dispH);

	*gameX = CLIP((int)((dispTouchX - x) / sx), 0, screenW);
	*gameY = CLIP((int)((dispTouchY - y) / sy), 0, screenH);
}

void SwitchEventSource::finishSimulatedMouseClicks() {
	for (int i = 0; i < 2; i++) {
		if (_simulatedClickStartTime[i] != 0) {
			uint32 currentTime = SDL_GetTicks();
			if (currentTime - _simulatedClickStartTime[i] >= SIMULATED_CLICK_DURATION) {
				int simulatedButton;
				if (i == 0) {
					simulatedButton = SDL_BUTTON_LEFT;
				} else {
					simulatedButton = SDL_BUTTON_RIGHT;
				}
				SDL_Event ev;
				ev.type = SDL_MOUSEBUTTONUP;
				ev.button.button = simulatedButton;
				ev.button.x = _mouseX;
				ev.button.y = _mouseY;
				SDL_PushEvent(&ev);

				_simulatedClickStartTime[i] = 0;
			}
		}
	}
}
#endif
