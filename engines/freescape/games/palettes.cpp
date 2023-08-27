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
	{0x00, 0x00, 0xee},
	{0xee, 0x00, 0x00},
	{0xee, 0x00, 0xee},
	{0x00, 0xee, 0x00},
	{0x00, 0xee, 0xee},
	{0xee, 0xee, 0x00},
	{0xee, 0xee, 0xee},
	{0x00, 0x00, 0x00},
};

byte kDrillerCPCPalette[32][3] = {
	{0x80, 0x80, 0x80}, // 0: special case?
	{0x11, 0x22, 0x33},
	{0x80, 0xff, 0x80}, // 2
	{0xff, 0xff, 0x80}, // 3
	{0x11, 0x22, 0x33},
	{0xff, 0x00, 0x80}, // 5
	{0x00, 0x80, 0x00}, // 6
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
	{0x00, 0x00, 0x00}, // 20
	{0x00, 0x00, 0xff}, // 21
	{0x00, 0x80, 0x00}, // 22
	{0x00, 0x80, 0xff}, // 23
	{0x80, 0x00, 0x80}, // 24
	{0x11, 0x22, 0x33},
	{0x11, 0x22, 0x33},
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
	} else if (_renderMode == Common::kRenderCGA) {
		// palette depends on the area
	} else if (_renderMode == Common::kRenderAmiga || _renderMode == Common::kRenderAtariST) {
		// palette depends on the area
	} else
		error("Invalid render mode, no palette selected");

	_gfx->setColorMap(&_colorMap);
}

void FreescapeEngine::loadPalettes(Common::SeekableReadStream *file, int offset) {
	file->seek(offset);
	int r, g, b;
	uint numberOfAreas = _areaMap.size();

	if (isDriller())
		// This loop will load all the available palettes, which are more
		// than the current areas in Driller. This indicates that more areas
		// were originally planned, but they are not in the final game
		numberOfAreas += 2;
	else if (isDark())
		numberOfAreas -= 2;

	for (uint i = 0; i < numberOfAreas; i++) {
		int label = readField(file, 8);
		auto palette = new byte[16][3];
		debugC(1, kFreescapeDebugParser, "Loading palette for area: %d at %lx", label, file->pos());
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

enum {
	kDrillerCGAPalettePinkBlue = 0,
	kDrillerCGAPaletteRedGreen = 1,
};

static const struct CGAPalettteEntry {
	int areaId;
	int palette;
} rawCGAPaletteTable[] {
	{1, kDrillerCGAPaletteRedGreen},
	{2, kDrillerCGAPalettePinkBlue},
	{3, kDrillerCGAPaletteRedGreen},
	{4, kDrillerCGAPalettePinkBlue},
	{5, kDrillerCGAPaletteRedGreen},
	{6, kDrillerCGAPalettePinkBlue},
	{7, kDrillerCGAPaletteRedGreen},
	{8, kDrillerCGAPalettePinkBlue},
	{9, kDrillerCGAPaletteRedGreen},
	{10, kDrillerCGAPalettePinkBlue},
	{11, kDrillerCGAPaletteRedGreen},
	{12, kDrillerCGAPalettePinkBlue},
	{13, kDrillerCGAPaletteRedGreen},
	{14, kDrillerCGAPalettePinkBlue},
	{15, kDrillerCGAPaletteRedGreen},
	{16, kDrillerCGAPalettePinkBlue},
	{17, kDrillerCGAPalettePinkBlue},
	{18, kDrillerCGAPalettePinkBlue},
	{19, kDrillerCGAPaletteRedGreen},
	{20, kDrillerCGAPalettePinkBlue},
	{21, kDrillerCGAPaletteRedGreen},
	{22, kDrillerCGAPalettePinkBlue},
	{23, kDrillerCGAPaletteRedGreen},
	{25, kDrillerCGAPalettePinkBlue},
	{27, kDrillerCGAPaletteRedGreen},
	{28, kDrillerCGAPalettePinkBlue},

	{31, kDrillerCGAPaletteRedGreen},
	{32, kDrillerCGAPalettePinkBlue},
	{127, kDrillerCGAPaletteRedGreen},
	{0, 0}   // This marks the end
};

byte kDrillerCGAPalettePinkBlueData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0xaa, 0xaa},
	{0xaa, 0x00, 0xaa},
	{0xaa, 0xaa, 0xaa},
};

byte kDrillerCGAPaletteRedGreenData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0xaa, 0x00},
	{0xaa, 0x00, 0x00},
	{0xaa, 0x55, 0x00},
};

void FreescapeEngine::swapPalette(uint16 levelID) {
	if (isAmiga() || isAtariST()) {
		// The following palette was not available in the demo, so we select another one
		if (isDemo() && levelID == 32)
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
		const CGAPalettteEntry *entry = rawCGAPaletteTable;
		while (entry->areaId) {
			if (entry->areaId == levelID) {
				if (entry->palette == kDrillerCGAPaletteRedGreen) {
					_gfx->_palette = (byte *)kDrillerCGAPaletteRedGreenData;
				} else if (entry->palette == kDrillerCGAPalettePinkBlue) {
					_gfx->_palette = (byte *)kDrillerCGAPalettePinkBlueData;
				} else
					error("Invalid CGA palette to use");
				break;
			}
			entry++;
		}

		assert(entry->areaId == levelID);
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

} // End of namespace Freescape
