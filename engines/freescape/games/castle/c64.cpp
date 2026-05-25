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

#include "common/file.h"
#include "graphics/managed_surface.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/c64.music.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

enum {
	kCastleC64DatabaseOffset = 0x9951,
	kCastleC64RuntimeDemoPointerOffset = 0x42,
	kCastleC64RuntimeAreaTableOffset = 0x4f,
	kCastleC64CompactDemoPointerOffset = 0x3e,
	kCastleC64CompactAreaTableOffset = 0x4b,
	kCastleC64BackgroundColor = 0x00,
	kCastleC64ScreenHighNibbleColor = 0x0c,
	kCastleC64ColorRamColor = 0x01,
	kCastleC64BorderCropLeft = 32,
	kCastleC64BorderCropTop = 35,
	kCastleC64MessageLeft = 118,
	kCastleC64MessageRight = 280,
	kCastleC64MessageX = 120,
	kCastleC64MessageY = 182
};

struct CastleC64Repeat {
	uint16 offset;
	byte count;
	byte value;
};

const uint16 kCastleC64DatabaseSkips[] = {
	0x01fc, 0x02fc, 0x05fa, 0x08f9, 0x09f9, 0x0af9, 0x0bf4, 0x0cf0,
	0x0deb, 0x0fe9, 0x10e5, 0x11e3, 0x13e1, 0x14de, 0x18dd, 0x19db,
	0x1cd9, 0x1ed7, 0x20cf, 0x21c8
};

const CastleC64Repeat kCastleC64DatabaseRepeats[] = {
	{ 0x0006, 4, 0x00 }, { 0x0009, 4, 0xff }, { 0x0010, 4, 0x55 }, { 0x001b, 4, 0xaa },
	{ 0x02dd, 4, 0x02 }, { 0x0327, 4, 0x00 }, { 0x0347, 4, 0x01 }, { 0x0355, 4, 0x00 },
	{ 0x05fe, 4, 0x00 }, { 0x0610, 4, 0x01 }, { 0x0936, 4, 0x00 }, { 0x0aca, 4, 0x00 },
	{ 0x0b58, 6, 0x00 }, { 0x0bd9, 6, 0x00 }, { 0x0c5a, 4, 0x01 }, { 0x0c69, 4, 0x01 },
	{ 0x0c78, 4, 0x02 }, { 0x0c87, 4, 0x02 }, { 0x0c95, 4, 0x00 }, { 0x0d43, 6, 0x00 },
	{ 0x0d9e, 6, 0x00 }, { 0x0ec9, 6, 0x00 }, { 0x0fea, 5, 0x00 }, { 0x1085, 6, 0x00 },
	{ 0x1163, 6, 0x00 }, { 0x127e, 6, 0x00 }, { 0x140b, 4, 0x06 }, { 0x1497, 6, 0x00 },
	{ 0x1543, 5, 0x00 }, { 0x1910, 6, 0x00 }, { 0x1a1b, 6, 0x00 }, { 0x1da9, 6, 0x00 },
	{ 0x1edb, 6, 0x00 }, { 0x1ee1, 6, 0x00 }, { 0x1ee7, 6, 0x00 }, { 0x20d0, 4, 0x00 },
	{ 0x2128, 6, 0x00 }, { 0x217e, 6, 0x00 }, { 0x21c5, 4, 0x00 }, { 0x2250, 6, 0x00 },
	{ 0x2255, 113, 0x00 }
};

static const byte kCastleC64FontData[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00,
	0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x66, 0x66, 0xff, 0x66, 0xff, 0x66, 0x66, 0x00,
	0x18, 0x3e, 0x58, 0x3c, 0x1a, 0x7c, 0x18, 0x00,
	0x62, 0x66, 0x0c, 0x18, 0x30, 0x66, 0x46, 0x00,
	0x3c, 0x66, 0x3c, 0x38, 0x67, 0x66, 0x3f, 0x00,
	0x06, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00,
	0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00,
	0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00,
	0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30,
	0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00,
	0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00,
	0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7e, 0x00,
	0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00,
	0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00,
	0x06, 0x0e, 0x1e, 0x66, 0x7f, 0x06, 0x06, 0x00,
	0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00,
	0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00,
	0x7e, 0x66, 0x0c, 0x18, 0x18, 0x18, 0x18, 0x00,
	0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00,
	0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00,
	0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00,
	0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30,
	0x0e, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0e, 0x00,
	0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00,
	0x70, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x70, 0x00,
	0x3c, 0x66, 0x06, 0x0c, 0x18, 0x00, 0x18, 0x00,
	0x3c, 0x66, 0x6e, 0x6e, 0x60, 0x62, 0x3c, 0x00,
	0x18, 0x3c, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00,
	0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00,
	0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00,
	0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00,
	0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7e, 0x00,
	0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00,
	0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3c, 0x00,
	0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00,
	0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
	0x1e, 0x0c, 0x0c, 0x0c, 0x0c, 0x6c, 0x38, 0x00,
	0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00,
	0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00,
	0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x00,
	0x66, 0x76, 0x7e, 0x7e, 0x6e, 0x66, 0x66, 0x00,
	0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00,
	0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00,
	0x3c, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x0e, 0x00,
	0x7c, 0x66, 0x66, 0x7c, 0x78, 0x6c, 0x66, 0x00,
	0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00,
	0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
	0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00,
	0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00,
	0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00,
	0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00,
	0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00,
	0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00,
	0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00,
	0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x00,
	0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00,
	0x18, 0x3c, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
	0x18, 0x0c, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00,
	0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x7c, 0x00,
	0x00, 0x00, 0x3c, 0x60, 0x60, 0x60, 0x3c, 0x00,
	0x00, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3e, 0x00,
	0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00,
	0x00, 0x0e, 0x18, 0x3e, 0x18, 0x18, 0x18, 0x00,
	0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x7c,
	0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x00,
	0x00, 0x18, 0x00, 0x38, 0x18, 0x18, 0x3c, 0x00,
	0x00, 0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x3c,
	0x00, 0x60, 0x60, 0x6c, 0x78, 0x6c, 0x66, 0x00,
	0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
	0x00, 0x00, 0x66, 0x7f, 0x7f, 0x6b, 0x63, 0x00,
	0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00,
	0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00,
	0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60,
	0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06,
	0x00, 0x00, 0x7c, 0x66, 0x60, 0x60, 0x60, 0x00,
	0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00,
	0x00, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x0e, 0x00,
	0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00,
	0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00,
	0x00, 0x00, 0x63, 0x6b, 0x7f, 0x3e, 0x36, 0x00,
	0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00,
	0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x0c, 0x78,
	0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00,
	0x0e, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0e, 0x00,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
	0x70, 0x18, 0x18, 0x0e, 0x18, 0x18, 0x70, 0x00,
	0x31, 0x6b, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

uint16 readCastleC64Uint16LE(const Common::Array<byte> &data, uint32 offset) {
	if (offset + 1 >= data.size())
		error("Castle C64 database pointer read out of range at 0x%x", offset);

	return data[offset] | (data[offset + 1] << 8);
}

Common::Array<byte> normalizeCastleC64Database(Common::SeekableReadStream *file) {
	file->seek(kCastleC64DatabaseOffset);
	if (file->pos() != kCastleC64DatabaseOffset)
		error("Unable to seek to Castle C64 database at 0x%x", kCastleC64DatabaseOffset);
	if (file->size() <= kCastleC64DatabaseOffset)
		error("Castle C64 database file is too short");

	uint32 rawSize = file->size() - kCastleC64DatabaseOffset;
	Common::Array<byte> raw;
	raw.resize(rawSize);
	if (file->read(&raw[0], rawSize) != rawSize)
		error("Unable to read Castle C64 database");
	if (raw.size() < 3)
		error("Castle C64 database is too short");

	const uint16 decodedSize = readCastleC64Uint16LE(raw, 1);
	Common::Array<byte> decoded;
	uint32 sourceOffset = 0;
	uint skipIndex = 0;
	uint repeatIndex = 0;

	while (decoded.size() < decodedSize) {
		if (sourceOffset >= raw.size())
			error("Castle C64 database normalization ran out of source data");

		if (skipIndex < ARRAYSIZE(kCastleC64DatabaseSkips) && sourceOffset == kCastleC64DatabaseSkips[skipIndex]) {
			sourceOffset++;
			skipIndex++;
			continue;
		}

		if (repeatIndex < ARRAYSIZE(kCastleC64DatabaseRepeats) && sourceOffset == kCastleC64DatabaseRepeats[repeatIndex].offset) {
			const CastleC64Repeat &repeat = kCastleC64DatabaseRepeats[repeatIndex];
			if (sourceOffset + 2 >= raw.size() || raw[sourceOffset + 1] != repeat.count || raw[sourceOffset + 2] != repeat.value)
				error("Castle C64 database repeat mismatch at 0x%x", sourceOffset);

			for (uint i = 0; i < repeat.count && decoded.size() < decodedSize; i++)
				decoded.push_back(repeat.value);

			sourceOffset += 3;
			repeatIndex++;
			continue;
		}

		decoded.push_back(raw[sourceOffset++]);
	}

	if (skipIndex != ARRAYSIZE(kCastleC64DatabaseSkips) || repeatIndex != ARRAYSIZE(kCastleC64DatabaseRepeats))
		error("Castle C64 database normalization did not consume all relocation entries");

	debugC(1, kFreescapeDebugParser, "Castle C64 normalized database: 0x%x -> 0x%x bytes", sourceOffset, decodedSize);
	return decoded;
}

Common::Array<Graphics::ManagedSurface *> loadCastleC64Font() {
	Common::Array<Graphics::ManagedSurface *> chars;

	for (uint chr = 0; chr < ARRAYSIZE(kCastleC64FontData) / 8; chr++) {
		Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
		surface->create(8, 8, Graphics::PixelFormat::createFormatCLUT8());
		surface->clear(0);

		for (int y = 0; y < 8; y++) {
			byte row = kCastleC64FontData[chr * 8 + y];
			for (int x = 0; x < 8; x++) {
				if (row & (0x80 >> x))
					surface->setPixel(x, y, 1);
			}
		}

		chars.push_back(surface);
	}

	return chars;
}

class CastleC64DatabaseReadStream : public Common::SeekableReadStream {
public:
	CastleC64DatabaseReadStream(const Common::Array<byte> &data) : _data(data), _pos(0), _eos(false), _colorMapRead(false) {
		if (_data.size() < kCastleC64RuntimeAreaTableOffset)
			error("Castle C64 normalized database is too short");

		for (uint i = 0; i < 4; i++)
			_compactPointerBytes[i] = _data[kCastleC64RuntimeDemoPointerOffset + i];

		byte areaCount = _data[0];
		_areaTable.resize(areaCount * 2);
		for (uint i = 0; i < areaCount; i++) {
			uint16 areaOffset = readCastleC64Uint16LE(_data, kCastleC64RuntimeAreaTableOffset + 2 * i);
			areaOffset += 4; // The shared C64 parser subtracts four from area pointers.
			_areaTable[2 * i] = areaOffset & 0xff;
			_areaTable[2 * i + 1] = areaOffset >> 8;
		}
	}

	uint32 read(void *dataPtr, uint32 dataSize) override {
		if (!dataPtr)
			return 0;

		if (_pos >= _data.size()) {
			_eos = true;
			return 0;
		}

		if (dataSize > _data.size() - _pos) {
			dataSize = _data.size() - _pos;
			_eos = true;
		}

		byte *dst = (byte *)dataPtr;
		for (uint32 i = 0; i < dataSize; i++)
			dst[i] = byteAt(_pos++);

		if (_pos >= kCastleC64RuntimeDemoPointerOffset)
			_colorMapRead = true;

		return dataSize;
	}

	bool eos() const override {
		return _eos;
	}

	void clearErr() override {
		_eos = false;
	}

	int64 pos() const override {
		return _pos;
	}

	int64 size() const override {
		return _data.size();
	}

	bool seek(int64 offs, int whence = SEEK_SET) override {
		switch (whence) {
		case SEEK_END:
			offs = _data.size() + offs;
			break;
		case SEEK_CUR:
			offs = _pos + offs;
			break;
		case SEEK_SET:
		default:
			break;
		}

		if (offs < 0)
			offs = 0;
		if (offs > (int64)_data.size())
			offs = _data.size();

		_pos = offs;
		_eos = false;
		return true;
	}

private:
	byte byteAt(uint32 offset) const {
		if (_colorMapRead && offset >= kCastleC64CompactDemoPointerOffset && offset < kCastleC64CompactDemoPointerOffset + 4)
			return _compactPointerBytes[offset - kCastleC64CompactDemoPointerOffset];

		if (offset >= kCastleC64CompactAreaTableOffset && offset < kCastleC64CompactAreaTableOffset + _areaTable.size())
			return _areaTable[offset - kCastleC64CompactAreaTableOffset];

		return _data[offset];
	}

	const Common::Array<byte> &_data;
	uint32 _pos;
	bool _eos;
	bool _colorMapRead;
	byte _compactPointerBytes[4];
	Common::Array<byte> _areaTable;
};

void CastleEngine::initC64() {
	_viewArea = Common::Rect(40, 32, 280, 152);
}

extern byte kC64Palette[16][3];

void CastleEngine::loadMessagesC64(Common::SeekableReadStream *file, int offset, int number) {
	file->seek(offset);
	debugC(1, kFreescapeDebugParser, "String table:");

	for (int i = 0; i < number; i++) {
		Common::String message;
		while (true) {
			byte c = file->readByte();
			if (c <= 1)
				break;
			if (c < 0x20)
				continue;
			if (c > 0xf0)
				c = ' ';
			message += c;
		}

		_messagesList.push_back(message);
		debugC(1, kFreescapeDebugParser, "'%s'", _messagesList[i].c_str());
	}
	debugC(1, kFreescapeDebugParser, "End of messages at %" PRIx64, file->pos());
}

void CastleEngine::loadRiddlesC64(Common::SeekableReadStream *file, int offset, int number) {
	file->seek(offset);

	for (int i = 0; i < number; i++) {
		Riddle riddle;
		riddle._origin = Common::Point(40, 33);

		int numberLines = file->readByte();
		debugC(1, kFreescapeDebugParser, "c64 riddle %d number of lines: %d", i, numberLines);

		for (int j = 0; j < numberLines; j++) {
			int8 x = (int8)file->readByte();
			int8 y = (int8)file->readByte();
			int size = file->readByte();

			if (size == 0xff)
				continue;

			file->readByte(); // color/control byte
			Common::String message;
			int chars = 0;
			while (chars < size) {
				byte c = file->readByte();
				if (c <= 1 || c < 0x20 || c > 0xf0)
					continue;
				message += c;
				chars++;
			}

			debugC(1, kFreescapeDebugParser, "'%s' with offset: %d, %d", message.c_str(), x, y);
			riddle._lines.push_back(RiddleText(x, y, message));
		}

		_riddleList.push_back(riddle);
	}

	debugC(1, kFreescapeDebugParser, "End of C64 riddles at %" PRIx64, file->pos());
}

void CastleEngine::loadAssetsC64FullGame() {
	Common::File file;
	file.open("castlemaster.c64.data");

	if (!file.isOpen())
		error("Failed to open castlemaster.c64.data");

	// The original tape loader preloads display support into high RAM before
	// this main program image starts; castlemaster.c64.data has no standalone
	// font block like the other C64 Freescape games.
	Common::Array<Graphics::ManagedSurface *> chars = loadCastleC64Font();
	_font = Font(chars);
	_font.setCharWidth(8);
	_fontLoaded = true;

	loadMessagesC64(&file, 0x13a9, 75);
	loadRiddlesC64(&file, 0x1823, 9);
	Common::Array<byte> database = normalizeCastleC64Database(&file);
	CastleC64DatabaseReadStream databaseStream(database);
	load8bitBinary(&databaseStream, 0, 16);

	for (uint i = 0; i < database[0]; i++) {
		uint16 areaOffset = readCastleC64Uint16LE(database, kCastleC64RuntimeAreaTableOffset + 2 * i);
		if (areaOffset + 6 >= database.size())
			error("Castle C64 area color read out of range at 0x%x", areaOffset);

		byte areaID = database[areaOffset + 2];
		if (!_areaMap.contains(areaID))
			continue;

		Area *area = _areaMap[areaID];
		// Original C64 code sets the multicolor bitmap colors as follows:
		// $2435 -> $d020/$d021, $2436 -> screen high nibble,
		// area byte +6 -> screen low nibble, and $2438 -> color RAM.
		area->_usualBackgroundColor = kCastleC64BackgroundColor;
		area->_underFireBackgroundColor = kCastleC64ScreenHighNibbleColor;
		area->_paperColor = database[areaOffset + 6] & 0x0f;
		area->_inkColor = kCastleC64ColorRamColor;
		debugC(1, kFreescapeDebugParser, "Castle C64 area %d colors: background=%d screen1=%d screen2=%d colorRAM=%d", areaID, area->_usualBackgroundColor, area->_underFireBackgroundColor, area->_paperColor, area->_inkColor);
	}

	Graphics::Surface *surf = loadBundledImage("castle_border");
	surf->convertToInPlace(_gfx->_texturePixelFormat);
	_border = new Graphics::ManagedSurface();
	if (surf->w == _screenW && surf->h == _screenH) {
		_border->copyFrom(*surf);
	} else {
		Common::Rect borderRect(kCastleC64BorderCropLeft, kCastleC64BorderCropTop, kCastleC64BorderCropLeft + _screenW, kCastleC64BorderCropTop + _screenH);
		if (surf->w < borderRect.right || surf->h < borderRect.bottom)
			error("Castle C64 border has unsupported dimensions %dx%d", surf->w, surf->h);

		_border->create(_screenW, _screenH, _gfx->_texturePixelFormat);
		_border->copyRectToSurface(*surf, 0, 0, borderRect);
	}
	surf->free();
	delete surf;

	_playerMusic = new CastleC64MusicPlayer();

	// TODO: title screen is in BASIC loader (file 009) - not yet extracted
}

void CastleEngine::drawC64UI(Graphics::Surface *surface) {
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x62, 0xD5, 0x32);

	uint8 r, g, b;
	_gfx->readFromPalette(0, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	// The original loader leaves "CASTLE MASTER" in the bottom message strip.
	// Clear the whole writable part of that strip before drawing runtime text.
	Common::Rect backRect(kCastleC64MessageLeft, 181, kCastleC64MessageRight, 192);
	surface->fillRect(backRect, back);

	Common::String message;
	int deadline = -1;
	getLatestMessages(message, deadline);
	if (deadline > 0 && deadline <= _countdown) {
		drawStringInSurface(message, kCastleC64MessageX, kCastleC64MessageY, front, back, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (_gameStateControl == kFreescapeGameStatePlaying) {
		if (ghostInArea() && !_ghostInAreaMessage.empty()) {
			drawStringInSurface(_ghostInAreaMessage, kCastleC64MessageX, kCastleC64MessageY, front, back, surface);
		} else {
			Common::String areaName = _currentArea->_name;
			uint areaMessageIndex = 16 + _currentArea->getAreaID();
			if (areaMessageIndex < _messagesList.size())
				areaName = _messagesList[areaMessageIndex];
			drawStringInSurface(areaName, kCastleC64MessageX, kCastleC64MessageY, front, back, surface);
		}
	}
}

} // End of namespace Freescape
