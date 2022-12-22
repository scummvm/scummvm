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

#ifndef ULTIMA8_GRAPHICS_SHAPEFRAME_H
#define ULTIMA8_GRAPHICS_SHAPEFRAME_H

namespace Ultima {
namespace Ultima8 {

class RawShapeFrame;

/** A decompressed version of the RawShapeFrame for easier rendering */
class ShapeFrame {
public:

	ShapeFrame(const RawShapeFrame *rawframe);
	~ShapeFrame();

	int32 _width, _height;
	int32 _xoff, _yoff;

	uint8 *_pixels;
	uint8 _keycolor;

	bool hasPoint(int32 x, int32 y) const;  // Check to see if a point is in the frame

	uint8 getPixelAtPoint(int32 x, int32 y) const;  // Get the pixel at the point

private:
	/**
	 * Load the pixel data from the raw shape rle data using key color for transparency
	 * @param rawframe the raw shape to load rle data
	 * @param keycolor the color representing transparency
	 * @return false if the keycolor is found in the raw shape frame data
	*/
	bool load(const RawShapeFrame *rawframe, uint8 keycolor);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
