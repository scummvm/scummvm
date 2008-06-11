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

#ifndef CINE_UNPACK_H
#define CINE_UNPACK_H


#include "common/scummsys.h"

namespace Cine {

/**
 * A LZ77 style decompressor for Delphine's data files
 * used in at least Future Wars and Operation Stealth.
 * @note Works backwards in the source and destination buffers.
 * @note Can work with source and destination in the same buffer if there's space.
 */
class CineUnpacker {
public:
	/** Returns true if unpacking was successful, otherwise false. */
	bool unpack(byte *dst, const byte *src, int srcLen);
private:
	/** Reads a single big endian 32-bit integer from the source and goes backwards 4 bytes. */
	uint32 readSource();

	/**
	 * Shifts the current internal 32-bit chunk to the right by one.
	 * Puts input carry into internal chunk's topmost (i.e. leftmost) bit.
	 * Returns the least significant bit that was shifted out.
	 */
	int rcr(int inputCarry);
	int nextBit();
	uint16 getBits(byte numBits);

	/**
	 * Copy raw bytes from the input stream and write them to the destination stream.
	 * This is used when no adequately long match is found in the sliding window.
	 * @param numBytes Amount of bytes to copy from the input stream
	 */
	void unpackRawBytes(uint16 numBytes);

	/**
	 * Copy bytes from the sliding window in the destination buffer.
	 * This is used when a match of two bytes or longer is found.
	 * @param offset Offset in the sliding window
	 * @param numBytes Amount of bytes to copy
	 */
	void copyRelocatedBytes(uint16 offset, uint16 numBytes);
private:
	int _datasize;    //!< Bytes left to write into the unpacked data stream
	uint32 _crc;      //!< Error-detecting code
	uint32 _chunk32b; //!< The current internal 32-bit chunk
	byte *_dst;       //!< Destination buffer pointer
	const byte *_src; //!< Source buffer pointer
};

} // End of namespace Cine

#endif
