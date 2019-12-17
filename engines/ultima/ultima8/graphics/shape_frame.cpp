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
                       const uint8 special[256], ConvertShapeFrame *prev) : line_offsets(0) {
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
	delete [] line_offsets;
}

// Some macros to make things easier
#define READ1(data,offset) (data[offset])
#define READ2(data,offset) (data[offset] + (data[offset+1] << 8))
#define READ4(data,offset) (data[offset] + (data[offset+1] << 8) + (data[offset+2] << 16) + (data[offset+3] << 24))

// This will load a u8 style shape 'optimzed'.
void ShapeFrame::LoadU8Format(const uint8 *data, uint32 /*size*/) {
	compressed = READ1(data, 8);
	width = static_cast<int16>(READ2(data, 10));
	height = static_cast<int16>(READ2(data, 12));
	xoff = static_cast<int16>(READ2(data, 14));
	yoff = static_cast<int16>(READ2(data, 16));

	if (height == 0) return;

	line_offsets = new uint32[height];

	data += 18;
	for (int32 i = 0; i < height; i++) {
		line_offsets[i] = READ2(data, 0) - ((height - i) * 2);
		data += 2;
	}

	rle_data = data;
}

// This will load a pentagram style shape 'optimzed'.
void ShapeFrame::LoadPentagramFormat(const uint8 *data, uint32 /*size*/) {
	compressed = READ1(data, 0);
	width = static_cast<int32>(READ4(data, 4));
	height = static_cast<int32>(READ4(data, 8));
	xoff = static_cast<int32>(READ4(data, 12));
	yoff = static_cast<int32>(READ4(data, 16));

	if (height == 0) return;

	line_offsets = new uint32[height];

	data += 20;
	for (int32 i = 0; i < height; i++) {
		line_offsets[i] = READ4(data, 0);
		data += 4;
	}

	rle_data = data;
}

// This will load any sort of shape via a ConvertShapeFormat struct
void ShapeFrame::LoadGenericFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format) {
	IBufferDataSource ds(data, size);

	ds.skip(format->bytes_frame_unknown);
	compressed = ds.readX(format->bytes_frame_compression);
	width = ds.readXS(format->bytes_frame_width);
	height = ds.readXS(format->bytes_frame_height);
	xoff = ds.readXS(format->bytes_frame_xoff);
	yoff = ds.readXS(format->bytes_frame_yoff);

	if (height == 0) return;

	line_offsets = new uint32[height];

	if (format->line_offset_absolute) for (int32 i = 0; i < height; i++) {
			line_offsets[i] = ds.readX(format->bytes_line_offset);
		}
	else for (int32 i = 0; i < height; i++) {
			line_offsets[i] = ds.readX(format->bytes_line_offset) - ((height - i) * format->bytes_line_offset);
		}

	rle_data = data + format->len_frameheader2 + height * format->bytes_line_offset;
}

// This will load an U8-compressed shape
void ShapeFrame::LoadU8CMPFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format, const uint8 special[256], ConvertShapeFrame *prev) {
	IBufferDataSource ds(data, size);

	ConvertShapeFrame f;

	f.ReadCmpFrame(&ds, format, special, prev);

	uint32 to_alloc = f.height + (f.bytes_rle + 3) / 4;
	line_offsets = new uint32[to_alloc];
	rle_data = reinterpret_cast<uint8 *>(line_offsets + f.height);

	compressed = f.compression;
	height = f.height;
	width = f.width;
	xoff = f.xoff;
	yoff = f.yoff;

	std::memcpy(line_offsets, f.line_offsets, f.height * 4);
	std::memcpy(const_cast<uint8 *>(rle_data), f.rle_data, f.bytes_rle);

	f.Free();
}

// Checks to see if the frame has a pixel at the point
bool ShapeFrame::hasPoint(int32 x, int32 y) const {
	// Add the offset
	x += xoff;
	y += yoff;

	// First gross culling based on dims
	if (x < 0 || y < 0 || x >= width || y >= height) return false;

	//
	// This is all pretty simple.
	//
	// All we do is decompress the line the check is on. Then we see if there
	// is a pixel at the location.
	//

	int32 xpos = 0;
	const uint8 *linedata = rle_data + line_offsets[y];

	do {
		xpos += *linedata++;

		if (xpos == width) break;

		int32 dlen = *linedata++;
		int type = 0;

		if (compressed) {
			type = dlen & 1;
			dlen >>= 1;
		}

		if (x >= xpos && x < (xpos + dlen)) return true;
		xpos += dlen;
		if (!type) linedata += dlen;
		else linedata++;

	} while (xpos < width);

	return false;
}

// Get the pixel at the point
uint8 ShapeFrame::getPixelAtPoint(int32 x, int32 y) const {
	// Add the offset
	x += xoff;
	y += yoff;

	// First gross culling based on dims
	if (x < 0 || y < 0 || x >= width || y >= height) return 0xFF;

	//
	// This is all pretty simple.
	//
	// All we do is decompress the line the check is on. Then we see if there
	// is a pixel at the location. And if there is, return it
	//

	int32 xpos = 0;
	const uint8 *linedata = rle_data + line_offsets[y];

	do {
		xpos += *linedata++;

		if (xpos == width) break;

		int32 dlen = *linedata++;
		int type = 0;

		if (compressed) {
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

	} while (xpos < width);

	return 0xFF;
}

void ShapeFrame::getConvertShapeFrame(ConvertShapeFrame &csf, bool need_bytes_rle) {
	csf.compression = compressed;
	csf.width = width;
	csf.height = height;
	csf.xoff = xoff;
	csf.yoff = yoff;
	csf.line_offsets = line_offsets;
	csf.bytes_rle = 0;
	csf.rle_data = const_cast<uint8 *>(rle_data);
}

} // End of namespace Ultima8
} // End of namespace Ultima
