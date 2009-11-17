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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/sdl/graphics.cpp $
 * $Id: graphics.cpp 44495 2009-09-30 16:16:53Z fingolfin $
 *
 */

#include "backends/platform/samsungtv/samsungtv.h"
#include "common/mutex.h"
#include "common/util.h"
#include "common/list.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/scaler.h"
#include "graphics/surface.h"

#if defined(SAMSUNGTV)

bool OSystem_SDL_SamsungTV::loadGFXMode() {
       OSystem_SDL::loadGFXMode();
       _realhwscreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 32, 0);

       return true;
}

void OSystem_SDL_SamsungTV::unloadGFXMode() {
       if (_realhwscreen) {
               SDL_FreeSurface(_realhwscreen);
               _realhwscreen = NULL;
       }

       OSystem_SDL::unloadGFXMode();
}

bool OSystem_SDL_SamsungTV::hotswapGFXMode() {
	if (!_screen)
		return false;

	SDL_FreeSurface(_realhwscreen); _realhwscreen = NULL;

	return OSystem_SDL::hotswapGFXMode();
}

void OSystem_SDL_SamsungTV::internUpdateScreen() {
	OSystem_SDL::internUpdateScreen();
	SDL_BlitSurface(_hwscreen, 0, _realhwscreen, 0);
	SDL_UpdateRect(_realhwscreen, 0, 0, 0, 0);
}

#endif
