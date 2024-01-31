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

Palette::Palette(uint num) : size(num) {
	memset(data, 0, sizeof(data));
}

Palette::Palette(const Palette &p) : size(p.size) {
	memcpy(data, p.data, size * 3);
}

bool Palette::equals(const Palette &p) const {
	return p.size == size && !memcmp(data, p.data, p.size * 3);
}

bool Palette::contains(const Palette& p) const {
	return p.size <= size && !memcmp(data, p.data, p.size * 3);
}

void Palette::clear() {
	memset(data, 0, sizeof(data));
}

void Palette::set(const byte *colors, uint start, uint num) {
	assert(start < 256 && (start + num) <= 256);
	if (size < start + num)
		size = start + num;
	memcpy(data + 3 * start, colors, 3 * num);
}

void Palette::set(const Palette &p, uint start, uint num) {
	assert(start < 256 && (start + num) <= 256);
	if (size < start + num)
		size = start + num;
	memcpy(data + 3 * start, p.data, 3 * num);
}

void Palette::grab(byte *colors, uint start, uint num) const {
	assert(start < 256 && (start + num) <= 256);
	memcpy(colors, data + 3 * start, 3 * num);
}

void Palette::grab(Palette &p, uint start, uint num) const {
	assert(start < 256 && (start + num) <= 256);
	if (p.size < num)
		p.size = num;
	memcpy(p.data, data + 3 * start, 3 * num);
}

PaletteLookup::PaletteLookup(): _palette() {
}

PaletteLookup::PaletteLookup(const Palette &palette): _palette(palette) {
}

PaletteLookup::PaletteLookup(const byte *palette, uint len) : _palette(len) {
	_palette.set(palette, 0, len);
}

bool PaletteLookup::setPalette(const Palette &palette) {
	// Check if the passed palette matched the one we have
	if (palette.equals(_palette))
		return false;

	_palette = palette;
	_colorHash.clear();

	return true;
}

bool PaletteLookup::setPalette(const byte *palette, uint len) {
	// Check if the passed palette matched the one we have
	if (len == _palette.size && !memcmp(_palette.data, palette, len * 3))
		return false;

	_palette.set(palette, 0, len);
	_colorHash.clear();

	return true;
}

byte PaletteLookup::findBestColor(byte cr, byte cg, byte cb, bool useNaiveAlg) {
	if (_palette.size == 0) {
		warning("PaletteLookup::findBestColor(): Palette was not set");
		return 0;
	}

	uint bestColor = 0;
	double min = 0xFFFFFFFF;

	uint32 color = cr << 16 | cg << 8 | cb;

	if (_colorHash.contains(color))
		return _colorHash[color];

	if (useNaiveAlg) {
		byte *palettePtr = _palette.data;

		for (uint i = 0; i < _palette.size; i++) {
			int redSquareDiff = (cr - palettePtr[0]) * (cr - palettePtr[0]);
			int greenSquareDiff = (cg - palettePtr[1]) * (cg - palettePtr[1]);
			int blueSquareDiff = (cb - palettePtr[2]) * (cb - palettePtr[2]);

			int weightedColorError = 3 * redSquareDiff + 5 * greenSquareDiff + 2 * blueSquareDiff;
			if (weightedColorError < min) {
				bestColor = i;
				min = weightedColorError;
			}

			palettePtr += 3;
		}
	} else {
		for (uint i = 0; i < _palette.size; ++i) {
			int rmean = (*(_palette.data + 3 * i + 0) + cr) / 2;
			int r = *(_palette.data + 3 * i + 0) - cr;
			int g = *(_palette.data + 3 * i + 1) - cg;
			int b = *(_palette.data + 3 * i + 2) - cb;

			double dist = sqrt((((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8));
			if (min > dist) {
				bestColor = i;
				min = dist;
			}
		}
	}

	_colorHash[color] = bestColor;

	return bestColor;
}

uint32 *PaletteLookup::createMap(const Palette &srcPalette, bool useNaiveAlg) {
	if (_palette.contains(srcPalette))
		return nullptr;

	uint32 *map = new uint32[srcPalette.size];
	const byte *palettePtr = srcPalette.data;
	for (uint i = 0; i < srcPalette.size; i++) {
		byte r = *palettePtr++;
		byte g = *palettePtr++;
		byte b = *palettePtr++;

		map[i] = findBestColor(r, g, b, useNaiveAlg);
	}
	return map;
}

} // end of namespace Graphics
