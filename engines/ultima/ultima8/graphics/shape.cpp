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
#include "ultima/ultima8/convert/convert_shape.h"
#include "ultima/ultima8/convert/u8/convert_shape_u8.h"
#include "ultima/ultima8/convert/crusader/convert_shape_crusader.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Shape)

DEFINE_CUSTOM_MEMORY_ALLOCATION(Shape)

Shape::Shape(const uint8 *data_, uint32 size_, const ConvertShapeFormat *format,
             const uint16 id, const uint32 shape) : _flexId(id), _shapeNum(shape) {
	// NB: U8 style!

	this->_data = data_;
	this->_size = size_;
	this->_palette = nullptr;

	if (!format) format = DetectShapeFormat(data_, size_);

	if (!format) {
		// Should be fatal?
		perr << "Error: Unable to detect shape format" << Std::endl;
		return;
	}

	// Load it as u8
	if (format == &U8ShapeFormat || format == &U82DShapeFormat)
		LoadU8Format(data_, size_, format);
	else if (format == &PentagramShapeFormat)
		LoadPentagramFormat(data_, size_, format);
	else
		LoadGenericFormat(data_, size_, format);
}

Shape::Shape(IDataSource *src, const ConvertShapeFormat *format)
	: _flexId(0), _shapeNum(0) {
	// NB: U8 style!

	this->_size = src->getSize();
	uint8 *d = new uint8[this->_size];
	this->_data = d;
	src->read(d, this->_size);
	this->_palette = nullptr;

	if (!format)
		format = DetectShapeFormat(_data, _size);

	if (!format) {
		// Should be fatal?
		perr << "Error: Unable to detect shape format" << Std::endl;
		return;
	}

	// Load it as u8
	if (format == &U8ShapeFormat || format == &U82DShapeFormat)
		LoadU8Format(_data, _size, format);
	else if (format == &PentagramShapeFormat)
		LoadPentagramFormat(_data, _size, format);
	else
		LoadGenericFormat(_data, _size, format);
}

Shape::~Shape() {
	for (unsigned int i = 0; i < _frames.size(); ++i)
		delete _frames[i];

	delete[] const_cast<uint8 *>(_data);
}

void Shape::getShapeId(uint16 &id, uint32 &shape) {
	id = _flexId;
	shape = _shapeNum;
}

// Some macros to make things easier
#define READ1(data,offset) (data[offset])
#define READ2(data,offset) (data[offset] + (data[offset+1] << 8))
#define READ3(data,offset) (data[offset] + (data[offset+1] << 8) + (data[offset+2] << 16))
#define READ4(data,offset) (data[offset] + (data[offset+1] << 8) + (data[offset+2] << 16) + (data[offset+3] << 24))

// This will load a u8 style shape 'optimzed'.
void Shape::LoadU8Format(const uint8 *data_, uint32 size_, const ConvertShapeFormat *format) {
	unsigned int framecount = READ2(data_, 4);

	if (framecount == 0) {
		LoadGenericFormat(data_, size_, format);
		return;
	}

	_frames.reserve(framecount);

	for (unsigned int i = 0; i < framecount; ++i) {
		uint32 frameoffset = READ3(data_, 6 + 6 * i);
		uint32 framesize = READ2(data_, 10 + 6 * i);

		_frames.push_back(new ShapeFrame(data_ + frameoffset, framesize, format));
	}
}

// This will load a pentagram style shape 'optimzed'.
void Shape::LoadPentagramFormat(const uint8 *data_, uint32 size_, const ConvertShapeFormat *format) {
	unsigned int framecount = READ4(data_, 4);

	if (framecount == 0) {
		LoadGenericFormat(data_, size_, format);
		return;
	}

	_frames.reserve(framecount);

	for (unsigned int i = 0; i < framecount; ++i) {
		uint32 frameoffset = READ4(data_, 8 + 8 * i);
		uint32 framesize = READ4(data_, 12 + 8 * i);

		_frames.push_back(new ShapeFrame(data_ + frameoffset, framesize, format));
	}
}

// This will load any sort of shape via a ConvertShapeFormat struct
void Shape::LoadGenericFormat(const uint8 *data, uint32 size, const ConvertShapeFormat *format) {
	uint32 framecount;
	uint32 frameoffset;
	uint32 framesize;
	IBufferDataSource ds(data, size);

	if (format->_bytes_ident) {
		uint8 *ident = new uint8[format->_bytes_ident];
		ds.read(ident, format->_bytes_ident);
		bool match = Std::memcmp(ident, format->_ident, format->_bytes_ident) == 0;
		delete[] ident;

		if (!match) {
			_frames.clear();
			return;
		}
	}

	// Read special buffer
	uint8 special[256];
	if (format->_bytes_special) {
		memset(special, 0, 256);
		for (uint32 i = 0; i < format->_bytes_special; i++) special[ds.read1() & 0xFF] = i + 2;
	}

	// Skip unknown
	ds.skip(format->_bytes_header_unk);

	// Read framecount, default 1 if no
	if (format->_bytes_num_frames) framecount = ds.readX(format->_bytes_num_frames);
	else framecount = 1;
	if (framecount == 0) framecount = ConvertShape::CalcNumFrames(&ds, format, size, 0);

	_frames.reserve(framecount);

	for (unsigned int i = 0; i < framecount; ++i) {
		// Read the offset
		if (format->_bytes_frame_offset) frameoffset = ds.readX(format->_bytes_frame_offset) + format->_bytes_special;
		else frameoffset = format->_len_header + (format->_len_frameheader * i);

		// Skip the unknown
		ds.skip(format->_bytes_frameheader_unk);

		// Read frame_length
		if (format->_bytes_frame_length) framesize = ds.readX(format->_bytes_frame_length) + format->_bytes_frame_length_kludge;
		else framesize = size - frameoffset;

		ConvertShapeFrame *prev = nullptr, p;

		if (format->_bytes_special && i > 0) {
			prev = &p;
			_frames[i - 1]->getConvertShapeFrame(p);
		}

		_frames.push_back(new ShapeFrame(data + frameoffset, framesize, format, special, prev));
	}
}

// This will detect the format of a shape
const ConvertShapeFormat *Shape::DetectShapeFormat(const uint8 *data, uint32 size) {
	IBufferDataSource ds(data, size);
	return Shape::DetectShapeFormat(&ds, size);
}

const ConvertShapeFormat *Shape::DetectShapeFormat(IDataSource *ds, uint32 size_) {
	const ConvertShapeFormat *ret = nullptr;

	if (ConvertShape::CheckUnsafe(ds, &PentagramShapeFormat, size_))
		ret = &PentagramShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &U8SKFShapeFormat, size_))
		ret = &U8SKFShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &U8ShapeFormat, size_))
		ret = &U8ShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &U82DShapeFormat, size_))
		ret = &U82DShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &CrusaderShapeFormat, size_))
		ret = &CrusaderShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &Crusader2DShapeFormat, size_))
		ret = &Crusader2DShapeFormat;
	else if (ConvertShape::CheckUnsafe(ds, &U8CMPShapeFormat, size_))
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

	for (unsigned int i = 0; i < _frames.size(); ++i) {
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

ShapeFrame *Shape::getFrame(unsigned int frame) {
	if (frame < _frames.size())
		return _frames[frame];
	else
		return nullptr;
}


} // End of namespace Ultima8
} // End of namespace Ultima
