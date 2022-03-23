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

#include "common/system.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/shared/util/geometry.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

using namespace AGS::Engine;

// ----------------------------------------------------------------------------
// INIT / SHUTDOWN
// ----------------------------------------------------------------------------

int sys_main_init(/*config*/) {
	return 0;
}

void sys_main_shutdown() {
	sys_window_destroy();
}

void sys_set_background_mode(bool on) {
	// TODO: consider if we want any implementation here, and what...
}


// ----------------------------------------------------------------------------
// DISPLAY UTILS
// ----------------------------------------------------------------------------
#ifdef TODO
const int DEFAULT_DISPLAY_INDEX = 0; // TODO: is this always right?
#endif

int sys_get_desktop_resolution(int &width, int &height) {
	// TODO: ScummVM has a hardcoded dummy desktop resolution. See if there's any
	// need to change the values, given we're hardcoded for pretend full-screen
	width = 9999;
	height = 9999;

	return 0;
}

void sys_get_desktop_modes(std::vector<AGS::Engine::DisplayMode> &dms) {
#ifdef TODO
	SDL_DisplayMode mode;
	const int display_id = DEFAULT_DISPLAY_INDEX;
	const int count = SDL_GetNumDisplayModes(display_id);
	dms.clear();
	for (int i = 0; i < count; ++i) {
		if (SDL_GetDisplayMode(display_id, i, &mode) != 0) {
			SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
			continue;
		}
		AGS::Engine::DisplayMode dm;
		dm.Width = mode.w;
		dm.Height = mode.h;
		dm.ColorDepth = SDL_BITSPERPIXEL(mode.format);
		dm.RefreshRate = mode.refresh_rate;
		dms.push_back(dm);
	}
#endif
}


// ----------------------------------------------------------------------------
// WINDOW UTILS
// ----------------------------------------------------------------------------
// TODO: support multiple windows? in case we need some for diag purposes etc

#ifdef TODO
static SDL_Window *window = nullptr;

SDL_Window *sys_window_create(const char *window_title, int w, int h, WindowMode mode, int ex_flags) {
	if (window) {
		sys_window_destroy();
	}
	// TODO: support display index selection (?)
	Uint32 flags = 0;
	switch (mode) {
	case kWnd_Windowed: flags |= SDL_WINDOW_RESIZABLE; break;
	case kWnd_Fullscreen: flags |= SDL_WINDOW_FULLSCREEN; break;
	case kWnd_FullDesktop: flags |= SDL_WINDOW_FULLSCREEN_DESKTOP; break;
	}
	flags |= ex_flags;
	window = SDL_CreateWindow(
		window_title,
		SDL_WINDOWPOS_CENTERED_DISPLAY(DEFAULT_DISPLAY_INDEX),
		SDL_WINDOWPOS_CENTERED_DISPLAY(DEFAULT_DISPLAY_INDEX),
		w,
		h,
		flags
	);
	return window;
}
#else
SDL_Window *sys_window_create(const char *window_title, int w, int h, bool windowed, int ex_flags) {
	error("TODO: sys_window_create");
	return nullptr;
}
#endif

SDL_Window *sys_get_window() {
	//return window;
	return nullptr;
}

void sys_window_set_style(WindowMode mode, int /*ex_flags*/) {
#ifdef TODO
	if (!window) return;
	Uint32 flags = 0;
	switch (mode) {
	case kWnd_Fullscreen: flags = SDL_WINDOW_FULLSCREEN; break;
	case kWnd_FullDesktop: flags = SDL_WINDOW_FULLSCREEN_DESKTOP; break;
}
	SDL_SetWindowFullscreen(window, flags);
#endif
}

void sys_window_show_cursor(bool on) {
	g_system->showMouse(on);
}

bool sys_window_lock_mouse(bool on) {
#ifdef TODO
	if (!window) return false;
	SDL_SetWindowGrab(window, static_cast<SDL_bool>(on));
	return on; // TODO: test if successful?
#endif
	return false;
}

void sys_window_set_mouse(int x, int y) {
	g_system->warpMouse(x, y);
}

void sys_window_destroy() {
#ifdef TODO
	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}
#endif
}

void sys_window_set_title(const char *title) {
	// No implementation in ScummVM
}

void sys_window_set_icon() {
	// No implementation in ScummVM
}

bool sys_window_set_size(int w, int h, bool center) {
	error("TODO: sys_window_set_size");
	return false;
}

#if AGS_PLATFORM_OS_WINDOWS
void *sys_win_get_window() {
	if (!window) return nullptr;
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;
	return hwnd;
}
#endif

} // namespace AGS3
