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

#if defined(MAEMO)

#include "common/scummsys.h"

#include "backends/events/maemosdl/maemosdl-events.h"
#include "common/translation.h"

namespace Maemo {

MaemoSdlEventSource::MaemoSdlEventSource() : SdlEventSource(), _clickEnabled(true) {

}

bool MaemoSdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {

	Model model = Model(((OSystem_SDL_Maemo *)g_system)->getModel());
	debug(10, "Model: %s %u %s %s", model.hwId, model.modelType, model.hwAlias, model.hwKeyboard ? "true" : "false");

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
				debug(9, "remapping to main menu");
				return true;
			} else if (ev.key.keysym.sym == SDLK_F6) {
				if (!model.hwKeyboard) {
					event.type = Common::EVENT_KEYDOWN;
					event.kbd.keycode = Common::KEYCODE_F7;
					event.kbd.ascii = Common::ASCII_F7;
					event.kbd.flags = 0;
					debug(9, "remapping to F7 down (virtual keyboard)");
					return true;
				} else {
					// handled in keyup
				}
			} else if (ev.key.keysym.sym == SDLK_F7) {
				event.type = Common::EVENT_RBUTTONDOWN;
				processMouseEvent(event, _km.x, _km.y);
				 debug(9, "remapping to right click down");
				return true;
			} else if (ev.key.keysym.sym == SDLK_F8) {
				if (ev.key.keysym.mod & KMOD_CTRL) {
					event.type = Common::EVENT_KEYDOWN;
					event.kbd.keycode = Common::KEYCODE_F7;
					event.kbd.ascii = Common::ASCII_F7;
					event.kbd.flags = 0;
					debug(9, "remapping to F7 down (virtual keyboard)");
					return true;
				} else {
					// handled in keyup
					return true;
				}
			}
			break;
		}
		case SDL_KEYUP: {
			if (ev.key.keysym.sym == SDLK_F4) {
				event.type = Common::EVENT_MAINMENU;
				return true;
			} else if (ev.key.keysym.sym == SDLK_F6) {
				if (!model.hwKeyboard) {
					event.type = Common::EVENT_KEYUP;
					event.kbd.keycode = Common::KEYCODE_F7;
					event.kbd.ascii = Common::ASCII_F7;
					event.kbd.flags = 0;
					debug(9, "remapping to F7 down (virtual keyboard)");
					return true;
				} else {
					bool currentState = ((OSystem_SDL *)g_system)->getGraphicsManager()->getFeatureState(OSystem::kFeatureFullscreenMode);
					g_system->beginGFXTransaction();
					((OSystem_SDL *)g_system)->getGraphicsManager()->setFeatureState(OSystem::kFeatureFullscreenMode, !currentState);
					g_system->endGFXTransaction();
					debug(9, "remapping to full screen toggle");
					return true;
				}
			} else if (ev.key.keysym.sym == SDLK_F7) {
				event.type = Common::EVENT_RBUTTONUP;
				processMouseEvent(event, _km.x, _km.y);
					debug(9, "remapping to right click up");
				return true;
			} else if (ev.key.keysym.sym == SDLK_F8) {
				if (ev.key.keysym.mod & KMOD_CTRL) {
					event.type = Common::EVENT_KEYUP;
					event.kbd.keycode = Common::KEYCODE_F7;
					event.kbd.ascii = Common::ASCII_F7;
					event.kbd.flags = 0;
					debug(9, "remapping to F7 up (virtual keyboard)");
					return true;
				} else {
					_clickEnabled = !_clickEnabled;
					((SurfaceSdlGraphicsManager*) _graphicsManager)->displayMessageOnOSD(
					  _clickEnabled ? _("Clicking Enabled") : _("Clicking Disabled"));
					debug(9, "remapping to click toggle");
					return true;
				}
			}
			break;
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

} // namespace Maemo

#endif // if defined(MAEMO)
