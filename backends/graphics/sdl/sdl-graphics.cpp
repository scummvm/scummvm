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

SdlGraphicsManager::SdlGraphicsManager(SdlEventSource *source, SdlWindow *window)
	: _eventSource(source), _window(window)
#if SDL_VERSION_ATLEAST(2, 0, 0)
	, _allowWindowSizeReset(false), _lastFlags(0)
#endif
	  {
}

SdlGraphicsManager::~SdlGraphicsManager() {
}

void SdlGraphicsManager::activateManager() {
	_eventSource->setGraphicsManager(this);
}

void SdlGraphicsManager::deactivateManager() {
	_eventSource->setGraphicsManager(0);
}

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
	return state;
}

bool SdlGraphicsManager::setState(const State &state) {
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
bool SdlGraphicsManager::createOrUpdateWindow(const int width, const int height, const Uint32 flags) {
	if (!_window) {
		return false;
	}

	// We only update the actual window when flags change (which usually means
	// fullscreen mode is entered/exited) or when updates are forced so that we
	// do not reset the window size whenever a game makes a call to change the
	// size or pixel format of the internal game surface (since a user may have
	// resized the game window)
	if (!_window->getSDLWindow() || _lastFlags != flags || _allowWindowSizeReset) {
		if (!_window->createOrUpdateWindow(width, height, flags)) {
			return false;
		}

		_lastFlags = flags;
		_allowWindowSizeReset = false;
	}

	return true;
}
#endif
