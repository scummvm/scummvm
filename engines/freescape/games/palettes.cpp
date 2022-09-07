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

#include "freescape/freescape.h"

namespace Freescape {

byte dos_EGA_palette[16][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0xaa},
	{0x00, 0xaa, 0x00},
	{0x00, 0xaa, 0xaa},
	{0xaa, 0x00, 0x00},
	{0xaa, 0x00, 0xaa},
	{0xaa, 0x55, 0x00},
	{0xaa, 0xaa, 0xaa},
	{0x55, 0x55, 0x55},
	{0x55, 0x55, 0xff},
	{0x55, 0xff, 0x55},
	{0x55, 0xff, 0xff},
	{0xff, 0x55, 0x55},
	{0xff, 0x55, 0xff},
	{0xff, 0xff, 0x55},
	{0xff, 0xff, 0xff}
};

byte dos_CGA_palette[4][3] = {
	{0x00, 0x00, 0x00},
	{0xff, 0xff, 0xff},
	{0xa8, 0x00, 0xa8},
	{0x00, 0xa8, 0xa8},
};


void FreescapeEngine::loadColorPalette() {
	Graphics::PixelBuffer *palette = nullptr;
	if (_renderMode == "ega")
		palette = new Graphics::PixelBuffer(_gfx->_palettePixelFormat, (byte*)&dos_EGA_palette);
	else if (_renderMode == "cga")
		palette = new Graphics::PixelBuffer(_gfx->_palettePixelFormat, (byte*)&dos_CGA_palette);

	_gfx->_palette = palette;
	_gfx->_colorMap = &_colorMap;
}

} // End of namespace Freescape