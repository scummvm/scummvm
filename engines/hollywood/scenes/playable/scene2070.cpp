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

#include "hollywood/scenes/playable/scene2070.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene2050LabyrinthReturnState = 0x0803;
const uint16 kScene2060RightPassageState = 0x080f;
const uint16 kScene2070EntryFromRightPassageState = 0x0817;
const uint16 kScene2080FirstState = 0x0820;
const uint16 kScene2070ViewportXOffset = 0x0068;
const uint16 kScene2070ViewportMaxXOffset = 0x00a8;
// The original calls LoadActorSpriteBankSetB4(), but the RESOURCE.000
// startup table maps that bank to offset/size table entry 0x0000.
const uint kScene2070ActorBankTableEntry = 0x0000;
const uint kScene2070ActorPaletteTableEntry = 0x00cc;
const uint kScene2070Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2070ForegroundFrameMillis = 75;
const uint32 kScene2070MemoryFrameMillis = 60;
const uint32 kScene2070OverlayFrameMillis = 75;
const uint kScene2070ForegroundDescriptorCount = 0x1a;
const uint kScene2070MemoryDescriptorCount = 0x5a;
const uint kScene2070InventoryOverlayDescriptorCount = 0x0d;
const uint kScene2070ExitVerbRecordIndex = 0x29;
const byte kScene2070SealMemoryLoopSound = 0x24;
const byte kScene2070SealMemoryEndSound = 0x2e;
const byte kScene2070SealMemoryEndSoundHook = 1;
const uint kScene2070SealMemoryEndSoundFrame = 0x56;

const byte kScene2070ForegroundFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25
};

const byte kScene2070SealMemoryFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
	27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,
	82, 83, 84, 85, 86, 87, 88, 89
};

const byte kScene2070InventoryOverlayFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

static_assert(ARRAYSIZE(kScene2070ForegroundFrameMap) == 0x1a, "Scene 2070 foreground frame map size changed");
static_assert(ARRAYSIZE(kScene2070SealMemoryFrameMap) == 0x62, "Scene 2070 seal memory frame map size changed");
static_assert(ARRAYSIZE(kScene2070InventoryOverlayFrameMap) == 14, "Scene 2070 inventory overlay frame map size changed");

class Scene2070DeltaFrameTarget {
public:
	explicit Scene2070DeltaFrameTarget(byte &frame) : _frame(frame) {}

	void setFrame(byte frame) { _frame = frame; }

private:
	byte &_frame;
};

static PlayableSceneConfig scene2070Config() {
	PlayableSceneConfig config(2070,
		SceneResourceLayout(11, 5, 10),
		SceneViewport(kScene2070ViewportXOffset, kScene2070ViewportXOffset, kScene2070ViewportMaxXOffset),
		SceneActorPose(0x0ef, 0x12c, 2));
	config.setActorResources(kScene2070ActorBankTableEntry, kScene2070ActorPaletteTableEntry);
	config.setTextResources(kScene2070Resource003RowsOffsetIndex, kScene2070SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = true;
	return config;
}

Scene2070::Scene2070(HollywoodEngine *vm) :
		PlayableScene(vm, scene2070Config()),
		_foregroundChannel(),
		_foregroundLayer(),
		_sealMemoryActive(false),
		_sealMemoryFrame(0) {
	_foregroundLayer.configure(7, kScene2070ForegroundDescriptorCount,
		kScene2070ForegroundFrameMap, ARRAYSIZE(kScene2070ForegroundFrameMap));
}

void Scene2070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetForegroundLayer();
	applySceneStateToHotspotsAndPatches(0xff);

	const GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene2070EntryFromRightPassageState) {
		_activeActorWorldX = 0x30e;
		_activeActorWorldY = 0x0ee;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x04b;
		_activeActorWorldY = 0x110;
		_activeActorFacing = 2;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene2070::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene2070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_foregroundLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (_sealMemoryActive)
		drawSealMemoryDeltaLayer();
}

void Scene2070::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene2070EntryFromRightPassageState)
		runEntryFromRightPassage();
	else
		runEntryFromLabyrinth();
}

bool Scene2070::advanceCustomGameplayLoop(uint32 delta) {
	advanceForegroundLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2070::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();

	switch (handlerId) {
	case 301: // Ir a laberinto (go to labyrinth): returns to B06 unless the seal route has been solved.
		state.mainFlowStateId = state.egyptSealPuzzleProgress == 2 ?
			kScene2050LabyrinthReturnState : kScene2060RightPassageState;
		return true;
	case 302: // Mirar laberinto (look at labyrinth): Ron says the maze was no match for him.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Coger antorcha (take torch): Ron says he will not need it.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Mirar antorcha (look at torch): ancient torch still burning.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 305: // Mirar agujero (look at hole): before/after the scarab opens the mechanism.
		beginSecondarySpeechLine(5, state.scene2070SealExitPatchState != 0 ? 1 : 0);
		return true;
	case 306: // Mirar sello (look at seal): second sacred labyrinth seal.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 307: // Ir a puerta (go to door): enter the next B08 seal chamber.
		state.mainFlowStateId = kScene2080FirstState;
		return true;
	case 308: // Mirar puerta (look at door): closed/open variant.
		beginSecondarySpeechLine(7, state.scene2070SealExitPatchState != 0 ? 1 : 0);
		return true;
	case 309: // Abrir puerta (open door): blocked until the seal mechanism has opened it.
		if (state.scene2070SealExitPatchState == 0)
			beginSecondarySpeechLine(7, 0);
		else
			beginStaticSecondarySpeechLine(8, 0);
		return true;
	case 310: // Cerrar puerta (close door): impossible before/after the mechanism changes the chamber.
		if (state.scene2070SealExitPatchState == 0)
			beginStaticSecondarySpeechLine(0x0b, 0);
		else
			beginStaticSecondarySpeechLine(0x13, (byte)_random.getRandomNumber(1));
		return true;
	case 311: // Hablar con Gunther Hecker (talk to Gunther Hecker): joke about the curse.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 312: // Mirar Gunther Hecker (look at Gunther Hecker): he is tough enough.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 313: // Coger navaja (take pocket knife): pickup overlay grants the pocket knife.
		runAnimatedInventoryStateChange();
		return true;
	case 314: // Mirar navaja (look at pocket knife): multi-purpose knife description.
		beginSecondarySpeechLine(11, 0);
		return true;
	default:
		return false;
	}
}

bool Scene2070::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x064, 0x2b7);
	if (targetY < 0x1df)
		++targetY;

	while (targetY < 0x1df) {
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
		++targetY;
	}

	if (walkableMaskAt(targetX, targetY) != 0)
		return true;

	while (targetY > 0) {
		--targetY;
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
	}

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene2070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;

	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if (state.scene2070SealExitPatchState != 0) {
		if (_sceneChunkTable.isValidChunk(5))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
	}

	if (state.scene2070InnerPassagePatchState == 0) {
		for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
			if (_fullPaletteRegionMask[color] > 3)
				_fullPaletteRegionMask[color] = 1;
			if (_paletteMask[kSceneColorToItemMap + color] == 6)
				_paletteMask[kSceneColorToItemMap + color] = 0;
			if (_paletteMask[kSceneColorToItemMap + color] == 8)
				_paletteMask[kSceneColorToItemMap + color] = 5;
			if (_paletteMask[kSceneColorToItemMap + color] == 9)
				_paletteMask[kSceneColorToItemMap + color] = 4;
		}
		if (_routeSteps.size() > 0x1b5)
			_routeSteps[0x1b5] = 2;
		if (_routeSteps.size() > 0x331)
			_routeSteps[0x331] = 1;
		if (_actorDepthYThresholds.size() > 3) {
			_actorDepthYThresholds[3] = 0;
			_drawActorDepthYThresholds[3] = 0;
		}
	} else {
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
			if (_fullPaletteRegionMask[color] == 8)
				_fullPaletteRegionMask[color] = 0;
			if (_paletteMask[kSceneColorToItemMap + color] == 8 ||
					_paletteMask[kSceneColorToItemMap + color] == 9)
				_paletteMask[kSceneColorToItemMap + color] = 0;
		}
	}

	if (state.scene2070HiddenItemPatchState == 0) {
		setColorMapItem(7, 0);
		if (state.scene2070InnerPassagePatchState != 0 && _sceneChunkTable.isValidChunk(9))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	if (state.scene2070SealExitPatchState == 0) {
		_hotspots.setVerbActionHandlerByGlobalRecordIndex(kScene2070ExitVerbRecordIndex, 1);
	} else {
		ScenePoint point;
		point.x = 0x30e;
		point.y = 0x0ee;
		_hotspots.setActionTarget(5, point, point);
		_hotspots.setVerbActionHandlerByGlobalRecordIndex(kScene2070ExitVerbRecordIndex, 0x133);
	}

	return true;
}

bool Scene2070::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void Scene2070::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

AmbientAudioProfile Scene2070::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2070::resetForegroundLayer() {
	_foregroundChannel.reset(0, kScene2070ForegroundFrameMillis);
	_foregroundLayer.visible = true;
	_foregroundLayer.reset(0);
}

void Scene2070::advanceForegroundLayer(uint32 delta) {
	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		byte nextFrame = (byte)(_foregroundLayer.frameIndex + 1);
		if (nextFrame >= ARRAYSIZE(kScene2070ForegroundFrameMap))
			nextFrame = 0;
		_foregroundLayer.setFrame(nextFrame);
	}
}

void Scene2070::runEntryFromLabyrinth() {
	setActiveActorPose(0x04b, 0x110, 2);
	_foregroundLayer.setFrame(0);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;

	GameplayState &state = _vm->gameState();
	if (state.egyptSealPuzzleProgress == 1) {
		if (!runSealMemoryEffect())
			return;
		state.egyptSealPuzzleProgress = 2;
	}

	beginSecondarySpeechLine(1, 0);
	runEntryPath(0x04b, 0x110, 2, 0x0ef, 0x12c);

	if (state.scene2070EntryProgress == 0) {
		beginSecondarySpeechLine(0, 0);
		state.scene2070EntryProgress = 1;
	}
}

void Scene2070::runEntryFromRightPassage() {
	setActiveActorPose(0x30e, 0x0ee, 4);
	_foregroundLayer.setFrame(0);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;

	GameplayState &state = _vm->gameState();
	if (state.scene2070EntryProgress < 2) {
		runEntryPathWithFinalFacing(0x30e, 0x0ee, 4, 0x2b7, 0x13f, 4, 0);
		beginStaticSecondarySpeechLine(0xd0, 0);
		state.scene2070EntryProgress = 2;
		return;
	}

	runEntryPath(0x30e, 0x0ee, 4, 0x24d, 0x130);
}

void Scene2070::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel) {
	setActiveActorPose(startX, startY, startFacing);

	drawPlayableComposite();
	presentFrame();
	walkActiveActorTo(targetX, targetY, finalFacing, finalCel, false);
	_activeActorFacing = finalFacing;
	_activeActorCel = finalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

bool Scene2070::runSealMemoryEffect() {
	drawPlayableComposite();
	presentFrame();
	_soundBank0.playSampleLooping(kScene2070SealMemoryLoopSound, 50);
	_sealMemoryActive = true;
	_sealMemoryFrame = 0;

	Scene2070DeltaFrameTarget target(_sealMemoryFrame);
	AnimationFrameRange range(kScene2070SealMemoryFrameMap, kScene2070MemoryFrameMillis);
	range.unskippable().hookAt(kScene2070SealMemoryEndSoundFrame,
		kScene2070SealMemoryEndSoundHook);
	const bool completed = playAnimationFrames(target, range);
	_sealMemoryActive = false;
	if (!completed) {
		_soundBank0.stop();
		drawPlayableComposite();
		presentFrame();
		return false;
	}

	GameplayState &state = _vm->gameState();
	state.scene2070SealExitPatchState = 1;
	applySceneStateToHotspotsAndPatches(1);
	drawPlayableComposite();
	presentFrame();
	return true;
}

void Scene2070::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene2070SealMemoryEndSoundHook && _sealMemoryActive &&
			frame == kScene2070SealMemoryEndSoundFrame)
		_soundBank0.playSample(kScene2070SealMemoryEndSound, 50);
}

void Scene2070::drawSealMemoryDeltaLayer() {
	const uint lastFrame = MIN<uint>(_sealMemoryFrame, kScene2070MemoryDescriptorCount - 1);
	// The resource frames are cumulative patches over the rebuilt scene.
	for (uint frame = 0; frame <= lastFrame; ++frame)
		drawClipFrameDelta(8, kScene2070MemoryDescriptorCount, (byte)frame);
}

void Scene2070::runAnimatedInventoryStateChange() {
	beginSecondarySpeechLine(10, 0);
	runActorReplacement(10, kScene2070InventoryOverlayDescriptorCount,
		kScene2070InventoryOverlayFrameMap, ARRAYSIZE(kScene2070InventoryOverlayFrameMap),
		kScene2070OverlayFrameMillis);

	addInventoryItem(0x2d);
	_soundBank0.playSample(1, 100);
	_vm->gameState().scene2070HiddenItemPatchState = 0;
	applySceneStateToHotspotsAndPatches(3);
}

void Scene2070::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 2 || _walkablePaletteMask[i] == 3)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene2070::setColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == sourceItem)
			_paletteMask[kSceneColorToItemMap + color] = destinationItem;
	}
}

} // End of namespace Hollywood
