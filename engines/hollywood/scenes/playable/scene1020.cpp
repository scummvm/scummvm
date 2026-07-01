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

#include "hollywood/scenes/playable/scene1020.h"

#include "common/debug.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene1020ArchiveName = "RESOURCE.A02";
const char *const kScene1020MusicArchiveName = "RESOURCE.M01";
const char *const kScene1020SoundArchiveName = "RESOURCE.S01";
const uint kScene1020InitialRequiredChunkCount = 23;
const uint kScene1020ArenaFirstChunk = 5;
const uint kScene1020ArenaLastChunk = 22;
const uint kScene1020StageIndex = 102;
const uint16 kScene1020FirstState = 0x03fc;
const uint16 kScene1020LastState = 0x0405;
const uint16 kScene1020RightEntryState = 0x03fc;
const uint16 kScene1020OverlayEntryState = 0x03fd;
const uint16 kScene1020ExitState1010RightEntry = 0x03f3;
const uint16 kScene1020ViewportXOffset = 0x0010;
const uint kScene1020ActorBankTableEntry = 0x0000;
const uint kScene1020ActorPaletteTableEntry = 0x00cc;
const uint kScene1020Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1020SpeechCueDescriptorTableOffset = 0x1135;
const int kScene1020DefaultActorX = 0x116;
const int kScene1020DefaultActorY = 0x143;
const byte kScene1020DefaultActorFacing = 2;
const int kScene1020RightEntryStartX = 0x050;
const int kScene1020RightEntryStartY = 0x108;
const int kScene1020RightEntryTargetX = 0x116;
const int kScene1020RightEntryTargetY = 0x143;
const byte kScene1020RightEntryFacing = 2;
const int kScene1020OverlayEntryX = 0x18d;
const int kScene1020OverlayEntryY = 0x155;
const byte kScene1020OverlayEntryFacing = 5;
const byte kScene1020FirstAmbientSoundCue = 0x25;
const byte kScene1020AmbientSoundCueCount = 7;
const byte kScene1020FirstAmbientMusicCue = 0x0b;
const byte kScene1020AmbientMusicCueCount = 5;
const byte kScene1020AmbientSoundProbabilityModulus = 25;
const byte kScene1020AmbientMusicProbabilityModulus = 50;
const uint32 kScene1020OverlayFrameMillis = 75;
const uint kScene1020ActionChunk14DescriptorCount = 6;
const uint kScene1020ActionChunk15DescriptorCount = 0x15;
const uint kScene1020ActionChunk16DescriptorCount = 0x0b;
const uint kScene1020ActionChunk17DescriptorCount = 0x15;
const uint kScene1020ActionChunk18DescriptorCount = 0x12;
const uint kScene1020ActionChunk19DescriptorCount = 0x0b;
const uint kScene1020ActionChunk22DescriptorCount = 0x0d;

const byte kScene1020ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene1020Chunk14ForwardFrameMap[] = {
	0, 1, 2, 3, 4, 5
};

const byte kScene1020Chunk14ReverseFrameMap[] = {
	0, 4, 3, 2, 1, 0
};

const byte kScene1020Chunk15PingPongFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 20, 20,
	20, 20, 19, 18, 17, 16, 15, 14,
	13, 12, 11, 10, 9, 8, 7, 6,
	5, 4, 3, 2, 1
};

const byte kScene1020Chunk16AlternatingFrameMap[] = {
	0, 1, 0, 1, 0, 1, 0, 1,
	0, 1, 0, 1, 0, 1, 0, 1,
	0, 1, 0, 1, 0
};

const byte kScene1020Chunk16ForwardFrameMap[] = {
	0, 0, 2, 3, 4, 5, 6, 7,
	8, 9, 10
};

const byte kScene1020Chunk17ForwardFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20
};

const byte kScene1020Chunk18StateChangeFrameMap[] = {
	0, 0, 1, 2, 3, 4, 17, 16,
	15, 14, 13, 12, 11, 8, 9, 10,
	9, 8, 9, 10, 9, 8, 9, 10,
	9, 8, 9, 10, 9, 8, 9, 10,
	9, 8, 9, 10, 9, 8, 4, 3,
	2, 1, 0
};

const byte kScene1020Chunk19EventFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 9, 8, 9, 10, 9,
	8, 9, 10, 9, 8, 9, 10, 9,
	8, 9, 10, 9, 8, 9, 10, 9,
	8, 7, 6, 5, 4, 3, 2, 1,
	0
};

const byte kScene1020Chunk22PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12
};

static PlayableSceneConfig scene1020Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene1020ArchiveName;
	config.initialRequiredChunkCount = kScene1020InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene1020ArenaFirstChunk;
	config.arenaLastChunk = kScene1020ArenaLastChunk;
	config.stageIndex = kScene1020StageIndex;
	config.debugName = "Scene 1020";
	config.viewportXOffset = kScene1020ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 1;
	config.actorBankTableEntry = kScene1020ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene1020ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene1020Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene1020SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene1020ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene1020ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 2;
	config.musicArchiveName = kScene1020MusicArchiveName;
	config.soundBank0ArchiveName = kScene1020SoundArchiveName;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene1020FirstState;
	config.mainFlowLastState = kScene1020LastState;
	return config;
}

Scene1020::Scene1020(HollywoodEngine *vm) :
		PlayableScene(vm, scene1020Config(), "scene1020", kScene1020DefaultActorX, kScene1020DefaultActorY,
			kScene1020DefaultActorFacing, 0xfd, 0xfb) {
}

bool Scene1020::shouldLoadArenaChunk(uint index) const {
	if (index < kScene1020ArenaFirstChunk || index > kScene1020ArenaLastChunk)
		return false;
	if (index <= 13)
		return true;
	if (isFirstEntryState())
		return index >= 14 && index <= 19;
	return index >= 20 && index <= 22;
}

bool Scene1020::hasCustomPreviewState() const {
	return true;
}

void Scene1020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();

	if (isFirstEntryState()) {
		_activeActorWorldX = kScene1020RightEntryTargetX;
		_activeActorWorldY = kScene1020RightEntryTargetY;
		_activeActorFacing = kScene1020RightEntryFacing;
	} else if (isSpecialOverlayEntryState()) {
		_activeActorWorldX = kScene1020OverlayEntryX;
		_activeActorWorldY = kScene1020OverlayEntryY;
		_activeActorFacing = kScene1020OverlayEntryFacing;
	} else {
		_activeActorWorldX = kScene1020DefaultActorX;
		_activeActorWorldY = kScene1020DefaultActorY;
		_activeActorFacing = kScene1020DefaultActorFacing;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene1020::hasCustomEntrySequence() const {
	return isFirstEntryState() || isSpecialOverlayEntryState();
}

void Scene1020::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (isFirstEntryState()) {
		runEntryPath(kScene1020RightEntryStartX, kScene1020RightEntryStartY,
			kScene1020RightEntryFacing, kScene1020RightEntryTargetX, kScene1020RightEntryTargetY);
		if (!state.seenScene1020EntryLine) {
			beginSecondarySpeechLine(0, 0);
			state.seenScene1020EntryLine = true;
		}
		return;
	}

	_activeActorWorldX = kScene1020OverlayEntryX;
	_activeActorWorldY = kScene1020OverlayEntryY;
	_activeActorFacing = kScene1020OverlayEntryFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

bool Scene1020::prepareCustomGameplayLoop() {
	return true;
}

bool Scene1020::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1020::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Return to scene 1010/right entry.
		_vm->gameState().mainFlowStateId = kScene1020ExitState1010RightEntry;
		return true;
	case 302: // Scene object row 1 speech.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Scene object row 2 speech.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // First event flag speech/update.
		handleSceneEventFlag0();
		return true;
	case 305: // Alternate resource-block pickup/event.
		handleSceneEventFlag0Overlay();
		return true;
	case 306: // Scene object row 5 speech.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Scene object row 6 speech.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Scene object row 7 speech.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Resource-block choice dependent speech.
		handleResourceBlockChoiceSpeech();
		return true;
	case 310: // Scene object row 9 speech.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Scene object row 10 speech.
		beginSecondarySpeechLine(0x0a, 0);
		return true;
	case 312: // Original callback is an empty shared-table handler.
		return true;
	case 313: // Object 7/8 resource-block animation branch.
		handleSceneVerb7Or8DescriptorAction();
		return true;
	case 314: // Scene object row 13 speech.
		beginSecondarySpeechLine(0x0d, 0);
		return true;
	case 315: // Scene object row 14 speech.
		beginSecondarySpeechLine(0x0e, 0);
		return true;
	case 316: // Alternate resource-block dependent speech.
		beginSecondarySpeechLine(0x0f, _vm->gameState().scene1020AlternateResourceBlockActive ? 1 : 0);
		return true;
	case 317: // Scene object row 16 speech.
		beginSecondarySpeechLine(0x10, 0);
		return true;
	case 318: // Scene object row 17 speech.
		beginSecondarySpeechLine(0x11, 0);
		return true;
	case 319: // Scene object row 18 speech.
		beginSecondarySpeechLine(0x12, 0);
		return true;
	case 320: // Row 19 after forcing event flag 1.
		handleSpeech19AfterEventFlag1();
		return true;
	case 321: // Event flag 1 speech/update.
		handleSceneEventFlag1Speech();
		return true;
	case 322: // Resource overlay chunk 18 state-change script.
		handleResourceOverlayChunk18StateChange();
		return true;
	case 323: // Resource overlay chunk 19 event script.
		handleResourceOverlayChunk19EventFlag();
		return true;
	default:
		return false;
	}
}

bool Scene1020::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	restoreBaseFramebufferFromOriginal();

	applyResourceBlockBackground();
	applySceneColorMapRules(0xff);
	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	GameplayState &state = _vm->gameState();
	if (!state.scene1020AlternateResourceBlockActive) {
		ScenePoint interactionPoint;
		ScenePoint approachPoint;
		if (state.scene1020ResourceBlockChoiceState == 0) {
			interactionPoint.x = 0x11e;
			interactionPoint.y = 0x14d;
			approachPoint.x = 0x11d;
			approachPoint.y = 0x14c;
		} else {
			interactionPoint.x = 0x0b7;
			interactionPoint.y = 0x18a;
			approachPoint.x = 0x0b6;
			approachPoint.y = 0x189;
		}
		_hotspots.setActionTarget(5, interactionPoint, approachPoint);
	}

	if (state.scene1020EventFlag1) {
		_hotspots.setVerbMovementModeByGlobalRecordIndex(99, 0);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(100, 0);
	}

	return true;
}

bool Scene1020::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)nextRegion;
	(void)state;
	(void)boundary;
	(void)requestedFacing;
	(void)restoredStepDeltas;
	return false;
}

bool Scene1020::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;
	(void)requestedFacing;
	(void)restoredStepDeltas;
	return false;
}

void Scene1020::applyResourceBlockBackground() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020AlternateResourceBlockActive) {
		if (shouldLoadArenaChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		if (shouldLoadArenaChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
		return;
	}

	if (shouldLoadArenaChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);

	const byte choice = MIN<byte>(state.scene1020ResourceBlockChoiceState, 2);
	const byte variant = MIN<byte>(state.scene1020ResourceBlockVariantState, 1);
	const uint chunkIndex = 7 + choice + variant;
	if (chunkIndex <= kScene1020ArenaLastChunk && shouldLoadArenaChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _baseFramebuffer);
}

void Scene1020::applySceneColorMapRules(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	GameplayState &state = _vm->gameState();
	const byte choice = MIN<byte>(state.scene1020ResourceBlockChoiceState, 2);
	const byte variant = MIN<byte>(state.scene1020ResourceBlockVariantState, 1);
	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
		byte item = originalItem;

		if (state.scene1020AlternateResourceBlockActive) {
			if (originalItem == 5 || originalItem == 0x0d || originalItem == 0x10)
				item = 1;
			else if (originalItem == 0x0e || originalItem == 0x0f)
				item = 0;
		} else if (choice == 0) {
			if (originalItem == 5)
				item = 5;
			else if (originalItem == 0x0d || originalItem == 0x10)
				item = 1;
			else if (originalItem == 0x0e)
				item = 0;
			else if (originalItem == 0x0f)
				item = 2;
		} else if (choice == 1) {
			if (originalItem == 0x0d)
				item = 5;
			else if (originalItem == 5 || originalItem == 0x10)
				item = 1;
			else if (originalItem == 0x0e)
				item = 0;
			else if (originalItem == 0x0f)
				item = 2;
		} else {
			if (originalItem == 0x0e || originalItem == 0x0f || originalItem == 0x10)
				item = 5;
			else if (originalItem == 5 || originalItem == 0x0d)
				item = 1;
		}

		if (state.scene1020AlternateResourceBlockActive) {
			if (originalItem == 6 || originalItem == 0x0f)
				item = 0;
		} else if (variant == 0) {
			if (originalItem == 6)
				item = 2;
		} else if (originalItem == 6 || originalItem == 0x0f) {
			item = 6;
		}

		if (state.scene1020AlternateResourceBlockActive && (originalItem == 2 || originalItem == 9))
			item = 0;

		if (state.scene1020EventFlag0) {
			if (originalItem == 3)
				item = 3;
		} else if (state.scene1020AlternateResourceBlockActive) {
			if (originalItem == 3)
				item = 0;
		} else if (originalItem == 3) {
			item = 2;
		}

		_paletteMask[kSceneColorToItemMap + i] = item;
	}

	if (state.scene1020AlternateResourceBlockActive && shouldLoadArenaChunk(12))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
	if (state.scene1020EventFlag0 && state.scene1020AlternateResourceBlockActive && shouldLoadArenaChunk(13))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);
	if (state.scene1020EventFlag1)
		copyStageSmallRow(13, 12);
}

void Scene1020::copyStageSmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene1020::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame) {
	if (!shouldLoadArenaChunk(chunkIndex)) {
		debugC(2, kDebugScene, "Scene1020 skipped unloaded overlay chunk %u", chunkIndex);
		return;
	}

	runConfiguredActionOverlay(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis,
		kActionOverlayShowActiveActor, patchFrame, 0xff);
}

void Scene1020::runOverlaySequenceWithActor(uint overlayChunkIndex, uint overlayDescriptorCount,
		const byte *overlayFrameMap, uint overlayFrameMapSize, uint directChunkIndex,
		uint directDescriptorCount, const byte *directFrameMap, uint directFrameMapSize) {
	runOverlaySequence(overlayChunkIndex, overlayDescriptorCount, overlayFrameMap,
		overlayFrameMapSize, kScene1020OverlayFrameMillis);
	runOverlaySequence(directChunkIndex, directDescriptorCount, directFrameMap,
		directFrameMapSize, kScene1020OverlayFrameMillis);
}

void Scene1020::handleSceneEventFlag0() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1020EventFlag0) {
		state.scene1020EventFlag0 = true;
		state.scene1020EventFlag3 = true;
		applySceneStateToHotspotsAndPatches(4);
		beginSecondarySpeechLine(3, 0);
		return;
	}

	beginStaticSecondarySpeechLine(0x0e, 0);
}

void Scene1020::handleSceneEventFlag0Overlay() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1020AlternateResourceBlockActive) {
		beginSecondarySpeechLine(4, 0);
		return;
	}

	beginSecondarySpeechLine(4, 1);
	runOverlaySequence(22, kScene1020ActionChunk22DescriptorCount, kScene1020Chunk22PickupFrameMap,
		ARRAYSIZE(kScene1020Chunk22PickupFrameMap), kScene1020OverlayFrameMillis, 8);
	addInventoryItem(0x16);
	_soundBank0.playSample(1, 100);
	state.scene1020EventFlag0 = false;
	applySceneStateToHotspotsAndPatches(4);
}

void Scene1020::handleResourceBlockChoiceSpeech() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020ResourceBlockChoiceState < 2)
		beginSecondarySpeechLine(0x11, 0);
	else
		beginSecondarySpeechLine(8, 2);
}

void Scene1020::handleSceneVerb7Or8DescriptorAction() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020AlternateResourceBlockActive) {
		dispatchGenericSceneAction(20);
		return;
	}

	if (_lastSceneActionItemId == 8) {
		if (state.scene1020ResourceBlockChoiceState == 0) {
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ForwardFrameMap,
				ARRAYSIZE(kScene1020Chunk14ForwardFrameMap), kScene1020OverlayFrameMillis);
			runOverlaySequence(15, kScene1020ActionChunk15DescriptorCount, kScene1020Chunk15PingPongFrameMap,
				ARRAYSIZE(kScene1020Chunk15PingPongFrameMap), kScene1020OverlayFrameMillis);
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
				ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
			beginSecondarySpeechLine(8, 2);
		} else if (state.scene1020ResourceBlockChoiceState == 1) {
			runOverlaySequenceWithActor(14, kScene1020ActionChunk14DescriptorCount,
				kScene1020Chunk14ForwardFrameMap, ARRAYSIZE(kScene1020Chunk14ForwardFrameMap),
				17, kScene1020ActionChunk17DescriptorCount,
				kScene1020Chunk17ForwardFrameMap, ARRAYSIZE(kScene1020Chunk17ForwardFrameMap));
			state.scene1020ResourceBlockChoiceState = 2;
			applySceneStateToHotspotsAndPatches(1);
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
				ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
			beginStaticSecondarySpeechLine(0x35, 0);
		} else if (state.scene1020ResourceBlockVariantState == 0) {
			beginSecondarySpeechLine(8, 2);
		} else {
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ForwardFrameMap,
				ARRAYSIZE(kScene1020Chunk14ForwardFrameMap), kScene1020OverlayFrameMillis);
			runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
				ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
			beginSecondarySpeechLine(0x0c, 0);
		}
		return;
	}

	if (_lastSceneActionItemId != 7)
		return;

	if (state.scene1020ResourceBlockChoiceState != 0) {
		beginSecondarySpeechLine(8, 2);
		return;
	}

	runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ForwardFrameMap,
		ARRAYSIZE(kScene1020Chunk14ForwardFrameMap), kScene1020OverlayFrameMillis);
	if (!state.scene1020EventFlag2) {
		runOverlaySequence(16, kScene1020ActionChunk16DescriptorCount, kScene1020Chunk16AlternatingFrameMap,
			ARRAYSIZE(kScene1020Chunk16AlternatingFrameMap), kScene1020OverlayFrameMillis);
		runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
			ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
		beginSecondarySpeechLine(8, 0);
		return;
	}

	runOverlaySequence(16, kScene1020ActionChunk16DescriptorCount, kScene1020Chunk16ForwardFrameMap,
		ARRAYSIZE(kScene1020Chunk16ForwardFrameMap), kScene1020OverlayFrameMillis);
	state.scene1020ResourceBlockChoiceState = 1;
	applySceneStateToHotspotsAndPatches(1);
	runOverlaySequence(14, kScene1020ActionChunk14DescriptorCount, kScene1020Chunk14ReverseFrameMap,
		ARRAYSIZE(kScene1020Chunk14ReverseFrameMap), kScene1020OverlayFrameMillis);
	beginSecondarySpeechLine(8, 1);
}

void Scene1020::handleSceneEventFlag1Speech() {
	GameplayState &state = _vm->gameState();
	if (!state.scene1020EventFlag1) {
		state.scene1020EventFlag1 = true;
		applySceneStateToHotspotsAndPatches(5);
		beginSecondarySpeechLine(0x14, 0);
		return;
	}

	beginSecondarySpeechLine(0x14, 1);
}

void Scene1020::handleSpeech19AfterEventFlag1() {
	if (!_vm->gameState().scene1020EventFlag1)
		handleSceneEventFlag1Speech();
	beginSecondarySpeechLine(0x13, 0);
}

void Scene1020::handleResourceOverlayChunk18StateChange() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020ResourceBlockChoiceState < 2) {
		beginSecondarySpeechLine(0x15, 0);
		return;
	}

	runOverlaySequence(18, kScene1020ActionChunk18DescriptorCount, kScene1020Chunk18StateChangeFrameMap,
		ARRAYSIZE(kScene1020Chunk18StateChangeFrameMap), kScene1020OverlayFrameMillis, 0x22);
	removeInventoryItem(0x1e);
	_soundBank0.playSample(1, 100);
	state.scene1020ResourceBlockVariantState = 1;
	applySceneStateToHotspotsAndPatches(2);
	beginSecondarySpeechLine(0x15, 2);
}

void Scene1020::handleResourceOverlayChunk19EventFlag() {
	GameplayState &state = _vm->gameState();
	if (state.scene1020EventFlag2) {
		beginSecondarySpeechLine(0x16, 1);
		return;
	}

	beginSecondarySpeechLine(0x16, 0);
	runOverlaySequence(19, kScene1020ActionChunk19DescriptorCount, kScene1020Chunk19EventFrameMap,
		ARRAYSIZE(kScene1020Chunk19EventFrameMap), kScene1020OverlayFrameMillis);
	state.scene1020EventFlag2 = true;
}

AmbientAudioProfile Scene1020::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(kScene1020FirstAmbientSoundCue,
		kScene1020AmbientSoundCueCount, 15, kScene1020AmbientSoundProbabilityModulus,
		kScene1020FirstAmbientMusicCue, kScene1020AmbientMusicCueCount, 100,
		kScene1020AmbientMusicProbabilityModulus);
}

bool Scene1020::isFirstEntryState() const {
	return _vm->gameState().mainFlowStateId == kScene1020RightEntryState;
}

bool Scene1020::isSpecialOverlayEntryState() const {
	return _vm->gameState().mainFlowStateId == kScene1020OverlayEntryState;
}

} // End of namespace Hollywood
