/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/raw_shape_frame.h"

namespace Ultima {
namespace Ultima8 {

ShapeFrame::ShapeFrame(const RawShapeFrame *rawframe) :
		_xoff(rawframe->_xoff), _yoff(rawframe->_yoff),
		_width(rawframe->_width), _height(rawframe->_height) {

	_pixels = new uint8[_width * _height];
	_mask = new uint8[_width * _height];

	memset(_mask, 0, _width * _height);

	for (int y = 0; y < _height; y++) {
		int32 xpos = 0;
		const uint8 *linedata = rawframe->_rle_data + rawframe->_line_offsets[y];

		do {
			xpos += *linedata++;

			if (xpos >= _width)
				break;

			int32 dlen = *linedata++;
			int type = 0;

			if (rawframe->_compressed) {
				type = dlen & 1;
				dlen >>= 1;
			}

			for (int doff = 0; doff < dlen; doff++) {
				_pixels[y * _width + xpos + doff] = *linedata;
				_mask[y * _width + xpos + doff] = 1;
				if (!type) {
					linedata++;
				}
			}

			xpos += dlen;
			if (type) {
				linedata++;
			}

		} while (xpos < _width);

	}

}

ShapeFrame::~ShapeFrame() {
	delete [] _pixels;
	delete [] _mask;
}

// Checks to see if the frame has a pixel at the point
bool ShapeFrame::hasPoint(int32 x, int32 y) const {
	// Add the offset
	x += _xoff;
	y += _yoff;

	// First gross culling based on dims
	if (x < 0 || y < 0 || x >= _width || y >= _height)
		return false;

	return _mask[y * _width + x] != 0;
}

// Get the pixel at the point
uint8 ShapeFrame::getPixelAtPoint(int32 x, int32 y) const {
	// Add the offset
	x += _xoff;
	y += _yoff;

	// First gross culling based on dims
	if (x < 0 || y < 0 || x >= _width || y >= _height)
		return 0xFF;

	return _pixels[y * _width + x];
}

} // End of namespace Ultima8
} // End of namespace Ultima
