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

#include "hollywood/scenes/playable/scene2040.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene2040ArchiveName = "RESOURCE.B04";
const char *const kScene2040MusicArchiveName = "RESOURCE.M02";
const char *const kScene2040SoundArchiveName = "RESOURCE.S02";
const uint kScene2040InitialRequiredChunkCount = 18;
const uint kScene2040ArenaFirstChunk = 5;
const uint kScene2040ArenaLastChunk = 17;
const uint kScene2040StageIndex = 204;
const uint16 kScene2030EntryFromSphinxState = 0x07f0;
const uint16 kScene2050EntryState = 0x0802;
const uint16 kScene2040EntryFromInteriorState = 0x07f9;
const uint16 kScene2040ViewportXOffset = 0x0000;
const uint16 kScene2040ViewportMaxXOffset = 0x0050;
const uint kScene2040ActorBankTableEntry = 0x0038;
const uint kScene2040ActorPaletteTableEntry = 0x00cc;
const uint kScene2040Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2040SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2040ForegroundFrameMillis = 300;
const uint32 kScene2040OverlayFrameMillis = 75;
const uint32 kScene2040SlowOverlayFrameMillis = 300;
const uint kScene2040ForegroundDescriptorCount = 5;
const uint kScene2040StonePickupDescriptorCount = 0x0b;
const uint kScene2040SphinxNoseDescriptorCount = 0x61;
const uint kScene2040ScarabPlacementDescriptorCount = 0x11;
const uint kScene2040EyeExchangeFirstDescriptorCount = 0x11;
const uint kScene2040EyeExchangeSecondDescriptorCount = 0x12;
const uint kScene2040BaseOpeningDescriptorCount = 0x10;

const byte kScene2040ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene2040ForegroundFrameMap[] = {
	0, 1, 2, 3, 4, 3, 2, 1
};

const byte kScene2040StonePickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const byte kScene2040SphinxNoseFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
	35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46,
	47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
	83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
	95, 96
};

const byte kScene2040ScarabPlacementFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 3, 13, 14, 3, 2, 1, 0, 11
};

const byte kScene2040EyeExchangeFirstFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 16, 15, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 4, 5,
	6, 7, 8, 9, 10, 11
};

const byte kScene2040EyeExchangeSecondFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17
};

const byte kScene2040BaseOpeningFrameMap[] = {
	8, 7, 6, 8, 8, 5, 4, 3, 3, 3, 3, 3, 3, 0, 0
};

static PlayableSceneConfig scene2040Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene2040ArchiveName;
	config.initialRequiredChunkCount = kScene2040InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene2040ArenaFirstChunk;
	config.arenaLastChunk = kScene2040ArenaLastChunk;
	config.stageIndex = kScene2040StageIndex;
	config.debugName = "Scene 2040";
	config.viewportXOffset = kScene2040ViewportXOffset;
	config.viewportMinXOffset = kScene2040ViewportXOffset;
	config.viewportMaxXOffset = kScene2040ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 2;
	config.actorBankTableEntry = kScene2040ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene2040ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene2040Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene2040SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene2040ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene2040ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 8;
	config.musicArchiveName = kScene2040MusicArchiveName;
	config.soundBank0ArchiveName = kScene2040SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	return config;
}

Scene2040::Scene2040(HollywoodEngine *vm) :
		PlayableScene(vm, scene2040Config(), "scene2040", 0x269, 0x175, 4, 0xfd, 0xfb),
		_foregroundChannel(),
		_foregroundLayer() {
	_foregroundLayer.configure(5, kScene2040ForegroundDescriptorCount,
		kScene2040ForegroundFrameMap, ARRAYSIZE(kScene2040ForegroundFrameMap));
}

void Scene2040::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetForegroundLayer();
	applySceneStateToHotspotsAndPatches(0xff);

	switch (_vm->gameState().mainFlowStateId) {
	case kScene2040EntryFromInteriorState:
		_activeActorWorldX = 0x23a;
		_activeActorWorldY = 0x168;
		_activeActorFacing = 4;
		break;
	default:
		_activeActorWorldX = 0x269;
		_activeActorWorldY = 0x175;
		_activeActorFacing = 4;
		break;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene2040::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();
	drawResourceSpriteLayer(_foregroundLayer);
}

void Scene2040::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene2040EntryFromInteriorState)
		runEntryFromInterior();
	else
		runEntryFromMarket();
}

bool Scene2040::prepareCustomGameplayLoop() {
	resetForegroundLayer();
	return true;
}

bool Scene2040::advanceCustomGameplayLoop(uint32 delta) {
	advanceForegroundLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2040::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();

	switch (handlerId) {
	case 301: // Ir a mercado egipcio (go to Egyptian market): return to scene 2030.
		state.mainFlowStateId = kScene2030EntryFromSphinxState;
		return true;
	case 302: // Ir al interior de la esfinge (go inside sphinx): gated forward exit.
		runExitToInterior();
		return true;
	case 303: // Mirar pasadizo (look at passage): no special response.
		return true;
	case 304: // Mirar montañita de arena (look at sand mound): changes after planting.
		if (state.scene2040SphinxFaceState == 1)
			beginSecondarySpeechLine(1, 0);
		else if (state.scene2040SphinxFaceState == 2)
			beginSecondarySpeechLine(1, 1);
		return true;
	case 305: // Mirar hendidura (look at slot): triangular slot exposed in the sand.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 306: // Coger planta del Nilo (take Nile plant): Ron will not take the whole plant.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 307: // Mirar planta del Nilo (look at Nile plant): fast-growing plant response.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 308: // Coger flor del Nilo (take Nile flower): pickup after the plant grows.
		runStonePickup();
		return true;
	case 309: // Mirar flor del Nilo (look at Nile flower): already collected response.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 310: // Mirar pata de la esfinge (look at sphinx paw): paws covered with sand.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 311: // Usar pala con pata de la esfinge izquierda (use shovel on left paw): already changed.
		beginSecondarySpeechLine(7, 1);
		return true;
	case 312: // Usar colmillo de tigre en hendidura (use tiger tooth in slot): opens the sphinx base.
		runBaseOpeningSequence();
		return true;
	case 313: // Usar ponchera en montañita de arena (use bowl on sand mound): updates the bowl inventory state.
		runEyeExchangeSequence();
		return true;
	case 314: // Usar semillas en montañita de arena (use seeds on sand mound): grows the Nile plant.
		runScarabPlacementSequence();
		return true;
	case 315: // Usar pala con pata de la esfinge derecha (use shovel on right paw): long sphinx face animation.
		runSphinxNoseSequence();
		return true;
	default:
		return false;
	}
}

bool Scene2040::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetX < 0x114)
		targetX = 0x114;
	if (targetX >= HollywoodEngine::kSceneBufferWidth)
		targetX = HollywoodEngine::kSceneBufferWidth - 1;
	if (targetY < 0)
		targetY = 0;
	if (targetY >= HollywoodEngine::kSceneBufferHeight)
		targetY = HollywoodEngine::kSceneBufferHeight - 1;

	if (targetY < 0x1df)
		++targetY;

	while (targetY < 0x1df) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		++targetY;
	}

	const uint bottomOffset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
	if (isFramebufferOffsetValid(bottomOffset) && _walkablePaletteMask[savedFramebufferPixelAt(bottomOffset)] != 0)
		return true;

	while (targetY > 0) {
		--targetY;
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
	}

	return true;
}

bool Scene2040::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)boundary;
	(void)restoredStepDeltas;

	if (currentRegion == 3 && nextRegion == 2) {
		requestedFacing = 2;
		return true;
	}
	if (currentRegion == 2 && nextRegion == 3 && state.x == 0x1ee && state.y == 0x153) {
		requestedFacing = 5;
		return true;
	}

	return false;
}

bool Scene2040::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;
	(void)restoredStepDeltas;

	if (currentRegion == 3) {
		requestedFacing = 5;
		return true;
	}

	return false;
}

bool Scene2040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (selector == 0 || selector == 0xff) {
		if (state.scene2040SphinxBasePatchState == 0) {
			replaceColorMapItem(2, 8);
		} else {
			replaceColorMapItem(8, 0);
			replaceColorMapItem(4, 2);
			restoreOriginalColorMapItem(2);
			if (_sceneChunkTable.isValidChunk(8))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		}
	}

	if (selector == 1 || selector == 0xff) {
		switch (state.scene2040SphinxFaceState) {
		case 0:
			if (_sceneChunkTable.isValidChunk(7))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
			replaceColorMapItem(3, 0);
			replaceColorMapItem(5, 0);
			replaceColorMapItem(4, 8);
			break;
		case 1:
		case 2:
			if (_sceneChunkTable.isValidChunk(6))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
			if (state.scene2040SphinxBasePatchState == 0 && _sceneChunkTable.isValidChunk(9))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
			restoreOriginalColorMapItem(3);
			restoreOriginalColorMapItem(4);
			replaceColorMapItem(5, 0);
			break;
		case 3:
			replaceColorMapItem(3, 0);
			restoreOriginalColorMapItem(5);
			if (_sceneChunkTable.isValidChunk(6))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
			if (state.scene2040SphinxBasePatchState == 0 && _sceneChunkTable.isValidChunk(9))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
			if (_sceneChunkTable.isValidChunk(11))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
			break;
		default:
			break;
		}
	}

	if (selector == 2 || selector == 0xff) {
		if (state.scene2040SphinxItemRevealed == 0) {
			replaceColorMapItem(6, 0);
			if (state.scene2040SphinxFaceState == 3 && _sceneChunkTable.isValidChunk(10))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
		} else {
			restoreOriginalColorMapItem(6);
		}
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene2040::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2040::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_vm->gameState().scene2040SphinxBasePatchState == 0) {
			if (_walkablePaletteMask[i] > 1)
				_walkablePaletteMask[i] = 0;
		} else if (_walkablePaletteMask[i] > walkablePaletteMaxRegion()) {
			_walkablePaletteMask[i] = 0;
		}
	}
}

void Scene2040::resetForegroundLayer() {
	_foregroundChannel.reset(0, kScene2040ForegroundFrameMillis);
	_foregroundLayer.visible = true;
	_foregroundLayer.reset(0);
}

void Scene2040::advanceForegroundLayer(uint32 delta) {
	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_foregroundChannel.frameIndex != 0) {
			if (_foregroundChannel.frameIndex < 7)
				++_foregroundChannel.frameIndex;
			else
				_foregroundChannel.frameIndex = 0;
		} else if (_random.getRandomNumber(24) == 0) {
			_foregroundChannel.frameIndex = 1;
		}
		_foregroundLayer.setFrame(_foregroundChannel.frameIndex);
	}
}

void Scene2040::runEntryFromMarket() {
	runEntryPath(0x366, 0x176, 4, 0x269, 0x175);
	_activeActorFacing = 4;
	_activeActorCel = 0;
}

void Scene2040::runEntryFromInterior() {
	_vm->gameplayMusic()->playMusicCue(0x0b, 100);
	runEntryPath(0x1a4, 0x190, 2, 0x1ee, 0x154);
	runEntryPath(0x1ee, 0x154, _activeActorFacing, 0x23a, 0x168);
	_activeActorFacing = 4;
	_activeActorCel = 0;
}

void Scene2040::runExitToInterior() {
	GameplayState &state = _vm->gameState();
	if (state.scene2040SphinxExitInterviewState < 2) {
		beginSecondarySpeechLine(11, 0);
		return;
	}

	if (state.scene2040SphinxExitInterviewState == 2) {
		beginSecondarySpeechLine(8, 2);
		state.scene2040SphinxExitInterviewState = 3;
	}

	walkActiveActorTo(0x1a4, 0x190, 4, 0, false);
	state.mainFlowStateId = kScene2050EntryState;
}

void Scene2040::runStonePickup() {
	runHiddenActorActionOverlay(13, kScene2040StonePickupDescriptorCount,
		kScene2040StonePickupFrameMap, ARRAYSIZE(kScene2040StonePickupFrameMap),
		kScene2040OverlayFrameMillis);
	_vm->gameState().scene2040SphinxItemRevealed = 0;
	applySceneStateToHotspotsAndPatches(2);
	addInventoryItem(0x2c);
	beginSecondarySpeechLine(5, 0);
}

void Scene2040::runSphinxNoseSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene2040SphinxFaceState != 0) {
		beginSecondarySpeechLine(7, 1);
		return;
	}

	if (_sceneChunkTable.isValidChunk(17)) {
		runActionOverlay(ActionOverlaySpec(17, kScene2040SphinxNoseDescriptorCount,
			kScene2040SphinxNoseFrameMap, ARRAYSIZE(kScene2040SphinxNoseFrameMap), kScene2040SlowOverlayFrameMillis)
			.hideActor()
			.soundAt(17, 0x2c, 50));
	}

	state.scene2040SphinxFaceState = 1;
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(7, 0);
}

void Scene2040::runScarabPlacementSequence() {
	runHiddenActorActionOverlay(14, kScene2040ScarabPlacementDescriptorCount,
		kScene2040ScarabPlacementFrameMap, ARRAYSIZE(kScene2040ScarabPlacementFrameMap),
		kScene2040SlowOverlayFrameMillis);
	removeInventoryItem(0x2b);
	_vm->gameState().scene2040SphinxFaceState = 2;
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(10, 1);
}

void Scene2040::runEyeExchangeSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene2040SphinxFaceState == 1) {
		beginSecondarySpeechLine(9, 0);
		return;
	}
	if (state.scene2040SphinxFaceState != 2)
		return;

	runActionOverlay(ActionOverlaySpec(14, kScene2040EyeExchangeFirstDescriptorCount,
		kScene2040EyeExchangeFirstFrameMap, ARRAYSIZE(kScene2040EyeExchangeFirstFrameMap), kScene2040SlowOverlayFrameMillis)
		.hideActor()
		.soundAt(8, 0x20));
	removeInventoryItem(0x52);
	addInventoryItem(0x1a);
	walkActiveActorTo(0x210, 0x172, 1, 0, false);
	runVisibleActorActionOverlay(12, kScene2040EyeExchangeSecondDescriptorCount,
		kScene2040EyeExchangeSecondFrameMap, ARRAYSIZE(kScene2040EyeExchangeSecondFrameMap),
		kScene2040SlowOverlayFrameMillis);
	beginSecondarySpeechLine(9, 1);
	state.scene2040SphinxFaceState = 3;
	state.scene2040SphinxItemRevealed = 1;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene2040::runBaseOpeningSequence() {
	GameplayState &state = _vm->gameState();

	if (state.scene2040SphinxBasePatchState != 0) {
		beginSecondarySpeechLine(8, 1);
		return;
	}

	runHiddenActorActionOverlay(16, kScene2040BaseOpeningDescriptorCount,
		kScene2040BaseOpeningFrameMap, ARRAYSIZE(kScene2040BaseOpeningFrameMap),
		kScene2040SlowOverlayFrameMillis);
	removeInventoryItem(0x26);
	beginSecondarySpeechLine(8, 0);
	walkActiveActorTo(0x26c, 0x16e, 5, 0, false);

	state.scene2040SphinxBasePatchState = 1;
	applySceneStateToHotspotsAndPatches(0);
	beginSecondarySpeechLine(8, 1);
	if (state.scene2040SphinxExitInterviewState == 2) {
		beginSecondarySpeechLine(8, 2);
		state.scene2040SphinxExitInterviewState = 3;
	}
}

void Scene2040::replaceColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene2040::restoreOriginalColorMapItem(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (originalColorMapItemAt(i) == itemId)
			_paletteMask[kSceneColorToItemMap + i] = itemId;
	}
}

byte Scene2040::originalColorMapItemAt(uint paletteIndex) const {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			paletteIndex >= kScenePaletteMapPageSize)
		return 0;

	return _paletteMaskOriginal[kSceneColorToItemMap + paletteIndex];
}

} // End of namespace Hollywood
