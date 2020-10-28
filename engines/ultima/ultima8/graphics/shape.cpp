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

#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/raw_shape_frame.h"
#include "ultima/ultima8/convert/convert_shape.h"
#include "ultima/ultima8/convert/u8/convert_shape_u8.h"
#include "ultima/ultima8/convert/crusader/convert_shape_crusader.h"
#include "ultima/ultima8/filesys/idata_source.h"

#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

Shape::Shape(const uint8 *data, uint32 size, const ConvertShapeFormat *format,
             const uint16 id, const uint32 shape)
		: _flexId(id), _shapeNum(shape), _palette(nullptr) {
	// NB: U8 style!
	loadFrames(data, size, format);

	delete[] const_cast<uint8 *>(data);
}

Shape::Shape(IDataSource *src, const ConvertShapeFormat *format)
	: _flexId(0), _shapeNum(0), _palette(nullptr) {
	// NB: U8 style!
	uint32 size = src->size();
	uint8 *data = new uint8[size];
	src->read(data, size);

	loadFrames(data, size, format);

	delete[] data;
}

Shape::~Shape() {
	for (uint i = 0; i < _frames.size(); ++i)
		delete _frames[i];
}

void Shape::loadFrames(const uint8 *data, uint32 size, const ConvertShapeFormat *format) {
	if (!format)
		format = DetectShapeFormat(data, size);

	if (!format) {
		// Should be fatal?
		perr << "Error: Unable to detect shape format" << Std::endl;
		return;
	}

	Common::Array<RawShapeFrame *> rawframes;
	// Load it as u8

	if (format == &U8ShapeFormat || format == &U82DShapeFormat)
		rawframes = loadU8Format(data, size, format);
	else if (format == &PentagramShapeFormat)
		rawframes = loadPentagramFormat(data, size, format);
	else
		rawframes = loadGenericFormat(data, size, format);

	for (uint i = 0; i < rawframes.size(); i++) {
		_frames.push_back(new ShapeFrame(rawframes[i]));
		delete rawframes[i];
	}
}

void Shape::getShapeId(uint16 &id, uint32 &shape) const {
	id = _flexId;
	shape = _shapeNum;
}

// This will load a u8 style shape 'optimized'.
Common::Array<RawShapeFrame *> Shape::loadU8Format(const uint8 *data, uint32 size, const ConvertShapeFormat *format) {
	Common::MemoryReadStream stream(data, size);
	stream.skip(4); // skip header
	unsigned int framecount = stream.readUint16LE();

	Common::Array<RawShapeFrame *> frames;

	if (framecount == 0) {
		return loadGenericFormat(data, size, format);
	}

	frames.reserve(framecount);

	for (uint i = 0; i < framecount; ++i) {
		uint32 frameoffset = stream.readUint32LE() & 0xFFFFFF;
		uint32 framesize = stream.readUint16LE();

		frames.push_back(new RawShapeFrame(data + frameoffset, framesize, format));
	}

	return frames;
}

// This will load a pentagram style shape 'optimized'.
Common::Array<RawShapeFrame *> Shape::loadPentagramFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format) {
	Common::MemoryReadStream stream(data, size);
	stream.skip(4); // skip header
	unsigned int framecount = stream.readUint16LE();
	Common::Array<RawShapeFrame *> frames;

	if (framecount == 0) {
		return loadGenericFormat(data, size, format);
	}

	frames.reserve(framecount);

	for (uint i = 0; i < framecount; ++i) {
		uint32 frameoffset = stream.readUint32LE();
		uint32 framesize = stream.readUint32LE();

		frames.push_back(new RawShapeFrame(data + frameoffset, framesize, format));
	}

	return frames;
}

// This will load any sort of shape via a ConvertShapeFormat struct
Common::Array<RawShapeFrame *> Shape::loadGenericFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format) {
	uint32 framecount;
	uint32 frameoffset;
	uint32 framesize;
	IBufferDataSource ds(data, size);

	Common::Array<RawShapeFrame *> frames;

	if (format->_bytes_ident) {
		uint8 *ident = new uint8[format->_bytes_ident];
		ds.read(ident, format->_bytes_ident);
		bool match = Std::memcmp(ident, format->_ident, format->_bytes_ident) == 0;
		delete[] ident;

		if (!match) {
			frames.clear();
			return frames;
		}
	}

	// Read special buffer
	uint8 special[256];
	if (format->_bytes_special) {
		memset(special, 0, 256);
		for (uint32 i = 0; i < format->_bytes_special; i++) special[ds.readByte() & 0xFF] = i + 2;
	}

	// Skip unknown
	if (format->_bytes_header_unk && format != &Crusader2DShapeFormat) {
		//uint32 val =
		ds.readX(format->_bytes_header_unk);
		//uint16 lowval = val & 0xff;
		//uint16 highval = (val >> 16) & 0xff;
		//uint32 dummy = 0 + lowval + highval + val;
	} else {
		// Appears to be shape Width x Height for Crusader 2D shapes,
		// not needed - we get them by frame.
		ds.skip(format->_bytes_header_unk);
	}

	// Read framecount, default 1 if no
	if (format->_bytes_num_frames) framecount = ds.readX(format->_bytes_num_frames);
	else framecount = 1;
	if (framecount == 0) framecount = ConvertShape::CalcNumFrames(&ds, format, size, 0);

	frames.reserve(framecount);

	for (uint i = 0; i < framecount; ++i) {
		// Read the offset
		if (format->_bytes_frame_offset) frameoffset = ds.readX(format->_bytes_frame_offset) + format->_bytes_special;
		else frameoffset = format->_len_header + (format->_len_frameheader * i);

		// Skip the unknown
		if (format->_bytes_frameheader_unk) {
			ds.readX(format->_bytes_frameheader_unk);
		}

		// Read frame_length
		if (format->_bytes_frame_length) framesize = ds.readX(format->_bytes_frame_length) + format->_bytes_frame_length_kludge;
		else framesize = size - frameoffset;

		if (framesize > size) {
			warning("shape frame %d goes off the end of the buffer, stopping early", i);
			break;
		}

		ConvertShapeFrame *prev = nullptr, p;

		if (format->_bytes_special && i > 0) {
			prev = &p;
			frames[i - 1]->getConvertShapeFrame(p);
		}

		frames.push_back(new RawShapeFrame(data + frameoffset, framesize, format, special, prev));
	}

	return frames;
}

// This will detect the format of a shape
const ConvertShapeFormat *Shape::DetectShapeFormat(const uint8 *data, uint32 size) {
	IBufferDataSource ds(data, size);
	return Shape::DetectShapeFormat(&ds, size);
}

const ConvertShapeFormat *Shape::DetectShapeFormat(IDataSource *ds, uint32 size) {
	const ConvertShapeFormat *ret = nullptr;

	if (ConvertShape::CheckUnsafe(ds, &PentagramShapeFormat, size))
		ret = &PentagramShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &U8SKFShapeFormat, size))
		ret = &U8SKFShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &U8ShapeFormat, size))
		ret = &U8ShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &U82DShapeFormat, size))
		ret = &U82DShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &CrusaderShapeFormat, size))
		ret = &CrusaderShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &Crusader2DShapeFormat, size))
		ret = &Crusader2DShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &U8CMPShapeFormat, size))
		ret = &U8CMPShapeFormat;

	return ret;
}

void Shape::getTotalDimensions(int32 &w, int32 &h, int32 &x, int32 &y) const {
	if (_frames.empty()) {
		w = 0;
		h = 0;
		x = 0;
		y = 0;
		return;
	}

	int32 minx = 1000000, maxx = -1000000;
	int32 miny = 1000000, maxy = -1000000;

	for (uint i = 0; i < _frames.size(); ++i) {
		ShapeFrame *frame = _frames[i];
		if (-frame->_xoff < minx)
			minx = -frame->_xoff;
		if (-frame->_yoff < miny)
			miny = -frame->_yoff;
		if (frame->_width - frame->_xoff - 1 > maxx)
			maxx = frame->_width - frame->_xoff - 1;
		if (frame->_height - frame->_yoff - 1 > maxy)
			maxy = frame->_height - frame->_yoff - 1;
	}

	w = maxx - minx + 1;
	h = maxy - miny + 1;
	x = -minx;
	y = -miny;
}

const ShapeFrame *Shape::getFrame(unsigned int frame) const {
	if (frame < _frames.size())
		return _frames[frame];
	else
		return nullptr;
}


} // End of namespace Ultima8
} // End of namespace Ultima
