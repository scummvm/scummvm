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

#include "common/scummsys.h"

#ifdef MAEMO

#include "backends/events/maemosdl/maemosdl-events.h"

bool MaemoSdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {

	// List of special N810 keys:
	// SDLK_F4 -> menu
	// SDLK_F5 -> home
	// SDLK_F6 -> fullscreen
	// SDLK_F7 -> zoom +
	// SDLK_F8 -> zoom -

	switch (ev.type) {
		case SDL_KEYDOWN:{
			if (ev.key.keysym.sym == SDLK_F4) {
				event.type = Common::EVENT_MAINMENU;
				return true;
			}
			break;
		}
		case SDL_KEYUP: {
			if (ev.key.keysym.sym == SDLK_F4) {
				event.type = Common::EVENT_MAINMENU;
				return true;
			}
			break;
		}
	}

	// Invoke parent implementation of this method
	return SdlEventSource::remapKey(ev, event);
}

#endif
