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

#if defined(MAEMO)

#include "common/scummsys.h"

#include "backends/events/maemosdl/maemosdl-events.h"
#include "backends/platform/maemo/maemo.h"
#include "common/translation.h"

namespace Maemo {

MaemoSdlEventSource::MaemoSdlEventSource() : SdlEventSource(), _clickEnabled(true) {

}

struct KeymapEntry {
	SDL_Keycode sym;
	Common::KeyCode keycode;
	uint16 ascii;
};

static const KeymapEntry keymapEntries[] = {
	{SDLK_F4, Common::KEYCODE_F11, 0},
	{SDLK_F5, Common::KEYCODE_F12, 0},
	{SDLK_F6, Common::KEYCODE_F13, 0},
	{SDLK_F7, Common::KEYCODE_F14, 0},
	{SDLK_F8, Common::KEYCODE_F15, 0},
	{SDLK_LAST, Common::KEYCODE_INVALID, 0}
};

bool MaemoSdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {

	// List of special N810 keys:
	// SDLK_F4 -> menu
	// SDLK_F5 -> home
	// SDLK_F6 -> fullscreen
	// SDLK_F7 -> zoom +
	// SDLK_F8 -> zoom -

	if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
		const KeymapEntry *entry;
		for (entry = keymapEntries; entry->sym != SDLK_LAST; ++entry) {
			if (ev.key.keysym.sym == entry->sym) {
				SDLModToOSystemKeyFlags(SDL_GetModState(), event);
				event.type = ev.type == SDL_KEYDOWN ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
				event.kbd.keycode = entry->keycode;
				event.kbd.ascii = entry->ascii;
				return true;
			}
		}
	}

	// Invoke parent implementation of this method
	return SdlEventSource::remapKey(ev, event);
}

bool MaemoSdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {

	if (ev.button.button == SDL_BUTTON_LEFT && !_clickEnabled) {
		return false;
	}

	// Invoke parent implementation of this method
	return SdlEventSource::handleMouseButtonDown(ev, event);
}

bool MaemoSdlEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {

	if (ev.button.button == SDL_BUTTON_LEFT && !_clickEnabled) {
		return false;
	}

	// Invoke parent implementation of this method
	return SdlEventSource::handleMouseButtonUp(ev, event);
}

bool MaemoSdlEventSource::toggleClickMode() {
	_clickEnabled = !_clickEnabled;
	_graphicsManager->displayMessageOnOSD(
	  _clickEnabled ? _("Clicking Enabled") : _("Clicking Disabled"));

	return _clickEnabled;
}

MaemoSdlEventObserver::MaemoSdlEventObserver(MaemoSdlEventSource *eventSource) {
	assert(eventSource);
	_eventSource = eventSource;
}

bool MaemoSdlEventObserver::notifyEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_START)
		return false;
	if (event.customType == kEventClickMode) {
		assert(_eventSource);
		_eventSource->toggleClickMode();
		return true;
	}
	return false;
}

} // namespace Maemo

#endif // if defined(MAEMO)
