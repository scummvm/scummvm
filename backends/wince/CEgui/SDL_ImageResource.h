/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef CEGUI_SDL_IMAGERESOURCE
#define CEGUI_SDL_IMAGERESOURCE

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "SDL.h"

namespace CEGUI {
	class SDL_ImageResource {
	public:
		SDL_ImageResource();
		SDL_Surface* load(WORD resourceID);
		SDL_Surface* get();
		int height();
		int width();
		virtual ~SDL_ImageResource();
	private:
		SDL_Surface *_surface;
	};
}

#endif