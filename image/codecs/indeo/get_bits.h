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

#include "common/scummsys.h"

/* Intel Indeo 4 bitstream reader
 *
 * Original copyright note:
 * Copyright (c) 2004 Michael Niedermayer
 */

#ifndef IMAGE_CODECS_INDEO_GET_BITS_H
#define IMAGE_CODECS_INDEO_GET_BITS_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/types.h"

namespace Image {
namespace Indeo {

#define AV_INPUT_BUFFER_PADDING_SIZE 32

/**
 * Intel Indeo Bitstream reader
 */
class GetBits {
private:
	const byte *_buffer;
	uint _index;
	uint _sizeInBits;
	uint _sizeInBitsPlus8;
public:
	/**
	* Constructor
	* @param buffer		Bitstream buffer, must be AV_INPUT_BUFFER_PADDING_SIZE bytes
	*        larger than the actual read bits because some optimized bitstream
	*        readers read 32 or 64 bit at once and could read over the end
	* @param bit_size the size of the buffer in bits
	* @return 0 on success, AVERROR_INVALIDDATA if the buffer_size would overflow.
	*/
	GetBits(const byte *buffer, size_t totalBits);

	/**
	 * Copy constructor
	 */
	GetBits(const GetBits &src);

	/**
	 * Returns the number of bits read
	 */
	uint getBitsCount() const { return _index; }

	/**
	 * The number of bits left
	 */
	int getBitsLeft() const { return _sizeInBits - _index; }

	void skipBitsLong(uint n) { _index += n; }

	/**
	 * Read MPEG-1 dc-style VLC (sign bit + mantissa with no MSB).
	 * if MSB not set it is negative
	 * @param n length in bits
	 */
	int getXbits(int n);

	/**
	 * Returns the next n bits, and does sign extension
	 */
	int getSbits(int n);

	/**
	 * Read 1-25 bits.
	 */
	uint getBits(int n);

	/**
	 * Read 0-25 bits.
	 */
	int getBitsZ(int n);

	uint getBitsLE(int n);

	/**
	 * Show 1-25 bits.
	 * Returns the data without updating the index
	 */
	uint showBits(int n);

	/**
	 * Skips a specified number of bits
	 */
	void skipBits(int n);

	/**
	 * Returns the next bit
	 */
	uint getBits1();

	/**
	 * Shows the next following bit
	 */
	uint showBits1();

	/**
	 * Skips the next bit
	 */
	void skipBits1();

	/**
	 * Read 0-32 bits.
	 */
	uint getBitsLong(int n);

	/**
	 * Read 0-64 bits.
	 */
	uint64 getBits64(int n);

	/**
	 * Read 0-32 bits as a signed integer.
	 */
	int getSbitsLong(int n);

	/**
	 * Show 0-32 bits.
	 */
	uint showBitsLong(int n);

	int checkMarker(void *logctx, const char *msg);

	/**
	 * Parse a VLC code.
	 * @param bits is the number of bits which will be read at once, must be
	 *             identical to nb_bits in init_vlc()
	 * @param max_depth is the number of times bits bits must be read to completely
	 *                  read the longest vlc code
	 *                  = (max_vlc_length + bits - 1) / bits
	 */
	int getVLC2(int16 (*table)[2], int bits, int maxDepth);

	int decode012();

	int decode210();

	int skip1stop8dataBits();

	const byte *alignGetBits();
};

} // End of namespace Indeo
} // End of namespace Image

#endif
