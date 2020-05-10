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

#if defined(PLAYSTATION3)

#include "backends/events/ps3sdl/ps3sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "engines/engine.h"

#include "common/util.h"
#include "common/events.h"

/**
 * The XMB (PS3 in game menu) needs the screen buffers to be constantly flip while open.
 * This pauses execution and keeps redrawing the screen until the XMB is closed.
 */
void PS3SdlEventSource::preprocessEvents(SDL_Event *event) {
	if (event->type == SDL_APP_DIDENTERBACKGROUND) {
		PauseToken pt;
		// XMB opened
		if (g_engine)
			pt = g_engine->pauseEngine();

		for (;;) {
			if (!SDL_PollEvent(event)) {
				// Locking the screen forces a full redraw
				Graphics::Surface* screen = g_system->lockScreen();
				if (screen) {
					g_system->unlockScreen();
					g_system->updateScreen();
				}
				SDL_Delay(10);
				continue;
			}
			if (event->type == SDL_QUIT)
				return;
			if (event->type == SDL_APP_DIDENTERFOREGROUND) {
				// XMB closed
				return;
			}
		}
	}
}

#endif
