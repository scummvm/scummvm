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

#include "hollywood/scenes/playable/scene3070.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene3070ArchiveName = "RESOURCE.C07";
const char *const kScene3070MusicArchiveName = "RESOURCE.M03";
const char *const kScene3070SoundArchiveName = "RESOURCE.S03";
const uint kScene3070InitialRequiredChunkCount = 24;
const uint kScene3070ArenaFirstChunk = 5;
const uint kScene3070ArenaLastChunk = 23;
const uint kScene3070StageIndex = 307;
const uint16 kScene3070FirstState = 0x0bfe;
const uint16 kScene3070LastState = 0x0c07;
const uint16 kScene3070EntryFromOtherSideState = 0x0bff;
const uint16 kScene3070LateCutsceneState = 0x0c00;
const uint16 kScene3060ReturnFromScene3070State = 0x0bf5;
const uint16 kScene3110ShortTransitionState = 0x0c27;
const uint16 kScene3070LaterUnimplementedCutsceneState = 0x23b4;
const uint16 kScene3070ViewportXOffset = 0x0090;
const uint16 kScene3070ViewportMinXOffset = 0x0068;
const uint16 kScene3070ViewportMaxXOffset = 0x0090;
const uint kScene3070ActorBankTableEntry = 0x0038;
const uint kScene3070ActorPaletteTableEntry = 0x00cc;
const uint kScene3070Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3070BackFrameMillis = 125;
const uint32 kScene3070SmallIdleFrameMillis = 75;
const uint32 kScene3070OverlayFrameMillis = 75;
const uint kScene3070BackDescriptorCount = 0x10;
const uint kScene3070SmallIdleDescriptorCount = 0x16;
const uint kScene3070PatchOverlayDescriptorCount = 9;
const byte kScene3070InterludePrimaryRowLeft = 0x21;
const byte kScene3070InterludePrimaryRowRight = 0x22;

const byte kScene3070ActorPathStepDeltaTable[] = {
	6, 1, 1, 3, 3, 3, 7, 1, 0, 0, 4, 3,
	3, 2, 8, 6, 6, 7, 6, 4, 10, 3, 2, 9,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	4, 3, 3, 4, 0, 4, 4, 2, 0, 4, 2, 5,
	6, 10, 10, 4, 6, 4, 10, 8, 8, 7, 5, 10,
	6, 4, 10, 3, 2, 9, 3, 2, 8, 6, 6, 7
};

const byte kScene3070BackFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15
};

const byte kScene3070SmallIdleFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
};

const byte kScene3070PatchOverlayFrameMap[] = {
	8, 0, 1, 2, 2, 1, 0, 8
};

const byte kScene3070ItemPatchPickupFrameMap[] = {
	8, 0, 1, 2, 3, 4, 5, 6, 7, 8
};

static PlayableSceneConfig scene3070Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene3070ArchiveName;
	config.initialRequiredChunkCount = kScene3070InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene3070ArenaFirstChunk;
	config.arenaLastChunk = kScene3070ArenaLastChunk;
	config.stageIndex = kScene3070StageIndex;
	config.debugName = "Scene 3070";
	config.viewportXOffset = kScene3070ViewportXOffset;
	config.viewportMinXOffset = kScene3070ViewportMinXOffset;
	config.viewportMaxXOffset = kScene3070ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 3;
	config.actorBankTableEntry = kScene3070ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene3070ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene3070Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene3070SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene3070ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene3070ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 2;
	config.musicArchiveName = kScene3070MusicArchiveName;
	config.soundBank0ArchiveName = kScene3070SoundArchiveName;
	config.mainFlowFirstState = kScene3070FirstState;
	config.mainFlowLastState = kScene3070LastState;
	return config;
}

Scene3070::Scene3070(HollywoodEngine *vm) :
		PlayableScene(vm, scene3070Config(), "scene3070", 0x23f, 0x192, 5, 0xfd, 0xfb),
		_backChannel(),
		_smallIdleChannel(),
		_backLayer(),
		_smallIdleLayer(),
		_smallIdleMode(0) {
	_backLayer.configure(17, kScene3070BackDescriptorCount,
		kScene3070BackFrameMap, ARRAYSIZE(kScene3070BackFrameMap));
	_smallIdleLayer.configure(7, kScene3070SmallIdleDescriptorCount,
		kScene3070SmallIdleFrameMap, ARRAYSIZE(kScene3070SmallIdleFrameMap));
}

bool Scene3070::hasCustomPreviewState() const {
	return true;
}

void Scene3070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene3070EntryFromOtherSideState || stateId == kScene3070LateCutsceneState) {
		_activeActorWorldX = 0x24a;
		_activeActorWorldY = 0x13d;
		_activeActorFacing = 5;
	} else {
		_activeActorWorldX = 0x23f;
		_activeActorWorldY = 0x192;
		_activeActorFacing = 5;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene3070::hasCustomComposite() const {
	return true;
}

void Scene3070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_backLayer);
	drawActionOverlayLayer();
	if (activeWorldY <= 0x165) {
		const uint chunkIndex = _vm->gameState().scene3070OperatingTableForegroundAlternate ? 13 : 5;
		if (_sceneChunkTable.isValidChunk(chunkIndex))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
	}
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (activeWorldY <= 0x165)
		drawForegroundBlocks(activeWorldY, actorDrawOrderMode);
	drawResourceSpriteLayer(_smallIdleLayer);
}

bool Scene3070::hasCustomEntrySequence() const {
	return true;
}

void Scene3070::runCustomEntrySequence() {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene3070EntryFromOtherSideState)
		runEntryFromOtherSide();
	else if (stateId == kScene3070LateCutsceneState)
		runLateCutsceneBranch();
	else
		runEntryFromSecretPassage();
}

bool Scene3070::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene3070::advanceCustomGameplayLoop(uint32 delta) {
	advanceBackLayer(delta);
	advanceSmallIdleLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3070::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Ir a pasadizo secreto (go to secret passage): return to scene 3060.
		state.mainFlowStateId = kScene3060ReturnFromScene3070State;
		return true;
	case 302: // Mirar escalera (look at stairs): connects to the library.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar riachuelo (look at brook): looks natural.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar mesa de operaciones (look at operating table), state-aware.
		beginSecondarySpeechLine(3, state.scene3070OperatingTableAlternateDescription ? 1 : 0);
		return true;
	case 305: // Mirar mesa (look at table): dusty table / surgical thread clue.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar cubetas (look at tubs).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar panel de controles / cajon (look at controls/drawer).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar interruptor (look at switch): starts the machine.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Mirar cajon / aguja e hilo quirurgicos (look at drawer / surgical needle and thread).
		if (!state.scene3070DrawerOpen) {
			beginSecondarySpeechLine(8, 0);
		} else if (state.scene3070SurgicalNeedleThreadState < 2) {
			beginSecondarySpeechLine(8, 1);
			if (state.scene3070SurgicalNeedleThreadState == 0) {
				state.scene3070SurgicalNeedleThreadState = 1;
				applySceneStateToHotspotsAndPatches(5);
			}
		} else {
			beginSecondarySpeechLine(8, 2);
		}
		return true;
	case 310: // Abrir cajon (open drawer): reveals needle and thread.
		runDoorPatchOverlay(true);
		return true;
	case 311: // Cerrar cajon (close drawer).
		runDoorPatchOverlay(false);
		return true;
	case 312: // Coger aguja e hilo quirurgicos (take surgical needle and thread): adds item 0x32.
		runItemPatchPickup();
		return true;
	case 313: // Mirar aguja e hilo / Frankenstein (look at needle and thread / Frankenstein).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 314: // Hablar con Frankenstein inconsciente (talk to unconscious Frankenstein).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 315: // Coger Frankenstein (take Frankenstein): leave him in place.
		beginSecondarySpeechLine(11, 0);
		return true;
	case 316: // Usar/animar Frankenstein (use/revive Frankenstein), state-aware.
		beginSecondarySpeechLine(12, state.scene3070FrankensteinRevivalAlternateResponse ? 1 : 0);
		return true;
	default:
		return false;
	}
}

bool Scene3070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 1 || selector == 2 || selector == 5) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

		const GameplayState &state = _vm->gameState();
		if (state.scene3070DrawerOpen) {
			if (_sceneChunkTable.isValidChunk(7))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
			replaceColorMapItemFromOriginal(9, state.scene3070SurgicalNeedleThreadState == 1 ? 9 : 8);
		} else {
			if (_sceneChunkTable.isValidChunk(8))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
			replaceColorMapItemFromOriginal(9, 8);
		}

		if (state.scene3070FrankensteinBodyState != 0) {
			if (_sceneChunkTable.isValidChunk(18))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[18], _baseFramebuffer);
			if (_sceneChunkTable.isValidChunk(17))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[17], _baseFramebuffer);
			replaceColorMapItemFromOriginal(10, 10);
		} else {
			removeColorMapItem(10);
		}

		if (state.scene3070SurgicalNeedleThreadTaken)
			removeColorMapItem(9);

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

AmbientAudioProfile Scene3070::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x10;
	profile.musicCueCount = 2;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3070::resetAnimationLayers() {
	_backChannel.reset(0, kScene3070BackFrameMillis);
	_smallIdleChannel.reset(0, kScene3070SmallIdleFrameMillis);
	_backLayer.visible = true;
	_smallIdleLayer.visible = true;
	_backLayer.reset(0);
	_smallIdleLayer.reset(0);
	_smallIdleMode = 0;
}

void Scene3070::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}
}

void Scene3070::removeColorMapItem(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == itemId)
			_paletteMask[kSceneColorToItemMap + i] = 0;
	}
}

void Scene3070::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene3070::advanceBackLayer(uint32 delta) {
	const uint frameCount = _backChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		const byte firstFrame = _vm->gameState().scene3070MachineRunning ? 8 : 0;
		const byte lastFrame = _vm->gameState().scene3070MachineRunning ? 15 : 7;
		if (_backChannel.frameIndex < firstFrame || _backChannel.frameIndex >= lastFrame)
			_backChannel.frameIndex = firstFrame;
		else
			++_backChannel.frameIndex;
		_backLayer.setFrame(_backChannel.frameIndex);
	}
}

void Scene3070::advanceSmallIdleLayer(uint32 delta) {
	const uint frameCount = _smallIdleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_smallIdleMode == 0) {
			if (_smallIdleChannel.frameIndex < 6) {
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
			if (_smallIdleChannel.frameIndex < 11)
				++_smallIdleChannel.frameIndex;
			else
				_smallIdleMode = _smallIdleChannel.frameIndex = 0;
		} else if (_smallIdleMode == 2) {
			if (_smallIdleChannel.frameIndex < 21)
				++_smallIdleChannel.frameIndex;
			else
				_smallIdleMode = _smallIdleChannel.frameIndex = 0;
		}
		_smallIdleLayer.setFrame(_smallIdleChannel.frameIndex);
	}
}

void Scene3070::runEntryFromSecretPassage() {
	runEntryPath(0x2fe, 0x133, 5, 0x23f, 0x192);
	_activeActorFacing = 5;
	if (!_vm->gameState().scene3070EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene3070EntryLineSeen = true;
	}
}

void Scene3070::runEntryFromOtherSide() {
	GameplayState &state = _vm->gameState();
	runEntryPath(0x0dc, 0x1b6, 1, 0x24a, 0x13d);
	_activeActorFacing = 5;

	if (!state.scene3070InterludeCutsceneSeen) {
		state.scene3070InterludeCutsceneSeen = true;
		beginSecondarySpeechLine(0x0d, 2);
		runInterludeCutscene();
		beginSecondarySpeechLine(0x0d, 3);
		state.scene3070InterludeUnlocked = true;
		return;
	}

	beginSecondarySpeechLine(0x0d, 5);
	state.mainFlowStateId = kScene3110ShortTransitionState;
}

void Scene3070::runLateCutsceneBranch() {
	runEntryPath(0x24a, 0x13d, 5, 0x24a, 0x13d);
	beginSecondarySpeechLine(0x0d, 7);
	beginPrimarySpeechLine(0x0d, 8, 0x212, 0x09e, 0x20, 0x00, 0x3f);
	beginPrimarySpeechLine(0x0d, 9, 0x212, 0x09e, 0x20, 0x00, 0x3f);
	beginSecondarySpeechLine(0x0d, 10);
	GameplayState &state = _vm->gameState();
	state.scene3070LateCutscenePlayed = true;
	state.scene9140VariantIndex = 3;
	state.scene9140ReturnStateId = 6000;
	state.mainFlowStateId = kScene3070LaterUnimplementedCutsceneState;
}

void Scene3070::runInterludeCutscene() {
	beginPrimarySpeechLine(kScene3070InterludePrimaryRowLeft, 0, 0x0e3, 0x084, 0x0d, 0x32, 0x3a);
	beginPrimarySpeechLine(kScene3070InterludePrimaryRowRight, 0, 0x079, 0x086, 0x0a, 0x3f, 0x00);
	beginPrimarySpeechLine(kScene3070InterludePrimaryRowLeft, 1, 0x0e3, 0x084, 0x0d, 0x32, 0x3a);
	beginPrimarySpeechLine(kScene3070InterludePrimaryRowRight, 1, 0x079, 0x086, 0x0a, 0x3f, 0x00);
}

void Scene3070::runDoorPatchOverlay(bool open) {
	GameplayState &state = _vm->gameState();
	if (state.scene3070DrawerOpen == open) {
		dispatchGenericSceneAction(open ? 20 : 12);
		return;
	}

	runActionOverlay(ActionOverlaySpec(9, kScene3070PatchOverlayDescriptorCount,
		kScene3070PatchOverlayFrameMap, ARRAYSIZE(kScene3070PatchOverlayFrameMap), kScene3070OverlayFrameMillis)
		.hideActor());
	state.scene3070DrawerOpen = open;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene3070::runItemPatchPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene3070SurgicalNeedleThreadTaken) {
		beginSecondarySpeechLine(8, 2);
		return;
	}

	runActionOverlay(ActionOverlaySpec(9, kScene3070PatchOverlayDescriptorCount,
		kScene3070ItemPatchPickupFrameMap, ARRAYSIZE(kScene3070ItemPatchPickupFrameMap), kScene3070OverlayFrameMillis)
		.hideActor());
	state.scene3070SurgicalNeedleThreadState = 2;
	state.scene3070SurgicalNeedleThreadTaken = true;
	applySceneStateToHotspotsAndPatches(0xff);
	addInventoryItem(0x32);
	_soundBank0.playSample(1, 100);
}

void Scene3070::drawForegroundBlocks(int activeWorldY, byte actorDrawOrderMode) {
	if (activeWorldY < 0x179) {
		const uint chunkIndex = _vm->gameState().scene3070WindowForegroundPatchActive ? 16 : 6;
		if (_sceneChunkTable.isValidChunk(chunkIndex))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
	}
	if (actorDrawOrderMode > 5 && _sceneChunkTable.isValidChunk(19))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[19], _sceneFramebuffer);
}

} // End of namespace Hollywood
