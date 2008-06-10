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

class CineUnpacker {
public:
	/** Returns true if unpacking was successful, otherwise false. */
	bool unpack(byte *dst, const byte *src, int srcLen);
private:
	/** Reads a single big endian 32-bit integer from the source and goes backwards 4 bytes. */
	uint32 readSource();
	int rcr(int CF);
	int nextBit();
	uint16 getBits(byte numBits);
	void unpackBytes(uint16 numBytes);
	void copyRelocatedBytes(uint16 offset, uint16 numBytes);
private:
	int _datasize;
	uint32 _crc;
	uint32 _chk;
	byte *_dst;
	const byte *_src;
};

} // End of namespace Cine

#endif
