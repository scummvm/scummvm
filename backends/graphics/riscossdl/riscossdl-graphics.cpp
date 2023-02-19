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

#if defined(RISCOS) && defined(SDL_BACKEND)

#include "backends/graphics/riscossdl/riscossdl-graphics.h"

bool RISCOSSdlGraphicsManager::hasFeature(OSystem::Feature f) const {
	if (f == OSystem::kFeatureVSync)
		return true;
	return SurfaceSdlGraphicsManager::hasFeature(f);
}

void RISCOSSdlGraphicsManager::initGraphicsSurface() {
	Uint32 flags = 0;

	if (_videoMode.fullscreen)
		flags |= SDL_FULLSCREEN;

	if (_videoMode.vsync && _videoMode.fullscreen) {
		flags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
	} else {
		/* Hardware surfaces and double buffering aren't supported in windowed mode on RISC OS. */
		flags |= SDL_SWSURFACE;
	}

	_hwScreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 16, flags);
	_isDoubleBuf = flags & SDL_DOUBLEBUF;
}

#endif
