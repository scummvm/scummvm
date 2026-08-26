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
const uint kScene5080SofaDescriptorCount = 6;
const byte kScene5080BookInventoryItem = 0x51;
const byte kScene5080ClosedPatchSourceItem = 10;

const byte kScene5080BookPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene5080SofaFrameMap[] = {
	0, 1, 2, 3, 4, 5, 4, 3, 4, 5, 4, 3, 4, 5, 4, 3, 2, 1, 0
};

static Common::Array<byte> sequentialFrameMap(uint frameCount) {
	Common::Array<byte> frameMap;
	frameMap.resize(frameCount);
	for (uint i = 0; i < frameMap.size(); ++i)
		frameMap[i] = (byte)i;
	return frameMap;
}

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
		PlayableScene(vm, scene5080Config()) {
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
	applySceneStateToHotspotsAndPatches(0xff);

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

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();
}

void Scene5080::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene5080AlternatePassageSide) {
		drawPlayableComposite();
		presentFrame();
		return;
	}

	setActiveActorPose(0x348, 0x15e, 4);
	drawPlayableComposite();
	presentFrame();

	runMineCartEntryClip();
	walkActiveActorTo(0x2b8, 0x188, state.scene5080EntryLineSeen ? 0xff : 5, 0, false);

	if (!state.scene5080EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5080EntryLineSeen = true;
	}
}

bool Scene5080::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene5080::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar/usar libro (look/use book): grants inventory item 0x51.
		runBookPickup();
		return true;
	case 302: // Mirar armario (look at wardrobe): Karl stores his belongings here.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Armario cerrado (locked wardrobe): no direct state change.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Coger sofa cama (take sofa bed): it is too dirty/old.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 305: // Usar sofa cama (use sofa bed): short bed animation, then locked response.
		runSofaOverlaySpeech();
		return true;
	case 306: // Coger vaso (take glass): shared refusal in the original.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar vaso (look at glass): dirty glass response.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar/hablar con tunel (look at tunnel).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Coger tunel (take tunnel): too dirty/pointless.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Dar/ir a escalera-paso (use stair passage): switch between the two room sides once unlocked.
		runPassageSideSwitch();
		return true;
	case 311: // Coger puerta (take door): it is closed.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Dar puerta (give door): first stair-door barrier check.
		handleStairDoorBarrier();
		return true;
	case 313: // Coger objeto oculto de la puerta/escalera: second stair-door construction check.
		handleStairDoorConstruction();
		return true;
	case 314: // Mirar/usar puerta oculta (look/use hidden door).
		beginSecondarySpeechLine(12, 0);
		return true;
	case 315: // Abrir puerta oculta (open hidden door): locked.
		beginSecondarySpeechLine(13, 0);
		return true;
	case 316: // Dar objeto correcto a puerta/escalera: unlocks passage-side switching.
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

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene5080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

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

	if ((selector == 3 || selector == 0xff) && state.scene5080BookTaken) {
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		remapSceneItemInColorMap(2, 4);
		clearSceneItemFromColorMap(kScene5080ClosedPatchSourceItem);
		remapActorDepthClass(5, 1);
	} else {
		if (state.scene5080AlternatePassageSide)
			remapSceneItemInColorMap(kScene5080ClosedPatchSourceItem, 2);
		remapActorDepthClass(5, 2);
	}

	rebuildWalkableMaskForCurrentSide();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	if (state.scene5080PassageUnlocked) {
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x44, 0);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x45, 0);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x46, 0);
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
	return createRandomAmbientAudioProfile(0x0d, 8, 75, 25, 0x0b, 5, 100, 50);
}

void Scene5080::runMineCartEntryClip() {
	const Common::Array<byte> frameMap = sequentialFrameMap(kScene5080EntryDescriptorCount);
	runActorReplacement(ActionOverlaySpec(5, kScene5080EntryDescriptorCount,
		frameMap.data(), frameMap.size(), kScene5080FrameMillis)
		.soundAt(0x3c, 0x16));
}

void Scene5080::runExitToMineSwitches() {
	walkActiveActorTo(0x348, 0x15e, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5080::runBookPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5080BookTaken || hasInventoryItem(kScene5080BookInventoryItem)) {
		beginSecondarySpeechLine(2, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(7, kScene5080BookPickupDescriptorCount,
		kScene5080BookPickupFrameMap, ARRAYSIZE(kScene5080BookPickupFrameMap), kScene5080FrameMillis));
	addInventoryItem(kScene5080BookInventoryItem);
	_soundBank0.playSample(1, 100);
	state.scene5080BookTaken = true;
	applySceneStateToHotspotsAndPatches(3);
}

void Scene5080::runSofaOverlaySpeech() {
	runActorReplacement(ActionOverlaySpec(8, kScene5080SofaDescriptorCount,
		kScene5080SofaFrameMap, ARRAYSIZE(kScene5080SofaFrameMap), kScene5080FrameMillis));
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

void Scene5080::handlePassageUnlock() {
	GameplayState &state = _vm->gameState();
	if (!state.scene5080PassageUnlocked) {
		state.scene5080PassageUnlocked = true;
		applySceneStateToHotspotsAndPatches(1);
		runPassageSideSwitch();
		return;
	}

	beginSecondarySpeechLine(11, 1);
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
	if (_paletteMask.size() < kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToActorDepthClassMap + color] == fromClass)
			_paletteMask[kSceneColorToActorDepthClassMap + color] = toClass;
	}
}

void Scene5080::copyStepDeltasForCurrentSide() {
	resetActorPathStepDeltas();
}

} // End of namespace Hollywood
