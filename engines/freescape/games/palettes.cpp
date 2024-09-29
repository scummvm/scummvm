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

byte kEGADefaultPalette[16][3] = {
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

byte kHerculesPaletteGreen[2][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0xff, 0x00},
};

byte kDrillerC64Palette[16][3] = {
	{0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF},
	{0x68, 0x37, 0x2B},
	{0x70, 0xA4, 0xB2},
	{0x6F, 0x3D, 0x86},
	{0x58, 0x8D, 0x43},
	{0x35, 0x28, 0x79},
	{0xB8, 0xC7, 0x6F},
	{0x6F, 0x4F, 0x25},
	{0x43, 0x39, 0x00},
	{0x9A, 0x67, 0x59},
	{0x44, 0x44, 0x44},
	{0x6C, 0x6C, 0x6C},
	{0x9A, 0xD2, 0x84},
	{0x6C, 0x5E, 0xB5},
	{0x95, 0x95, 0x95}
};

byte kDrillerZXPalette[9][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0xd8},
	{0xd8, 0x00, 0x00},
	{0xd8, 0x00, 0xd8},
	{0x00, 0xd8, 0x00},
	{0x00, 0xd8, 0xd8},
	{0xd8, 0xd8, 0x00},
	{0xd8, 0xd8, 0xd8},
	{0x00, 0x00, 0x00},
};

byte kDrillerCPCPalette[32][3] = {
	{0x80, 0x80, 0x80}, // 0: special case?
	{0x00, 0x00, 0x00}, // 1: used in dark only?
	{0x80, 0xff, 0x80}, // 2
	{0xff, 0xff, 0x80}, // 3
	{0x11, 0x22, 0x33},
	{0xff, 0x00, 0x80}, // 5
	{0x00, 0x80, 0x80}, // 6
	{0xff, 0x80, 0x80}, // 7
	{0x11, 0x22, 0x33},
	{0x11, 0x22, 0x33},
	{0xff, 0xff, 0x00}, // 10
	{0xff, 0xff, 0xff}, // 11
	{0xff, 0x00, 0x00}, // 12
	{0x11, 0x22, 0x33},
	{0xff, 0x80, 0x00}, // 14
	{0x11, 0x22, 0x33},
	{0x11, 0x22, 0x33},
	{0x00, 0xff, 0x80}, // 17
	{0x00, 0xff, 0x00}, // 18
	{0x80, 0xff, 0xff}, // 19
	{0x80, 0x80, 0x80}, // 20
	{0x00, 0x00, 0xff}, // 21
	{0x00, 0x80, 0x00}, // 22
	{0x00, 0x80, 0xff}, // 23
	{0x80, 0x00, 0x80}, // 24
	{0x80, 0xff, 0x80}, // 25
	{0x80, 0xff, 0x00}, // 26
	{0x00, 0xff, 0xff}, // 27
	{0x80, 0x00, 0x00}, // 28
	{0x11, 0x22, 0x33},
	{0x11, 0x22, 0x33},
	{0x80, 0x80, 0xff}, // 31
};

void FreescapeEngine::loadColorPalette() {
	if (_renderMode == Common::kRenderEGA) {
		_gfx->_palette = (byte *)&kEGADefaultPalette;
	} else if (_renderMode == Common::kRenderC64) {
		_gfx->_palette = (byte *)&kDrillerC64Palette;
	} else if (_renderMode == Common::kRenderZX) {
		_gfx->_palette = (byte *)kDrillerZXPalette;
	} else if (_renderMode == Common::kRenderCPC) {
		_gfx->_palette = (byte *)kDrillerCPCPalette;
	} else if (_renderMode == Common::kRenderHercG) {
		_gfx->_palette = (byte *)&kHerculesPaletteGreen;
	} else if (_renderMode == Common::kRenderCGA) {
		// palette depends on the area
	} else if (_renderMode == Common::kRenderAmiga || _renderMode == Common::kRenderAtariST) {
		// palette depends on the area
	} else
		error("Invalid render mode, no palette selected");

	_gfx->setColorMap(&_colorMap);
}

byte *FreescapeEngine::loadPalette(Common::SeekableReadStream *file) {
	int r, g, b;
	auto palette = new byte[16][3];
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
	return (byte *)palette;
}

void FreescapeEngine::loadPalettes(Common::SeekableReadStream *file, int offset) {
	file->seek(offset);
	int r, g, b;
	uint numberOfAreas = _areaMap.size();

	// This loop will load all the available palettes, which are more
	// than the current areas. This indicates that more areas
	// were originally planned, but they are not in the final game
	if (isDriller())
		numberOfAreas += 2;
	else if (isDark())
		numberOfAreas += 5;

	for (uint i = 0; i < numberOfAreas; i++) {
		int label = readField(file, 8);
		if (label == 255)
			break;
		auto palette = new byte[16][3];
		debugC(1, kFreescapeDebugParser, "Loading palette for area: %d at %" PRIx64, label, file->pos());
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
	if (isAmiga() || isAtariST()) {
		// The following palette was not available in the demo, so we select another one
		if (isDriller() && isDemo() && levelID == 32)
			levelID = 31;

		_gfx->_palette = _paletteByArea[levelID];
	} else if (isSpectrum() || isCPC() || isC64()) {
		_gfx->_inkColor = _areaMap[levelID]->_inkColor;
		_gfx->_paperColor = _areaMap[levelID]->_paperColor;
		_gfx->_underFireBackgroundColor = _areaMap[levelID]->_underFireBackgroundColor;

		if (!_border)
			return;

		byte *palette = (byte *)malloc(sizeof(byte) * 4 * 3);
		for (int c = 0; c < 4; c++) {
			byte r, g, b;
			_gfx->selectColorFromFourColorPalette(c, r, g, b);
			palette[3 * c + 0] = r;
			palette[3 * c + 1] = g;
			palette[3 * c + 2] = b;
		}
		_border->setPalette(palette, 0, 4);
		free(palette);
		processBorder();
	} else if (isDOS() && _renderMode == Common::kRenderCGA) {
		_gfx->_palette = findCGAPalette(levelID);
		if (!_border)
			return;
		_border->setPalette(_gfx->_palette, 0, 4);
		processBorder();
	} else if (isDOS() && _renderMode == Common::kRenderEGA) {
		if (!_border)
			return;

		_border->setPalette(_gfx->_palette, 0, 4);
		processBorder();
	}

}

byte *FreescapeEngine::findCGAPalette(uint16 levelID) {
	const CGAPaletteEntry *entry = _rawCGAPaletteByArea;
	byte *palette = nullptr;
	while (entry->areaId) {
		if (entry->areaId == levelID) {
			palette = entry->palette;
			break;
		}
		entry++;
	}

	return palette;
}

} // End of namespace Freescape
