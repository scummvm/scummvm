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

#include "hollywood/scenes/playable/scene7040.h"

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

const char *const kG04ArchiveName = "RESOURCE.G04";
const char *const kResource000Name = "RESOURCE.000";
const char *const kStage003ArchiveName = "RESOURCE.003";
const char *const kGameplayMusicArchiveName = "RESOURCE.M07";
const char *const kGameplaySoundBank0ArchiveName = "RESOURCE.S07";
const uint16 kG04Chunk10DescriptorCount = 5;
const uint16 kG04Chunk11DescriptorCount = 0x1f;
const uint16 kG04Chunk12DescriptorCount = 3;
const uint16 kG04Chunk13DescriptorCount = 0x1c;
const uint16 kG04Chunk14ActionDescriptorCount = 0x32;
const uint16 kG04Chunk14AltDescriptorCount = 0x38;
const uint16 kG04Chunk16DescriptorCount = 0x0b;
const uint16 kG04Chunk17DescriptorCount = 9;
const uint16 kG04Chunk18DescriptorCount = 0x0a;
const uint kG04RelationRecordSize = 4;
const byte kG04AmbientMusicCueStillFrame = 0x0f;
const uint16 kG04State7040 = 0x1b80;
const uint16 kG04State7041 = 0x1b81;
const uint16 kG04State7042 = 0x1b82;
const uint16 kG04LastInteractiveState = 0x1b89;
const uint16 kG04ReturnState7031 = 0x1b77;
const uint16 kG04ExitState7050 = 0x1b8a;
const uint16 kG04ExitState7060 = 0x1b94;
const uint16 kG04InitialViewportXOffset = 0xc8;
const uint kSceneColorToItemMapOffset = 0x100;
const uint kSceneColorMapSize = 0x100;
const byte kG04SecondarySpeechTextColor = 0xfd;
const byte kG04PrimarySpeechTextColor = 0xfb;
const byte kG04PanelDarkColor = 0xe7;
const byte kG04PanelFillColor = 0xe8;
const byte kG04PanelSlotColor = 0xe9;
const byte kG04PanelLineColor = 0xea;
const byte kG04PanelSelectedColor = 0xf1;
const byte kG04PanelSelectedLineColor = 0xf2;
const byte kG04PanelTextColor = 0xfc;
const byte kG04Entry7040Facing = 1;
const int kG04Entry7040StartX = 100;
const int kG04Entry7040StartY = 0x1b1;
const int kG04Entry7040FirstTargetX = 0x14a;
const int kG04Entry7040FirstTargetY = 0x139;
const int kG04Entry7040RepeatTargetX = 0x16f;
const int kG04Entry7040RepeatTargetY = 0x177;
const byte kG04Entry7041Facing = 4;
const int kG04Entry7041StartX = 600;
const int kG04Entry7041StartY = 0x132;
const byte kG04Entry7042Facing = 4;
const int kG04Entry7042StartX = 0x322;
const int kG04Entry7042StartY = 0x1c9;
const int kG04Entry7042TargetX = 0x29e;
const int kG04Entry7042TargetY = 0x1cc;
const uint kResource000InventoryActionTablesEntry = 0xc8;
const uint kResource000FixedInventoryVerbTableOffset = 0xec54;
const uint32 kG04ActorPathFrameMillis = 60;
const uint32 kG04Chunk11FrameMillis = 75;
const uint32 kG04Chunk14FrameMillis = 75;
const uint32 kG04Chunk16FrameMillis = 75;
const uint32 kG04Chunk17FrameMillis = 125;
const uint32 kG04AmbientMusicCheckMillis = 250;
const byte kInvalidFacing = 0xff;
const byte kInvalidCel = 0xff;
const byte kG04Chunk11FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 1, 28, 29, 30, 0, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
	27, 1, 0, 0, 0, 0, 0, 0
};
const byte kG04Chunk14ActionFrameMap[] = {
	49, 49, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 14, 18, 19, 20, 21, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 34, 33, 36, 35, 34, 35, 36, 34, 33, 36, 35,
	33, 36, 35, 33, 34, 35, 36, 34, 33, 34, 35, 36, 34, 33, 36, 35,
	34, 33, 34, 35, 36, 34, 33, 36, 35, 34, 35, 36, 34, 33, 36, 35,
	33, 36, 35, 33, 34, 35, 36, 34, 33, 34, 35, 36, 34, 33, 36, 35,
	34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 21, 22, 23, 24, 21, 37,
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 0, 0, 0, 0
};
const byte kG04Chunk14AltFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	27, 26, 25, 24, 19, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 55, 55, 0
};
const byte kG04Chunk16PostItemFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5, 6,
	5, 4, 5, 6, 5, 4, 5, 6, 5, 4, 3, 2, 1, 0, 7, 8,
	9, 10, 7, 0, 0, 0, 0, 0, 0, 0
};
const byte kG04MajorHotspotFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 6, 7, 8, 7, 6, 6, 7, 8,
	9, 8, 7, 6, 6, 7, 8, 9, 10, 9, 8, 7, 6, 6, 7, 8,
	9, 10, 11, 11, 11, 11, 12, 13, 14, 15, 16, 17, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27
};
const byte kG04Chunk10ExitFrameMap[] = { 0, 0, 1, 2, 3, 4 };
const byte kG04Chunk18PickupItem0FFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9
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

Scene7040::Scene7040(HollywoodEngine *vm) :
		_vm(vm),
		_resourceArenaCursor(0),
		_random("scene7040"),
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
		_chunk5FrameMillis(kG04Chunk11FrameMillis),
		_ambientMusicTimerAccumulator(0),
		_secondaryActorTimerAccumulator(0),
		_primaryLeftSpeechTimerAccumulator(0),
		_previousAmbientMusicTrackId(0),
		_activeActorWorldX(kG04Entry7040FirstTargetX),
		_activeActorWorldY(kG04Entry7040FirstTargetY),
		_activeActorFacing(kG04Entry7040Facing),
		_activeActorCel(0),
		_activeActorDrawOrderMode(0),
		_secondaryActorFrame(0),
		_actionOverlayVisible(false),
		_actionOverlayChunkIndex(0),
		_actionOverlayDescriptorCount(0),
		_actionOverlayFrameIndex(0),
		_chunk11FrameIndex(0),
		_chunk12FrameIndex(0),
		_chunk14ActionFrameIndex(0),
		_chunk14AltFrameIndex(0),
		_chunk16FrameIndex(1),
		_chunk17FrameIndex(0),
		_preItemIdleState(0),
		_postItemIdleState(0),
		_chunk12OverlayVisible(false),
		_chunk14ActionVisible(false),
		_chunk14AltVisible(false),
		_hideActiveActor(false),
		_chunk11TimerAccumulator(0),
		_chunk12TimerAccumulator(0),
		_chunk16TimerAccumulator(0),
		_chunk17TimerAccumulator(0),
		_skipRequested(false) {
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));
	_paletteResource.resize(kPaletteSize);
	_paletteCurrent.resize(kPaletteSize);
	_baseFramebufferOriginal.resize(kFrameBufferSize);
	_baseFramebuffer.resize(kFrameBufferSize);
	_sceneFramebuffer.resize(kFrameBufferSize);
	_savedFramebuffer.resize(kFrameBufferSize);
	_paletteMaskOriginal.resize(0x700);
	_fullPaletteRegionMask.resize(kG04PaletteMaskUsedBytes);
	_walkablePaletteMask.resize(kG04PaletteMaskUsedBytes);
	_colorToActorDepthClassMap.resize(kScenePaletteMapPageSize);
	_actorDepthYThresholds.resize(kScenePaletteRegionCount);
	_drawActorDepthYThresholds.resize(kScenePaletteRegionCount);
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
	_speechOverlay.colorIndex = kG04SecondarySpeechTextColor;
	_speechOverlay.centerX = 0;
	_speechOverlay.topY = 0;
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.colorIndex = kG04PrimarySpeechTextColor;
	_primarySpeechOverlay.centerX = 0;
	_primarySpeechOverlay.topY = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
}

bool Scene7040::play() {
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

bool Scene7040::load() {
	if (!loadResource000RuntimeTables(_resource000OffsetTable, _resource000SizeTable) ||
			!loadResource000ActorBankSet00(_resource000OffsetTable, _resource000SizeTable) ||
			!loadResource000InventoryActionTables(_resource000OffsetTable))
		return false;
	if (!_panelArt.load())
		return false;

	if (!_vm->resources()->readChunkTable(Common::Path(kG04ArchiveName), _g04ChunkTable)) {
		warning("Failed to read %s header", kG04ArchiveName);
		return false;
	}

	for (uint i = 0; i < kG04InitialRequiredChunkCount; ++i) {
		if (!_g04ChunkTable.isValidChunk(i)) {
			warning("%s is missing required Scene 7040 chunk %u", kG04ArchiveName, i);
			return false;
		}
	}

	if (!loadFixedChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadFixedChunk(1, _paletteResource, kPaletteSize) ||
			!loadVariableChunk(2, _fillRuns) ||
			!loadVariableChunk(3, _paletteMask) ||
			!loadVariableChunk(4, _metadata))
		return false;

	if (_vm->gameState().g01Item0BSequenceCompleted &&
			!loadFixedChunk(19, _paletteResource, kPaletteSize))
		return false;

	_baseFramebufferOriginal = _baseFramebuffer;
	_paletteMaskOriginal = _paletteMask;

	if (_paletteMask.size() < kG04PaletteMaskUsedBytes) {
		warning("%s chunk 3 is shorter than the G04 palette mask table", kG04ArchiveName);
		return false;
	}
	if (!initializeActorDepthTables())
		return false;
	if (!initializeScenePathTables())
		return false;

	uint32 arenaSize = 0;
	for (uint i = kG04ArenaFirstChunk; i <= kG04ArenaLastChunk; ++i)
		arenaSize += _g04ChunkTable.sizes[i];

	_resourceArena.resize(arenaSize);
	memset(_resourceArena.data(), 0, _resourceArena.size());
	_resourceArenaCursor = 0;
	memset(_resourceChunkOffsets, 0, sizeof(_resourceChunkOffsets));

	for (uint i = kG04ArenaFirstChunk; i <= kG04ArenaLastChunk; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	memset(_savedFramebuffer.data(), 0, _savedFramebuffer.size());
	expandFillRunsToSavedFramebuffer();
	for (uint i = 0; i < _savedFramebuffer.size(); ++i) {
		if (_savedFramebuffer[i] == 0xff)
			_savedFramebuffer[i] = 0xfa;
	}
	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	if (!loadResource000Owner1ActorPalette(_resource000OffsetTable) ||
			!loadStage003SceneRows())
		return false;
	_panelArt.applyInteractiveObjectPalette(_paletteCurrent);

	if (!_hotspots.load(_paletteMask, _metadata, _stage003SmallRows))
		return false;

	_vm->gameplayMusic()->setArchive(Common::Path(kGameplayMusicArchiveName));
	_soundBank0.setArchive(Common::Path(kGameplaySoundBank0ArchiveName));
	_ambientSoundBank0.setArchive(Common::Path(kGameplaySoundBank0ArchiveName));

	debugC(1, kDebugScene, "Scene 7040 loaded RESOURCE.G04");
	return true;
}

bool Scene7040::loadResource000RuntimeTables(Common::Array<byte> &offsetTable, Common::Array<byte> &sizeTable) {
	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s for Scene 7040 actor resources", kResource000Name);
		return false;
	}

	if ((uint32)file.size() < 1 + (2 * kResource000TableByteCount)) {
		warning("%s is too small for Scene 7040 runtime tables", kResource000Name);
		return false;
	}

	file.seek(1);
	offsetTable.resize(kResource000TableByteCount);
	sizeTable.resize(kResource000TableByteCount);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size() ||
			file.read(sizeTable.data(), sizeTable.size()) != sizeTable.size()) {
		warning("Failed to read %s runtime tables for Scene 7040", kResource000Name);
		return false;
	}

	return true;
}

bool Scene7040::loadResource000ActorBankSet00(const Common::Array<byte> &offsetTable, const Common::Array<byte> &sizeTable) {
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

	debugC(1, kDebugResources, "Loaded %s actor bank set 00 for Scene 7040", kResource000Name);
	return true;
}

bool Scene7040::loadResource000Owner1ActorPalette(const Common::Array<byte> &offsetTable) {
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

bool Scene7040::loadResource000InventoryActionTables(const Common::Array<byte> &offsetTable) {
	if (kResource000InventoryActionTablesEntry + 4 > offsetTable.size()) {
		warning("%s inventory action table entry is out of range", kResource000Name);
		return false;
	}

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s inventory action tables", kResource000Name);
		return false;
	}

	const uint32 tableOffset = readUint32LE(offsetTable, kResource000InventoryActionTablesEntry);
	const uint32 fixedTableOffset = tableOffset + kResource000FixedInventoryVerbTableOffset;
	const uint fixedTableEntryCount = GameplayState::kFixedInventoryActionTableEntryCount - 1;
	if (fixedTableOffset > (uint32)file.size() ||
			fixedTableEntryCount * 2 > (uint32)file.size() - fixedTableOffset) {
		warning("%s fixed inventory action table is out of range", kResource000Name);
		return false;
	}

	GameplayState &state = _vm->gameState();
	for (uint i = 0; i < GameplayState::kFixedInventoryActionTableEntryCount; ++i)
		state.fixedInventoryVerbHandlerIdsByItemAndStrip[i] = 0;

	file.seek(fixedTableOffset);
	for (uint i = 1; i < GameplayState::kFixedInventoryActionTableEntryCount; ++i)
		state.fixedInventoryVerbHandlerIdsByItemAndStrip[i] = file.readUint16LE();
	if (file.err()) {
		warning("Failed to read %s fixed inventory action table", kResource000Name);
		return false;
	}

	state.inventoryOwner1ResourceTablesLoaded = true;
	return true;
}

bool Scene7040::loadStage003SceneRows() {
	Common::File file;
	if (!file.open(Common::Path(kStage003ArchiveName))) {
		warning("Failed to open %s for Scene 7040 text", kStage003ArchiveName);
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

	_owner1SmallRows.resize((uint32)(owner1SmallRowCount + 1) * kStage003SmallRowSize);
	memset(_owner1SmallRows.data(), 0, _owner1SmallRows.size());
	_owner1LargeRows.resize((uint32)(owner1LargeRowCount + 1) * kStage003LargeRowSize);
	memset(_owner1LargeRows.data(), 0, _owner1LargeRows.size());
	file.seek(owner1RowsOffset);
	if (file.read(_owner1SmallRows.data() + kStage003SmallRowSize, owner1SmallRowBytes) != owner1SmallRowBytes) {
		warning("Failed to read %s owner 1 small text rows", kStage003ArchiveName);
		return false;
	}
	if (file.read(_owner1LargeRows.data() + kStage003LargeRowSize, owner1LargeRowBytes) != owner1LargeRowBytes) {
		warning("Failed to read %s owner 1 large text rows", kStage003ArchiveName);
		return false;
	}

	for (uint row = 1; row <= owner1SmallRowCount; ++row) {
		for (uint column = 0; column < kStage003SmallRowSize; ++column)
			_owner1SmallRows[row * kStage003SmallRowSize + column] -= _stage003DecodeKey[column];
	}

	for (uint row = 1; row <= owner1LargeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_owner1LargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
	}

	const uint32 stageOffsetEntry = kStage003DecodeKeySize + (kG04StageIndex * 4);
	if (stageOffsetEntry + 4 > kStage003DecodeKeySize + kStage003StageOffsetTableSize ||
			stageOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s has no stage 704 offset entry", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffsetEntry);
	const uint32 stageOffset = file.readUint32LE();
	if (stageOffset + kStage003DescriptorTableSize + 3 > (uint32)file.size()) {
		warning("%s stage 704 descriptor table is out of range", kStage003ArchiveName);
		return false;
	}

	file.seek(stageOffset);
	if (file.read(_stage003StageBlock.data(), _stage003StageBlock.size()) != _stage003StageBlock.size()) {
		warning("Failed to read %s stage 704 descriptor table", kStage003ArchiveName);
		return false;
	}

	const byte smallRowCount = file.readByte();
	const uint16 largeRowCount = file.readUint16LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kStage003SmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kStage003LargeRowSize;
	if (file.pos() + smallRowBytes + largeRowBytes > file.size()) {
		warning("%s stage 704 text rows are out of range", kStage003ArchiveName);
		return false;
	}

	_stage003SmallRows.resize((uint32)(smallRowCount + 1) * kStage003SmallRowSize);
	memset(_stage003SmallRows.data(), 0, _stage003SmallRows.size());
	if (file.read(_stage003SmallRows.data() + kStage003SmallRowSize, smallRowBytes) != smallRowBytes) {
		warning("Failed to read %s stage 704 small text rows", kStage003ArchiveName);
		return false;
	}

	_stage003LargeRows.resize(largeRowBytes);
	if (file.read(_stage003LargeRows.data(), _stage003LargeRows.size()) != _stage003LargeRows.size()) {
		warning("Failed to read %s stage 704 large text rows", kStage003ArchiveName);
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

	debugC(1, kDebugResources, "Loaded %s stage 704 text rows: smallRows=%u largeRows=%u",
		kStage003ArchiveName, smallRowCount, largeRowCount);
	return true;
}

bool Scene7040::loadFixedChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kG04ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kG04ArchiveName, index);
		return false;
	}

	if (stream->size() > fixedSize || destination.size() < fixedSize) {
		warning("%s chunk %u does not fit its fixed Scene 7040 destination", kG04ArchiveName, index);
		return false;
	}

	memset(destination.data(), 0, destination.size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kG04ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s fixed chunk %u: size=%u", kG04ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene7040::loadVariableChunk(uint index, Common::Array<byte> &destination) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kG04ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kG04ArchiveName, index);
		return false;
	}

	destination.resize(stream->size());
	if (stream->read(destination.data(), stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kG04ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s variable chunk %u: size=%u", kG04ArchiveName, index, (uint)stream->size());
	return true;
}

bool Scene7040::loadArenaChunk(uint index) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->resources()->createChunkReadStream(Common::Path(kG04ArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kG04ArchiveName, index);
		return false;
	}

	if (_resourceArenaCursor + stream->size() > _resourceArena.size()) {
		warning("%s chunk %u does not fit the Scene 7040 resource arena", kG04ArchiveName, index);
		return false;
	}

	_resourceChunkOffsets[index] = _resourceArenaCursor;
	if (stream->read(_resourceArena.data() + _resourceArenaCursor, stream->size()) != (uint32)stream->size()) {
		warning("Failed to read %s chunk %u", kG04ArchiveName, index);
		return false;
	}

	debugC(1, kDebugResources, "Loaded %s arena chunk %u: offset=%u size=%u",
		kG04ArchiveName, index, _resourceArenaCursor, (uint)stream->size());
	_resourceArenaCursor += stream->size();
	return true;
}

bool Scene7040::initializeActorDepthTables() {
	if (_metadata.size() < kActorDepthThresholds + kScenePaletteRegionCount * 2) {
		warning("%s chunk 4 is too short for G04 actor depth thresholds", kG04ArchiveName);
		return false;
	}
	if (_paletteMask.size() < kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize) {
		warning("%s chunk 3 is too short for G04 actor depth map", kG04ArchiveName);
		return false;
	}

	for (uint i = 0; i < _actorDepthYThresholds.size(); ++i)
		_actorDepthYThresholds[i] = readUint16LE(_metadata, kActorDepthThresholds + i * 2);
	_drawActorDepthYThresholds = _actorDepthYThresholds;

	memcpy(_colorToActorDepthClassMap.data(),
		_paletteMask.data() + kSceneColorToActorDepthClassMap,
		_colorToActorDepthClassMap.size());
	return true;
}

void Scene7040::updateActorDepthThresholds(byte actorDrawOrderMode) {
	_drawActorDepthYThresholds = _actorDepthYThresholds;
	if (_drawActorDepthYThresholds.size() > 2)
		_drawActorDepthYThresholds[2] = actorDrawOrderMode == 6 ? 0x3e7 : 0x158;
}

void Scene7040::expandFillRunsToSavedFramebuffer() {
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

bool Scene7040::initializeScenePathTables() {
	const uint boundaryBytes = kSceneRouteBoundaryPointCount * 4;
	if (_metadata.size() < kRouteBoundaryPoints + boundaryBytes ||
			_metadata.size() < kRouteBoundarySteps + kSceneRouteStepCount) {
		warning("%s chunk 4 is too short for G04 path route tables", kG04ArchiveName);
		return false;
	}

	memcpy(_fullPaletteRegionMask.data(), _paletteMask.data(), _fullPaletteRegionMask.size());
	memcpy(_walkablePaletteMask.data(), _paletteMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 3)
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

void Scene7040::initializePreviewState() {
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
	_chunk11FrameIndex = 0;
	_chunk12FrameIndex = 0;
	_chunk14ActionFrameIndex = 0;
	_chunk14AltFrameIndex = 0;
	_chunk16FrameIndex = 1;
	_chunk17FrameIndex = 0;
	_preItemIdleState = 0;
	_postItemIdleState = 0;
	_chunk12OverlayVisible = false;
	_chunk14ActionVisible = false;
	_chunk14AltVisible = false;
	_hideActiveActor = false;
	_chunk6IdlePairAAltPhase = _random.getRandomNumber(1) != 0;
	_chunk6IdlePairBAltPhase = _random.getRandomNumber(1) != 0;
	_primaryLeftSpeechActive = false;
	_chunk6IdlePairATicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk6IdlePairBTicksRemaining = (byte)(_random.getRandomNumber(0x18) + 10);
	_chunk9AmbientDecisionCounter = 0;
	_chunk5TimerAccumulator = 0;
	_chunk6TimerAccumulator = 0;
	_chunk5FrameMillis = kG04Chunk11FrameMillis;
	_ambientMusicTimerAccumulator = 0;
	_secondaryActorTimerAccumulator = 0;
	_primaryLeftSpeechTimerAccumulator = 0;
	_chunk11TimerAccumulator = 0;
	_chunk12TimerAccumulator = 0;
	_chunk16TimerAccumulator = 0;
	_chunk17TimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	_activeActorWorldX = kG04Entry7040FirstTargetX;
	_activeActorWorldY = kG04Entry7040FirstTargetY;
	_activeActorFacing = kG04Entry7040Facing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene7040::drawPreviewComposite() {
	drawCutsceneComposite(false, 0, 0, 0, 0, false, 0, 0, 0, 0);
}

void Scene7040::drawCutsceneComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	if (_vm->gameState().g01Item0BSequenceCompleted) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[17], 0,
			kG04Chunk17DescriptorCount, _chunk17FrameIndex, _sceneFramebuffer);
		const byte frame = _chunk16FrameIndex < ARRAYSIZE(kG04Chunk16PostItemFrameMap) ?
			kG04Chunk16PostItemFrameMap[_chunk16FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[16], 0,
			kG04Chunk16DescriptorCount, frame, _sceneFramebuffer);
	} else {
		if (_chunk12OverlayVisible) {
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[12], 0,
				kG04Chunk12DescriptorCount, _chunk12FrameIndex, _sceneFramebuffer);
		}
		const byte frame = _chunk11FrameIndex < ARRAYSIZE(kG04Chunk11FrameMap) ?
			kG04Chunk11FrameMap[_chunk11FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[11], 0,
			kG04Chunk11DescriptorCount, frame, _sceneFramebuffer);
		if (_chunk14ActionVisible) {
			const byte actionFrame = _chunk14ActionFrameIndex < ARRAYSIZE(kG04Chunk14ActionFrameMap) ?
				kG04Chunk14ActionFrameMap[_chunk14ActionFrameIndex] : 0;
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[14], 0,
				kG04Chunk14ActionDescriptorCount, actionFrame, _sceneFramebuffer);
		}
		if (_chunk14AltVisible) {
			const byte altFrame = _chunk14AltFrameIndex < ARRAYSIZE(kG04Chunk14AltFrameMap) ?
				kG04Chunk14AltFrameMap[_chunk14AltFrameIndex] : 0;
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[14], 0,
				kG04Chunk14AltDescriptorCount, altFrame, _sceneFramebuffer);
		}
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

	uint blockChunk = 5;
	if (actorDrawOrderMode == 2 || actorDrawOrderMode == 3) {
		blockChunk = activeWorldY <= 0x15f ? 6 : 0;
	} else if (actorDrawOrderMode == 6) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
		blockChunk = _vm->gameState().g04PatchState == 1 ? 9 : 0;
	}
	if (blockChunk != 0)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[blockChunk], _sceneFramebuffer);
}

void Scene7040::drawActionOverlayComposite() {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	if (_chunk12OverlayVisible) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[12], 0,
			kG04Chunk12DescriptorCount, _chunk12FrameIndex, _sceneFramebuffer);
	}

	if (_vm->gameState().g01Item0BSequenceCompleted) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[17], 0,
			kG04Chunk17DescriptorCount, _chunk17FrameIndex, _sceneFramebuffer);
		const byte frame = _chunk16FrameIndex < ARRAYSIZE(kG04Chunk16PostItemFrameMap) ?
			kG04Chunk16PostItemFrameMap[_chunk16FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[16], 0,
			kG04Chunk16DescriptorCount, frame, _sceneFramebuffer);
		if (_actionOverlayVisible) {
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
				_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
		}
	} else {
		if (_actionOverlayVisible) {
			drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
				_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
		}
		const byte frame = _chunk11FrameIndex < ARRAYSIZE(kG04Chunk11FrameMap) ?
			kG04Chunk11FrameMap[_chunk11FrameIndex] : 0;
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[11], 0,
			kG04Chunk11DescriptorCount, frame, _sceneFramebuffer);
	}

	uint blockChunk = 5;
	if (_activeActorDrawOrderMode == 2 || _activeActorDrawOrderMode == 3) {
		blockChunk = _activeActorWorldY <= 0x15f ? 6 : 0;
	} else if (_activeActorDrawOrderMode == 6) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
		blockChunk = _vm->gameState().g04PatchState == 1 ? 9 : 0;
	}
	if (blockChunk != 0)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[blockChunk], _sceneFramebuffer);
}

void Scene7040::drawPlayableComposite() {
	const bool drawActiveActor = !_hideActiveActor;
	drawCutsceneComposite(drawActiveActor, _activeActorFacing, _activeActorCel, _activeActorWorldX, _activeActorWorldY,
		false, _activeActorFacing, _secondaryActorFrame, _activeActorWorldX, _activeActorWorldY,
		_activeActorDrawOrderMode);
}

void Scene7040::drawMappedSpriteFrame(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize, byte frameIndex) {
	if (chunkIndex >= HollywoodEngine::kResourceChunkCount || frameIndex >= frameMapSize)
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[chunkIndex], 0,
		descriptorCount, frameMap[frameIndex], _sceneFramebuffer);
}

void Scene7040::drawActiveActorFrame(byte facing, byte cel, int worldX, int worldY, int minimumYExclusive) {
	if (facing >= kActorFacingCount || cel >= kActorCelsPerFacing)
		return;

	const uint descriptorIndex = facing * kActorCelsPerFacing + cel;
	if (descriptorIndex >= _activeActorDescriptors.size())
		return;

	const ActiveActorSpriteDescriptor &descriptor = _activeActorDescriptors[descriptorIndex];
	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	drawActorRun(_activeActorRunStreams, descriptor.runStreamOffset, facing * kActiveActorFacingRunStride,
		descriptor.opaqueRunCount, spriteX, spriteY, minimumYExclusive, worldY);
}

int Scene7040::drawSecondaryActorFrame(byte facing, byte frame, int worldX, int worldY) {
	if (facing >= kActorFacingCount || frame >= kSecondaryActorFramesPerFacing)
		return -1;

	const uint descriptorIndex = facing * kSecondaryActorFramesPerFacing + frame;
	if (descriptorIndex >= _secondaryActorDescriptors.size())
		return -1;

	const SecondaryActorSpriteDescriptor &descriptor = _secondaryActorDescriptors[descriptorIndex];
	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	return drawActorRun(_secondaryActorRunStreams, descriptor.runStreamOffset, facing * kSecondaryActorFacingRunStride,
		descriptor.runCount, spriteX, spriteY, -1, worldY);
}

int Scene7040::drawActorRun(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, int actorWorldY) {
	(void)actorWorldY;

	return drawActorRunStream(runStreams, cursor, runBase, runCount, spriteX, spriteY,
		minimumYExclusive, _sceneFramebuffer, nullptr);
}

void Scene7040::runEntryCutscene() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kG04State7041) {
		_soundBank0.playSample(4, 100);
		runEntryPath(kG04Entry7041StartX, kG04Entry7041StartY, kG04Entry7041Facing,
			kG04Entry7041StartX, kG04Entry7041StartY);
	} else if (state.mainFlowStateId == kG04State7042) {
		runEntryPath(kG04Entry7042StartX, kG04Entry7042StartY, kG04Entry7042Facing,
			kG04Entry7042TargetX, kG04Entry7042TargetY);
	} else {
		const int targetX = state.g04EntryConversationPlayed ?
			kG04Entry7040RepeatTargetX : kG04Entry7040FirstTargetX;
		const int targetY = state.g04EntryConversationPlayed ?
			kG04Entry7040RepeatTargetY : kG04Entry7040FirstTargetY;
		runEntryPath(kG04Entry7040StartX, kG04Entry7040StartY, kG04Entry7040Facing,
			targetX, targetY);
		state.g04EntryConversationPlayed = true;
	}
}

void Scene7040::runEntryPath(int startX, int startY, byte startFacing, int targetX, int targetY) {
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
		if (waitSceneMillis(kG04ActorPathFrameMillis))
			return;
	}

	_activeActorWorldX = targetX;
	_activeActorWorldY = targetY;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_activeActorCel = 0;
	drawPlayableComposite();
	presentFrame();
}

bool Scene7040::runBasicGameplayLoop() {
	GameplayLoop loop(_vm, this);
	return loop.run();
}

const SceneHotspotTable &Scene7040::hotspots() const {
	return _hotspots;
}

const Common::Array<byte> &Scene7040::savedFramebuffer() const {
	return _savedFramebuffer;
}

uint16 Scene7040::viewportXOffset() const {
	return kG04InitialViewportXOffset;
}

uint16 Scene7040::viewportYOffset() const {
	return 0;
}

void Scene7040::prepareGameplayLoop() {
	clearAllSpeechOverlays();
	_primaryLeftSpeechActive = false;
	_primaryLeftSpeechTimerAccumulator = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	_actionOverlayVisible = false;
	_chunk12OverlayVisible = false;
	_chunk14ActionVisible = false;
	_chunk14AltVisible = false;
	_hideActiveActor = false;
}

void Scene7040::advanceGameplayLoop(uint32 delta) {
	if (_vm->gameState().g01Item0BSequenceCompleted)
		advanceChunk16PostItemAnimation(delta);
	else
		advanceChunk11PreItemIdleAnimation(delta);

	updateAmbientAudioAndMusicCues(delta);
}

void Scene7040::drawGameplayFrame() {
	drawPlayableComposite();
}

void Scene7040::presentGameplayFrame(const SceneHoverCaption &hoverCaption, const GameplayPanelState &panelState) {
	presentFrame(&hoverCaption, &panelState);
}

void Scene7040::prepareOptionsMenuPalette(Common::Array<byte> &palette) const {
	palette = _paletteCurrent;
	_panelArt.applyInteractiveObjectPalette(palette);
}

bool Scene7040::shouldExitGameplayLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return stateId < kG04State7040 || stateId > kG04LastInteractiveState;
}

Common::String Scene7040::inventoryItemName(byte owner, byte itemId) const {
	if (owner != 1)
		return Common::String();

	const uint offset = (uint)itemId * kStage003SmallRowSize;
	if (offset >= _owner1SmallRows.size())
		return Common::String();

	const byte *row = _owner1SmallRows.data() + offset;
	uint length = 0;
	while (offset + length < _owner1SmallRows.size() &&
			length < kStage003SmallRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

void Scene7040::handleLeftClick(const GameplayLoopCursorState &state) {
	_vm->cursor()->leaveInteractiveMode();
	processSceneActionClick(state);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void Scene7040::handleInventoryItemClick(const GameplayLoopCursorState &state) {
	_vm->cursor()->leaveInteractiveMode();
	dispatchSceneAction(state.inventoryActionHandlerId);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void Scene7040::processSceneActionClick(const GameplayLoopCursorState &state) {
	byte itemId = state.resolvedItem;
	if (state.relationModeActive) {
		processSceneRelationClick(state, itemId);
		return;
	}

	if (itemId == 0) {
		if (state.currentStrip != 1)
			return;

		int targetX = state.sceneX;
		int targetY = state.sceneY;
		adjustWalkTargetToFloorMask(targetX, targetY);
		walkActiveActorTo(targetX, targetY, kInvalidFacing, 0);
		return;
	}

	SceneVerbActionRecord actionRecord = _hotspots.verbActionRecord(itemId, state.currentStrip);
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

void Scene7040::processSceneRelationClick(const GameplayLoopCursorState &state, byte itemId) {
	if (itemId == 0)
		return;

	const SceneVerbActionRecord actionRecord =
		relationActionRecord(state.primaryInventoryItem, itemId, state.relationMode);
	if (actionRecord.actionHandlerId == 0)
		return;

	const SceneActionTarget target = _hotspots.actionTarget(itemId);
	int targetX = _activeActorWorldX;
	int targetY = _activeActorWorldY;
	byte finalFacing = kInvalidFacing;

	if (actionRecord.movementMode != 0) {
		targetX = target.interactionPoint.x;
		targetY = target.interactionPoint.y;
		finalFacing = target.facing;
	} else {
		const bool atInteractionPoint =
			_activeActorWorldX == target.interactionPoint.x &&
			_activeActorWorldY == target.interactionPoint.y;
		if (atInteractionPoint) {
			if (_activeActorFacing != target.facing)
				finalFacing = target.facing;
		} else if (target.approachPoint.x != 0 || target.approachPoint.y != 0) {
			finalFacing = calculateFacingTowardPoint(_activeActorWorldX, _activeActorWorldY,
				target.approachPoint.x, target.approachPoint.y);
		}
	}

	walkActiveActorTo(targetX, targetY, finalFacing, 0);
	dispatchSceneAction(actionRecord.actionHandlerId);
}

SceneVerbActionRecord Scene7040::relationActionRecord(byte inventoryItemId, byte sceneItemId, byte relationMode) const {
	SceneVerbActionRecord record;
	record.actionHandlerId = 0;
	record.movementMode = 0;

	if (sceneItemId >= HollywoodEngine::kSceneItemCount)
		return record;

	const uint tableOffset = relationMode == 2 ? kSceneMode2RelationOverlay : kSceneRelationRecords;
	const uint recordIndex = (uint)inventoryItemId * HollywoodEngine::kSceneItemCount + sceneItemId;
	const uint offset = tableOffset + recordIndex * kG04RelationRecordSize;
	if (offset + kG04RelationRecordSize > _metadata.size())
		return record;

	record.actionHandlerId = readUint16LE(_metadata, offset);
	record.movementMode = readUint16LE(_metadata, offset + 2);
	return record;
}

void Scene7040::dispatchSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 0:
	case 1:
		break;
	case 2:
		beginStaticSecondarySpeechLine(1, (byte)_random.getRandomNumber(1));
		break;
	case 3:
		beginStaticSecondarySpeechLine(2, 0);
		break;
	case 4:
		beginStaticSecondarySpeechLine(3, (byte)_random.getRandomNumber(1));
		break;
	case 5:
	{
		const byte variant = (byte)_random.getRandomNumber(2);
		if (variant == 2)
			beginStaticSecondarySpeechLine(3, 1);
		else
			beginStaticSecondarySpeechLine(4, variant);
		break;
	}
	case 6:
		beginStaticSecondarySpeechLine(5, 0);
		break;
	case 7:
		beginStaticSecondarySpeechLine(6, (byte)_random.getRandomNumber(1));
		break;
	case 8:
		beginStaticSecondarySpeechLine(7, 0);
		break;
	case 9:
		beginStaticSecondarySpeechLine(8, 0);
		break;
	case 10:
		beginStaticSecondarySpeechLine(9, (byte)_random.getRandomNumber(1));
		break;
	case 11:
		beginStaticSecondarySpeechLine(0x0a, 0);
		break;
	case 12:
		beginStaticSecondarySpeechLine(0x0b, 0);
		break;
	case 13:
		beginStaticSecondarySpeechLine(0x0c, (byte)_random.getRandomNumber(1));
		break;
	case 14:
		beginStaticSecondarySpeechLine(0x0d, (byte)_random.getRandomNumber(1));
		break;
	case 15:
		beginStaticSecondarySpeechLine(0x0e, 0);
		break;
	case 16:
		beginStaticSecondarySpeechLine(0x0f, (byte)_random.getRandomNumber(2));
		break;
	case 17:
		beginStaticSecondarySpeechLine(0x10, 0);
		break;
	case 18:
		beginStaticSecondarySpeechLine(0x11, (byte)_random.getRandomNumber(1));
		break;
	case 19:
		beginStaticSecondarySpeechLine(0x12, (byte)_random.getRandomNumber(2));
		break;
	case 20:
		beginStaticSecondarySpeechLine(0x13, 0);
		break;
	case 21:
		beginStaticSecondarySpeechLine(0x14, 0);
		break;
	case 22:
		beginStaticSecondarySpeechLine(0x15, 0);
		break;
	case 23:
		beginStaticSecondarySpeechLine(0x16, (byte)_random.getRandomNumber(1));
		break;
	case 24:
		beginStaticSecondarySpeechLine(0x17, (byte)_random.getRandomNumber(1));
		break;
	case 25:
		beginStaticSecondarySpeechLine(0x18, (byte)_random.getRandomNumber(1));
		break;
	case 26:
		beginStaticSecondarySpeechLine(0x19, 0);
		break;
	case 27:
		beginStaticSecondarySpeechLine(0x1a, 0);
		break;
	case 28:
		beginStaticSecondarySpeechLine(0x1b, 0);
		break;
	case 29:
		beginStaticSecondarySpeechLine(0x1c, 0);
		break;
	case 30:
		beginStaticSecondarySpeechLine(0x1d, 0);
		break;
	case 31:
		beginStaticSecondarySpeechLine(0x1e, 0);
		break;
	case 32:
		beginStaticSecondarySpeechLine(0x1f, 0);
		break;
	case 33:
		beginStaticSecondarySpeechLine(0x20, 0);
		break;
	case 34:
		beginStaticSecondarySpeechLine(0x21, 0);
		break;
	case 36:
		beginStaticSecondarySpeechLine(0x23, 0);
		break;
	case 38:
		beginStaticSecondarySpeechLine(0x25, 0);
		break;
	case 39:
		beginStaticSecondarySpeechLine(0x26, 0);
		break;
	case 40:
		beginStaticSecondarySpeechLine(0x27, 0);
		break;
	case 301:
		handleActionSlot00ReturnToG03();
		break;
	case 302:
		handleActionSlot01ProgressSpeech();
		break;
	case 303:
		handleActionSlot02MajorHotspotAction();
		break;
	case 304:
		handleActionSlot03TransitionToState7060();
		break;
	case 305:
		break;
	case 306:
		handleActionSlot05ExitProgressSpeech();
		break;
	case 307:
		handleActionSlot06TransitionToG05();
		break;
	case 308:
		break;
	case 309:
		break;
	case 310:
		handleActionSlot09PickupItem0FThenExit();
		break;
	case 311:
		handleActionSlot10CommonSpeech();
		break;
	case 312:
		handleActionHandler312ProgressSpeech();
		break;
	case 313:
		handleActionHandler313ConversationGate();
		break;
	case 314:
		handleActionHandler314Item0BSpeech();
		break;
	default:
		warning("Unhandled Scene7040 action handler %u", handlerId);
		break;
	}
}

void Scene7040::walkActiveActorTo(int targetX, int targetY, byte finalFacing, byte finalCel) {
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
		waitSceneMillis(kG04ActorPathFrameMillis);
	}

	drawPlayableComposite();
	presentFrame();
}

void Scene7040::adjustWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetX > 0x30f)
		targetX = 0x30f;

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

void Scene7040::queueActorPathWithPaletteRegionRouting(int startX, int startY, int targetX, int targetY,
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
			if (currentRegion == 3 && nextRegion == 3 &&
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
	if (currentRegion == 3 && targetRegion == 3 && state.x < targetX && targetY <= state.y)
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

void Scene7040::buildActorPathFramesBetweenPoints(ActorPathBuildState &state, int targetX, int targetY,
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

void Scene7040::appendActorPathFrame(const ActorPathBuildState &state) {
	ActorPathFrame frame;
	frame.drawOrderMode = state.drawOrderMode;
	frame.facing = state.facing;
	frame.cel = state.cel;
	frame.worldX = (int16)CLIP<int>(state.x, -32768, 32767);
	frame.worldY = (int16)CLIP<int>(state.y, -32768, 32767);
	_actorPathFrames.push_back(frame);
}

ScenePoint Scene7040::nearestPaletteRouteBoundaryPoint(int startX, int startY, byte currentRegion, byte nextRegion) const {
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

ScenePoint Scene7040::bestPaletteRouteBoundaryPoint(int startX, int startY, int targetX, int targetY,
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

byte Scene7040::paletteRegionAt(int x, int y) const {
	if (x < 0 || y < 0 || x >= HollywoodEngine::kSceneBufferWidth || y >= HollywoodEngine::kSceneBufferHeight ||
			_fullPaletteRegionMask.empty())
		return 0;

	const uint offset = y * HollywoodEngine::kSceneBufferWidth + x;
	if (offset >= _savedFramebuffer.size())
		return 0;

	return _fullPaletteRegionMask[_savedFramebuffer[offset]];
}

byte Scene7040::calculateMovementFacingForPath(int fromX, int fromY, int toX, int toY, int requestedFacing) const {
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

uint Scene7040::calculateWalkStepCountForAxisDelta(int startAxis, int targetAxis, byte facing, byte cel) const {
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

byte Scene7040::nextActorPathCel(byte cel) const {
	return cel == 12 ? 1 : (byte)(cel + 1);
}

uint Scene7040::actorPathStepDelta(byte facing, byte cel) const {
	if (facing >= kActorFacingCount || cel == 0 || cel > 12)
		return 0;

	const uint offset = (uint)facing * 12 + cel - 1;
	if (offset >= _actorPathStepDeltas.size())
		return 0;

	return _actorPathStepDeltas[offset];
}

byte Scene7040::calculateFacingTowardPoint(int fromX, int fromY, int toX, int toY) const {
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

void Scene7040::applySceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0 || selector == 0xff) {
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		for (uint i = 0; i < _fullPaletteRegionMask.size(); ++i) {
			if (_paletteMaskOriginal[i] == 7)
				_fullPaletteRegionMask[i] = _vm->gameState().g01Item0BSequenceCompleted ? 0 : 1;
		}

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMapOffset + kSceneColorMapSize &&
				_paletteMask.size() >= kSceneColorToItemMapOffset + kSceneColorMapSize) {
			for (uint i = 0; i < kSceneColorMapSize; ++i) {
				const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMapOffset + i];
				if (!_vm->gameState().g01Item0BSequenceCompleted) {
					if (originalItem == 9)
						_paletteMask[kSceneColorToItemMapOffset + i] = 2;
					if (originalItem == 10)
						_paletteMask[kSceneColorToItemMapOffset + i] = 0;
				} else {
					if (originalItem == 8)
						_paletteMask[kSceneColorToItemMapOffset + i] = 0;
					if (originalItem == 9 || originalItem == 10)
						_paletteMask[kSceneColorToItemMapOffset + i] = 8;
				}
			}
		}

		if (_vm->gameState().g01Item0BSequenceCompleted) {
			if (_metadata.size() >= kSceneItemInteractionPoints + 9 * 4 &&
					_metadata.size() >= kSceneItemFacing + 3) {
				const uint item2Interaction = kSceneItemInteractionPoints + 2 * 4;
				_metadata[item2Interaction] = 0xf2;
				_metadata[item2Interaction + 1] = 0;
				_metadata[item2Interaction + 2] = 0x46;
				_metadata[item2Interaction + 3] = 1;
				const uint item8Interaction = kSceneItemInteractionPoints + 8 * 4;
				_metadata[item8Interaction] = 0xf2;
				_metadata[item8Interaction + 1] = 0;
				_metadata[item8Interaction + 2] = 0x46;
				_metadata[item8Interaction + 3] = 1;
				_metadata[kSceneItemFacing + 2] = 1;
			}
		}

		rebuildWalkablePaletteMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	if (selector == 3 || selector == 0xff) {
		if (!_baseFramebufferOriginal.empty())
			memcpy(_baseFramebuffer.data(), _baseFramebufferOriginal.data(), _baseFramebuffer.size());

		if (_vm->gameState().g04PatchState == 1) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
		} else {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		}
	}
}

void Scene7040::rebuildWalkablePaletteMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 3)
			_walkablePaletteMask[i] = 0;
	}
}

bool Scene7040::hasInventoryItem(byte itemId) const {
	const byte owner = _vm->gameState().currentInventoryOwnerIndex;
	return _vm->gameState().hasInventoryItem(owner, itemId);
}

void Scene7040::addInventoryItem(byte itemId) {
	GameplayState &state = _vm->gameState();
	state.addInventoryItem(state.currentInventoryOwnerIndex, itemId);
}

void Scene7040::removeInventoryItem(byte itemId) {
	GameplayState &state = _vm->gameState();
	state.removeInventoryItem(state.currentInventoryOwnerIndex, itemId);
}

void Scene7040::handleActionSlot00ReturnToG03() {
	_vm->gameState().mainFlowStateId = kG04ReturnState7031;
}

void Scene7040::handleActionSlot01ProgressSpeech() {
	beginSecondarySpeechLine(1, _vm->gameState().g04MajorActionProgress == 0 ? 0 : 1);
}

void Scene7040::handleActionSlot02MajorHotspotAction() {
	GameplayState &state = _vm->gameState();
	if (state.g01Item0BSequenceCompleted) {
		beginSecondarySpeechLine(3, 0x0b);
		return;
	}

	_chunk12OverlayVisible = true;
	runMappedActionOverlay(13, kG04Chunk13DescriptorCount, kG04MajorHotspotFrameMap,
		ARRAYSIZE(kG04MajorHotspotFrameMap), kG04Chunk14FrameMillis, 0x2c, false);
	_chunk12OverlayVisible = false;

	walkActiveActorTo(0x10d, 0x124, state.g04MajorActionProgress == 2 ? 4 : 5, 0);
	switch (state.g04MajorActionProgress) {
	case 0:
		beginSecondarySpeechLine(2, 0);
		state.g04MajorActionProgress = 1;
		break;
	case 1:
		beginSecondarySpeechLine(2, 1);
		state.g04MajorActionProgress = 2;
		break;
	case 2:
		beginSecondarySpeechLine(3, 10);
		state.g04MajorActionProgress = 3;
		_sceneStateFlags[1] = 1;
		break;
	default:
		beginSecondarySpeechLine(2, 2);
		break;
	}
}

void Scene7040::handleActionSlot03TransitionToState7060() {
	_vm->gameState().mainFlowStateId = kG04ExitState7060;
}

void Scene7040::handleActionSlot05ExitProgressSpeech() {
	beginSecondarySpeechLine(5, _vm->gameState().g04ExitActionDone ? 1 : 0);
}

void Scene7040::handleActionSlot06TransitionToG05() {
	runMappedActionOverlay(10, kG04Chunk10DescriptorCount, kG04Chunk10ExitFrameMap,
		ARRAYSIZE(kG04Chunk10ExitFrameMap), kG04Chunk14FrameMillis, -1, false);
	_vm->gameState().g04ExitActionDone = true;
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kG04ExitState7050;
}

void Scene7040::handleActionSlot09PickupItem0FThenExit() {
	GameplayState &state = _vm->gameState();
	if (state.g04MajorActionProgress <= 2 || state.g04PatchState == 2) {
		beginSecondarySpeechLine(9, 0);
		return;
	}
	if (!state.g01Item0BSequenceCompleted) {
		beginSecondarySpeechLine(8, 0);
		return;
	}

	beginSecondarySpeechLine(8, 1);
	runMappedActionOverlay(18, kG04Chunk18DescriptorCount, kG04Chunk18PickupItem0FFrameMap,
		ARRAYSIZE(kG04Chunk18PickupItem0FFrameMap), kG04Chunk14FrameMillis, -1, false);
	addInventoryItem(0x0f);
	_soundBank0.playSample(1, 100);
	state.g04PatchState = 2;
	beginSecondarySpeechLine(8, 2);
	walkActiveActorTo(600, 0x132, kInvalidFacing, 0);
	handleActionSlot06TransitionToG05();
}

void Scene7040::handleActionSlot10CommonSpeech() {
	beginSecondarySpeechLine(9, 0);
}

void Scene7040::handleActionHandler312ProgressSpeech() {
	GameplayState &state = _vm->gameState();
	if (state.g04MajorActionProgress == 3)
		beginSecondarySpeechLine(10, state.g04PatchState >= 2 ? 1 : 0);
	else
		beginSecondarySpeechLine(0x2d, 0);
}

void Scene7040::handleActionHandler313ConversationGate() {
	if (_vm->gameState().g01Item0BSequenceCompleted) {
		beginSecondarySpeechLine(11, 2);
		return;
	}
	beginSecondarySpeechLine(11, 0);
}

void Scene7040::handleActionHandler314Item0BSpeech() {
	beginSecondarySpeechLine(11, _vm->gameState().g01Item0BSequenceCompleted ? 1 : 0);
}

void Scene7040::runMappedActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int statePatchFrame) {
	runMappedActionOverlay(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis, statePatchFrame, false);
}

void Scene7040::runMappedActionOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int statePatchFrame, bool hideActiveActor) {
	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = hideActiveActor;
	_actionOverlayVisible = true;
	_actionOverlayChunkIndex = (byte)chunkIndex;
	_actionOverlayDescriptorCount = (byte)descriptorCount;
	for (uint frame = 0; frame < frameMapSize && !Engine::shouldQuit(); ++frame) {
		_actionOverlayFrameIndex = frameMap[frame];
		if (statePatchFrame >= 0 && (int)frame == statePatchFrame) {
			_soundBank0.playSample(0x15, 100);
			if (_vm->gameState().g04MajorActionProgress == 2) {
				_vm->gameState().g04PatchState = 1;
				applySceneStateToHotspotsAndPatches(3);
			}
		}
		if (waitSceneMillis(frameMillis))
			break;
	}
	_actionOverlayVisible = false;
	_actionOverlayFrameIndex = 0;
	_hideActiveActor = previousHideActiveActor;
	drawPlayableComposite();
	presentFrame();
}

bool Scene7040::waitSceneMillis(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit()) {
		if (pollEvents(true))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		advanceGameplayLoop(slice);
		if (_actionOverlayVisible)
			drawActionOverlayComposite();
		else
			drawPlayableComposite();
		presentFrame();
		remaining -= slice;
	}

	return Engine::shouldQuit();
}

void Scene7040::updateAmbientAudioAndMusicCues(uint32 delta) {
	_ambientMusicTimerAccumulator += delta;
	if (_ambientMusicTimerAccumulator < kG04AmbientMusicCheckMillis)
		return;
	_ambientMusicTimerAccumulator %= kG04AmbientMusicCheckMillis;

	if (!_ambientSoundBank0.isPlaying())
		_ambientSoundBank0.playSample(0x0b, 100);

	if (_vm->gameplayMusic()->isPlaying())
		return;

	GameplayState &state = _vm->gameState();
	if (state.currentRandomAmbientMusicTrackId != kG04AmbientMusicCueStillFrame) {
		_previousAmbientMusicTrackId = state.currentRandomAmbientMusicTrackId;
		state.currentRandomAmbientMusicTrackId = kG04AmbientMusicCueStillFrame;
		_chunk5FrameIndex = 0;
		_vm->gameplayMusic()->playMusicCue(state.currentRandomAmbientMusicTrackId, 100);
		return;
	}

	byte nextTrack = 0;
	do {
		nextTrack = (byte)(0x0c + _random.getRandomNumber(2));
	} while (nextTrack == _previousAmbientMusicTrackId);

	_previousAmbientMusicTrackId = state.currentRandomAmbientMusicTrackId;
	state.currentRandomAmbientMusicTrackId = nextTrack;
	_vm->gameplayMusic()->playMusicCue(state.currentRandomAmbientMusicTrackId, 100);
}

void Scene7040::advanceChunk5AmbientOverlay() {
}

void Scene7040::advanceChunk6IdleFrames() {
}

void Scene7040::advanceChunk11PreItemIdleAnimation(uint32 delta) {
	_chunk11TimerAccumulator += delta;
	while (_chunk11TimerAccumulator >= kG04Chunk11FrameMillis) {
		_chunk11TimerAccumulator -= kG04Chunk11FrameMillis;
		if (_preItemIdleState == 3)
			continue;

		if (_preItemIdleState == 0) {
			if (_random.getRandomNumber(0x31) == 0) {
				_preItemIdleState = 2;
				_chunk11FrameIndex = 0;
			} else if (_random.getRandomNumber(0x0e) == 0) {
				_preItemIdleState = 1;
				_chunk11FrameIndex = 1;
			}
		} else if (_preItemIdleState == 1) {
			_chunk11FrameIndex = 0;
			_preItemIdleState = 0;
		} else if (_preItemIdleState == 2) {
			if (_chunk11FrameIndex == 6) {
				_chunk11FrameIndex = 0;
				_preItemIdleState = 0;
			} else {
				++_chunk11FrameIndex;
			}
		}
	}
}

void Scene7040::advanceChunk16PostItemAnimation(uint32 delta) {
	_chunk16TimerAccumulator += delta;
	while (_chunk16TimerAccumulator >= kG04Chunk16FrameMillis) {
		_chunk16TimerAccumulator -= kG04Chunk16FrameMillis;
		switch (_postItemIdleState) {
		case 0:
			++_chunk16FrameIndex;
			if (_chunk16FrameIndex >= 5)
				_postItemIdleState = 1;
			break;
		case 1:
			++_chunk16FrameIndex;
			if (_chunk16FrameIndex >= 0x1a)
				_postItemIdleState = 2;
			break;
		case 2:
			++_chunk16FrameIndex;
			if (_chunk16FrameIndex >= 0x1e)
				_postItemIdleState = 3;
			break;
		default:
			if (_random.getRandomNumber(0x0e) == 0)
				_chunk16FrameIndex = 0x22;
			else
				_chunk16FrameIndex = 0x1e;
			break;
		}
		if (_chunk16FrameIndex >= ARRAYSIZE(kG04Chunk16PostItemFrameMap))
			_chunk16FrameIndex = 1;
	}

	_chunk17TimerAccumulator += delta;
	while (_chunk17TimerAccumulator >= kG04Chunk17FrameMillis) {
		_chunk17TimerAccumulator -= kG04Chunk17FrameMillis;
		if (_postItemIdleState > 1 || _chunk17FrameIndex != 0)
			_chunk17FrameIndex = _chunk17FrameIndex == 8 ? 0 : (byte)(_chunk17FrameIndex + 1);
	}
}

void Scene7040::advanceSecondaryActorSpeechFrame() {
	byte nextFrame = _secondaryActorFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _secondaryActorFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(kSecondaryActorFramesPerFacing - 1);

	if (nextFrame == _secondaryActorFrame)
		nextFrame = (byte)((_secondaryActorFrame + 1) % kSecondaryActorFramesPerFacing);

	_secondaryActorFrame = nextFrame;
}

void Scene7040::advancePrimaryLeftSpeechFrame() {
	byte nextFrame = _primaryLeftSpeechLastFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == _primaryLeftSpeechLastFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(3);

	if (nextFrame == _primaryLeftSpeechLastFrame)
		nextFrame = (byte)((_primaryLeftSpeechLastFrame + 1) % 4);

	_primaryLeftSpeechLastFrame = nextFrame;
	_chunk5FrameIndex = 0x0b + nextFrame;
}

void Scene7040::clearSpeechOverlay() {
	_speechOverlay.visible = false;
	_speechOverlay.lines.clear();
}

void Scene7040::clearAllSpeechOverlays() {
	clearSpeechOverlay();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
}

void Scene7040::drawSpeechOverlay() {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return;

	drawSpeechOverlay(_speechOverlay);
	drawSpeechOverlay(_primarySpeechOverlay);
}

void Scene7040::drawSpeechOverlay(const SpeechOverlay &overlay) {
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
		const int x = (int)overlay.centerX - (lineWidth >> 1) - kG04InitialViewportXOffset;
		const int y = (int)overlay.topY + lineIndex * kOriginalSpeechLineHeight;
		font->drawString(&screenSurface, line, x, y, lineWidth, overlay.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene7040::beginSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	runSpeechLine(_speechOverlay, rowIndex, frameIndex, _activeActorWorldX, 0,
		kG04SecondarySpeechTextColor, false, false);
}

bool Scene7040::startSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return false;
	(void)continuationCount;

	const Common::String text = getResource003LargeTextRecord(textRecordId);
	if (text.empty())
		return false;

	_speechOverlay.visible = true;
	_speechOverlay.colorIndex = kG04SecondarySpeechTextColor;
	wrapActorSpeechText(text, _activeActorWorldX, _speechOverlay.lines);
	calculateSecondarySpeechBounds(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	_secondaryActorTimerAccumulator = 0;

	return voiceSampleId != 0 && _speech.playSample(voiceSampleId, 100);
}

void Scene7040::beginStaticSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(_speechOverlay, textRecordId, continuationCount, voiceSampleId, _activeActorWorldX, 0,
		kG04SecondarySpeechTextColor, false, false);
}

void Scene7040::beginPrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue) {
	const uint paletteOffset = kG04PrimarySpeechTextColor * 3;
	if (_paletteCurrent.size() > paletteOffset + 2) {
		_paletteCurrent[paletteOffset] = red;
		_paletteCurrent[paletteOffset + 1] = green;
		_paletteCurrent[paletteOffset + 2] = blue;
	}

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, centerX, topY,
		kG04PrimarySpeechTextColor, true, false);
}

void Scene7040::beginPrimaryLeftSpeechLine(uint16 rowIndex, byte frameIndex) {
	const uint paletteOffset = kG04PrimarySpeechTextColor * 3;
	if (_paletteCurrent.size() > paletteOffset + 2) {
		_paletteCurrent[paletteOffset] = 0x33;
		_paletteCurrent[paletteOffset + 1] = 0x22;
		_paletteCurrent[paletteOffset + 2] = 0x39;
	}

	runSpeechLine(_primarySpeechOverlay, rowIndex, frameIndex, 0xfa, 0x136,
		kG04PrimarySpeechTextColor, true, true);
}

void Scene7040::runSpeechLine(SpeechOverlay &overlay, uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte colorIndex, bool useRequestedTop, bool animatePrimaryLeft) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runSpeechCue(overlay, textRecordId, continuationCount, voiceSampleId, centerX, topY, colorIndex,
		useRequestedTop, animatePrimaryLeft);
}

void Scene7040::runSpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount,
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

bool Scene7040::getStage003Cue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 100) * 5;
	if (offset + 5 > _stage003StageBlock.size())
		return false;

	textRecordId = readUint16LE(_stage003StageBlock, offset);
	continuationCount = _stage003StageBlock[offset + 2];
	voiceSampleId = readUint16LE(_stage003StageBlock, offset + 3);
	return textRecordId != 0;
}

bool Scene7040::getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId, byte &continuationCount,
		uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 10) * 5;
	if (offset + 5 > _owner1SpeechCueDescriptors.size())
		return false;

	textRecordId = readUint16LE(_owner1SpeechCueDescriptors, offset);
	continuationCount = _owner1SpeechCueDescriptors[offset + 2];
	voiceSampleId = readUint16LE(_owner1SpeechCueDescriptors, offset + 3);
	return textRecordId != 0;
}

void Scene7040::wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars = 0x32;
	const int anchorX = anchorSceneX - kG04InitialViewportXOffset;
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

Common::String Scene7040::getResource003LargeTextRecord(uint16 recordId) const {
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

uint Scene7040::actorSpeechTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

void Scene7040::calculateSecondarySpeechBounds(int actorWorldX, int actorWorldY) {
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

bool Scene7040::waitForSpeechOrDelay(uint32 fallbackMillis, bool animatePrimaryLeft) {
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

void Scene7040::applyGameplayPanelPalette() {
	if (_paletteCurrent.size() <= kG04PanelTextColor * 3 + 2)
		return;

	const bool originalPaletteApplied = _panelArt.applyInteractiveObjectPalette(_paletteCurrent);
	if (!originalPaletteApplied) {
		const byte colors[] = {
			kG04PanelDarkColor, 0x05, 0x06, 0x08,
			kG04PanelFillColor, 0x0b, 0x0d, 0x11,
			kG04PanelSlotColor, 0x14, 0x16, 0x1a,
			kG04PanelLineColor, 0x24, 0x25, 0x28,
			kG04PanelSelectedColor, 0x2e, 0x1d, 0x0e,
			kG04PanelSelectedLineColor, 0x3a, 0x2d, 0x16
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

	const uint textOffset = kG04PanelTextColor * 3;
	_paletteCurrent[textOffset] = 0x32;
	_paletteCurrent[textOffset + 1] = _paletteCurrent[0x2d7];
	_paletteCurrent[textOffset + 2] = _paletteCurrent[0x2d8];
}

void Scene7040::drawGameplayPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	if (panelState.inventoryPanelVisible)
		drawInventoryPanel(surface, panelState);
	else if (panelState.verbPanelVisible)
		drawVerbPanel(surface, panelState);
}

void Scene7040::drawVerbPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	_panelArt.drawVerbPanel(surface, _savedFramebuffer, kG04InitialViewportXOffset, 0, panelState,
		_vm->font());
}

void Scene7040::drawInventoryPanel(Graphics::Surface &surface, const GameplayPanelState &panelState) {
	_panelArt.drawDialogueInventoryPanel(surface, _savedFramebuffer, kG04InitialViewportXOffset, 0,
		panelState, _vm->gameState(), _vm->font());
}

void Scene7040::presentFrame(const SceneHoverCaption *hoverCaption, const GameplayPanelState *panelState) {
	if (hoverCaption)
		hoverCaption->applyPalette(_paletteCurrent);
	if (panelState && panelState->visible())
		applyGameplayPanelPalette();
	uploadPalette6Bit(_paletteCurrent);

	for (uint y = 0; y < HollywoodEngine::kScreenHeight; ++y) {
		const uint sourceOffset = kG04InitialViewportXOffset + y * HollywoodEngine::kSceneBufferWidth;
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

bool Scene7040::pollEvents(bool allowSkip) {
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

bool Scene7040::delay(uint32 millis) {
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
