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

#define _NO_ZIP_
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/file.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/system/graphics/rle_compress.h"
#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

std::vector<unsigned char> rleBuffer::_buffer0(4096);
std::vector<unsigned char> rleBuffer::_buffer1(4096);

bool operator == (const rleBuffer &buf1, const rleBuffer &buf2) {
	if (!(buf1._header_offset == buf2._header_offset)) return false;
	if (!(buf1._data_offset == buf2._data_offset)) return false;
	if (!(buf1._header == buf2._header)) return false;
	if (!(buf1._data == buf2._data)) return false;

	return true;
}

rleBuffer::rleBuffer() : _bits_per_pixel(32) {
}

rleBuffer::rleBuffer(const rleBuffer &buf) : _header_offset(buf._header_offset),
	_data_offset(buf._data_offset),
	_header(buf._header),
	_data(buf._data),
	_bits_per_pixel(buf._bits_per_pixel) {
}

rleBuffer::~rleBuffer() {
	_header_offset.clear();
	_data_offset.clear();
	_header.clear();

	_data.clear();
}

rleBuffer &rleBuffer::operator = (const rleBuffer &buf) {
	if (this == &buf) return *this;

	_header_offset = buf._header_offset;
	_data_offset = buf._data_offset;

	_header = buf._header;
	_data = buf._data;

	_bits_per_pixel = buf._bits_per_pixel;

	return *this;
}

bool rleBuffer::encode(int sx, int sy, const unsigned char *buf) {
	_header_offset.resize(sy);
	_data_offset.resize(sy);

	_header.clear();
	_data.clear();

	_data.reserve(sx * sy);

	const unsigned *buffer = reinterpret_cast<const unsigned *>(buf);

	for (int y = 0; y < sy; y ++) {
		int count = 0;

		_header_offset[y] = _header.size();
		_data_offset[y] = _data.size();

		while (count < sx) {
			int index = count;
			unsigned pixel = buffer[index ++];

			while (index < sx && index - count < 127 && buffer[index] == pixel)
				index ++;

			if (index - count == 1) {
				while (index < sx && index - count < 127 && (buffer[index] != buffer[index - 1] || (index > 1 && buffer[index] != buffer[index - 2])))
					index ++;

				while (index < sx && buffer[index] == buffer[index - 1])
					index --;

				_header.push_back(static_cast<char>(count - index));
				for (int i = count; i < index; i ++)
					_data.push_back(buffer[i]);
			} else {
				_header.push_back(static_cast<char>(index - count));
				_data.push_back(pixel);
			}

			count = index;
		}
		buffer += sx;
	}
	std::vector<unsigned>(_data).swap(_data);

	resize_buffers();

	return true;
}

bool rleBuffer::decode_line(int y, unsigned char *out_buf) const {
	const char *header_ptr = &*(_header.begin() + _header_offset[y]);
	const unsigned *data_ptr = &*(_data.begin() + _data_offset[y]);

	unsigned *out_ptr = reinterpret_cast<unsigned *>(out_buf);

	int size = line_header_length(y);

	for (int i = 0; i < size; i ++) {
		char count = *header_ptr++;
		if (count > 0) {
			for (int j = 0; j < count; j ++)
				*out_ptr++ = *data_ptr;
			data_ptr ++;
		} else {
			count = -count;
			memcpy(out_ptr, data_ptr, count * sizeof(unsigned));

			out_ptr += count;
			data_ptr += count;
		}
	}

	return true;
}

bool rleBuffer::decode_pixel(int x, int y, unsigned &pixel) {
	const char *header_ptr = &*(_header.begin() + _header_offset[y]);
	const unsigned *data_ptr = &*(_data.begin() + _data_offset[y]);

	int xx = 0;
	char count = *header_ptr++;

	while (xx + abs(count) < x) {
		if (count > 0) {
			data_ptr ++;
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

unsigned rleBuffer::size() {
	return _data.size() * sizeof(unsigned) + _data_offset.size() + _header_offset.size() * sizeof(unsigned) + _header.size();
}

bool rleBuffer::convert_data(int bits_per_pixel) {
	if (_bits_per_pixel == bits_per_pixel)
		return true;

	int sz = _data.size();

	switch (_bits_per_pixel) {
	case 15:
	case 16:
		if (bits_per_pixel == 24 || bits_per_pixel == 32) {
			unsigned short *short_ptr = reinterpret_cast<unsigned short *>(&*_data.begin());

			for (int i = 0; i < sz; i ++) {
				short_ptr++;
				*short_ptr++ <<= 8;
			}

			short_ptr = reinterpret_cast<unsigned short *>(&*_data.begin());
			unsigned char *char_ptr = reinterpret_cast<unsigned char *>(&*_data.begin());

			for (int i = 0; i < sz; i ++) {
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
			unsigned short *short_ptr = reinterpret_cast<unsigned short *>(&*_data.begin());

			for (int i = 0; i < sz; i ++) {
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
			unsigned char *src_ptr = reinterpret_cast<unsigned char *>(&*_data.begin());
			unsigned short *dest_ptr = reinterpret_cast<unsigned short *>(&*_data.begin());

			for (int i = 0; i < sz; i ++) {
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

void rleBuffer::resize_buffers() {
	unsigned len = line_length() * sizeof(unsigned);

	if (_buffer0.size() < len)
		_buffer0.resize(len);
	if (_buffer1.size() < len)
		_buffer1.resize(len);
}

int rleBuffer::line_length() {
	if (_header_offset.empty()) return 0;

	int sz = (_header_offset.size() > 1) ? _header_offset[1] : _header.size();

	int len = 0;
	for (int i = 0; i < sz; i ++) {
		len += abs(_header[i]);
	}

	return len;
}

int rleBuffer::line_header_length(int line_num) const {
	if (line_num < _header_offset.size() - 1)
		return _header_offset[line_num + 1] - _header_offset[line_num];
	else
		return _header.size() - _header_offset[line_num];
}


bool rleBuffer::load(Common::SeekableReadStream *fh) {
	int32 sz = fh->readUint32LE();
	_header_offset.resize(sz);

	sz = fh->readSint32LE();
	_data_offset.resize(sz);

	sz = fh->readSint32LE();
	_header.resize(sz + 1);
	_header[sz] = 0;

	sz = fh->readSint32LE();
	_data.resize(sz);

	for (int i = 0; i < _header_offset.size(); i++) {
		_header_offset[i] = fh->readUint32LE();
	}

	for (int i = 0; i < _data_offset.size(); i++) {
		_data_offset[i] = fh->readUint32LE();
	}

	for (int i = 0; i < _header.size() - 1; i++) {
		_header[i] = fh->readByte();
	}

	for (int i = 0; i < _data.size(); i++) {
		_data[i] = fh->readUint32LE();
	}

	resize_buffers();

	return true;
}

} // namespace QDEngine
