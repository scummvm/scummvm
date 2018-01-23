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
		for (int j = 0; j < 2; j++) {
			_finger[i][j].id = -1;
		}
	}
}

void PSP2EventSource::preprocessEvents(SDL_Event *event) {

	// prevent suspend (scummvm games contains a lot of cutscenes..)
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);

	// left mouse click gesture: single finger short tap
	// right mouse click gesture: second finger short tap while first finger is still down
	if (event->type == SDL_FINGERDOWN || event->type == SDL_FINGERUP || event->type == SDL_FINGERMOTION) {
		// front (0) or back (1)  panel?
		SDL_TouchID port = event->tfinger.touchId;
		// which touchID (for multitouch)?
		SDL_FingerID id = event->tfinger.fingerId;

		int numFingersDown = 0;
		for (int j = 0; j < 2; j++) {
			if (_finger[port][j].id >= 0) {
				numFingersDown++;
			}
		}

		if (port < SCE_TOUCH_PORT_MAX_NUM && port >= 0) {
			if (event->type == SDL_FINGERDOWN) {
				for (int i = 0; i < 2; i++) {
					if (_finger[port][i].id == -1 || i == 1) {
						_finger[port][i].id = id;
						_finger[port][i].timeLastDown = event->tfinger.timestamp;
						break;
					}
				}
			} else if (event->type == SDL_FINGERUP) {
				// 250 ms long tap is interpreted as right/left mouse click depending on number of fingers down
				for (int i = 0; i < 2; i++) {
					if (_finger[port][i].id == id) {

						_finger[port][i].id = -1;

						if ((event->tfinger.timestamp - _finger[port][i].timeLastDown) <= 250) {
							if (numFingersDown == 2 || numFingersDown == 1) {
								Uint8 simulatedButton = 0;
								if (numFingersDown == 2) {
									simulatedButton = SDL_BUTTON_RIGHT;
								} else if (numFingersDown == 1) {
									simulatedButton = SDL_BUTTON_LEFT;
								}

								// simulate button click due to tap gesture
								event->type = SDL_MOUSEBUTTONDOWN;
								event->button.button = simulatedButton;
								event->button.x = _km.x / MULTIPLIER;
								event->button.y = _km.y / MULTIPLIER;

								SDL_Event ev;
								ev.type = SDL_MOUSEBUTTONUP;
								ev.button.button = simulatedButton;
								ev.button.x = _km.x / MULTIPLIER;
								ev.button.y = _km.y / MULTIPLIER;
								SDL_PushEvent(&ev);
							}
						}
					}
				}
			} else if (event->type == SDL_FINGERMOTION && numFingersDown == 1) {

				// convert touch events to relative mouse pointer events
				Sint32 mouse_x = _km.x / MULTIPLIER + (event->tfinger.dx * _km.x_max);
				Sint32 mouse_y = _km.y / MULTIPLIER + (event->tfinger.dy * _km.y_max);

				if (mouse_x > _km.x_max) {
					mouse_x = _km.x_max;
				} else if (mouse_x < 0) {
					mouse_x = 0;
				}
				if (mouse_y > _km.y_max) {
					mouse_y = _km.y_max;
				} else if (mouse_y < 0) {
					mouse_y = 0;
				}

				event->type = SDL_MOUSEMOTION;
				event->motion.x = mouse_x;
				event->motion.y = mouse_y;
			}
		}
	}
}

#endif
