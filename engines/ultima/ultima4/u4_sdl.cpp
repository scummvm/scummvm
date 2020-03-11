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


#include "ultima/ultima4/u4_sdl.h"

namespace Ultima {
namespace Ultima4 {

static inline int u4_SDL_Init() {
#ifdef TODO
	return SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
#else
	return 0;
#endif
}

static inline void u4_SDL_Delete() {
#ifdef TODO
	SDL_Quit();
#endif
}

int u4_SDL_InitSubSystem(uint32 flags) {
#ifdef TODO
    int f = SDL_WasInit(SDL_INIT_EVERYTHING);
    if (f == 0) {
        u4_SDL_Init();
    }
    if (!SDL_WasInit(flags))
        return SDL_InitSubSystem(flags);
    else return 0;    
#else
	return 0;
#endif
}

void u4_SDL_QuitSubSystem(uint32 flags) {
#ifdef TODO
	if (SDL_WasInit(SDL_INIT_EVERYTHING) == flags)
        u4_SDL_Delete();
    else SDL_QuitSubSystem(flags);
#endif
}

} // End of namespace Ultima4
} // End of namespace Ultima
