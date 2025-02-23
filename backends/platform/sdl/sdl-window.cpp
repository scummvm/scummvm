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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/sdl/sdl-window.h"

#include "common/textconsole.h"
#include "common/util.h"
#include "common/config-manager.h"

#include "icons/scummvm.xpm"

#if SDL_VERSION_ATLEAST(3, 0, 0)
static const uint32 fullscreenMask = SDL_WINDOW_FULLSCREEN;
#elif SDL_VERSION_ATLEAST(2, 0, 0)
static const uint32 fullscreenMask = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN;
#endif

SdlWindow::SdlWindow() :
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_window(nullptr), _windowCaption("ScummVM"),
	_lastFlags(0), _lastX(SDL_WINDOWPOS_UNDEFINED), _lastY(SDL_WINDOWPOS_UNDEFINED),
#endif
	_inputGrabState(false), _inputLockState(false),
	_resizable(true)
	{
		memset(&grabRect, 0, sizeof(grabRect));

#if SDL_VERSION_ATLEAST(2, 0, 0)
#elif SDL_VERSION_ATLEAST(1, 2, 10)
	// Query the desktop resolution. We simply hope nothing tried to change
	// the resolution so far.
	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	if (videoInfo && videoInfo->current_w > 0 && videoInfo->current_h > 0) {
		_desktopRes = Common::Rect(videoInfo->current_w, videoInfo->current_h);
	}
#elif defined(MAEMO)
	// All supported Maemo devices have a display resolution of 800x480
	_desktopRes = Common::Rect(800, 480);
#elif defined(KOLIBRIOS)
	// TODO: Use kolibriOS call to determine this.
	_desktopRes = Common::Rect(640, 480);
#else
#error Unable to detect screen resolution
#endif
}

SdlWindow::~SdlWindow() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	destroyWindow();
#endif
}

void SdlWindow::setupIcon() {
#ifndef __MORPHOS__
	int x, y, w, h, ncols, nbytes, i;
	unsigned int rgba[256];
	unsigned int *icon;

	if (sscanf(scummvm_icon[0], "%d %d %d %d", &w, &h, &ncols, &nbytes) != 4) {
		warning("Wrong format of scummvm_icon[0] (%s)", scummvm_icon[0]);

		return;
	}
	if ((w > 512) || (h > 512) || (ncols > 255) || (nbytes > 1)) {
		warning("Could not load the built-in icon (%d %d %d %d)", w, h, ncols, nbytes);
		return;
	}
	icon = (unsigned int*)malloc(w*h*sizeof(unsigned int));
	if (!icon) {
		warning("Could not allocate temp storage for the built-in icon");
		return;
	}

	for (i = 0; i < ncols; i++) {
		unsigned char code;
		char color[32];
		memset(color, 0, sizeof(color));
		unsigned int col;
		if (sscanf(scummvm_icon[1 + i], "%c c %s", &code, color) != 2) {
			warning("Wrong format of scummvm_icon[%d] (%s)", 1 + i, scummvm_icon[1 + i]);
		}
		if (!strcmp(color, "None"))
			col = 0x00000000;
		else if (!strcmp(color, "black"))
			col = 0xFF000000;
		else if (!strcmp(color, "gray20"))
			col = 0xFF333333;
		else if (color[0] == '#') {
			if (sscanf(color + 1, "%06x", &col) != 1) {
				warning("Wrong format of color (%s)", color + 1);
			}
			col |= 0xFF000000;
		} else {
			warning("Could not load the built-in icon (%d %s - %s) ", code, color, scummvm_icon[1 + i]);
			free(icon);
			return;
		}

		rgba[code] = col;
	}
	for (y = 0; y < h; y++) {
		const char *line = scummvm_icon[1 + ncols + y];
		for (x = 0; x < w; x++) {
			icon[x + w * y] = rgba[(int)line[x]];
		}
	}

#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_Surface *sdl_surf = SDL_CreateSurfaceFrom(w, h, SDL_GetPixelFormatForMasks(32, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000), icon, w * 4);
#else
	SDL_Surface *sdl_surf = SDL_CreateRGBSurfaceFrom(icon, w, h, 32, w * 4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
#endif
	if (!sdl_surf) {
		warning("SDL_CreateRGBSurfaceFrom(icon) failed");
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_window) {
		SDL_SetWindowIcon(_window, sdl_surf);
	}
#else
	SDL_WM_SetIcon(sdl_surf, NULL);
#endif

#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_DestroySurface(sdl_surf);
#else
	SDL_FreeSurface(sdl_surf);
#endif
	free(icon);
#endif
}

void SdlWindow::setWindowCaption(const Common::String &caption) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_windowCaption = caption;
	if (_window) {
		SDL_SetWindowTitle(_window, caption.c_str());
	}
#else
	SDL_WM_SetCaption(caption.c_str(), caption.c_str());
#endif
}

void SdlWindow::setResizable(bool resizable) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	if (_window) {
		SDL_SetWindowResizable(_window, resizable);
	}
#elif SDL_VERSION_ATLEAST(2, 0, 5)
	if (_window) {
		SDL_SetWindowResizable(_window, resizable ? SDL_TRUE : SDL_FALSE);
	}
#endif
	_resizable = resizable;
}

void SdlWindow::grabMouse(bool grab) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_window) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_SetWindowMouseGrab(_window, grab);
#else
		SDL_SetWindowGrab(_window, grab ? SDL_TRUE : SDL_FALSE);
#endif
#if SDL_VERSION_ATLEAST(2, 0, 18)
		SDL_SetWindowMouseRect(_window, grab ? &grabRect : NULL);
#endif
	}
	_inputGrabState = grab;
#else
	if (grab) {
		_inputGrabState = true;
		SDL_WM_GrabInput(SDL_GRAB_ON);
	} else {
		_inputGrabState = false;
		if (!_inputLockState)
			SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
#endif
}

void SdlWindow::setMouseRect(const Common::Rect &rect) {
	float dpiScale = getSdlDpiScalingFactor();
	grabRect.x = (int)(rect.left / dpiScale + 0.5f);
	grabRect.y = (int)(rect.top / dpiScale + 0.5f);
	grabRect.w = (int)(rect.width() / dpiScale + 0.5f);
	grabRect.h = (int)(rect.height() / dpiScale + 0.5f);

#if SDL_VERSION_ATLEAST(2, 0, 18)
	if (_inputGrabState || _lastFlags & fullscreenMask) {
		SDL_SetWindowMouseRect(_window, &grabRect);
	}
#endif
}

bool SdlWindow::lockMouse(bool lock) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_SetWindowRelativeMouseMode(_window, lock);
	_inputLockState = lock;
#elif SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetRelativeMouseMode(lock ? SDL_TRUE : SDL_FALSE);
	_inputLockState = lock;
#else
	if (lock) {
		_inputLockState = true;
		SDL_WM_GrabInput(SDL_GRAB_ON);
	} else {
		_inputLockState = false;
		if (!_inputGrabState)
			SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
#endif
	return true;
}

bool SdlWindow::hasMouseFocus() const {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_window) {
		return (SDL_GetWindowFlags(_window) & SDL_WINDOW_MOUSE_FOCUS);
	} else {
		return false;
	}
#else
	return (SDL_GetAppState() & SDL_APPMOUSEFOCUS);
#endif
}

bool SdlWindow::warpMouseInWindow(int x, int y) {
	if (hasMouseFocus()) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (_window) {
			float dpiScale = getSdlDpiScalingFactor();
			x = (int)(x / dpiScale + 0.5f);
			y = (int)(y / dpiScale + 0.5f);
			SDL_WarpMouseInWindow(_window, x, y);
			return true;
		}
#else
		SDL_WarpMouse(x, y);
		return true;
#endif
	}

	return false;
}

void SdlWindow::iconifyWindow() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_window) {
		SDL_MinimizeWindow(_window);
	}
#else
	SDL_WM_IconifyWindow();
#endif
}

#if !SDL_VERSION_ATLEAST(3, 0, 0)
bool SdlWindow::getSDLWMInformation(SDL_SysWMinfo *info) const {
	SDL_VERSION(&info->version);
#if SDL_VERSION_ATLEAST(2, 0, 0)
	return _window ? (SDL_GetWindowWMInfo(_window, info) == SDL_TRUE) : false;
#elif !defined(__MORPHOS__)
	return SDL_GetWMInfo(info);
#else
	return false;
#endif
}
#endif

Common::Rect SdlWindow::getDesktopResolution() {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	const SDL_DisplayMode* pDisplayMode = SDL_GetDesktopDisplayMode(getDisplayIndex());
	if (pDisplayMode) {
		_desktopRes = Common::Rect(pDisplayMode->w, pDisplayMode->h);
	} else {
		warning("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
	}
#elif SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_DisplayMode displayMode;
	if (!SDL_GetDesktopDisplayMode(getDisplayIndex(), &displayMode)) {
		_desktopRes = Common::Rect(displayMode.w, displayMode.h);
	}
#endif

	return _desktopRes;
}

void SdlWindow::getDisplayDpi(float *dpi, float *defaultDpi) const {
	const float systemDpi =
#ifdef __APPLE__
	72.0f;
#elif defined(_WIN32)
	96.0f;
#else
	90.0f; // ScummVM default
#endif
	if (defaultDpi)
		*defaultDpi = systemDpi;

	if (dpi) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
		*dpi = SDL_GetWindowDisplayScale(_window) * systemDpi;
#elif SDL_VERSION_ATLEAST(2, 0, 4)
		if (SDL_GetDisplayDPI(getDisplayIndex(), nullptr, dpi, nullptr) != 0) {
			*dpi = systemDpi;
		}
#else
		*dpi = systemDpi;
#endif
	}
}

float SdlWindow::getDpiScalingFactor() const {
	if (ConfMan.hasKey("forced_dpi_scaling"))
		return ConfMan.getInt("forced_dpi_scaling") / 100.f;

	float dpi, defaultDpi;
	getDisplayDpi(&dpi, &defaultDpi);
	float ratio = dpi / defaultDpi;
	debug(4, "Reported DPI: %g default: %g, ratio %g, clipped: %g", dpi, defaultDpi, ratio, CLIP(ratio, 1.0f, 4.0f));
	// Getting the DPI can be unreliable, so clamp the scaling factor to make sure
	// we do not return unreasonable values.
	return CLIP(ratio, 1.0f, 4.0f);
}

float SdlWindow::getSdlDpiScalingFactor() const {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	return SDL_GetWindowDisplayScale(getSDLWindow());
#elif SDL_VERSION_ATLEAST(2, 0, 0)
	int windowWidth, windowHeight;
	SDL_GetWindowSize(getSDLWindow(), &windowWidth, &windowHeight);
	int realWidth, realHeight;
	SDL_GL_GetDrawableSize(getSDLWindow(), &realWidth, &realHeight);
	return (float)realWidth / (float)windowWidth;
#else
	return 1.f;
#endif
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
SDL_Surface *copySDLSurface(SDL_Surface *src) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	const bool locked = SDL_MUSTLOCK(src);

	if (locked) {
		if (!SDL_LockSurface(src)) {
			return nullptr;
		}
	}
#else
	const bool locked = SDL_MUSTLOCK(src) == SDL_TRUE;

	if (locked) {
		if (SDL_LockSurface(src) != 0) {
			return nullptr;
		}
	}
#endif

#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_Surface *res = SDL_CreateSurfaceFrom(src->w, src->h, src->format,
						   src->pixels, src->pitch);
#else
	SDL_Surface *res = SDL_CreateRGBSurfaceFrom(src->pixels,
	                       src->w, src->h, src->format->BitsPerPixel,
	                       src->pitch, src->format->Rmask, src->format->Gmask,
	                       src->format->Bmask, src->format->Amask);
#endif

	if (locked) {
		SDL_UnlockSurface(src);
	}

	return res;
}

int SdlWindow::getDisplayIndex() const {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	int display = 0;
	int num_displays;
	SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);
	if (num_displays > 0) {
		display = static_cast<int>(displays[0]);
	}
	SDL_free(displays);
	return display;
#else
	if (_window) {
		int displayIndex = SDL_GetWindowDisplayIndex(_window);
		if (displayIndex >= 0)
			return displayIndex;
	}
	// Default to primary display
	return 0;
#endif
}

bool SdlWindow::createOrUpdateWindow(int width, int height, uint32 flags) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	if (_inputGrabState) {
		flags |= SDL_WINDOW_MOUSE_GRABBED;
	}
#else
	if (_inputGrabState) {
		flags |= SDL_WINDOW_INPUT_GRABBED;
	}
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_resizable) {
		flags |= SDL_WINDOW_RESIZABLE;
	}
#endif

#if SDL_VERSION_ATLEAST(3, 0, 0)
	const uint32 updateableFlagsMask = fullscreenMask | SDL_WINDOW_MOUSE_GRABBED | SDL_WINDOW_RESIZABLE;
#elif SDL_VERSION_ATLEAST(2, 0, 5)
	// SDL_WINDOW_RESIZABLE can be updated without recreating the window starting with SDL 2.0.5
	// Even though some users may switch the SDL version when it's linked dynamically, 2.0.5 is now getting quite old
	const uint32 updateableFlagsMask = fullscreenMask | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_RESIZABLE;
#else
	const uint32 updateableFlagsMask = fullscreenMask | SDL_WINDOW_INPUT_GRABBED;
#endif

	const uint32 oldNonUpdateableFlags = _lastFlags & ~updateableFlagsMask;
	const uint32 newNonUpdateableFlags = flags & ~updateableFlagsMask;

	const uint32 fullscreenFlags = flags & fullscreenMask;

	// This is terrible, but there is no way in SDL to get information on the
	// maximum bounds of a window with decoration, and SDL is too dumb to make
	// sure the window's surface doesn't grow beyond the display bounds, which
	// can easily happen with 3x scalers. There is a function in SDL to get the
	// window decoration size, but it only exists starting in SDL 2.0.5, which
	// is a buggy release on some platforms so we can't safely use 2.0.5+
	// features since some users replace the SDL dynamic library with 2.0.4, and
	// the documentation says it only works on X11 anyway, which means it is
	// basically worthless. So we'll just try to keep things closeish to the
	// maximum for now.
	Common::Rect desktopRes = getDesktopResolution();
	if (
		!fullscreenFlags
#if defined(MACOSX)
		// On macOS a maximized window is borderless
		&& !(flags & SDL_WINDOW_MAXIMIZED)
#endif
	) {
		int top, left, bottom, right;

#if SDL_VERSION_ATLEAST(3, 0, 0)
		if (!_window || !SDL_GetWindowBordersSize(_window, &top, &left, &bottom, &right))
#elif SDL_VERSION_ATLEAST(2, 0, 5)
		if (!_window || SDL_GetWindowBordersSize(_window, &top, &left, &bottom, &right) < 0)
#endif
		{
			left = right = 10;
			top = bottom = 15;
		}
		desktopRes.right -= (left + right);
		desktopRes.bottom -= (top + bottom);
	}

	if (width > desktopRes.right) {
		width = desktopRes.right;
	}

	if (height > desktopRes.bottom) {
		height = desktopRes.bottom;
	}

	if (!_window || oldNonUpdateableFlags != newNonUpdateableFlags) {
		destroyWindow();
#if SDL_VERSION_ATLEAST(3, 0, 0)
		SDL_PropertiesID props = SDL_CreateProperties();
		SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, _windowCaption.c_str());
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, _lastX);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, _lastY);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, flags);
		_window = SDL_CreateWindowWithProperties(props);
		SDL_DestroyProperties(props);
#else
		_window = SDL_CreateWindow(_windowCaption.c_str(), _lastX,
								   _lastY, width, height, flags);
#endif
		if (_window) {
			setupIcon();
		}
	} else {
		if (fullscreenFlags) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
			if (!SDL_SetWindowFullscreenMode(_window, NULL))
				warning("SDL_SetWindowFullscreenMode failed: %s", SDL_GetError());
			if (!SDL_SyncWindow(_window))
				warning("SDL_SyncWindow failed: %s", SDL_GetError());
#else
			SDL_DisplayMode fullscreenMode;
			fullscreenMode.w = width;
			fullscreenMode.h = height;
			fullscreenMode.driverdata = nullptr;
			fullscreenMode.format = 0;
			fullscreenMode.refresh_rate = 0;
			SDL_SetWindowDisplayMode(_window, &fullscreenMode);
#endif
		} else {
			SDL_SetWindowSize(_window, width, height);
		}

		SDL_SetWindowFullscreen(_window, fullscreenFlags);
	}

#if SDL_VERSION_ATLEAST(3, 0, 0)
	const bool shouldGrab = (flags & SDL_WINDOW_MOUSE_GRABBED) || fullscreenFlags;
	SDL_SetWindowMouseGrab(_window, shouldGrab);
#else
	const bool shouldGrab = (flags & SDL_WINDOW_INPUT_GRABBED) || fullscreenFlags;
	SDL_SetWindowGrab(_window, shouldGrab ? SDL_TRUE : SDL_FALSE);
#endif
#if SDL_VERSION_ATLEAST(2, 0, 18)
	SDL_SetWindowMouseRect(_window, shouldGrab ? &grabRect : NULL);
#endif

#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_SetWindowResizable(_window, _resizable);
#elif SDL_VERSION_ATLEAST(2, 0, 5)
	SDL_SetWindowResizable(_window, _resizable ? SDL_TRUE : SDL_FALSE);
#endif

	if (!_window) {
		return false;
	}

#if defined(MACOSX)
	// macOS windows with the flag SDL_WINDOW_FULLSCREEN_DESKTOP exiting their fullscreen space
	// ignore the size set by SDL_SetWindowSize while they were in fullscreen mode.
	// Instead, they revert back to their previous windowed mode size.
	// This is a bug in SDL2: https://github.com/libsdl-org/SDL/issues/2518.
	// TODO: Remove the call to SDL_SetWindowSize below once the SDL bug is fixed.

	// In some cases at this point there may be a pending SDL resize event with the old size.
	// This happens for example if we destroyed the window, or when switching between windowed
	// and fullscreen modes. If we changed the window size here, this pending event will have the
	// old (and incorrect) size. To avoid any issue we call SDL_SetWindowSize() to generate another
	// resize event (SDL_WINDOWEVENT_SIZE_CHANGED) so that the last resize event we receive has
	// the correct size. This fixes for exmample bug #9971: SDL2: Fullscreen to RTL launcher resolution
	SDL_SetWindowSize(_window, width, height);
#endif

	_lastFlags = flags;

	return true;
}

void SdlWindow::destroyWindow() {
	if (_window) {
		if (!(_lastFlags & fullscreenMask)) {
			SDL_GetWindowPosition(_window, &_lastX, &_lastY);
		}
		SDL_DestroyWindow(_window);
		_window = nullptr;
	}
}
#endif
