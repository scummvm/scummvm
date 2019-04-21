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

#include "titanic/support/direct_draw.h"
#include "titanic/debugger.h"
#include "titanic/titanic.h"
#include "common/debug.h"
#include "engines/util.h"
#include "graphics/pixelformat.h"
#include "graphics/screen.h"

namespace Titanic {

DirectDraw::DirectDraw() : _windowed(false), _width(0), _height(0),
		_bpp(0), _numBackSurfaces(0) {
}

void DirectDraw::setDisplayMode(int width, int height, int bpp, int refreshRate) {
	debugC(DEBUG_BASIC, kDebugGraphics, "DirectDraw::SetDisplayMode (%d x %d), %d bpp",
		width, height, bpp);
	assert(bpp == 16);

	Graphics::PixelFormat pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(width, height, &pixelFormat);
}

void DirectDraw::diagnostics() {
	debugC(DEBUG_BASIC, kDebugGraphics, "Running DirectDraw Diagnostic...");
}

DirectDrawSurface *DirectDraw::createSurfaceFromDesc(const DDSurfaceDesc &desc) {
	DirectDrawSurface *surface = new DirectDrawSurface();
	surface->create(desc._w, desc._h, desc._bpp);

	return surface;
}

/*------------------------------------------------------------------------*/

DirectDrawManager::DirectDrawManager(TitanicEngine *vm, bool windowed) {
	_mainSurface = nullptr;
	_backSurfaces[0] = _backSurfaces[1] = nullptr;
	_directDraw._windowed = windowed;
}

void DirectDrawManager::initVideo(int width, int height, int bpp, int numBackSurfaces) {
	debugC(DEBUG_BASIC, kDebugGraphics, "Initialising video surfaces");
	assert(numBackSurfaces == 0);

	_directDraw._width = width;
	_directDraw._numBackSurfaces = numBackSurfaces;
	_directDraw._height = height;
	_directDraw._bpp = bpp;

	if (_directDraw._windowed) {
		initWindowed();
	} else {
		initFullScreen();
	}
}

void DirectDrawManager::initFullScreen() {
	debugC(DEBUG_BASIC, kDebugGraphics, "Creating surfaces");
	_directDraw.setDisplayMode(_directDraw._width, _directDraw._height,
		_directDraw._bpp, 0);

	// Set up the main surface to point to the screen
	_mainSurface = new DirectDrawSurface();
	_mainSurface->create(g_vm->_screen);
}

DirectDrawSurface *DirectDrawManager::createSurface(int w, int h, int bpp, int surfaceNum) {
	if (surfaceNum)
		return nullptr;

	assert(_mainSurface);
	return _directDraw.createSurfaceFromDesc(DDSurfaceDesc(w, h, bpp));
}

} // End of namespace Titanic
