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

#include "ultima/ultima8/misc/common_types.h"

#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/gfx/raw_shape_frame.h"

namespace Ultima {
namespace Ultima8 {

ShapeFrame::ShapeFrame(const RawShapeFrame *rawframe) :
		_width(_surface.w), _height(_surface.h),
		_xoff(rawframe->_xoff), _yoff(rawframe->_yoff),
		_keycolor(0xFF) {

	_surface.create(rawframe->_width, rawframe->_height, Graphics::PixelFormat::createFormatCLUT8());

	// load adjusting keycolor until success
	if (!load(rawframe, _keycolor)) {
		_keycolor = 0;
		while (!load(rawframe, _keycolor) && _keycolor < 0xFF) {
			_keycolor++;
		}
	}
}

ShapeFrame::~ShapeFrame() {
	_surface.free();
}

bool ShapeFrame::load(const RawShapeFrame *rawframe, uint8 keycolor) {
	bool result = true;
	uint8 *pixels = reinterpret_cast<uint8 *>(_surface.getPixels());
	memset(pixels, keycolor, _surface.w * _surface.h);

	for (int y = 0; y < _surface.h; y++) {
		int32 xpos = 0;
		const uint8 *linedata = rawframe->_rle_data + rawframe->_line_offsets[y];

		do {
			xpos += *linedata++;

			if (xpos >= _surface.w)
				break;

			int32 dlen = *linedata++;
			int type = 0;

			if (rawframe->_compressed) {
				type = dlen & 1;
				dlen >>= 1;
			}

			for (int doff = 0; doff < dlen; doff++) {
				if (*linedata == keycolor)
					result = false;
				pixels[y * _surface.w + xpos + doff] = *linedata;
				if (!type) {
					linedata++;
				}
			}

			xpos += dlen;
			if (type) {
				linedata++;
			}

		} while (xpos < _surface.w);
	}
	return result;
}

// Checks to see if the frame has a pixel at the point
bool ShapeFrame::hasPoint(int x, int y) const {
	// Add the offset
	x += _xoff;
	y += _yoff;

	// First gross culling based on dims
	if (x < 0 || y < 0 || x >= _surface.w || y >= _surface.h)
		return false;

	return _surface.getPixel(x, y) != _keycolor;
}

// Get the pixel at the point
uint8 ShapeFrame::getPixel(int x, int y) const {
	// Add the offset
	x += _xoff;
	y += _yoff;

	// First gross culling based on dims
	if (x < 0 || y < 0 || x >= _surface.w || y >= _surface.h)
		return _keycolor;

	return _surface.getPixel(x, y);
}

} // End of namespace Ultima8
} // End of namespace Ultima
