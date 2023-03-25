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

#ifndef DIRECTOR_PICTURE_H
#define DIRECTOR_PICTURE_H

#include "graphics/surface.h"

namespace Image {
class ImageDecoder;
}

namespace Director {

struct Picture {
	Graphics::Surface _surface;
	byte *_palette = nullptr;
	int _paletteColors = 0;

	int getPaletteSize() const {
		return _paletteColors * 3;
	}

	Picture(Image::ImageDecoder &img);
	Picture(Picture &picture);
	~Picture();
private:
	void copyPalette(const byte *src, int numColors);
};

} // End of namespace Director

#endif // DIRECTOR_PICTURE_H
