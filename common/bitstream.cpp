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

#include "common/bitstream.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Common {

BitStream::BitStream() {
}

BitStream::~BitStream() {
}

void BitStream::skip(uint32 n) {
	while (n-- > 0)
		getBit();
}


BitStreamBE::BitStreamBE(SeekableReadStream &stream, uint32 bitCount) : _value(0), _inValue(0) {
	if ((bitCount % 8) != 0)
		error("Big-endian bit stream size has to be divisible by 8");

	// Read the number of bytes of the stream

	uint32 byteSize = bitCount / 8;
	byte *data = (byte *)malloc(byteSize);

	if (stream.read(data, byteSize) != byteSize) {
		free(data);
		error("Bad BitStreamBE size");
	}

	_stream = new MemoryReadStream(data, byteSize, DisposeAfterUse::YES);
}

BitStreamBE::BitStreamBE(const byte *data, uint32 bitCount) : _value(0), _inValue(0) {
	if ((bitCount % 8) != 0)
		error("Big-endian bit stream size has to be divisible by 8");

	// Copy the number of bytes from the data array

	uint32 byteSize = bitCount / 8;
	byte *dataN = (byte *)malloc(byteSize);

	memcpy(dataN, data, byteSize);

	_stream = new MemoryReadStream(dataN, byteSize, DisposeAfterUse::YES);
}

BitStreamBE::~BitStreamBE() {
	delete _stream;
}

uint32 BitStreamBE::getBit() {
	if (_inValue == 0) {
		// Need to get new byte

		if (_stream->eos())
			error("End of bit stream reached");

		_value = _stream->readByte();
	}

	// Get the current bit
	int b = ((_value & 0x80) == 0) ? 0 : 1;

	// Shift to the next bit
	_value <<= 1;

	// Increase the position within the current byte
	_inValue = (_inValue + 1) % 8;

	return b;
}

uint32 BitStreamBE::getBits(uint32 n) {
	if (n > 32)
		error("Too many bits requested to be read");

	// Read the number of bits
	uint32 v = 0;
	while (n-- > 0)
		v = (v << 1) | getBit();

	return v;
}

void BitStreamBE::addBit(uint32 &x, uint32 n) {
	x = (x << 1) | getBit();
}

uint32 BitStreamBE::pos() const {
	if (_stream->pos() == 0)
		return 0;

	uint32 p = (_inValue == 0) ? _stream->pos() : (_stream->pos() - 1);
	return p * 8 + _inValue;
}

uint32 BitStreamBE::size() const {
	return _stream->size() * 8;
}


BitStream32LE::BitStream32LE(SeekableReadStream &stream, uint32 bitCount) : _value(0), _inValue(0) {
	if ((bitCount % 32) != 0)
		error("32bit little-endian bit stream size has to be divisible by 32");

	// Read the number of bytes of the stream

	uint32 byteSize = bitCount / 8;
	byte *data = (byte *)malloc(byteSize);

	if (stream.read(data, byteSize) != byteSize) {
		free(data);
		error("Bad BitStream32LE size");
	}

	_stream = new MemoryReadStream(data, byteSize, DisposeAfterUse::YES);
}

BitStream32LE::BitStream32LE(const byte *data, uint32 bitCount) : _value(0), _inValue(0) {
	if ((bitCount % 32) != 0)
		error("32bit little-endian bit stream size has to be divisible by 32");

	// Copy the number of bytes from the data array

	uint32 byteSize = bitCount / 8;
	byte *dataN = (byte *)malloc(byteSize);

	memcpy(dataN, data, byteSize);

	_stream = new MemoryReadStream(dataN, byteSize, DisposeAfterUse::YES);
}

BitStream32LE::~BitStream32LE() {
	delete _stream;
}

uint32 BitStream32LE::getBit() {
	if (_inValue == 0) {
		// Need to get new 32bit value

		if (_stream->eos())
			error("End of bit stream reached");

		_value = _stream->readUint32LE();
	}

	// Get the current bit
	int b = ((_value & 1) == 0) ? 0 : 1;

	// Shift to the next bit
	_value >>= 1;

	// Increase the position within the current byte
	_inValue = (_inValue + 1) % 32;

	return b;
}

uint32 BitStream32LE::getBits(uint32 n) {
	if (n > 32)
		error("Too many bits requested to be read");

	// Read the number of bits
	uint32 v = 0;
	for (uint32 i = 0; i < n; i++)
		v = (v >> 1) | (((uint32) getBit()) << 31);

	v >>= (32 - n);
	return v;
}

void BitStream32LE::addBit(uint32 &x, uint32 n) {
	x = (x & ~(1 << n)) | (getBit() << n);
}

uint32 BitStream32LE::pos() const {
	if (_stream->pos() == 0)
		return 0;

	uint32 p = ((_inValue == 0) ? _stream->pos() : (_stream->pos() - 1)) & 0xFFFFFFFC;
	return p * 8 + _inValue;
}

uint32 BitStream32LE::size() const {
	return _stream->size() * 8;
}

} // End of namespace Common
