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
#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

#include <math.h>

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
const byte kG01PanelDarkColor = 0xe7;
const byte kG01PanelFillColor = 0xe8;
const byte kG01PanelSlotColor = 0xe9;
const byte kG01PanelLineColor = 0xea;
const byte kG01PanelSelectedColor = 0xf1;
const byte kG01PanelSelectedLineColor = 0xf2;
const byte kG01PanelTextColor = 0xfc;
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
const byte kActorPathStepDeltaTableSet00[] = {
	2, 2, 2, 3, 3, 0, 2, 2, 2, 3, 3, 0,
	6, 7, 7, 5, 5, 5, 4, 6, 6, 5, 3, 4,
	5, 5, 5, 6, 2, 3, 4, 8, 10, 6, 3, 5,
	3, 0, 0, 3, 3, 3, 3, 0, 0, 3, 3, 3,
	4, 8, 10, 6, 3, 5, 5, 5, 5, 6, 2, 3,
	4, 6, 6, 5, 3, 4, 6, 7, 7, 5, 5, 5
};
const byte kActorFacingTurnTable[] = {
	0, 0, 0, 1, 0, 0, 1, 2, 0, 1, 2, 3, 5, 4, 0, 5, 0, 0,
	0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 3, 0, 0, 5, 4, 0, 5, 0,
	1, 0, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 3, 4, 0, 1, 0, 5,
	4, 5, 0, 2, 1, 0, 2, 0, 0, 0, 0, 0, 4, 0, 0, 4, 5, 0,
	5, 0, 0, 5, 0, 1, 3, 2, 0, 3, 0, 0, 0, 0, 0, 5, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 1, 2, 4, 3, 0, 4, 0, 0, 0, 0, 0
};
const int8 kActorPathAxisDirectionByFacing[] = { -1, 1, 1, 1, -1, -1 };
const byte kActorInitialCelByFacing[] = { 0, 12, 12, 12, 1, 12 };
const float kActorPathDiagonalSlopeThreshold = 0.087488f;
const float kActorFacingSteepSlopeThreshold = 3.732051f;
const float kActorFacingDiagonalSlopeThreshold = 0.267949f;

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
		_primaryLeftSpeechLastFrame(0),
		_chunk11Visible(false),
		_chunk14Visible(false),
		_chunk15Visible(false),
		_chunk10IdlePairAAltPhase(false),
		_chunk10IdlePairBAltPhase(false),
		_primaryLeftSpeechActive(false),
		_chunk10IdlePairATicksRemaining(10),
		_chunk10IdlePairBTicksRemaining(16),
		_chunk9AmbientDecisionCounter(0),
		_chunk8TimerAccumulator(0),
		_chunk10TimerAccumulator(0),
		_secondaryActorTimerAccumulator(0),
		_dialogueOverlayTimerAccumulator(0),
		_primaryLeftSpeechTimerAccumulator(0),
		_activeActorWorldX(kG01SueEntryTargetX),
		_activeActorWorldY(kG01SueEntryTargetY),
		_activeActorFacing(kG01SueEntryFacing),
		_activeActorCel(kG01SueEntryFinalCel),
		_activeActorDrawOrderMode(0),
		_secondaryActorFrame(0),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_savedFramebuffer.resize(kFrameBufferSize);
	_fullPaletteRegionMask.resize(kG01PaletteMaskUsedBytes);
	_walkablePaletteMask.resize(kG01PaletteMaskUsedBytes);
	_screen.resize(HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight);
	_activeActorRunStreams.resize(kActorFacingCount * kActiveActorFacingRunStride);
	_secondaryActorRunStreams.resize(kActorFacingCount * kSecondaryActorFacingRunStride);
	_activeActorDescriptors.resize(kActorFacingCount * kActorCelsPerFacing);
	_secondaryActorDescriptors.resize(kActorFacingCount * kSecondaryActorFramesPerFacing);
	_stage003DecodeKey.resize(kStage003DecodeKeySize);
	_stage003StageBlock.resize(kStage003DescriptorTableSize);
	_owner1SpeechCueDescriptors.resize(kOwner1SpeechCueDescriptorTableSize);
	_routeBoundaryPoints.resize(kSceneRouteBoundaryPointCount);
	_routeSteps.resize(kSceneRouteStepCount);
	_actorPathStepDeltas.resize(ARRAYSIZE(kActorPathStepDeltaTableSet00));
	memcpy(_actorPathStepDeltas.data(), kActorPathStepDeltaTableSet00, _actorPathStepDeltas.size());
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
	if (!_panelArt.load())
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
			!loadFixedChunk(1, _paletteResource, kPaletteSize) ||
			!loadVariableChunk(2, _fillRuns) ||
			!loadVariableChunk(3, _paletteMask) ||
			!loadVariableChunk(4, _metadata))
		return false;

	if (_paletteMask.size() < kG01PaletteMaskUsedBytes) {
		warning("%s chunk 3 is shorter than the G01 palette mask table", kG01ArchiveName);
		return false;
	}
	if (!initializeScenePathTables())
		return false;

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

bool Scene7010::initializeScenePathTables() {
	const uint boundaryBytes = kSceneRouteBoundaryPointCount * 4;
	if (_metadata.size() < kRouteBoundaryPoints + boundaryBytes ||
			_metadata.size() < kRouteBoundarySteps + kSceneRouteStepCount) {
		warning("%s chunk 4 is too short for G01 path route tables", kG01ArchiveName);
		return false;
	}

	memcpy(_fullPaletteRegionMask.data(), _paletteMask.data(), _fullPaletteRegionMask.size());
	memcpy(_walkablePaletteMask.data(), _paletteMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 1)
			_walkablePaletteMask[i] = 0;
	}

	for (uint i = 0; i < _routeBoundaryPoints.size(); ++i) {
		const uint offset = kRouteBoundaryPoints + i * 4;
		_routeBoundaryPoints[i].x = readSint16LE(_metadata, offset);
		_routeBoundaryPoints[i].y = readSint16LE(_metadata, offset + 2);
	}
	memcpy(_routeSteps.data(), _metadata.data() + kRouteBoundarySteps, _routeSteps.size());
	return true;
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
	_primaryLeftSpeechLastFrame = 0;
	_chunk11Visible = false;
	_chunk14Visible = false;
	_chunk15Visible = false;
	_chunk10IdlePairAAltPhase = _random.getRandomNumber(1) != 0;
	_chunk10IdlePairBAltPhase = _random.getRandomNumber(1) != 0;
	_primaryLeftSpeechActive = false;
	_chunk10IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk10IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk9AmbientDecisionCounter = 0;
	_chunk8TimerAccumulator = 0;
	_chunk10TimerAccumulator = 0;
	_secondaryActorTimerAccumulator = 0;
	_dialogueOverlayTimerAccumulator = 0;
	_primaryLeftSpeechTimerAccumulator = 0;
	_activeActorWorldX = kG01SueEntryTargetX;
	_activeActorWorldY = kG01SueEntryTargetY;
	_activeActorFacing = kG01SueEntryFacing;
	_activeActorCel = kG01SueEntryFinalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
}

void Scene7010::drawPreviewComposite() {
	drawCutsceneComposite(false, 0, 0, 0, 0, false, 0, 0, 0, 0);
}

void Scene7010::drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
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

	if (drawSecondaryActor) {
		const int secondaryActorBottomY = drawSecondaryActorFrame(secondaryFacing, secondaryFrame,
			secondaryWorldX, secondaryWorldY);
		if (drawActiveActor)
			drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, secondaryActorBottomY);
	} else if (drawActiveActor) {
		drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, -1);
	}

	if (actorDrawOrderMode == 1) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	} else {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	}
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
		drawSecondaryActor, _activeActorFacing, _secondaryActorFrame, _activeActorWorldX, _activeActorWorldY,
		_activeActorDrawOrderMode);
}

void Scene7010::drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex) {
	if (chunkIndex >= HollywoodEngine::kResourceChunkCount || frameIndex >= frameMapSize)
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[chunkIndex], 0,
		descriptorCount, frameMap[frameIndex], _sceneFramebuffer);
}

void Scene7010::drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY, int minimumYExclusive) {
	if (facing >= kActorFacingCount || cel >= kActorCelsPerFacing)
		return;

	const uint descriptorIndex = facing * kActorCelsPerFacing + cel;
	if (descriptorIndex >= _activeActorDescriptors.size())
		return;

	const ActiveActorSpriteDescriptor &descriptor = _activeActorDescriptors[descriptorIndex];
	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	drawActorRun(_activeActorRunStreams, descriptor.runStreamOffset, facing * kActiveActorFacingRunStride,
		descriptor.opaqueRunCount, spriteX, spriteY, minimumYExclusive);
}

int Scene7010::drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY) {
	if (facing >= kActorFacingCount || frame >= kSecondaryActorFramesPerFacing)
		return -1;

	const uint descriptorIndex = facing * kSecondaryActorFramesPerFacing + frame;
	if (descriptorIndex >= _secondaryActorDescriptors.size())
		return -1;

	const SecondaryActorSpriteDescriptor &descriptor = _secondaryActorDescriptors[descriptorIndex];
	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	return drawActorRun(_secondaryActorRunStreams, descriptor.runStreamOffset, facing * kSecondaryActorFacingRunStride,
		descriptor.runCount, spriteX, spriteY, -1);
}

int Scene7010::drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive) {
	return drawActorRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
		minimumYExclusive, _sceneFramebuffer, nullptr);
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
		_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
		drawCutsceneComposite(true, _activeActorFacing, _activeActorCel, _activeActorWorldX, _activeActorWorldY,
			false, 0, 0, 0, 0, _activeActorDrawOrderMode);
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
			advanceChunk10IdleFrames();
			chunk10Accumulator = 0;
		}
	}

	_activeActorWorldX = kG01SueEntryTargetX;
	_activeActorWorldY = kG01SueEntryTargetY;
	_activeActorFacing = kG01SueEntryFacing;
	_activeActorCel = kG01SueEntryFinalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene7010::runJuniorSpeech() {
	const uint32 speechMillis = _speech.isPlaying() ?
		MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(2800, _speechOverlay.lines.size() * 1500);
	uint32 elapsed = 0;
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
		_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
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
			advanceSecondaryActorSpeechFrame();
			frameAccumulator = 0;
		}
		if (chunk8Accumulator >= kG01Chunk8FrameMillis) {
			_chunk8FrameIndex = (_chunk8FrameIndex == 7) ? 0 : _chunk8FrameIndex + 1;
			if ((_chunk8FrameIndex & 1) != 0)
				_chunk9AmbientOverlayFrameIndex ^= 1;
			chunk8Accumulator = 0;
		}
		if (chunk10Accumulator >= kG01Chunk10FrameMillis) {
			advanceChunk10IdleFrames();
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
	_primaryLeftSpeechActive = false;
	_primaryLeftSpeechTimerAccumulator = 0;
	_activeActorWorldX = kG01SueEntryTargetX;
	_activeActorWorldY = kG01SueEntryTargetY;
	_activeActorFacing = kG01SueEntryFacing;
	_activeActorCel = kG01SueEntryFinalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
}

void Scene7010::advanceGameplayLoop(uint32 delta) {
	if (_primaryLeftSpeechActive && _primarySpeechOverlay.visible) {
		_primaryLeftSpeechTimerAccumulator += delta;
		while (_primaryLeftSpeechTimerAccumulator >= kG01Chunk10FrameMillis) {
			advancePrimaryLeftSpeechFrame();
			_primaryLeftSpeechTimerAccumulator -= kG01Chunk10FrameMillis;
		}
	} else {
		_primaryLeftSpeechTimerAccumulator = 0;
		_chunk8TimerAccumulator += delta;
		while (_chunk8TimerAccumulator >= kG01Chunk8FrameMillis) {
			advanceChunk8Cycle();
			_chunk8TimerAccumulator -= kG01Chunk8FrameMillis;
		}
	}

	_chunk10TimerAccumulator += delta;
	while (_chunk10TimerAccumulator >= kG01Chunk10FrameMillis) {
		advanceChunk10IdleFrames();
		_chunk10TimerAccumulator -= kG01Chunk10FrameMillis;
	}

	if (_speechOverlay.visible) {
		_secondaryActorTimerAccumulator += delta;
		while (_secondaryActorTimerAccumulator >= kG01SecondaryActorFrameMillis) {
			advanceSecondaryActorSpeechFrame();
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

void Scene7010::presentGameplayFrame(const SceneHoverCaption &hoverCaption, const GameplayPanelState &panelState) {
	presentFrame(&hoverCaption, &panelState);
}

void Scene7010::prepareOptionsMenuPalette(Common::Array<byte> &palette) const {
	palette = _paletteCurrent;
	_panelArt.applyPalette(palette);
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
	byte finalFacing = kInvalidFacing;
	byte finalCel = kInvalidCel;

	if (actionRecord.movementMode == 0) {
		const bool atInteractionPoint =
			_activeActorWorldX == target.interactionPoint.x &&
			_activeActorWorldY == target.interactionPoint.y;
		if (atInteractionPoint) {
			if (_activeActorFacing != target.facing)
				finalFacing = target.facing;
		} else {
			targetX = _activeActorWorldX;
			targetY = _activeActorWorldY;
			if (target.approachPoint.x != 0 || target.approachPoint.y != 0) {
				finalFacing = calculateFacingTowardPoint(_activeActorWorldX, _activeActorWorldY,
					target.approachPoint.x, target.approachPoint.y);
			}
		}
	}
	if (actionRecord.movementMode == 1)
		finalFacing = target.facing;
	if (actionRecord.movementMode != 3)
		finalCel = 0;

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
	queueActorPathWithPaletteRegionRouting(_activeActorWorldX, _activeActorWorldY, targetX, targetY,
		finalFacing, finalCel);

	if (_actorPathFrames.size() <= 1) {
		drawPlayableComposite();
		presentFrame();
		return;
	}

	for (uint frameIndex = 1; frameIndex < _actorPathFrames.size() && !Engine::shouldQuit(); ++frameIndex) {
		const ActorPathFrame &frame = _actorPathFrames[frameIndex];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		waitSceneMillis(kG01ActorPathFrameMillis);
	}

	drawPlayableComposite();
	presentFrame();
}

void Scene7010::adjustWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x16b, 0x268);

	while (targetY < 0x1df) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (offset < _savedFramebuffer.size() && _walkablePaletteMask[_savedFramebuffer[offset]] != 0)
			return;
		++targetY;
	}

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (offset < _savedFramebuffer.size() && _walkablePaletteMask[_savedFramebuffer[offset]] != 0)
			return;
		--targetY;
	}
}

void Scene7010::queueActorPathWithPaletteRegionRouting(int startX, int startY, int targetX, int targetY,
		byte finalFacing, byte finalCel) {
	_actorPathFrames.clear();
	memcpy(_actorPathStepDeltas.data(), kActorPathStepDeltaTableSet00, _actorPathStepDeltas.size());

	ActorPathBuildState state;
	state.drawOrderMode = _activeActorDrawOrderMode;
	state.facing = _activeActorFacing;
	state.cel = nextActorPathCel(_activeActorCel);
	state.x = startX;
	state.y = startY;
	appendActorPathFrame(state);

	byte currentRegion = paletteRegionAt(startX, startY);
	if (currentRegion == 0)
		currentRegion = _activeActorDrawOrderMode;

	byte targetRegion = paletteRegionAt(targetX, targetY);
	if (targetRegion == 0)
		targetRegion = currentRegion;

	if (currentRegion != targetRegion) {
		for (uint stepIndex = 0; stepIndex < kScenePaletteRegionRouteStepCount &&
				currentRegion != targetRegion; ++stepIndex) {
			const uint routeOffset =
				((uint)currentRegion * kScenePaletteRegionCount + targetRegion) *
				kScenePaletteRegionRouteStepCount + stepIndex;
			if (routeOffset >= _routeSteps.size())
				break;

			const byte nextRegion = _routeSteps[routeOffset];
			if (nextRegion == 0 || nextRegion >= kScenePaletteRegionCount)
				break;

			state.drawOrderMode = currentRegion;
			const ScenePoint boundary = nextRegion == targetRegion ?
				bestPaletteRouteBoundaryPoint(state.x, state.y, targetX, targetY, currentRegion, nextRegion) :
				nearestPaletteRouteBoundaryPoint(state.x, state.y, currentRegion, nextRegion);

			byte segmentFinalFacing = kInvalidFacing;
			byte segmentFinalCel = kInvalidCel;
			if (boundary.x == targetX && boundary.y == targetY) {
				segmentFinalFacing = finalFacing;
				segmentFinalCel = finalCel;
			}

			int requestedFacing = -1;
			bool restoredStepDeltas = false;
			if (currentRegion == 3 && nextRegion == 2) {
				memcpy(_actorPathStepDeltas.data() + 12, kActorPathStepDeltaTableSet00 + 48, 12);
				requestedFacing = 1;
				restoredStepDeltas = true;
			}
			if (currentRegion == 2 && nextRegion == 1 &&
					state.x < boundary.x && boundary.y <= state.y)
				requestedFacing = 1;

			buildActorPathFramesBetweenPoints(state, boundary.x, boundary.y,
				segmentFinalFacing, segmentFinalCel, requestedFacing);
			if (restoredStepDeltas)
				memcpy(_actorPathStepDeltas.data(), kActorPathStepDeltaTableSet00, _actorPathStepDeltas.size());

			currentRegion = nextRegion;
		}
	}

	int requestedFacing = -1;
	if (currentRegion == 3 && targetRegion == 3)
		requestedFacing = 4;
	if (currentRegion == 1 && targetRegion == 1 && state.x < targetX && targetY <= state.y)
		requestedFacing = 1;

	state.drawOrderMode = currentRegion;
	buildActorPathFramesBetweenPoints(state, targetX, targetY, finalFacing, finalCel, requestedFacing);

	if (!_actorPathFrames.empty()) {
		const ActorPathFrame &lastFrame = _actorPathFrames.back();
		_activeActorWorldX = lastFrame.worldX;
		_activeActorWorldY = lastFrame.worldY;
		_activeActorFacing = lastFrame.facing;
		_activeActorCel = lastFrame.cel;
		_activeActorDrawOrderMode = lastFrame.drawOrderMode;
	}
}

void Scene7010::buildActorPathFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
		byte finalFacing, byte finalCel, int requestedFacing) {
	if (targetX == state.x && targetY == state.y) {
		if (finalFacing != kInvalidFacing && state.facing != finalFacing) {
			for (uint turnStep = 0; turnStep < 3 && state.facing != finalFacing; ++turnStep) {
				const uint turnOffset = ((uint)state.facing * kActorFacingCount + finalFacing) * 3 + turnStep;
				state.facing = kActorFacingTurnTable[turnOffset];
				state.cel = kActorInitialCelByFacing[state.facing];
				appendActorPathFrame(state);
				state.cel = nextActorPathCel(state.cel);
			}
		}
		if (finalCel != kInvalidCel)
			state.cel = finalCel;
		appendActorPathFrame(state);
		state.cel = nextActorPathCel(state.cel);
		return;
	}

	const byte movementFacing = calculateMovementFacingForPath(state.x, state.y, targetX, targetY, requestedFacing);
	if (state.facing != movementFacing) {
		for (uint turnStep = 0; turnStep < 3 && state.facing != movementFacing; ++turnStep) {
			const uint turnOffset = ((uint)state.facing * kActorFacingCount + movementFacing) * 3 + turnStep;
			state.facing = kActorFacingTurnTable[turnOffset];
			state.cel = kActorInitialCelByFacing[state.facing];
			appendActorPathFrame(state);
			state.cel = nextActorPathCel(state.cel);
		}
	}

	const int startX = state.x;
	const int startY = state.y;
	const int principalStart = (movementFacing == 0 || movementFacing == 3) ? startY : startX;
	const int principalTarget = (movementFacing == 0 || movementFacing == 3) ? targetY : targetX;
	const uint stepCount = calculateWalkStepCountForAxisDelta(principalStart, principalTarget,
		movementFacing, state.cel);

	if (stepCount != 0) {
		for (uint step = 1; step <= stepCount; ++step) {
			const int delta = actorPathStepDelta(movementFacing, state.cel);
			if (movementFacing == 0 || movementFacing == 3) {
				state.y += (startY < targetY) ? delta : -delta;
				state.x = startX + ((targetX - startX) * (int)step) / (int)stepCount;
			} else {
				state.x += (startX < targetX) ? delta : -delta;
				state.y = startY + ((targetY - startY) * (int)step) / (int)stepCount;
			}
			state.facing = movementFacing;
			appendActorPathFrame(state);
			state.cel = nextActorPathCel(state.cel);
		}
	}

	state.x = targetX;
	state.y = targetY;
	if (finalFacing != kInvalidFacing && state.facing != finalFacing) {
		for (uint turnStep = 0; turnStep < 3 && state.facing != finalFacing; ++turnStep) {
			const uint turnOffset = ((uint)state.facing * kActorFacingCount + finalFacing) * 3 + turnStep;
			state.facing = kActorFacingTurnTable[turnOffset];
			state.cel = kActorInitialCelByFacing[state.facing];
			appendActorPathFrame(state);
			state.cel = nextActorPathCel(state.cel);
		}
	}
	if (finalCel != kInvalidCel)
		state.cel = finalCel;
	appendActorPathFrame(state);
	state.cel = nextActorPathCel(state.cel);
}

void Scene7010::appendActorPathFrame(const ActorPathBuildState &state) {
	ActorPathFrame frame;
	frame.drawOrderMode = state.drawOrderMode;
	frame.facing = state.facing;
	frame.cel = state.cel;
	frame.worldX = (int16)CLIP<int>(state.x, -32768, 32767);
	frame.worldY = (int16)CLIP<int>(state.y, -32768, 32767);
	_actorPathFrames.push_back(frame);
}

ScenePoint Scene7010::nearestPaletteRouteBoundaryPoint(int startX, int startY, byte currentRegion, byte nextRegion) const {
	ScenePoint bestPoint;
	memset(&bestPoint, 0, sizeof(bestPoint));

	const uint baseIndex = ((uint)currentRegion * kScenePaletteRegionCount + nextRegion) *
		kScenePaletteRegionBoundaryCandidateCount;
	float bestScore = 0.0f;
	for (uint candidate = 0; candidate < kScenePaletteRegionBoundaryCandidateCount; ++candidate) {
		const uint pointIndex = baseIndex + candidate;
		if (pointIndex >= _routeBoundaryPoints.size())
			break;

		const ScenePoint point = _routeBoundaryPoints[pointIndex];
		const float score = sqrtf((float)ABS(startX - point.x)) + sqrtf((float)ABS(startY - point.y));
		if (candidate == 0 || score < bestScore) {
			bestScore = score;
			bestPoint = point;
		}
	}

	return bestPoint;
}

ScenePoint Scene7010::bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
		byte currentRegion, byte targetRegion) const {
	ScenePoint bestPoint;
	memset(&bestPoint, 0, sizeof(bestPoint));

	const uint baseIndex = ((uint)currentRegion * kScenePaletteRegionCount + targetRegion) *
		kScenePaletteRegionBoundaryCandidateCount;
	float bestScore = 0.0f;
	for (uint candidate = 0; candidate < kScenePaletteRegionBoundaryCandidateCount; ++candidate) {
		const uint pointIndex = baseIndex + candidate;
		if (pointIndex >= _routeBoundaryPoints.size())
			break;

		const ScenePoint point = _routeBoundaryPoints[pointIndex];
		const float score =
			sqrtf((float)ABS(startX - point.x)) +
			sqrtf((float)ABS(startY - point.y)) +
			sqrtf((float)ABS(targetX - point.x)) +
			sqrtf((float)ABS(targetY - point.y));
		if (candidate == 0 || score < bestScore) {
			bestScore = score;
			bestPoint = point;
		}
	}

	return bestPoint;
}

byte Scene7010::paletteRegionAt(int x, int y) const {
	if (x < 0 || y < 0 || x >= HollywoodEngine::kSceneBufferWidth || y >= HollywoodEngine::kSceneBufferHeight ||
			_fullPaletteRegionMask.empty())
		return 0;

	const uint offset = y * HollywoodEngine::kSceneBufferWidth + x;
	if (offset >= _savedFramebuffer.size())
		return 0;

	return _fullPaletteRegionMask[_savedFramebuffer[offset]];
}

byte Scene7010::calculateMovementFacingForPath(int fromX, int fromY, int toX, int toY, int requestedFacing) const {
	if (requestedFacing >= 0)
		return (byte)requestedFacing;

	if (toX == fromX)
		return fromY < toY ? 3 : 0;

	const float slope = (float)ABS(toY - fromY) / (float)MAX<int>(1, ABS(toX - fromX));
	if (fromX < toX) {
		if (toY < fromY) {
			if (slope < 1.0f)
				return slope <= kActorPathDiagonalSlopeThreshold ? 2 : 1;
			return 0;
		}
		return slope < 1.0f ? 2 : 3;
	}

	if (toY < fromY) {
		if (slope > 1.0f)
			return 0;
		return slope > kActorPathDiagonalSlopeThreshold ? 5 : 4;
	}
	return slope > 1.0f ? 3 : 4;
}

uint Scene7010::calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis, byte facing, byte cel) const {
	if (facing >= kActorFacingCount)
		return 0;

	const int direction = kActorPathAxisDirectionByFacing[facing];
	int remaining = (targetAxis - startAxis) * direction;
	if (remaining <= 0)
		return 0;

	uint steps = 0;
	byte nextCel = cel;
	while (actorPathStepDelta(facing, nextCel) < (uint)remaining) {
		remaining -= (int)actorPathStepDelta(facing, nextCel);
		nextCel = nextActorPathCel(nextCel);
		++steps;
		if (steps > 300)
			break;
	}

	return steps;
}

byte Scene7010::nextActorPathCel(byte cel) const {
	return cel == 12 ? 1 : (byte)(cel + 1);
}

uint Scene7010::actorPathStepDelta(byte facing, byte cel) const {
	if (facing >= kActorFacingCount || cel == 0 || cel > 12)
		return 0;

	const uint offset = (uint)facing * 12 + cel - 1;
	if (offset >= _actorPathStepDeltas.size())
		return 0;

	return _actorPathStepDeltas[offset];
}

byte Scene7010::calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const {
	if (toX == fromX)
		return fromY < toY ? 3 : 0;

	const float slope = (float)ABS(toY - fromY) / (float)MAX<int>(1, ABS(toX - fromX));
	if (toX > fromX) {
		if (toY < fromY)
			return slope > kActorFacingSteepSlopeThreshold ? 0 :
				(slope > kActorFacingDiagonalSlopeThreshold ? 1 : 2);
		return slope > kActorFacingSteepSlopeThreshold ? 3 : 2;
	}

	if (toY < fromY)
		return slope > kActorFacingSteepSlopeThreshold ? 0 :
			(slope > kActorFacingDiagonalSlopeThreshold ? 5 : 4);
	return slope > kActorFacingSteepSlopeThreshold ? 3 : 4;
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
	const byte owner = _vm->gameState().currentInventoryOwnerIndex;
	return _vm->gameState().hasInventoryItem(owner, itemId);
}

void Scene7010::addInventoryItem(byte itemId) {
	GameplayState &state = _vm->gameState();
	state.addInventoryItem(state.currentInventoryOwnerIndex, itemId);
}

void Scene7010::removeInventoryItem(byte itemId) {
	GameplayState &state = _vm->gameState();
	state.removeInventoryItem(state.currentInventoryOwnerIndex, itemId);
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
	byte primaryFollowUpFrame = 2;
	if (_sceneStateFlags[3] == 0) {
		const byte firstVisitVariant = (byte)_random.getRandomNumber(2);
		if (firstVisitVariant == 0) {
			beginPrimaryLeftSpeechLine(8, 0);
			primaryFollowUpFrame = 3;
		} else if (firstVisitVariant == 1) {
			beginPrimaryLeftSpeechLine(8, 0);
			primaryFollowUpFrame = 4;
		} else {
			beginPrimaryLeftSpeechLine(8, 3);
			primaryFollowUpFrame = 4;
		}
		_sceneStateFlags[3] = 1;
	} else {
		beginPrimaryLeftSpeechLine(8, 1);
	}
	beginPrimaryLeftSpeechLine(8, primaryFollowUpFrame);
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
	if (!_chunk10IdlePairAAltPhase) {
		if (_chunk10IdlePairATicksRemaining == 0) {
			_chunk10IdlePairAAltPhase = true;
			_chunk10IdleFrameA = 0;
			_chunk10IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
		} else {
			--_chunk10IdlePairATicksRemaining;
			_chunk10IdleFrameB = 8 + (byte)_random.getRandomNumber(3);
		}
	} else if (_chunk10IdlePairATicksRemaining == 0) {
		_chunk10IdlePairAAltPhase = false;
		_chunk10IdleFrameB = 8;
		_chunk10IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	} else {
		--_chunk10IdlePairATicksRemaining;
		_chunk10IdleFrameA = (byte)_random.getRandomNumber(3);
	}

	if (!_chunk10IdlePairBAltPhase) {
		if (_chunk10IdlePairBTicksRemaining == 0) {
			_chunk10IdlePairBAltPhase = true;
			_chunk10IdleFrameC = 4;
			_chunk10IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
		} else {
			--_chunk10IdlePairBTicksRemaining;
			_chunk10IdleFrameD = 0x0c + (byte)_random.getRandomNumber(3);
		}
	} else if (_chunk10IdlePairBTicksRemaining == 0) {
		_chunk10IdlePairBAltPhase = false;
		_chunk10IdleFrameD = 0x0c;
		_chunk10IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	} else {
		--_chunk10IdlePairBTicksRemaining;
		_chunk10IdleFrameC = 4 + (byte)_random.getRandomNumber(3);
	}
}

void Scene7010::advanceSecondaryActorSpeechFrame() {
	byte nextFrame = _secondaryActorFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _secondaryActorFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(kSecondaryActorFramesPerFacing - 1);

	if (nextFrame == _secondaryActorFrame)
		nextFrame = (byte)((_secondaryActorFrame + 1) % kSecondaryActorFramesPerFacing);

	_secondaryActorFrame = nextFrame;
}

void Scene7010::advancePrimaryLeftSpeechFrame() {
	byte nextFrame = _primaryLeftSpeechLastFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _primaryLeftSpeechLastFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(3);

	if (nextFrame == _primaryLeftSpeechLastFrame)
		nextFrame = (byte)((_primaryLeftSpeechLastFrame + 1) % 4);

	_primaryLeftSpeechLastFrame = nextFrame;
	_chunk8FrameIndex = 0x0b + nextFrame;
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
		kG01SecondarySpeechTextColor, false, false);
}

void Scene7010::beginStaticSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(_speechOverlay, textRecordId, continuationCount, voiceSampleId, _activeActorWorldX, 0,
		kG01SecondarySpeechTextColor, false, false);
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
		kG01PrimarySpeechTextColor, true, false);
}

void Scene7010::beginPrimaryLeftSpeechLine(uint16 rowIndex, byte frameIndex) {
	const uint paletteOffset = kG01PrimarySpeechTextColor * 3;
	if (_paletteCurrent.size() > paletteOffset + 2) {
		_paletteCurrent[paletteOffset] = 0x33;
		_paletteCurrent[paletteOffset + 1] = 0x22;
		_paletteCurrent[paletteOffset + 2] = 0x39;
	}

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, 0xfa, 0x136,
		kG01PrimarySpeechTextColor, true, true);
}

void Scene7010::runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(overlay, textRecordId, continuationCount, voiceSampleId, centerX, topY, colorIndex,
		useRequestedTop, animatePrimaryLeft);
}

void Scene7010::runSpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount,
		uint16 voiceSampleId, uint16 centerX, uint16 topY, byte colorIndex, bool useRequestedTop,
		bool animatePrimaryLeft) {
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
		_primaryLeftSpeechActive = animatePrimaryLeft;
		if (animatePrimaryLeft)
			_chunk8FrameIndex = 0x0b;
		const bool interrupted = waitForSpeechOrDelay(duration, animatePrimaryLeft);
		if (animatePrimaryLeft) {
			_primaryLeftSpeechActive = false;
			_primaryLeftSpeechTimerAccumulator = 0;
			_chunk8FrameIndex = 0x0b;
		}
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

bool Scene7010::waitForSpeechOrDelay(uint32 fallbackMillis, bool animatePrimaryLeft) {
	uint32 elapsed = 0;
	while (!Engine::shouldQuit()) {
		const bool speechActive = _speech.isPlaying();
		if (!speechActive && elapsed >= fallbackMillis)
			break;

		const uint32 slice = speechActive ? 50 : MIN<uint32>(50, fallbackMillis - elapsed);
		if (waitSceneMillis(slice))
			return true;
		if (animatePrimaryLeft && !_primarySpeechOverlay.visible)
			break;
		elapsed += slice;
	}

	return Engine::shouldQuit();
}

void Scene7010::applyGameplayPanelPalette() {
	if (_paletteCurrent.size() <= kG01PanelTextColor * 3 + 2)
		return;

	const bool originalPaletteApplied = _panelArt.applyPalette(_paletteCurrent);
	if (!originalPaletteApplied) {
		const byte colors[] = {
			kG01PanelDarkColor, 0x05, 0x06, 0x08,
			kG01PanelFillColor, 0x0b, 0x0d, 0x11,
			kG01PanelSlotColor, 0x14, 0x16, 0x1a,
			kG01PanelLineColor, 0x24, 0x25, 0x28,
			kG01PanelSelectedColor, 0x2e, 0x1d, 0x0e,
			kG01PanelSelectedLineColor, 0x3a, 0x2d, 0x16
		};
		for (uint i = 0; i < ARRAYSIZE(colors); i += 4) {
			const uint paletteOffset = colors[i] * 3;
			if (paletteOffset + 2 < _paletteCurrent.size()) {
				_paletteCurrent[paletteOffset] = colors[i + 1];
				_paletteCurrent[paletteOffset + 1] = colors[i + 2];
				_paletteCurrent[paletteOffset + 2] = colors[i + 3];
			}
		}
	}

	const uint textOffset = kG01PanelTextColor * 3;
	_paletteCurrent[textOffset] = 0x32;
	_paletteCurrent[textOffset + 1] = _paletteCurrent[0x2d7];
	_paletteCurrent[textOffset + 2] = _paletteCurrent[0x2d8];
}

void Scene7010::drawGameplayPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	if (panelState.inventoryPanelVisible)
		drawInventoryPanel(surface, panelState);
	else if (panelState.verbPanelVisible)
		drawVerbPanel(surface, panelState);
}

void Scene7010::drawVerbPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	_panelArt.drawVerbPanel(surface, _savedFramebuffer, kG01InitialViewportXOffset, 0, panelState,
		_vm->font());
}

void Scene7010::drawInventoryPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	_panelArt.drawDialogueInventoryPanel(surface, _savedFramebuffer, kG01InitialViewportXOffset, 0,
		panelState, _vm->gameState(), _vm->font());
}

void Scene7010::presentFrame(const SceneHoverCaption *hoverCaption, const GameplayPanelState *panelState) {
	if (hoverCaption)
		hoverCaption->applyPalette(_paletteCurrent);
	if (panelState && panelState->visible())
		applyGameplayPanelPalette();
	uploadPalette6Bit(_paletteCurrent);

	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		const uint sourceOffset = kG01InitialViewportXOffset + y * HollywoodEngine::kSceneBufferWidth;
		memcpy(_screen.data() + y * HollywoodEngine::kScreenWidth,
			_sceneFramebuffer.data() + sourceOffset,
			HollywoodEngine::kScreenWidth);
	}

	drawSpeechOverlay();
	Graphics::Surface screenSurface;
	screenSurface.init(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight,
		HollywoodEngine::kScreenWidth, _screen.data(), Graphics::PixelFormat::createFormatCLUT8());
	if (panelState && panelState->visible())
		drawGameplayPanel(screenSurface, *panelState);
	else if (_vm->font() && _vm->font()->isLoaded()) {
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
