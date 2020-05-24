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

#include "ultima/ultima8/graphics/raw_shape_frame.h"
#include "ultima/ultima8/convert/convert_shape.h"
#include "ultima/ultima8/convert/u8/convert_shape_u8.h"
#include "ultima/ultima8/filesys/idata_source.h"

#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

/*
  parse data and fill class
 */
RawShapeFrame::RawShapeFrame(const uint8 *data, uint32 size, const ConvertShapeFormat *format,
                       const uint8 special[256], ConvertShapeFrame *prev) : _line_offsets(0),
					   _rle_data(nullptr) {
	// Load it as u8
	if (!format || format == &U8ShapeFormat || format == &U82DShapeFormat)
		loadU8Format(data, size);
	else if (format == &PentagramShapeFormat)
		loadPentagramFormat(data, size);
	else if (format == &U8CMPShapeFormat)
		loadU8CMPFormat(data, size, format, special, prev);
	else
		loadGenericFormat(data, size, format);
}

RawShapeFrame::~RawShapeFrame() {
	delete [] _line_offsets;
}

// This will load a u8 style shape 'optimized'.
void RawShapeFrame::loadU8Format(const uint8 *data, uint32 size) {
	Common::MemoryReadStream stream(data, size + 8);

	stream.skip(8); // skip header
	_compressed = stream.readByte();
	stream.skip(1);
	_width = stream.readUint16LE();
	_height = stream.readUint16LE();
	_xoff = stream.readUint16LE();
	_yoff = stream.readUint16LE();

	if (_height == 0)
		return;

	_line_offsets = new uint32[_height];

	for (int32 i = 0; i < _height; i++) {
		_line_offsets[i] = stream.readUint16LE() - ((_height - i) * 2);
	}

	_rle_data = data + stream.pos();
}

// This will load a pentagram style shape 'optimized'.
void RawShapeFrame::loadPentagramFormat(const uint8 *data, uint32 size) {
	Common::MemoryReadStream stream(data, size);

	_compressed = stream.readByte();
	stream.skip(3);
	_width = stream.readSint32LE();
	_height = stream.readSint32LE();
	_xoff = stream.readSint32LE();
	_yoff = stream.readSint32LE();

	if (_height == 0)
		return;

	_line_offsets = new uint32[_height];

	for (int32 i = 0; i < _height; i++) {
		_line_offsets[i] = stream.readSint32LE();
	}

	_rle_data = data + stream.pos();
}

// This will load any sort of shape via a ConvertShapeFormat struct
void RawShapeFrame::loadGenericFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format) {
	IBufferDataSource ds(data + format->_bytes_frame_unknown, size);

	_compressed = ds.readX(format->_bytes_frame_compression);
	_width = ds.readXS(format->_bytes_frame_width);
	_height = ds.readXS(format->_bytes_frame_height);
	_xoff = ds.readXS(format->_bytes_frame_xoff);
	_yoff = ds.readXS(format->_bytes_frame_yoff);

	if (_height == 0)
		return;

	// Fairly arbitrary sanity check
	if (_height > 4096 || _width > 4096 || _xoff > 4096 || _yoff > 4096) {
		warning("got some invalid data loading shape");
		_width = _height = _xoff = _yoff = 0;
		return;
	}

	_line_offsets = new uint32[_height];

	for (int32 i = 0; i < _height; i++) {
		if (format->_line_offset_absolute) {
			_line_offsets[i] = ds.readX(format->_bytes_line_offset);
		} else {
			if (ds.size() - ds.pos() < (int32)format->_bytes_line_offset) {
				warning("going off end of %d buffer at %d reading %d",
						ds.size(), ds.pos(), format->_bytes_line_offset);
			}
			_line_offsets[i] = ds.readX(format->_bytes_line_offset) - ((_height - i) * format->_bytes_line_offset);
		}
	}

	_rle_data = data + format->_len_frameheader2 + _height * format->_bytes_line_offset;
}

// This will load an U8-compressed shape
void RawShapeFrame::loadU8CMPFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format, const uint8 special[256], ConvertShapeFrame *prev) {
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

	memcpy(_line_offsets, f._line_offsets, f._height * 4);
	memcpy(const_cast<uint8 *>(_rle_data), f._rle_data, f._bytes_rle);

	f.Free();
}

void RawShapeFrame::getConvertShapeFrame(ConvertShapeFrame &csf) {
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
