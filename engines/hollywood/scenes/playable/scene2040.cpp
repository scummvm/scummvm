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
const uint32 kScene2040ActionFrameMillis = 75;
const uint kScene2040ForegroundDescriptorCount = 5;
const uint kScene2040FlowerPickupDescriptorCount = 0x0b;
const uint kScene2040SphinxNoseDescriptorCount = 0x61;
const uint kScene2040SeedPlantingDescriptorCount = 0x11;
const uint kScene2040EyeExchangeFirstDescriptorCount = 0x11;
const uint kScene2040EyeExchangeSecondDescriptorCount = 0x12;
const uint kScene2040BaseOpeningDescriptorCount = 0x0a;
const uint kScene2040BaseOpeningDeltaTableEntryCount = 0x1d;
const uint kScene2040EyePaletteChunk = 15;
const uint kScene2040BaseOpeningDeltaChunk = 18;
const byte kScene2040EyePaletteFirstColor = 0xf2;
const byte kScene2040EyePaletteLastColor = 0xf9;
const byte kScene2040FlowerPickupHook = 1;
const byte kScene2040SphinxNoseHook = 2;
const byte kScene2040EyeExchangeFirstHook = 3;
const byte kScene2040EyeExchangeSecondHook = 4;

const byte kScene2040ForegroundFrameMap[] = {
	0, 1, 2, 3, 4, 3, 2, 1
};

const byte kScene2040FlowerPickupFrameMap[] = {
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

const byte kScene2040SeedPlantingFrameMap[] = {
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
	8, 8, 7, 6, 5, 4, 3, 3, 3, 3, 3, 2, 1, 0, 9
};

const byte kScene2040BaseOpeningDeltaFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, 26, 27, 28
};

static PlayableSceneConfig scene2040Config() {
	PlayableSceneConfig config(2040,
		SceneResourceLayout(19, 5, 18),
		SceneViewport(kScene2040ViewportXOffset, kScene2040ViewportXOffset, kScene2040ViewportMaxXOffset),
		SceneActorPose(0x269, 0x175, 4));
	config.setActorResources(kScene2040ActorBankTableEntry, kScene2040ActorPaletteTableEntry);
	config.setTextResources(kScene2040Resource003RowsOffsetIndex, kScene2040SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 8;
	config.useActorDepthTest = true;
	return config;
}

Scene2040::Scene2040(HollywoodEngine *vm) :
		PlayableScene(vm, scene2040Config()),
		_foregroundChannel(),
		_behindActorLayer(),
		_foregroundLayer(),
		_routeStartX(0),
		_routeStartY(0) {
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
	copyBaseFramebufferToSceneFramebuffer();
	updateSceneDepthThresholds(actorDrawOrderMode);
	drawResourceSpriteLayer(_behindActorLayer);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();
	drawResourceSpriteLayer(_foregroundLayer);
}

void Scene2040::updateSceneDepthThresholds(byte actorDrawOrderMode) {
	_drawActorDepthYThresholds = _actorDepthYThresholds;
	if (_drawActorDepthYThresholds.size() > 2)
		_drawActorDepthYThresholds[2] = actorDrawOrderMode == 3 ? 0x03e7 : 0;
}

void Scene2040::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene2040EntryFromInteriorState)
		runEntryFromInterior();
	else
		runEntryFromMarket();
}

bool Scene2040::prepareCustomGameplayLoop() {
	clearResourceLayer(_behindActorLayer);
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
	case 303: // Mirar pasadizo (look at passage): describe the dark passage.
		beginSecondarySpeechLine(0, 0);
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
		runFlowerPickup();
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
		runSeedPlantingSequence();
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

void Scene2040::prepareCustomActorPathRoute(int startX, int startY) {
	_routeStartX = startX;
	_routeStartY = startY;
}

bool Scene2040::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 3 && nextRegion == 2) {
		copyStepDeltasFromB4(0x18, 0, 0x0c);
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 2 && nextRegion == 3 && _routeStartX == 0x1ee && _routeStartY == 0x153) {
		requestedFacing = 5;
		restoredStepDeltas = true;
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

	if (currentRegion == 3) {
		copyStepDeltasFromB4(0x3c, 0x24, 0x0c);
		requestedFacing = 5;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene2040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene2040SphinxBasePatchState == 0) {
		replaceColorMapItem(2, 8);
	} else {
		replaceColorMapItem(8, 0);
		replaceColorMapItem(4, 2);
		restoreOriginalColorMapItem(2);
		if (_sceneChunkTable.isValidChunk(8))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
	}

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
		if (state.scene2040SphinxBasePatchState == 0) {
			if (_sceneChunkTable.isValidChunk(9))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
			restoreOriginalColorMapItem(4);
		}
		restoreOriginalColorMapItem(3);
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

	if (state.scene2040SphinxItemRevealed == 0) {
		replaceColorMapItem(6, 0);
		if (state.scene2040SphinxFaceState == 3 && _sceneChunkTable.isValidChunk(10))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
	} else {
		restoreOriginalColorMapItem(6);
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene2040::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 125;
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
}

void Scene2040::runEntryFromInterior() {
	_vm->gameplayMusic()->playMusicCue(0x0b, 100);
	runEntryPath(0x1a4, 0x190, 2, 0x1ee, 0x154);
	runEntryPath(0x1ee, 0x154, _activeActorFacing, 0x23a, 0x168);
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

	const uint entranceOffset = 0x190 * HollywoodEngine::kSceneBufferWidth + 0x1a4;
	const byte entranceColor = savedFramebufferPixelAt(entranceOffset);
	const bool hasEntranceRegion = entranceColor < _fullPaletteRegionMask.size();
	const byte previousEntranceRegion = hasEntranceRegion ? _fullPaletteRegionMask[entranceColor] : 0;
	// The doorway is not floor; the original routes it as region 3 for this scripted walk.
	if (hasEntranceRegion)
		_fullPaletteRegionMask[entranceColor] = 3;

	const bool completed = walkActiveActorTo(0x1a4, 0x190, 0xff, 0xff, false);
	if (hasEntranceRegion)
		_fullPaletteRegionMask[entranceColor] = previousEntranceRegion;
	if (!completed)
		return;

	state.mainFlowStateId = kScene2050EntryState;
}

void Scene2040::runFlowerPickup() {
	runActorReplacement(ActionOverlaySpec(13, kScene2040FlowerPickupDescriptorCount,
		kScene2040FlowerPickupFrameMap, ARRAYSIZE(kScene2040FlowerPickupFrameMap),
		kScene2040ActionFrameMillis)
		.hookAt(6, kScene2040FlowerPickupHook));
	_vm->gameState().scene2040SphinxItemRevealed = 0;
	applySceneStateToHotspotsAndPatches(2);
	addInventoryItem(0x2c);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(5, 0);
}

void Scene2040::runSphinxNoseSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene2040SphinxFaceState != 0) {
		beginSecondarySpeechLine(7, 1);
		return;
	}

	if (_sceneChunkTable.isValidChunk(9))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);

	if (_sceneChunkTable.isValidChunk(17)) {
		runActorReplacement(ActionOverlaySpec(17, kScene2040SphinxNoseDescriptorCount,
			kScene2040SphinxNoseFrameMap, ARRAYSIZE(kScene2040SphinxNoseFrameMap), kScene2040ActionFrameMillis)
			.hookEveryFrame(kScene2040SphinxNoseHook));
	}

	state.scene2040SphinxFaceState = 1;
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(7, 0);
}

void Scene2040::runSeedPlantingSequence() {
	beginSecondarySpeechLine(10, 0);
	runActorReplacement(14, kScene2040SeedPlantingDescriptorCount,
		kScene2040SeedPlantingFrameMap, ARRAYSIZE(kScene2040SeedPlantingFrameMap),
		kScene2040ActionFrameMillis);
	removeInventoryItem(0x2b);
	_soundBank0.playSample(1, 100);
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

	installEyeEffectPalette();
	runActorReplacement(ActionOverlaySpec(14, kScene2040EyeExchangeFirstDescriptorCount,
		kScene2040EyeExchangeFirstFrameMap, ARRAYSIZE(kScene2040EyeExchangeFirstFrameMap), kScene2040ActionFrameMillis)
		.hookEveryFrame(kScene2040EyeExchangeFirstHook));
	_soundBank0.stop();
	restoreEyeEffectPalette();
	removeInventoryItem(0x52);
	addInventoryItem(0x1a);
	_soundBank0.playSample(1, 100);
	walkActiveActorTo(0x210, 0x172, 1, 0, false);
	AnimationFrameRange secondRange(0, ARRAYSIZE(kScene2040EyeExchangeSecondFrameMap) - 1,
		kScene2040ActionFrameMillis);
	secondRange.hookEveryFrame(kScene2040EyeExchangeSecondHook);
	playResourceLayerSequence(_behindActorLayer, 12, kScene2040EyeExchangeSecondDescriptorCount,
		kScene2040EyeExchangeSecondFrameMap, ARRAYSIZE(kScene2040EyeExchangeSecondFrameMap), secondRange);
	_soundBank0.stop();
	state.scene2040SphinxFaceState = 3;
	state.scene2040SphinxItemRevealed = 1;
	applySceneStateToHotspotsAndPatches(0xff);
	beginSecondarySpeechLine(9, 1);
}

void Scene2040::runBaseOpeningSequence() {
	GameplayState &state = _vm->gameState();

	if (state.scene2040SphinxBasePatchState != 0) {
		beginSecondarySpeechLine(8, 1);
		return;
	}

	runActorReplacement(16, kScene2040BaseOpeningDescriptorCount,
		kScene2040BaseOpeningFrameMap, ARRAYSIZE(kScene2040BaseOpeningFrameMap),
		kScene2040ActionFrameMillis);
	removeInventoryItem(0x26);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(8, 0);
	walkActiveActorTo(0x26c, 0x16e, 5, 0, false);
	runBaseOpeningDeltaSequence();

	state.scene2040SphinxBasePatchState = 1;
	applySceneStateToHotspotsAndPatches(0);
	beginSecondarySpeechLine(8, 1);
	if (state.scene2040SphinxExitInterviewState == 2) {
		beginSecondarySpeechLine(8, 2);
		state.scene2040SphinxExitInterviewState = 3;
	}
}

void Scene2040::runBaseOpeningDeltaSequence() {
	if (!_sceneChunkTable.isValidChunk(kScene2040BaseOpeningDeltaChunk))
		return;

	_soundBank0.playSample(0x2d, 100);
	for (uint frame = 0; frame < ARRAYSIZE(kScene2040BaseOpeningDeltaFrameMap) &&
			!animationPlaybackShouldStop(); ++frame) {
		restoreResourceSpriteLayerBackground(_foregroundLayer, _baseFramebuffer);
		drawResourceSpriteLayer(_foregroundLayer);
		drawClipFrameDelta(kScene2040BaseOpeningDeltaChunk,
			kScene2040BaseOpeningDeltaTableEntryCount,
			kScene2040BaseOpeningDeltaFrameMap[frame]);
		presentFrame();
		if (waitDeltaClipFrameMillis(kScene2040ActionFrameMillis))
			break;
		advanceForegroundLayer(kScene2040ActionFrameMillis);
		updateAmbientAudioAndMusicCues(kScene2040ActionFrameMillis);
	}
	_soundBank0.stop();
}

void Scene2040::handleAnimationFrameHook(byte hookId, uint frame) {
	switch (hookId) {
	case kScene2040FlowerPickupHook:
		if (_sceneChunkTable.isValidChunk(10))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
		break;
	case kScene2040SphinxNoseHook:
		if (frame < ARRAYSIZE(kScene2040SphinxNoseFrameMap)) {
			const byte descriptor = kScene2040SphinxNoseFrameMap[frame];
			if (descriptor == 0x11 || descriptor == 0x1d || descriptor == 0x29 ||
					descriptor == 0x35 || descriptor == 0x41 || descriptor == 0x4d)
				_soundBank0.playSample(0x2c, 50);
		}
		break;
	case kScene2040EyeExchangeFirstHook:
		if (frame == 8)
			_soundBank0.playSample(0x20, 100);
		else if (frame == 0x17)
			_soundBank0.stop();
		if (frame != 0 && frame % 4 == 0)
			rotateEyeEffectPalette();
		break;
	case kScene2040EyeExchangeSecondHook:
		if (frame == 1)
			_soundBank0.playSample(0x1f, 100);
		else if (frame == 0x12)
			_soundBank0.stop();
		break;
	default:
		PlayableScene::handleAnimationFrameHook(hookId, frame);
		break;
	}
}

void Scene2040::installEyeEffectPalette() {
	const uint paletteOffset = kScene2040EyePaletteFirstColor * 3;
	const uint byteCount = (kScene2040EyePaletteLastColor - kScene2040EyePaletteFirstColor + 1) * 3;
	if (!_sceneChunkTable.isValidChunk(kScene2040EyePaletteChunk) ||
			_sceneChunkTable.sizes[kScene2040EyePaletteChunk] < byteCount ||
			_paletteCurrent.size() < paletteOffset + byteCount)
		return;

	const uint32 resourceOffset = _resourceChunkOffsets[kScene2040EyePaletteChunk];
	if (resourceOffset > _resourceArena.size() || byteCount > _resourceArena.size() - resourceOffset)
		return;

	memcpy(_paletteCurrent.data() + paletteOffset, _resourceArena.data() + resourceOffset, byteCount);
	_displayPalette.markAllDirty();
}

void Scene2040::restoreEyeEffectPalette() {
	const uint paletteOffset = kScene2040EyePaletteFirstColor * 3;
	const uint byteCount = (kScene2040EyePaletteLastColor - kScene2040EyePaletteFirstColor + 1) * 3;
	if (_paletteCurrent.size() < paletteOffset + byteCount ||
			_paletteResource.size() < paletteOffset + byteCount)
		return;

	memcpy(_paletteCurrent.data() + paletteOffset, _paletteResource.data() + paletteOffset, byteCount);
	_displayPalette.markAllDirty();
}

void Scene2040::rotateEyeEffectPalette() {
	const uint lastOffset = kScene2040EyePaletteLastColor * 3;
	if (_paletteCurrent.size() < lastOffset + 3)
		return;

	byte lastColor[3];
	memcpy(lastColor, _paletteCurrent.data() + lastOffset, sizeof(lastColor));
	for (int color = kScene2040EyePaletteLastColor; color > kScene2040EyePaletteFirstColor; --color)
		memcpy(_paletteCurrent.data() + color * 3, _paletteCurrent.data() + (color - 1) * 3, 3);
	memcpy(_paletteCurrent.data() + kScene2040EyePaletteFirstColor * 3, lastColor, sizeof(lastColor));
	_displayPalette.markAllDirty();
}

void Scene2040::copyStepDeltasFromB4(uint targetOffset, uint sourceOffset, uint count) {
	for (uint i = 0; i < count && targetOffset + i < _actorPathStepDeltas.size() &&
			sourceOffset + i < ARRAYSIZE(kActorPathStepDeltaTableSetB4); ++i) {
		_actorPathStepDeltas[targetOffset + i] = kActorPathStepDeltaTableSetB4[sourceOffset + i];
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
