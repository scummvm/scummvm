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

#ifndef TITANIC_DIRECT_DRAW_H
#define TITANIC_DIRECT_DRAW_H

#include "common/scummsys.h"
#include "common/array.h"
#include "titanic/support/direct_draw_surface.h"

namespace Titanic {

class TitanicEngine;

class DirectDraw {
public:
	bool _windowed;
	int _width;
	int _height;
	int _bpp;
	int _numBackSurfaces;
public:
	DirectDraw();

	/**
	 * Sets a new display mode
	 */
	void setDisplayMode(int width, int height, int bpp, int refreshRate);

	/**
	 * Logs diagnostic information
	 */
	void diagnostics();

	/**
	 * Create a surface from a passed description record
	 */
	DirectDrawSurface *createSurfaceFromDesc(const DDSurfaceDesc &desc);
};

class DirectDrawManager {
public:
	DirectDraw _directDraw;
	DirectDrawSurface *_mainSurface;
	DirectDrawSurface *_backSurfaces[2];
public:
	DirectDrawManager(TitanicEngine *vm, bool windowed);

	/**
	 * Initializes video surfaces
	 * @param width				Screen width
	 * @param height			Screen height
	 * @param bpp				Bits per pixel
	 * @param numBackSurfaces	Number of back surfaces
	 */
	void initVideo(int width, int height, int bpp, int numBackSurfaces);

	/**
	 * Initializes the surfaces in windowed mode
	 */
	void initWindowed() { initFullScreen(); }

	/**
	 * Initializes the surfaces for the screen
	 */
	void initFullScreen();

	/**
	 * Create a surface
	 */
	DirectDrawSurface *createSurface(int w, int h, int bpp, int surfaceNum);
};

} // End of namespace Titanic

#endif /* TITANIC_DIRECT_DRAW_H */
