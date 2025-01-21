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

#include "got/gfx/gfx_pics.h"
#include "common/file.h"
#include "got/utils/file.h"

namespace Got {
namespace Gfx {

void convertPaneDataToSurface(const byte *src, Graphics::ManagedSurface &surf) {
	surf.setTransparentColor(0);

	// It's split into 4 panes per 4 pixels, so we need
	// to juggle the pixels into their correct order
	for (int plane = 0; plane < 4; ++plane) {
		for (int y = 0; y < surf.h; ++y) {
			byte *dest = (byte *)surf.getBasePtr(plane, y);

			for (int x = 0; x < (surf.w / 4); ++x, dest += 4, ++src) {
				// For some reason, both '0' and '15' are both hard-coded
				// as transparent colors in the graphics drawing. Simplify
				// this for ScummVM by changing all 15's to 0
				*dest = (*src == 15) ? 0 : *src;
			}
		}
	}
}

void GfxPics::clear() {
	delete[] _array;
	_array = nullptr;
}

void GfxPics::resize(uint newSize) {
	assert(!_array); // Don't support multiple resizes
	_array = new Graphics::ManagedSurface[newSize];
	_size = newSize;
}

void GfxPics::load(const Common::String &name, int blockSize) {
	File f(name);

	// Set up array of images
	clear();
	if (blockSize == -1) {
		// Only a single image
		resize(1);
		blockSize = f.size();
	} else {
		resize(f.size() / blockSize);
	}

	byte *buff = new byte[blockSize];

	for (uint idx = 0; idx < size(); ++idx) {
		int w = f.readUint16LE() * 4;
		int h = f.readUint16LE();
		f.skip(2); // Unused transparent color. It's always 15
		f.read(buff, blockSize - 6);

		Graphics::ManagedSurface &s = (*this)[idx];
		s.create(w, h);

		convertPaneDataToSurface(buff, s);
	}

	delete[] buff;
}

GfxPics &GfxPics::operator=(const GfxPics &src) {
	clear();
	resize(src._size);

	for (size_t i = 0; i < _size; ++i)
		_array[i].copyFrom(src._array[i]);

	return *this;
}

void BgPics::setArea(int area) {
	if (area != _area) {
		_area = area;
		load();
	}
}

void BgPics::load() {
	const Common::String name = Common::String::format("BPICS%d", _area);
	GfxPics::load(name, 262);
}

} // namespace Gfx
} // namespace Got
