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

/*
 * Pack-Ice depacker is based on IceDecompressor:
 * https://github.com/temisu/ancient
 *
 * BSD 2-Clause License
 *
 * Copyright (c) 2017-2026, Teemu Suutari
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common/compression/packice.h"

#include "common/endian.h"

namespace Common {

namespace PackIce {

static uint32 makeBitMask(uint8 bitCount) {
	return bitCount == 32 ? 0xFFFFFFFF : ((1U << bitCount) - 1);
}

static bool detectHeader(uint32 hdr, uint32 footer) {
	return footer == MKTAG('I', 'c', 'e', '!') ||
			hdr == MKTAG('I', 'c', 'e', '!') ||
			hdr == MKTAG('T', 'M', 'M', '!') ||
			hdr == MKTAG('T', 'S', 'M', '!') ||
			hdr == MKTAG('S', 'H', 'E', '!') ||
			hdr == MKTAG('I', 'C', 'E', '!');
}

class BackwardInputStream {
public:
	BackwardInputStream(const byte *data, uint32 startOffset, uint32 endOffset)
		: _data(data), _pos(endOffset), _startOffset(startOffset) {
	}

	bool readByte(byte &value) {
		if (_pos <= _startOffset)
			return false;
		value = _data[--_pos];
		return true;
	}

	bool readBE32(uint32 &value) {
		if (_pos < _startOffset + 4)
			return false;
		_pos -= 4;
		value = READ_BE_UINT32(_data + _pos);
		return true;
	}

	bool eof() const {
		return _pos == _startOffset;
	}

	uint32 remainingBytes() const {
		return _pos - _startOffset;
	}

private:
	const byte *_data;
	uint32 _pos;
	uint32 _startOffset;
};

class MSBBitReader {
public:
	MSBBitReader(BackwardInputStream &inputStream) : _inputStream(inputStream) {
	}

	void reset(uint32 value, uint8 bitCount) {
		_bitBuffer = value;
		_bitsLeft = bitCount;
	}

	bool readBitsBE32(uint32 count, uint32 &value) {
		value = 0;
		while (count) {
			if (!_bitsLeft) {
				if (!_inputStream.readBE32(_bitBuffer))
					return false;
				_bitsLeft = 32;
			}
			const uint8 bitsToRead = (count < _bitsLeft) ? (uint8)count : _bitsLeft;
			_bitsLeft -= bitsToRead;
			const uint32 nextBits = (_bitBuffer >> _bitsLeft) & makeBitMask(bitsToRead);
			value = bitsToRead == 32 ? nextBits : ((value << bitsToRead) | nextBits);
			count -= bitsToRead;
		}
		return true;
	}

	bool readBits8(uint32 count, uint32 &value) {
		value = 0;
		while (count) {
			if (!_bitsLeft) {
				byte nextByte = 0;
				if (!_inputStream.readByte(nextByte))
					return false;
				_bitBuffer = nextByte;
				_bitsLeft = 8;
			}
			const uint8 bitsToRead = (count < _bitsLeft) ? (uint8)count : _bitsLeft;
			_bitsLeft -= bitsToRead;
			const uint32 nextBits = (_bitBuffer >> _bitsLeft) & makeBitMask(bitsToRead);
			value = bitsToRead == 32 ? nextBits : ((value << bitsToRead) | nextBits);
			count -= bitsToRead;
		}
		return true;
	}

	uint32 availableBits() const {
		return _bitsLeft + _inputStream.remainingBytes() * 8;
	}

private:
	BackwardInputStream &_inputStream;
	uint32 _bitBuffer = 0;
	uint8 _bitsLeft = 0;
};

class BitReaderProxy {
public:
	BitReaderProxy(MSBBitReader &bitReader, bool useBytes) : _bitReader(bitReader), _useBytes(useBytes) {
	}

	bool readBits(uint32 count, uint32 &value) {
		return _useBytes ? _bitReader.readBits8(count, value) : _bitReader.readBitsBE32(count, value);
	}

private:
	MSBBitReader &_bitReader;
	bool _useBytes;
};

class VariableLengthCodeDecoder {
public:
	VariableLengthCodeDecoder(const uint8 *bitLengths, uint32 count) : _bitLengths(bitLengths), _count(count) {
		_offsets[0] = 0;
		for (uint32 i = 1; i < _count; ++i)
			_offsets[i] = _offsets[i - 1] + (1U << _bitLengths[i - 1]);
	}

	bool decode(BitReaderProxy &bits, uint32 base, uint32 &value) const {
		if (base >= _count)
			return false;

		uint32 extra = 0;
		if (!bits.readBits(_bitLengths[base], extra))
			return false;
		value = _offsets[base] + extra;
		return true;
	}

	bool decodeCascade(BitReaderProxy &bits, uint32 &value) const {
		for (uint32 i = 0; i < _count; ++i) {
			const uint8 bitLength = _bitLengths[i];
			uint32 extra = 0;
			if (!bits.readBits(bitLength, extra))
				return false;
			if (i + 1 == _count || extra != makeBitMask(bitLength)) {
				value = _offsets[i] - i + extra;
				return true;
			}
		}

		return false;
	}

private:
	const uint8 *_bitLengths;
	uint32 _count;
	uint32 _offsets[8];
};

class BackwardOutputStream {
public:
	BackwardOutputStream(Common::Array<byte> &data) : _data(data), _pos(data.size()) {
	}

	bool writeByte(byte value) {
		if (!_pos)
			return false;
		_data[--_pos] = value;
		return true;
	}

	bool copy(uint32 distance, uint32 count) {
		if (!distance || _pos < count || _pos + distance > _data.size())
			return false;

		for (uint32 i = 0; i < count; ++i) {
			--_pos;
			_data[_pos] = _data[_pos + distance];
		}
		return true;
	}

	bool eof() const {
		return _pos == 0;
	}

private:
	Common::Array<byte> &_data;
	uint32 _pos;
};

static bool decompressInternal(const byte *data, uint32 streamStart, uint32 streamEnd, Common::Array<byte> &rawData,
		Common::PackIceVersion version, bool useBytes, bool allowPictureMode) {
	static const uint8 litOld[] = { 1, 2, 2, 3, 10 };
	static const uint8 litNew[] = { 1, 2, 2, 3, 8, 15 };
	static const uint8 countBaseBits[] = { 1, 1, 1, 1 };
	static const uint8 countBits[] = { 0, 0, 1, 2, 10 };
	static const uint8 distanceBaseBits[] = { 1, 1 };
	static const uint8 distanceBits[] = { 5, 8, 12 };

	BackwardInputStream inputStream(data, streamStart, streamEnd);
	MSBBitReader bitReader(inputStream);
	BitReaderProxy bits(bitReader, useBytes);

	uint32 value = 0;
	if (useBytes) {
		byte initialByte = 0;
		if (!inputStream.readByte(initialByte))
			return false;
		value = initialByte;
	} else {
		if (!inputStream.readBE32(value))
			return false;
	}

	uint32 shiftedValue = value;
	uint32 count = 0;
	while (shiftedValue) {
		shiftedValue <<= 1;
		++count;
	}
	if (count)
		--count;
	if (count)
		bitReader.reset(value >> (32 - count), count - (useBytes ? 24 : 0));

	BackwardOutputStream outputStream(rawData);

	VariableLengthCodeDecoder litVlcDecoderOld(litOld, ARRAYSIZE(litOld));
	VariableLengthCodeDecoder litVlcDecoderNew(litNew, ARRAYSIZE(litNew));
	VariableLengthCodeDecoder countBaseDecoder(countBaseBits, ARRAYSIZE(countBaseBits));
	VariableLengthCodeDecoder countDecoder(countBits, ARRAYSIZE(countBits));
	VariableLengthCodeDecoder distanceBaseDecoder(distanceBaseBits, ARRAYSIZE(distanceBaseBits));
	VariableLengthCodeDecoder distanceDecoder(distanceBits, ARRAYSIZE(distanceBits));

	for (;;) {
		uint32 bit = 0;
		if (!bits.readBits(1, bit))
			return false;
		if (bit) {
			uint32 litLength = 0;
			if (version ? !litVlcDecoderNew.decodeCascade(bits, litLength) :
					!litVlcDecoderOld.decodeCascade(bits, litLength))
				return false;
			++litLength;
			for (uint32 i = 0; i < litLength; ++i) {
				byte literal = 0;
				if (!inputStream.readByte(literal) || !outputStream.writeByte(literal))
					return false;
			}
		}

		if (outputStream.eof())
			break;

		uint32 countBase = 0;
		if (!countBaseDecoder.decodeCascade(bits, countBase))
			return false;
		uint32 copyCount = 0;
		if (!countDecoder.decode(bits, countBase, copyCount))
			return false;
		copyCount += 2;

		uint32 distance = 0;
		if (copyCount == 2) {
			uint32 bitValue = 0;
			if (!bits.readBits(1, bitValue))
				return false;
			if (bitValue) {
				if (!bits.readBits(9, distance))
					return false;
				distance += 0x40;
			} else if (!bits.readBits(6, distance)) {
				return false;
			}
			distance += copyCount - (useBytes ? 1 : 0);
		} else {
			uint32 distanceBase = 0;
			if (!distanceBaseDecoder.decodeCascade(bits, distanceBase))
				return false;
			if (distanceBase < 2)
				distanceBase ^= 1;
			if (!distanceDecoder.decode(bits, distanceBase, distance))
				return false;
			if (useBytes) {
				if (distance)
					distance += copyCount - 1;
				else
					distance = 1;
			} else {
				distance += copyCount;
			}
		}

		if (!outputStream.copy(distance, copyCount))
			return false;
	}

	if (allowPictureMode && version && bitReader.availableBits()) {
		uint32 pictureMode = 0;
		if (!bits.readBits(1, pictureMode))
			return false;
		if (pictureMode) {
			uint32 pictureSize = 32000;
			if (version == Common::kPackIceVersion231) {
				uint32 hasPictureSize = 0;
				if (bitReader.availableBits() >= 17 && bits.readBits(1, hasPictureSize) && hasPictureSize) {
					if (!bits.readBits(16, pictureSize))
						return false;
					pictureSize = pictureSize * 8 + 8;
				}
			}
			if (!Common::convertPackIcePictureData(rawData, pictureSize))
				return false;
		}
	}

	return inputStream.eof();
}

} // End of namespace PackIce

bool detectPackIceHeader(const byte *data, uint32 size, bool exactSizeKnown) {
	if (!data || size < 8)
		return false;

	const uint32 hdr = READ_BE_UINT32(data);
	const uint32 footer = exactSizeKnown ? READ_BE_UINT32(data + size - 4) : 0;
	return PackIce::detectHeader(hdr, footer);
}

bool parsePackIceHeader(const byte *data, uint32 size, bool exactSizeKnown, PackIceHeader &header) {
	if (!detectPackIceHeader(data, size, exactSizeKnown))
		return false;

	const uint32 hdr = READ_BE_UINT32(data);
	const uint32 footer = exactSizeKnown ? READ_BE_UINT32(data + size - 4) : 0;
	if (footer == MKTAG('I', 'c', 'e', '!')) {
		header.packedSize = size;
		header.rawSize = READ_BE_UINT32(data + size - 8);
		header.version = kPackIceVersion110;
	} else {
		header.packedSize = READ_BE_UINT32(data + 4);
		if (!header.packedSize || header.packedSize > size)
			return false;
		header.rawSize = READ_BE_UINT32(data + 8);
		header.version = (hdr == MKTAG('I', 'C', 'E', '!')) ? kPackIceVersion231 : kPackIceVersion200;
	}

	return header.rawSize != 0;
}

const char *getPackIceName(PackIceVersion version) {
	static const char *const names[] = {
		"Ice: Pack-Ice v1.1 - v1.14",
		"Ice: Pack-Ice v2.0 - v2.20",
		"ICE: Pack-Ice v2.31+"
	};

	return names[version];
}

bool decompressPackIce(const byte *data, uint32 size, Common::Array<byte> &out, bool exactSizeKnown) {
	PackIceHeader header;
	if (!parsePackIceHeader(data, size, exactSizeKnown, header))
		return false;

	out.resize(header.rawSize);
	const uint32 streamStart = header.version ? 12 : 0;
	const uint32 streamEnd = header.packedSize - (header.version ? 0 : 8);

	if (header.version) {
		if (header.version == kPackIceVersion200 &&
				PackIce::decompressInternal(data, streamStart, streamEnd, out, header.version, false, true))
			return true;

		out.resize(header.rawSize);
		if (!PackIce::decompressInternal(data, streamStart, streamEnd, out, header.version, true, true))
			return false;
		return true;
	}

	return PackIce::decompressInternal(data, streamStart, streamEnd, out, header.version, false, false);
}

bool decompressPackIceStream(const byte *data, uint32 size, uint32 streamStart, uint32 streamEnd,
		uint32 rawSize, Common::Array<byte> &out, bool useBytes) {
	if (!data || streamStart >= streamEnd || streamEnd > size || rawSize == 0)
		return false;

	out.resize(rawSize);
	return PackIce::decompressInternal(data, streamStart, streamEnd, out, kPackIceVersion200, useBytes, false);
}

bool convertPackIcePictureData(Common::Array<byte> &data, uint32 pictureSize) {
	if (!pictureSize)
		return true;
	if (data.size() < pictureSize)
		return false;

	const uint32 start = data.size() - pictureSize;
	for (uint32 i = start; i + 7 < data.size(); i += 8) {
		uint16 values[4] = { 0, 0, 0, 0 };
		for (uint32 j = 0; j < 8; j += 2) {
			uint16 tmp = READ_BE_UINT16(data.begin() + i + 6 - j);
			for (uint32 k = 0; k < 16; ++k) {
				values[k & 3] = (uint16)((values[k & 3] << 1) | (tmp >> 15));
				tmp <<= 1;
			}
		}
		for (uint32 j = 0; j < 4; ++j) {
			data[i + j * 2] = (byte)(values[j] >> 8);
			data[i + j * 2 + 1] = (byte)values[j];
		}
	}

	return true;
}

} // End of namespace Common
