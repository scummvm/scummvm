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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_DECOMPRESS_H
#define NANCY_DECOMPRESS_H

#include "common/scummsys.h"

namespace Common {
class ReadStream;
class WriteStream;
class MemoryWriteStream;
}

namespace Nancy {

class Decompressor {
public:
	Decompressor();
	~Decompressor();

	// Decompresses data from input until the end of the stream
	// The output stream must have the right size for the decompressed data
	bool decompress(Common::SeekableReadStream &input, Common::MemoryWriteStream &output);

private:
	enum {
		kBufSize = 4096,
		kBufStart = 4078
	};

	void init(Common::SeekableReadStream &input, Common::WriteStream &output);
	bool readByte(byte &b);
	bool writeByte(byte b);

	byte _buf[kBufSize];
	uint _bufpos;
	bool _err;
	byte _val;
	Common::WriteStream *_output;

	byte *_input;
	byte *_pos;
	byte *_end;
};

} // End of namespace Nancy

#endif // NANCY_DECOMPRESS_H
