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

#include "common/std/vector.h"

#include "common/file.h"

#include "qdengine/system/graphics/rle_compress.h"
#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

byte *g_buffer0 = nullptr;
byte *g_buffer1 = nullptr;
uint32 g_buffersLen = 0;

static void ensureBuffers() {
	if (g_buffer0 == nullptr) {
		g_buffer0 = (byte *)calloc(4096, 1);
		g_buffer1 = (byte *)calloc(4096, 1);

		g_buffersLen = 4096U;
	}
}

bool operator == (const RLEBuffer &buf1, const RLEBuffer &buf2) {
	if (!(buf1._header_offset == buf2._header_offset)) return false;
	if (!(buf1._data_offset == buf2._data_offset)) return false;
	if (!(buf1._header == buf2._header)) return false;
	if (!(buf1._data == buf2._data)) return false;

	return true;
}

RLEBuffer::RLEBuffer() : _bits_per_pixel(32) {
}

RLEBuffer::RLEBuffer(const RLEBuffer &buf) : _header_offset(buf._header_offset),
	_data_offset(buf._data_offset),
	_header(buf._header),
	_data(buf._data),
	_bits_per_pixel(buf._bits_per_pixel) {

	ensureBuffers();
}

RLEBuffer::~RLEBuffer() {
	_header_offset.clear();
	_data_offset.clear();
	_header.clear();

	_data.clear();
}

bool RLEBuffer::decode_line(int y, int buffer_id) const {
	ensureBuffers();

	if (buffer_id)
		return decode_line(y, g_buffer1);
	else
		return decode_line(y, g_buffer0);
}

 const byte *RLEBuffer::get_buffer(int buffer_id) {
	ensureBuffers();

	if (buffer_id)
		return g_buffer1;
	else
		return g_buffer0;
}

void RLEBuffer::releaseBuffers() {
	free(g_buffer0);
	g_buffer0 = nullptr;
	free(g_buffer1);
	g_buffer1 = nullptr;
}

RLEBuffer &RLEBuffer::operator = (const RLEBuffer &buf) {
	if (this == &buf) return *this;

	_header_offset = buf._header_offset;
	_data_offset = buf._data_offset;

	_header = buf._header;
	_data = buf._data;

	_bits_per_pixel = buf._bits_per_pixel;

	return *this;
}

bool RLEBuffer::encode(int sx, int sy, const byte *buf) {
	_header_offset.resize(sy);
	_data_offset.resize(sy);

	_header.clear();
	_data.clear();

	_data.reserve(sx * sy);

	const uint32 *buffer = reinterpret_cast<const uint32 *>(buf);

	for (int y = 0; y < sy; y++) {
		int count = 0;

		_header_offset[y] = _header.size();
		_data_offset[y] = _data.size();

		while (count < sx) {
			int index = count;
			uint32 pixel = buffer[index++];

			while (index < sx && index - count < 127 && buffer[index] == pixel)
				index++;

			if (index - count == 1) {
				while (index < sx && index - count < 127 && (buffer[index] != buffer[index - 1] || (index > 1 && buffer[index] != buffer[index - 2])))
					index++;

				while (index < sx && buffer[index] == buffer[index - 1])
					index --;

				_header.push_back(static_cast<char>(count - index));
				for (int i = count; i < index; i++)
					_data.push_back(buffer[i]);
			} else {
				_header.push_back(static_cast<char>(index - count));
				_data.push_back(pixel);
			}

			count = index;
		}
		buffer += sx;
	}
	Std::vector<uint32>(_data).swap(_data);

	resize_buffers();

	return true;
}

bool RLEBuffer::decode_line(int y, byte *out_buf) const {
	const char *header_ptr = &*(_header.begin() + _header_offset[y]);
	const uint32 *data_ptr = &*(_data.begin() + _data_offset[y]);

	uint32 *out_ptr = reinterpret_cast<uint32 *>(out_buf);

	int size = line_header_length(y);

	for (int i = 0; i < size; i++) {
		char count = *header_ptr++;
		if (count > 0) {
			for (int j = 0; j < count; j++)
				*out_ptr++ = *data_ptr;
			data_ptr++;
		} else {
			count = -count;
			memcpy(out_ptr, data_ptr, count * sizeof(uint32));

			out_ptr += count;
			data_ptr += count;
		}
	}

	return true;
}

bool RLEBuffer::decode_pixel(int x, int y, uint32 &pixel) {
	const char *header_ptr = &*(_header.begin() + _header_offset[y]);
	const uint32 *data_ptr = &*(_data.begin() + _data_offset[y]);

	int xx = 0;
	char count = *header_ptr++;

	while (xx + abs(count) < x) {
		if (count > 0) {
			data_ptr++;
		} else {
			count = -count;
			data_ptr += count;
		}
		xx += count;
		count = *header_ptr++;
	}

	if (count > 0) {
		pixel = *data_ptr;
	} else {
		data_ptr += x - xx;
		pixel = *data_ptr;
	}

	return true;
}

uint32 RLEBuffer::size() {
	return _data.size() * sizeof(uint32) + _data_offset.size() + _header_offset.size() * sizeof(uint32) + _header.size();
}

bool RLEBuffer::convert_data(int bits_per_pixel) {
	if (_bits_per_pixel == bits_per_pixel)
		return true;

	int sz = _data.size();

	switch (_bits_per_pixel) {
	case 15:
	case 16:
		if (bits_per_pixel == 24 || bits_per_pixel == 32) {
			uint16 *short_ptr = reinterpret_cast<uint16 *>(&*_data.begin());

			for (int i = 0; i < sz; i++) {
				short_ptr++;
				*short_ptr++ <<= 8;
			}

			short_ptr = reinterpret_cast<uint16 *>(&*_data.begin());
			byte *char_ptr = reinterpret_cast<byte *>(&*_data.begin());

			for (int i = 0; i < sz; i++) {
				byte r, g, b;

				if (_bits_per_pixel == 15)
					grDispatcher::split_rgb555u(*short_ptr++, r, g, b);
				else
					grDispatcher::split_rgb565u(*short_ptr++, r, g, b);

				short_ptr++;

				char_ptr[0] = b;
				char_ptr[1] = g;
				char_ptr[2] = r;

				char_ptr += 4;
			}
		} else {
			uint16 *short_ptr = reinterpret_cast<uint16 *>(&*_data.begin());

			for (int i = 0; i < sz; i++) {
				byte r, g, b;

				if (_bits_per_pixel == 15) {
					grDispatcher::split_rgb555u(*short_ptr, r, g, b);
					*short_ptr++ = grDispatcher::make_rgb565u(r, g, b);
				} else {
					grDispatcher::split_rgb565u(*short_ptr, r, g, b);
					*short_ptr++ = grDispatcher::make_rgb555u(r, g, b);
				}

				short_ptr++;
			}
		}
		break;
	case 24:
	case 32:
		if (bits_per_pixel == 15 || bits_per_pixel == 16) {
			byte *src_ptr = reinterpret_cast<byte *>(&*_data.begin());
			uint16 *dest_ptr = reinterpret_cast<uint16 *>(&*_data.begin());

			for (int i = 0; i < sz; i++) {
				*dest_ptr++ = (bits_per_pixel == 15) ? grDispatcher::make_rgb555u(src_ptr[2], src_ptr[1], src_ptr[0]) : grDispatcher::make_rgb565u(src_ptr[2], src_ptr[1], src_ptr[0]);
				*dest_ptr++ >>= 8;
				src_ptr += 4;
			}
		}
		break;
	}

	_bits_per_pixel = bits_per_pixel;

	return true;
}

void RLEBuffer::resize_buffers() {
	uint32 len = line_length() * sizeof(uint32);

	if (g_buffersLen < len) {
		if (!(g_buffer0 = (byte *)realloc(g_buffer0, len)) || !(g_buffer1 = (byte *)realloc(g_buffer1, len)))
			error("RLEBuffer::resize_buffers(): Cannot realloc buffers");

		g_buffersLen = len;
	}
}

int RLEBuffer::line_length() {
	if (_header_offset.empty()) return 0;

	uint sz = (_header_offset.size() > 1) ? _header_offset[1] : _header.size();

	uint len = 0;
	for (uint i = 0; i < sz; i++) {
		len += abs(_header[i]);
	}

	return len;
}

int RLEBuffer::line_header_length(int line_num) const {
	if (line_num < (int)_header_offset.size() - 1)
		return _header_offset[line_num + 1] - _header_offset[line_num];
	else
		return _header.size() - _header_offset[line_num];
}


bool RLEBuffer::load(Common::SeekableReadStream *fh) {
	uint32 sz = fh->readUint32LE();
	_header_offset.resize(sz);

	sz = fh->readSint32LE();
	_data_offset.resize(sz);

	sz = fh->readSint32LE();
	_header.resize(sz + 1);
	_header[sz] = 0;

	sz = fh->readSint32LE();
	_data.resize(sz);

	for (uint i = 0; i < _header_offset.size(); i++) {
		_header_offset[i] = fh->readUint32LE();
	}

	for (uint i = 0; i < _data_offset.size(); i++) {
		_data_offset[i] = fh->readUint32LE();
	}

	for (uint i = 0; i < _header.size() - 1; i++) {
		_header[i] = fh->readByte();
	}

	for (uint i = 0; i < _data.size(); i++) {
		_data[i] = fh->readUint32LE();
	}

	resize_buffers();

	return true;
}

} // namespace QDEngine
