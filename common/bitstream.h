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

// Based on eos' BitStream implementation

#ifndef COMMON_BITSTREAM_H
#define COMMON_BITSTREAM_H

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "common/stream.h"
#include "common/types.h"
#include "common/util.h"

namespace Common {

/**
 * A template implementing a bit stream for different data memory layouts.
 *
 * Such a bit stream reads valueBits-wide values from the data stream and
 * gives access to their bits, one at a time.
 *
 * For example, a bit stream with the layout parameters 32, true, false
 * for valueBits, isLE and isMSB2LSB, reads 32bit little-endian values
 * from the data stream and hands out the bits in the order of LSB to MSB.
 */
template<class STREAM, int valueBits, bool isLE, bool isMSB2LSB>
class BitStreamImpl {
private:
	STREAM *_stream;			///< The input stream.
	DisposeAfterUse::Flag _disposeAfterUse; ///< Should we delete the stream on destruction?

	uint32 _value;   ///< Current value.
	uint8  _inValue; ///< Position within the current value.
	uint32 _size;    ///< Total bitstream size (in bits)
	uint32 _pos;     ///< Current bitstream position (in bits)

	/** Read a data value. */
	inline uint32 readData() {
		if (isLE) {
			if (valueBits ==  8)
				return _stream->readByte();
			if (valueBits == 16)
				return _stream->readUint16LE();
			if (valueBits == 32)
				return _stream->readUint32LE();
		} else {
			if (valueBits ==  8)
				return _stream->readByte();
			if (valueBits == 16)
				return _stream->readUint16BE();
			if (valueBits == 32)
				return _stream->readUint32BE();
		}

		assert(false);
		return 0;
	}

	/** Read the next data value. */
	inline void readValue() {
		if (_size - _pos < valueBits)
			error("BitStreamImpl::readValue(): End of bit stream reached");

		_value = readData();
		if (_stream->err() || _stream->eos())
			error("BitStreamImpl::readValue(): Read error");

		// If we're reading the bits MSB first, we need to shift the value to that position
		if (isMSB2LSB)
			_value <<= 32 - valueBits;
		}

public:
	/** Create a bit stream using this input data stream and optionally delete it on destruction. */
	BitStreamImpl(STREAM *stream, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::NO) :
		_stream(stream), _disposeAfterUse(disposeAfterUse), _value(0), _inValue(0), _pos(0) {

		if ((valueBits != 8) && (valueBits != 16) && (valueBits != 32))
			error("BitStreamImpl: Invalid memory layout %d, %d, %d", valueBits, isLE, isMSB2LSB);

		_size = (_stream->size() & ~((uint32) ((valueBits >> 3) - 1))) * 8;
	}

	/** Create a bit stream using this input data stream. */
	BitStreamImpl(STREAM &stream) :
		_stream(&stream), _disposeAfterUse(DisposeAfterUse::NO), _value(0), _inValue(0), _pos(0) {

		if ((valueBits != 8) && (valueBits != 16) && (valueBits != 32))
			error("BitStreamImpl: Invalid memory layout %d, %d, %d", valueBits, isLE, isMSB2LSB);

		_size = (_stream->size() & ~((uint32) ((valueBits >> 3) - 1))) * 8;
	}

	~BitStreamImpl() {
		if (_disposeAfterUse == DisposeAfterUse::YES)
			delete _stream;
	}

private:
	uint32 getBit_internal() {
		// Get the current bit
		uint32 b = 0;
		if (isMSB2LSB)
			b = ((_value & 0x80000000) == 0) ? 0 : 1;
		else
			b = ((_value & 1) == 0) ? 0 : 1;

		// Shift to the next bit
		if (isMSB2LSB)
			_value <<= 1;
		else
			_value >>= 1;

		return b;
	}

public:
	/** Read a bit from the bit stream. */
	uint32 getBit() {
		// Check if we need the next value
		if (_inValue == 0)
			readValue();

		uint32 b = getBit_internal();

		// Increase the position within the current value
		_inValue = (_inValue + 1) % valueBits;
		_pos++;

		return b;
	}

	/**
	 * Read a multi-bit value from the bit stream.
	 *
	 * The value is read as if just taken as a whole from the bitstream.
	 *
	 * For example:
	 * Reading a 4-bit value from an 8-bit bitstream with the contents 01010011:
	 * If the bitstream is MSB2LSB, the 4-bit value would be 0101.
	 * If the bitstream is LSB2MSB, the 4-bit value would be 0011.
	 */
	uint32 getBits(uint8 n) {
		if (n == 0)
			return 0;

		if (n > 32)
			error("BitStreamImpl::getBits(): Too many bits requested to be read");

		// Read the number of bits
		uint32 v = 0;

		uint8 nOrig = n;
		if (_inValue) {
			int count = MIN((int)n, valueBits - _inValue);
			for (int i = 0; i < count; ++i) {
				if (isMSB2LSB) {
					v = (v << 1) | getBit_internal();
				} else {
					v = (v >> 1) | (getBit_internal() << 31);
				}
			}

			n -= count;
		}

		while (n > 0) {
			// NB: readValue doesn't care that _inValue is incorrect here
			readValue();

			int count = MIN((int)n, valueBits);
			for (int i = 0; i < count; ++i) {
				if (isMSB2LSB) {
					v = (v << 1) | getBit_internal();
				} else {
					v = (v >> 1) | (getBit_internal() << 31);
				}
			}

			n -= count;
		}

		_inValue = (_inValue + nOrig) % valueBits;
		_pos += nOrig;

		if (!isMSB2LSB)
			v >>= (32 - nOrig);

		return v;
	}

	/** Read a bit from the bit stream, without changing the stream's position. */
	uint32 peekBit() {
		uint32 value   = _value;
		uint8  inValue = _inValue;
		uint32 curStreamPos  = _stream->pos();
		uint32 curPos = _pos;

		uint32 v = getBit();

		_pos     = curPos;
		_stream->seek(curStreamPos);
		_inValue = inValue;
		_value   = value;

		return v;
	}

	/**
	 * Read a multi-bit value from the bit stream, without changing the stream's position.
	 *
	 * The bit order is the same as in getBits().
	 */
	uint32 peekBits(uint8 n) {
		uint32 value   = _value;
		uint8  inValue = _inValue;
		uint32 curStreamPos  = _stream->pos();
		uint32 curPos = _pos;

		uint32 v = getBits(n);

		_pos     = curPos;
		_stream->seek(curStreamPos);
		_inValue = inValue;
		_value   = value;

		return v;
	}

	/**
	 * Add a bit to the value x, making it an n+1-bit value.
	 *
	 * The current value is shifted and the bit is added to the
	 * appropriate place, dependant on the stream's bitorder.
	 *
	 * For example:
	 * A bit y is added to the value 00001100 with size 4.
	 * If the stream's bitorder is MSB2LSB, the resulting value is 0001100y.
	 * If the stream's bitorder is LSB2MSB, the resulting value is 000y1100.
	 */
	void addBit(uint32 &x, uint32 n) {
		if (n >= 32)
			error("BitStreamImpl::addBit(): Too many bits requested to be read");

		if (isMSB2LSB)
			x = (x << 1) | getBit();
		else
			x = (x & ~(1 << n)) | (getBit() << n);
	}

	/** Rewind the bit stream back to the start. */
	void rewind() {
		_stream->seek(0);

		_value   = 0;
		_inValue = 0;
		_pos     = 0;
	}

	/** Skip the specified amount of bits. */
	void skip(uint32 n) {
		while (n-- > 0)
			getBit();
	}

	/** Skip the bits to closest data value border. */
	void align() {
		while (_inValue)
			getBit();
	}

	/** Return the stream position in bits. */
	uint32 pos() const {
		return _pos;
	}

	/** Return the stream size in bits. */
	uint32 size() const {
		return _size;
	}

	bool eos() const {
		return _stream->eos() || (_pos >= _size);
	}
};



/**
 * A cut-down version of MemoryReadStream specifically for use with BitStream.
 * It removes the virtual call overhead for reading bytes from a memory buffer,
 * and allows directly inlining this access.
 *
 * The code duplication with MemoryReadStream is not ideal.
 * It might be possible to avoid this by making this a final subclass of
 * MemoryReadStream, but that is a C++11 feature.
 */
class BitStreamMemoryStream {
private:
	const byte * const _ptrOrig;
	const byte *_ptr;
	const uint32 _size;
	uint32 _pos;
	DisposeAfterUse::Flag _disposeMemory;
	bool _eos;

public:
	BitStreamMemoryStream(const byte *dataPtr, uint32 dataSize, DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) :
		_ptrOrig(dataPtr),
		_ptr(dataPtr),
		_size(dataSize),
		_pos(0),
		_disposeMemory(disposeMemory),
		_eos(false) {}

	~BitStreamMemoryStream() {
		if (_disposeMemory)
			free(const_cast<byte *>(_ptrOrig));
	}

	bool eos() const {
		return _eos;
	}

	bool err() const {
		return false;
	}

	int32 pos() const {
		return _pos;
	}

	int32 size() const {
		return _size;
	}

	bool seek(uint32 offset) {
		assert(offset <= _size);

		_eos = false;
		_pos = offset;
		_ptr = _ptrOrig + _pos;
		return true;
	}

	byte readByte() {
		if (_pos >= _size) {
			_eos = true;
			return 0;
		}

		_pos++;
		return *_ptr++;
	}

	uint16 readUint16LE() {
		if (_pos + 2 > _size) {
			_eos = true;
			if (_pos < _size) {
				_pos++;
				return *_ptr++;
			} else {
				return 0;
			}
		}

		uint16 val = READ_LE_UINT16(_ptr);

		_pos += 2;
		_ptr += 2;

		return val;
	}

	uint16 readUint16BE() {
		if (_pos + 2 > _size) {
			_eos = true;
			if (_pos < _size) {
				_pos++;
				return (*_ptr++) << 8;
			} else {
				return 0;
			}
		}

		uint16 val = READ_LE_UINT16(_ptr);

		_pos += 2;
		_ptr += 2;

		return val;
	}

	uint32 readUint32LE() {
		if (_pos + 4 > _size) {
			uint32 val = readByte();
			val |= (uint32)readByte() << 8;
			val |= (uint32)readByte() << 16;
			val |= (uint32)readByte() << 24;

			return val;
		}

		uint32 val = READ_LE_UINT32(_ptr);

		_pos += 4;
		_ptr += 4;

		return val;
	}

	uint32 readUint32BE() {
		if (_pos + 4 > _size) {
			uint32 val = (uint32)readByte() << 24;
			val |= (uint32)readByte() << 16;
			val |= (uint32)readByte() << 8;
			val |= (uint32)readByte();

			return val;
		}

		uint32 val = READ_BE_UINT32(_ptr);

		_pos += 4;
		_ptr += 4;

		return val;
	}

};


// typedefs for various memory layouts.

/** 8-bit data, MSB to LSB. */
typedef BitStreamImpl<SeekableReadStream, 8, false, true > BitStream8MSB;
/** 8-bit data, LSB to MSB. */
typedef BitStreamImpl<SeekableReadStream, 8, false, false> BitStream8LSB;

/** 16-bit little-endian data, MSB to LSB. */
typedef BitStreamImpl<SeekableReadStream, 16, true , true > BitStream16LEMSB;
/** 16-bit little-endian data, LSB to MSB. */
typedef BitStreamImpl<SeekableReadStream, 16, true , false> BitStream16LELSB;
/** 16-bit big-endian data, MSB to LSB. */
typedef BitStreamImpl<SeekableReadStream, 16, false, true > BitStream16BEMSB;
/** 16-bit big-endian data, LSB to MSB. */
typedef BitStreamImpl<SeekableReadStream, 16, false, false> BitStream16BELSB;

/** 32-bit little-endian data, MSB to LSB. */
typedef BitStreamImpl<SeekableReadStream, 32, true , true > BitStream32LEMSB;
/** 32-bit little-endian data, LSB to MSB. */
typedef BitStreamImpl<SeekableReadStream, 32, true , false> BitStream32LELSB;
/** 32-bit big-endian data, MSB to LSB. */
typedef BitStreamImpl<SeekableReadStream, 32, false, true > BitStream32BEMSB;
/** 32-bit big-endian data, LSB to MSB. */
typedef BitStreamImpl<SeekableReadStream, 32, false, false> BitStream32BELSB;



/** 8-bit data, MSB to LSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 8, false, true > BitStreamMemory8MSB;
/** 8-bit data, LSB to MSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 8, false, false> BitStreamMemory8LSB;

/** 16-bit little-endian data, MSB to LSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 16, true , true > BitStreamMemory16LEMSB;
/** 16-bit little-endian data, LSB to MSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 16, true , false> BitStreamMemory16LELSB;
/** 16-bit big-endian data, MSB to LSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 16, false, true > BitStreamMemory16BEMSB;
/** 16-bit big-endian data, LSB to MSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 16, false, false> BitStreamMemory16BELSB;

/** 32-bit little-endian data, MSB to LSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 32, true , true > BitStreamMemory32LEMSB;
/** 32-bit little-endian data, LSB to MSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 32, true , false> BitStreamMemory32LELSB;
/** 32-bit big-endian data, MSB to LSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 32, false, true > BitStreamMemory32BEMSB;
/** 32-bit big-endian data, LSB to MSB. */
typedef BitStreamImpl<BitStreamMemoryStream, 32, false, false> BitStreamMemory32BELSB;


} // End of namespace Common

#endif // COMMON_BITSTREAM_H
