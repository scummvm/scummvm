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

#include "hollywood/scenes/playable/scene5080.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5080ViewportXOffset = 0x0050;
const uint kScene5080ActorBankTableEntrySet5A = 0x0070;
const uint kScene5080ActorBankTableEntrySetB4 = 0x0000;
const uint kScene5080ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5080SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5080FrameMillis = 75;
const uint kScene5080EntryDescriptorCount = 0x54;
const uint kScene5080BookPickupDescriptorCount = 0x0d;
const uint kScene5080WardrobeDescriptorCount = 6;
const byte kScene5080KeyInventoryItem = 0x4b;
const byte kScene5080BookInventoryItem = 0x51;
const byte kScene5080BookSourceItem = 10;
const byte kScene5080StairDoorSceneItem = 8;

enum {
	kScene5080MineCartArrivalHook = 1
};

const byte kScene5080AmbientSoundVolumes[] = {
	10, 10, 10, 2, 10, 10, 10, 100
};

const byte kScene5080BookPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene5080WardrobeFrameMap[] = {
	0, 1, 2, 3, 4, 5, 4, 3, 4, 5, 4, 3, 4, 5, 4, 3, 2, 1, 0
};

static PlayableSceneConfig scene5080Config() {
	PlayableSceneConfig config(5080,
		SceneResourceLayout(5, 5, 8),
		SceneViewport(kScene5080ViewportXOffset, kScene5080ViewportXOffset, kScene5080ViewportXOffset),
		SceneActorPose(0x2b8, 0x188, 5));
	config.setActorResources(kScene5080ActorBankTableEntrySet5A, kScene5080ActorPaletteTableEntry);
	config.setTextResources(0, kScene5080SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet5A);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	return config;
}

Scene5080::Scene5080(HollywoodEngine *vm) :
		PlayableScene(vm, scene5080Config()),
		_mineCartLayer(),
		_mineCartRumbleActive(false) {
	_mineCartLayer.configure(5, kScene5080EntryDescriptorCount, nullptr, 0);
}

uint Scene5080::resource000ActorBankTableEntry() const {
	return _vm->gameState().scene5080AlternatePassageSide ?
		kScene5080ActorBankTableEntrySetB4 : kScene5080ActorBankTableEntrySet5A;
}

const byte *Scene5080::actorPathStepDeltaTable() const {
	return _vm->gameState().scene5080AlternatePassageSide ?
		kActorPathStepDeltaTableSetB4 : kActorPathStepDeltaTableSet5A;
}

uint Scene5080::actorPathStepDeltaTableSize() const {
	return _vm->gameState().scene5080AlternatePassageSide ?
		ARRAYSIZE(kActorPathStepDeltaTableSetB4) : ARRAYSIZE(kActorPathStepDeltaTableSet5A);
}

void Scene5080::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	copyStepDeltasForCurrentSide();
	_mineCartLayer.visible = false;
	_mineCartLayer.reset(0);
	_mineCartRumbleActive = false;

	if (_vm->gameState().scene5080AlternatePassageSide) {
		_activeActorWorldX = 0x166;
		_activeActorWorldY = 0x0d6;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x2b8;
		_activeActorWorldY = 0x188;
		_activeActorFacing = 5;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene5080::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;
	if (drawActiveActor || drawSecondaryActor)
		updateSceneActorDepthAndPalette(activeFacing, activeWorldX, activeWorldY);

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();
	drawResourceSpriteLayer(_mineCartLayer);
}

bool Scene5080::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene5080::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene5080AlternatePassageSide) {
		drawPlayableComposite();
		presentFrame();
		return;
	}

	setActiveActorPose(0x348, 0x15e, 4);
	runMineCartEntryClip();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	walkActiveActorTo(0x2b8, 0x188, state.scene5080EntryLineSeen ? 0xff : 5, 0, false);

	if (!state.scene5080EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5080EntryLineSeen = true;
	}
}

void Scene5080::advanceCustomGameplayLoop(uint32 delta) {
	(void)delta;
	ensureAmbientSoundCuePlaying(1, 0x0c, 10);
	if (_mineCartRumbleActive && !_soundBank0.isPlaying())
		_soundBank0.playSample(0x18, 100);
}

bool Scene5080::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Usar vagoneta (use mine cart): return to the switch room.
		runExitToMineSwitches();
		return true;
	case 302: // Coger libro (take book): play the pickup and grant Karl's book.
		runBookPickup();
		return true;
	case 303: // Unreferenced original fallback: a random generic failure remark.
		beginStaticSecondarySpeechLine(1, (byte)_random.getRandomNumber(2));
		return true;
	case 304: // Mirar armario (look at wardrobe): Karl keeps his belongings here.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Abrir armario (open wardrobe): animate the failed locked-door attempt.
		runWardrobeAttempt();
		return true;
	case 306: // Mirar sofá (look at sofa): describe the old, filthy sofa bed.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Usar sofá (use sofa): Ron refuses to rest on it.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Coger/usar vaso (take/use glass): Ron refuses because it is filthy.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Mirar vaso (look at glass): describe Karl's poor hygiene.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Ir a escalera (go to stairs): cross to the other side once unlocked.
		runPassageSideSwitch();
		return true;
	case 311: // Mirar escalera (look at stairs): identify its railway-sleeper construction.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Ir a puerta (go to door): discover the blocked stairway, or cross it.
		handleStairDoorBarrier();
		return true;
	case 313: // Mirar puerta (look at door): describe it and update the approach path.
		handleStairDoorConstruction();
		return true;
	case 314: // Usar/abrir puerta (use/open door): report locked or already open.
		handleStairDoorUseOrOpen();
		return true;
	case 315: // Cerrar puerta (close door): report closed or refuse to close it.
		handleStairDoorClose();
		return true;
	case 316: // Dar llave a puerta (give key to door): unlock and cross the passage.
		handlePassageUnlock();
		return true;
	default:
		return false;
	}
}

bool Scene5080::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (_vm->gameState().scene5080AlternatePassageSide)
		targetX = CLIP<int>(targetX, 0x08f, 0x1e9);
	else if (targetX < 0x24e)
		targetX = 0x24e;

	if (targetY < 0x1df)
		++targetY;
	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene5080::customizeRouteSegment(byte currentRegion, byte nextRegion,
		const ActorPathBuildState &state, const ScenePoint &boundary,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 4 && nextRegion == 5) {
		copyRouteStepDeltas(0x30, kActorPathStepDeltaTableSetB4,
			ARRAYSIZE(kActorPathStepDeltaTableSetB4), 0);
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 3 && nextRegion == 1) {
		copyRouteStepDeltas(0x18, kActorPathStepDeltaTableSet5A,
			ARRAYSIZE(kActorPathStepDeltaTableSet5A), 0x24);
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene5080::customizeRouteFinal(byte currentRegion, byte targetRegion,
		const ActorPathBuildState &state, int targetX, int targetY,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)targetRegion;
	(void)state;

	if (targetX == 0x258 && targetY == 0x0fd) {
		copyRouteStepDeltas(0x3c, kActorPathStepDeltaTableSet5A,
			ARRAYSIZE(kActorPathStepDeltaTableSet5A), 0);
		requestedFacing = 5;
		restoredStepDeltas = true;
		return true;
	}
	if (targetX == 0x230 && targetY == 0x157) {
		copyRouteStepDeltas(0x0c, kActorPathStepDeltaTableSetB4,
			ARRAYSIZE(kActorPathStepDeltaTableSetB4), 0x24);
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene5080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	if (_paletteMaskOriginal.size() >= kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize &&
			_colorToActorDepthClassMap.size() >= kScenePaletteMapPageSize) {
		memcpy(_colorToActorDepthClassMap.data(),
			_paletteMaskOriginal.data() + kSceneColorToActorDepthClassMap,
			kScenePaletteMapPageSize);
	}

	GameplayState &state = _vm->gameState();
	if (state.scene5080BookTaken || hasInventoryItem(kScene5080BookInventoryItem))
		state.scene5080BookTaken = true;

	if (!state.scene5080AlternatePassageSide) {
		clearSceneItemFromColorMap(2);
		clearSceneItemFromColorMap(3);
		clearSceneItemFromColorMap(4);
		clearSceneItemFromColorMap(5);
		clearSceneItemFromColorMap(9);
		clearSceneItemFromColorMap(10);
	} else {
		clearSceneItemFromColorMap(1);
		clearSceneItemFromColorMap(6);
		clearSceneItemFromColorMap(7);
		clearSceneItemFromColorMap(8);
		remapSceneItemInColorMap(9, 7);
		remapSceneItemInColorMap(10, 2);
	}

	if (state.scene5080BookTaken) {
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		remapSceneItemInColorMap(2, 4);
		remapSceneItemInColorMap(kScene5080BookSourceItem, 0);
		remapActorDepthClass(5, 1);
	} else {
		if (state.scene5080AlternatePassageSide)
			remapSceneItemInColorMap(kScene5080BookSourceItem, 2);
		remapActorDepthClass(5, 2);
	}

	rebuildWalkableMaskForCurrentSide();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	if (state.scene5080PassageUnlocked) {
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x44, 0);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x45, 0);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x46, 0);
		_hotspots.setRelationMovementMode(kScene5080KeyInventoryItem,
			kScene5080StairDoorSceneItem, 1, 0);
	}
	if (state.scene5080StairDoorConstructionSeen)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x44, 0);
	if (!state.scene5080PassageUnlocked && state.scene5080StairDoorBarrierSeen)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x41, 0);

	ScenePoint interactionPoint;
	ScenePoint approachPoint;
	if (state.scene5080AlternatePassageSide) {
		interactionPoint.x = 0x230;
		interactionPoint.y = 0x157;
		approachPoint.x = 0x1aa;
		approachPoint.y = 0x0a4;
	} else {
		interactionPoint.x = 0x258;
		interactionPoint.y = 0x0fd;
		approachPoint.x = 0x286;
		approachPoint.y = 0x13a;
	}
	_hotspots.setActionTarget(7, interactionPoint, approachPoint);
	return true;
}

AmbientAudioProfile Scene5080::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 10, 25, 0x0b, 5, 100, 50);
}

byte Scene5080::ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const {
	if (cueId >= 0x0d && cueId <= 0x14)
		return kScene5080AmbientSoundVolumes[cueId - 0x0d];
	return defaultVolumePercent;
}

bool Scene5080::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene5080::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

void Scene5080::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	if (hookId != kScene5080MineCartArrivalHook) {
		PlayableScene::handleAnimationFrameHook(hookId, frame);
		return;
	}

	_mineCartRumbleActive = false;
	_soundBank0.playSample(0x16, 100);
}

void Scene5080::runMineCartEntryClip() {
	if (!_sceneChunkTable.isValidChunk(5)) {
		drawPlayableComposite();
		fadePaletteFromBlack();
		return;
	}

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	_mineCartLayer.visible = true;
	_mineCartLayer.reset(0);
	drawPlayableComposite();
	if (fadePaletteFromBlack()) {
		_mineCartLayer.visible = false;
		_hideActiveActor = previousHideActiveActor;
		return;
	}

	ensureAmbientSoundCuePlaying(1, 0x0c, 10);
	_mineCartRumbleActive = true;
	_soundBank0.playSample(0x18, 100);
	playAndPresentAnimationFrames(_mineCartLayer,
		AnimationFrameRange(0, kScene5080EntryDescriptorCount - 1, kScene5080FrameMillis)
			.hookAt(0x3c, kScene5080MineCartArrivalHook)
			.unskippable()
			.noFinalFrameDelay());

	_mineCartRumbleActive = false;
	_mineCartLayer.visible = false;
	_hideActiveActor = previousHideActiveActor;
}

void Scene5080::runExitToMineSwitches() {
	walkActiveActorTo(0x348, 0x15e, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5080::runBookPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5080BookTaken || hasInventoryItem(kScene5080BookInventoryItem))
		return;

	beginStaticSecondarySpeechLine(0x14, (byte)_random.getRandomNumber(4));
	runActorReplacement(ActionOverlaySpec(7, kScene5080BookPickupDescriptorCount,
		kScene5080BookPickupFrameMap, ARRAYSIZE(kScene5080BookPickupFrameMap), kScene5080FrameMillis)
		.noFinalFrameDelay()
		.noRedrawAtEnd());
	addInventoryItem(kScene5080BookInventoryItem);
	_soundBank0.playSample(1, 100);
	state.scene5080BookTaken = true;
	applySceneStateToHotspotsAndPatches(3);
	drawPlayableComposite();
	presentFrame();
}

void Scene5080::runWardrobeAttempt() {
	runActorReplacement(ActionOverlaySpec(8, kScene5080WardrobeDescriptorCount,
		kScene5080WardrobeFrameMap, ARRAYSIZE(kScene5080WardrobeFrameMap), kScene5080FrameMillis)
		.noFinalFrameDelay());
	beginSecondarySpeechLine(3, 0);
}

void Scene5080::runPassageSideSwitch() {
	GameplayState &state = _vm->gameState();
	if (!state.scene5080PassageUnlocked) {
		beginSecondarySpeechLine(8, 0);
		walkActiveActorTo(0x2b8, 0x188, 0xff, 0, false);
		return;
	}

	state.scene5080AlternatePassageSide = !state.scene5080AlternatePassageSide;
	switchActiveActorBankForCurrentSide();
	applySceneStateToHotspotsAndPatches(0xff);

	if (state.scene5080AlternatePassageSide) {
		setActiveActorPose(0x230, 0x157, 4);
		drawPlayableComposite();
		presentFrame();
		walkActiveActorTo(0x166, 0x0d6, 0xff, 0, false);
	} else {
		setActiveActorPose(0x258, 0x0fd, 2);
		drawPlayableComposite();
		presentFrame();
		walkActiveActorTo(0x2b8, 0x188, 0xff, 0, false);
	}
}

void Scene5080::handleStairDoorBarrier() {
	GameplayState &state = _vm->gameState();
	if (state.scene5080PassageUnlocked) {
		runPassageSideSwitch();
		return;
	}

	beginSecondarySpeechLine(10, 0);
	if (!state.scene5080StairDoorBarrierSeen) {
		state.scene5080StairDoorBarrierSeen = true;
		applySceneStateToHotspotsAndPatches(5);
		walkActiveActorTo(0x2b8, 0x188, 0xff, 0, false);
	}
}

void Scene5080::handleStairDoorConstruction() {
	GameplayState &state = _vm->gameState();
	if (state.scene5080PassageUnlocked) {
		beginSecondarySpeechLine(11, 1);
		return;
	}

	beginSecondarySpeechLine(11, 0);
	if (!state.scene5080StairDoorConstructionSeen) {
		state.scene5080StairDoorConstructionSeen = true;
		applySceneStateToHotspotsAndPatches(4);
		walkActiveActorTo(0x2b8, 0x188, 0xff, 0, false);
	}
}

void Scene5080::handleStairDoorUseOrOpen() {
	if (_vm->gameState().scene5080PassageUnlocked) {
		beginStaticSecondarySpeechLine(8, 0);
		return;
	}

	beginSecondarySpeechLine(12, 0);
	walkActiveActorTo(0x2b8, 0x188, 0xff, 0, false);
}

void Scene5080::handleStairDoorClose() {
	if (!_vm->gameState().scene5080PassageUnlocked) {
		beginStaticSecondarySpeechLine(11, 0);
		return;
	}

	beginStaticSecondarySpeechLine(0x13, (byte)_random.getRandomNumber(1));
}

void Scene5080::handlePassageUnlock() {
	GameplayState &state = _vm->gameState();
	if (!state.scene5080PassageUnlocked) {
		state.scene5080PassageUnlocked = true;
		applySceneStateToHotspotsAndPatches(1);
		beginStaticSecondarySpeechLine(8, 0);
		runPassageSideSwitch();
		return;
	}

	beginStaticSecondarySpeechLine(8, 0);
}

void Scene5080::switchActiveActorBankForCurrentSide() {
	if (!loadResource000ActorBank(_resource000OffsetTable, _resource000SizeTable))
		warning("%s failed to reload active actor bank for passage side", sceneDebugName());
	copyStepDeltasForCurrentSide();
}

void Scene5080::rebuildWalkableMaskForCurrentSide() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	const bool alternateSide = _vm->gameState().scene5080AlternatePassageSide;
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if ((!alternateSide && _walkablePaletteMask[i] > 2) ||
				(alternateSide && _walkablePaletteMask[i] < 6))
			_walkablePaletteMask[i] = 0;
	}
}

void Scene5080::updateSceneActorDepthAndPalette(byte facing, int worldX, int worldY) {
	_drawActorDepthYThresholds = _actorDepthYThresholds;
	if (_drawActorDepthYThresholds.size() > 2) {
		const bool sideFacing = facing >= 1 && facing <= 4;
		_drawActorDepthYThresholds[1] = sideFacing ? 0x01a2 : 0;
		_drawActorDepthYThresholds[2] = sideFacing ? 0x03e7 : 0x010c;
	}
	if (_drawActorDepthYThresholds.size() > 4) {
		_drawActorDepthYThresholds[4] =
			((facing >= 1 && facing <= 5) || (worldX > 0x178 && worldY < 0x0cf)) ? 0x03e7 : 0;
	}

	if (facing != 5 ||
			_paletteMaskOriginal.size() < kSceneColorToFootstepSoundMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToFootstepSoundMap + kScenePaletteMapPageSize)
		return;

	const bool alternatePaletteRegion = paletteRegionAt(worldX, worldY) > 5;
	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToActorPaletteDeltaClassMap + color] == 2) {
			_paletteMask[kSceneColorToActorPaletteDeltaClassMap + color] =
				alternatePaletteRegion ? 3 : 2;
		}
		if (_paletteMaskOriginal[kSceneColorToFootstepSoundMap + color] == 2) {
			_paletteMask[kSceneColorToFootstepSoundMap + color] =
				alternatePaletteRegion ? 10 : 2;
		}
	}
}

void Scene5080::clearSceneItemFromColorMap(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

void Scene5080::remapSceneItemInColorMap(byte fromItemId, byte toItemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + color] == fromItemId)
			_paletteMask[kSceneColorToItemMap + color] = toItemId;
	}
}

void Scene5080::remapActorDepthClass(byte fromClass, byte toClass) {
	if (_paletteMask.size() < kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize ||
			_colorToActorDepthClassMap.size() < kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToActorDepthClassMap + color] == fromClass) {
			_paletteMask[kSceneColorToActorDepthClassMap + color] = toClass;
			_colorToActorDepthClassMap[color] = toClass;
		}
	}
}

void Scene5080::copyRouteStepDeltas(uint destinationFirst, const byte *source,
		uint sourceSize, uint sourceFirst) {
	for (uint i = 0; i < 0x0c && destinationFirst + i < _actorPathStepDeltas.size() &&
			sourceFirst + i < sourceSize; ++i) {
		_actorPathStepDeltas[destinationFirst + i] = source[sourceFirst + i];
	}
}

void Scene5080::copyStepDeltasForCurrentSide() {
	resetActorPathStepDeltas();
}

} // End of namespace Hollywood
