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

#ifndef QDENGINE_SYSTEM_GRAPHICS_RLE_COMPRESS_H
#define QDENGINE_SYSTEM_GRAPHICS_RLE_COMPRESS_H


namespace QDEngine {

//! Массив, сжатый методом RLE.
class RLEBuffer {
public:
	RLEBuffer();
	RLEBuffer(const RLEBuffer &buf);
	~RLEBuffer();

	RLEBuffer &operator = (const RLEBuffer &buf);

	bool encode(int sx, int sy, const byte *buf);

	bool decode_line(int y, byte *out_buf) const;

	bool decode_line(int y, int buffer_id = 0) const;

	bool decode_pixel(int x, int y, uint32 &pixel);

	static const byte *get_buffer(int buffer_id);

	void resize_buffers();

	uint32 size();
	int line_length();
	int line_header_length(int line_num) const;

	uint32 header_size() const {
		return _header.size();
	}
	uint32 data_size() const {
		return _data.size();
	}

	const char *header_ptr(int y = 0) const {
		return &*(_header.begin() + _header_offset[y]);
	}
	const uint32 *data_ptr(int y = 0) const {
		return &*(_data.begin() + _data_offset[y]);
	}

	bool load(Common::SeekableReadStream *fh);

	bool convert_data(int bits_per_pixel = 16);

	static void releaseBuffers();

private:
	Std::vector<uint32> _header_offset;
	Std::vector<uint32> _data_offset;

	Std::vector<char> _header;
	Std::vector<uint32> _data;

	int _bits_per_pixel;

	friend bool operator == (const RLEBuffer &buf1, const RLEBuffer &buf2);
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_RLE_COMPRESS_H
