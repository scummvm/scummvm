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

#include "backends/graphics/opendingux/opendingux-graphics.h"

void OpenDinguxGraphicsManager::initGraphicsSurface() {
#ifdef RS90
	Uint32 flags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;
	int bpp = 8;
#else
	Uint32 flags = SDL_SWSURFACE | SDL_FULLSCREEN;
	int bpp = 16;
#endif
	_hwScreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, bpp, flags);
	_isDoubleBuf = flags & SDL_DOUBLEBUF;
	_isHwPalette = flags & SDL_HWPALETTE;
}

void OpenDinguxGraphicsManager::getDefaultResolution(uint &w, uint &h) {
#ifdef RS90
	SDL_PixelFormat p;
	p.BitsPerPixel = 16;
	p.BytesPerPixel = 2;
	p.Rloss = 3;
	p.Gloss = 2;
	p.Bloss = 3;
	p.Rshift = 11;
	p.Gshift = 5;
	p.Bshift = 0;
	p.Rmask = 0xf800;
	p.Gmask = 0x07e0;
	p.Bmask = 0x001f;
	p.colorkey = 0;
	p.alpha = 0;
	// Only native screen resolution is supported in RGB565 fullscreen hwsurface.
	SDL_Rect const* const*availableModes = SDL_ListModes(&p, SDL_FULLSCREEN|SDL_HWSURFACE);
	w = availableModes[0]->w;
	h = availableModes[0]->h;
	// Request 320x200 for the RG99, to let the software aspect correction kick in
	if ( w == 320 && h == 480 ) h = 200;
#else
	w = 320;
	h = 200;
#endif
}
