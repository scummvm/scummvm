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
#include "common/memstream.h"
#include "common/random.h"
#include "graphics/managed_surface.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/c64.music.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/variables.h"

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
	kCastleC64MessageY = 182,
	kCastleC64GateFrameTicks = 2,
	kCastleC64GateLiftStep = 2,
	kCastleC64GateTransparent = 4,
	kCastleC64ThunderClockTicks = 60,
	kCastleC64LightningIdle = 0,
	kCastleC64LightningBolt = 1,
	kCastleC64LightningFlash = 2
};

// Match the colors of the bundled C64 border. The unused palette entries are
// black; the HUD uses only these ten VIC colors.
static const byte kCastleC64UIPalette[16][3] = {
	{0, 0, 0}, {255, 255, 255}, {0, 0, 0}, {0, 0, 0},
	{0, 0, 0}, {98, 213, 50}, {0, 0, 0}, {255, 255, 70},
	{183, 99, 30}, {119, 83, 0}, {0, 0, 0}, {98, 98, 98},
	{148, 148, 148}, {183, 255, 134}, {0, 0, 0}, {205, 205, 205}
};

static uint32 castleC64UIColor(const Graphics::PixelFormat &format, byte color) {
	const byte *rgb = kCastleC64UIPalette[color];
	return format.ARGBToColor(255, rgb[0], rgb[1], rgb[2]);
}

static Common::Array<byte> unpackCastleC64UI(Common::SeekableReadStream *file) {
	// The startup relocates the packed stream from $0d50 to $2708, then
	// expands it into $0200..$ffff. Decode only through the screen attributes
	// at $c400..$c7e7: the later bitmap pages require the separate tape loader.
	// Page flags at $09ff descend through memory, least significant bit first.
	// A clear bit selects a page with its own escape byte and count/value runs.
	Common::Array<byte> packed;
	packed.resize(file->size());
	file->seek(0);
	if (packed.size() < 0x551 || file->read(packed.data(), packed.size()) != packed.size())
		error("Unable to read Castle C64 UI data");

	Common::Array<byte> data;
	data.resize(0xc800);
	uint32 source = 0x551; // $0d50, including the PRG load-address adjustment
	int flagOffset = 0x200; // $09ff
	byte flags = packed[flagOffset];
	int bitsLeft = 6; // The first two pages are not part of the packed stream.
	for (uint page = 2; page < 0xc8; page++) {
		if (!bitsLeft) {
			flags = packed[--flagOffset];
			bitsLeft = 8;
		}
		bool raw = flags & 1;
		flags >>= 1;
		bitsLeft--;
		uint end = (page + 1) * 256;
		if (source >= packed.size())
			error("Truncated Castle C64 UI page %x", page);
		byte escape = raw ? 0 : packed[source++];
		for (uint dest = page * 256; dest < end;) {
			if (source >= packed.size())
				error("Truncated Castle C64 UI page %x", page);
			byte value = packed[source++];
			uint count = 1;
			if (!raw && value == escape) {
				if (source + 2 > packed.size())
					error("Truncated Castle C64 UI run");
				count = packed[source++];
				if (!count)
					count = 256;
				value = packed[source++];
			}
			if (count > end - dest)
				error("Castle C64 UI run crosses a page boundary");
			while (count--)
				data[dest++] = value;
		}
	}
	return data;
}

static void loadCastleC64Bitmap(const Common::Array<byte> &data, uint address, uint width, uint height, Graphics::ManagedSurface *surface) {
	if (!width || !height || address + width * height > data.size())
		error("Invalid Castle C64 bitmap at %x", address);
	surface->create(width * 8, height, Graphics::PixelFormat::createFormatCLUT8());
	for (uint y = 0; y < height; y++) {
		for (uint x = 0; x < width * 8; x += 2) {
			byte color = (data[address + y * width + x / 8] >> (6 - x % 8)) & 3;
			surface->setPixel(x, y, color);
			surface->setPixel(x + 1, y, color);
		}
	}
}

static void loadCastleC64Frame(const Common::Array<byte> &data, uint address, Graphics::ManagedSurface *surface, int frame = 0) {
	// $7cf6 reads a five-byte header: byte width, height, final-byte mask,
	// and frame size. These HUD frames all use the whole final byte.
	if (address + 5 > data.size())
		error("Missing Castle C64 UI frame header at %x", address);
	uint width = data[address];
	uint height = data[address + 1];
	uint size = data[address + 3] | (data[address + 4] << 8);
	uint pixels = address + 5 + frame * size;
	if (!width || !height || data[address + 2] != 0xff || size != width * height || pixels + size > data.size())
		error("Invalid Castle C64 UI frame at %x", address);
	loadCastleC64Bitmap(data, pixels, width, height, surface);
}

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

static Common::Array<Graphics::ManagedSurface *> loadCastleC64Font(const Common::Array<byte> &data) {
	Common::Array<Graphics::ManagedSurface *> chars;

	// $85eb expands four packed bytes per character into eight rows of
	// double-width pixels. Row 1 uses color RAM for the highlight.
	for (uint chr = 32; chr < 128; chr++) {
		Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
		surface->create(10, 8, Graphics::PixelFormat::createFormatCLUT8());
		surface->clear(0);
		uint glyph = (chr >= 'a' && chr <= 'z') ? chr - 'a' + 'A' : chr;
		if (glyph <= 'Z') {
			for (int y = 0; y < 8; y++) {
				byte row = data[0x231a + (glyph - 32) * 4 + y / 2];
				row = (y & 1) ? row & 15 : row >> 4;
				for (int x = 0; x < 4; x++) {
					if (row & (8 >> x)) {
						surface->setPixel(x * 2, y, y == 1 ? 2 : 1);
						surface->setPixel(x * 2 + 1, y, y == 1 ? 2 : 1);
					}
				}
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
	_c64LiftingGateStartTicks = -1;
	_c64MusicEnabled = true;
	_c64SpiritAttackStartTicks = -1;
	resetC64Lightning();

	// C64 call sites: throw $63b3, climb/drop $53cf/$549f, area change
	// $6cde, and a damaging landing $8142. The gate supplies the start sound.
	_soundIndexShoot = 5;
	_soundIndexCollide = 3;
	_soundIndexStepUp = 12;
	_soundIndexStepDown = 12;
	_soundIndexMenu = 3;
	_soundIndexFallen = 8;
	_soundIndexStart = -1;
	_soundIndexAreaChange = 7;
}

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
		riddle._origin = Common::Point(40, 32);
		int previousInset = 0;

		int numberLines = file->readByte();
		debugC(1, kFreescapeDebugParser, "c64 riddle %d number of lines: %d", i, numberLines);

		for (int j = 0; j < numberLines; j++) {
			int8 x = 2 * (int8)file->readByte(); // C64 horizontal offsets count pixel pairs.
			int8 y = (int8)file->readByte();
			int size = file->readByte();

			// $6dd6 applies both deltas to every record, including the rows
			// of asterisks. Discarding those rows also loses the text origin.
			if (size == 0xff) {
				riddle._lines.push_back(RiddleText(x + 6 - previousInset, y, "********************"));
				previousInset = 6;
				continue;
			}

			int inset = file->readByte() ? 6 : 0;
			if (inset)
				size--;
			if (size < 0 || file->pos() + size > file->size())
				error("Truncated Castle C64 riddle %d", i);
			Common::String message;
			for (int chars = 0; chars < size; chars++)
				message += file->readByte();

			debugC(1, kFreescapeDebugParser, "'%s' with offset: %d, %d", message.c_str(), x, y);
			riddle._lines.push_back(RiddleText(x + inset - previousInset, y, message));
			previousInset = inset;
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

	Common::Array<byte> uiData = unpackCastleC64UI(&file);
	Common::MemoryReadStream uiStream(uiData.data(), uiData.size());
	Common::Array<Graphics::ManagedSurface *> chars = loadCastleC64Font(uiData);
	_font = Font(chars);
	_font.setCharWidth(10);
	_font.setSecondaryColor(castleC64UIColor(_gfx->_texturePixelFormat, 15));
	_fontLoaded = true;

	loadMessagesC64(&uiStream, 0x1401, 75);
	loadRiddlesC64(&uiStream, 0x18ae, 9);

	// $7403 selects the screen-RAM colour pairs at counter values 8 and 4.
	_c64SpiritAttackColors[0] = uiData[0x735c];
	_c64SpiritAttackColors[1] = uiData[0x735b];

	// $4d06 tiles sixteen bytes per row; $4ddd overlays a single 85-row
	// bolt. Both bitmaps use VIC multicolor pixel pairs, without headers.
	loadCastleC64Bitmap(uiData, 0x21fa, 16, 18, &_c64MountainBackground);
	loadCastleC64Bitmap(uiData, 0x20f8, 3, 85, &_c64Lightning);

	// Preserve multicolor pixel indices until drawing. VIC colors depend on
	// the destination 8x8 cell, even within a single moving weight or key.
	file.seek(0x301); // Packed color RAM at $0b00, high nibble first.
	_c64UIColors.resize(1000 * 4);
	for (int cell = 0; cell < 1000; cell += 2) {
		byte colorRAM = file.readByte();
		for (int i = 0; i < 2; i++) {
			byte screen = uiData[0xc400 + cell + i];
			byte colors[4] = {0, byte(screen >> 4), byte(screen & 15), byte(i ? colorRAM & 15 : colorRAM >> 4)};
			for (int color = 0; color < 4; color++)
				_c64UIColors[(cell + i) * 4 + color] = castleC64UIColor(_gfx->_texturePixelFormat, colors[color]);
		}
	}

	loadCastleC64Frame(uiData, 0x1dd9, &_c64KeysBackground);
	_keysBorderFrames.push_back(new Graphics::ManagedSurface());
	loadCastleC64Frame(uiData, 0x1e32, _keysBorderFrames[0]);
	_spiritsMeterIndicatorBackgroundFrame = new Graphics::ManagedSurface();
	loadCastleC64Frame(uiData, 0x1e45, _spiritsMeterIndicatorBackgroundFrame);
	_spiritsMeterIndicatorFrame = new Graphics::ManagedSurface();
	loadCastleC64Frame(uiData, 0x1e8a, _spiritsMeterIndicatorFrame);
	_strenghtBackgroundFrame = new Graphics::ManagedSurface();
	loadCastleC64Frame(uiData, 0x1e9f, _strenghtBackgroundFrame);
	_strenghtBarFrame = new Graphics::ManagedSurface();
	loadCastleC64Frame(uiData, 0x1f49, _strenghtBarFrame);
	for (int frame = 0; frame < 4; frame++) {
		_strenghtWeightsFrames.push_back(new Graphics::ManagedSurface());
		loadCastleC64Frame(uiData, 0x1f6f, _strenghtWeightsFrames[frame], frame);
	}

	// $8367 draws ten 24-pixel columns. Four-row crossbars repeat every
	// 24 rows, with fifteen rows of vertical bars below the lowest crossbar.
	// Crossbars overwrite all pixels, including pen 0; between them only
	// the leftmost and rightmost pixel pairs cover the scene ($84a4).
	_c64Gate.create(240, 120, Graphics::PixelFormat::createFormatCLUT8());
	_c64Gate.fillRect(Common::Rect(240, 120), kCastleC64GateTransparent);
	for (int y = 0; y < _c64Gate.h; y++) {
		int fromBottom = _c64Gate.h - 1 - y;
		if (fromBottom < 15 || (fromBottom - 15) % 24 >= 4) {
			for (int x = 0; x < _c64Gate.w; x += 24) {
				_c64Gate.fillRect(Common::Rect(x, y, x + 2, y + 1), 1);
				_c64Gate.fillRect(Common::Rect(x + 22, y, x + 24, y + 1), 3);
			}
			continue;
		}
		int row = 3 - (fromBottom - 15) % 24;
		for (int x = 0; x < _c64Gate.w; x += 2) {
			byte pixels = uiData[0x82d7 + row * 3 + (x / 8) % 3];
			byte color = (pixels >> (6 - x % 8)) & 3;
			_c64Gate.setPixel(x, y, color);
			_c64Gate.setPixel(x + 1, y, color);
		}
	}

	// $82e3 uses twenty heights, followed by $ff, to accelerate the fall
	// and bounce twice after landing. Each step waits for two timer ticks.
	_c64GateDropHeights.clear();
	for (int frame = 0; frame < 20; frame++)
		_c64GateDropHeights.push_back(uiData[0x82c2 + frame]);

	// $6ee7 stretches seven three-byte rows across the riddle board. The
	// bottom reverses the top six rows; row six fills the middle of the board.
	Graphics::ManagedSurface *riddleFrames[3];
	const byte riddleColors[4] = {0, 9, 7, 9};
	for (int frame = 0; frame < 3; frame++) {
		riddleFrames[frame] = new Graphics::ManagedSurface();
		int height = frame == 1 ? 1 : 6;
		riddleFrames[frame]->create(240, height, _gfx->_texturePixelFormat);
		for (int y = 0; y < height; y++) {
			int row = frame == 0 ? y : (frame == 1 ? 6 : 5 - y);
			for (int x = 0; x < 240; x += 2) {
				int column = x < 8 ? 0 : (x >= 232 ? 2 : 1);
				byte pixels = uiData[0x2023 + row * 3 + column];
				byte color = riddleColors[(pixels >> (6 - x % 8)) & 3];
				uint32 pixel = castleC64UIColor(_gfx->_texturePixelFormat, color);
				riddleFrames[frame]->setPixel(x, y, pixel);
				riddleFrames[frame]->setPixel(x + 1, y, pixel);
			}
		}
	}
	_riddleTopFrame = riddleFrames[0];
	_riddleBackgroundFrame = riddleFrames[1];
	_riddleBottomFrame = riddleFrames[2];

	// The IRQ at $74be advances the flag every eight PAL ticks. Its frames
	// are six bitmap cells in C64 cell order, copied to $e128 and $e268.
	for (int frame = 0; frame < 4; frame++) {
		Graphics::ManagedSurface *flag = new Graphics::ManagedSurface();
		flag->create(24, 16, Graphics::PixelFormat::createFormatCLUT8());
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 24; x += 2) {
				byte pixels = uiData[0x2038 + frame * 48 + (y / 8) * 24 + (x / 8) * 8 + y % 8];
				byte color = (pixels >> (6 - x % 8)) & 3;
				flag->setPixel(x, y, color);
				flag->setPixel(x + 1, y, color);
			}
		}
		_flagFrames.push_back(flag);
	}

	Common::Array<byte> database = normalizeCastleC64Database(&file);
	CastleC64DatabaseReadStream databaseStream(database);
	load8bitBinary(&databaseStream, 0, 16);

	for (uint i = 0; i < database[0]; i++) {
		uint16 areaOffset = readCastleC64Uint16LE(database, kCastleC64RuntimeAreaTableOffset + 2 * i);
		if (areaOffset + 6u >= database.size())
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

	_sound = createCastleC64Sound(_mixer, uiData);
	_playerMusic = new CastleC64MusicPlayer(_mixer);

	// TODO: title screen is in BASIC loader (file 009) - not yet extracted
}

void CastleEngine::drawC64HudSurface(Graphics::Surface *surface, const Graphics::Surface &frame, const Common::Point &origin) {
	for (int y = 0; y < frame.h; y++) {
		const byte *src = (const byte *)frame.getBasePtr(0, y);
		for (int x = 0; x < frame.w; x++) {
			int cell = ((origin.y + y) / 8) * 40 + (origin.x + x) / 8;
			surface->setPixel(origin.x + x, origin.y + y, _c64UIColors[cell * 4 + src[x]]);
		}
	}
}

void CastleEngine::drawC64InfoMenu(Graphics::Surface *surface) {
	uint32 front = castleC64UIColor(surface->format, 9);
	uint32 highlight = castleC64UIColor(surface->format, 15);
	uint32 back = castleC64UIColor(surface->format, 0);
	surface->fillRect(_viewArea, back);
	Common::String keys = _messagesList[72];
	Common::String spirits = _messagesList[73];
	Common::String score = _messagesList[74];
	Common::replace(keys, "XX", Common::String::format("%2d", MIN<uint>(_keysCollected.size(), 10)));
	Common::replace(spirits, "XX", Common::String::format("%2d", _gameStateVars[k8bitVariableSpiritsDestroyed]));
	Common::replace(score, "XXXXXXX", Common::String::format("%07d", _gameStateVars[k8bitVariableScore]));

	// The original menu at $7808 uses these rows within the 3D viewport.
	drawStringInSurface("********************", 60, 46, front, highlight, back, surface);
	drawStringInSurface(_messagesList[68], 50, 61, front, highlight, back, surface);
	drawStringInSurface(_c64MusicEnabled ? "F-AUDIO: MUSIC" : "F-AUDIO: EFFECTS", 50, 72, front, highlight, back, surface);
	drawStringInSurface(_messagesList[69], 50, 82, front, highlight, back, surface);
	drawStringInSurface(keys, 130, 82, front, highlight, back, surface);
	drawStringInSurface(_messagesList[70], 50, 93, front, highlight, back, surface);
	drawStringInSurface(spirits, 130, 93, front, highlight, back, surface);
	drawStringInSurface(_messagesList[71], 50, 104, front, highlight, back, surface);
	int strength = CLIP<int>(_gameStateVars[k8bitVariableShield], 1, 24);
	drawStringInSurface(_messagesList[62 + (strength - 1) / 4], 150, 104, front, highlight, back, surface);
	drawStringInSurface(score, 80, 115, front, highlight, back, surface);
	drawStringInSurface("********************", 60, 133, front, highlight, back, surface);
}

void CastleEngine::toggleC64AudioMode() {
	_c64MusicEnabled = !_c64MusicEnabled;
	_syncSound = false;
	// $79bf switches between music and effects. Stop and release the old
	// SID first, since both modes use all three voices of the same chip.
	if (_c64MusicEnabled) {
		enableCastleC64Sound(_sound, false);
		if (_playerMusic)
			_playerMusic->startMusic();
	} else {
		if (_playerMusic)
			_playerMusic->stopMusic();
		enableCastleC64Sound(_sound, true);
		if (_sound)
			_sound->playSound(3, Sound::kTypeNormal);
	}
}

void CastleEngine::updateC64SpiritPalette() {
	int screenHigh = _currentArea->_underFireBackgroundColor;
	int screenLow = _currentArea->_paperColor;
	if (_gameStateControl == kFreescapeGameStatePlaying && !isPaused() && !_disableSensors && ghostInArea()) {
		int ticks = _ticks;
		if (_c64SpiritAttackStartTicks < 0)
			_c64SpiritAttackStartTicks = ticks;

		// $73cb-$742d counts down from ten at 50 Hz: two ticks of the
		// area colours, four of $78, four of $82, then repeat. Only the
		// viewport's screen RAM changes; background and colour RAM do not.
		int phase = (ticks - _c64SpiritAttackStartTicks) % 10;
		if (phase >= 2) {
			byte colors = _c64SpiritAttackColors[phase < 6 ? 0 : 1];
			screenHigh = colors >> 4;
			screenLow = colors & 15;
		}
	} else {
		_c64SpiritAttackStartTicks = -1;
	}

	if (_gfx->_underFireBackgroundColor == screenHigh && _gfx->_paperColor == screenLow)
		return;
	_gfx->_underFireBackgroundColor = screenHigh;
	_gfx->_paperColor = screenLow;
	updateC64BackgroundPalette();
}

void CastleEngine::updateC64BackgroundPalette() {
	uint32 colors[4];
	for (int color = 0; color < 4; color++) {
		uint8 r, g, b;
		_gfx->selectColorFromFourColorPalette(color, r, g, b);
		// Pen 0 is transparent when compositing the lightning bitmap.
		colors[color] = _gfx->_texturePixelFormat.ARGBToColor(color ? 255 : 0, r, g, b);
	}
	if (!_background)
		_background = new Graphics::ManagedSurface();
	if (_thunderFrames.empty())
		_thunderFrames.push_back(new Graphics::ManagedSurface());

	const Graphics::Surface *sources[] = {&_c64MountainBackground.rawSurface(), &_c64Lightning.rawSurface()};
	Graphics::ManagedSurface *destinations[] = {_background, _thunderFrames[0]};
	for (uint frame = 0; frame < ARRAYSIZE(sources); frame++) {
		const Graphics::Surface &src = *sources[frame];
		Graphics::ManagedSurface *dst = destinations[frame];
		dst->create(src.w, src.h, _gfx->_texturePixelFormat);
		for (int y = 0; y < src.h; y++) {
			const byte *pixels = (const byte *)src.getBasePtr(0, y);
			for (int x = 0; x < src.w; x++)
				dst->setPixel(x, y, colors[pixels[x]]);
		}
	}

	delete _skyTexture;
	_skyTexture = nullptr;
	for (auto *texture : _thunderTextures)
		delete texture;
	_thunderTextures.clear();
}

void CastleEngine::resetC64Lightning() {
	_c64NextLightningTicks = -1;
	_c64LightningPhase = kCastleC64LightningIdle;
	_c64LightningPhaseTicks = 0;
	_c64LightningX = 0;
}

void CastleEngine::updateC64Lightning() {
	int ticks = _ticks;
	if (_gameStateControl != kFreescapeGameStatePlaying) {
		resetC64Lightning();
		return;
	}

	// $7531 decrements the initial counter of 5 every 60 PAL ticks,
	// stopping at 1 until the main loop draws the bolt ($4cb8).
	if (_c64NextLightningTicks < 0)
		_c64NextLightningTicks = ticks + 4 * kCastleC64ThunderClockTicks;
	if (_c64LightningPhase == kCastleC64LightningIdle && ticks >= _c64NextLightningTicks) {
		// $483e reloads from the low six timer bits plus ten; the next
		// bolt appears when that counter reaches 1.
		_c64NextLightningTicks = ticks + (9 + _rnd->getRandomNumber(63)) * kCastleC64ThunderClockTicks;
		if (_currentArea->isOutside() && !_avoidRenderingFrames) {
			_c64LightningPhase = kCastleC64LightningBolt;
			_c64LightningPhaseTicks = ticks + 1;
			// $4d90 chooses one of 27 byte-aligned positions in the viewport.
			_c64LightningX = 8 * (1 + _rnd->getRandomNumber(26));
		}
	} else if (_c64LightningPhase != kCastleC64LightningIdle && ticks >= _c64LightningPhaseTicks) {
		if (_c64LightningPhase == kCastleC64LightningBolt) {
			_c64LightningPhase = kCastleC64LightningFlash;
			_c64LightningPhaseTicks = ticks + 1;
			// $484d plays sound 8 and flashes the background for one PAL tick.
			// A rendering callback must not enter the scripted SOUND wait loop.
			if (_sound && _currentArea->isOutside())
				_sound->playSound(8, Sound::kTypeNormal);
		} else {
			_c64LightningPhase = kCastleC64LightningIdle;
		}
	}
	if (!_currentArea->isOutside())
		_c64LightningPhase = kCastleC64LightningIdle;
}

void CastleEngine::drawC64Background() {
	updateC64SpiritPalette();
	updateC64Lightning();
	clearBackground();
	_gfx->drawBackground(_currentArea->_skyColor);
	if (_avoidRenderingFrames || !_currentArea->isOutside())
		return;

	// Use the same perspective skybox as the other Castle releases. Center
	// it on the camera so movement does not introduce foreground parallax.
	Math::Vector3d camera = _inWaitLoop ? _position : getCameraRenderPosition();
	if (_currentArea->getAreaID() == 1 && _background) {
		if (!_skyTexture)
			_skyTexture = _gfx->createTexture(_background->surfacePtr(), true);
		_gfx->drawSkybox(_skyTexture, camera);
	}

	if (_c64LightningPhase == kCastleC64LightningFlash)
		_gfx->clear(255, 255, 255);
	if (_c64LightningPhase != kCastleC64LightningBolt || _thunderFrames.empty())
		return;

	// $4c19/$4da5 place the bottom of the lightning bitmap ten rows above
	// the horizon. Project that horizon using the Castle viewport's FOV.
	float horizontal = sqrt(_cameraFront.x() * _cameraFront.x() + _cameraFront.z() * _cameraFront.z());
	if (horizontal < 0.001f)
		return;
	float focalLength = _viewArea.height() * 0.5f * 1.6f / tan(Math::deg2rad(75.0f) * 0.5f);
	float horizon = _viewArea.top + _viewArea.height() * 0.5f + focalLength * _cameraFront.y() / horizontal;
	if (horizon < _viewArea.top || horizon > _viewArea.bottom + 95)
		return;
	int horizonY = int(horizon) + 1;

	// Compose a full-screen layer: the shader renderer's 2D path does not
	// support partial source/destination rectangles. Clip before uploading.
	const Graphics::Surface &source = _thunderFrames[0]->rawSurface();
	int x = _viewArea.left + _c64LightningX;
	int y = horizonY - 95;
	Common::Rect dst(x, y, x + source.w, y + source.h);
	dst.clip(_viewArea);
	if (dst.isEmpty())
		return;
	Common::Rect src(dst.left - x, dst.top - y, dst.right - x, dst.bottom - y);
	Graphics::ManagedSurface lightning(_screenW, _screenH, _gfx->_texturePixelFormat);
	lightning.clear(0);
	lightning.copyRectToSurfaceWithKey(source, dst.left, dst.top, src, 0);
	if (_thunderTextures.empty())
		_thunderTextures.push_back(_gfx->createTexture(lightning.surfacePtr()));
	else
		_thunderTextures[0]->update(lightning.surfacePtr());
	_gfx->setViewport(_fullscreenViewArea);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _thunderTextures[0]);
	_gfx->setViewport(_viewArea);

	// The lightning blit changes the OpenGL matrices. Restore the camera
	// before drawing the 3D scene, which must occlude the background.
	_gfx->updateProjectionMatrix(75.0f, 1.6f, _nearClipPlane, _farClipPlane * 100);
	_gfx->positionCamera(camera, camera + _cameraFront, _roll);
}

void CastleEngine::liftC64Gate() {
	// $8347 raises the gate by two rows per step. Start the clock after the
	// initial area is ready so its setup does not consume animation time.
	_c64LiftingGateStartTicks = _ticks;
	for (int step = 0; step < _c64Gate.h / kCastleC64GateLiftStep && !shouldQuit(); step++) {
		int remaining = _c64LiftingGateStartTicks + (step + 1) * kCastleC64GateFrameTicks - _ticks;
		if (remaining <= 0)
			continue;
		// $834b retriggers the rattle on every step, then $8363 plays the
		// impact. Gate sounds interrupt effects without waiting for them.
		if (_sound)
			_sound->playSound(3, Sound::kTypeNormal);
		waitInLoop(remaining - 1);
	}
	if (_sound && !shouldQuit())
		_sound->playSound(2, Sound::kTypeNormal);
	_c64LiftingGateStartTicks = -1;
}

void CastleEngine::dropC64Gate() {
	// $49bb completes the fall before polling for a restart. The wait loop
	// consumes pending gameplay input while still allowing the user to quit.
	_droppingGateStartTicks = _ticks;
	for (uint frame = 0; frame < _c64GateDropHeights.size() && !shouldQuit(); frame++) {
		int remaining = _droppingGateStartTicks + (frame + 1) * kCastleC64GateFrameTicks - _ticks;
		if (remaining <= 0)
			continue;
		// $8300 plays an impact each time the gate reaches the ground.
		if (_sound && _c64GateDropHeights[frame] == _c64Gate.h)
			_sound->playSound(2, Sound::kTypeNormal);
		waitInLoop(remaining - 1);
	}
}

void CastleEngine::drawC64Gate(Graphics::Surface *surface) {
	int height;
	if ((_gameStateControl == kFreescapeGameStateStart || _gameStateControl == kFreescapeGameStateRestart) && _c64LiftingGateStartTicks >= 0) {
		int ticks = MAX(0, _ticks - _c64LiftingGateStartTicks);
		height = MAX(0, _c64Gate.h - (ticks / kCastleC64GateFrameTicks) * kCastleC64GateLiftStep);
	} else if (_gameStateControl == kFreescapeGameStateEnd && _droppingGateStartTicks >= 0 && !hasEscaped()) {
		int ticks = MAX(0, _ticks - _droppingGateStartTicks);
		int frame = MIN<int>(ticks / kCastleC64GateFrameTicks, _c64GateDropHeights.size() - 1);
		height = _c64GateDropHeights[frame];
	} else {
		return;
	}
	if (!height)
		return;

	// The gate is drawn into the viewport bitmap and uses its current VIC
	// colors, unlike the HUD frames whose colors come from the static border.
	uint32 colors[4];
	for (int color = 0; color < 4; color++) {
		uint8 r, g, b;
		_gfx->selectColorFromFourColorPalette(color, r, g, b);
		colors[color] = surface->format.ARGBToColor(255, r, g, b);
	}
	for (int y = 0; y < height; y++) {
		const byte *src = (const byte *)_c64Gate.getBasePtr(0, _c64Gate.h - height + y);
		for (int x = 0; x < _c64Gate.w; x++) {
			if (src[x] != kCastleC64GateTransparent)
				surface->setPixel(_viewArea.left + x, _viewArea.top + y, colors[src[x]]);
		}
	}
}

void CastleEngine::drawC64UI(Graphics::Surface *surface) {
	drawC64Gate(surface);

	uint32 front = castleC64UIColor(surface->format, 5);
	uint32 back = castleC64UIColor(surface->format, 0);
	_font.setSecondaryColor(castleC64UIColor(surface->format, 15));

	// $702c rebuilds the key rack on a full redraw, including after loading.
	// Composing it afresh also removes keys when restarting or loading a save.
	Graphics::ManagedSurface buffer(96, 19, Graphics::PixelFormat::createFormatCLUT8());
	buffer.copyRectToSurface(_c64KeysBackground, 0, 0, Common::Rect(48, 14));
	for (uint key = 0; key < MIN<uint>(_keysCollected.size(), 10); key++)
		buffer.copyRectToSurfaceWithKey(*_keysBorderFrames[0], 42 - 4 * key, 0, Common::Rect(8, 14), 0);
	drawC64HudSurface(surface, buffer.getSubArea(Common::Rect(48, 14)), Common::Point(48, 179));

	// $7171 draws paired discs from the outside inward, four pixels apart.
	// A partial disc precedes the full discs; strength below four lowers the
	// bar and both discs. Only the original 88x15 window is copied to the HUD.
	buffer.copyRectToSurface(*_strenghtBackgroundFrame, 0, 0, Common::Rect(88, 15));
	int strength = CLIP<int>(_gameStateVars[k8bitVariableShield], 0, 24);
	int drop = MAX(0, 4 - strength);
	buffer.copyRectToSurface(*_strenghtBarFrame, 6, 6 + drop, Common::Rect(88, 3));
	int pairs = (strength + 3) / 4;
	for (int pair = 0; pair < pairs; pair++) {
		int frame = (pair == 0 && strength % 4) ? 4 - strength % 4 : 0;
		buffer.copyRectToSurfaceWithKey(*_strenghtWeightsFrames[frame], 8 + pair * 4, drop, Common::Rect(8, 15), 0);
		buffer.copyRectToSurfaceWithKey(*_strenghtWeightsFrames[frame], 80 - pair * 4, drop, Common::Rect(8, 15), 0);
	}
	drawC64HudSurface(surface, buffer.getSubArea(Common::Rect(88, 15)), Common::Point(40, 158));

	// $726a rounds the spirit position up to a pixel pair, then clips the
	// moving face to the middle eight bitmap cells. Derive it from saved
	// state here so a loaded game does not display the previous position.
	int spiritsDestroyed = CLIP<int>(_gameStateVars[k8bitVariableSpiritsDestroyed], 0, _spiritsToKill);
	int position = CLIP<int>(_spiritsMeter * (_spiritsToKill - spiritsDestroyed) / _spiritsToKill, 0, 64);
	buffer.copyRectToSurface(*_spiritsMeterIndicatorBackgroundFrame, 8, 0, Common::Rect(64, 8));
	buffer.copyRectToSurfaceWithKey(*_spiritsMeterIndicatorFrame, (position + 1) & ~1, 0, Common::Rect(16, 8), 0);
	drawC64HudSurface(surface, buffer.getSubArea(Common::Rect(8, 0, 72, 8)), Common::Point(160, 161));

	int flagFrame = (g_system->getMillis() / 160) % 4;
	drawC64HudSurface(surface, *_flagFrames[flagFrame], Common::Point(296, 0));
	// TODO: animate the eye indicator using the frames at $1fb0.

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
