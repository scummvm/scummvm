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

#ifndef ULTIMA8_GRAPHICS_RAW_SHAPEFRAME_H
#define ULTIMA8_GRAPHICS_RAW_SHAPEFRAME_H

namespace Ultima {
namespace Ultima8 {

struct ConvertShapeFormat;
struct ConvertShapeFrame;

/**
 * A raw shape frame just contains references into the (possibly compressed)
 * data for the shape and is used as an intermediate step in the loading
 * process.
 */
class RawShapeFrame {
	friend class ShapeFrame;
public:

	// parse data.
	//
	// You will find this is quite similar to the ConvertShapeFrame except
	// all the unknown crap is removed. It's designed to allow for painting
	// only, and for speed when loading.

	RawShapeFrame(const uint8 *data, uint32 size, const ConvertShapeFormat *format = 0,
	           const uint8 special[256] = 0, ConvertShapeFrame *prev = 0);
	~RawShapeFrame();

	void getConvertShapeFrame(ConvertShapeFrame &csf);

private:

	uint32              _compressed;
	int32               _width, _height;
	int32               _xoff, _yoff;

	uint32              *_line_offsets;      // Note these are offsets into rle_data
	const uint8         *_rle_data;

	// This will load a u8 style shape 'optimized'.
	void loadU8Format(const uint8 *data, uint32 size);

	// This will load a pentagram style shape 'optimized'.
	void loadPentagramFormat(const uint8 *data, uint32 size);

	// This will load any sort of shape via a ConvertShapeFormat struct
	// Crusader shapes must be loaded this way
	void loadGenericFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format);

	// This will load a u8-compressed shape
	void loadU8CMPFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format, const uint8 special[256], ConvertShapeFrame *prev);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
