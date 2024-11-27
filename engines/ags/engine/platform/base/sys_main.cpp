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

void sys_set_background_mode(bool /*on*/) {
	// TODO: consider if we want any implementation here, and what...
}


// ----------------------------------------------------------------------------
// DISPLAY UTILS
// ----------------------------------------------------------------------------

const int DEFAULT_DISPLAY_INDEX = 0;

int sys_get_window_display_index() {
#if (AGS_PLATFORM_DESKTOP && !AGS_PLATFORM_SCUMMVM)
	int index = -1;
	SDL_Window *window = sys_get_window();
	if (window)
		index = SDL_GetWindowDisplayIndex(window);
	return index >= 0 ? index : DEFAULT_DISPLAY_INDEX;
#else
	return DEFAULT_DISPLAY_INDEX;
#endif
}

int sys_get_desktop_resolution(int &width, int &height) {
	// TODO: ScummVM has a hardcoded dummy desktop resolution. See if there's any
	// need to change the values, given we're hardcoded for pretend full-screen
	width = 9999;
	height = 9999;

	return 0;
}

void sys_get_desktop_modes(std::vector<AGS::Engine::DisplayMode> &dms, int color_depth) {
#ifdef TODO
	SDL_DisplayMode mode;
	const int display_id = sys_get_window_display_index();
	const int count = SDL_GetNumDisplayModes(display_id);
	dms.clear();
	for (int i = 0; i < count; ++i) {
		if (SDL_GetDisplayMode(display_id, i, &mode) != 0) {
			SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
			continue;
		}
		const int bitsdepth = SDL_BITSPERPIXEL(mode.format);
		if ((color_depth == 0) || (bitsdepth != color_depth)) {
			continue;
		}
		AGS::Engine::DisplayMode dm;
		dm.Width = mode.w;
		dm.Height = mode.h;
		dm.ColorDepth = bitsdepth;
		dm.RefreshRate = mode.refresh_rate;
		dms.push_back(dm);
	}
#endif
}

void sys_renderer_set_output(const AGS::Shared::String &name) {
#ifndef AGS_PLATFORM_SCUMMVM
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, name.GetCStr());
#endif
}

// ----------------------------------------------------------------------------
// AUDIO UTILS
// ----------------------------------------------------------------------------

bool sys_audio_init(const AGS::Shared::String &driver_name) {
#ifdef AGS_PLATFORM_SCUMMVM
	return true;
#else
	// IMPORTANT: we must use a combination of SDL_setenv and SDL_InitSubSystem
	// here, and NOT use SDL_AudioInit, because SDL_AudioInit does not increment
	// subsystem's reference count. Which in turn may cause problems down the
	// way when initializing any additional SDL-based audio lib or plugin;
	// at the very least - the mojoAl (OpenAL's implementation we're using).
	bool res = false;
	// If user config contained a driver request, then apply one for a try
	if (!driver_name.IsEmpty())
		SDL_setenv("SDL_AUDIODRIVER", driver_name.GetCStr(), 1);
	const char *env_drv = SDL_getenv("SDL_AUDIODRIVER");
	Debug::Printf("Requested audio driver: %s", env_drv ? env_drv : "default");
	res = SDL_InitSubSystem(SDL_INIT_AUDIO) == 0;
	// If there have been an explicit request that failed, then try to force
	// SDL to go through a list of supported drivers and see if that succeeds.
	if (!res && env_drv) {
		Debug::Printf(kDbgMsg_Error, "Failed to initialize requested audio driver '%s'; error: %s", env_drv, SDL_GetError());
		Debug::Printf("Attempt to initialize any audio driver from the known list");
		SDL_setenv("SDL_AUDIODRIVER", "", 1);
		res = SDL_InitSubSystem(SDL_INIT_AUDIO) == 0;
	}
	if (res)
		Debug::Printf(kDbgMsg_Info, "Audio driver: %s", SDL_GetCurrentAudioDriver());
	else
		Debug::Printf(kDbgMsg_Error, "Failed to initialize any audio driver; error: %s", SDL_GetError());
	return res;
#endif
}

void sys_audio_shutdown() {
#ifndef AGS_PLATFORM_SCUMMVM
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
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
#if (AGS_PLATFORM_MOBILE)
	// Resizable flag is necessary for fullscreen app rotation
	flags |= SDL_WINDOW_RESIZABLE;
#endif
	window = SDL_CreateWindow(
		window_title,
		SDL_WINDOWPOS_CENTERED_DISPLAY(DEFAULT_DISPLAY_INDEX),
		SDL_WINDOWPOS_CENTERED_DISPLAY(DEFAULT_DISPLAY_INDEX),
		w,
		h,
		flags
	);
#if (AGS_PLATFORM_DESKTOP)
	// CHECKME: this is done because SDL2 has some bug(s) during
	// centering. See: https://github.com/libsdl-org/SDL/issues/6875
	// TODO: SDL2 docs mentioned that on some systems the window border size
	// may be known only after the window is displayed, which means that
	// this may have to be called with a short delay (but how to know when?)
	if (mode == kWnd_Windowed)
		sys_window_center();
#endif
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

void sys_window_center(int display_index) {
	// No implementation in ScummVM
}

void sys_window_fit_in_display(int display_index) {
	// No implementation in ScummVM
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
