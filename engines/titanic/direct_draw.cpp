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

#include "common/debug.h"
#include "engines/util.h"
#include "titanic/titanic.h"
#include "titanic/direct_draw.h"

namespace Titanic {

DirectDraw::DirectDraw(TitanicEngine *vm) : _vm(vm) {
	_field8 = 0;
	_fieldC = 0;
	_width = 0;
	_height = 0;
	_bpp = 0;
	_numBackSurfaces = 0;
	_field24 = 0;
}

void DirectDraw::setDisplayMode(int width, int height, int bpp, int refreshRate) {
	debugC(ERROR_BASIC, kDebugGraphics, "DirectDraw::SetDisplayMode (%d x %d), %d bpp",
		width, height, bpp);
	assert(bpp == 8);
	initGraphics(width, height, true);
}

void DirectDraw::diagnostics() {
	debugC(ERROR_BASIC, kDebugGraphics, "Running DirectDraw Diagnostic...");
}

/*------------------------------------------------------------------------*/

DirectDrawManager::DirectDrawManager(TitanicEngine *vm, int v) : _directDraw(vm) {
	_mainSurface = nullptr;
	_backSurfaces[0] = _backSurfaces[1] = nullptr;
	_directDraw._field8 = v;
}

void DirectDrawManager::initVideo(int width, int height, int bpp, int numBackSurfaces) {
	debugC(ERROR_BASIC, kDebugGraphics, "Initialising video surfaces");
	_directDraw._width = width;
	_directDraw._numBackSurfaces = numBackSurfaces;
	_directDraw._height = height;
	_directDraw._bpp = bpp;

	if (numBackSurfaces) {
		setResolution();
	} else {
		initSurface();
	}
}

void DirectDrawManager::setResolution() {
	// TODO
}

void DirectDrawManager::proc2() {

}

void DirectDrawManager::proc3() {

}

void DirectDrawManager::initSurface() {
	debugC(ERROR_BASIC, kDebugGraphics, "Creating surfaces");
	_directDraw.setDisplayMode(_directDraw._width, _directDraw._height,
		_directDraw._bpp, 0);

	_mainSurface = new Graphics::Surface();
	_mainSurface->create(_directDraw._width, _directDraw._height,
		Graphics::PixelFormat::createFormatCLUT8());
	_backSurfaces[0] = new Graphics::Surface();
	_backSurfaces[0]->create(_directDraw._width, _directDraw._height,
		Graphics::PixelFormat::createFormatCLUT8());
}

} // End of namespace Titanic
