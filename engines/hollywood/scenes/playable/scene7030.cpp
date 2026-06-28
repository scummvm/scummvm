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

#include "hollywood/scenes/playable/scene7030.h"

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

#include <math.h>

namespace Hollywood {

const char *const kG03ArchiveName = "RESOURCE.G03";
const char *const kResource000Name = "RESOURCE.000";
const char *const kStage003ArchiveName = "RESOURCE.003";
const char *const kGameplayMusicArchiveName = "RESOURCE.M07";
const char *const kGameplaySoundBank0ArchiveName = "RESOURCE.S07";
const uint16 kG03Chunk5DescriptorCount = 9;
const uint16 kG03Chunk6DescriptorCount = 0x10;
const uint16 kG03Chunk7DescriptorCount = 0x0d;
const uint16 kG03Chunk10DescriptorCount = 0x0a;
const uint16 kG03Chunk11DescriptorCount = 0x20;
const byte kG03AmbientMusicCueStillFrame = 0x0f;
const uint16 kG03State7030 = 0x1b76;
const uint16 kG03State7031 = 0x1b77;
const uint16 kG03LastInteractiveState = 0x1b7f;
const uint16 kG03ExitState7040 = 0x1b80;
const uint16 kG01ReturnState7011 = 0x1b63;
const uint16 kG03InitialViewportXOffset = 0x60;
const uint kSceneColorToItemMapOffset = 0x100;
const uint kSceneColorMapSize = 0x100;
const byte kG03SecondarySpeechTextColor = 0xfd;
const byte kG03PrimarySpeechTextColor = 0xfb;
const byte kG03PanelDarkColor = 0xe7;
const byte kG03PanelFillColor = 0xe8;
const byte kG03PanelSlotColor = 0xe9;
const byte kG03PanelLineColor = 0xea;
const byte kG03PanelSelectedColor = 0xf1;
const byte kG03PanelSelectedLineColor = 0xf2;
const byte kG03PanelTextColor = 0xfc;
const int kG03VerbPanelTopY = 0x19f;
const int kG03VerbPanelCaptionY = 0x19f;
const int kG03VerbPanelStripTopY = 0x1a7;
const int kG03VerbPanelStripHeight = 0x1b;
const int kG03VerbPanelStripWidth = 0x58;
const int kG03InventoryPanelTopY = 0x10d;
const int kG03InventoryPanelCaptionY = 0x10d;
const int kG03InventorySlotStartX = 0x32;
const int kG03InventorySlotStartY = 0x13c;
const int kG03InventorySlotSize = 0x40;
const int kG03InventorySlotGap = 4;
const uint16 kG03VerbPanelStripXOffsets[9] = {
	0xff, 0, 8, 97, 186, 276, 366, 456, 545
};
const byte kG03Entry7030Facing = 4;
const int kG03Entry7030StartX = 0x312;
const int kG03Entry7030StartY = 0x19d;
const int kG03Entry7030TargetX = 0x1fa;
const int kG03Entry7030TargetY = 0x142;
const byte kG03Entry7031Facing = 2;
const int kG03Entry7031StartX = 0x60;
const int kG03Entry7031StartY = 0x10e;
const int kG03Entry7031TargetX = 0x133;
const int kG03Entry7031TargetY = 0x134;
const uint32 kG03ActorPathFrameMillis = 60;
const uint32 kG03SecondaryActorFrameMillis = 150;
const uint32 kG03Chunk5FrameMillis = 75;
const uint32 kG03Chunk5FastFrameMillis = 60;
const uint32 kG03Chunk6FrameMillis = 125;
const uint32 kG03AmbientMusicCheckMillis = 250;
const byte kInvalidFacing = 0xff;
const byte kInvalidCel = 0xff;
const byte kG03Chunk5FrameMap[] = {
	0, 0, 1, 2, 3, 4, 3, 2, 3, 4, 3, 2, 1, 0, 5, 6,
	7, 8, 7, 6, 7, 8, 7, 6, 5
};
const byte kG03Chunk7PickupItem0BFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};
const byte kG03Chunk10PickupItem0CFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 9, 0
};
const byte kG03Chunk11ExchangeItem0CFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 25, 25, 25, 25, 25, 25,
	25, 25, 25, 25, 26, 27, 28, 29, 30, 31, 13, 12, 11, 10, 9, 8,
	7, 6, 5, 4, 3, 2, 1, 0
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

Scene7030::Scene7030(HollywoodEngine *vm) :
		_vm(vm),
		_resourceArenaCursor(0),
		_random("hollywood_scene7030"),
		_chunk5FrameIndex(1),
		_chunk6IdleFrameA(0),
		_chunk6IdleFrameB(4),
		_chunk6IdleFrameC(8),
		_chunk6IdleFrameD(0x0c),
		_primaryLeftSpeechLastFrame(0),
		_chunk6IdlePairAAltPhase(false),
		_chunk6IdlePairBAltPhase(false),
		_primaryLeftSpeechActive(false),
		_chunk6IdlePairATicksRemaining(10),
		_chunk6IdlePairBTicksRemaining(16),
		_chunk9AmbientDecisionCounter(0),
		_chunk5FrameDirection(1),
		_chunk5TimerAccumulator(0),
		_chunk6TimerAccumulator(0),
		_chunk5FrameMillis(kG03Chunk5FrameMillis),
		_ambientMusicTimerAccumulator(0),
		_secondaryActorTimerAccumulator(0),
		_primaryLeftSpeechTimerAccumulator(0),
		_previousAmbientMusicTrackId(0),
		_activeActorWorldX(kG03Entry7030TargetX),
		_activeActorWorldY(kG03Entry7030TargetY),
		_activeActorFacing(kG03Entry7030Facing),
		_activeActorCel(0),
		_activeActorDrawOrderMode(0),
		_secondaryActorFrame(0),
		_actionOverlayVisible(false),
		_actionOverlayChunkIndex(0),
		_actionOverlayDescriptorCount(0),
		_actionOverlayFrameIndex(0),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kHollywoodPaletteSize);
	_paletteCurrent.resize(kHollywoodPaletteSize);
	_baseFramebufferOriginal.resize(kFrameBufferSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_savedFramebuffer.resize(kFrameBufferSize);
	_paletteMaskOriginal.resize(0x700);
	_fullPaletteRegionMask.resize(kG03PaletteMaskUsedBytes);
	_walkablePaletteMask.resize(kG03PaletteMaskUsedBytes);
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
	_speechOverlay.colorIndex = kG03SecondarySpeechTextColor;
	_speechOverlay.centerX = 0;
	_speechOverlay.topY = 0;
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.colorIndex = kG03PrimarySpeechTextColor;
	_primarySpeechOverlay.centerX = 0;
	_primarySpeechOverlay.topY = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
}

bool Scene7030::play() {
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

bool Scene7030::load() {
	if (!loadResource000RuntimeTables(_resource000OffsetTable, _resource000SizeTable) ||
			!loadResource000ActorBankSet00(_resource000OffsetTable, _resource000SizeTable))
		return false;

	if (!_vm->resources()->readChunkTable(Common::Path(kG03ArchiveName), _g03ChunkTable)) {
		warning("Failed to read %s header", kG03ArchiveName);
		return false;
	}

	for (uint i = 0; i < kG03InitialRequiredChunkCount; ++i) {
		if (!_g03ChunkTable.isValidChunk(i)) {
			warning("%s is missing required Scene 7030 chunk %u", kG03ArchiveName, i);
			return false;
		}
	}

	if (!loadFixedChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadFixedChunk(1, _paletteResource, kHollywoodPaletteSize) ||
			!loadVariableChunk(2, _fillRuns) ||
			!loadVariableChunk(3, _paletteMask) ||
			!loadVariableChunk(4, _metadata))
		return false;

	_baseFramebufferOriginal = _baseFramebuffer;
	_paletteMaskOriginal = _paletteMask;

	if (_paletteMask.size() < kG03PaletteMaskUsedBytes) {
		warning("%s chunk 3 is shorter than the G03 palette mask table", kG03ArchiveName);
		return false;
	}
	if (!initializeScenePathTables())
		return false;

	uint32 arenaSize = 0;
	for (uint i = kG03ArenaFirstChunk; i <= kG03ArenaLastChunk; ++i)
		arenaSize += _g03ChunkTable.sizes[i];

	_resourceArena.resize(arenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));

	for (uint i = kG03ArenaFirstChunk; i <= kG03ArenaLastChunk; ++i) {
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

	_vm->gameplayMusic()->setArchive(Common::Path(kGameplayMusicArchiveName));
	_soundBank0.setArchive(Common::Path(kGameplaySoundBank0ArchiveName));

	debugC(1, kDebugScene, "Scene 7030 loaded RESOURCE.G03");
	return true;
}

bool Scene7030::loadResource000RuntimeTables(Common::Array<byte> &offsetTable, Common::Array<byte> &sizeTable) {
	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s for Scene 7030 actor resources", kResource000Name);
		return false;
	}

	if ((uint32)file.size() < 1 + (2 * kResource000TableByteCount)) {
		warning("%s is too small for Scene 7030 runtime tables", kResource000Name);
		return false;
	}

	file.seek(1);
	offsetTable.resize(kResource000TableByteCount);
	sizeTable.resize(kResource000TableByteCount);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size() ||
			file.read(sizeTable.data(), sizeTable.size()) != sizeTable.size()) {
		warning("Failed to read %s runtime tables for Scene 7030", kResource000Name);
		return false;
	}

	return true;
}

bool Scene7030::loadResource000ActorBankSet00(const Common::Array<byte> &offsetTable, const Common::Array<byte> &sizeTable) {
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

	debugC(1, kDebugResources, "Loaded %s actor bank set 00 for Scene 7030", kResource000Name);
	return true;
}

bool Scene7030::loadResource000Owner1ActorPalette(const Common::Array<byte> &offsetTable) {
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

bool Scene7030::loadStage003SceneRows() {
	Common::File file;
	if (!file.open(Common::Path(kStage003ArchiveName))) {
		warning("Failed to open %s for Scene 7030 text", kStage003ArchiveName);
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

	const uint32 stageOffsetEntry = kStage003DecodeKeySize + (kG03StageIndex * 4);
	if (stageOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			stageOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s has no stage 703 offset entry", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffsetEntry);
	const uint32 stageOffset = file.readUint32LE();
	if (stageOffset + kStage003DescriptorTableSize + 3 > (uint32)file.size()) {
		warning("%s stage 703 descriptor table is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffset);
	if (file.read(_stage003StageBlock.data(), _stage003StageBlock.size()) != _stage003StageBlock.size()) {
		warning("Failed to read %s stage 703 descriptor table", kStage003ArchiveName);
		return false;
	}

	const byte smallRowCount = file.readByte();
	const uint16 largeRowCount = file.readUint16LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kStage003SmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kStage003LargeRowSize;
	if (file.pos() + smallRowBytes + largeRowBytes > file.size()) {
		warning("%s stage 703 text rows are out of range", kStage003ArchiveName);
		return false;
	}

	_stage003SmallRows.resize((uint32)(smallRowCount + 1) * kStage003SmallRowSize);
	memset(_stage003SmallRows.data(), 0, _stage003SmallRows.size());
	if (file.read(_stage003SmallRows.data() + kStage003SmallRowSize, smallRowBytes) != smallRowBytes) {
		warning("Failed to read %s stage 703 small text rows", kStage003ArchiveName);
		return false;
	}

	_stage003LargeRows.resize(largeRowBytes);
	if (file.read(_stage003LargeRows.data(), _stage003LargeRows.size()) != _stage003LargeRows.size()) {
		warning("Failed to read %s stage 703 large text rows", kStage003ArchiveName);
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

	debugC(1, kDebugResources, "Loaded %s stage 703 text rows: smallRows=%u largeRows=%u",
		kStage003ArchiveName, smallRowCount, largeRowCount);
	return true;
}

bool Scene7030::loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kG03ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kG03ArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed Scene 7030 destination", kG03ArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kG03ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s fixed chunk %u: size=%u", kG03ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene7030::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kG03ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kG03ArchiveName, index);
		return false;
	}

	destination.resize(stream->size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kG03ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s variable chunk %u: size=%u", kG03ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene7030::loadArenaChunk(uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kG03ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kG03ArchiveName, index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the Scene 7030 resource arena", kG03ArchiveName, index);
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kG03ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		kG03ArchiveName, index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

void Scene7030::expandFillRunsToSavedFramebuffer() {
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

bool Scene7030::initializeScenePathTables() {
	const uint boundaryBytes = kSceneRouteBoundaryPointCount * 4;
	if (_metadata.size() < kRouteBoundaryPoints + boundaryBytes ||
			_metadata.size() < kRouteBoundarySteps + kSceneRouteStepCount) {
		warning("%s chunk 4 is too short for G03 path route tables", kG03ArchiveName);
		return false;
	}

	memcpy(_fullPaletteRegionMask.data(), _paletteMask.data(), _fullPaletteRegionMask.size());
	memcpy(_walkablePaletteMask.data(), _paletteMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 4)
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

void Scene7030::initializePreviewState() {
	_chunk5FrameIndex = 1;
	_chunk5FrameDirection = 1;
	_chunk6IdleFrameA = 0;
	_chunk6IdleFrameB = 4;
	_chunk6IdleFrameC = 8;
	_chunk6IdleFrameD = 0x0c;
	_primaryLeftSpeechLastFrame = 0;
	_actionOverlayVisible = false;
	_actionOverlayChunkIndex = 0;
	_actionOverlayDescriptorCount = 0;
	_actionOverlayFrameIndex = 0;
	_chunk6IdlePairAAltPhase = _random.getRandomNumber(1) != 0;
	_chunk6IdlePairBAltPhase = _random.getRandomNumber(1) != 0;
	_primaryLeftSpeechActive = false;
	_chunk6IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk6IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk9AmbientDecisionCounter = 0;
	_chunk5TimerAccumulator = 0;
	_chunk6TimerAccumulator = 0;
	_chunk5FrameMillis = kG03Chunk5FrameMillis;
	_ambientMusicTimerAccumulator = 0;
	_secondaryActorTimerAccumulator = 0;
	_primaryLeftSpeechTimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	_activeActorWorldX = kG03Entry7030TargetX;
	_activeActorWorldY = kG03Entry7030TargetY;
	_activeActorFacing = kG03Entry7030Facing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	_sceneStateFlags[2] = 2;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene7030::drawPreviewComposite() {
	drawCutsceneComposite(false, 0, 0, 0, 0, false, 0, 0, 0, 0);
}

void Scene7030::drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[6], 0,
		kG03Chunk6DescriptorCount, _chunk6IdleFrameA, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[6], 0,
		kG03Chunk6DescriptorCount, _chunk6IdleFrameB, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[6], 0,
		kG03Chunk6DescriptorCount, _chunk6IdleFrameC, _sceneFramebuffer);
	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[6], 0,
		kG03Chunk6DescriptorCount, _chunk6IdleFrameD, _sceneFramebuffer);

	if (_sceneStateFlags[0] != 0) {
		const byte frame = _chunk5FrameIndex < ARRAYSIZE(kG03Chunk5FrameMap) ?
			kG03Chunk5FrameMap[_chunk5FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[5], 0,
			kG03Chunk5DescriptorCount, frame, _sceneFramebuffer);
	}

	if (drawSecondaryActor) {
		const int secondaryActorBottomY = drawSecondaryActorFrame(secondaryFacing, secondaryFrame,
			secondaryWorldX, secondaryWorldY);
		if (drawActiveActor)
			drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, secondaryActorBottomY);
	} else if (drawActiveActor) {
		drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, -1);
	}

	if (_actionOverlayVisible) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
			_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
	}
}

void Scene7030::drawPlayableComposite() {
	const bool drawSecondaryActor = _speechOverlay.visible;
	drawCutsceneComposite(true, _activeActorFacing, _activeActorCel, _activeActorWorldX, _activeActorWorldY,
		drawSecondaryActor, _activeActorFacing, _secondaryActorFrame, _activeActorWorldX, _activeActorWorldY,
		_activeActorDrawOrderMode);
}

void Scene7030::drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex) {
	if (chunkIndex >= HollywoodEngine::kResourceChunkCount || frameIndex >= frameMapSize)
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[chunkIndex], 0,
		descriptorCount, frameMap[frameIndex], _sceneFramebuffer);
}

void Scene7030::drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY, int minimumYExclusive) {
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

int Scene7030::drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY) {
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

int Scene7030::drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive) {
	cursor += runBase;
	int lastRunY = minimumYExclusive;
	for (uint runIndex = 0; runIndex < runCount; ++runIndex) {
		if (cursor + 3 > runStreams.size())
			return lastRunY;

		const int xOffset = runStreams[cursor++];
		const int yOffset = runStreams[cursor++];
		const uint pixelCount = runStreams[cursor++];
		if (cursor + pixelCount > runStreams.size())
			return lastRunY;

		const int dstY = spriteY + yOffset;
		lastRunY = dstY;
		if (dstY > minimumYExclusive && dstY >= 0 && dstY < HollywoodEngine::kSceneBufferHeight) {
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

	return lastRunY;
}

void Scene7030::runEntryCutscene() {
	if (_vm->gameState().mainFlowStateId == kG03State7031) {
		runEntryPath(kG03Entry7031StartX, kG03Entry7031StartY, kG03Entry7031Facing,
			kG03Entry7031TargetX, kG03Entry7031TargetY);
	} else {
		runEntryPath(kG03Entry7030StartX, kG03Entry7030StartY, kG03Entry7030Facing,
			kG03Entry7030TargetX, kG03Entry7030TargetY);
	}
}

void Scene7030::runEntryPath(int startX, int startY, byte startFacing, int targetX, int targetY) {
	_activeActorWorldX = startX;
	_activeActorWorldY = startY;
	_activeActorFacing = startFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_chunk5FrameIndex = 1;
	_chunk6IdleFrameA = 0;
	_chunk6IdleFrameB = 4;
	_chunk6IdleFrameC = 8;
	_chunk6IdleFrameD = 0x0c;
	_chunk6IdlePairAAltPhase = _random.getRandomNumber(1) != 0;
	_chunk6IdlePairBAltPhase = _random.getRandomNumber(1) != 0;
	_chunk6IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk6IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);

	drawPlayableComposite();
	presentFrame();

	queueActorPathWithPaletteRegionRouting(startX, startY, targetX, targetY, kInvalidFacing, 0);
	for (uint frameIndex = 1; frameIndex < _actorPathFrames.size() && !_skipRequested && !Engine::shouldQuit(); ++frameIndex) {
		const ActorPathFrame &frame = _actorPathFrames[frameIndex];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
		if (waitSceneMillis(kG03ActorPathFrameMillis))
			return;
	}

	_activeActorWorldX = targetX;
	_activeActorWorldY = targetY;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_activeActorCel = 0;
	drawPlayableComposite();
	presentFrame();
}

bool Scene7030::runBasicGameplayLoop() {
	GameplayLoop loop(_vm, this);
	return loop.run();
}

const SceneHotspotTable &Scene7030::hotspots() const {
	return _hotspots;
}

const Common::Array<byte> &Scene7030::savedFramebuffer() const {
	return _savedFramebuffer;
}

uint16 Scene7030::viewportXOffset() const {
	return kG03InitialViewportXOffset;
}

uint16 Scene7030::viewportYOffset() const {
	return 0;
}

void Scene7030::prepareGameplayLoop() {
	clearAllSpeechOverlays();
	_primaryLeftSpeechActive = false;
	_primaryLeftSpeechTimerAccumulator = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	_actionOverlayVisible = false;
}

void Scene7030::advanceGameplayLoop(uint32 delta) {
	if (_primaryLeftSpeechActive && _primarySpeechOverlay.visible) {
		_primaryLeftSpeechTimerAccumulator += delta;
		while (_primaryLeftSpeechTimerAccumulator >= kG03Chunk6FrameMillis) {
			advancePrimaryLeftSpeechFrame();
			_primaryLeftSpeechTimerAccumulator -= kG03Chunk6FrameMillis;
		}
	} else {
		_primaryLeftSpeechTimerAccumulator = 0;
		if (_sceneStateFlags[0] != 0) {
			_chunk5TimerAccumulator += delta;
			while (_chunk5TimerAccumulator >= _chunk5FrameMillis) {
				advanceChunk5AmbientOverlay();
				_chunk5TimerAccumulator -= _chunk5FrameMillis;
			}
		} else {
			_chunk5TimerAccumulator = 0;
		}
	}

	_chunk6TimerAccumulator += delta;
	while (_chunk6TimerAccumulator >= kG03Chunk6FrameMillis) {
		advanceChunk6IdleFrames();
		_chunk6TimerAccumulator -= kG03Chunk6FrameMillis;
	}

	if (_speechOverlay.visible) {
		_secondaryActorTimerAccumulator += delta;
		while (_secondaryActorTimerAccumulator >= kG03SecondaryActorFrameMillis) {
			advanceSecondaryActorSpeechFrame();
			_secondaryActorTimerAccumulator -= kG03SecondaryActorFrameMillis;
		}
	} else {
		_secondaryActorFrame = 0;
		_secondaryActorTimerAccumulator = 0;
	}

	updateAmbientAudioAndMusicCues(delta);
}

void Scene7030::drawGameplayFrame() {
	drawPlayableComposite();
}

void Scene7030::presentGameplayFrame(const SceneHoverCaption &hoverCaption, const GameplayPanelState &panelState) {
	presentFrame(&hoverCaption, &panelState);
}

bool Scene7030::shouldExitGameplayLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return stateId < kG03State7030 || stateId > kG03LastInteractiveState;
}

void Scene7030::handleLeftClick(const GameplayLoopCursorState &state) {
	_vm->cursor()->leaveInteractiveMode();
	processSceneActionClick(state);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void Scene7030::processSceneActionClick(const GameplayLoopCursorState &state) {
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

void Scene7030::dispatchSceneAction(uint16 handlerId) {
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
		handleActionSlot00TransitionToG04();
		break;
	case 302:
		handleActionSlot01SecondarySpeech();
		break;
	case 303:
		handleActionSlot02TransitionToG01Alt();
		break;
	case 304:
		handleActionSlot03SecondarySpeech();
		break;
	case 305:
		handleActionSlot04SecondarySpeech();
		break;
	case 306:
		handleActionSlot05ToggleSceneState0Speech();
		break;
	case 309:
		handleActionSlot08CommonSpeech();
		break;
	case 310:
		handleActionSlot09CommonSpeech();
		break;
	case 311:
		handleActionSlot10CommonSpeech();
		break;
	case 312:
		handleActionSlot11ExchangeItem0CFor0D();
		break;
	case 313:
		handleActionSlot12PickupItem0B();
		break;
	case 314:
		handleActionSlot13PickupItem0C();
		break;
	case 315:
		handleActionSlot14SecondarySpeech();
		break;
	default:
		warning("Unhandled Scene7030 action handler %u", handlerId);
		break;
	}
}

void Scene7030::walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel) {
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
		waitSceneMillis(kG03ActorPathFrameMillis);
	}

	drawPlayableComposite();
	presentFrame();
}

void Scene7030::adjustWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetX > 0x242)
		targetX = 0x242;

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

void Scene7030::queueActorPathWithPaletteRegionRouting(int startX, int startY, int targetX, int targetY,
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
			if (currentRegion == 6 && nextRegion == 1)
				requestedFacing = 4;
			if (currentRegion == 1 && nextRegion == 6 &&
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
	bool restoredStepDeltas = false;
	if (currentRegion == 6 && targetRegion == 6) {
		memcpy(_actorPathStepDeltas.data() + 12, kActorPathStepDeltaTableSet00 + 24, 12);
		requestedFacing = 1;
		restoredStepDeltas = true;
	}
	if (currentRegion == 1 && targetRegion == 1 && state.x < targetX && targetY <= state.y)
		requestedFacing = 1;

	state.drawOrderMode = currentRegion;
	buildActorPathFramesBetweenPoints(state, targetX, targetY, finalFacing, finalCel, requestedFacing);
	if (restoredStepDeltas)
		memcpy(_actorPathStepDeltas.data(), kActorPathStepDeltaTableSet00, _actorPathStepDeltas.size());

	if (!_actorPathFrames.empty()) {
		const ActorPathFrame &lastFrame = _actorPathFrames.back();
		_activeActorWorldX = lastFrame.worldX;
		_activeActorWorldY = lastFrame.worldY;
		_activeActorFacing = lastFrame.facing;
		_activeActorCel = lastFrame.cel;
		_activeActorDrawOrderMode = lastFrame.drawOrderMode;
	}
}

void Scene7030::buildActorPathFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
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

void Scene7030::appendActorPathFrame(const ActorPathBuildState &state) {
	ActorPathFrame frame;
	frame.drawOrderMode = state.drawOrderMode;
	frame.facing = state.facing;
	frame.cel = state.cel;
	frame.worldX = (int16)CLIP<int>(state.x, -32768, 32767);
	frame.worldY = (int16)CLIP<int>(state.y, -32768, 32767);
	_actorPathFrames.push_back(frame);
}

ScenePoint Scene7030::nearestPaletteRouteBoundaryPoint(int startX, int startY, byte currentRegion, byte nextRegion) const {
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

ScenePoint Scene7030::bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
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

byte Scene7030::paletteRegionAt(int x, int y) const {
	if (x < 0 || y < 0 || x >= HollywoodEngine::kSceneBufferWidth || y >= HollywoodEngine::kSceneBufferHeight ||
			_fullPaletteRegionMask.empty())
		return 0;

	const uint offset = y * HollywoodEngine::kSceneBufferWidth + x;
	if (offset >= _savedFramebuffer.size())
		return 0;

	return _fullPaletteRegionMask[_savedFramebuffer[offset]];
}

byte Scene7030::calculateMovementFacingForPath(int fromX, int fromY, int toX, int toY, int requestedFacing) const {
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

uint Scene7030::calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis, byte facing, byte cel) const {
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

byte Scene7030::nextActorPathCel(byte cel) const {
	return cel == 12 ? 1 : (byte)(cel + 1);
}

uint Scene7030::actorPathStepDelta(byte facing, byte cel) const {
	if (facing >= kActorFacingCount || cel == 0 || cel > 12)
		return 0;

	const uint offset = (uint)facing * 12 + cel - 1;
	if (offset >= _actorPathStepDeltas.size())
		return 0;

	return _actorPathStepDeltas[offset];
}

byte Scene7030::calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const {
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

void Scene7030::applySceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0 || selector == 0xff) {
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		for (uint i = 0; i < _fullPaletteRegionMask.size(); ++i) {
			if (_paletteMaskOriginal[i] == 7)
				_fullPaletteRegionMask[i] = _sceneStateFlags[0] == 1 ? 0 : 1;
		}

		if (_sceneStateFlags[0] != 1) {
			if (_routeBoundaryPoints.size() > 0x100) {
				_routeBoundaryPoints[0x4b].x = 0x149;
				_routeBoundaryPoints[0x4b].y = 0x136;
				_routeBoundaryPoints[0x4c].x = 0x16f;
				_routeBoundaryPoints[0x4c].y = 0x13f;
				_routeBoundaryPoints[0xff].x = 0x14a;
				_routeBoundaryPoints[0xff].y = 0x136;
				_routeBoundaryPoints[0x100].x = 0x170;
				_routeBoundaryPoints[0x100].y = 0x13e;
			}
		}

		rebuildWalkablePaletteMask();
	}

	if (selector == 2 || selector == 0xff) {
		if (!_baseFramebufferOriginal.empty())
			memcpy(_baseFramebuffer.data(), _baseFramebufferOriginal.data(), _baseFramebuffer.size());

		if (_sceneStateFlags[2] != 0) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		} else {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
		}

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMapOffset + kSceneColorMapSize &&
				_paletteMask.size() >= kSceneColorToItemMapOffset + kSceneColorMapSize) {
			memcpy(_paletteMask.data() + kSceneColorToItemMapOffset,
				_paletteMaskOriginal.data() + kSceneColorToItemMapOffset, kSceneColorMapSize);
			if (_sceneStateFlags[2] == 0) {
				for (uint i = 0; i < kSceneColorMapSize; ++i) {
					if (_paletteMaskOriginal[kSceneColorToItemMapOffset + i] == 5)
						_paletteMask[kSceneColorToItemMapOffset + i] = 0;
				}
			}
		}

		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}
}

void Scene7030::rebuildWalkablePaletteMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 4)
			_walkablePaletteMask[i] = 0;
	}
}

bool Scene7030::hasInventoryItem(byte itemId) const {
	return itemId < ARRAYSIZE(_inventoryItems) && _inventoryItems[itemId];
}

void Scene7030::addInventoryItem(byte itemId) {
	if (itemId < ARRAYSIZE(_inventoryItems))
		_inventoryItems[itemId] = true;
}

void Scene7030::removeInventoryItem(byte itemId) {
	if (itemId < ARRAYSIZE(_inventoryItems))
		_inventoryItems[itemId] = false;
}

void Scene7030::handleActionSlot00TransitionToG04() {
	_vm->gameState().mainFlowStateId = kG03ExitState7040;
}

void Scene7030::handleActionSlot01SecondarySpeech() {
	beginSecondarySpeechLine(1, 0);
}

void Scene7030::handleActionSlot02TransitionToG01Alt() {
	_vm->gameState().mainFlowStateId = kG01ReturnState7011;
}

void Scene7030::handleActionSlot03SecondarySpeech() {
	beginSecondarySpeechLine(2, 0);
}

void Scene7030::handleActionSlot04SecondarySpeech() {
	beginSecondarySpeechLine(3, 0);
}

void Scene7030::handleActionSlot05ToggleSceneState0Speech() {
	if (_sceneStateFlags[0] == 1) {
		beginSecondarySpeechLine(4, 0);
		_sceneStateFlags[0] = 2;
		applySceneStateToHotspotsAndPatches(0);
	} else {
		beginSecondarySpeechLine(4, 1);
	}
}

void Scene7030::handleActionSlot08CommonSpeech() {
	beginSecondarySpeechLine(7, 0);
	_sceneStateFlags[2] = 2;
	applySceneStateToHotspotsAndPatches(2);
}

void Scene7030::handleActionSlot09CommonSpeech() {
	beginSecondarySpeechLine(8, 0);
}

void Scene7030::handleActionSlot10CommonSpeech() {
	beginSecondarySpeechLine(9, 0);
}

void Scene7030::handleActionSlot11ExchangeItem0CFor0D() {
	beginSecondarySpeechLine(10, 0);
	runMappedActionOverlay(11, kG03Chunk11DescriptorCount, kG03Chunk11ExchangeItem0CFrameMap,
		ARRAYSIZE(kG03Chunk11ExchangeItem0CFrameMap), kG03Chunk5FrameMillis);
	removeInventoryItem(0x0c);
	addInventoryItem(0x0d);
	_vm->gameState().inventoryPanelRedrawn = true;
	_soundBank0.playSample(1, 100);
}

void Scene7030::handleActionSlot12PickupItem0B() {
	if (hasInventoryItem(0x0b)) {
		beginSecondarySpeechLine(5, 2);
		return;
	}
	if (_vm->gameState().currentRandomAmbientMusicTrackId != kG03AmbientMusicCueStillFrame) {
		beginSecondarySpeechLine(5, 3);
		return;
	}

	beginSecondarySpeechLine(5, 0);
	runMappedActionOverlay(7, kG03Chunk7DescriptorCount, kG03Chunk7PickupItem0BFrameMap,
		ARRAYSIZE(kG03Chunk7PickupItem0BFrameMap), kG03Chunk5FrameMillis);
	addInventoryItem(0x0b);
	_vm->gameState().inventoryPanelRedrawn = true;
	_soundBank0.playSample(1, 100);
	_sceneStateFlags[1] = 1;
	beginSecondarySpeechLine(5, 1);
}

void Scene7030::handleActionSlot13PickupItem0C() {
	if (_sceneStateFlags[2] == 1) {
		handleActionSlot08CommonSpeech();
		clearSpeechOverlay();
		_speech.stop();
	}

	runMappedActionOverlay(10, kG03Chunk10DescriptorCount, kG03Chunk10PickupItem0CFrameMap,
		ARRAYSIZE(kG03Chunk10PickupItem0CFrameMap), kG03Chunk5FrameMillis, 3);
	addInventoryItem(0x0c);
	_vm->gameState().inventoryPanelRedrawn = true;
	_soundBank0.playSample(1, 100);
}

void Scene7030::handleActionSlot14SecondarySpeech() {
	beginSecondarySpeechLine(10, 1);
}

void Scene7030::runMappedActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int statePatchFrame) {
	_actionOverlayVisible = true;
	_actionOverlayChunkIndex = (byte)chunkIndex;
	_actionOverlayDescriptorCount = (byte)descriptorCount;
	for (uint frame = 0; frame < frameMapSize && !Engine::shouldQuit(); ++frame) {
		_actionOverlayFrameIndex = frameMap[frame];
		if (statePatchFrame >= 0 && (int)frame == statePatchFrame) {
			_sceneStateFlags[2] = 0;
			applySceneStateToHotspotsAndPatches(2);
		}
		if (waitSceneMillis(frameMillis))
			break;
	}
	_actionOverlayVisible = false;
	_actionOverlayFrameIndex = 0;
	drawPlayableComposite();
	presentFrame();
}

bool Scene7030::waitSceneMillis(uint32 millis) {
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

void Scene7030::updateAmbientAudioAndMusicCues(uint32 delta) {
	_ambientMusicTimerAccumulator += delta;
	if (_ambientMusicTimerAccumulator < kG03AmbientMusicCheckMillis)
		return;
	_ambientMusicTimerAccumulator %= kG03AmbientMusicCheckMillis;

	if (_vm->gameplayMusic()->isPlaying())
		return;

	GameplayState &state = _vm->gameState();
	if (state.currentRandomAmbientMusicTrackId != kG03AmbientMusicCueStillFrame) {
		_previousAmbientMusicTrackId = state.currentRandomAmbientMusicTrackId;
		state.currentRandomAmbientMusicTrackId = kG03AmbientMusicCueStillFrame;
		_chunk5FrameIndex = 0;
		_chunk5FrameMillis = kG03Chunk5FrameMillis;
		_vm->gameplayMusic()->playMusicCue(state.currentRandomAmbientMusicTrackId, 75);
		return;
	}

	byte nextTrack = 0;
	do {
		nextTrack = (byte)(0x0c + _random.getRandomNumber(2));
	} while (nextTrack == _previousAmbientMusicTrackId);

	_previousAmbientMusicTrackId = state.currentRandomAmbientMusicTrackId;
	state.currentRandomAmbientMusicTrackId = nextTrack;
	_chunk5FrameIndex = 1;
	_chunk5FrameDirection = 1;
	_chunk5FrameMillis = nextTrack == 0x0e ? kG03Chunk5FastFrameMillis : kG03Chunk5FrameMillis;
	_vm->gameplayMusic()->playMusicCue(state.currentRandomAmbientMusicTrackId, 75);
}

void Scene7030::advanceChunk5AmbientOverlay() {
	if (_vm->gameState().currentRandomAmbientMusicTrackId == kG03AmbientMusicCueStillFrame) {
		if (_chunk5FrameIndex < ARRAYSIZE(kG03Chunk5FrameMap) && kG03Chunk5FrameMap[_chunk5FrameIndex] != 0) {
			if ((_chunk5FrameIndex % 12) < 7)
				--_chunk5FrameIndex;
			else
				++_chunk5FrameIndex;
		}
		return;
	}

	if (_chunk9AmbientDecisionCounter == 0) {
		_chunk9AmbientDecisionCounter = (byte)(_random.getRandomNumber(3) + 1);
		_chunk5FrameDirection = _random.getRandomNumber(1) == 0 ? -1 : 1;
	}
	_chunk5FrameIndex = (byte)((int)_chunk5FrameIndex + _chunk5FrameDirection);
	if (_chunk5FrameIndex == 0)
		_chunk5FrameIndex = 0x18;
	else if (_chunk5FrameIndex == 0x19)
		_chunk5FrameIndex = 1;
	--_chunk9AmbientDecisionCounter;
}

void Scene7030::advanceChunk6IdleFrames() {
	if (!_chunk6IdlePairAAltPhase) {
		if (_chunk6IdlePairATicksRemaining == 0) {
			_chunk6IdlePairAAltPhase = true;
			_chunk6IdleFrameB = 4;
			_chunk6IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
		} else {
			--_chunk6IdlePairATicksRemaining;
			_chunk6IdleFrameA = (byte)_random.getRandomNumber(3);
		}
	} else if (_chunk6IdlePairATicksRemaining == 0) {
		_chunk6IdlePairAAltPhase = false;
		_chunk6IdleFrameA = 0;
		_chunk6IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	} else {
		--_chunk6IdlePairATicksRemaining;
		_chunk6IdleFrameB = 4 + (byte)_random.getRandomNumber(3);
	}

	if (!_chunk6IdlePairBAltPhase) {
		if (_chunk6IdlePairBTicksRemaining == 0) {
			_chunk6IdlePairBAltPhase = true;
			_chunk6IdleFrameC = 8;
			_chunk6IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
		} else {
			--_chunk6IdlePairBTicksRemaining;
			_chunk6IdleFrameD = 0x0c + (byte)_random.getRandomNumber(3);
		}
	} else if (_chunk6IdlePairBTicksRemaining == 0) {
		_chunk6IdlePairBAltPhase = false;
		_chunk6IdleFrameD = 0x0c;
		_chunk6IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	} else {
		--_chunk6IdlePairBTicksRemaining;
		_chunk6IdleFrameC = 8 + (byte)_random.getRandomNumber(3);
	}
}

void Scene7030::advanceSecondaryActorSpeechFrame() {
	byte nextFrame = _secondaryActorFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _secondaryActorFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(kSecondaryActorFramesPerFacing - 1);

	if (nextFrame == _secondaryActorFrame)
		nextFrame = (byte)((_secondaryActorFrame + 1) % kSecondaryActorFramesPerFacing);

	_secondaryActorFrame = nextFrame;
}

void Scene7030::advancePrimaryLeftSpeechFrame() {
	byte nextFrame = _primaryLeftSpeechLastFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _primaryLeftSpeechLastFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(3);

	if (nextFrame == _primaryLeftSpeechLastFrame)
		nextFrame = (byte)((_primaryLeftSpeechLastFrame + 1) % 4);

	_primaryLeftSpeechLastFrame = nextFrame;
	_chunk5FrameIndex = 0x0b + nextFrame;
}

void Scene7030::clearSpeechOverlay() {
	_speechOverlay.visible = false;
	_speechOverlay.lines.clear();
}

void Scene7030::clearAllSpeechOverlays() {
	clearSpeechOverlay();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
}

void Scene7030::drawSpeechOverlay() {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return;

	drawSpeechOverlay(_speechOverlay);
	drawSpeechOverlay(_primarySpeechOverlay);
}

void Scene7030::drawSpeechOverlay(const SpeechOverlay &overlay) {
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
		const int x = (int)overlay.centerX - (lineWidth >> 1) - kG03InitialViewportXOffset;
		const int y = (int)overlay.topY + lineIndex * kOriginalSpeechLineHeight;
		font->drawString(&screenSurface, line, x, y, lineWidth, overlay.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene7030::beginSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	runSpeechLine(_speechOverlay, rowIndex, frameIndex, _activeActorWorldX, 0,
		kG03SecondarySpeechTextColor, false, false);
}

void Scene7030::beginStaticSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(_speechOverlay, textRecordId, continuationCount, voiceSampleId, _activeActorWorldX, 0,
		kG03SecondarySpeechTextColor, false, false);
}

void Scene7030::beginPrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue) {
	const uint paletteOffset = kG03PrimarySpeechTextColor * 3;
	if (_paletteCurrent.size() > paletteOffset + 2) {
		_paletteCurrent[paletteOffset] = red;
		_paletteCurrent[paletteOffset + 1] = green;
		_paletteCurrent[paletteOffset + 2] = blue;
	}

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, centerX, topY,
		kG03PrimarySpeechTextColor, true, false);
}

void Scene7030::beginPrimaryLeftSpeechLine(uint16 rowIndex, byte frameIndex) {
	const uint paletteOffset = kG03PrimarySpeechTextColor * 3;
	if (_paletteCurrent.size() > paletteOffset + 2) {
		_paletteCurrent[paletteOffset] = 0x33;
		_paletteCurrent[paletteOffset + 1] = 0x22;
		_paletteCurrent[paletteOffset + 2] = 0x39;
	}

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, 0xfa, 0x136,
		kG03PrimarySpeechTextColor, true, true);
}

void Scene7030::runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(overlay, textRecordId, continuationCount, voiceSampleId, centerX, topY, colorIndex,
		useRequestedTop, animatePrimaryLeft);
}

void Scene7030::runSpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount,
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
			_chunk5FrameIndex = 0x0b;
		const bool interrupted = waitForSpeechOrDelay(duration, animatePrimaryLeft);
		if (animatePrimaryLeft) {
			_primaryLeftSpeechActive = false;
			_primaryLeftSpeechTimerAccumulator = 0;
			_chunk5FrameIndex = 0x0b;
		}
		_speech.stop();
		overlay.visible = false;
		overlay.lines.clear();
		if (interrupted)
			break;
	}
}

bool Scene7030::getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 100) * 5;
	if (offset + 5 > _stage003StageBlock.size())
		return false;

	textRecordId = readUint16LE(_stage003StageBlock, offset);
	continuationCount = _stage003StageBlock[offset + 2];
	voiceSampleId = readUint16LE(_stage003StageBlock, offset + 3);
	return textRecordId != 0;
}

bool Scene7030::getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 10) * 5;
	if (offset + 5 > _owner1SpeechCueDescriptors.size())
		return false;

	textRecordId = readUint16LE(_owner1SpeechCueDescriptors, offset);
	continuationCount = _owner1SpeechCueDescriptors[offset + 2];
	voiceSampleId = readUint16LE(_owner1SpeechCueDescriptors, offset + 3);
	return textRecordId != 0;
}

void Scene7030::wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars = 0x32;
	const int anchorX = anchorSceneX - kG03InitialViewportXOffset;
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

Common::String Scene7030::getResource003LargeTextRecord(uint16 recordId) const {
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

uint Scene7030::actorSpeechTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void Scene7030::calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY) {
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

bool Scene7030::waitForSpeechOrDelay(uint32 fallbackMillis, bool animatePrimaryLeft) {
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

void Scene7030::applyGameplayPanelPalette() {
	if (_paletteCurrent.size() <= kG03PanelTextColor * 3 + 2)
		return;

	const byte colors[] = {
		kG03PanelDarkColor, 0x05, 0x06, 0x08,
		kG03PanelFillColor, 0x0b, 0x0d, 0x11,
		kG03PanelSlotColor, 0x14, 0x16, 0x1a,
		kG03PanelLineColor, 0x24, 0x25, 0x28,
		kG03PanelSelectedColor, 0x2e, 0x1d, 0x0e,
		kG03PanelSelectedLineColor, 0x3a, 0x2d, 0x16
	};
	for (uint i = 0; i < ARRAYSIZE(colors); i += 4) {
		const uint paletteOffset = colors[i] * 3;
		if (paletteOffset + 2 < _paletteCurrent.size()) {
			_paletteCurrent[paletteOffset] = colors[i + 1];
			_paletteCurrent[paletteOffset + 1] = colors[i + 2];
			_paletteCurrent[paletteOffset + 2] = colors[i + 3];
		}
	}

	const uint textOffset = kG03PanelTextColor * 3;
	_paletteCurrent[textOffset] = 0x32;
	_paletteCurrent[textOffset + 1] = _paletteCurrent[0x2d7];
	_paletteCurrent[textOffset + 2] = _paletteCurrent[0x2d8];
}

void Scene7030::drawGameplayPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	if (panelState.inventoryPanelVisible)
		drawInventoryPanel(surface, panelState);
	else if (panelState.verbPanelVisible)
		drawVerbPanel(surface, panelState);
}

void Scene7030::drawVerbPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	fillScreenRect(0, kG03VerbPanelTopY, HollywoodEngine::kScreenWidth,
		HollywoodEngine::kScreenHeight - kG03VerbPanelTopY, kG03PanelFillColor);
	drawScreenRect(0, kG03VerbPanelTopY, HollywoodEngine::kScreenWidth,
		HollywoodEngine::kScreenHeight - kG03VerbPanelTopY, kG03PanelLineColor);
	drawPanelText(surface, panelState.captionText, kG03VerbPanelCaptionY, kG03PanelTextColor);

	for (byte stripIndex = 2; stripIndex <= 8; ++stripIndex) {
		const int x = kG03VerbPanelStripXOffsets[stripIndex];
		const bool selected = stripIndex == panelState.currentStrip;
		fillScreenRect(x, kG03VerbPanelStripTopY, kG03VerbPanelStripWidth, kG03VerbPanelStripHeight,
			selected ? kG03PanelSelectedColor : kG03PanelDarkColor);
		drawScreenRect(x, kG03VerbPanelStripTopY, kG03VerbPanelStripWidth, kG03VerbPanelStripHeight,
			selected ? kG03PanelSelectedLineColor : kG03PanelLineColor);
		drawPanelButtonText(surface, inventoryActionCaption(stripIndex), x,
			kG03VerbPanelStripTopY + 3, kG03VerbPanelStripWidth, kG03PanelTextColor);
	}
}

void Scene7030::drawInventoryPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	fillScreenRect(0, kG03InventoryPanelTopY, HollywoodEngine::kScreenWidth,
		HollywoodEngine::kScreenHeight - kG03InventoryPanelTopY, kG03PanelFillColor);
	drawScreenRect(0, kG03InventoryPanelTopY, HollywoodEngine::kScreenWidth,
		HollywoodEngine::kScreenHeight - kG03InventoryPanelTopY, kG03PanelLineColor);
	drawPanelText(surface, panelState.captionText, kG03InventoryPanelCaptionY, kG03PanelTextColor);

	for (uint slot = 0; slot < 16; ++slot) {
		const int column = slot % 8;
		const int row = slot / 8;
		const int x = kG03InventorySlotStartX + column * (kG03InventorySlotSize + kG03InventorySlotGap);
		const int y = kG03InventorySlotStartY + row * (kG03InventorySlotSize + kG03InventorySlotGap);
		fillScreenRect(x, y, kG03InventorySlotSize, kG03InventorySlotSize, kG03PanelSlotColor);
		drawScreenRect(x, y, kG03InventorySlotSize, kG03InventorySlotSize, kG03PanelLineColor);
	}
}

void Scene7030::drawPanelText(Graphics::Surface &surface, const Common::String &text, int y, byte colorIndex) {
	if (!_vm->font() || !_vm->font()->isLoaded() || text.empty())
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);
	const int textWidth = font->getStringWidth(text) + 2;
	const int x = MAX<int>(0, (HollywoodEngine::kScreenWidth - textWidth) / 2);
	font->drawString(&surface, text, x, y, textWidth, colorIndex, Graphics::kTextAlignLeft, 0, false, true);
}

void Scene7030::drawPanelButtonText(Graphics::Surface &surface, const Common::String &text,
		int x, int y, int width, byte colorIndex) {
	if (!_vm->font() || !_vm->font()->isLoaded() || text.empty())
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);
	const int textWidth = MIN<int>(font->getStringWidth(text) + 2, width);
	const int textX = x + MAX<int>(1, (width - textWidth) / 2);
	font->drawString(&surface, text, textX, y, textWidth, colorIndex, Graphics::kTextAlignLeft, 0, false, true);
}

void Scene7030::fillScreenRect(int x, int y, int width, int height, byte colorIndex) {
	const int left = CLIP<int>(x, 0, HollywoodEngine::kScreenWidth);
	const int top = CLIP<int>(y, 0, HollywoodEngine::kScreenHeight);
	const int right = CLIP<int>(x + width, 0, HollywoodEngine::kScreenWidth);
	const int bottom = CLIP<int>(y + height, 0, HollywoodEngine::kScreenHeight);
	if (right <= left || bottom <= top)
		return;

	for (int row = top; row < bottom; ++row)
		memset(_screen.data() + row * HollywoodEngine::kScreenWidth + left, colorIndex, right - left);
}

void Scene7030::drawScreenRect(int x, int y, int width, int height, byte colorIndex) {
	fillScreenRect(x, y, width, 1, colorIndex);
	fillScreenRect(x, y + height - 1, width, 1, colorIndex);
	fillScreenRect(x, y, 1, height, colorIndex);
	fillScreenRect(x + width - 1, y, 1, height, colorIndex);
}

void Scene7030::presentFrame(const SceneHoverCaption *hoverCaption, const GameplayPanelState *panelState) {
	if (hoverCaption)
		hoverCaption->applyPalette(_paletteCurrent);
	if (panelState && panelState->visible())
		applyGameplayPanelPalette();
	uploadPalette6Bit(_paletteCurrent);

	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		const uint sourceOffset = kG03InitialViewportXOffset + y * HollywoodEngine::kSceneBufferWidth;
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

bool Scene7030::pollEvents(bool allowSkip) {
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

bool Scene7030::delay(uint32 millis) {
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
