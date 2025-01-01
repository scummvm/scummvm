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

#include "common/file.h"

#ifndef MEDIASTATION_CHUNK_H
#define MEDIASTATION_CHUNK_H

namespace MediaStation {

class Chunk : public Common::SeekableReadStream {
private:
	Common::SeekableReadStream *_input = nullptr;
	uint32 _dataStartOffset = 0;
	uint32 _dataEndOffset = 0;

public:
	uint32 id = 0;
	uint32 length = 0;

	Chunk() = default;
	Chunk(Common::SeekableReadStream *stream);

	uint32 bytesRemaining() {
		return _dataEndOffset - pos();
	}

	// ReadStream implementation
	bool eos() const {
		return _input->eos();
	}
	bool err() const {
		return _input->err();
	}
	void clearErr() {
		_input->clearErr();
	}
	uint32 read(void *dataPtr, uint32 dataSize) {
		if (pos() > _dataEndOffset) {
			uint overrun = pos() - _dataEndOffset;
			error("Attempted to read 0x%x bytes at a location 0x%x bytes past end of chunk (@0x%llx)", dataSize, overrun, static_cast<long long int>(pos()));
		} else {
			return _input->read(dataPtr, dataSize);
		}
	}
	int64 pos() const {
		return _input->pos();
	}
	int64 size() const {
		return _input->size();
	}
	bool seek(int64 offset, int whence = SEEK_SET) {
		// TODO: This is a bad hack and should be cleaned up!
		bool result = _input->seek(offset, whence);
		if (result == false) {
			return false;
		}

		if (pos() < _dataStartOffset) {
			uint overrun = _dataStartOffset - offset;
			error("Attempted to seek 0x%x bytes before start of chunk (@0x%llx)", overrun, static_cast<long long int>(pos()));
		} else if (pos() > _dataEndOffset) {
			uint overrun = offset - _dataEndOffset;
			error("Attempted to seek 0x%x bytes past end of chunk (@0x%llx)", overrun, static_cast<long long int>(pos()));
		}
		return true;
	}
	bool skip(uint32 offset) {
		return seek(offset, SEEK_CUR);
	}
};

} // End of namespace MediaStation

#endif
