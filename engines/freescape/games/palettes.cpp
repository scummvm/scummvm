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
	else if (_renderMode == "amiga" || _renderMode == "atari")
		palette = nullptr; // palette depends on the area
	else
		error("Invalid render mode, no palette selected");

	_gfx->_palette = palette;
	_gfx->_colorMap = &_colorMap;
}

void FreescapeEngine::loadAmigaPalette(Common::SeekableReadStream *file, int offset) {
	file->seek(offset);
	int r, g, b;
	for (int i = 0; i < int(_areaMap.size()); i++) {
		int label = readField(file, 8);
		auto palette = new byte[16][3];
		debug("Loading palette for area: %d", label);
		for (int c = 0; c < 16; c++) {
			int v = file->readUint16BE();
			r = (v & 0xf00) >> 8;
			r = r << 4 | r;
			palette[c][0] = byte(r);
			g = (v & 0xf0) >> 4;
			g = g << 4 | g;
			palette[c][1] = byte(g);
			b = v & 0xf;
			b = b << 4 | b;
			palette[c][2] = byte(b);
		}

		assert(!_amigaPalette.contains(label));
		_amigaPalette[label] = (byte*) palette;
	}
}

void FreescapeEngine::swapAmigaPalette(uint16 levelID) {
	if (_gfx->_palette)
		delete _gfx->_palette;

	_gfx->_palette = new Graphics::PixelBuffer(_gfx->_palettePixelFormat, _amigaPalette[levelID]);
}


} // End of namespace Freescape