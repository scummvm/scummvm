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

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv(a)

#include "common/config-manager.h"

#include "backends/platform/androidsdl/androidsdl-sdl.h"
#include "backends/events/androidsdl/androidsdl-events.h"
#include <SDL_android.h>
#include <SDL_screenkeyboard.h>

void OSystem_ANDROIDSDL::initBackend() {
	// Create the backend custom managers

	if (_eventSource == 0)
		_eventSource = new AndroidSdlEventSource();

	if (!ConfMan.hasKey("browser_lastpath") || (ConfMan.hasKey("browser_lastpath") && (ConfMan.get("browser_lastpath") == "/storage")))
		ConfMan.set("browser_lastpath", getenv("SDCARD"));

	if (!ConfMan.hasKey("gfx_mode"))
		ConfMan.set("gfx_mode", "2x");

	if (!ConfMan.hasKey("swap_menu_and_back_buttons"))
		ConfMan.setBool("swap_menu_and_back_buttons", true);
	else
		swapMenuAndBackButtons(ConfMan.getBool("swap_menu_and_back_buttons"));

	if (!ConfMan.hasKey("touchpad_mouse_mode")) {
		const bool enable = SDL_ANDROID_GetMouseEmulationMode();
		ConfMan.setBool("touchpad_mouse_mode", enable);
	} else
		touchpadMode(ConfMan.getBool("touchpad_mouse_mode"));

	if (!ConfMan.hasKey("onscreen_control")) {
		const bool enable = SDL_ANDROID_GetScreenKeyboardShown();
		ConfMan.setBool("onscreen_control", enable);
	} else
		showOnScreenControl(ConfMan.getBool("onscreen_control"));

	// Call parent implementation of this method
	OSystem_POSIX::initBackend();
}

void OSystem_ANDROIDSDL::showOnScreenControl(bool enable) {
	if (enable)
		SDL_ANDROID_SetScreenKeyboardShown(1);
	else
		SDL_ANDROID_SetScreenKeyboardShown(0);
}

void OSystem_ANDROIDSDL::touchpadMode(bool enable) {
	if (enable)
		switchToRelativeMouseMode();
	else
		switchToDirectMouseMode();
}

void OSystem_ANDROIDSDL::swapMenuAndBackButtons(bool enable) {
	static int KEYCODE_MENU = 82;
	static int KEYCODE_BACK = 4;
	if (enable) {
		SDL_ANDROID_SetAndroidKeycode(KEYCODE_BACK, SDLK_F13);
		SDL_ANDROID_SetAndroidKeycode(KEYCODE_MENU, SDLK_ESCAPE);
	} else {
		SDL_ANDROID_SetAndroidKeycode(KEYCODE_BACK, SDLK_ESCAPE);
		SDL_ANDROID_SetAndroidKeycode(KEYCODE_MENU, SDLK_F13);
	}
}

void OSystem_ANDROIDSDL::switchToDirectMouseMode() {
	SDL_ANDROID_SetMouseEmulationMode(0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
}

void OSystem_ANDROIDSDL::switchToRelativeMouseMode() {
	SDL_ANDROID_SetMouseEmulationMode(1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
}

void OSystem_ANDROIDSDL::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureTouchpadMode:
		ConfMan.setBool("touchpad_mouse_mode", enable);
		touchpadMode(enable);
		break;
	case kFeatureOnScreenControl:
		ConfMan.setBool("onscreen_control", enable);
		showOnScreenControl(enable);
		break;
	case kFeatureSwapMenuAndBackButtons:
		ConfMan.setBool("swap_menu_and_back_buttons", enable);
		swapMenuAndBackButtons(enable);
		break;
	default:
		OSystem_POSIX::setFeatureState(f, enable);
		break;
	}
}

bool OSystem_ANDROIDSDL::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureTouchpadMode:
		return ConfMan.getBool("touchpad_mouse_mode");
		break;
	case kFeatureOnScreenControl:
		return ConfMan.getBool("onscreen_control");
		break;
	case kFeatureSwapMenuAndBackButtons:
		return ConfMan.getBool("swap_menu_and_back_buttons");
		break;
	default:
		return OSystem_POSIX::getFeatureState(f);
		break;
	}
}

bool OSystem_ANDROIDSDL::hasFeature(Feature f) {
	if (f == kFeatureFullscreenMode)
		return false;
	return (f == kFeatureTouchpadMode ||
			f == kFeatureOnScreenControl ||
			f == kFeatureSwapMenuAndBackButtons ||
			f == OSystem_POSIX::getFeatureState(f));
}
