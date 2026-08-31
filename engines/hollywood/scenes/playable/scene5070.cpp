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

#include "hollywood/scenes/playable/scene5070.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5070ViewportXOffset = 0x00a0;
const uint16 kScene5070ViewportMaxXOffset = 0x00c8;
const uint kScene5070ActorBankTableEntry = 0x0000;
const uint kScene5070ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5070FrameMillis = 75;
const uint32 kScene5070MineCartTimerNumerator = 125;
const uint kScene5070MineCartDescriptorCount = 0x30;
const uint kScene5070AviatorCapPickupDescriptorCount = 0x0c;
const uint kScene5070ShovelPickupDescriptorCount = 0x0d;
const byte kScene5070ShovelSceneItem = 2;
const byte kScene5070HangingSceneItem = 6;
const byte kScene5070AviatorCapUnidentifiedState = 1;
const byte kScene5070AviatorCapIdentifiedState = 2;
const byte kScene5070AviatorCapTakenState = 0;
const byte kScene5070AviatorCapInventoryItem = 0x4f;
const byte kScene5070ShovelInventoryItem = 0x50;
const byte kScene5070AviatorCapNameRow = 7;
const byte kScene5070HangingItemNameRow = 6;
const uint kScene5070HangingItemVerbRecordIndex = 0x34;
const int kScene5070EntryStartX = 0x3ab;
const int kScene5070EntryStartY = 0x1df;
const int kScene5070EntryTargetX = 0x2e3;
const int kScene5070EntryTargetY = 0x1d6;
const int kScene5070CentralGapLeftX = 0x0e6;
const int kScene5070CentralGapSplitX = 0x137;
const int kScene5070CentralGapRightX = 0x188;
const int kScene5070MaximumWalkY = 0x1df;

enum Scene5070AnimationHookId {
	kScene5070ShovelBackgroundPatchHook = 1
};

const byte kScene5070MineCartDelayBuckets[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2,
	3, 3, 3, 3,
	4, 4, 4,
	5, 5, 5,
	6, 6, 6, 6,
	7, 7, 7,
	8, 8, 8,
	9, 9, 9, 9,
	10, 10, 10,
	11, 11, 11, 11,
	12, 12, 12, 12
};

const byte kScene5070AmbientSoundVolumes[] = {
	10, 10, 10, 2, 10, 10, 10, 100
};

const byte kScene5070AviatorCapPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene5070ShovelPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

PlayableSceneConfig scene5070Config() {
	PlayableSceneConfig config(5070,
		SceneResourceLayout(5, 5, 11),
		SceneViewport(kScene5070ViewportXOffset, kScene5070ViewportXOffset, kScene5070ViewportMaxXOffset),
		SceneActorPose(0x2e3, 0x1d6, 5));
	config.setActorResources(kScene5070ActorBankTableEntry, kScene5070ActorPaletteTableEntry);
	config.setTextResources(0, kScene5070SpeechCueDescriptorTableOffset);
	return config;
}

const uint kScene5070MineCartLayer = 0;
const SceneLayerSpec kScene5070LayerSpecs[] = {
	{kSceneAnimationInFrontOfActors, 9, kScene5070MineCartDescriptorCount,
		nullptr, 0, false, 0}
};

Scene5070::Scene5070(HollywoodEngine *vm) :
		PlayableScene(vm, scene5070Config()),
		_mineCartRumbleActive(false) {
	_sceneLayers.configure(kScene5070LayerSpecs);
}

void Scene5070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);
	_mineCartRumbleActive = false;

	setActiveActorPose(kScene5070EntryTargetX, kScene5070EntryTargetY, 5);
}

void Scene5070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();
	drawLayerStack(_sceneLayers, kSceneAnimationInFrontOfActors);
	if (_sceneLayers.layerVisible(kScene5070MineCartLayer))
		drawMineCartForeground();
}

bool Scene5070::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene5070::runCustomEntrySequence() {
	setActiveActorPose(kScene5070EntryStartX, kScene5070EntryStartY, 4);
	runMineCartEntryClip();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	walkActiveActorTo(kScene5070EntryTargetX, kScene5070EntryTargetY, 5, 0, false);
}

void Scene5070::advanceCustomGameplayLoop(uint32 delta) {
	(void)delta;
	ensureAmbientSoundCuePlaying(1, 0x0c, 10);
	if (_mineCartRumbleActive && !_soundBank0.isPlaying())
		_soundBank0.playSample(0x18, 100);
}

bool Scene5070::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();

	switch (handlerId) {
	case 301: // Ir a vagoneta/salida (go to cart/exit): return to mine switches.
		runExitToMineSwitches();
		return true;
	case 302: // Coger pala (take shovel): grants inventory item 0x50.
		runShovelPickup();
		return true;
	case 303: // Mirar pala/túnel cercano (look at shovel/near tunnel).
		beginSecondarySpeechLine(0, 0);
		return true;
	case 304: // Ir a/abrir túnel o caseta (go/open tunnel or shed): Ron refuses to leave the cart.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 305: // Mirar algo colgado/caseta (look at hanging object/shed).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 306: // Coger gorro de aviador (take aviator cap): grants inventory item 0x4f.
		runAviatorCapPickup();
		return true;
	case 307: // Mirar objeto colgado (look at hanging object): identifies it as an aviator cap.
		if (state.scene5070AviatorCapState == kScene5070AviatorCapUnidentifiedState) {
			beginSecondarySpeechLine(3, 0);
			state.scene5070AviatorCapState = kScene5070AviatorCapIdentifiedState;
			applySceneStateToHotspotsAndPatches(1);
		}
		return true;
	default:
		return false;
	}
}

bool Scene5070::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetX > kScene5070CentralGapLeftX && targetX < kScene5070CentralGapRightX)
		targetX = targetX < kScene5070CentralGapSplitX ?
			kScene5070CentralGapLeftX : kScene5070CentralGapRightX;

	if (targetY < kScene5070MaximumWalkY)
		++targetY;
	while (targetY < kScene5070MaximumWalkY && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	if (targetY == kScene5070MaximumWalkY) {
		while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
			--targetY;
	}
	return true;
}

bool Scene5070::customizeRouteSegment(byte currentRegion, byte nextRegion,
		const ActorPathBuildState &state, const ScenePoint &boundary,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 2 && nextRegion == 1) {
		requestedFacing = 5;
		return true;
	}
	if ((currentRegion == 4 && nextRegion == 5) ||
			(currentRegion == 5 && nextRegion == 6) ||
			(currentRegion == 6 && nextRegion == 7) ||
			(currentRegion == 7 && nextRegion == 8)) {
		requestedFacing = 4;
		copySlopeStepDeltasFromSet5A(0x30);
		restoredStepDeltas = true;
		return true;
	}
	if ((currentRegion == 8 && nextRegion == 7) ||
			(currentRegion == 7 && nextRegion == 6) ||
			(currentRegion == 6 && nextRegion == 5) ||
			(currentRegion == 5 && nextRegion == 4)) {
		requestedFacing = 2;
		copySlopeStepDeltasFromSet5A(0x18);
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene5070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene5070AviatorCapState > kScene5070AviatorCapIdentifiedState)
		state.scene5070AviatorCapState = kScene5070AviatorCapUnidentifiedState;

	if (state.scene5070ShovelTaken || hasInventoryItem(kScene5070ShovelInventoryItem)) {
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5070ShovelSceneItem);
	}
	if (state.scene5070AviatorCapState == kScene5070AviatorCapTakenState || hasInventoryItem(kScene5070AviatorCapInventoryItem)) {
		if (_sceneChunkTable.isValidChunk(5))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5070HangingSceneItem);
	} else if (state.scene5070AviatorCapState == kScene5070AviatorCapIdentifiedState) {
		copyStageSmallRow(kScene5070HangingItemNameRow, kScene5070AviatorCapNameRow);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (state.scene5070AviatorCapState == kScene5070AviatorCapIdentifiedState)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene5070HangingItemVerbRecordIndex, 0);
	return true;
}

AmbientAudioProfile Scene5070::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 10, 25, 0x0b, 5, 100, 50);
}

byte Scene5070::ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const {
	if (cueId >= 0x0d && cueId <= 0x14)
		return kScene5070AmbientSoundVolumes[cueId - 0x0d];
	return defaultVolumePercent;
}

bool Scene5070::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene5070::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

void Scene5070::runMineCartEntryClip() {
	if (!_sceneChunkTable.isValidChunk(9))
		return;

	ResourceSpriteLayer &mineCartLayer = _sceneLayers.layer(kScene5070MineCartLayer);
	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	mineCartLayer.visible = true;
	mineCartLayer.reset(0);
	drawPlayableComposite();
	if (fadePaletteFromBlack()) {
		mineCartLayer.visible = false;
		_hideActiveActor = previousHideActiveActor;
		return;
	}

	ensureAmbientSoundCuePlaying(1, 0x0c, 10);
	_mineCartRumbleActive = true;
	_soundBank0.playSample(0x18, 100);
	for (uint frame = 0; frame < ARRAYSIZE(kScene5070MineCartDelayBuckets) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++frame) {
		const uint32 frameMillis = kScene5070MineCartTimerNumerator /
			MAX<uint32>(1, 13 - kScene5070MineCartDelayBuckets[frame]);
		if (waitSceneMillis(frameMillis, false))
			break;

		const byte nextFrame = (byte)(frame + 1);
		mineCartLayer.setFrame(nextFrame);
		if (nextFrame == 0x23) {
			_mineCartRumbleActive = false;
			_soundBank0.playSample(0x16, 100);
		}
		drawPlayableComposite();
		presentFrame();
	}

	_mineCartRumbleActive = false;
	mineCartLayer.visible = false;
	_hideActiveActor = previousHideActiveActor;
}

void Scene5070::runExitToMineSwitches() {
	walkActiveActorTo(0x3ab, 0x1df, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5070::runShovelPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5070ShovelTaken || hasInventoryItem(kScene5070ShovelInventoryItem)) {
		beginSecondarySpeechLine(0, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(8, kScene5070ShovelPickupDescriptorCount,
		kScene5070ShovelPickupFrameMap, ARRAYSIZE(kScene5070ShovelPickupFrameMap), kScene5070FrameMillis)
		.hookAt(6, kScene5070ShovelBackgroundPatchHook)
		.noFinalFrameDelay());
	addInventoryItem(kScene5070ShovelInventoryItem);
	_soundBank0.playSample(1, 100);
	state.scene5070ShovelTaken = true;
	applySceneStateToHotspotsAndPatches(0);
}

void Scene5070::runAviatorCapPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5070AviatorCapState == kScene5070AviatorCapTakenState || hasInventoryItem(kScene5070AviatorCapInventoryItem)) {
		beginSecondarySpeechLine(3, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(7, kScene5070AviatorCapPickupDescriptorCount,
		kScene5070AviatorCapPickupFrameMap, ARRAYSIZE(kScene5070AviatorCapPickupFrameMap), kScene5070FrameMillis)
		.noFinalFrameDelay()
		.noRedrawAtEnd());
	addInventoryItem(kScene5070AviatorCapInventoryItem);
	_soundBank0.playSample(1, 100);
	state.scene5070AviatorCapState = kScene5070AviatorCapTakenState;
	applySceneStateToHotspotsAndPatches(1);
	drawPlayableComposite();
	presentFrame();
}

void Scene5070::drawMineCartForeground() {
	const byte state = _vm->gameState().scene5070AviatorCapState;
	const uint chunkIndex = (state == kScene5070AviatorCapTakenState ||
		hasInventoryItem(kScene5070AviatorCapInventoryItem)) ? 11 : 10;
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

void Scene5070::copySlopeStepDeltasFromSet5A(uint firstOffset) {
	for (uint i = 0; i < 0x0c && firstOffset + i < _actorPathStepDeltas.size() &&
			firstOffset + i < ARRAYSIZE(kActorPathStepDeltaTableSet5A); ++i)
		_actorPathStepDeltas[firstOffset + i] = kActorPathStepDeltaTableSet5A[firstOffset + i];
}

void Scene5070::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	if (hookId != kScene5070ShovelBackgroundPatchHook)
		return;

	_vm->gameState().scene5070ShovelTaken = true;
	applySceneStateToHotspotsAndPatches(0);
}

void Scene5070::copyStageSmallRow(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene5070::clearSceneItemFromColorMap(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
