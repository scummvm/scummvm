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
#include "ultima/ultima8/convert/convert_shape.h"
#include "ultima/ultima8/convert/u8/convert_shape_u8.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

/*
  parse data and fill class
 */
ShapeFrame::ShapeFrame(const uint8 *data, uint32 size, const ConvertShapeFormat *format,
                       const uint8 special[256], ConvertShapeFrame *prev) : _line_offsets(0) {
	// Load it as u8
	if (!format || format == &U8ShapeFormat || format == &U82DShapeFormat)
		LoadU8Format(data, size);
	else if (format == &PentagramShapeFormat)
		LoadPentagramFormat(data, size);
	else if (format == &U8CMPShapeFormat)
		LoadU8CMPFormat(data, size, format, special, prev);
	else
		LoadGenericFormat(data, size, format);
}

ShapeFrame::~ShapeFrame() {
	delete [] _line_offsets;
}

// Some macros to make things easier
#define READ1(data,offset) (data[offset])
#define READ2(data,offset) (data[offset] + (data[offset+1] << 8))
#define READ4(data,offset) (data[offset] + (data[offset+1] << 8) + (data[offset+2] << 16) + (data[offset+3] << 24))

// This will load a u8 style shape 'optimzed'.
void ShapeFrame::LoadU8Format(const uint8 *data, uint32 /*size*/) {
	_compressed = READ1(data, 8);
	_width = static_cast<int16>(READ2(data, 10));
	_height = static_cast<int16>(READ2(data, 12));
	_xoff = static_cast<int16>(READ2(data, 14));
	_yoff = static_cast<int16>(READ2(data, 16));

	if (_height == 0)
		return;

	_line_offsets = new uint32[_height];

	data += 18;
	for (int32 i = 0; i < _height; i++) {
		_line_offsets[i] = READ2(data, 0) - ((_height - i) * 2);
		data += 2;
	}

	_rle_data = data;
}

// This will load a pentagram style shape 'optimzed'.
void ShapeFrame::LoadPentagramFormat(const uint8 *data, uint32 /*size*/) {
	_compressed = READ1(data, 0);
	_width = static_cast<int32>(READ4(data, 4));
	_height = static_cast<int32>(READ4(data, 8));
	_xoff = static_cast<int32>(READ4(data, 12));
	_yoff = static_cast<int32>(READ4(data, 16));

	if (_height == 0) return;

	_line_offsets = new uint32[_height];

	data += 20;
	for (int32 i = 0; i < _height; i++) {
		_line_offsets[i] = READ4(data, 0);
		data += 4;
	}

	_rle_data = data;
}

// This will load any sort of shape via a ConvertShapeFormat struct
void ShapeFrame::LoadGenericFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format) {
	IBufferDataSource ds(data, size);

	ds.skip(format->_bytes_frame_unknown);
	_compressed = ds.readX(format->_bytes_frame_compression);
	_width = ds.readXS(format->_bytes_frame_width);
	_height = ds.readXS(format->_bytes_frame_height);
	_xoff = ds.readXS(format->_bytes_frame_xoff);
	_yoff = ds.readXS(format->_bytes_frame_yoff);

	if (_height == 0) return;

	_line_offsets = new uint32[_height];

	if (format->_line_offset_absolute) for (int32 i = 0; i < _height; i++) {
			_line_offsets[i] = ds.readX(format->_bytes_line_offset);
		}
	else for (int32 i = 0; i < _height; i++) {
			_line_offsets[i] = ds.readX(format->_bytes_line_offset) - ((_height - i) * format->_bytes_line_offset);
		}

	_rle_data = data + format->_len_frameheader2 + _height * format->_bytes_line_offset;
}

// This will load an U8-compressed shape
void ShapeFrame::LoadU8CMPFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format, const uint8 special[256], ConvertShapeFrame *prev) {
	IBufferDataSource ds(data, size);

	ConvertShapeFrame f;

	f.ReadCmpFrame(&ds, format, special, prev);

	uint32 to_alloc = f._height + (f._bytes_rle + 3) / 4;
	_line_offsets = new uint32[to_alloc];
	_rle_data = reinterpret_cast<uint8 *>(_line_offsets + f._height);

	_compressed = f._compression;
	_height = f._height;
	_width = f._width;
	_xoff = f._xoff;
	_yoff = f._yoff;

	Std::memcpy(_line_offsets, f._line_offsets, f._height * 4);
	Std::memcpy(const_cast<uint8 *>(_rle_data), f._rle_data, f._bytes_rle);

	f.Free();
}

// Checks to see if the frame has a pixel at the point
bool ShapeFrame::hasPoint(int32 x, int32 y) const {
	// Add the offset
	x += _xoff;
	y += _yoff;

	// First gross culling based on dims
	if (x < 0 || y < 0 || x >= _width || y >= _height) return false;

	//
	// This is all pretty simple.
	//
	// All we do is decompress the line the check is on. Then we see if there
	// is a pixel at the location.
	//

	int32 xpos = 0;
	const uint8 *linedata = _rle_data + _line_offsets[y];

	do {
		xpos += *linedata++;

		if (xpos == _width) break;

		int32 dlen = *linedata++;
		int type = 0;

		if (_compressed) {
			type = dlen & 1;
			dlen >>= 1;
		}

		if (x >= xpos && x < (xpos + dlen)) return true;
		xpos += dlen;
		if (!type) linedata += dlen;
		else linedata++;

	} while (xpos < _width);

	return false;
}

// Get the pixel at the point
uint8 ShapeFrame::getPixelAtPoint(int32 x, int32 y) const {
	// Add the offset
	x += _xoff;
	y += _yoff;

	// First gross culling based on dims
	if (x < 0 || y < 0 || x >= _width || y >= _height)
		return 0xFF;

	//
	// This is all pretty simple.
	//
	// All we do is decompress the line the check is on. Then we see if there
	// is a pixel at the location. And if there is, return it
	//

	int32 xpos = 0;
	const uint8 *linedata = _rle_data + _line_offsets[y];

	do {
		xpos += *linedata++;

		if (xpos == _width)
			break;

		int32 dlen = *linedata++;
		int type = 0;

		if (_compressed) {
			type = dlen & 1;
			dlen >>= 1;
		}

		if (x >= xpos && x < (xpos + dlen)) {
			if (!type) linedata += x - xpos;
			return *linedata;
		}
		xpos += dlen;
		if (!type) linedata += dlen;
		else linedata++;

	} while (xpos < _width);

	return 0xFF;
}

void ShapeFrame::getConvertShapeFrame(ConvertShapeFrame &csf, bool need_bytes_rle) {
	csf._compression = _compressed;
	csf._width = _width;
	csf._height = _height;
	csf._xoff = _xoff;
	csf._yoff = _yoff;
	csf._line_offsets = _line_offsets;
	csf._bytes_rle = 0;
	csf._rle_data = const_cast<uint8 *>(_rle_data);
}

} // End of namespace Ultima8
} // End of namespace Ultima
