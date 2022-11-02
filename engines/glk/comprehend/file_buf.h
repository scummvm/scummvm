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

#ifndef GLK_COMPREHEND_FILE_BUF_H
#define GLK_COMPREHEND_FILE_BUF_H

#include "common/array.h"
#include "common/memstream.h"
#include "common/stream.h"

namespace Glk {
namespace Comprehend {

struct FileBuffer : public Common::SeekableReadStream {
private:
	Common::Array<byte> _data;
	Common::Array<bool> _readBytes;
	int32 _pos;

public:
	FileBuffer() : _pos(0) {}
	FileBuffer(const Common::String &filename);
	FileBuffer(Common::ReadStream *stream, size_t size);
	static bool exists(const Common::String &filename);
	void close();

	int64 pos() const override {
		return _pos;
	}
	int64 size() const override {
		return _data.size();
	}
	bool seek(int64 offset, int whence = SEEK_SET) override;

	bool eos() const override {
		return _pos >= (int)_data.size();
	}
	uint32 read(void *dataPtr, uint32 dataSize) override;

	const byte *dataPtr() const {
		return &_data[_pos];
	}
	size_t strlen(bool *eof = nullptr);

	/*
	 * Debugging function to show regions of a file that have not been read.
	 */
	void showUnmarked();
};

#define file_buf_get_array(fb, type, base, array, member, size) \
	do {                                                        \
		uint __i;                                               \
		for (__i = (base); __i < (base) + (size); __i++)        \
			(array)[__i].member = fb->read##type();    \
	} while (0)

#define file_buf_get_array_u8(fb, base, array, member, size) \
	file_buf_get_array(fb, Byte, base, array, member, size)

#define file_buf_get_array_le16(fb, base, array, member, size) \
	file_buf_get_array(fb, Uint16LE, base, array, member, size)


} // namespace Comprehend
} // namespace Glk

#endif
