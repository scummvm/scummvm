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

#include "glk/comprehend/file_buf.h"
#include "common/algorithm.h"
#include "common/file.h"

namespace Glk {
namespace Comprehend {

FileBuffer::FileBuffer(const Common::String &filename) : _pos(0) {
	// Open the file
	Common::File f;
	if (!f.open(filename))
		error("Could not open - %s", filename.c_str());

	_data.resize(f.size());
	_readBytes.resize(f.size());
	f.read(&_data[0], f.size());
}

FileBuffer::FileBuffer(Common::ReadStream *stream, size_t size) : _pos(0) {
	_data.resize(size);
	_readBytes.resize(size);
	stream->read(&_data[0], size);
}


bool FileBuffer::exists(const Common::String &filename) {
	return Common::File::exists(filename);
}

void FileBuffer::close() {
	_data.clear();
	_readBytes.clear();
	_pos = 0;
}

bool FileBuffer::seek(int32 offset, int whence) {
	switch (whence) {
	case SEEK_SET:
		_pos = offset;
		break;
	case SEEK_CUR:
		_pos += offset;
		break;
	case SEEK_END:
		_pos = (int)_data.size() + offset;
		break;
	default:
		break;
	}

	return true;
}

uint32 FileBuffer::read(void *dataPtr, uint32 dataSize) {
	int32 bytesRead = CLIP((int32)dataSize, (int32)0, (int32)_data.size() - _pos);
	if (bytesRead) {
		Common::fill(&_readBytes[_pos], &_readBytes[_pos] + bytesRead, true);
		Common::copy(&_data[_pos], &_data[_pos] + bytesRead, (byte *)dataPtr);
		_pos += bytesRead;
	}

	return bytesRead;
}

size_t FileBuffer::strlen(bool *eof) {
	uint8 *end;

	if (eof)
		*eof = false;

	end = (uint8 *)memchr(&_data[_pos], '\0', size() - _pos);
	if (!end) {
		// No null terminator - string is remaining length
		if (eof)
			*eof = true;
		return size() - _pos;
	}

	return end - &_data[_pos];
}

void FileBuffer::showUnmarked() {
	int i, start = -1;

	for (i = 0; i < (int)_data.size(); i++) {
		if (!_readBytes[i] && start == -1)
			start = i;

		if ((_readBytes[i] || i == (int)_data.size() - 1) && start != -1) {
			warning("%.4x - %.4x unmarked (%d bytes)\n",
			        start, i - 1, i - start);
			start = -1;
		}
	}
}

} // namespace Comprehend
} // namespace Glk
