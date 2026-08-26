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
const byte kScene3080MusicVolumePercent = 50;
const uint kScene3080LargeLayerDescriptorCount = 0x10;
const uint kScene3080SmallIdleDescriptorCount = 0x16;
const uint kScene3080DiaryOverlayDescriptorCount = 0x0e;
const uint kScene3080StickOverlayDescriptorCount = 0x0d;
const uint kScene3080FlyerOverlayDescriptorCount = 9;
const byte kScene3080DiaryPatchHook = 1;
const byte kScene3080StickPatchHook = 2;
const byte kScene3080FlyerSoundHook = 3;

const byte kScene3080LargeLayerFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15
};

const byte kScene3080SmallIdleFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
};

const byte kScene3080DiaryOverlayFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

const byte kScene3080StickOverlayFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene3080FlyerOverlayFrameMap[] = {
	8, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
	1, 0, 0, 1, 1, 0, 0, 1, 2, 3,
	4, 5, 6, 7, 8
};

static PlayableSceneConfig scene3080Config() {
	PlayableSceneConfig config(3080,
		SceneResourceLayout(18, 5, 17),
		SceneViewport(kScene3080ViewportXOffset),
		SceneActorPose(0x150, 0x1bf, 1));
	config.setActorResources(kScene3080ActorBankTableEntry, kScene3080ActorPaletteTableEntry);
	config.setTextResources(kScene3080Resource003RowsOffsetIndex, kScene3080SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene3080::Scene3080(HollywoodEngine *vm) :
		PlayableScene(vm, scene3080Config()),
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

void Scene3080::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_largeLayer);
	if (_actionOverlayPlayer.replacesActor()) {
		drawActionOverlayLayer();
		drawResourceSpriteLayer(_smallIdleLayer);
		return;
	}

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (activeWorldY <= 0x165) {
		drawForegroundBlocks();
		const uint chunkIndex = _vm->gameState().scene3080WindowOpened ? 16 : 6;
		if (_sceneChunkTable.isValidChunk(chunkIndex))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
	}
	drawResourceSpriteLayer(_smallIdleLayer);
}

void Scene3080::runCustomEntrySequence() {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene3080EntryFromCabinState)
		runEntryFromCabin();
	else if (stateId == kScene3080EntryFromBrookState)
		runEntryFromBrook();
	else if (stateId == kScene3080EntryFromForestState)
		runEntryFromForest();
}

bool Scene3080::prepareCustomGameplayLoop() {
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
		state.scene3080CabinDoorVisited = true;
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
		beginSecondarySpeechLine(3, state.scene3080CabinDoorVisited ? 1 : 0);
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
		beginSecondarySpeechLine(8, state.scene3080WindowOpened ? 0 : 1);
		return true;
	case 312: // Abrir ventana (open window).
		if (!state.scene3080WindowOpened)
			beginSecondarySpeechLine(9, 0);
		else
			dispatchGenericSceneAction(9);
		return true;
	case 313: // Cerrar ventana (close window).
		if (!state.scene3080WindowOpened)
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
	case 316: // Usar folleto en árbol (use flyer on tree): make the flyer sticky.
		runFlyerCoatingOverlay();
		return true;
	default:
		return false;
	}
}

bool Scene3080::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x08a, 0x220);
	if (targetY < 0x1df)
		++targetY;

	while (targetY <= 0x1df) {
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
		++targetY;
	}

	targetY = 0x1df;
	while (targetY > 0) {
		--targetY;
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
	}
	return true;
}

bool Scene3080::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 6 && nextRegion == 7) {
		for (uint offset = 0x0c; offset <= 0x17; ++offset)
			_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 6 && nextRegion == 5) {
		for (uint offset = 0x30; offset <= 0x3b; ++offset)
			_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}
	if ((currentRegion == 8 && nextRegion == 7) ||
			(currentRegion == 7 && nextRegion == 6)) {
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}
	return false;
}

bool Scene3080::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)restoredStepDeltas;

	const byte previousRegion = state.drawOrderMode;
	if (currentRegion == 8 ||
			(currentRegion == 4 && ((targetX == 0x13d && targetY == 0x164) || previousRegion == 3))) {
		requestedFacing = 1;
		return true;
	}
	return false;
}

bool Scene3080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 2 || selector == 3 || selector == 4) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

		removeColorMapItem(6);
		restoreOrRemoveDiaryHotspot();
		if (_vm->gameState().scene3080WindowOpened && _sceneChunkTable.isValidChunk(15))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[15], _baseFramebuffer);
		if (_vm->gameState().scene3080BranchTaken) {
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
	profile.musicFirstCueId = _vm->gameState().scene3090BlindManPlayingSaxophone ? 0x11 : 0x10;
	profile.musicCueCount = 1;
	profile.musicVolumePercent = kScene3080MusicVolumePercent;
	profile.musicProbabilityModulus = 1;
	return profile;
}

void Scene3080::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene3080DiaryPatchHook) {
		if (_sceneChunkTable.isValidChunk(12))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
		return;
	}
	if (hookId == kScene3080StickPatchHook) {
		if (_sceneChunkTable.isValidChunk(8))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		return;
	}
	if (hookId != kScene3080FlyerSoundHook)
		return;

	if (frame == 8)
		_soundBank0.playSampleLooping(0x1b, 75);
	else if (frame == 28)
		_soundBank0.stop();
}

bool Scene3080::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void Scene3080::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
	if (_vm->gameState().mainFlowStateId != kScene3090State)
		_vm->gameplayMusic()->stop();
}

void Scene3080::resetAnimationLayers() {
	const byte largeFrame = _vm->gameState().scene3080ChimneySmokeAnimationChanged ? 8 : 0;
	_largeChannel.reset(largeFrame, kScene3080LargeFrameMillis);
	_smallIdleChannel.reset(0, kScene3080SmallIdleFrameMillis);
	_largeLayer.visible = true;
	_smallIdleLayer.visible = true;
	_largeLayer.reset(largeFrame);
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
	if (state.scene3080FrankensteinDiaryRevealed && !state.scene3080FrankensteinDiaryTaken) {
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
		const byte firstFrame = _vm->gameState().scene3080ChimneySmokeAnimationChanged ? 8 : 0;
		const byte lastFrame = _vm->gameState().scene3080ChimneySmokeAnimationChanged ? 15 : 7;
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
			++_smallIdleChannel.frameIndex;
			if (_smallIdleChannel.frameIndex == 7)
				_smallIdleMode = 1;
		} else if (_smallIdleMode == 1) {
			if (_smallIdleChannel.frameIndex < 11) {
				++_smallIdleChannel.frameIndex;
			} else if (_random.getRandomNumber(19) == 0) {
				_smallIdleMode = 2;
				_smallIdleChannel.frameIndex = 12;
			} else {
				_smallIdleChannel.frameIndex = 8;
			}
		} else if (_smallIdleMode == 2) {
			++_smallIdleChannel.frameIndex;
			if (_smallIdleChannel.frameIndex == 22)
				_smallIdleMode = 3;
		} else if (_random.getRandomNumber(99) == 0) {
			_smallIdleMode = 0;
			_smallIdleChannel.frameIndex = 0;
		}
		_smallIdleLayer.setFrame(_smallIdleChannel.frameIndex);
	}
}

void Scene3080::runEntryFromForest() {
	startEntryMusic();
	runEntryPath(0x0b4, 0x1df, 1, 0x150, 0x1bf);
	GameplayState &state = _vm->gameState();
	if (!state.scene3080EntryLineSeen) {
		state.scene3080EntryLineSeen = true;
		beginSecondarySpeechLine(0, 0);
	}
}

void Scene3080::runEntryFromCabin() {
	startEntryMusic();
	runEntryPath(0x13d, 0x164, 4, 0x12c, 0x1c2);
}

void Scene3080::runEntryFromBrook() {
	_vm->gameplayMusic()->setVolume(kScene3080MusicVolumePercent);
	runEntryPath(0x1db, 0x137, 4, 0x1c0, 0x17e);
}

void Scene3080::startEntryMusic() {
	GameplayState &state = _vm->gameState();
	state.currentAmbientMusicCueId = state.scene3090BlindManPlayingSaxophone ? 0x11 : 0x10;
	_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, kScene3080MusicVolumePercent);
}

void Scene3080::runDiaryPickup() {
	GameplayState &state = _vm->gameState();
	if (!state.scene3080FrankensteinDiaryRevealed || state.scene3080FrankensteinDiaryTaken) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(10, kScene3080DiaryOverlayDescriptorCount,
		kScene3080DiaryOverlayFrameMap, ARRAYSIZE(kScene3080DiaryOverlayFrameMap), kScene3080OverlayFrameMillis)
		.hookAt(10, kScene3080DiaryPatchHook)
		.noRedrawAtEnd());
	state.scene3080FrankensteinDiaryTaken = true;
	state.scene3080FrankensteinDiaryRevealed = false;
	applySceneStateToHotspotsAndPatches(2);
	addInventoryItem(0x33);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(5, 0);
}

void Scene3080::runStickPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene3080BranchTaken) {
		beginSecondarySpeechLine(10, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(9, kScene3080StickOverlayDescriptorCount,
		kScene3080StickOverlayFrameMap, ARRAYSIZE(kScene3080StickOverlayFrameMap), kScene3080OverlayFrameMillis)
		.hookAt(10, kScene3080StickPatchHook)
		.noRedrawAtEnd());
	state.scene3080BranchTaken = true;
	applySceneStateToHotspotsAndPatches(4);
	addInventoryItem(0x35);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene3080::runFlyerCoatingOverlay() {
	runActorReplacement(ActionOverlaySpec(14, kScene3080FlyerOverlayDescriptorCount,
		kScene3080FlyerOverlayFrameMap, ARRAYSIZE(kScene3080FlyerOverlayFrameMap), kScene3080OverlayFrameMillis)
		.hookEveryFrame(kScene3080FlyerSoundHook));
	_soundBank0.stop();
	addInventoryItem(0x34);
	removeInventoryItem(0x58);
	_soundBank0.playSample(1, 100);
}

void Scene3080::drawForegroundBlocks() {
	const uint chunkIndex = _vm->gameState().scene3080BranchTaken ? 5 : 13;
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

} // End of namespace Hollywood
