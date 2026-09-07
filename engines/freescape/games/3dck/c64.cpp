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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/endian.h"

#include "freescape/games/3dck/8bit.h"

namespace Freescape {

static Common::Array<byte> loadKitC64Program(const char *name, uint16 address, uint32 size) {
	Common::File file;
	if (!file.open(name) || file.size() != size + 2 || file.readUint16LE() != address)
		error("Invalid 3D Construction Kit C64 file %s", name);
	Common::Array<byte> data;
	data.resize(size);
	if (file.read(data.data(), size) != size)
		error("Truncated 3D Construction Kit C64 file %s", name);
	return data;
}

void Kit8Engine::loadPresentationC64() {
	Common::Array<byte> runner = loadKitC64Program("3D1", 0x0400, 39936);
	Common::Array<byte> characters = loadKitC64Program("3D3", 0xc400, 3072);
	Common::Array<byte> bitmap = loadKitC64Program("3D4", 0x3800, 8000);
	memcpy(_colorPatterns, runner.data() + 0x672e - 0x0400, sizeof(_colorPatterns));

	for (int y = 0; y < _screenH; ++y) {
		for (int x = 0; x < _screenW; ++x) {
			uint cell = (y / 8) * 40 + x / 8;
			byte pen = (bitmap[8 * cell + (y & 7)] >> (6 - (x & 6))) & 3;
			byte screen = runner[0x3000 - 0x0400 + cell];
			// Index 16 follows the VIC background colour of the current area.
			byte color = pen == 0 ? 16 : pen == 1 ? screen >> 4 : pen == 2 ? screen & 15 :
				runner[0x3400 - 0x0400 + cell] & 15;
			_borderSurface.setPixel(x, y, color);
		}
	}
	_borderSurface.fillRect(_viewArea, 255);

	uint16 font = READ_LE_UINT16(runner.data() + 0x1a7b - 0x0400);
	if (font < 0xc400 || font + 64 * 8 > 0xc400 + characters.size())
		error("Invalid 3D Construction Kit C64 font address");
	for (uint chr = 32; chr < 128; ++chr) {
		uint glyph = chr == 32 ? 0 : chr >= 65 ? chr - 64 : chr - 21;
		for (uint row = 0; row < 8; ++row) {
			byte pixels = characters[font - 0xc400 + 8 * glyph + row];
			byte bits = 0;
			for (uint col = 0; col < 4; ++col) {
				if (((pixels >> (6 - 2 * col)) & 3) == 2)
					bits |= 3 << (6 - 2 * col);
			}
			_fontData[chr - 32][row] = bits;
		}
	}
	_hasFont = true;
}

void Kit8Engine::applyPaletteC64() {
	_gfx->_fourColorBackground = _palette[0];
	_gfx->_underFireBackgroundColor = _palette[2];
	_gfx->_paperColor = _palette[1];
	_gfx->_inkColor = _palette[3];
	_currentArea->_usualBackgroundColor = 1;
	_currentArea->_skyColor = 1;
	_currentArea->_underFireBackgroundColor = 4;
}

} // namespace Freescape
