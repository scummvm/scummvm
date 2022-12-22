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
	{0xff, 0xff, 0xff}};

void FreescapeEngine::loadColorPalette() {
	if (_renderMode == Common::kRenderEGA) {
		_gfx->_palette = (byte *)&dos_EGA_palette;
	} else if (_renderMode == Common::kRenderCGA) {
		_gfx->_palette = nullptr; // palette depends on the area
	} else if (_renderMode == Common::kRenderAmiga || _renderMode == Common::kRenderAtariST) {
		_gfx->_palette = nullptr; // palette depends on the area
	} else
		error("Invalid render mode, no palette selected");

	_gfx->_colorMap = &_colorMap;
}

void FreescapeEngine::loadPalettes(Common::SeekableReadStream *file, int offset) {
	file->seek(offset);
	int r, g, b;
	// This loop will load all the available palettes, which are more
	// the current areas. This indicates that more areas
	// were originally planned, but they are not in the final game
	for (uint i = 0; i < _areaMap.size() + 2; i++) {
		int label = readField(file, 8);
		auto palette = new byte[16][3];
		debugC(1, kFreescapeDebugParser, "Loading palette for area: %d", label);
		for (int c = 0; c < 16; c++) {
			int v = file->readUint16BE();
			r = (v & 0xf00) >> 8;
			r = r << 4 | r;
			palette[c][0] = r & 0xff;
			g = (v & 0xf0) >> 4;
			g = g << 4 | g;
			palette[c][1] = g & 0xff;
			b = v & 0xf;
			b = b << 4 | b;
			palette[c][2] = b & 0xff;
		}

		assert(!_paletteByArea.contains(label));
		_paletteByArea[label] = (byte *)palette;
	}
}

void FreescapeEngine::swapPalette(uint16 levelID) {
	if (isAmiga() || isAtariST())
		_gfx->_palette = _paletteByArea[levelID];
	else if (isDOS() && _renderMode == Common::kRenderCGA) {
		assert(_borderCGAByArea.contains(levelID));
		assert(_paletteCGAByArea.contains(levelID));
		_borderTexture = _borderCGAByArea.getVal(levelID);
		_gfx->_palette = _paletteCGAByArea.getVal(levelID);
	}
}

} // End of namespace Freescape
