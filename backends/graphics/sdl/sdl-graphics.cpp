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
	: _eventSource(source) {
}

SdlGraphicsManager::~SdlGraphicsManager() {
}

void SdlGraphicsManager::activateManager() {
	_eventSource->setGraphicsManager(this);
}

void SdlGraphicsManager::deactivateManager() {
	_eventSource->setGraphicsManager(0);
}

void SdlGraphicsManager::setWindowCaption(const Common::String &caption) {
	SDL_WM_SetCaption(caption.c_str(), caption.c_str());
}

void SdlGraphicsManager::setWindowIcon(SDL_Surface *icon) {
	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
}

void SdlGraphicsManager::toggleMouseGrab() {
	if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
		SDL_WM_GrabInput(SDL_GRAB_ON);
	} else {
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
}

bool SdlGraphicsManager::hasMouseFocus() const {
	return (SDL_GetAppState() & SDL_APPMOUSEFOCUS);
}

void SdlGraphicsManager::warpMouseInWindow(uint x, uint y) {
	SDL_WarpMouse(x, y);
}

void SdlGraphicsManager::iconifyWindow() {
	SDL_WM_IconifyWindow();
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
