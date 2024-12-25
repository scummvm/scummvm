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
 */

#include "graphics/palette.h"

namespace Graphics {

static const byte EGA_PALETTE[16][3] = {
	{ 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xaa }, { 0x00, 0xaa, 0x00 },
	{ 0x00, 0xaa, 0xaa }, { 0xaa, 0x00, 0x00 }, { 0xaa, 0x00, 0xaa },
	{ 0xaa, 0x55, 0x00 }, { 0xaa, 0xaa, 0xaa }, { 0x55, 0x55, 0x55 },
	{ 0x55, 0x55, 0xff }, { 0x55, 0xff, 0x55 }, { 0x55, 0xff, 0xff },
	{ 0xff, 0x55, 0x55 }, { 0xff, 0x55, 0xff }, { 0xff, 0xff, 0x55 },
	{ 0xff, 0xff, 0xff }
};


Palette::Palette(uint size) : _data(nullptr), _size(size) {
	if (_size > 0) {
		_data = new byte[_size * 3]();
	}
}

Palette::Palette(const byte *data, uint size) : _data(nullptr), _size(0) {
	if (data && size > 0) {
		_size = size;
		_data = new byte[_size * 3]();
		memcpy(_data, data, _size * 3);
	}
}

Palette::Palette(const Palette &p) : _data(nullptr), _size(p._size) {
	if (_size > 0) {
		_data = new byte[_size * 3]();
		memcpy(_data, p._data, _size * 3);
	}
}

Palette::~Palette() {
	delete[] _data;
}

Palette Palette::createEGAPalette() {
	return Palette(&EGA_PALETTE[0][0], 16);
}


Palette &Palette::operator=(const Palette &rhs) {
	delete[] _data;
	_data = nullptr;
	_size = rhs._size;

	if (_size > 0) {
		_data = new byte[_size * 3]();
		memcpy(_data, rhs._data, _size * 3);
	}

	return *this;
}

bool Palette::equals(const Palette &p) const {
	return p._size == _size && !memcmp(_data, p._data, p._size * 3);
}

bool Palette::contains(const Palette& p) const {
	return p._size <= _size && !memcmp(_data, p._data, p._size * 3);
}

byte Palette::findBestColor(byte cr, byte cg, byte cb, ColorDistanceMethod method) const {
	uint bestColor = 0;
	uint32 min = 0xFFFFFFFF;

	switch (method)
	{
	case kColorDistanceNaive:
		for (uint i = 0; i < _size; i++) {
			int r = _data[3 * i + 0] - cr;
			int g = _data[3 * i + 1] - cg;
			int b = _data[3 * i + 2] - cb;
			if (r == 0 && g == 0 && b == 0)
				return i;

			uint32 distWeighted = 3 * r * r + 5 * g * g + 2 * b * b;
			if (distWeighted < min) {
				bestColor = i;
				min = distWeighted;
			}
		}
		break;
	case kColorDistanceRedmean:
		for (uint i = 0; i < _size; ++i) {
			int r = _data[3 * i + 0] - cr;
			int g = _data[3 * i + 1] - cg;
			int b = _data[3 * i + 2] - cb;
			if (r == 0 && g == 0 && b == 0)
				return i;

			int rmean = (_data[3 * i + 0] + cr) / 2;
			uint32 distSquared = (((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8);
			if (distSquared < min) {
				bestColor = i;
				min = distSquared;
			}
		}
		break;
	default:
		break;
	}

	return bestColor;
}

void Palette::clear() {
	if (_size > 0)
		memset(_data, 0, _size);
}

void Palette::set(const byte *colors, uint start, uint num) {
	assert(start < _size && (start + num) <= _size);
	memcpy(_data + 3 * start, colors, 3 * num);
}

void Palette::set(const Palette &p, uint start, uint num) {
	assert(start < _size && (start + num) <= _size);
	memcpy(_data + 3 * start, p._data, 3 * num);
}

void Palette::grab(byte *colors, uint start, uint num) const {
	assert(start < _size && (start + num) <= _size);
	memcpy(colors, _data + 3 * start, 3 * num);
}

void Palette::grab(Palette &p, uint start, uint num) const {
	assert(start < _size && (start + num) <= _size);
	memcpy(p._data, _data + 3 * start, 3 * num);
}

PaletteLookup::PaletteLookup(): _palette(256) {
	_paletteSize = 0;
}

PaletteLookup::PaletteLookup(const byte *palette, uint len) : _palette(256) {
	_paletteSize = len;

	_palette.set(palette, 0, len);
}

bool PaletteLookup::setPalette(const byte *palette, uint len)  {
	// Check if the passed palette matched the one we have
	if (len == _paletteSize && !memcmp(_palette.data(), palette, len * 3))
		return false;

	_paletteSize = len;
	_palette.set(palette, 0, len);
	_colorHash.clear();

	return true;
}

byte PaletteLookup::findBestColor(byte cr, byte cg, byte cb, ColorDistanceMethod method) {
	if (_paletteSize == 0) {
		warning("PaletteLookup::findBestColor(): Palette was not set");
		return 0;
	}

	uint32 color = cr << 16 | cg << 8 | cb;

	if (_colorHash.contains(color))
		return _colorHash[color];

	uint bestColor = _palette.findBestColor(cr, cg, cb, method);
	_colorHash[color] = bestColor;

	return bestColor;
}

uint32 *PaletteLookup::createMap(const byte *srcPalette, uint len, ColorDistanceMethod method) {
	if (len <= _paletteSize && memcmp(_palette.data(), srcPalette, len * 3) == 0)
		return nullptr;

	uint32 *map = new uint32[len];
	for (uint i = 0; i < len; i++) {
		byte r = *srcPalette++;
		byte g = *srcPalette++;
		byte b = *srcPalette++;

		map[i] = findBestColor(r, g, b, method);
	}
	return map;
}

} // end of namespace Graphics
