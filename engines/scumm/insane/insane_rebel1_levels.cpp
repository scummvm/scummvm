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

#include "common/system.h"
#include "common/endian.h"

#include "scumm/scumm_v7.h"
#include "scumm/file.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

// From smush/codec1.cpp
void smushDecodeRA1Transparent(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

static void decodeBomp(byte *dst, const byte *src, int width, int height, int pitch) {
	while (height--) {
		byte *dstNext = dst + pitch;
		const byte *srcNext = src + 2 + READ_LE_UINT16(src);
		src += 2;
		int len = width;
		byte *d = dst;
		do {
			int offs = READ_LE_UINT16(src); src += 2;
			d += offs;
			len -= offs;
			if (len <= 0)
				break;
			int w = READ_LE_UINT16(src) + 1; src += 2;
			len -= w;
			if (len < 0)
				w += len;
			memcpy(d, src, w);
			src += w;
			d += w;
		} while (len > 0);
		dst = dstNext;
		src = srcNext;
	}
}

// Load an RA1 NUT sprite file (ANIM v1).
// RA1 NUTs can have odd-size FOBJ chunks padded to 2-byte alignment within
// FRME containers. This loader handles that padding properly, unlike the
// shared NutRenderer::loadFont which assumes even-size chunks.
bool InsaneRebel1::loadRA1Nut(const char *filename, RA1SpriteBank &bank) {
	ScummFile *file = _vm->instantiateScummFile();
	_vm->openFile(*file, filename);
	if (!file->isOpen()) {
		warning("InsaneRebel1::loadRA1Nut: can't open %s", filename);
		delete file;
		return false;
	}

	uint32 tag = file->readUint32BE();
	if (tag != MKTAG('A','N','I','M')) {
		warning("InsaneRebel1::loadRA1Nut: no ANIM tag in %s", filename);
		delete file;
		return false;
	}
	uint32 animSize = file->readUint32BE();
	byte *data = (byte *)malloc(animSize);
	file->read(data, animSize);
	file->close();
	delete file;

	// data[0..3] = AHDR tag, data[4..7] = AHDR size
	if (READ_BE_UINT32(data) != MKTAG('A','H','D','R')) {
		warning("InsaneRebel1::loadRA1Nut: no AHDR in %s", filename);
		free(data);
		return false;
	}

	const uint16 expectedSprites = READ_LE_UINT16(data + 10);
	bank.numSprites = expectedSprites;
	bank.sprites = new RA1Sprite[bank.numSprites];
	memset(bank.sprites, 0, sizeof(RA1Sprite) * bank.numSprites);

	uint32 *fobjOffsets = (uint32 *)calloc(expectedSprites, sizeof(uint32));
	if (!fobjOffsets) {
		free(data);
		return false;
	}

	// Pass 1: Parse ANIM chunks properly and collect FRME->FOBJ offsets in-order.
	uint32 decodedSize = 0;
	uint16 foundSprites = 0;
	uint32 chunkOffset = 0;
	while (chunkOffset + 8 <= animSize && foundSprites < expectedSprites) {
		uint32 chunkTag = READ_BE_UINT32(data + chunkOffset);
		uint32 chunkSize = READ_BE_UINT32(data + chunkOffset + 4);
		uint32 chunkDataOffset = chunkOffset + 8;
		uint32 chunkEnd = chunkDataOffset + chunkSize;
		if (chunkEnd > animSize)
			break;

		if (chunkTag == MKTAG('F','R','M','E')) {
			uint32 subOffset = chunkDataOffset;
			while (subOffset + 8 <= chunkEnd) {
				uint32 subTag = READ_BE_UINT32(data + subOffset);
				uint32 subSize = READ_BE_UINT32(data + subOffset + 4);
				uint32 subDataOffset = subOffset + 8;
				uint32 subEnd = subDataOffset + subSize;
				if (subEnd > chunkEnd)
					break;

				if (subTag == MKTAG('F','O','B','J') && subOffset + 22 <= animSize) {
					uint16 w = READ_LE_UINT16(data + subOffset + 14);
					uint16 h = READ_LE_UINT16(data + subOffset + 16);
					decodedSize += (uint32)w * (uint32)h;
					fobjOffsets[foundSprites] = subOffset;
					break;
				}

				subOffset = subEnd;
				if (subSize & 1)
					subOffset++;
			}
			// Always increment for every FRME to preserve char-to-glyph alignment.
			// Empty FRMEs (no FOBJ) keep fobjOffsets[i] = 0, decoded as blank sprites.
			foundSprites++;
		}

		chunkOffset = chunkEnd;
		if (chunkSize & 1)
			chunkOffset++;
	}

	bank.decodedData = (byte *)calloc(decodedSize ? decodedSize : 1, 1);
	bank.decodedSize = decodedSize;
	byte *decPtr = bank.decodedData;

	// Pass 2: Decode collected FOBJ entries.
	for (uint16 i = 0; i < foundSprites; i++) {
		uint32 fobjOffset = fobjOffsets[i];
		if (fobjOffset == 0) {
			// Empty FRME (no FOBJ) — leave sprite as blank (zeroed by memset).
			continue;
		}

		int codec = READ_LE_UINT16(data + fobjOffset + 8);
		bank.sprites[i].xoffs = READ_LE_INT16(data + fobjOffset + 10);
		bank.sprites[i].yoffs = READ_LE_INT16(data + fobjOffset + 12);
		bank.sprites[i].width = READ_LE_UINT16(data + fobjOffset + 14);
		bank.sprites[i].height = READ_LE_UINT16(data + fobjOffset + 16);

		int pixelCount = bank.sprites[i].width * bank.sprites[i].height;
		const byte *fobjData = data + fobjOffset + 22;

		if (codec == 21) {
			bank.sprites[i].data = decPtr;
			decodeBomp(decPtr, fobjData, bank.sprites[i].width,
					   bank.sprites[i].height, bank.sprites[i].width);
		} else if (codec == 1) {
			// RA1 codec 1 in NUTs (e.g. LVL2/L2LASER.NUT): RLE where color 0 is transparent.
			// Decode into a zero-cleared sprite buffer so skipped pixels stay transparent.
			bank.sprites[i].data = decPtr;
			smushDecodeRA1Transparent(decPtr, fobjData, 0, 0,
				bank.sprites[i].width, bank.sprites[i].height, bank.sprites[i].width);
		} else {
			bank.sprites[i].width = 0;
			bank.sprites[i].height = 0;
			bank.sprites[i].data = nullptr;
			warning("InsaneRebel1::loadRA1Nut: unsupported codec %d in sprite %d", codec, i);
		}

		decPtr += pixelCount;
	}

	free(fobjOffsets);

	free(data);
	debug(1, "InsaneRebel1::loadRA1Nut('%s'): expected=%d found=%d decoded=%d bytes",
		  filename, expectedSprites, foundSprites, decodedSize);
	return true;
}

void InsaneRebel1::loadLevelSprites(int level) {
	// Ship direction bank — not all levels have one (e.g. Level 2 is first-person)
	Common::String bankFile = Common::String::format("LVL%d/L%dBANK1.NUT", level, level);
	if (!loadRA1Nut(bankFile.c_str(), _shipBank)) {
		debug(1, "InsaneRebel1: No BANK1 for level %d (first-person level)", level);
	}
	loadRA1Nut("SYS/DISPLAY.NUT", _displayBank);

	// Explosion sprites — try BANG first, then EXPLD
	Common::String bangFile = Common::String::format("LVL%d/L%dBANG.NUT", level, level);
	if (!loadRA1Nut(bangFile.c_str(), _bangBank)) {
		Common::String expldFile = Common::String::format("LVL%d/L%dEXPLD.NUT", level, level);
		loadRA1Nut(expldFile.c_str(), _bangBank);
	}

	// Laser/shot effect sprites
	Common::String laserFile = Common::String::format("LVL%d/L%dLASER.NUT", level, level);
	loadRA1Nut(laserFile.c_str(), _laserBank);
}

} // End of namespace Scumm
