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
#include "scumm/insane/rebel1/rebel.h"
#include "scumm/smush/rebel/anim_ra1.h"
#include "scumm/smush/rebel/codec_ra1.h"

namespace Scumm {

void resetSpriteBank(RA1SpriteBank &bank) {
	delete[] bank.sprites;
	bank.sprites = nullptr;
	free(bank.decodedData);
	bank.decodedData = nullptr;
	bank.numSprites = 0;
	bank.decodedSize = 0;
}

// Load an RA1 NUT sprite file (ANIM v1).
// RA1 NUTs can have odd-size FOBJ chunks padded to 2-byte alignment within
// FRME containers. This loader handles that padding properly, unlike the
// shared NutRenderer::loadFont which assumes even-size chunks.
bool InsaneRebel1::loadRA1Nut(const char *filename, RA1SpriteBank &bank, bool warnIfMissing) {
	resetSpriteBank(bank);

	ScummFile *file = _vm->instantiateScummFile();
	_vm->openFile(*file, filename);
	if (!file->isOpen()) {
		if (warnIfMissing)
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
	RA1AnimChunkIterator chunks(data, animSize);
	RA1AnimChunk chunk;
	while (chunks.next(chunk) && foundSprites < expectedSprites) {
		if (chunk.tag == MKTAG('F','R','M','E')) {
			RA1AnimChunkIterator subChunks(data, (uint32)chunk.dataOffset, (uint32)chunk.endOffset);
			RA1AnimChunk subChunk;
			while (subChunks.next(subChunk)) {
				if (subChunk.tag == MKTAG('F','O','B','J') && subChunk.size >= 14) {
					uint16 w = READ_LE_UINT16(data + subChunk.dataOffset + 6);
					uint16 h = READ_LE_UINT16(data + subChunk.dataOffset + 8);
					decodedSize += (uint32)w * (uint32)h;
					fobjOffsets[foundSprites] = (uint32)subChunk.offset;
					break;
				}
			}
			// Always increment for every FRME to preserve char-to-glyph alignment.
			// Empty FRMEs (no FOBJ) keep fobjOffsets[i] = 0, decoded as blank sprites.
			foundSprites++;
		}
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
		const uint32 fobjSize = READ_BE_UINT32(data + fobjOffset + 4);
		const int fobjDataSize = (fobjSize >= 14) ? (int)(fobjSize - 14) : 0;

		if (codec == 21) {
			bank.sprites[i].data = decPtr;
			smushDecodeRA1SkipCopy(decPtr, fobjData, 0, 0, bank.sprites[i].width,
				bank.sprites[i].height, bank.sprites[i].width, bank.sprites[i].width,
				bank.sprites[i].height, fobjDataSize);
		} else if (codec == 1) {
			// RA1 codec 1 in NUTs (e.g. LVL2/L2LASER.NUT): RLE where color 0 is transparent.
			// Decode into a zero-cleared sprite buffer so skipped pixels stay transparent.
			bank.sprites[i].data = decPtr;
			smushDecodeRA1Transparent(decPtr, fobjData, 0, 0,
				bank.sprites[i].width, bank.sprites[i].height, bank.sprites[i].width,
				fobjDataSize);
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
	debugC(DEBUG_INSANE, "InsaneRebel1::loadRA1Nut('%s'): expected=%d found=%d decoded=%d bytes",
		  filename, expectedSprites, foundSprites, decodedSize);
	return true;
}

void InsaneRebel1::loadLevelSprites(int level) {
	// Ship/character direction bank — try BANK1, BANK, then PILOT (Level 9 on-foot)
	Common::String bankFile = Common::String::format("LVL%d/L%dBANK1.NUT", level, level);
	if (!loadRA1Nut(bankFile.c_str(), _shipBank, false)) {
		Common::String legacyBankFile = Common::String::format("LVL%d/L%dBANK.NUT", level, level);
		if (!loadRA1Nut(legacyBankFile.c_str(), _shipBank, false)) {
			Common::String pilotFile = Common::String::format("LVL%d/L%dPILOT.NUT", level, level);
			if (!loadRA1Nut(pilotFile.c_str(), _shipBank, false))
				debugC(DEBUG_INSANE, "InsaneRebel1::loadLevelSprites: No BANK1/BANK/PILOT for level %d", level);
		}
	}

	// Secondary ship bank used by some level-specific handlers (e.g. LVL1 mode-2).
	Common::String bankFileAlt = Common::String::format("LVL%d/L%dBANK2.NUT", level, level);
	if (!loadRA1Nut(bankFileAlt.c_str(), _shipBankAlt, false)) {
		debugC(DEBUG_INSANE, "InsaneRebel1::loadLevelSprites: No BANK2 for level %d", level);
	}

	loadRA1Nut("SYS/DISPLAY.NUT", _displayBank);

	// Explosion sprites — try BANG first, then EXPLD
	Common::String bangFile = Common::String::format("LVL%d/L%dBANG.NUT", level, level);
	if (!loadRA1Nut(bangFile.c_str(), _bangBank, false)) {
		Common::String expldFile = Common::String::format("LVL%d/L%dEXPLD.NUT", level, level);
		if (!loadRA1Nut(expldFile.c_str(), _bangBank, false))
			debugC(DEBUG_INSANE, "InsaneRebel1::loadLevelSprites: No BANG/EXPLD for level %d", level);
	}

	// Laser/shot effect sprites
	Common::String laserFile = Common::String::format("LVL%d/L%dLASER.NUT", level, level);
	loadRA1Nut(laserFile.c_str(), _laserBank);
}

} // End of namespace Scumm
