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
 */

// Based on eos' BitStream implementation

#ifndef COMMON_BITSTREAM_H
#define COMMON_BITSTREAM_H

#include "common/scummsys.h"

namespace Common {

class SeekableReadStream;

/**
 * A bit stream, giving access to data one bit at a time.
 *
 * Used in engines:
 *  - scumm
 */
class BitStream {
public:
	BitStream();
	virtual ~BitStream();

	/** Read a bit from the bitstream. */
	virtual uint32 getBit() = 0;

	/** Read a number of bits, creating a multi-bit value. */
	virtual uint32 getBits(uint32 n) = 0;

	/** Add more bits, creating a multi-bit value in stages. */
	virtual void addBit(uint32 &x, uint32 n) = 0;

	/** Skip a number of bits. */
	void skip(uint32 n);

	/** Get the current position, in bits. */
	virtual uint32 pos()  const = 0;
	/** Return the number of bits in the stream. */
	virtual uint32 size() const = 0;
};

/**
 * A big-endian bit stream.
 *
 * The input data is read one byte at a time. Their bits are handed out
 * in the order of MSB to LSB. When all 8 bits of a byte have been consumed,
 * another input data byte is read.
 */
class BitStreamBE : public BitStream {
public:
	/**
	 * Create a big endian bit stream.
	 *
	 * Reads and copies bitCount bits from the provided stream.
	 * Ownership of the stream is not transferred.
	 */
	BitStreamBE(SeekableReadStream &stream, uint32 bitCount);

	/**
	 * Create a big endian bit stream.
	 *
	 * Reads and copies bitCount bits from the provided data.
	 * Ownership of the data is not transferred.
	 */
	BitStreamBE(const byte *data, uint32 bitCount);

	~BitStreamBE();

	uint32 getBit();

	/**
	 * Read a number of bits, creating a multi-bit value.
	 *
	 * The bits are read one at a time, in the order MSB to LSB and
	 * or'd together to create a multi-bit value.
	 */
	uint32 getBits(uint32 n);

	/**
	 * Add more bits, creating a multi-bit value in stages.
	 *
	 * Shifts in n new bits into the value x, in the order of MSB to LSB.
	 */
	void addBit(uint32 &x, uint32 n);

	uint32 pos()  const;
	uint32 size() const;

private:
	SeekableReadStream *_stream;

	byte  _value;   ///< Current byte.
	uint8 _inValue; ///< Position within the current byte.
};

/**
 * A little-endian bit stream, reading 32bit values at a time.
 *
 * The input data is read one little-endian uint32 at a time. Their bits are
 * handed out in the order of LSB to MSB. When all 8 bits of a byte have been
 * consumed, another little-endian input data uint32 is read.
 */
class BitStream32LE : public BitStream {
public:
	/**
	 * Create a little-endian bit stream.
	 *
	 * Reads and copies bitCount bits from the provided stream.
	 * Ownership of the stream is not transferred.
	 */
	BitStream32LE(SeekableReadStream &stream, uint32 bitCount);

	/**
	 * Create a little-endian bit stream.
	 *
	 * Reads and copies bitCount bits from the provided data.
	 * Ownership of the data is not transferred.
	 */
	BitStream32LE(const byte *data, uint32 bitCount);

	~BitStream32LE();

	uint32 getBit();

	/**
	 * Read a number of bits, creating a multi-bit value.
	 *
	 * The bits are read one at a time, in the order LSB to MSB and
	 * or'd together to create a multi-bit value.
	 */
	uint32 getBits(uint32 n);

	/**
	 * Add more bits, creating a multi-bit value in stages.
	 *
	 * Shifts in n new bits into the value x, in the order of LSB to MSB.
	 */
	void addBit(uint32 &x, uint32 n);

	uint32 pos()  const;
	uint32 size() const;

private:
	SeekableReadStream *_stream;

	uint32 _value;   ///< Current 32bit value.
	uint8  _inValue; ///< Position within the current 32bit value.
};

} // End of namespace Common

#endif // COMMON_BITSTREAM_H
