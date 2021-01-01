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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "types.h"
#include "pak.h"

#include "common/endian.h"

static void writeStringList(PAKFile &out, const char *filename, const StringListProvider *provider);
static void writeRawData(PAKFile &out, const char *filename, const ByteProvider *provider);
static void writeRoomList(PAKFile &out, const char *filename, const RoomProvider *provider);
static void writeShapeList(PAKFile &out, const char *filename, const ShapeProvider *provider);
static void writeAmigaSfxTable(PAKFile &out, const char *filename, const AmigaSfxTableProvider *provider);
static void writeK2SeqData(PAKFile &out, const char *filename, const HoFSequenceProvider *provider);
static void writeK2SeqItemAnimData(PAKFile &out, const char *filename, const HoFSeqItemAnimDataProvider *provider);
static void writeK2ItemAnimDefinition(PAKFile &out, const char *filename, const ItemAnimDefinitionProvider *provider);
static void writeLoLCharData(PAKFile &out, const char *filename, const LoLCharacterProvider *provider);
static void writeLoLSpellData(PAKFile &out, const char *filename, const SpellPropertyProvider *provider);
static void writeLoLCompassData(PAKFile &out, const char *filename, const CompassDefProvider *provider);
static void writeLoLFlightShpData(PAKFile &out, const char *filename, const FlyingObjectShapeProvider *provider);
static void writeLoLButtonData(PAKFile &out, const char *filename, const LoLButtonDefProvider *provider);
static void writeRawDataBe16(PAKFile &out, const char *filename, const Uint16Provider *provider);
static void writeRawDataBe32(PAKFile &out, const char *filename, const Uint32Provider *provider);
static void writeEoB2SequenceData(PAKFile &out, const char *filename, const DarkMoonAnimCommandProvider *provider);
static void writeEoB2ShapeData(PAKFile &out, const char *filename, const DarkMoonShapeDefProvider *provider);
static void writeEoBNpcData(PAKFile &out, const char *filename, const EoBCharacterProvider *provider);

void writeResource(PAKFile &out, const char *filename, ResTypes type, const void *provider) {
	switch (type) {
	case kStringList:
		writeStringList(out, filename, (const StringListProvider *)provider);
		break;

	case kRawData:
		writeRawData(out, filename, (const ByteProvider *)provider);
		break;

	case kRoomList:
		writeRoomList(out, filename, (const RoomProvider *)provider);
		break;

	case kShapeList:
		writeShapeList(out, filename, (const ShapeProvider *)provider);
		break;

	case kAmigaSfxTable:
		writeAmigaSfxTable(out, filename, (const AmigaSfxTableProvider *)provider);
		break;

	case k2SeqData:
		writeK2SeqData(out, filename, (const HoFSequenceProvider *)provider);
		break;

	case k2SeqItemAnimData:
		writeK2SeqItemAnimData(out, filename, (const HoFSeqItemAnimDataProvider *)provider);
		break;

	case k2ItemAnimDefinition:
		writeK2ItemAnimDefinition(out, filename, (const ItemAnimDefinitionProvider *)provider);
		break;

	case kLoLCharData:
		writeLoLCharData(out, filename, (const LoLCharacterProvider *)provider);
		break;

	case kLoLSpellData:
		writeLoLSpellData(out, filename, (const SpellPropertyProvider *)provider);
		break;

	case kLoLCompassData:
		writeLoLCompassData(out, filename, (const CompassDefProvider *)provider);
		break;

	case kLoLFlightShpData:
		writeLoLFlightShpData(out, filename, (const FlyingObjectShapeProvider *)provider);
		break;

	case kLoLButtonData:
		writeLoLButtonData(out, filename, (const LoLButtonDefProvider *)provider);
		break;

	case kRawDataBe16:
		writeRawDataBe16(out, filename, (const Uint16Provider *)provider);
		break;

	case kRawDataBe32:
		writeRawDataBe32(out, filename, (const Uint32Provider *)provider);
		break;

	case kEoB2SequenceData:
		writeEoB2SequenceData(out, filename, (const DarkMoonAnimCommandProvider *)provider);
		break;

	case kEoB2ShapeData:
		writeEoB2ShapeData(out, filename, (const DarkMoonShapeDefProvider *)provider);
		break;

	case kEoBNpcData:
		writeEoBNpcData(out, filename, (const EoBCharacterProvider *)provider);
		break;

	default:
		break;
	}
}

static void writeStringList(PAKFile &out, const char *filename, const StringListProvider *provider) {
	// Step 1: Calculate size
	size_t size = 4;
	for (uint i = 0; i < provider->numEntries; ++i) {
		size += strlen(provider->data[i]) + 1;
	}

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	WRITE_BE_UINT32(dst, provider->numEntries); dst += 4;
	for (uint i = 0; i < provider->numEntries; ++i) {
		size_t num = strlen(provider->data[i]) + 1;
		memcpy(dst, provider->data[i], num); dst += num;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeRawData(PAKFile &out, const char *filename, const ByteProvider *provider) {
	// Step 1: Calculate size
	size_t size = provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];
	memcpy(output, provider->data, size);

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeRoomList(PAKFile &out, const char *filename, const RoomProvider *provider) {
	// Step 1: Calculate size
	size_t size = 4 + 9 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	WRITE_BE_UINT32(dst, provider->numEntries); dst += 4;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const Room &room = provider->data[i];

		*dst = room.index; dst += 1;
		WRITE_BE_UINT16(dst, room.north); dst += 2;
		WRITE_BE_UINT16(dst, room.east); dst += 2;
		WRITE_BE_UINT16(dst, room.south); dst += 2;
		WRITE_BE_UINT16(dst, room.west); dst += 2;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeShapeList(PAKFile &out, const char *filename, const ShapeProvider *provider) {
	// Step 1: Calculate size
	size_t size = 4 + 7 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	WRITE_BE_UINT32(dst, provider->numEntries); dst += 4;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const Shape &shape = provider->data[i];

		*dst = shape.imageIndex; dst += 1;
		*dst = shape.x; dst += 1;
		*dst = shape.y; dst += 1;
		*dst = shape.w; dst += 1;
		*dst = shape.h; dst += 1;
		*dst = shape.xOffset; dst += 1;
		*dst = shape.yOffset; dst += 1;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeAmigaSfxTable(PAKFile &out, const char *filename, const AmigaSfxTableProvider *provider) {
	// Step 1: Calculate size
	size_t size = 4 + 6 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	WRITE_BE_UINT32(dst, provider->numEntries); dst += 4;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const AmigaSfxTable &entry = provider->data[i];

		*dst = entry.note; dst += 1;
		*dst = entry.patch; dst += 1;
		WRITE_BE_UINT16(dst, entry.duration); dst += 2;
		*dst = entry.volume; dst += 1;
		*dst = entry.pan; dst += 1;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeK2SeqData(PAKFile &out, const char *filename, const HoFSequenceProvider *provider) {
	// Step 1: Calculate size
	size_t size = 4 + (2 + 48) * provider->numSequences + (2 + 32) * provider->numNestedSequences;

	const size_t startSequenceOffset  = 4 + 2 * provider->numSequences + 2 * provider->numNestedSequences;
	const size_t startNestedSequences = startSequenceOffset + 48 * provider->numSequences;
	const size_t startControlOffset   = size;

	for (uint i = 0; i < provider->numNestedSequences; ++i) {
		if (provider->nestedSequences[i].numControls) {
			size += 1 + 4 * provider->nestedSequences[i].numControls;
		}
	}

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *offsetTable = output;
	byte *sequenceDst = output + startSequenceOffset;
	byte *nestedDst   = output + startNestedSequences;
	byte *controlDst  = output + startControlOffset;

	// First write all sequences
	WRITE_BE_UINT16(offsetTable, provider->numSequences); offsetTable += 2;
	for (uint i = 0; i < provider->numSequences; ++i) {
		const HoFSequence &entry = provider->sequences[i];

		// Write location to the offset table
		WRITE_BE_UINT16(offsetTable, sequenceDst - output); offsetTable += 2;

		// Write actual sequence data
		WRITE_BE_UINT16(sequenceDst, entry.flags); sequenceDst += 2;
		memcpy(sequenceDst, entry.wsaFile, 14); sequenceDst += 14;
		memcpy(sequenceDst, entry.cpsFile, 14); sequenceDst += 14;
		*sequenceDst = entry.fadeInTransitionType; sequenceDst += 1;
		*sequenceDst = entry.fadeOutTransitionType; sequenceDst += 1;
		WRITE_BE_UINT16(sequenceDst, entry.stringIndex1); sequenceDst += 2;
		WRITE_BE_UINT16(sequenceDst, entry.stringIndex2); sequenceDst += 2;
		WRITE_BE_UINT16(sequenceDst, entry.startFrame); sequenceDst += 2;
		WRITE_BE_UINT16(sequenceDst, entry.numFrames); sequenceDst += 2;
		WRITE_BE_UINT16(sequenceDst, entry.duration); sequenceDst += 2;
		WRITE_BE_UINT16(sequenceDst, entry.xPos); sequenceDst += 2;
		WRITE_BE_UINT16(sequenceDst, entry.yPos); sequenceDst += 2;
		WRITE_BE_UINT16(sequenceDst, entry.timeout); sequenceDst += 2;
	}

	assert(sequenceDst == nestedDst);

	// Then write all nested sequences
	WRITE_BE_UINT16(offsetTable, provider->numNestedSequences); offsetTable += 2;
	for (uint i = 0; i < provider->numNestedSequences; ++i) {
		const HoFNestedSequence &entry = provider->nestedSequences[i];

		// Write location to the offset table
		WRITE_BE_UINT16(offsetTable, nestedDst - output); offsetTable += 2;

		// Write the nested sequence data
		WRITE_BE_UINT16(nestedDst, entry.flags); nestedDst += 2;
		memcpy(nestedDst, entry.wsaFile, 14); nestedDst += 14;
		WRITE_BE_UINT16(nestedDst, entry.startFrame); nestedDst += 2;
		WRITE_BE_UINT16(nestedDst, entry.endFrame); nestedDst += 2;
		WRITE_BE_UINT16(nestedDst, entry.frameDelay); nestedDst += 2;
		WRITE_BE_UINT16(nestedDst, entry.x); nestedDst += 2;
		WRITE_BE_UINT16(nestedDst, entry.y); nestedDst += 2;

		if (entry.numControls) {
			WRITE_BE_UINT16(nestedDst, controlDst - output); nestedDst += 2;

			*controlDst = entry.numControls; controlDst += 1;
			for (uint j = 0; j < entry.numControls; ++j) {
				WRITE_BE_UINT16(controlDst, entry.wsaControl[j].index); controlDst += 2;
				WRITE_BE_UINT16(controlDst, entry.wsaControl[j].delay); controlDst += 2;
			}
		} else {
			WRITE_BE_UINT16(nestedDst, 0); nestedDst += 2;
		}

		WRITE_BE_UINT16(nestedDst, entry.fadeInTransitionType); nestedDst += 2;
		WRITE_BE_UINT16(nestedDst, entry.fadeOutTransitionType); nestedDst += 2;
	}

	assert(offsetTable == output + startSequenceOffset);
	assert(nestedDst == output + startControlOffset);
	assert(controlDst == output + size);

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeK2SeqItemAnimData(PAKFile &out, const char *filename, const HoFSeqItemAnimDataProvider *provider) {
	// Step 1: Calculate size
	size_t size = 1 + 44 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	*dst = provider->numEntries; dst += 1;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const HoFSeqItemAnimData &entry = provider->data[i];

		WRITE_BE_UINT16(dst, entry.itemIndex); dst += 2;
		WRITE_BE_UINT16(dst, entry.y); dst += 2;

		for (uint j = 0; j < 20; ++j) {
			WRITE_BE_UINT16(dst, entry.frames[j]); dst += 2;
		}
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeK2ItemAnimDefinition(PAKFile &out, const char *filename, const ItemAnimDefinitionProvider *provider) {
	// Step 1: Calculate size
	size_t size = 1 + 3 * provider->numEntries;

	for (uint i = 0; i < provider->numEntries; ++i) {
		size += provider->data[i].numFrames * 4;
	}

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	*dst = provider->numEntries; dst += 1;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const ItemAnimDefinition &entry = provider->data[i];

		WRITE_BE_UINT16(dst, entry.itemIndex); dst += 2;
		*dst = entry.numFrames; dst += 1;

		for (uint j = 0; j < entry.numFrames; ++j) {
			WRITE_BE_UINT16(dst, entry.frames[j].index); dst += 2;
			WRITE_BE_UINT16(dst, entry.frames[j].delay); dst += 2;
		}
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeLoLCharData(PAKFile &out, const char *filename, const LoLCharacterProvider *provider) {
	// Step 1: Calculate size
	size_t size = 130 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const LoLCharacter &entry = provider->data[i];

		WRITE_LE_UINT16(dst, entry.flags); dst += 2;
		memcpy(dst, entry.name, 11); dst += 11;
		*dst = entry.raceClassSex; dst += 1;
		WRITE_LE_UINT16(dst, entry.id); dst += 2;
		*dst = entry.curFaceFrame; dst += 1;
		*dst = entry.tempFaceFrame; dst += 1;
		*dst = entry.screamSfx; dst += 1;
		WRITE_BE_UINT32(dst, 0xDEADBEEF); dst += 4;
		for (uint j = 0; j < 8; ++j) {
			WRITE_LE_UINT16(dst, entry.itemsMight[j]); dst += 2;
		}
		for (uint j = 0; j < 8; ++j) {
			WRITE_LE_UINT16(dst, entry.protectionAgainstItems[j]); dst += 2;
		}
		WRITE_LE_UINT16(dst, entry.itemProtection); dst += 2;
		WRITE_LE_UINT16(dst, entry.hitPointsCur); dst += 2;
		WRITE_LE_UINT16(dst, entry.hitPointsMax); dst += 2;
		WRITE_LE_UINT16(dst, entry.magicPointsCur); dst += 2;
		WRITE_LE_UINT16(dst, entry.magicPointsMax); dst += 2;
		*dst = entry.field_41; dst += 1;
		WRITE_LE_UINT16(dst, entry.damageSuffered); dst += 2;
		WRITE_LE_UINT16(dst, entry.weaponHit); dst += 2;
		WRITE_LE_UINT16(dst, entry.totalMightModifier); dst += 2;
		WRITE_LE_UINT16(dst, entry.totalProtectionModifier); dst += 2;
		WRITE_LE_UINT16(dst, entry.might); dst += 2;
		WRITE_LE_UINT16(dst, entry.protection); dst += 2;
		WRITE_LE_UINT16(dst, entry.nextAnimUpdateCountdown); dst += 2;
		for (uint j = 0; j < 11; ++j) {
			WRITE_LE_UINT16(dst, entry.items[j]); dst += 2;
		}
		for (uint j = 0; j < 3; ++j) {
			*dst = entry.skillLevels[j]; dst += 1;
		}
		for (uint j = 0; j < 3; ++j) {
			*dst = entry.skillModifiers[j]; dst += 1;
		}
		for (uint j = 0; j < 3; ++j) {
			WRITE_LE_UINT32(dst, entry.experiencePts[j]); dst += 4;
		}
		for (uint j = 0; j < 5; ++j) {
			*dst = entry.characterUpdateEvents[j]; dst += 1;
		}
		for (uint j = 0; j < 5; ++j) {
			*dst = entry.characterUpdateDelay[j]; dst += 1;
		}
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeLoLSpellData(PAKFile &out, const char *filename, const SpellPropertyProvider *provider) {
	// Step 1: Calculate size
	size_t size = 28 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const SpellProperty &entry = provider->data[i];

		WRITE_LE_UINT16(dst, entry.spellNameCode); dst += 2;
		for (uint j = 0; j < 4; ++j) {
			WRITE_LE_UINT16(dst, entry.mpRequired[j]); dst += 2;
		}
		WRITE_LE_UINT16(dst, entry.field_a); dst += 2;
		WRITE_LE_UINT16(dst, entry.field_c); dst += 2;
		for (uint j = 0; j < 4; ++j) {
			WRITE_LE_UINT16(dst, entry.hpRequired[j]); dst += 2;
		}
		WRITE_LE_UINT16(dst, entry.field_16); dst += 2;
		WRITE_LE_UINT16(dst, entry.field_18); dst += 2;
		WRITE_LE_UINT16(dst, entry.flags); dst += 2;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeLoLCompassData(PAKFile &out, const char *filename, const CompassDefProvider *provider) {
	// Step 1: Calculate size
	size_t size = 4 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const CompassDef &entry = provider->data[i];

		*dst = entry.shapeIndex; dst += 1;
		*dst = entry.x; dst += 1;
		*dst = entry.y; dst += 1;
		*dst = entry.flags; dst += 1;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeLoLFlightShpData(PAKFile &out, const char *filename, const FlyingObjectShapeProvider *provider) {
	// Step 1: Calculate size
	size_t size = 5 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const FlyingObjectShape &entry = provider->data[i];

		*dst = entry.shapeFront; dst += 1;
		*dst = entry.shapeBack; dst += 1;
		*dst = entry.shapeLeft; dst += 1;
		*dst = entry.drawFlags; dst += 1;
		*dst = entry.flipFlags; dst += 1;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeLoLButtonData(PAKFile &out, const char *filename, const LoLButtonDefProvider *provider) {
	// Step 1: Calculate size
	size_t size = 18 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const LoLButtonDef &entry = provider->data[i];

		WRITE_BE_UINT16(dst, entry.buttonFlags); dst += 2;
		WRITE_BE_UINT16(dst, entry.keyCode); dst += 2;
		WRITE_BE_UINT16(dst, entry.keyCode2); dst += 2;
		WRITE_BE_UINT16(dst, entry.x); dst += 2;
		WRITE_BE_UINT16(dst, entry.y); dst += 2;
		WRITE_BE_UINT16(dst, entry.w); dst += 2;
		WRITE_BE_UINT16(dst, entry.h); dst += 2;
		WRITE_BE_UINT16(dst, entry.index); dst += 2;
		WRITE_BE_UINT16(dst, entry.screenDim); dst += 2;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeRawDataBe16(PAKFile &out, const char *filename, const Uint16Provider *provider) {
	// Step 1: Calculate size
	size_t size = 2 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	for (uint i = 0; i < provider->numEntries; ++i) {
		WRITE_BE_UINT16(dst, provider->data[i]); dst += 2;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeRawDataBe32(PAKFile &out, const char *filename, const Uint32Provider *provider) {
	// Step 1: Calculate size
	size_t size = 4 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	for (uint i = 0; i < provider->numEntries; ++i) {
		WRITE_BE_UINT32(dst, provider->data[i]); dst += 4;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeEoB2SequenceData(PAKFile &out, const char *filename, const DarkMoonAnimCommandProvider *provider) {
	// Step 1: Calculate size
	size_t size = 11 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const DarkMoonAnimCommand &entry = provider->data[i];

		*dst = entry.command; dst += 1;
		*dst = entry.obj; dst += 1;
		WRITE_BE_UINT16(dst, entry.x1); dst += 2;
		*dst = entry.y1; dst += 1;
		*dst = entry.delay; dst += 1;
		*dst = entry.pal; dst += 1;
		*dst = entry.x2; dst += 1;
		*dst = entry.y2; dst += 1;
		*dst = entry.w; dst += 1;
		*dst = entry.h; dst += 1;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeEoB2ShapeData(PAKFile &out, const char *filename, const DarkMoonShapeDefProvider *provider) {
	// Step 1: Calculate size
	size_t size = 6 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const DarkMoonShapeDef &entry = provider->data[i];

		WRITE_BE_UINT16(dst, entry.index); dst += 2;
		*dst = entry.x; dst += 1;
		*dst = entry.y; dst += 1;
		*dst = entry.w; dst += 1;
		*dst = entry.h; dst += 1;
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}

static void writeEoBNpcData(PAKFile &out, const char *filename, const EoBCharacterProvider *provider) {
	// Step 1: Calculate size
	size_t size = 2 + 111 * provider->numEntries;

	// Step 2: Write data
	byte *const output = new byte[size];

	byte *dst = output;
	WRITE_BE_UINT16(dst, provider->numEntries); dst += 2;
	for (uint i = 0; i < provider->numEntries; ++i) {
		const EoBCharacter &entry = provider->data[i];

		*dst = entry.id; dst += 1;
		*dst = entry.flags; dst += 1;
		//memcpy(dst, entry.name, 11); dst += 11;
		*dst = entry.strengthCur; dst += 1;
		*dst = entry.strengthMax; dst += 1;
		*dst = entry.strengthExtCur; dst += 1;
		*dst = entry.strengthExtMax; dst += 1;
		*dst = entry.intelligenceCur; dst += 1;
		*dst = entry.intelligenceMax; dst += 1;
		*dst = entry.wisdomCur; dst += 1;
		*dst = entry.wisdomMax; dst += 1;
		*dst = entry.dexterityCur; dst += 1;
		*dst = entry.dexterityMax; dst += 1;
		*dst = entry.constitutionCur; dst += 1;
		*dst = entry.constitutionMax; dst += 1;
		*dst = entry.charismaCur; dst += 1;
		*dst = entry.charismaMax; dst += 1;
		WRITE_BE_UINT16(dst, entry.hitPointsCur); dst += 2;
		WRITE_BE_UINT16(dst, entry.hitPointsMax); dst += 2;
		*dst = entry.armorClass; dst += 1;
		*dst = entry.disabledSlots; dst += 1;
		*dst = entry.raceSex; dst += 1;
		*dst = entry.cClass; dst += 1;
		*dst = entry.alignment; dst += 1;
		*dst = entry.portrait; dst += 1;
		*dst = entry.food; dst += 1;
		memcpy(dst, entry.level, 3); dst += 3;
		WRITE_BE_UINT32(dst, entry.experience[0]); dst += 4;
		WRITE_BE_UINT32(dst, entry.experience[1]); dst += 4;
		WRITE_BE_UINT32(dst, entry.experience[2]); dst += 4;
		WRITE_BE_UINT32(dst, entry.mageSpellsAvailableFlags); dst += 4;
		for (uint j = 0; j < 27; ++j) {
			WRITE_BE_UINT16(dst, entry.inventory[j]); dst += 2;
		}
	}

	// Step 3: Add data to output
	out.addFile(filename, output, size);
}
