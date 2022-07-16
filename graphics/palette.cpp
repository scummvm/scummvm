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

PaletteLookup::PaletteLookup() {
	_paletteSize = 0;
}

PaletteLookup::PaletteLookup(const byte *palette, uint len)  {
	_paletteSize = len;

	memcpy(_palette, palette, len * 3);
}

bool PaletteLookup::setPalette(const byte *palette, uint len)  {
	// Check if the passed palette matched the one we have
	if (len == _paletteSize && !memcmp(_palette, palette, len * 3))
		return false;

	_paletteSize = len;
	memcpy(_palette, palette, len * 3);
	_colorHash.clear();

	return true;
}

byte PaletteLookup::findBestColor(byte cr, byte cg, byte cb, bool useNaiveAlg) {
	if (_paletteSize == 0) {
		warning("PaletteLookup::findBestColor(): Palette was not set");
		return 0;
	}

	uint bestColor = 0;
	double min = 0xFFFFFFFF;

	uint32 color = cr << 16 | cg << 8 | cb;

	if (_colorHash.contains(color))
		return _colorHash[color];

	if (useNaiveAlg) {
		byte *palettePtr = _palette;

		for (uint i = 0; i < _paletteSize; i++) {
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
		for (uint i = 0; i < _paletteSize; ++i) {
			int rmean = (*(_palette + 3 * i + 0) + cr) / 2;
			int r = *(_palette + 3 * i + 0) - cr;
			int g = *(_palette + 3 * i + 1) - cg;
			int b = *(_palette + 3 * i + 2) - cb;

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

} // end of namespace Graphics
