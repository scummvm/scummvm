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

#include "common/memstream.h"
#include "common/textconsole.h"

#include "engines/nancy/decompress.h"

namespace Nancy {

void Decompressor::init(Common::ReadStream &input, Common::WriteStream &output) {
	memset(_buf, ' ', kBufSize);
	_bufpos = kBufStart;
	_err = false;
	_val = 0;
	_input = &input;
	_output = &output;
}

bool Decompressor::readByte(byte &b) {
	b = _input->readByte();

	if (_input->eos())
		return false;

	if (_input->err())
		error("Read error encountered during decompression");

	b -= _val++;
	return true;
}

bool Decompressor::writeByte(byte b) {
	_output->writeByte(b);
	_buf[_bufpos++] = b;
	_bufpos &= kBufSize - 1;
	return true;
}

bool Decompressor::decompress(Common::ReadStream &input, Common::MemoryWriteStream &output) {
	init(input, output);
	uint16 bits = 0;

	while (1) {
		byte b;

		bits >>= 1;

		// The highest 8 bits are used to keep track of how many bits are left to process
		if (!(bits & 0x100)) {
			// Out of bits
			if (!readByte(b))
				break;
			bits = 0xff00 | b;
		}

		if (bits & 1) {
			// Literal byte
			if (!readByte(b))
				break;
			writeByte(b);
		} else {
			// Copy from buffer
			byte b2;
			if (!readByte(b) || !readByte(b2))
				break;

			uint16 offset = b | ((b2 & 0xf0) << 4);
			uint16 len = (b2 & 0xf) + 3;

			for (uint i = 0; i < len; i++)
				writeByte(_buf[(offset + i) & (kBufSize - 1)]);
		}
	}

	if (output.err() || output.pos() != output.size()) {
		warning("Failed to decompress resource");
		return false;
	}

	return true;
}

} // End of namespace Nancy
