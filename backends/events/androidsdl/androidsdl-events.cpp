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

#if defined(ANDROIDSDL)

#include "backends/events/androidsdl/androidsdl-events.h"
#include "backends/platform/androidsdl/androidsdl-sdl.h"

bool AndroidSdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
#if defined(SDL_BUTTON_MIDDLE)
	if (ev.button.button == SDL_BUTTON_MIDDLE) {
		const bool show_onscreen = g_system->getFeatureState(OSystem::kFeatureOnScreenControl);
		g_system->setFeatureState(OSystem::kFeatureOnScreenControl, !show_onscreen);
	}
#endif

	return SdlEventSource::handleMouseButtonDown(ev, event);
}

bool AndroidSdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {
	if (false) {}

	if (ev.key.keysym.sym == SDLK_F13) {
		event.type = Common::EVENT_MAINMENU;
		return true;
	} else {
		// Let the events fall through if we didn't change them, this may not be the best way to
		// set it up, but i'm not sure how sdl would like it if we let if fall through then redid it though.
		// and yes i have an huge terminal size so i dont wrap soon enough.
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = (Common::KeyCode)ev.key.keysym.sym;
		event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
	}

	return false;
}

int AndroidSdlEventSource::mapKey(SDL_Keycode sdlKey, SDL_Keymod mod, Uint16 unicode) {
	Common::KeyCode key = SDLToOSystemKeycode(sdlKey);

	if (key >= Common::KEYCODE_F1 && key <= Common::KEYCODE_F9) {
		return key - Common::KEYCODE_F1 + Common::ASCII_F1;
	} else if (key >= Common::KEYCODE_KP0 && key <= Common::KEYCODE_KP9) {
		return key - Common::KEYCODE_KP0 + '0';
	} else if (key >= Common::KEYCODE_UP && key <= Common::KEYCODE_PAGEDOWN) {
		return key;
	} else if (key >= 'a' && key <= 'z' && (mod & KMOD_SHIFT)) {
		return key & ~0x20;
	} else if (key >= Common::KEYCODE_NUMLOCK && key < Common::KEYCODE_LAST) {
		return 0;
	} else {
		return key;
	}
}

#endif
