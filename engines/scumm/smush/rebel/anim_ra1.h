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

#ifndef SCUMM_SMUSH_REBEL_ANIM_RA1_H
#define SCUMM_SMUSH_REBEL_ANIM_RA1_H

#include "common/endian.h"
#include "common/stream.h"

namespace Scumm {

struct RA1AnimChunk {
	uint32 tag;
	uint32 size;
	int64 offset;
	int64 dataOffset;
	int64 endOffset;
	int64 nextOffset;
	const byte *data;
};

class RA1AnimChunkIterator {
public:
	RA1AnimChunkIterator(const byte *data, uint32 size)
		: _data(data), _offset(0), _endOffset(size) {}

	RA1AnimChunkIterator(const byte *data, uint32 startOffset, uint32 endOffset)
		: _data(data), _offset(startOffset), _endOffset(endOffset) {
		if (_offset > _endOffset)
			_offset = _endOffset;
	}

	bool next(RA1AnimChunk &chunk) {
		if (_data == nullptr || _offset + 8 > _endOffset)
			return false;

		const uint32 size = READ_BE_UINT32(_data + _offset + 4);
		const uint32 dataOffset = _offset + 8;
		if (size > _endOffset - dataOffset) {
			_offset = _endOffset;
			return false;
		}

		chunk.tag = READ_BE_UINT32(_data + _offset);
		chunk.size = size;
		chunk.offset = _offset;
		chunk.dataOffset = dataOffset;
		chunk.endOffset = dataOffset + size;
		chunk.nextOffset = chunk.endOffset + (size & 1);
		if (chunk.nextOffset > _endOffset)
			chunk.nextOffset = _endOffset;
		chunk.data = _data + dataOffset;

		_offset = (uint32)chunk.nextOffset;
		return true;
	}

private:
	const byte *_data;
	uint32 _offset;
	uint32 _endOffset;
};

class RA1AnimStreamChunkIterator {
public:
	RA1AnimStreamChunkIterator(Common::SeekableReadStream &stream, int64 endOffset)
		: _stream(stream), _endOffset(endOffset) {}

	bool next(RA1AnimChunk &chunk) {
		if (_stream.pos() + 8 > _endOffset)
			return false;

		const int64 chunkOffset = _stream.pos();
		const uint32 tag = _stream.readUint32BE();
		const uint32 size = _stream.readUint32BE();
		const int64 dataOffset = _stream.pos();
		const int64 endOffset = dataOffset + size;
		if (endOffset < dataOffset || endOffset > _endOffset)
			return false;

		chunk.tag = tag;
		chunk.size = size;
		chunk.offset = chunkOffset;
		chunk.dataOffset = dataOffset;
		chunk.endOffset = endOffset;
		chunk.nextOffset = endOffset + (size & 1);
		if (chunk.nextOffset > _endOffset)
			chunk.nextOffset = _endOffset;
		chunk.data = nullptr;
		return true;
	}

	void skip(const RA1AnimChunk &chunk) {
		_stream.seek(chunk.nextOffset, SEEK_SET);
	}

private:
	Common::SeekableReadStream &_stream;
	int64 _endOffset;
};

class RA1FrameChunkIterator {
public:
	RA1FrameChunkIterator(Common::SeekableReadStream &stream, int32 &remaining)
		: _stream(stream), _remaining(remaining) {}

	bool next(RA1AnimChunk &chunk) {
		while (_remaining > 1) {
			if ((_stream.pos() & 1) && _remaining > 0) {
				const byte pad = _stream.readByte();
				if (pad == 0) {
					_remaining--;
				} else {
					_stream.seek(-1, SEEK_CUR);
				}
			}

			if (_remaining < 8) {
				_stream.skip(_remaining);
				_remaining = 0;
				return false;
			}

			const int64 chunkOffset = _stream.pos();
			chunk.tag = _stream.readUint32BE();
			chunk.size = _stream.readUint32BE();
			chunk.offset = chunkOffset;
			chunk.dataOffset = _stream.pos();
			chunk.endOffset = chunk.dataOffset + chunk.size;
			chunk.nextOffset = chunk.endOffset;
			chunk.data = nullptr;
			return true;
		}

		if (_remaining == 1) {
			_stream.skip(1);
			_remaining = 0;
		}
		return false;
	}

	bool fits(const RA1AnimChunk &chunk) const {
		return _remaining >= 8 && chunk.size <= (uint32)(_remaining - 8);
	}

	void skip(const RA1AnimChunk &chunk) {
		const int32 consumed = fits(chunk) ? (int32)chunk.size + 8 : _remaining;
		_remaining -= consumed;
		_stream.seek(chunk.endOffset, SEEK_SET);
	}

private:
	Common::SeekableReadStream &_stream;
	int32 &_remaining;
};

} // End of namespace Scumm

#endif
