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
#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/dark/dark.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/wb.h"

namespace Freescape {

// Code lengths and their bases, for the three escape coded fields of the packer
// below. A field is read again with the next, shorter code whenever it comes
// out with every bit set.
const byte kAtariPackLiteralBits[4] = { 0x0a, 0x03, 0x02, 0x02 };
const byte kAtariPackLiteralBase[4] = { 0x0e, 0x07, 0x04, 0x01 };
const byte kAtariPackLengthBits[5] = { 0x0a, 0x02, 0x01, 0x00, 0x00 };
const byte kAtariPackLengthBase[5] = { 0x0a, 0x06, 0x04, 0x03, 0x02 };
const byte kAtariPackOffsetBits[3] = { 0x0b, 0x04, 0x07 };
const uint16 kAtariPackOffsetBase[3] = { 0x0120, 0x0000, 0x0020 };

// The bit stream is read backwards, one byte at a time, each byte carrying a
// set bit below its data to mark where it ends.
struct AtariPackReader {
	const byte *data;
	uint32 pos;
	byte bits;

	byte getBit() {
		byte bit = (bits >> 7) & 1;
		bits = (bits << 1) & 0xff;
		if (bits == 0) {
			byte next = data[--pos];
			bits = ((next << 1) | bit) & 0xff;
			bit = (next >> 7) & 1;
		}
		return bit;
	}

	uint16 readBits(int count) {
		uint16 value = 0;
		while (count-- > 0)
			value = (value << 1) | getBit();
		return value;
	}

	// Several codes are introduced by the number of set bits before them
	int countOnes(int start) {
		while (getBit()) {
			if (--start < 0)
				break;
		}
		return start + 1;
	}
};

// Expand an Atari ST executable packed with the "****" packer, which some
// releases ship instead of the plain program: its TEXT segment holds a small
// loader and its DATA segment the packed stream. Ported from that loader; the
// stream expands backwards, from its end towards its start, and so does the
// output. Returns nullptr when the file is not packed, so it can be used as is.
Common::SeekableReadStream *depackAtariExecutable(Common::SeekableReadStream *file) {
	uint32 size = file->size();
	byte *data = (byte *)malloc(size);
	file->seek(0);
	file->read(data, size);

	uint32 stream = 0x1c + READ_BE_UINT32(data + 2);
	if (size < 0x1c || stream + 12 > size || READ_BE_UINT32(data + stream) != 0x2a2a2a2a) {
		free(data);
		return nullptr;
	}

	uint32 base = stream + 4;
	uint32 unpackedSize = READ_BE_UINT32(data + base);
	uint32 packedSize = READ_BE_UINT32(data + base + 4);
	if (!unpackedSize || base + 4 + packedSize > size || packedSize < 12) {
		free(data);
		return nullptr;
	}

	AtariPackReader in;
	in.data = data;
	in.pos = base + 4 + packedSize - 6;
	if ((int16)READ_BE_UINT16(data + in.pos) < 0)
		in.pos--;
	in.bits = data[--in.pos];

	byte *out = (byte *)malloc(unpackedSize);
	uint32 dst = unpackedSize;

	while (in.pos > base && dst > 0) {
		if (in.getBit()) {
			uint16 count = 0;
			if (in.getBit()) {
				int i = 3;
				while (true) {
					count = in.readBits(kAtariPackLiteralBits[i]);
					if (i == 0 || count != (1 << kAtariPackLiteralBits[i]) - 1)
						break;
					i--;
				}
				count += kAtariPackLiteralBase[i];
			}
			if (count + 1 > dst || in.pos < count + 1)
				break;
			for (uint16 i = 0; i <= count; i++)
				out[--dst] = data[--in.pos];
		}

		if (in.pos <= base + 8)
			break;

		int i = in.countOnes(3);
		uint16 length = kAtariPackLengthBase[i];
		if (kAtariPackLengthBits[i])
			length += in.readBits(kAtariPackLengthBits[i]);

		uint16 offset;
		if (length == 2) {
			// short matches carry their offset in a code of their own
			offset = in.getBit() ? in.readBits(9) + 0x40 : in.readBits(6);
		} else {
			int j = in.countOnes(1);
			offset = in.readBits(kAtariPackOffsetBits[j] + 1) + kAtariPackOffsetBase[j];
		}

		uint32 src = dst + offset + length;
		if (length > dst || src > unpackedSize)
			break;
		for (uint16 n = 0; n < length; n++)
			out[--dst] = out[--src];
	}

	free(data);
	if (dst != 0) {
		warning("The packed Atari executable expanded to %d bytes short of %d",
			dst, unpackedSize);
		free(out);
		return nullptr;
	}

	return new Common::MemoryReadStream(out, unpackedSize, DisposeAfterUse::YES);
}

// The Stampede cover disk ships 0.DRK packed; every other release ships it as
// the plain program, and expanding it gives back the same executable.
Common::SeekableReadStream *DarkEngine::openAtariExecutable() {
	Common::File *file = new Common::File();
	if (!file->open("0.drk"))
		error("Failed to open 0.drk");

	Common::SeekableReadStream *depacked = depackAtariExecutable(file);
	if (!depacked)
		return file;

	delete file;
	return depacked;
}

void DarkEngine::loadAssetsAtariFullGame() {
	Common::SeekableReadStream *executable = openAtariExecutable();
	_title = loadAndConvertNeoImage(executable, 0x13ec);

	// Atari ST Dark Side: same COLOR5 cycling as Amiga.
	{
		static const uint16 kDarkSideCyclingTable[] = {
			0x000, 0xE6D, 0x600, 0x900, 0xC00, 0xF00, 0xF30, 0xF60,
			0xF90, 0xFC0, 0xFF0, 0xAF0, 0x5F0, 0x6F8, 0x7FD, 0x7EF,
			0xBDF, 0xDDF, 0xBCF, 0x9BF, 0x7BF, 0x6BF, 0x5AF, 0x4AF,
			0x29F, 0x18F, 0x07F, 0x04C, 0x02A, 0x007
		};
		for (int i = 0; i < 30; i++)
			_gfx->_colorCyclingTable.push_back(kDarkSideCyclingTable[i]);
	}
	_gfx->_colorCyclingPaletteIndex = 5;
	_gfx->_colorCyclingSpeed = 1;
	_gfx->_colorCyclingTimer = 0;

	// same array, ending at program address $132E, i.e. 0x134A in the file
	Common::SeekableReadStream *stream = decryptFileAmigaAtari("1.drk", executable, 840);
	delete executable;
	parseAmigaAtariHeader(stream);

	_border = loadAndConvertNeoImage(stream, 0xd710);

	Common::Array<Graphics::ManagedSurface *> chars;
	chars = getCharsAmigaAtariInternal(8, 8, - 7 - 8, 16, 16, stream, 0xd06a, 85);
	_fontBig = Font(chars);

	chars = getCharsAmigaAtariInternal(8, 8, 0, 10, 8, stream, 0xd49a, 85);
	_fontMedium = Font(chars);

	chars = getCharsAmigaAtariInternal(8, 5, - 7 - 8, 10, 16, stream, 0xd49a, 85);
	_fontSmall = Font(chars);
	_fontSmall.setCharWidth(4);

	_fontLoaded = true;
	load8bitBinary(stream, 0x20918, 16);
	loadMessagesVariableSize(stream, 0x3f6f, 66);
	loadPalettes(stream, 0x204d6);
	loadGlobalObjects(stream, 0x32f6, 24);
	_sound = loadSoundsFx(stream, 0x266e8, 11);

	// DSMUSIC2.ST is an embedded GEMDOS executable, listed in the file table
	// at stream offset $60.
	{
		const uint32 kDsMusicOffset = 0xBCA2;
		const uint32 kGemdosHeaderSize = 0x1C;
		const uint32 kDsMusicTextSize = 0x1246;

		stream->seek(kDsMusicOffset + kGemdosHeaderSize);
		_musicData.resize(kDsMusicTextSize);
		stream->read(_musicData.data(), kDsMusicTextSize);

		delete _playerMusic;
		_playerMusic = makeWallyBebenAtariPlayer(_musicData.data(), _musicData.size(),
			kDarkSideAtariOffsets);
	}

	byte *palette = getPaletteFromNeoImage(stream, 0xd710);
	loadAmigaCompass(stream, palette, kAtariSpriteDelta);
	loadAmigaIndicatorSprites(stream, palette, kAtariSpriteDelta);
	loadJetpackRawFrames(stream, kAtariSpriteDelta);
	free(palette);

	for (auto &area : _areaMap) {
		// Center and pad each area name so we do not have to do it at each frame
		area._value->_name = centerAndPadString(area._value->_name, 26);
	}
}

} // End of namespace Freescape
