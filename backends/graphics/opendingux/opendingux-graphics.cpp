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
	Uint32 flags = _videoMode.isHwPalette ? (SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF) : SDL_SWSURFACE;
#ifndef RS90
	flags |= SDL_FULLSCREEN;
#endif
	_hwScreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, _videoMode.isHwPalette ? 8 : 16,
				     flags);
	_isDoubleBuf = flags & SDL_DOUBLEBUF;
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
	if (h > w) h /= 2; // RG99 has a 320x480 screen, gui should render at 320x240 to look correct
#else
	w = 320;
	h = 200;
#endif
}

void OpenDinguxGraphicsManager::setupHardwareSize() {
#ifdef RS90
	_videoMode.isHwPalette = true;
	_videoMode.scaleFactor = 1;
#else
	_videoMode.isHwPalette = false;
#endif
	SurfaceSdlGraphicsManager::setupHardwareSize();
}
