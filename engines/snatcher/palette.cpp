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


#include "snatcher/palette.h"
#include "graphics/paletteman.h"

namespace Snatcher {

Palette::Palette(const Graphics::PixelFormat *pxf, PaletteManager *pm, GraphicsEngine::GfxState &state) : _pixelFormat(pxf ? *pxf : Graphics::PixelFormat::createFormatCLUT8()), _palMan(pm), _gfxState(state) {
	assert(pm);
	_sysPalette = new uint8[256]();
}

Palette::~Palette() {
	delete[] _sysPalette;
}

void Palette::block() {
	_blocked = true;
	memset(_sysPalette, 0, 256);
	if (_pixelFormat.bytesPerPixel == 1)
		_palMan->setPalette(_sysPalette, 0, 256);
}

void Palette::unblock() {
	_blocked = false;
	if (_restoreAfterUnblock) {
		updateSystemPalette();
		_restoreAfterUnblock = false;
	}
}

Palette *Palette::create(const Graphics::PixelFormat *pxf, PaletteManager *pm, Common::Platform platform, GraphicsEngine::GfxState &state) {
	switch (platform) {
	case Common::kPlatformSegaCD:
		return createSegaPalette(pxf, pm, state);
	default:
		break;
	};
	return 0;
}

} // End of namespace Snatcher
