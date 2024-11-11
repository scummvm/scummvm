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

#include "image/image_decoder.h"
#include "director/picture.h"

namespace Director {

Picture::Picture(Image::ImageDecoder &img) {
	_surface.copyFrom(*img.getSurface());
	copyPalette(img.getPalette(), img.getPaletteColorCount());
}

Picture::Picture(Picture &picture) {
	_surface.copyFrom(picture._surface);
	copyPalette(picture._palette, picture._paletteColors);
}

Picture::~Picture() {
	_surface.free();
	delete[] _palette;
}

void Picture::copyPalette(const byte *src, int numColors) {
	delete[] _palette;
	if (src) {
		_paletteColors = numColors;
		_palette = new byte[3 * 256]();
		memcpy(_palette, src, getPaletteSize());
	} else {
		_paletteColors = 0;
	}
}

} // End of namespace Director
