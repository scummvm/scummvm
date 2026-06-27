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

#include "hollywood/scenes/playable/scene7010.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/system.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kG01ArchiveName = "RESOURCE.G01";
const char *const kResource000Name = "RESOURCE.000";
const char *const kStage003ArchiveName = "RESOURCE.003";
const uint16 kG01Chunk8DescriptorCount = 0x16;
const uint16 kG01Chunk9DescriptorCount = 2;
const uint16 kG01Chunk10DescriptorCount = 0x10;
const byte kG01AmbientMusicCueWithoutChunk9 = 0x0f;
const uint16 kG01InitialViewportXOffset = 0;
const byte kG01SecondarySpeechTextColor = 0xfd;
const byte kG01PrimarySpeechTextColor = 0xfb;
const byte kG01SueEntryFacing = 1;
const byte kG01SueEntryFinalCel = 0;
const int kG01SueEntryStartX = 0x16b;
const int kG01SueEntryStartY = 0x1df;
const int kG01SueEntryTargetX = 0x184;
const int kG01SueEntryTargetY = 0x1c6;
const uint32 kG01ActorPathFrameMillis = 60;
const uint32 kG01SecondaryActorFrameMillis = 150;
const uint32 kG01Chunk8FrameMillis = 75;
const uint32 kG01Chunk10FrameMillis = 125;
const uint32 kG01DialogueOverlayFrameMillis = 60;
const byte kInvalidFacing = 0xff;
const byte kInvalidCel = 0xff;
const byte kG01Chunk8FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 5, 7, 8, 9, 10, 11, 12, 13, 21,
	9, 8, 7, 0, 14, 15
};
const byte kG01Chunk11FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 20, 19, 18, 13, 12,
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 35, 13, 26, 27,
	28, 29, 30, 31, 32, 33, 34, 13, 36
};
const byte kG01Chunk14FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, 26, 27, 28, 29, 30, 31
};
const byte kG01Chunk15FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 19, 20, 21, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 22
};
const byte kG01DialogueOverlayMode1FrameMap[] = { 0, 1, 2, 1 };
const byte kG01DialogueOverlayMode2FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26
};
const byte kInitialInventoryItems[] = { 1, 2, 5, 7 };

Scene7010::Scene7010(HollywoodEngine *vm) :
		_vm(vm),
		_resourceArenaCursor(0),
		_random("hollywood_scene7010"),
		_chunk8FrameIndex(0),
		_chunk9AmbientOverlayFrameIndex(0),
		_chunk10IdleFrameA(0),
		_chunk10IdleFrameB(8),
		_chunk10IdleFrameC(4),
		_chunk10IdleFrameD(0x0c),
		_chunk11FrameIndex(0),
		_chunk14FrameIndex(0),
		_chunk15FrameIndex(0),
		_dialogueOverlayFrameIndex(0),
		_dialogueOverlayMode(0),
		_chunk11Visible(false),
		_chunk14Visible(false),
		_chunk15Visible(false),
		_chunk10IdlePairAAltPhase(false),
		_chunk10IdlePairBAltPhase(false),
		_chunk10IdlePairATicksRemaining(10),
		_chunk10IdlePairBTicksRemaining(16),
		_chunk9AmbientDecisionCounter(0),
		_chunk10DeterministicCounter(0),
		_chunk8TimerAccumulator(0),
		_chunk10TimerAccumulator(0),
		_secondaryActorTimerAccumulator(0),
		_dialogueOverlayTimerAccumulator(0),
		_secondaryActorIdleTick(0),
		_activeActorWorldX(kG01SueEntryTargetX),
		_activeActorWorldY(kG01SueEntryTargetY),
		_activeActorFacing(kG01SueEntryFacing),
		_activeActorCel(kG01SueEntryFinalCel),
		_secondaryActorFrame(0),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kHollywoodPaletteSize);
	_paletteCurrent.resize(kHollywoodPaletteSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_savedFramebuffer.resize(kFrameBufferSize);
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
	_activeActorRunStreams.resize(kActorFacingCount * kActiveActorFacingRunStride);
	_secondaryActorRunStreams.resize(kActorFacingCount * kSecondaryActorFacingRunStride);
	_activeActorDescriptors.resize(kActorFacingCount * kActorCelsPerFacing);
	_secondaryActorDescriptors.resize(kActorFacingCount * kSecondaryActorFramesPerFacing);
	_stage003DecodeKey.resize(kStage003DecodeKeySize);
	_stage003StageBlock.resize(kStage003DescriptorTableSize);
	_owner1SpeechCueDescriptors.resize(kOwner1SpeechCueDescriptorTableSize);
	_speechOverlay.visible = false;
	_speechOverlay.colorIndex = kG01SecondarySpeechTextColor;
	_speechOverlay.centerX = 0;
	_speechOverlay.topY = 0;
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.colorIndex = kG01PrimarySpeechTextColor;
	_primarySpeechOverlay.centerX = 0;
	_primarySpeechOverlay.topY = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
}

bool Scene7010::play() {
	if (!load())
		return false;

	initializePreviewState();
	drawPreviewComposite();
	presentFrame();
	runEntryCutscene();
	if (Engine::shouldQuit())
		return true;

	_skipRequested = false;
	return runBasicGameplayLoop();
}

bool Scene7010::load() {
	if (!loadResource000RuntimeTables(_resource000OffsetTable, _resource000SizeTable) ||
			!loadResource000ActorBankSet00(_resource000OffsetTable, _resource000SizeTable))
		return false;

	if (!_vm->resources()->readChunkTable(Common::Path(kG01ArchiveName), _g01ChunkTable)) {
		warning("Failed to read %s header", kG01ArchiveName);
		return false;
	}

	for (uint i = 0; i < kG01InitialRequiredChunkCount; ++i) {
		if (!_g01ChunkTable.isValidChunk(i)) {
			warning("%s is missing required Scene 7010 chunk %u", kG01ArchiveName, i);
			return false;
		}
	}

	if (!loadFixedChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadFixedChunk(1, _paletteResource, kHollywoodPaletteSize) ||
			!loadVariableChunk(2, _fillRuns) ||
			!loadVariableChunk(3, _paletteMask) ||
			!loadVariableChunk(4, _metadata))
		return false;

	if (_paletteMask.size() < kG01PaletteMaskUsedBytes) {
		warning("%s chunk 3 is shorter than the G01 palette mask table", kG01ArchiveName);
		return false;
	}

	uint32 arenaSize = 0;
	for (uint i = kG01ArenaFirstChunk; i <= kG01ArenaLastChunk; ++i)
		arenaSize += _g01ChunkTable.sizes[i];

	_resourceArena.resize(arenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));

	for (uint i = kG01ArenaFirstChunk; i <= kG01ArenaLastChunk; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	memset(_savedFramebuffer.data(), 0, _savedFramebuffer.size());
	expandFillRunsToSavedFramebuffer();
	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	if (!loadResource000Owner1ActorPalette(_resource000OffsetTable) ||
			!loadStage003SceneRows())
		return false;

	if (!_hotspots.load(_paletteMask, _metadata, _stage003SmallRows))
		return false;

	debugC(1, kDebugScene, "Scene 7010 loaded RESOURCE.G01 for non-interactive preview");
	return true;
}

bool Scene7010::loadResource000RuntimeTables(Common::Array<byte> &offsetTable, Common::Array<byte> &sizeTable) {
	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s for Scene 7010 actor resources", kResource000Name);
		return false;
	}

	if ((uint32)file.size() < 1 + (2 * kResource000TableByteCount)) {
		warning("%s is too small for Scene 7010 runtime tables", kResource000Name);
		return false;
	}

	file.seek(1);
	offsetTable.resize(kResource000TableByteCount);
	sizeTable.resize(kResource000TableByteCount);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size() ||
			file.read(sizeTable.data(), sizeTable.size()) != sizeTable.size()) {
		warning("Failed to read %s runtime tables for Scene 7010", kResource000Name);
		return false;
	}

	return true;
}

bool Scene7010::loadResource000ActorBankSet00(const Common::Array<byte> &offsetTable, const Common::Array<byte> &sizeTable) {
	if (kResource000ActorSet00TableEntry + 4 > offsetTable.size() ||
			kResource000ActorSet00TableEntry + kResource000ActorSet00SegmentCount * 4 > sizeTable.size()) {
		warning("%s actor bank set 00 table entries are out of range", kResource000Name);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s actor bank set 00", kResource000Name);
		return false;
	}

	const uint32 actorBankOffset = readUint32LE(offsetTable, kResource000ActorSet00TableEntry);
	if (actorBankOffset > (uint32)file.size()) {
		warning("%s actor bank set 00 offset is out of range", kResource000Name);
		return false;
	}

	file.seek(actorBankOffset);
	memset(_activeActorRunStreams.data(), 0, _activeActorRunStreams.size());
	memset(_secondaryActorRunStreams.data(), 0, _secondaryActorRunStreams.size());

	for (uint segment = 0; segment < kResource000ActorSet00SegmentCount; ++segment) {
		const uint32 segmentSize = readUint32LE(sizeTable, kResource000ActorSet00TableEntry + segment * 4);
		if (segment <= 5) {
			if (segmentSize > kActiveActorFacingRunStride) {
				warning("%s actor set 00 active run segment %u is too large", kResource000Name, segment);
				return false;
			}
			if (file.read(_activeActorRunStreams.data() + segment * kActiveActorFacingRunStride, segmentSize) != segmentSize) {
				warning("Failed to read %s actor set 00 active run segment %u", kResource000Name, segment);
				return false;
			}
		} else if (segment == 6) {
			if (segmentSize % kActiveActorDescriptorSize != 0) {
				warning("%s actor set 00 active descriptors have invalid size", kResource000Name);
				return false;
			}
			Common::Array<byte> descriptors;
			descriptors.resize(segmentSize);
			if (file.read(descriptors.data(), descriptors.size()) != descriptors.size()) {
				warning("Failed to read %s actor set 00 active descriptors", kResource000Name);
				return false;
			}
			const uint descriptorCount = MIN<uint>(_activeActorDescriptors.size(), descriptors.size() / kActiveActorDescriptorSize);
			for (uint i = 0; i < descriptorCount; ++i) {
				const uint offset = i * kActiveActorDescriptorSize;
				_activeActorDescriptors[i].runStreamOffset = readUint32LE(descriptors, offset);
				_activeActorDescriptors[i].opaqueRunCount = readUint32LE(descriptors, offset + 4);
				_activeActorDescriptors[i].paletteRunCount = readUint32LE(descriptors, offset + 8);
				_activeActorDescriptors[i].anchorX = readSint16LE(descriptors, offset + 12);
				_activeActorDescriptors[i].anchorY = readSint16LE(descriptors, offset + 16);
				_activeActorDescriptors[i].width = readUint16LE(descriptors, offset + 20);
				_activeActorDescriptors[i].height = readUint16LE(descriptors, offset + 24);
			}
		} else if (segment <= 12) {
			const uint facing = segment - 7;
			if (segmentSize > kSecondaryActorFacingRunStride) {
				warning("%s actor set 00 secondary run segment %u is too large", kResource000Name, facing);
				return false;
			}
			if (file.read(_secondaryActorRunStreams.data() + facing * kSecondaryActorFacingRunStride, segmentSize) != segmentSize) {
				warning("Failed to read %s actor set 00 secondary run segment %u", kResource000Name, facing);
				return false;
			}
		} else {
			if (segmentSize % kSecondaryActorDescriptorSize != 0) {
				warning("%s actor set 00 secondary descriptors have invalid size", kResource000Name);
				return false;
			}
			Common::Array<byte> descriptors;
			descriptors.resize(segmentSize);
			if (file.read(descriptors.data(), descriptors.size()) != descriptors.size()) {
				warning("Failed to read %s actor set 00 secondary descriptors", kResource000Name);
				return false;
			}
			const uint descriptorCount = MIN<uint>(_secondaryActorDescriptors.size(), descriptors.size() / kSecondaryActorDescriptorSize);
			for (uint i = 0; i < descriptorCount; ++i) {
				const uint offset = i * kSecondaryActorDescriptorSize;
				_secondaryActorDescriptors[i].runStreamOffset = readUint32LE(descriptors, offset);
				_secondaryActorDescriptors[i].runCount = readUint32LE(descriptors, offset + 4);
				_secondaryActorDescriptors[i].anchorX = readSint16LE(descriptors, offset + 8);
				_secondaryActorDescriptors[i].anchorY = readSint16LE(descriptors, offset + 12);
			}
		}
	}

	debugC(1, kDebugResources, "Loaded %s actor bank set 00 for Scene 7010", kResource000Name);
	return true;
}

bool Scene7010::loadResource000Owner1ActorPalette(const Common::Array<byte> &offsetTable) {
	if (kResource000Owner1PaletteTableEntry + 4 > offsetTable.size()) {
		warning("%s owner 1 palette table entry is out of range", kResource000Name);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s owner 1 palette", kResource000Name);
		return false;
	}

	const uint32 paletteOffset = readUint32LE(offsetTable, kResource000Owner1PaletteTableEntry);
	if (paletteOffset > (uint32)file.size() || kOwner1ActorPaletteBytes > (uint32)file.size() - paletteOffset ||
			0x270 + kOwner1ActorPaletteBytes > _paletteCurrent.size()) {
		warning("%s owner 1 palette is out of range", kResource000Name);
		return false;
	}

	file.seek(paletteOffset);
	if (file.read(_paletteCurrent.data() + 0x270, kOwner1ActorPaletteBytes) != kOwner1ActorPaletteBytes) {
		warning("Failed to read %s owner 1 palette", kResource000Name);
		return false;
	}

	return true;
}

bool Scene7010::loadStage003SceneRows() {
	Common::File file;
	if (!file.open(Common::Path(kStage003ArchiveName))) {
		warning("Failed to open %s for Scene 7010 text", kStage003ArchiveName);
		return false;
	}

	if (file.read(_stage003DecodeKey.data(), _stage003DecodeKey.size()) != _stage003DecodeKey.size()) {
		warning("Failed to read %s row decode key", kStage003ArchiveName);
		return false;
	}

	if (kOwner1SpeechCueDescriptorTableOffset + kOwner1SpeechCueDescriptorTableSize + 3 > (uint32)file.size()) {
		warning("%s owner 1 speech cue table is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(kOwner1SpeechCueDescriptorTableOffset);
	if (file.read(_owner1SpeechCueDescriptors.data(), _owner1SpeechCueDescriptors.size()) !=
			_owner1SpeechCueDescriptors.size()) {
		warning("Failed to read %s owner 1 speech cue table", kStage003ArchiveName);
		return false;
	}

	const byte owner1SmallRowCount = file.readByte();
	const uint16 owner1LargeRowCount = file.readUint16LE();
	if (file.err()) {
		warning("Failed to read %s owner 1 text row counts", kStage003ArchiveName);
		return false;
	}

	const uint32 owner1RowsOffsetEntry = kStage003DecodeKeySize + kOwner1Resource003RowsOffsetIndex * 4;
	if (owner1RowsOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			owner1RowsOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s owner 1 text row offset entry is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(owner1RowsOffsetEntry);
	const uint32 owner1RowsOffset = file.readUint32LE();
	const uint32 owner1SmallRowBytes = (uint32)owner1SmallRowCount * kStage003SmallRowSize;
	const uint32 owner1LargeRowBytes = (uint32)owner1LargeRowCount * kStage003LargeRowSize;
	if (owner1RowsOffset == 0 ||
			owner1RowsOffset + owner1SmallRowBytes + owner1LargeRowBytes > (uint32)file.size()) {
		warning("%s owner 1 text rows are out of range", kStage003ArchiveName);
		return false;
	}

	_owner1LargeRows.resize((uint32)(owner1LargeRowCount + 1) * kStage003LargeRowSize);
	memset(_owner1LargeRows.data(), 0, _owner1LargeRows.size());
	file.seek(owner1RowsOffset + owner1SmallRowBytes);
	if (file.read(_owner1LargeRows.data() + kStage003LargeRowSize, owner1LargeRowBytes) != owner1LargeRowBytes) {
		warning("Failed to read %s owner 1 large text rows", kStage003ArchiveName);
		return false;
	}

	for (uint row = 1; row <= owner1LargeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_owner1LargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
	}

	const uint32 stageOffsetEntry = kStage003DecodeKeySize + (kG01StageIndex * 4);
	if (stageOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			stageOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s has no stage 701 offset entry", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffsetEntry);
	const uint32 stageOffset = file.readUint32LE();
	if (stageOffset + kStage003DescriptorTableSize + 3 > (uint32)file.size()) {
		warning("%s stage 701 descriptor table is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffset);
	if (file.read(_stage003StageBlock.data(), _stage003StageBlock.size()) != _stage003StageBlock.size()) {
		warning("Failed to read %s stage 701 descriptor table", kStage003ArchiveName);
		return false;
	}

	const byte smallRowCount = file.readByte();
	const uint16 largeRowCount = file.readUint16LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kStage003SmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kStage003LargeRowSize;
	if (file.pos() + smallRowBytes + largeRowBytes > file.size()) {
		warning("%s stage 701 text rows are out of range", kStage003ArchiveName);
		return false;
	}

	_stage003SmallRows.resize((uint32)(smallRowCount + 1) * kStage003SmallRowSize);
	memset(_stage003SmallRows.data(), 0, _stage003SmallRows.size());
	if (file.read(_stage003SmallRows.data() + kStage003SmallRowSize, smallRowBytes) != smallRowBytes) {
		warning("Failed to read %s stage 701 small text rows", kStage003ArchiveName);
		return false;
	}

	_stage003LargeRows.resize(largeRowBytes);
	if (file.read(_stage003LargeRows.data(), _stage003LargeRows.size()) != _stage003LargeRows.size()) {
		warning("Failed to read %s stage 701 large text rows", kStage003ArchiveName);
		return false;
	}

	for (uint row = 1; row <= smallRowCount; ++row) {
		for (uint column = 0; column < kStage003SmallRowSize; ++column)
			_stage003SmallRows[row * kStage003SmallRowSize + column] -= _stage003DecodeKey[column];
	}

	for (uint row = 0; row < largeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_stage003LargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
	}

	debugC(1, kDebugResources, "Loaded %s stage 701 text rows: smallRows=%u largeRows=%u",
		kStage003ArchiveName, smallRowCount, largeRowCount);
	return true;
}

bool Scene7010::loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kG01ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kG01ArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed Scene 7010 destination", kG01ArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kG01ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s fixed chunk %u: size=%u", kG01ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene7010::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kG01ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kG01ArchiveName, index);
		return false;
	}

	destination.resize(stream->size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kG01ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s variable chunk %u: size=%u", kG01ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene7010::loadArenaChunk(uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kG01ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kG01ArchiveName, index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the Scene 7010 resource arena", kG01ArchiveName, index);
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kG01ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		kG01ArchiveName, index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

void Scene7010::expandFillRunsToSavedFramebuffer() {
	uint destinationOffset = 0;
	uint sourceOffset = 0;
	while (destinationOffset < _savedFramebuffer.size() && sourceOffset + 3 <= _fillRuns.size()) {
		const byte fill = _fillRuns[sourceOffset];
		const uint16 runLength = readUint16LE(_fillRuns, sourceOffset + 1);
		sourceOffset += 3;

		const uint count = MIN<uint>(runLength, _savedFramebuffer.size() - destinationOffset);
		if (count != 0) {
			memset(_savedFramebuffer.data() + destinationOffset, fill, count);
			destinationOffset += count;
		}
	}
}

void Scene7010::initializePreviewState() {
	_chunk8FrameIndex = _vm->gameState().currentRandomAmbientMusicTrackId == kG01AmbientMusicCueWithoutChunk9 ? 0x14 : 0;
	_chunk9AmbientOverlayFrameIndex = 0;
	_chunk10IdleFrameA = 0;
	_chunk10IdleFrameB = 8;
	_chunk10IdleFrameC = 4;
	_chunk10IdleFrameD = 0x0c;
	_chunk11FrameIndex = 0;
	_chunk14FrameIndex = 0;
	_chunk15FrameIndex = 0;
	_dialogueOverlayFrameIndex = 0;
	_dialogueOverlayMode = 0;
	_chunk11Visible = false;
	_chunk14Visible = false;
	_chunk15Visible = false;
	_chunk10IdlePairAAltPhase = false;
	_chunk10IdlePairBAltPhase = false;
	_chunk10IdlePairATicksRemaining = 10;
	_chunk10IdlePairBTicksRemaining = 16;
	_chunk9AmbientDecisionCounter = 0;
	_chunk10DeterministicCounter = 0;
	_chunk8TimerAccumulator = 0;
	_chunk10TimerAccumulator = 0;
	_secondaryActorTimerAccumulator = 0;
	_dialogueOverlayTimerAccumulator = 0;
	_secondaryActorIdleTick = 0;
	_activeActorWorldX = kG01SueEntryTargetX;
	_activeActorWorldY = kG01SueEntryTargetY;
	_activeActorFacing = kG01SueEntryFacing;
	_activeActorCel = kG01SueEntryFinalCel;
	_secondaryActorFrame = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	for (uint i = 0; i < ARRAYSIZE(kInitialInventoryItems); ++i)
		_inventoryItems[kInitialInventoryItems[i]] = true;
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
}

void Scene7010::drawPreviewComposite() {
	drawCutsceneComposite(false, 0, 0, 0, 0, false, 0, 0, 0, 0);
}

void Scene7010::drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY) {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[10], 0,
		kG01Chunk10DescriptorCount, _chunk10IdleFrameA, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[10], 0,
		kG01Chunk10DescriptorCount, _chunk10IdleFrameC, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[10], 0,
		kG01Chunk10DescriptorCount, _chunk10IdleFrameB, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[10], 0,
		kG01Chunk10DescriptorCount, _chunk10IdleFrameD, _sceneFramebuffer);

	if (_dialogueOverlayMode == 1)
		drawMappedSpriteFrame(12, 3, kG01DialogueOverlayMode1FrameMap,
			ARRAYSIZE(kG01DialogueOverlayMode1FrameMap), _dialogueOverlayFrameIndex);
	else if (_dialogueOverlayMode == 2)
		drawMappedSpriteFrame(16, 0x1b, kG01DialogueOverlayMode2FrameMap,
			ARRAYSIZE(kG01DialogueOverlayMode2FrameMap), _dialogueOverlayFrameIndex);

	if (_chunk14Visible)
		drawMappedSpriteFrame(14, 0x20, kG01Chunk14FrameMap, ARRAYSIZE(kG01Chunk14FrameMap),
			_chunk14FrameIndex);
	if (_chunk11Visible)
		drawMappedSpriteFrame(11, 0x25, kG01Chunk11FrameMap, ARRAYSIZE(kG01Chunk11FrameMap),
			_chunk11FrameIndex);

	if (drawSecondaryActor)
		drawSecondaryActorFrame(secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY);
	if (drawActiveActor)
		drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY);

	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	const byte chunk8DescriptorIndex = _chunk8FrameIndex < ARRAYSIZE(kG01Chunk8FrameMap) ?
		kG01Chunk8FrameMap[_chunk8FrameIndex] : 0;
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[8], 0,
		kG01Chunk8DescriptorCount, chunk8DescriptorIndex, _sceneFramebuffer);

	if (_vm->gameState().currentRandomAmbientMusicTrackId != kG01AmbientMusicCueWithoutChunk9) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[9], 0,
			kG01Chunk9DescriptorCount, _chunk9AmbientOverlayFrameIndex, _sceneFramebuffer);
	}

	if (_chunk15Visible)
		drawMappedSpriteFrame(15, 0x17, kG01Chunk15FrameMap, ARRAYSIZE(kG01Chunk15FrameMap),
			_chunk15FrameIndex);
}

void Scene7010::drawPlayableComposite() {
	const bool drawSecondaryActor = _speechOverlay.visible;
	drawCutsceneComposite(true, _activeActorFacing, _activeActorCel, _activeActorWorldX, _activeActorWorldY,
		drawSecondaryActor, _activeActorFacing, _secondaryActorFrame, _activeActorWorldX, _activeActorWorldY);
}

void Scene7010::drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex) {
	if (chunkIndex >= HollywoodEngine::kResourceChunkCount || frameIndex >= frameMapSize)
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[chunkIndex], 0,
		descriptorCount, frameMap[frameIndex], _sceneFramebuffer);
}

void Scene7010::drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY) {
	if (facing >= kActorFacingCount || cel >= kActorCelsPerFacing)
		return;

	const uint descriptorIndex = facing * kActorCelsPerFacing + cel;
	if (descriptorIndex >= _activeActorDescriptors.size())
		return;

	const ActiveActorSpriteDescriptor &descriptor = _activeActorDescriptors[descriptorIndex];
	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	drawActorRun(_activeActorRunStreams, descriptor.runStreamOffset, facing * kActiveActorFacingRunStride,
		descriptor.opaqueRunCount, spriteX, spriteY);
}

void Scene7010::drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY) {
	if (facing >= kActorFacingCount || frame >= kSecondaryActorFramesPerFacing)
		return;

	const uint descriptorIndex = facing * kSecondaryActorFramesPerFacing + frame;
	if (descriptorIndex >= _secondaryActorDescriptors.size())
		return;

	const SecondaryActorSpriteDescriptor &descriptor = _secondaryActorDescriptors[descriptorIndex];
	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	drawActorRun(_secondaryActorRunStreams, descriptor.runStreamOffset, facing * kSecondaryActorFacingRunStride,
		descriptor.runCount, spriteX, spriteY);
}

void Scene7010::drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount, int spriteX, int spriteY) {
	cursor += runBase;
	for (uint runIndex = 0; runIndex < runCount; ++runIndex) {
		if (cursor + 3 > runStreams.size())
			return;

		const int xOffset = runStreams[cursor++];
		const int yOffset = runStreams[cursor++];
		const uint pixelCount = runStreams[cursor++];
		if (cursor + pixelCount > runStreams.size())
			return;

		const int dstY = spriteY + yOffset;
		if (dstY >= 0 && dstY < HollywoodEngine::kSceneBufferHeight) {
			int dstX = spriteX + xOffset;
			uint sourceOffset = 0;
			uint copyCount = pixelCount;
			if (dstX < 0) {
				const uint clipped = MIN<uint>(copyCount, (uint)-dstX);
				sourceOffset += clipped;
				copyCount -= clipped;
				dstX = 0;
			}
			if (dstX + (int)copyCount > HollywoodEngine::kSceneBufferWidth)
				copyCount = MAX<int>(0, HollywoodEngine::kSceneBufferWidth - dstX);

			if (copyCount != 0) {
				const uint destinationOffset = dstX + dstY * HollywoodEngine::kSceneBufferWidth;
				if (destinationOffset + copyCount <= _sceneFramebuffer.size())
					memcpy(_sceneFramebuffer.data() + destinationOffset, runStreams.data() + cursor + sourceOffset, copyCount);
			}
		}

		cursor += pixelCount;
	}
}

void Scene7010::runEntryCutscene() {
	runSueEntryPath();
	if (_skipRequested || Engine::shouldQuit())
		return;

	beginJuniorSpeech();
	runJuniorSpeech();
	_sceneStateFlags[0] = 1;
}

void Scene7010::runSueEntryPath() {
	const byte cels[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 1, 2, 3, 4, kG01SueEntryFinalCel };
	uint32 chunk8Accumulator = 0;
	uint32 chunk10Accumulator = 0;
	uint32 lastMillis = g_system->getMillis();

	for (uint frame = 0; frame < ARRAYSIZE(cels) && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		if (pollEvents(true))
			return;

		const int x = kG01SueEntryStartX + ((kG01SueEntryTargetX - kG01SueEntryStartX) * (int)frame) / (int)(ARRAYSIZE(cels) - 1);
		const int y = kG01SueEntryStartY + ((kG01SueEntryTargetY - kG01SueEntryStartY) * (int)frame) / (int)(ARRAYSIZE(cels) - 1);
		_activeActorWorldX = x;
		_activeActorWorldY = y;
		_activeActorFacing = kG01SueEntryFacing;
		_activeActorCel = cels[frame];
		drawCutsceneComposite(true, _activeActorFacing, _activeActorCel, _activeActorWorldX, _activeActorWorldY,
			false, 0, 0, 0, 0);
		presentFrame();

		uint32 waited = 0;
		while (waited < kG01ActorPathFrameMillis && !_skipRequested && !Engine::shouldQuit()) {
			if (pollEvents(true))
				return;
			const uint32 slice = MIN<uint32>(10, kG01ActorPathFrameMillis - waited);
			g_system->delayMillis(slice);
			waited += slice;
		}

		const uint32 now = g_system->getMillis();
		const uint32 elapsed = now - lastMillis;
		lastMillis = now;
		chunk8Accumulator += elapsed;
		chunk10Accumulator += elapsed;
		if (chunk8Accumulator >= kG01Chunk8FrameMillis) {
			_chunk8FrameIndex = (_chunk8FrameIndex == 7) ? 0 : _chunk8FrameIndex + 1;
			if ((_chunk8FrameIndex & 1) != 0)
				_chunk9AmbientOverlayFrameIndex ^= 1;
			chunk8Accumulator = 0;
		}
		if (chunk10Accumulator >= kG01Chunk10FrameMillis) {
			_chunk10IdleFrameB = _chunk10IdleFrameB == 0x0b ? 8 : _chunk10IdleFrameB + 1;
			_chunk10IdleFrameD = _chunk10IdleFrameD == 0x0f ? 0x0c : _chunk10IdleFrameD + 1;
			chunk10Accumulator = 0;
		}
	}

	_activeActorWorldX = kG01SueEntryTargetX;
	_activeActorWorldY = kG01SueEntryTargetY;
	_activeActorFacing = kG01SueEntryFacing;
	_activeActorCel = kG01SueEntryFinalCel;
}

void Scene7010::runJuniorSpeech() {
	const byte secondaryFrames[] = { 0, 1, 2, 3, 4, 3, 2, 1 };
	const uint32 speechMillis = _speech.isPlaying() ?
		MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(2800, _speechOverlay.lines.size() * 1500);
	uint32 elapsed = 0;
	uint frameIndex = 0;
	uint32 frameAccumulator = kG01SecondaryActorFrameMillis;
	uint32 chunk8Accumulator = 0;
	uint32 chunk10Accumulator = 0;
	uint32 lastMillis = g_system->getMillis();

	while ((_speech.isPlaying() || elapsed < speechMillis) && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents(true))
			break;

		_activeActorWorldX = kG01SueEntryTargetX;
		_activeActorWorldY = kG01SueEntryTargetY;
		_activeActorFacing = kG01SueEntryFacing;
		_activeActorCel = kG01SueEntryFinalCel;
		_secondaryActorFrame = secondaryFrames[frameIndex % ARRAYSIZE(secondaryFrames)];
		drawPlayableComposite();
		presentFrame();

		g_system->delayMillis(10);
		elapsed += 10;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;
		chunk8Accumulator += delta;
		chunk10Accumulator += delta;
		if (frameAccumulator >= kG01SecondaryActorFrameMillis) {
			++frameIndex;
			frameAccumulator = 0;
		}
		if (chunk8Accumulator >= kG01Chunk8FrameMillis) {
			_chunk8FrameIndex = (_chunk8FrameIndex == 7) ? 0 : _chunk8FrameIndex + 1;
			if ((_chunk8FrameIndex & 1) != 0)
				_chunk9AmbientOverlayFrameIndex ^= 1;
			chunk8Accumulator = 0;
		}
		if (chunk10Accumulator >= kG01Chunk10FrameMillis) {
			_chunk10IdleFrameA = _chunk10IdleFrameA == 3 ? 0 : _chunk10IdleFrameA + 1;
			_chunk10IdleFrameC = _chunk10IdleFrameC == 7 ? 4 : _chunk10IdleFrameC + 1;
			chunk10Accumulator = 0;
		}
	}

	clearSpeechOverlay();
	_speech.stop();
	_secondaryActorFrame = 0;
	drawPlayableComposite();
	presentFrame();
}

bool Scene7010::runBasicGameplayLoop() {
	GameplayLoop loop(_vm, this);
	return loop.run();
}

const SceneHotspotTable &Scene7010::hotspots() const {
	return _hotspots;
}

const Common::Array<byte> &Scene7010::savedFramebuffer() const {
	return _savedFramebuffer;
}

uint16 Scene7010::viewportXOffset() const {
	return kG01InitialViewportXOffset;
}

uint16 Scene7010::viewportYOffset() const {
	return 0;
}

void Scene7010::prepareGameplayLoop() {
	clearAllSpeechOverlays();
	_activeActorWorldX = kG01SueEntryTargetX;
	_activeActorWorldY = kG01SueEntryTargetY;
	_activeActorFacing = kG01SueEntryFacing;
	_activeActorCel = kG01SueEntryFinalCel;
	_secondaryActorFrame = 0;
}

void Scene7010::advanceGameplayLoop(uint32 delta) {
	_chunk8TimerAccumulator += delta;
	while (_chunk8TimerAccumulator >= kG01Chunk8FrameMillis) {
		advanceChunk8Cycle();
		_chunk8TimerAccumulator -= kG01Chunk8FrameMillis;
	}

	_chunk10TimerAccumulator += delta;
	while (_chunk10TimerAccumulator >= kG01Chunk10FrameMillis) {
		advanceChunk10IdleFrames();
		_chunk10TimerAccumulator -= kG01Chunk10FrameMillis;
	}

	if (_speechOverlay.visible) {
		_secondaryActorTimerAccumulator += delta;
		while (_secondaryActorTimerAccumulator >= kG01SecondaryActorFrameMillis) {
			advanceSecondaryActorIdleFrame();
			_secondaryActorTimerAccumulator -= kG01SecondaryActorFrameMillis;
		}
	} else {
		_secondaryActorFrame = 0;
		_secondaryActorTimerAccumulator = 0;
	}

	advanceDialogueOverlay(delta);
}

void Scene7010::drawGameplayFrame() {
	drawPlayableComposite();
}

void Scene7010::presentGameplayFrame(const SceneHoverCaption &hoverCaption) {
	presentFrame(&hoverCaption);
}

bool Scene7010::shouldExitGameplayLoop() const {
	return _vm->gameState().mainFlowStateId != 0x1b62;
}

void Scene7010::handleLeftClick(const GameplayLoopCursorState &state) {
	_vm->cursor()->leaveInteractiveMode();
	processSceneActionClick(state);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void Scene7010::processSceneActionClick(const GameplayLoopCursorState &state) {
	byte itemId = state.resolvedItem;
	SceneVerbActionRecord actionRecord = _hotspots.verbActionRecord(itemId, state.currentStrip);

	if (itemId == 0) {
		if (state.currentStrip != 1)
			return;

		int targetX = state.sceneX;
		int targetY = state.sceneY;
		adjustWalkTargetToFloorMask(targetX, targetY);
		walkActiveActorTo(targetX, targetY, kInvalidFacing, 0);
		return;
	}

	if (actionRecord.actionHandlerId == 0)
		return;

	const SceneActionTarget target = _hotspots.actionTarget(itemId);
	int targetX = target.interactionPoint.x;
	int targetY = target.interactionPoint.y;
	byte finalFacing = target.facing;
	byte finalCel = 0;

	if (actionRecord.movementMode == 0 && (target.approachPoint.x != 0 || target.approachPoint.y != 0)) {
		targetX = target.approachPoint.x;
		targetY = target.approachPoint.y;
		finalFacing = target.facing;
	} else if (actionRecord.movementMode == 3) {
		finalCel = kInvalidCel;
	}

	walkActiveActorTo(targetX, targetY, finalFacing, finalCel);
	dispatchSceneAction(actionRecord.actionHandlerId);
}

void Scene7010::dispatchSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 0:
	case 1:
		break;
	case 9:
		beginStaticSecondarySpeechLine(8, 0);
		break;
	case 10:
		beginStaticSecondarySpeechLine(9, (byte)_random.getRandomNumber(1));
		break;
	case 20:
		beginStaticSecondarySpeechLine(0x13, 0);
		break;
	case 301:
		handleActionSlot00TransitionToG03();
		break;
	case 302:
		handleActionSlot01SecondarySpeech();
		break;
	case 303:
		handleActionSlot02SecondarySpeech();
		break;
	case 304:
		handleActionSlot03DialogueSequence();
		break;
	case 305:
		handleActionSlot04Item06Speech();
		break;
	case 307:
		handleActionSlot06Item0BSequence();
		break;
	case 308:
		handleActionSlot07DialogueAndReturn();
		break;
	case 309:
		handleActionSlot08CommonSpeech();
		break;
	default:
		warning("Unhandled Scene7010 action handler %u", handlerId);
		break;
	}
}

void Scene7010::walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel) {
	if (targetX == _activeActorWorldX && targetY == _activeActorWorldY) {
		if (finalFacing != kInvalidFacing)
			_activeActorFacing = finalFacing;
		if (finalCel != kInvalidCel)
			_activeActorCel = finalCel;
		drawPlayableComposite();
		presentFrame();
		return;
	}

	const int startX = _activeActorWorldX;
	const int startY = _activeActorWorldY;
	byte movementFacing = calculateFacingTowardPoint(startX, startY, targetX, targetY);
	const uint steps = MAX<uint>(1, MAX<int>(ABS(targetX - startX), ABS(targetY - startY)) / 7);

	for (uint step = 1; step <= steps && !Engine::shouldQuit(); ++step) {
		_activeActorWorldX = startX + ((targetX - startX) * (int)step) / (int)steps;
		_activeActorWorldY = startY + ((targetY - startY) * (int)step) / (int)steps;
		_activeActorFacing = movementFacing;
		_activeActorCel = (byte)(1 + (step % 12));
		waitSceneMillis(kG01ActorPathFrameMillis);
	}

	_activeActorWorldX = targetX;
	_activeActorWorldY = targetY;
	if (finalFacing != kInvalidFacing)
		_activeActorFacing = finalFacing;
	else
		_activeActorFacing = movementFacing;
	_activeActorCel = finalCel == kInvalidCel ? 0 : finalCel;
	drawPlayableComposite();
	presentFrame();
}

void Scene7010::adjustWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x16b, 0x268);
	targetY = CLIP<int>(targetY, 0, HollywoodEngine::kSceneBufferHeight - 1);

	while (targetY < 0x1df) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (offset < _savedFramebuffer.size() && _paletteMask[_savedFramebuffer[offset]] != 0)
			return;
		++targetY;
	}

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (offset < _savedFramebuffer.size() && _paletteMask[_savedFramebuffer[offset]] != 0)
			return;
		--targetY;
	}
}

byte Scene7010::calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const {
	if (toX == fromX)
		return fromY < toY ? 3 : 0;

	const float slope = (float)ABS(toY - fromY) / (float)MAX<int>(1, ABS(toX - fromX));
	if (toX > fromX) {
		if (toY < fromY)
			return slope > 1.0f ? 0 : (slope > 0.33f ? 1 : 2);
		return slope > 1.0f ? 3 : 2;
	}

	if (toY < fromY)
		return slope > 1.0f ? 0 : (slope > 0.33f ? 5 : 4);
	return slope > 1.0f ? 3 : 4;
}

void Scene7010::advanceDialogueOverlay(uint32 delta) {
	if (_dialogueOverlayMode == 0)
		return;

	_dialogueOverlayTimerAccumulator += delta;
	while (_dialogueOverlayTimerAccumulator >= kG01DialogueOverlayFrameMillis) {
		_dialogueOverlayTimerAccumulator -= kG01DialogueOverlayFrameMillis;
		if (_dialogueOverlayMode == 1) {
			_dialogueOverlayFrameIndex = _dialogueOverlayFrameIndex == 3 ? 0 : _dialogueOverlayFrameIndex + 1;
		} else {
			_dialogueOverlayFrameIndex = _dialogueOverlayFrameIndex == 0x1b ? 5 : _dialogueOverlayFrameIndex + 1;
		}
	}
}

void Scene7010::applySceneStateToHotspotsAndPatches(byte selector) {
	if ((selector == 2 || selector == 0xff) && _sceneStateFlags[2] == 1) {
		const uint sourceOffset = 5 * kStage003SmallRowSize;
		const uint destinationOffset = 3 * kStage003SmallRowSize;
		if (sourceOffset + kStage003SmallRowSize <= _stage003SmallRows.size() &&
				destinationOffset + kStage003SmallRowSize <= _stage003SmallRows.size()) {
			memcpy(_stage003SmallRows.data() + destinationOffset,
				_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
			_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		}
	}

	if (selector == 4 || selector == 0xff)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x19, _sceneStateFlags[4] == 0 ? 1 : 0);
}

bool Scene7010::hasInventoryItem(byte itemId) const {
	return itemId < ARRAYSIZE(_inventoryItems) && _inventoryItems[itemId];
}

void Scene7010::addInventoryItem(byte itemId) {
	if (itemId < ARRAYSIZE(_inventoryItems))
		_inventoryItems[itemId] = true;
}

void Scene7010::removeInventoryItem(byte itemId) {
	if (itemId < ARRAYSIZE(_inventoryItems))
		_inventoryItems[itemId] = false;
}

void Scene7010::handleActionSlot00TransitionToG03() {
	_vm->gameState().mainFlowStateId = 0x1b76;
}

void Scene7010::handleActionSlot01SecondarySpeech() {
	beginSecondarySpeechLine(1, 0);
}

void Scene7010::handleActionSlot02SecondarySpeech() {
	beginSecondarySpeechLine(2, _sceneStateFlags[1] == 0 ? 0 : 1);
}

void Scene7010::handleActionSlot03DialogueSequence() {
	if (!hasInventoryItem(6)) {
		beginSecondarySpeechLine(3, 0);
		return;
	}

	if (_sceneStateFlags[2] == 0)
		handleActionSlot04Item06Speech();

	beginSecondarySpeechLine(_sceneStateFlags[4] == 1 ? 3 : 4, _sceneStateFlags[4] == 1 ? 1 : 2);
	walkActiveActorTo(0x298, 0x1af, 1, 0);
	_chunk11Visible = true;
	runChunk11FrameRange(0, 0x0e);
	beginPrimarySpeechLine(99, _sceneStateFlags[4] == 1 ? 0 : 2, 0x302, 0xe3, 0x28, 0x16, 0x0b);
	runChunk11FrameRange(0x12, 0x16);
	beginPrimarySpeechLine(99, _sceneStateFlags[4] == 1 ? 1 : 4, 0x2ee, 0xe8, 0x28, 0x16, 0x0b);
	runChunk11FrameRange(0x1a, 0x1e);
	walkActiveActorTo(0x17b, 0x1b2, kInvalidFacing, 0);
	_chunk11Visible = false;
	if (_sceneStateFlags[4] == 1)
		_sceneStateFlags[4] = 2;
	else
		beginSecondarySpeechLine(5, 0);
}

void Scene7010::handleActionSlot04Item06Speech() {
	if (_sceneStateFlags[2] == 0) {
		beginSecondarySpeechLine(4, 0);
		_sceneStateFlags[2] = 1;
		applySceneStateToHotspotsAndPatches(2);
		return;
	}

	beginSecondarySpeechLine(4, hasInventoryItem(6) ? 2 : 1);
}

void Scene7010::handleActionSlot06Item0BSequence() {
	if (_sceneStateFlags[1] == 0) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	runChunk15ItemSequence();
	removeInventoryItem(0x0b);
	runDialogueOverlayFrames(5, 0x1b, 0);
	beginSecondarySpeechLine(6, 4);
}

void Scene7010::handleActionSlot07DialogueAndReturn() {
	beginSecondarySpeechLine(7, 0);
	runChunk8RevealSequence();
	if (_sceneStateFlags[3] == 0) {
		beginPrimarySpeechLine(8, 0, 0xfa, 0x136, 0x33, 0x22, 0x39);
		beginPrimarySpeechLine(8, 4, 0xfa, 0x136, 0x33, 0x22, 0x39);
		_sceneStateFlags[3] = 1;
	} else {
		beginPrimarySpeechLine(8, 1, 0xfa, 0x136, 0x33, 0x22, 0x39);
		beginPrimarySpeechLine(8, 2, 0xfa, 0x136, 0x33, 0x22, 0x39);
	}
	beginSecondarySpeechLine(7, 1);
	runChunk8HideSequence();
	walkActiveActorTo(0x16b, 0x1df, 3, 0);
	beginSecondarySpeechLine(7, 2);
}

void Scene7010::handleActionSlot08CommonSpeech() {
	beginSecondarySpeechLine(9, 0);
}

void Scene7010::runChunk8RevealSequence() {
	_chunk8FrameIndex = 7;
	for (byte frame = 7; frame <= 0x0b && !Engine::shouldQuit(); ++frame) {
		_chunk8FrameIndex = frame;
		waitSceneMillis(kG01Chunk8FrameMillis);
	}
}

void Scene7010::runChunk8HideSequence() {
	for (byte frame = 0x0f; frame <= 0x13 && !Engine::shouldQuit(); ++frame) {
		_chunk8FrameIndex = frame;
		waitSceneMillis(kG01Chunk8FrameMillis);
	}
	_chunk8FrameIndex = _vm->gameState().currentRandomAmbientMusicTrackId == kG01AmbientMusicCueWithoutChunk9 ? 0x14 : 0;
}

void Scene7010::runChunk11FrameRange(byte startFrame, byte endFrame) {
	_chunk11Visible = true;
	for (byte frame = startFrame; frame <= endFrame && !Engine::shouldQuit(); ++frame) {
		_chunk11FrameIndex = frame;
		waitSceneMillis(75);
	}
}

void Scene7010::runChunk14FrameRange(byte startFrame, byte endFrame) {
	_chunk14Visible = true;
	_chunk11Visible = false;
	for (byte frame = startFrame; frame <= endFrame && !Engine::shouldQuit(); ++frame) {
		_chunk14FrameIndex = frame;
		waitSceneMillis(75);
	}
	_chunk14Visible = false;
}

void Scene7010::runChunk15ItemSequence() {
	_chunk15Visible = true;
	for (byte frame = 0; frame <= 0x17 && !Engine::shouldQuit(); ++frame) {
		_chunk15FrameIndex = frame;
		if (frame == 10) {
			beginPrimarySpeechLine(6, 1, 0x20e, 0x109, 0x3f, 0x28, 0x32);
			beginPrimarySpeechLine(6, 2, 0x20e, 0x109, 0x3f, 0x28, 0x32);
			runDialogueOverlayFrames(0, 5, 2);
			beginPrimarySpeechLine(6, 3, 0x20e, 0x109, 0x3f, 0x28, 0x32);
		}
		waitSceneMillis(75);
	}
	_chunk15Visible = false;
}

void Scene7010::runDialogueOverlayFrames(byte startFrame, byte endFrame, byte finalMode) {
	_dialogueOverlayMode = 2;
	_sceneStateFlags[1] = 2;
	for (byte frame = startFrame; frame <= endFrame && !Engine::shouldQuit(); ++frame) {
		_dialogueOverlayFrameIndex = frame;
		waitSceneMillis(kG01DialogueOverlayFrameMillis);
	}
	_dialogueOverlayMode = finalMode;
	_sceneStateFlags[1] = finalMode;
	_dialogueOverlayFrameIndex = finalMode == 0 ? 0 : endFrame;
}

bool Scene7010::waitSceneMillis(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		advanceGameplayLoop(slice);
		drawPlayableComposite();
		presentFrame();
		remaining -= slice;
	}

	return Engine::shouldQuit();
}

void Scene7010::advanceChunk8Cycle() {
	if (_vm->gameState().currentRandomAmbientMusicTrackId == kG01AmbientMusicCueWithoutChunk9) {
		_chunk8FrameIndex = _chunk8FrameIndex == 0x1a ? 0x14 : _chunk8FrameIndex + 1;
		return;
	}

	_chunk8FrameIndex = _chunk8FrameIndex == 7 ? 0 : _chunk8FrameIndex + 1;
	if ((_chunk8FrameIndex & 1) == 0)
		return;

	if (_chunk9AmbientOverlayFrameIndex == 1) {
		_chunk9AmbientOverlayFrameIndex = 0;
	} else {
		++_chunk9AmbientDecisionCounter;
		if ((_chunk9AmbientDecisionCounter % 5) == 0)
			_chunk9AmbientOverlayFrameIndex = 1;
	}
}

void Scene7010::advanceChunk10IdleFrames() {
	++_chunk10DeterministicCounter;
	const byte nextFrameOffset = _chunk10DeterministicCounter & 3;

	if (!_chunk10IdlePairAAltPhase) {
		if (_chunk10IdlePairATicksRemaining == 0) {
			_chunk10IdlePairAAltPhase = true;
			_chunk10IdleFrameA = 0;
			_chunk10IdlePairATicksRemaining = 18;
		} else {
			--_chunk10IdlePairATicksRemaining;
			_chunk10IdleFrameB = 8 + nextFrameOffset;
		}
	} else if (_chunk10IdlePairATicksRemaining == 0) {
		_chunk10IdlePairAAltPhase = false;
		_chunk10IdleFrameB = 8;
		_chunk10IdlePairATicksRemaining = 18;
	} else {
		--_chunk10IdlePairATicksRemaining;
		_chunk10IdleFrameA = nextFrameOffset;
	}

	if (!_chunk10IdlePairBAltPhase) {
		if (_chunk10IdlePairBTicksRemaining == 0) {
			_chunk10IdlePairBAltPhase = true;
			_chunk10IdleFrameC = 4;
			_chunk10IdlePairBTicksRemaining = 14;
		} else {
			--_chunk10IdlePairBTicksRemaining;
			_chunk10IdleFrameD = 0x0c + nextFrameOffset;
		}
	} else if (_chunk10IdlePairBTicksRemaining == 0) {
		_chunk10IdlePairBAltPhase = false;
		_chunk10IdleFrameD = 0x0c;
		_chunk10IdlePairBTicksRemaining = 14;
	} else {
		--_chunk10IdlePairBTicksRemaining;
		_chunk10IdleFrameC = 4 + nextFrameOffset;
	}
}

void Scene7010::advanceSecondaryActorIdleFrame() {
	if (_secondaryActorFrame == 4) {
		_secondaryActorFrame = 0;
		return;
	}

	++_secondaryActorIdleTick;
	if ((_secondaryActorIdleTick % 15) == 0)
		_secondaryActorFrame = 4;
}

void Scene7010::beginJuniorSpeech() {
	clearSpeechOverlay();
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(0, 0, textRecordId, continuationCount, voiceSampleId))
		return;

	if (voiceSampleId != 0)
		_speech.playSample(voiceSampleId, 100);

	const Common::String text = getResource003LargeTextRecord(textRecordId);
	if (text.empty())
		return;

	wrapActorSpeechText(text, kG01SueEntryTargetX, _speechOverlay.lines);
	if (_speechOverlay.lines.empty())
		return;

	_speechOverlay.visible = true;
	_speechOverlay.colorIndex = kG01SecondarySpeechTextColor;
	calculateSecondarySpeechBounds(kG01SueEntryTargetX, kG01SueEntryTargetY);
}

void Scene7010::clearSpeechOverlay() {
	_speechOverlay.visible = false;
	_speechOverlay.lines.clear();
}

void Scene7010::clearAllSpeechOverlays() {
	clearSpeechOverlay();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
}

void Scene7010::drawSpeechOverlay() {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return;

	drawSpeechOverlay(_speechOverlay);
	drawSpeechOverlay(_primarySpeechOverlay);
}

void Scene7010::drawSpeechOverlay(const SpeechOverlay &overlay) {
	if (!overlay.visible)
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);

	Graphics::Surface screenSurface;
	screenSurface.init(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight,
		HollywoodEngine::kScreenWidth, _screen.data(), Graphics::PixelFormat::createFormatCLUT8());

	for (uint lineIndex = 0; lineIndex < overlay.lines.size(); ++lineIndex) {
		const Common::String &line = overlay.lines[lineIndex];
		const int lineWidth = actorSpeechTextWidth(line);
		const int x = (int)overlay.centerX - (lineWidth >> 1) - kG01InitialViewportXOffset;
		const int y = (int)overlay.topY + lineIndex * kOriginalSpeechLineHeight;
		font->drawString(&screenSurface, line, x, y, lineWidth, overlay.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene7010::beginSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	runSpeechLine(_speechOverlay, rowIndex, frameIndex, _activeActorWorldX, 0,
		kG01SecondarySpeechTextColor, false);
}

void Scene7010::beginStaticSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(_speechOverlay, textRecordId, continuationCount, voiceSampleId, _activeActorWorldX, 0,
		kG01SecondarySpeechTextColor, false);
}

void Scene7010::beginPrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue) {
	const uint paletteOffset = kG01PrimarySpeechTextColor * 3;
	if (_paletteCurrent.size() > paletteOffset + 2) {
		_paletteCurrent[paletteOffset] = red;
		_paletteCurrent[paletteOffset + 1] = green;
		_paletteCurrent[paletteOffset + 2] = blue;
	}

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, centerX, topY,
		kG01PrimarySpeechTextColor, true);
}

void Scene7010::runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte colorIndex, bool useRequestedTop) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(overlay, textRecordId, continuationCount, voiceSampleId, centerX, topY, colorIndex, useRequestedTop);
}

void Scene7010::runSpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount,
		uint16 voiceSampleId, uint16 centerX, uint16 topY, byte colorIndex, bool useRequestedTop) {
	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !Engine::shouldQuit(); ++part) {
		const Common::String text = getResource003LargeTextRecord(textRecordId + part);
		if (text.empty())
			continue;

		overlay.visible = true;
		overlay.colorIndex = colorIndex;
		wrapActorSpeechText(text, centerX, overlay.lines);
		if (useRequestedTop) {
			overlay.centerX = centerX;
			overlay.topY = topY;
		} else {
			calculateSecondarySpeechBounds(centerX, _activeActorWorldY);
		}

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, overlay.lines.size() * 1100);
		const bool interrupted = waitForSpeechOrDelay(duration);
		_speech.stop();
		overlay.visible = false;
		overlay.lines.clear();
		if (interrupted)
			break;
	}
}

bool Scene7010::getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 100) * 5;
	if (offset + 5 > _stage003StageBlock.size())
		return false;

	textRecordId = readUint16LE(_stage003StageBlock, offset);
	continuationCount = _stage003StageBlock[offset + 2];
	voiceSampleId = readUint16LE(_stage003StageBlock, offset + 3);
	return textRecordId != 0;
}

bool Scene7010::getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 10) * 5;
	if (offset + 5 > _owner1SpeechCueDescriptors.size())
		return false;

	textRecordId = readUint16LE(_owner1SpeechCueDescriptors, offset);
	continuationCount = _owner1SpeechCueDescriptors[offset + 2];
	voiceSampleId = readUint16LE(_owner1SpeechCueDescriptors, offset + 3);
	return textRecordId != 0;
}

void Scene7010::wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars = 0x32;
	const int anchorX = anchorSceneX - kG01InitialViewportXOffset;
	if (anchorX < 0xa0)
		maxChars = (anchorX * 0x32) / 0xa0;
	else if (HollywoodEngine::kScreenWidth - anchorX < 0xa0)
		maxChars = ((HollywoodEngine::kScreenWidth - anchorX) * 0x32) / 0xa0;
	maxChars = MAX<uint>(maxChars, 0x18);

	const uint lineShrink = maxChars < 0x2a ? (maxChars > 0x20 ? 2 : 1) : 3;
	const char *source = text.c_str();
	const uint textLength = text.size();
	uint cursor = 0;
	while (cursor < textLength && lines.size() < 10) {
		uint end = textLength;
		if (cursor + maxChars < textLength) {
			end = cursor + maxChars;
			while (end > cursor && (byte)source[end] != 0x20 && source[end] != 0)
				--end;
			while (end > cursor && (byte)source[end - 1] == 0x20)
				--end;
			if (end == cursor)
				end = MIN<uint>(textLength, cursor + maxChars);
		}

		lines.push_back(Common::String(source + cursor, end - cursor));

		cursor = end;
		while (cursor < textLength && (byte)source[cursor] == 0x20)
			++cursor;

		maxChars = maxChars > lineShrink ? maxChars - lineShrink : 1;
	}
}

Common::String Scene7010::getResource003LargeTextRecord(uint16 recordId) const {
	if (recordId < kStage003LargeRowBaseIndex) {
		const uint offset = (uint)recordId * kStage003LargeRowSize;
		if (recordId == 0 || offset >= _owner1LargeRows.size())
			return Common::String();

		const byte *row = _owner1LargeRows.data() + offset;
		uint length = 0;
		while (length < kStage003LargeRowSize && row[length] != 0)
			++length;

		return Common::String((const char *)row, length);
	}

	const uint localRecordId = recordId - kStage003LargeRowBaseIndex;
	const uint offset = localRecordId * kStage003LargeRowSize;
	if (offset >= _stage003LargeRows.size())
		return Common::String();

	const byte *row = _stage003LargeRows.data() + offset;
	uint length = 0;
	while (length < kStage003LargeRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

uint Scene7010::actorSpeechTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void Scene7010::calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY) {
	uint textWidth = 0;
	for (uint i = 0; i < _speechOverlay.lines.size(); ++i)
		textWidth = MAX<uint>(textWidth, actorSpeechTextWidth(_speechOverlay.lines[i]));

	int centerX = actorWorldX;
	if (((centerX - (int)(textWidth >> 1)) - 1 + (int)textWidth) > 0x27e)
		centerX = (textWidth & 1) == 0 ? 0x27e - (textWidth >> 1) : 0x27d - (textWidth >> 1);
	if (centerX - (int)(textWidth >> 1) < 1)
		centerX = (textWidth >> 1) + 1;

	int topY = actorWorldY - (int)_speechOverlay.lines.size() * kOriginalSpeechLineHeight - 0xbe;
	if (topY < 1)
		topY = 1;

	_speechOverlay.centerX = (uint16)centerX;
	_speechOverlay.topY = (uint16)topY;
}

bool Scene7010::waitForSpeechOrDelay(uint32 fallbackMillis) {
	uint32 elapsed = 0;
	while (!Engine::shouldQuit()) {
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsed >= fallbackMillis)
			break;

		const uint32 slice = speechActive ? 50 : MIN<uint32>(50, fallbackMillis - elapsed);
		if (waitSceneMillis(slice))
			return true;
		elapsed += slice;
	}

	return Engine::shouldQuit();
}

void Scene7010::presentFrame(const SceneHoverCaption *hoverCaption) {
	if (hoverCaption)
		hoverCaption->applyPalette(_paletteCurrent);
	uploadPalette6Bit(_paletteCurrent);

	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		const uint sourceOffset = kG01InitialViewportXOffset + y * HollywoodEngine::kSceneBufferWidth;
		memcpy(_screen.data() + y * HollywoodEngine::kScreenWidth,
			_sceneFramebuffer.data() + sourceOffset,
			HollywoodEngine::kScreenWidth);
	}

	drawSpeechOverlay();
	if (_vm->font() && _vm->font()->isLoaded()) {
		Graphics::Surface screenSurface;
		screenSurface.init(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight,
			HollywoodEngine::kScreenWidth, _screen.data(), Graphics::PixelFormat::createFormatCLUT8());
		if (hoverCaption)
			hoverCaption->draw(screenSurface, *_vm->font());
	}

	g_system->copyRectToScreen(_screen.data(), HollywoodEngine::kScreenWidth, 0, 0,
		HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight);
	g_system->updateScreen();
}

bool Scene7010::pollEvents(bool allowSkip) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			return true;
		case Common::EVENT_KEYDOWN:
			if (allowSkip && (event.kbd.keycode == Common::KEYCODE_ESCAPE ||
					event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_SPACE)) {
				_skipRequested = true;
				return true;
			}
			break;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
			break;
		default:
			break;
		}
	}

	return false;
}

bool Scene7010::delay(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !_skipRequested && !Engine::shouldQuit()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;
	}

	return _skipRequested || Engine::shouldQuit();
}

} // End of namespace Hollywood
