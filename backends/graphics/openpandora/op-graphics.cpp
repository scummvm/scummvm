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

#include "common/scummsys.h"

#if defined(OPENPANDORA)

#include "backends/graphics/openpandora/op-graphics.h"
#include "backends/events/openpandora/op-events.h"
#include "graphics/scaler/aspect.h"
#include "common/mutex.h"
#include "common/textconsole.h"

static SDL_Cursor *hiddenCursor;

OPGraphicsManager::OPGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window)
	: SurfaceSdlGraphicsManager(sdlEventSource, window) {
}

bool OPGraphicsManager::loadGFXMode() {

	uint8_t hiddenCursorData = 0;
	hiddenCursor = SDL_CreateCursor(&hiddenCursorData, &hiddenCursorData, 8, 1, 0, 0);

	/* On the OpenPandora we need to work around an SDL assumption that
	   returns relative mouse coordinates when you get to the screen
	   edges using the touchscreen. The workaround is to set a blank
	   SDL cursor and not disable it (Hackish I know).

	   The root issues likes in the Windows Manager GRAB code in SDL.
	   That is why the issue is not seen on framebuffer devices like the
	   GP2X (there is no X window manager ;)).
	*/
	SDL_ShowCursor(SDL_ENABLE);
	SDL_SetCursor(hiddenCursor);

	return SurfaceSdlGraphicsManager::loadGFXMode();
}

void OPGraphicsManager::unloadGFXMode() {

	uint8_t hiddenCursorData = 0;
	hiddenCursor = SDL_CreateCursor(&hiddenCursorData, &hiddenCursorData, 8, 1, 0, 0);

	// Free the hidden SDL cursor created in loadGFXMode
	SDL_FreeCursor(hiddenCursor);

	SurfaceSdlGraphicsManager::unloadGFXMode();
}

bool OPGraphicsManager::showMouse(bool visible) {
	if (visible == _cursorVisible) {
		return visible;
	}

	int showCursor = SDL_DISABLE;
	if (visible) {
		// _cursorX and _cursorY are currently always clipped to the active
		// area, so we need to ask SDL where the system's mouse cursor is
		// instead
		int x, y;
		SDL_GetMouseState(&x, &y);
		if (!_activeArea.drawRect.contains(Common::Point(x, y))) {
			showCursor = SDL_ENABLE;
		}
	}
	//SDL_ShowCursor(showCursor);

	return WindowedGraphicsManager::showMouse(visible);
}

bool OPGraphicsManager::notifyMousePosition(Common::Point &mouse) {
	mouse.x = CLIP<int16>(mouse.x, 0, _windowWidth - 1);
	mouse.y = CLIP<int16>(mouse.y, 0, _windowHeight - 1);

	int showCursor = SDL_DISABLE;
	// Currently on macOS we need to scale the events for HiDPI screen, but on
	// Windows we do not. We can find out if we need to do it by querying the
	// SDL window size vs the SDL drawable size.
	float dpiScale = _window->getSdlDpiScalingFactor();
	mouse.x = (int)(mouse.x * dpiScale + 0.5f);
	mouse.y = (int)(mouse.y * dpiScale + 0.5f);
	bool valid = true;
	if (_activeArea.drawRect.contains(mouse)) {
		_cursorLastInActiveArea = true;
	} else {
		// The right/bottom edges are not part of the drawRect. As the clipping
		// is done in drawable area coordinates, but the mouse position is set
		// in window coordinates, we need to subtract as many pixels from the
		// edges as corresponds to one pixel in the window coordinates.
		mouse.x = CLIP<int>(mouse.x, _activeArea.drawRect.left,
							_activeArea.drawRect.right - (int)(1 * dpiScale + 0.5f));
		mouse.y = CLIP<int>(mouse.y, _activeArea.drawRect.top,
							_activeArea.drawRect.bottom - (int)(1 * dpiScale + 0.5f));

		if (_window->mouseIsGrabbed() ||
			// Keep the mouse inside the game area during dragging to prevent an
			// event mismatch where the mouseup event gets lost because it is
			// performed outside of the game area
			(_cursorLastInActiveArea && SDL_GetMouseState(nullptr, nullptr) != 0)) {
			setSystemMousePosition(mouse.x, mouse.y);
		} else {
			// Allow the in-game mouse to get a final movement event to the edge
			// of the window if the mouse was moved out of the game area
			if (_cursorLastInActiveArea) {
				_cursorLastInActiveArea = false;
			} else if (_cursorVisible) {
				// Keep sending events to the game if the cursor is invisible,
				// since otherwise if a game lets you skip a cutscene by
				// clicking and the user moved the mouse outside the active
				// area, the clicks wouldn't do anything, which would be
				// confusing
				valid = false;
			}

			if (_cursorVisible) {
				showCursor = SDL_ENABLE;
			}
		}
	}

	//SDL_ShowCursor(showCursor);
	if (valid) {
		setMousePosition(mouse.x, mouse.y);
		mouse = convertWindowToVirtual(mouse.x, mouse.y);
	}
	return valid;
}

#endif
