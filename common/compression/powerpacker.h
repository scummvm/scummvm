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

#ifndef COMMON_POWERPACKER_H
#define COMMON_POWERPACKER_H

#include "common/stream.h"

namespace Common {
class PowerPackerStream : public Common::SeekableReadStream {

	SeekableReadStream *_stream;
	bool				_dispose;

private:
	static int ppDecrunchBuffer(const byte *src, byte *dest, uint32 src_len, uint32 dest_len);
	static uint16 getCrunchType(uint32 signature);

public:
	PowerPackerStream(Common::SeekableReadStream &stream);

	static byte *unpackBuffer(const byte *input, uint32 input_len, uint32 &output_len);

	~PowerPackerStream() override {
		if (_dispose) delete _stream;
	}

	int64 size() const override {
		return _stream->size();
	}

	int64 pos() const override {
		return _stream->pos();
	}

	bool eos() const override {
		return _stream->eos();
	}

	bool seek(int64 offs, int whence = SEEK_SET) override {
		return _stream->seek(offs, whence);
	}

	uint32 read(void *dataPtr, uint32 dataSize) override {
		return _stream->read(dataPtr, dataSize);
	}

};
} // End of namespace Common

#endif
