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

#include "igor/unpack.h"

#include "common/crc.h"
#include "common/endian.h"

namespace Igor {
namespace {

const uint32 kSupportedFileSize = 39537;
const uint32 kSupportedCRC32 = 0x8edcfa28;
const uint32 kImageSize = kSpanishFloppyExeImageSize;
const uint32 kEncodedWordBytes = 38972;
const uint32 kCompressedDataOffset = 656;
const uint32 kLengthTable = 586;
const uint32 kExtendedLengthTable = 597;
const uint32 kLongLengthTable = 608;
const uint32 kDistanceTable = 624;

class UnpackReader {
public:
	UnpackReader(const Common::Array<byte> &bytes)
		: _bytes(bytes), _position(kCompressedDataOffset), _control(0),
		  _remaining(0), _error(nullptr) {
		refill();
	}

	byte readByte() {
		if (_position >= _bytes.size()) {
			fail("Truncated compressed stream");
			return 0;
		}
		return _bytes[_position++];
	}

	uint16 readWord() {
		const uint16 low = readByte();
		return low | (uint16(readByte()) << 8);
	}

	uint bit() {
		const uint value = _control & 1;
		_control >>= 1;
		// Refill before reading a following literal or match distance.
		if (--_remaining == 0)
			refill();
		return value;
	}

	uint bits(uint count) {
		uint value = 0;
		while (count--)
			value = (value << 1) | bit();
		return value;
	}

	uint appendBit(uint value) { return (value << 1) | bit(); }

	uint table(uint32 base, uint index) {
		if (index >= 32 || base + index >= _bytes.size()) {
			fail("Invalid decompression table index");
			return 0;
		}
		return _bytes[base + index];
	}

	void fail(const char *message) {
		if (!_error)
			_error = message;
	}

	const char *getError() const { return _error; }

private:
	void refill() {
		_control = readWord();
		_remaining = 16;
	}

	const Common::Array<byte> &_bytes;
	uint32 _position;
	uint16 _control;
	uint _remaining;
	const char *_error;
};

uint matchLength(UnpackReader &reader) {
	uint code = reader.bits(2);
	if (code >= 2)
		return code;

	code = reader.appendBit(code);
	if (code != 0) {
		code = reader.appendBit(code);
		if (code >= 5)
			code = reader.appendBit(code);
	}
	if (code <= 12)
		return reader.table(kLengthTable, code);

	code = reader.appendBit(code & 3);
	if (code >= 5)
		code = reader.appendBit(code);
	if (code <= 12)
		return reader.table(kExtendedLengthTable, code);

	code = reader.appendBit(code & 3);
	if (code >= 5)
		code = reader.appendBit(code);
	return reader.table(kLongLengthTable, code);
}

uint matchDistance(UnpackReader &reader, uint length) {
	uint high = 0;
	if (length != 2 && reader.bit() == 0) {
		uint code = reader.bits(3);
		if (code >= 2) {
			code = reader.appendBit(code);
			if (code >= 8) {
				code = reader.appendBit(code);
				if (code >= 23) {
					code = reader.appendBit(code);
					high = code & 0xdf;
				} else {
					high = reader.table(kDistanceTable, code);
				}
			} else {
				high = reader.table(kDistanceTable, code);
			}
		} else {
			high = reader.table(kDistanceTable, code);
		}
	}
	return (high << 8) | reader.readByte();
}

void checkRelocations(UnpackReader &reader, uint32 imageSize) {
	// Resources use the unrelocated image; validate the relocation metadata
	// without changing pointers to depend on a DOS load address.
	while (!reader.getError()) {
		const uint count = reader.readByte();
		if (count == 0)
			return;
		const uint32 segment = reader.readWord();
		for (uint i = 0; i < count && !reader.getError(); ++i) {
			const uint32 offset = reader.readWord();
			const uint32 location = segment * 16 + offset;
			if (location >= imageSize || imageSize - location < 2)
				reader.fail("Relocation outside resident image");
		}
	}
}

} // End of anonymous namespace

bool unpackSpanishFloppyExe(const byte *input, uint32 size,
                           Common::Array<byte> &image, Common::String &error) {
	image.clear();
	error.clear();
	if (!input || size != kSupportedFileSize || Common::CRC32().crcFast(input, size) != kSupportedCRC32) {
		error = "Unsupported packed Spanish floppy executable";
		return false;
	}

	Common::Array<byte> decoded;
	decoded.resize(size);
	memcpy(decoded.data(), input, size);
	// Restore the encoded prefix used by the native word transform.
	decoded[0] = 0xcc;
	decoded[1] = 0x2f;
	decoded[2] = 0x3f;
	uint16 key = 0x3ad8;
	for (uint32 i = 0; i < kEncodedWordBytes; i += 2) {
		const uint16 difference = uint16(READ_LE_UINT16(decoded.data() + i) - key);
		key = difference;
		WRITE_LE_UINT16(decoded.data() + i, uint16((difference << 1) | (difference >> 15)));
	}

	UnpackReader reader(decoded);
	Common::Array<byte> result;
	result.reserve(kImageSize);
	bool ended = false;
	for (uint32 token = 0; token < size * 2 && !reader.getError(); ++token) {
		if (reader.bit() == 0) {
			if (result.size() == kImageSize) {
				reader.fail("Resident image exceeds supported size");
				break;
			}
			result.push_back(reader.readByte());
			continue;
		}

		uint length = matchLength(reader);
		if (length == 25) {
			const uint extension = reader.readByte();
			if (extension >= 254) {
				if (extension == 255) {
					ended = true;
					break;
				}
				// Segment refresh changes address representation only.
				continue;
			}
			length += extension;
		}
		if (length < 2 || length > kImageSize - result.size()) {
			reader.fail("Invalid match length");
			break;
		}
		const uint distance = matchDistance(reader, length);
		if (distance == 0 || distance > result.size()) {
			reader.fail("Invalid match distance");
			break;
		}
		for (uint i = 0; i < length; ++i)
			result.push_back(result[result.size() - distance]);
	}
	if (!ended || result.size() != kImageSize)
		reader.fail("Incomplete resident image");
	if (!reader.getError())
		checkRelocations(reader, result.size());
	if (reader.getError()) {
		error = reader.getError();
		return false;
	}
	image.swap(result);
	return true;
}

} // End of namespace Igor
