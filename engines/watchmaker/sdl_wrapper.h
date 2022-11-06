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

#ifndef WATCHMAKER_SDL_WRAPPER_H
#define WATCHMAKER_SDL_WRAPPER_H

#include "watchmaker/utils.h"
#include "watchmaker/ll/ll_mouse.h"

namespace Watchmaker {

struct SDL_Window;
struct SDL_Renderer;
class sdl_wrapper {
public:
	SDL_Window *window;
	SDL_Renderer *renderer;
	bool shouldQuit = false;

	void getWindowSize(unsigned int &width, unsigned int &height);

	void initWindow();

	void pollSDL();
};


} // End of namespace Watchmaker

#endif // WATCHMAKER_SDL_WRAPPER_H
