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

#include "hollywood/scenes/playable/scene3080.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene3080ArchiveName = "RESOURCE.C08";
const char *const kScene3080MusicArchiveName = "RESOURCE.M03";
const char *const kScene3080SoundArchiveName = "RESOURCE.S03";
const uint kScene3080InitialRequiredChunkCount = 18;
const uint kScene3080ArenaFirstChunk = 5;
const uint kScene3080ArenaLastChunk = 17;
const uint kScene3080StageIndex = 308;
const uint16 kScene3080FirstState = 0x0c08;
const uint16 kScene3080LastState = 0x0c11;
const uint16 kScene3080EntryFromForestState = 0x0c08;
const uint16 kScene3080EntryFromCabinState = 0x0c09;
const uint16 kScene3080EntryFromBrookState = 0x0c0a;
const uint16 kScene3010EntryFromScene3080State = 0x0bc3;
const uint16 kScene3090State = 0x0c12;
const uint16 kScene3100State = 0x0c1c;
const uint16 kScene3080ViewportXOffset = 0x0078;
const uint kScene3080ActorBankTableEntry = 0x0038;
const uint kScene3080ActorPaletteTableEntry = 0x00cc;
const uint kScene3080Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3080SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3080LargeFrameMillis = 125;
const uint32 kScene3080SmallIdleFrameMillis = 100;
const uint32 kScene3080OverlayFrameMillis = 75;
const uint kScene3080LargeLayerDescriptorCount = 0x10;
const uint kScene3080SmallIdleDescriptorCount = 0x16;
const uint kScene3080DiaryOverlayDescriptorCount = 0x0e;
const uint kScene3080StickOverlayDescriptorCount = 0x0d;
const uint kScene3080BranchExchangeDescriptorCount = 9;

const byte kScene3080ActorPathStepDeltaTable[] = {
	6, 1, 1, 3, 3, 3, 7, 1, 0, 0, 4, 3,
	3, 2, 8, 6, 6, 7, 6, 4, 10, 3, 2, 9,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	4, 3, 3, 4, 0, 4, 4, 2, 0, 4, 2, 5,
	6, 10, 10, 4, 6, 4, 10, 8, 8, 7, 5, 10,
	6, 4, 10, 3, 2, 9, 3, 2, 8, 6, 6, 7
};

const byte kScene3080LargeLayerFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15
};

const byte kScene3080SmallIdleFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
};

const byte kScene3080DiaryOverlayFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

const byte kScene3080StickOverlayFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene3080BranchExchangeFrameMap[] = {
	8, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
	1, 0, 0, 1, 1, 0, 0, 1, 2, 3,
	4, 5, 6, 7, 8
};

static PlayableSceneConfig scene3080Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene3080ArchiveName;
	config.initialRequiredChunkCount = kScene3080InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene3080ArenaFirstChunk;
	config.arenaLastChunk = kScene3080ArenaLastChunk;
	config.stageIndex = kScene3080StageIndex;
	config.debugName = "Scene 3080";
	config.viewportXOffset = kScene3080ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 3;
	config.actorBankTableEntry = kScene3080ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene3080ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene3080Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene3080SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene3080ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene3080ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene3080MusicArchiveName;
	config.soundBank0ArchiveName = kScene3080SoundArchiveName;
	config.mainFlowFirstState = kScene3080FirstState;
	config.mainFlowLastState = kScene3080LastState;
	return config;
}

Scene3080::Scene3080(HollywoodEngine *vm) :
		PlayableScene(vm, scene3080Config(), "scene3080", 0x150, 0x1bf, 1, 0xfd, 0xfb),
		_largeChannel(),
		_smallIdleChannel(),
		_largeLayer(),
		_smallIdleLayer(),
		_smallIdleMode(0) {
	_largeLayer.configure(17, kScene3080LargeLayerDescriptorCount,
		kScene3080LargeLayerFrameMap, ARRAYSIZE(kScene3080LargeLayerFrameMap));
	_smallIdleLayer.configure(7, kScene3080SmallIdleDescriptorCount,
		kScene3080SmallIdleFrameMap, ARRAYSIZE(kScene3080SmallIdleFrameMap));
}

bool Scene3080::hasCustomPreviewState() const {
	return true;
}

void Scene3080::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene3080EntryFromCabinState) {
		_activeActorWorldX = 0x12c;
		_activeActorWorldY = 0x1c2;
		_activeActorFacing = 4;
	} else if (stateId == kScene3080EntryFromBrookState) {
		_activeActorWorldX = 0x1c0;
		_activeActorWorldY = 0x17e;
		_activeActorFacing = 4;
	} else if (stateId == kScene3080EntryFromForestState) {
		_activeActorWorldX = 0x150;
		_activeActorWorldY = 0x1bf;
		_activeActorFacing = 1;
	} else {
		_activeActorWorldX = 0x150;
		_activeActorWorldY = 0x1bf;
		_activeActorFacing = 1;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene3080::hasCustomComposite() const {
	return true;
}

void Scene3080::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_largeLayer);
	drawActionOverlayLayer();
	if (activeWorldY <= 0x165)
		drawForegroundBlocks(activeWorldY);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (activeWorldY <= 0x165) {
		const uint chunkIndex = _vm->gameState().scene3080WindowPatchActive ? 16 : 6;
		if (_sceneChunkTable.isValidChunk(chunkIndex))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
	}
	drawResourceSpriteLayer(_smallIdleLayer);
}

bool Scene3080::hasCustomEntrySequence() const {
	return true;
}

void Scene3080::runCustomEntrySequence() {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene3080EntryFromCabinState)
		runEntryFromCabin();
	else if (stateId == kScene3080EntryFromBrookState)
		runEntryFromBrook();
	else
		runEntryFromForest();
}

bool Scene3080::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene3080::advanceCustomGameplayLoop(uint32 delta) {
	advanceLargeLayer(delta);
	advanceSmallIdleLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3080::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Ir a casa de Frankenstein (go to Frankenstein house): return to scene 3010.
		state.mainFlowStateId = kScene3010EntryFromScene3080State;
		return true;
	case 302: // Ir a puerta de la cabaña (go to cabin door): enter scene 3100.
		state.scene3080DoorSeen = true;
		state.mainFlowStateId = kScene3100State;
		return true;
	case 303: // Ir a camino del riachuelo (go to brook path): enter scene 3090.
		state.mainFlowStateId = kScene3090State;
		return true;
	case 304: // Mirar camino a casa de Frankenstein (look at Frankenstein-house path).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 305: // Mirar árbol (look at tree).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 306: // Mirar camino/puerta (look at path/door), changes after door visit.
		beginSecondarySpeechLine(3, state.scene3080DoorSeen ? 1 : 0);
		return true;
	case 307: // Mirar camino del riachuelo (look at brook path).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 308: // Coger diario secreto del Dr. Frankenstein (take Frankenstein diary).
		runDiaryPickup();
		return true;
	case 309: // Mirar diario secreto del Dr. Frankenstein (look at Frankenstein diary).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 310: // Mirar chimenea (look at chimney).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 311: // Mirar ventana (look at window).
		beginSecondarySpeechLine(8, state.scene3080WindowPatchActive ? 0 : 1);
		return true;
	case 312: // Abrir ventana (open window).
		if (!state.scene3080WindowPatchActive)
			beginSecondarySpeechLine(9, 0);
		else
			dispatchGenericSceneAction(9);
		return true;
	case 313: // Cerrar ventana (close window).
		if (!state.scene3080WindowPatchActive)
			dispatchGenericSceneAction(12);
		else
			dispatchGenericSceneAction(20);
		return true;
	case 314: // Coger palo/rama (take stick/branch).
		runStickPickup();
		return true;
	case 315: // Mirar palo/rama (look at stick/branch).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 316: // Usar rama en ventana/chimenea (use branch exchange overlay).
		runBranchExchangeOverlay();
		return true;
	default:
		return false;
	}
}

bool Scene3080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 2 || selector == 3 || selector == 4) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

		removeColorMapItem(6);
		restoreOrRemoveDiaryHotspot();
		if (_vm->gameState().scene3080WindowPatchActive && _sceneChunkTable.isValidChunk(15))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[15], _baseFramebuffer);
		if (_vm->gameState().scene3080StickTaken) {
			if (_sceneChunkTable.isValidChunk(8))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
			removeColorMapItem(8);
		}

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

AmbientAudioProfile Scene3080::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x10;
	profile.musicCueCount = 2;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3080::resetAnimationLayers() {
	_largeChannel.reset(0, kScene3080LargeFrameMillis);
	_smallIdleChannel.reset(0, kScene3080SmallIdleFrameMillis);
	_largeLayer.visible = true;
	_smallIdleLayer.visible = true;
	_largeLayer.reset(0);
	_smallIdleLayer.reset(0);
	_smallIdleMode = 0;
}

void Scene3080::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 6 || _walkablePaletteMask[i] == 8 ||
				_walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}
}

void Scene3080::removeColorMapItem(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == itemId)
			_paletteMask[kSceneColorToItemMap + i] = 0;
	}
}

void Scene3080::restoreOrRemoveDiaryHotspot() {
	const GameplayState &state = _vm->gameState();
	if (state.scene3080FrankensteinDiaryRevealed && !state.scene3080DiaryTaken) {
		if (_sceneChunkTable.isValidChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
	} else {
		if (_sceneChunkTable.isValidChunk(12))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
		removeColorMapItem(5);
	}
}

void Scene3080::advanceLargeLayer(uint32 delta) {
	const uint frameCount = _largeChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte firstFrame = _vm->gameState().scene3080SmokeAlternateFrames ? 8 : 0;
		const byte lastFrame = _vm->gameState().scene3080SmokeAlternateFrames ? 15 : 7;
		if (_largeChannel.frameIndex < firstFrame || _largeChannel.frameIndex >= lastFrame)
			_largeChannel.frameIndex = firstFrame;
		else
			++_largeChannel.frameIndex;
		_largeLayer.setFrame(_largeChannel.frameIndex);
	}
}

void Scene3080::advanceSmallIdleLayer(uint32 delta) {
	const uint frameCount = _smallIdleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_smallIdleMode == 0) {
			if (_smallIdleChannel.frameIndex < 7) {
				++_smallIdleChannel.frameIndex;
			} else if (_random.getRandomNumber(14) == 0) {
				_smallIdleMode = 1;
				_smallIdleChannel.frameIndex = 8;
			} else if (_random.getRandomNumber(29) == 0) {
				_smallIdleMode = 2;
				_smallIdleChannel.frameIndex = 12;
			} else {
				_smallIdleChannel.frameIndex = 0;
			}
		} else if (_smallIdleMode == 1) {
			if (_smallIdleChannel.frameIndex < 11) {
				++_smallIdleChannel.frameIndex;
			} else {
				_smallIdleMode = 0;
				_smallIdleChannel.frameIndex = 0;
			}
		} else if (_smallIdleMode == 2) {
			if (_smallIdleChannel.frameIndex < 21) {
				++_smallIdleChannel.frameIndex;
			} else {
				_smallIdleMode = 0;
				_smallIdleChannel.frameIndex = 0;
			}
		}
		_smallIdleLayer.setFrame(_smallIdleChannel.frameIndex);
	}
}

void Scene3080::runEntryFromForest() {
	runEntryPath(0x0b4, 0x1df, 1, 0x150, 0x1bf);
	if (!_vm->gameState().seenScene3080EntryLine) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().seenScene3080EntryLine = true;
	}
}

void Scene3080::runEntryFromCabin() {
	runEntryPath(0x13d, 0x164, 4, 0x12c, 0x1c2);
}

void Scene3080::runEntryFromBrook() {
	runEntryPath(0x1db, 0x137, 4, 0x1c0, 0x17e);
}

void Scene3080::runDiaryPickup() {
	GameplayState &state = _vm->gameState();
	if (!state.scene3080FrankensteinDiaryRevealed || state.scene3080DiaryTaken) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	state.scene3080DiaryTaken = true;
	state.scene3080FrankensteinDiaryRevealed = false;
	runConfiguredActionOverlay(10, kScene3080DiaryOverlayDescriptorCount,
		kScene3080DiaryOverlayFrameMap, ARRAYSIZE(kScene3080DiaryOverlayFrameMap),
		kScene3080OverlayFrameMillis, kActionOverlayHideActiveActor, 7, 2, 7, 1);
	addInventoryItem(0x33);
	applySceneStateToHotspotsAndPatches(2);
	beginSecondarySpeechLine(5, 0);
}

void Scene3080::runStickPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene3080StickTaken) {
		beginSecondarySpeechLine(10, 0);
		return;
	}

	state.scene3080StickTaken = true;
	runConfiguredActionOverlay(9, kScene3080StickOverlayDescriptorCount,
		kScene3080StickOverlayFrameMap, ARRAYSIZE(kScene3080StickOverlayFrameMap),
		kScene3080OverlayFrameMillis, kActionOverlayHideActiveActor, 7, 4, 7, 1);
	addInventoryItem(0x35);
	applySceneStateToHotspotsAndPatches(4);
	dispatchGenericSceneAction(21);
}

void Scene3080::runBranchExchangeOverlay() {
	runConfiguredActionOverlay(14, kScene3080BranchExchangeDescriptorCount,
		kScene3080BranchExchangeFrameMap, ARRAYSIZE(kScene3080BranchExchangeFrameMap),
		kScene3080OverlayFrameMillis, kActionOverlayHideActiveActor, -1, 0, 8, 0x1b);
	removeInventoryItem(0x58);
	addInventoryItem(0x34);
}

void Scene3080::drawForegroundBlocks(int activeWorldY) {
	(void)activeWorldY;
	const uint chunkIndex = _vm->gameState().scene3080StickTaken ? 5 : 13;
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

} // End of namespace Hollywood
