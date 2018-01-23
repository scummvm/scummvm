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

#include "common/scummsys.h"

#if defined(PSP2)

#include <psp2/kernel/processmgr.h>
#include <psp2/touch.h>
#include "backends/platform/sdl/psp2/psp2.h"
#include "backends/events/psp2sdl/psp2sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "engines/engine.h"

#include "common/util.h"
#include "common/events.h"
#include "common/config-manager.h"

#include "math.h"

PSP2EventSource::PSP2EventSource() {
	for (int i = 0; i < SCE_TOUCH_PORT_MAX_NUM; i++) {
		for (int j = 0; j < MAX_NUM_FINGERS; j++) {
			_finger[i][j].id = -1;
		}
	}
	// need specs of front panel for accurate direct touch
	sceTouchGetPanelInfo(0, &panelInfo);
}

void PSP2EventSource::preprocessEvents(SDL_Event *event) {

	// prevent suspend (scummvm games contain a lot of cutscenes..)
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);

	// Supported touch gestures:
	// left mouse click: single finger short tap
	// right mouse click: second finger short tap while first finger is still down
	// pointer motion: single finger drag
	if (event->type == SDL_FINGERDOWN || event->type == SDL_FINGERUP || event->type == SDL_FINGERMOTION) {
		// front (0) or back (1) panel
		SDL_TouchID port = event->tfinger.touchId;
		if (port < SCE_TOUCH_PORT_MAX_NUM && port >= 0) {
			switch (event->type) {
			case SDL_FINGERDOWN:
				preprocessFingerDown(event);
				break;
			case SDL_FINGERUP:
				preprocessFingerUp(event);
				break;
			case SDL_FINGERMOTION:
				preprocessFingerMotion(event);
				break;
			}
		}
	}
}

void PSP2EventSource::preprocessFingerDown(SDL_Event *event) {
	// front (0) or back (1) panel
	SDL_TouchID port = event->tfinger.touchId;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id >= 0) {
			numFingersDown++;
		}
	}

	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id == -1) {
			_finger[port][i].id = id;
			_finger[port][i].timeLastDown = event->tfinger.timestamp;
			break;
		}
	}
}

void PSP2EventSource::preprocessFingerUp(SDL_Event *event) {
	// front (0) or back (1) panel
	SDL_TouchID port = event->tfinger.touchId;
	// id (for multitouch)
	SDL_FingerID id = event->tfinger.fingerId;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id >= 0) {
			numFingersDown++;
		}
	}

	int x = _km.x / MULTIPLIER;
	int y = _km.y / MULTIPLIER;

	if (!ConfMan.getBool("touchpad_mouse_mode") && port == 0) {
		convertTouchToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
	}

	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id == id) {
			_finger[port][i].id = -1;
			if ((event->tfinger.timestamp - _finger[port][i].timeLastDown) <= 250) {
				// short (<250 ms) tap is interpreted as right/left mouse click depending on # fingers already down
				if (numFingersDown == 2 || numFingersDown == 1) {
					Uint8 simulatedButton = 0;
					if (numFingersDown == 2) {
						simulatedButton = SDL_BUTTON_RIGHT;
					} else if (numFingersDown == 1) {
						simulatedButton = SDL_BUTTON_LEFT;
					}

					event->type = SDL_MOUSEBUTTONDOWN;
					event->button.button = simulatedButton;
					event->button.x = x;
					event->button.y = y;

					SDL_Event ev;
					ev.type = SDL_MOUSEBUTTONUP;
					ev.button.button = simulatedButton;
					ev.button.x = x;
					ev.button.y = y;
					SDL_PushEvent(&ev);
				}
			}
		}
	}
}

void PSP2EventSource::preprocessFingerMotion(SDL_Event *event) {
	// front (0) or back (1) panel
	SDL_TouchID port = event->tfinger.touchId;

	// find out how many fingers were down before this event
	int numFingersDown = 0;
	for (int i = 0; i < MAX_NUM_FINGERS; i++) {
		if (_finger[port][i].id >= 0) {
			numFingersDown++;
		}
	}

	if (numFingersDown == 1) {

		int x = _km.x / MULTIPLIER;;
		int y = _km.y / MULTIPLIER;

		if (!ConfMan.getBool("touchpad_mouse_mode") && port == 0) {
			convertTouchToGameXY(event->tfinger.x, event->tfinger.y, &x, &y);
		} else {
			// for relative mode, use the pointer speed setting
			float speedFactor = 1.0;

			switch (ConfMan.getInt("kbdmouse_speed")) {
			// 0.25 keyboard pointer speed
			case 0:
				speedFactor = 0.25;
				break;
			// 0.5 speed
			case 1:
				speedFactor = 0.5;
				break;
			// 0.75 speed
			case 2:
				speedFactor = 0.75;
				break;
			// 1.0 speed
			case 3:
				speedFactor = 1.0;
				break;
			// 1.25 speed
			case 4:
				speedFactor = 1.25;
				break;
			// 1.5 speed
			case 5:
				speedFactor = 1.5;
				break;
			// 1.75 speed
			case 6:
				speedFactor = 1.75;
				break;
			// 2.0 speed
			case 7:
				speedFactor = 2.0;
				break;
			default:
				speedFactor = 1.0;
			}

			// convert touch events to relative mouse pointer events
			// Whenever an SDL_event involving the mouse is processed,
			// _km.x/y are truncated from subpixel precision to regular pixel precision.
			// Therefore, there's no need here to deal with subpixel precision in _km.x/y.
			x = (_km.x / MULTIPLIER + (event->tfinger.dx * 1.25 * speedFactor * _km.x_max));
			y = (_km.y / MULTIPLIER + (event->tfinger.dy * 1.25 * speedFactor * _km.y_max));
		}

		if (x > _km.x_max) {
			x = _km.x_max;
		} else if (x < 0) {
			x = 0;
		}
		if (y > _km.y_max) {
			y = _km.y_max;
		} else if (y < 0) {
			y = 0;
		}

		event->type = SDL_MOUSEMOTION;
		event->motion.x = x;
		event->motion.y = y;
	}
}

void PSP2EventSource::convertTouchToGameXY(float touchX, float touchY, int *gameX, int *gameY) {
	// Find touch coordinates in terms of Vita screen pixels
	float aaTouchX = touchX * (panelInfo.maxAaX - panelInfo.minAaX) + panelInfo.minAaX;
	float aaTouchY = touchY * (panelInfo.maxAaY - panelInfo.minAaY) + panelInfo.minAaY;

	float screenTouchX = (aaTouchX - panelInfo.minDispX) * 960 / panelInfo.maxDispX;
	float screenTouchY = (aaTouchY - panelInfo.minDispY) * 544 / panelInfo.maxDispY;

	// Find four corners of game screen in Vita screen coordinates
	// This depends on the fullscreen and aspect ratio correction settings (at least on Vita)

	float gameXMin = 0;
	float gameXMax = 0;
	float gameYMin = 0;
	float gameYMax = 0;
	float aspectRatio = 4.0 / 3.0;

	// vertical
	if (ConfMan.getBool("fullscreen")) {
		gameYMin = 0.0;
		gameYMax = 544.0;
	} else {
		if (_km.y_max <= 272) {
			gameYMin = (544.0 - (_km.y_max * 2.0)) / 2.0;
			gameYMax = 544.0 - gameYMin;
		} else {
			gameYMin = (544.0 - (_km.y_max)) / 2.0;
			gameYMax = 544 - gameYMin;
		}
	}
	// horizontal
	if (ConfMan.getBool("aspect_ratio")) {
		aspectRatio = 4.0/3.0;
	} else {
		aspectRatio = (float)_km.x_max / (float)_km.y_max;
	}
	gameXMin = (960 - (gameYMax - gameYMin) * aspectRatio) / 2.0;
	gameXMax = 960.0 - gameXMin;

	// find game pixel coordinates corresponding to front panel touch coordinates
	int x = (screenTouchX - gameXMin) / (gameXMax - gameXMin) * _km.x_max;
	int y = (screenTouchY - gameYMin) / (gameYMax - gameYMin) * _km.y_max;

	if (x < 0) {
		x = 0;
	} else if (x > _km.x_max) {
		x = _km.x_max;
	} else if (y < 0) {
		y = 0;
	} else if (y > _km.y_max) {
		y = _km.y_max;
	}
	*gameX = x;
	*gameY = y;
}

#endif
