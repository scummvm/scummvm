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

#include "watchmaker/sdl_wrapper.h"
#include "watchmaker/classes/do_keyboard.h"
#include "common/events.h"
#include "common/system.h"

namespace Watchmaker {

void sdl_wrapper::getWindowSize(unsigned int &width, unsigned int &height) {
	width = g_system->getWidth();
	height = g_system->getHeight();
}

void sdl_wrapper::pollSDL() {
	// Process events
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			mMove += abs(event.relMouse.x);
			mMove += abs(event.relMouse.y);
			mMoveX += event.relMouse.x;
			mMoveY += event.relMouse.y;
			mPosx = event.mouse.x;
			mPosy = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			bLPressed = true;
			break;
		case Common::EVENT_LBUTTONUP:
			bLPressed = false;
			break;
		case Common::EVENT_KEYUP:
			KeyTable[event.kbd.keycode] = 0x10;
			break;
		case Common::EVENT_QUIT:
			shouldQuit = true;
		default:
			warning("Unhandled event: %d", event.type);
		}
	}
#if 0
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			shouldQuit = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				bLPressed = true;
				warning("LEFT PRESSED");
				break;
			case SDL_BUTTON_RIGHT:
				bRPressed = true;
				break;
			default:
				break;
			}
			* /
		/*
		case SDL_MOUSEBUTTONUP:
		switch (event.button.button) {
		    case SDL_BUTTON_LEFT:
		        bLPressed = false;
		        warning("LEFT RELEASED");
		        break;
		    case SDL_BUTTON_RIGHT:
		        bRPressed = false;
		        break;
		    default:
		        break;
		}*/
		case SDL_KEYUP:
			KeyTable[event.key.keysym.scancode] = 0x10;
			break;

		}
	}
#endif
}

void sdl_wrapper::initWindow() {
#if 0
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		shouldQuit = true;
		return;
	}

	window = SDL_CreateWindow("The Watchmaker", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);

	if (window == nullptr) {
		warning("Couldn't create window: %s", SDL_GetError());
		assert(false);
	}
	int numRenderers = SDL_GetNumRenderDrivers();
	int renderIndex = -1;
	for (int i = 0; i < numRenderers; i++) {
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		warning("Renderer(%d): %s", i, info.name);
		if (strcmp(info.name, "opengl") == 0) {
			renderIndex = i;
		}
	}
	renderer = SDL_CreateRenderer(window, renderIndex, SDL_RENDERER_ACCELERATED);
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	warning("Renderer: %s", info.name);
	IMG_Init(0);
#endif
}

} // End of namespace Watchmaker
