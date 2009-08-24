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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "sci/sci.h"
#include "sci/exereader.h"

namespace Sci {

int _bitCount;
uint16 _bits;

Common::Platform getGameExePlatform(Common::SeekableReadStream *exeStream) {
	byte magic[4];
	// Make sure that the executable is at least 4KB big
	if (exeStream->size() < 4096)
		return Common::kPlatformUnknown;

	// Read exe header
	exeStream->read(magic, 4);

	// Check if the header contains known magic bytes

	// Information obtained from http://magicdb.org/magic.db
	// Check if it's a DOS executable
	if (magic[0] == 'M' && magic[1] == 'Z') {
		return Common::kPlatformPC;
	}

	// Check if it's an Amiga executable
	if ((magic[2] == 0x03 && magic[3] == 0xF3) ||
		(magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')) {
		return Common::kPlatformAmiga;
	}

	// Check if it's an Atari executable
	if ((magic[0] == 0x60 && magic[1] == 0x1A))
		return Common::kPlatformAtariST;

	// Check if it's a Mac exe

	// Resource map offset
	int32 offset = (int32)READ_BE_UINT32(magic);
	offset += 28;
	if (exeStream->size() <= offset)
		return Common::kPlatformUnknown;

	// Skip number of types in map
	exeStream->skip(2);
//	uint16 val = exeStream->readUint16BE() + 1;
	exeStream->skip(2);

	// Keep reading till we find the "CODE" bit
	while (!exeStream->eos()) {
		exeStream->skip(4);
		if (exeStream->eos())
			return Common::kPlatformUnknown;

		exeStream->read(magic, 4);
		if (exeStream->eos())
			return Common::kPlatformUnknown;

		if (!memcmp(magic, "CODE", 4)) {
			return Common::kPlatformMacintosh;
		}
		// Skip to the next list entry
		exeStream->skip(4);
		if (exeStream->eos())
			return Common::kPlatformUnknown;
	}

	// If we've reached here, the file type is unknown
	return Common::kPlatformUnknown;
}

uint getBit(Common::SeekableReadStream *input) {
	uint bit = _bits & 1;
	_bitCount--;

	if (_bitCount <= 0) {
		_bits = input->readByte();
		_bits |= input->readByte() << 8;

		if (_bitCount == -1) { // special case for first bit word
			bit = _bits & 1;
			_bits >>= 1;
		}

		_bitCount += 16;
	} else
		_bits >>= 1;

	return bit;
}

} // End of namespace Sci
