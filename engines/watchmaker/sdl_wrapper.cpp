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

namespace Watchmaker {

void sdl_wrapper::getWindowSize(unsigned int &width, unsigned int &height) {
#if 0
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	width = w;
	height = h;
#endif
}

int sdl_wrapper::getBitDepth() const {
#if 0
	int pixelFormat = SDL_GetWindowPixelFormat(window);
	int bpp;
	uint32 rMask, gMask, bMask, aMask;
	SDL_PixelFormatEnumToMasks(pixelFormat, &bpp, &rMask, &gMask, &bMask, &aMask);
	return bpp;
#endif
}

void sdl_wrapper::pollSDL() {
#if 0
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				shouldQuit = true;
				break;
			case SDL_MOUSEMOTION:
				mMove += abs(event.motion.xrel);
				mMove += abs(event.motion.yrel);
				mMoveX += event.motion.xrel;
				mMoveY += event.motion.yrel;
				mPosx = event.motion.x;
				mPosy = event.motion.y;
				break;/*
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
				}*/
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
