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

#include "backends/graphics/sdl/sdl-graphics.h"

#include "backends/platform/sdl/sdl-sys.h"
#include "backends/events/sdl/sdl-events.h"
#include "common/textconsole.h"

SdlGraphicsManager::SdlGraphicsManager(SdlEventSource *source)
	: _eventSource(source)
#if SDL_VERSION_ATLEAST(2, 0, 0)
	  , _window(nullptr), _inputGrabState(false), _windowCaption("ScummVM"), _windowIcon(nullptr)
#endif
	{
}

SdlGraphicsManager::~SdlGraphicsManager() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_FreeSurface(_windowIcon);
	destroyWindow();
#endif
}

void SdlGraphicsManager::activateManager() {
	_eventSource->setGraphicsManager(this);
}

void SdlGraphicsManager::deactivateManager() {
	_eventSource->setGraphicsManager(0);
}

void SdlGraphicsManager::setWindowCaption(const Common::String &caption) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_windowCaption = caption;
	if (_window) {
		SDL_SetWindowTitle(_window, caption.c_str());
	}
#else
	SDL_WM_SetCaption(caption.c_str(), caption.c_str());
#endif
}

void SdlGraphicsManager::setWindowIcon(SDL_Surface *icon) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_FreeSurface(_windowIcon);
	_windowIcon = icon;
	if (_window) {
		SDL_SetWindowIcon(_window, icon);
	}
#else
	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
#endif
}

void SdlGraphicsManager::toggleMouseGrab() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_window) {
		_inputGrabState = !(SDL_GetWindowGrab(_window) == SDL_TRUE);
		SDL_SetWindowGrab(_window, _inputGrabState ? SDL_TRUE : SDL_FALSE);
	}
#else
	if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
	} else {
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
#endif
}

bool SdlGraphicsManager::hasMouseFocus() const {
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

void SdlGraphicsManager::warpMouseInWindow(uint x, uint y) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_window) {
		SDL_WarpMouseInWindow(_window, x, y);
	}
#else
	SDL_WarpMouse(x, y);
#endif
}

void SdlGraphicsManager::iconifyWindow() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_window) {
		SDL_MinimizeWindow(_window);
	}
#else
	SDL_WM_IconifyWindow();
#endif
}

SdlGraphicsManager::State::State()
#if SDL_VERSION_ATLEAST(2, 0, 0)
    : windowIcon(nullptr)
#endif
    {
}

SdlGraphicsManager::State::~State() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_FreeSurface(windowIcon);
#endif
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
SDL_Surface *copySDLSurface(SDL_Surface *src) {
	const bool locked = SDL_MUSTLOCK(src) == SDL_TRUE;

	if (locked) {
		if (SDL_LockSurface(src) != 0) {
			return nullptr;
		}
	}

	SDL_Surface *res = SDL_CreateRGBSurfaceFrom(src->pixels,
	                       src->w, src->h, src->format->BitsPerPixel,
	                       src->pitch, src->format->Rmask, src->format->Gmask,
	                       src->format->Bmask, src->format->Amask);

	if (locked) {
		SDL_UnlockSurface(src);
	}

	return res;
}
#endif

SdlGraphicsManager::State SdlGraphicsManager::getState() {
	State state;

	state.screenWidth   = getWidth();
	state.screenHeight  = getHeight();
	state.aspectRatio   = getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	state.fullscreen    = getFeatureState(OSystem::kFeatureFullscreenMode);
	state.cursorPalette = getFeatureState(OSystem::kFeatureCursorPalette);
#ifdef USE_RGB_COLOR
	state.pixelFormat   = getScreenFormat();
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
	state.inputGrabState = _inputGrabState;
	state.windowCaption  = _windowCaption;
	state.windowIcon     = copySDLSurface(_windowIcon);
#endif

	return state;
}

bool SdlGraphicsManager::setState(const State &state) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_inputGrabState = state.inputGrabState;
	if (!_window) {
		_windowCaption  = state.windowCaption;
		SDL_FreeSurface(_windowIcon);
		_windowIcon     = copySDLSurface(state.windowIcon);
	} else {
		SDL_SetWindowGrab(_window, _inputGrabState ? SDL_TRUE : SDL_FALSE);
		setWindowCaption(state.windowCaption);
		setWindowIcon(copySDLSurface(state.windowIcon));
	}
#endif

	beginGFXTransaction();
#ifdef USE_RGB_COLOR
		initSize(state.screenWidth, state.screenHeight, &state.pixelFormat);
#else
		initSize(state.screenWidth, state.screenHeight, 0);
#endif
		setFeatureState(OSystem::kFeatureAspectRatioCorrection, state.aspectRatio);
		setFeatureState(OSystem::kFeatureFullscreenMode, state.fullscreen);
		setFeatureState(OSystem::kFeatureCursorPalette, state.cursorPalette);

	if (endGFXTransaction() != OSystem::kTransactionSuccess) {
		return false;
	} else {
		return true;
	}
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
bool SdlGraphicsManager::createWindow(int width, int height, uint32 flags) {
	destroyWindow();

	if (_inputGrabState) {
		flags |= SDL_WINDOW_INPUT_GRABBED;
	}

	_window = SDL_CreateWindow(_windowCaption.c_str(), SDL_WINDOWPOS_UNDEFINED,
	                           SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (!_window) {
		return false;
	}
	SDL_SetWindowIcon(_window, _windowIcon);

	return true;
}

void SdlGraphicsManager::destroyWindow() {
	SDL_DestroyWindow(_window);
	_window = nullptr;
}
#endif
