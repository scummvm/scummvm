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
class rleBuffer {
public:
	rleBuffer();
	rleBuffer(const rleBuffer &buf);
	~rleBuffer();

	rleBuffer &operator = (const rleBuffer &buf);

	bool encode(int sx, int sy, const unsigned char *buf);

	bool decode_line(int y, unsigned char *out_buf) const;

	inline bool decode_line(int y, int buffer_id = 0) const {
		if (buffer_id)
			return decode_line(y, &*buffer1_.begin());
		else
			return decode_line(y, &*buffer0_.begin());
	}

	bool decode_pixel(int x, int y, unsigned &pixel);

	static inline const unsigned char *get_buffer(int buffer_id) {
		if (buffer_id) return &*buffer1_.begin();
		else return &*buffer0_.begin();
	}

	void resize_buffers();

	unsigned size();
	int line_length();
	int line_header_length(int line_num) const;

	unsigned header_size() const {
		return header_.size();
	}
	unsigned data_size() const {
		return data_.size();
	}

	const char *header_ptr(int y = 0) const {
		return &*(header_.begin() + header_offset_[y]);
	}
	const unsigned *data_ptr(int y = 0) const {
		return &*(data_.begin() + data_offset_[y]);
	}

	bool save(class XStream &fh);
	bool load(class XStream &fh);
	bool load(class XZipStream &fh);
	bool load(Common::SeekableReadStream *fh);

	bool convert_data(int bits_per_pixel = 16);

private:
	std::vector<unsigned> header_offset_;
	std::vector<unsigned> data_offset_;

	std::vector<char> header_;
	std::vector<unsigned> data_;

	int bits_per_pixel_;

	static std::vector<unsigned char> buffer0_;
	static std::vector<unsigned char> buffer1_;

	friend bool operator == (const rleBuffer &buf1, const rleBuffer &buf2);
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_RLE_COMPRESS_H
