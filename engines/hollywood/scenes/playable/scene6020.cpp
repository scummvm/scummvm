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

#include "hollywood/scenes/playable/scene6020.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene6020EntryFromScene6030State = 0x1785;
const uint16 kScene6010ReturnFromScene6020State = 0x177b;
const uint16 kScene6030EntryState = 0x178e;
const uint16 kScene6020ViewportXOffset = 0x0090;
const uint16 kScene6020ViewportMinXOffset = 0x0090;
const uint16 kScene6020ViewportMaxXOffset = 0x00a0;
const uint kScene6020ActorBankTableEntry = 0x0000;
const uint kScene6020ActorPaletteTableEntry = 0x00cc;
const uint kScene6020Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6020SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6020FrameMillis = 75;
const uint kScene6020Chunk8DescriptorCount = 6;
const uint kScene6020Chunk9DescriptorCount = 0x0c;
const uint kScene6020Chunk10DescriptorCount = 0x0c;
const uint kScene6020Chunk11DescriptorCount = 0x0c;
const uint kScene6020TaffyDescriptorCount = 0x56;
const uint kScene6020Chunk15DescriptorCount = 0x0c;
const uint kScene6020Chunk16DescriptorCount = 6;
const uint kScene6020RatHandoffChunkIndex = 0x16;
const uint kScene6020RatHandoffDescriptorCount = 9;
const uint kScene6020WalkOffChunkIndex = 0x17;
const uint kScene6020WalkOffDescriptorCount = 0x34;
const byte kScene6020TaffySpeechBaseFrame = 0x10;
const byte kScene6020TaffySpeechGroup2BaseFrame = 0x54;
const byte kScene6020TaffySpeechGroup3BaseFrame = 0x5d;
const byte kScene6020TaffySpeechGroup4BaseFrame = 0x82;
const uint32 kScene6020TaffyFrameMillis = 75;
const uint kScene6020DeskMagnifierVisibleChunk = 13;
const uint kScene6020DeskMagnifierHiddenChunk = 14;
const byte kScene6020TaffyHotspotItem = 5;
const byte kScene6020DeskMagnifierHotspotItem = 6;
const byte kScene6020MagnifierInventoryItem = 0x5a;
const byte kScene6020TaffyDialogueStageId = 0x62;
const byte kScene6020TaffyDialoguePrimaryRow = 99;
const uint kScene6020TaffyDialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene6020DialogueTransitionEnd = 0;
const byte kScene6020DialogueTransitionDown = 1;
const byte kScene6020DialogueTransitionUp = 2;
const byte kScene6020DialogueTransitionStay = 3;
const byte kScene6020DialogueTransitionUpTwo = 4;
const byte kScene6020DialogueNoResponseFrame = 0xff;

const byte kScene6020SmallObjectFrameMap[] = {
	0, 1, 2, 3, 4, 5, 4, 3, 4, 5, 4, 3, 2, 1, 0
};

const byte kScene6020PickupForwardFrameMap[] = {
	0x0b, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0x0b
};

const byte kScene6020PickupShortFrameMap[] = {
	0x0b, 0, 1, 2, 3, 2, 1, 0, 0x0b
};

const byte kScene6020PickupReverseFrameMap[] = {
	0x0b, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0x0b
};

const byte kScene6020LateSceneObjectFrameMap[] = {
	0, 0, 1, 2, 3, 4, 3, 2, 3, 4, 3, 2, 3, 4, 5
};

const byte kScene6020TaffyFrameMap[] = {
	0, 1, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0, 9, 10, 11, 12,
	12, 13, 14, 15, 16, 11, 10, 9, 0, 9, 10, 11, 12, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 22, 21, 20, 18, 17, 12, 11,
	10, 9, 0, 21, 20, 29, 30, 31, 32, 33, 34, 35, 36, 37, 36, 35,
	34, 33, 32, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 0,
	6, 7, 8, 81, 81, 82, 83, 84, 85, 8, 7, 6, 0, 56, 57, 58,
	58, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
	0, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 54, 53,
	52, 51, 76, 77, 78, 79, 80
};

const byte kScene6020RatHandoffObjectFrames[] = {
	7, 7, 6, 6, 6, 6, 6, 5, 4, 3, 2, 1, 0,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

const byte kScene6020RatHandoffTaffyFrames[] = {
	0x71, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81,
	0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81
};

const byte kScene6020TaffyLookUpFrames[] = {
	0x0c, 0x0d, 0x0e, 0x0f
};

const byte kScene6020TaffyExitIntroFrames[] = {
	0x15, 0x15, 0x16, 0x17, 0x18, 0x50, 0x51, 0x52, 0x53
};

const byte kScene6020TaffyExitOutroFrames[] = {
	0x59, 0x5a, 0x5b, 0x5c
};

static PlayableSceneConfig scene6020Config() {
	PlayableSceneConfig config(6020,
		SceneResourceLayout(21, 5, 20),
		SceneViewport(kScene6020ViewportXOffset, kScene6020ViewportMinXOffset, kScene6020ViewportMaxXOffset),
		SceneActorPose(0x2d4, 0x1a8, 4));
	config.setActorResources(kScene6020ActorBankTableEntry, kScene6020ActorPaletteTableEntry);
	config.setTextResources(kScene6020Resource003RowsOffsetIndex, kScene6020SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene6020::Scene6020(HollywoodEngine *vm) :
		PlayableScene(vm, scene6020Config()),
		_originalColorToItemMap(),
		_taffyChannel(),
		_taffyLayer(),
		_taffyAnimationState(0),
		_taffyHoldCounter(0),
		_taffyDeskMagnifierHidden(true),
		_taffyDepartureResource(),
		_taffyWalkOffResourceOffset(0),
		_taffyDepartureResourceLoaded(false),
		_taffyDepartureAnimationActive(false),
		_taffyDepartureFrameVisible(false),
		_taffyDepartureFrameBaseOffset(0),
		_taffyDepartureFrameDescriptorCount(0),
		_taffyDepartureFrameDescriptorIndex(0) {
	_taffyLayer.configure(12, kScene6020TaffyDescriptorCount,
		kScene6020TaffyFrameMap, ARRAYSIZE(kScene6020TaffyFrameMap));
}

void Scene6020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetTaffyLayer();
	if (_vm->gameState().mainFlowStateId == kScene6020EntryFromScene6030State) {
		_activeActorWorldX = 0x0ce;
		_activeActorWorldY = 0x17c;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x2d4;
		_activeActorWorldY = 0x1a8;
		_activeActorFacing = 4;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene6020::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_taffyLayer);
	drawTaffyForegroundBlock();
	drawTaffyDepartureLayer();
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldX, activeWorldY);
}

void Scene6020::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene6020EntryFromScene6030State)
		runEntryFromScene6030();
	else
		runEntryFromScene6010();
}

bool Scene6020::prepareCustomGameplayLoop() {
	resetTaffyLayer();
	return true;
}

bool Scene6020::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceTaffyLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene6020::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar puerta (look at door).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar/Abrir puerta (use/open door): locked.
		runSmallObjectAnimation();
		return true;
	case 303: // Ir a exterior (go outside): return to scene 6010.
		runExitToScene6010();
		return true;
	case 304: // Mirar archivador (look at filing cabinet).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Usar/Abrir archivador (use/open filing cabinet).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar maquina de cafe (look at coffee machine).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Usar maquina de cafe (use coffee machine).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Hablar con secretaria/Taffy (talk to secretary/Taffy).
		runDialogueAndMaybeEnterScene6030();
		return true;
	case 309: // Mirar secretaria/Taffy (look at secretary/Taffy).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Coger lupa (take magnifying glass).
		runPickupItem5A();
		return true;
	case 311: // Mirar lupa (look at magnifying glass).
		beginSecondarySpeechLine(9, hasInventoryItem(kScene6020MagnifierInventoryItem) ? 1 : 0);
		return true;
	case 312: // Coger paraguas (take umbrella).
		runPickupItem5B();
		return true;
	case 313: // Mirar paraguas (look at umbrella).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314: // Coger bolso (take purse/bag).
		runPickupItem5E();
		return true;
	case 315: // Mirar bolso (look at purse/bag).
		beginSecondarySpeechLine(13, _vm->gameState().scene6020TaffyKnown ? 1 : 0);
		return true;
	case 316: // Usar margarita con maquina de cafe (use daisy with coffee machine).
		runUseItem39Overlay();
		return true;
	case 317: // Dar/usar rata con secretaria/Taffy (give/use rat with secretary/Taffy).
		runUseItem64Overlay();
		return true;
	case 318: // Coger agenda (take notebook/address book).
		beginSecondarySpeechLine(17, 0);
		return true;
	case 319: // Mirar agenda (look at notebook/address book).
		runLateSceneObjectAnimation();
		return true;
	case 320: // Mirar telefono (look at telephone).
		beginSecondarySpeechLine(19, 0);
		return true;
	case 321: // Usar telefono (use telephone).
		runFinalSceneObjectAnimation();
		return true;
	default:
		return false;
	}
}

bool Scene6020::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	rememberOriginalColorMap();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	applyTaffyDeskMagnifierPatch();
	if (_vm->gameState().scene6020TaffyLeft)
		applyTaffyLeftSceneStatePatch();

	if (hasInventoryItem(0x5b)) {
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		replaceColorMapItemFromOriginal(7, 0);
	}

	if (hasInventoryItem(0x5e)) {
		if (_sceneChunkTable.isValidChunk(7))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
		replaceColorMapItemFromOriginal(8, 0);
	}

	applyTaffyKnownSceneLabel();
	rebuildWorkingWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

bool Scene6020::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene6020::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	switch (animationGroup) {
	case 2:
		return kScene6020TaffySpeechGroup2BaseFrame;
	case 3:
		return kScene6020TaffySpeechGroup3BaseFrame;
	case 4:
		return kScene6020TaffySpeechGroup4BaseFrame;
	default:
		return kScene6020TaffySpeechBaseFrame;
	}
}

void Scene6020::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_taffyLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene6020::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene6020::resetTaffyLayer() {
	_taffyChannel.reset(0, kScene6020TaffyFrameMillis);
	_taffyLayer.reset(0);
	_taffyLayer.visible = !_vm->gameState().scene6020TaffyLeft;
	_taffyAnimationState = 0;
	_taffyHoldCounter = 0;
	_taffyDeskMagnifierHidden = !_vm->gameState().scene6020TaffyLeft ||
		hasInventoryItem(kScene6020MagnifierInventoryItem);
	_taffyDepartureAnimationActive = false;
	_taffyDepartureFrameVisible = false;
}

void Scene6020::advanceTaffyLayer(uint32 delta) {
	if (!_taffyLayer.visible || _vm->gameState().scene6020TaffyLeft || _taffyDepartureAnimationActive)
		return;

	const uint frameCount = _taffyChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_taffyAnimationState == 0) {
			if (_taffyLayer.frameIndex == 0) {
				if (_random.getRandomNumber(14) == 0) {
					_taffyLayer.setFrame(1);
				} else if (_random.getRandomNumber(29) == 0) {
					_taffyAnimationState = 1;
					_taffyHoldCounter = (byte)(_random.getRandomNumber(9) + 2);
					_taffyLayer.setFrame(2);
				} else if (_random.getRandomNumber(49) == 0) {
					_taffyAnimationState = 2;
					_taffyLayer.setFrame(0x19);
				}
			} else {
				_taffyLayer.setFrame(0);
			}
		} else if (_taffyAnimationState == 1) {
			if (_taffyLayer.frameIndex < 6) {
				_taffyLayer.setFrame(_taffyLayer.frameIndex + 1);
			} else if (_taffyLayer.frameIndex == 6 && _taffyHoldCounter != 0) {
				--_taffyHoldCounter;
			} else if (_taffyLayer.frameIndex < 0x0b) {
				_taffyLayer.setFrame(_taffyLayer.frameIndex + 1);
			} else {
				_taffyLayer.setFrame(0);
				_taffyAnimationState = 0;
			}
		} else if (_taffyAnimationState == 2) {
			if (_taffyLayer.frameIndex > 0x21) {
				_taffyAnimationState = 3;
				_taffyHoldCounter = (byte)(_random.getRandomNumber(4) + 1);
				_taffyLayer.setFrame(0x23);
			} else {
				const byte nextFrame = _taffyLayer.frameIndex + 1;
				if (nextFrame == 0x1f)
					setTaffyDeskMagnifierHidden(false);
				_taffyLayer.setFrame(nextFrame);
			}
		} else if (_taffyAnimationState == 3) {
			if (_taffyLayer.frameIndex < 0x29) {
				_taffyLayer.setFrame(_taffyLayer.frameIndex + 1);
			} else if (_taffyHoldCounter != 0) {
				--_taffyHoldCounter;
				_taffyLayer.setFrame(0x23);
			} else if (!_taffyDeskMagnifierHidden) {
				_taffyAnimationState = 4;
				_taffyLayer.setFrame(0x2a);
			} else {
				_taffyAnimationState = 5;
				_taffyLayer.setFrame(0x33);
			}
		} else if (_taffyAnimationState == 4) {
			if (_taffyLayer.frameIndex > 0x31) {
				_taffyLayer.setFrame(0);
				_taffyAnimationState = 0;
			} else {
				const byte nextFrame = _taffyLayer.frameIndex + 1;
				_taffyLayer.setFrame(nextFrame);
				if (nextFrame == 0x2c)
					setTaffyDeskMagnifierHidden(true);
			}
		} else if (_taffyAnimationState == 5) {
			if (_taffyLayer.frameIndex < 0x4f) {
				_taffyLayer.setFrame(_taffyLayer.frameIndex + 1);
			} else {
				_taffyLayer.setFrame(0);
				_taffyAnimationState = 0;
				setTaffyDeskMagnifierHidden(true);
			}
		}
	}
}

void Scene6020::drawTaffyForegroundBlock() {
	if (!_taffyLayer.visible || !_sceneChunkTable.isValidChunk(19))
		return;

	const byte descriptor = (byte)_taffyLayer.descriptorIndex();
	if (descriptor == 8 || (descriptor > 0x2c && descriptor < 0x38) ||
			(descriptor > 0x4b && descriptor < 0x56))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[19], _sceneFramebuffer);
}

void Scene6020::drawForegroundBlocks(int activeWorldX, int activeWorldY) {
	if (activeWorldY < 0x18a && activeWorldX < 0x1c3 && _sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene6020::setTaffyDeskMagnifierHidden(bool hidden) {
	if (!hidden && hasInventoryItem(kScene6020MagnifierInventoryItem))
		hidden = true;

	if (_taffyDeskMagnifierHidden == hidden)
		return;

	_taffyDeskMagnifierHidden = hidden;
	applyTaffyDeskMagnifierPatch();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
}

void Scene6020::applyTaffyDeskMagnifierPatch() {
	const bool itemTaken = hasInventoryItem(kScene6020MagnifierInventoryItem);
	const bool hidden = itemTaken || _taffyDeskMagnifierHidden;
	const uint chunkIndex = hidden ? kScene6020DeskMagnifierHiddenChunk : kScene6020DeskMagnifierVisibleChunk;
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _baseFramebuffer);

	replaceColorMapItemFromOriginal(kScene6020DeskMagnifierHotspotItem,
		hidden ? 0 : kScene6020DeskMagnifierHotspotItem);
}

void Scene6020::applyTaffyLeftSceneStatePatch() {
	replaceColorMapItemFromOriginal(kScene6020TaffyHotspotItem, 0);
	replaceColorMapItemFromOriginal(kScene6020DeskMagnifierHotspotItem, 0);
}

bool Scene6020::loadTaffyDepartureResource() {
	if (_taffyDepartureResourceLoaded)
		return true;

	Common::Array<byte> ratHandoffChunk;
	Common::Array<byte> walkOffChunk;
	if (!_resources.loadVariableChunk(kScene6020RatHandoffChunkIndex, ratHandoffChunk) ||
			!_resources.loadVariableChunk(kScene6020WalkOffChunkIndex, walkOffChunk))
		return false;

	_taffyDepartureResource.resize(ratHandoffChunk.size() + walkOffChunk.size());
	if (!ratHandoffChunk.empty())
		memcpy(_taffyDepartureResource.data(), ratHandoffChunk.data(), ratHandoffChunk.size());
	_taffyWalkOffResourceOffset = ratHandoffChunk.size();
	if (!walkOffChunk.empty())
		memcpy(_taffyDepartureResource.data() + _taffyWalkOffResourceOffset,
			walkOffChunk.data(), walkOffChunk.size());

	_taffyDepartureResourceLoaded = true;
	return true;
}

void Scene6020::setTaffyDepartureFrame(uint32 baseOffset, uint16 descriptorCount, uint16 descriptorIndex) {
	_taffyDepartureFrameVisible = true;
	_taffyDepartureFrameBaseOffset = baseOffset;
	_taffyDepartureFrameDescriptorCount = descriptorCount;
	_taffyDepartureFrameDescriptorIndex = descriptorIndex;
}

void Scene6020::clearTaffyDepartureFrame() {
	_taffyDepartureFrameVisible = false;
}

void Scene6020::drawTaffyDepartureLayer() {
	if (!_taffyDepartureFrameVisible || _taffyDepartureResource.empty())
		return;

	drawStripSpriteFrame(_taffyDepartureResource, _taffyDepartureFrameBaseOffset, 0,
		_taffyDepartureFrameDescriptorCount, _taffyDepartureFrameDescriptorIndex, _sceneFramebuffer);
}

void Scene6020::rememberOriginalColorMap() {
	if (!_originalColorToItemMap.empty() ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	_originalColorToItemMap.resize(kScenePaletteMapPageSize);
	memcpy(_originalColorToItemMap.data(), _paletteMaskOriginal.data() + kSceneColorToItemMap,
		_originalColorToItemMap.size());
}

void Scene6020::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_originalColorToItemMap.size() < kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_originalColorToItemMap[i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene6020::rebuildWorkingWalkableMask() {
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 4)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene6020::applyTaffyKnownSceneLabel() {
	if (!_vm->gameState().scene6020TaffyKnown || _vm->gameState().scene6020TaffyLeft)
		return;

	const uint secretaryRowOffset = 5 * kStage003SmallRowSize;
	const uint taffyRowOffset = 11 * kStage003SmallRowSize;
	if (secretaryRowOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			taffyRowOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + secretaryRowOffset,
		_stage003SmallRows.data() + taffyRowOffset, kStage003SmallRowSize);
}

void Scene6020::runEntryFromScene6010() {
	_soundBank0.playSample(4, 100);
	runEntryPath(0x3a0, 0x1c6, 4, 0x2d4, 0x1a8);
}

void Scene6020::runEntryFromScene6030() {
	_soundBank0.playSample(4, 100);
	runEntryPath(0x0ce, 0x17c, 2, 0x0ce, 0x17c);
}

void Scene6020::runSmallObjectAnimation() {
	runActorReplacement(ActionOverlaySpec(8, kScene6020Chunk8DescriptorCount,
		kScene6020SmallObjectFrameMap, ARRAYSIZE(kScene6020SmallObjectFrameMap), kScene6020FrameMillis));
	beginSecondarySpeechLine(2, 0);
}

void Scene6020::runPickupItem5A() {
	if (hasInventoryItem(kScene6020MagnifierInventoryItem)) {
		beginSecondarySpeechLine(8, 1);
		return;
	}

	if (_taffyAnimationState != 3)
		return;

	runActorReplacement(ActionOverlaySpec(15, kScene6020Chunk15DescriptorCount,
		kScene6020PickupForwardFrameMap, ARRAYSIZE(kScene6020PickupForwardFrameMap), kScene6020FrameMillis)
		.patchAt(6, 2));
	_taffyDeskMagnifierHidden = true;
	addInventoryItem(kScene6020MagnifierInventoryItem);
	applySceneStateToHotspotsAndPatches(2);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(8, 0);
}

void Scene6020::runPickupItem5B() {
	if (hasInventoryItem(0x5b)) {
		beginSecondarySpeechLine(10, 3);
		return;
	}

	runActorReplacement(ActionOverlaySpec(10, kScene6020Chunk10DescriptorCount,
		kScene6020PickupForwardFrameMap, ARRAYSIZE(kScene6020PickupForwardFrameMap), kScene6020FrameMillis)
		.patchAt(4, 3));
	addInventoryItem(0x5b);
	applySceneStateToHotspotsAndPatches(3);
	_soundBank0.playSample(1, 100);
	beginPrimarySpeechLine(10, 0, 499, 0xbd, 0x2a, 0x3f, 0x0e);
	beginSecondarySpeechLine(10, 1);
	beginPrimarySpeechLine(10, 2, 499, 0xbd, 0x2a, 0x3f, 0x0e);
	beginSecondarySpeechLine(10, 3);
}

void Scene6020::runPickupItem5E() {
	if (hasInventoryItem(0x5e)) {
		beginSecondarySpeechLine(12, 3);
		return;
	}

	if (!_vm->gameState().scene6020TaffyLeft) {
		runActorReplacement(ActionOverlaySpec(9, kScene6020Chunk9DescriptorCount,
			kScene6020PickupShortFrameMap, ARRAYSIZE(kScene6020PickupShortFrameMap), kScene6020FrameMillis));
		beginPrimarySpeechLine(12, 0, 499, 0xbd, 0x2a, 0x3f, 0x0e);
		beginSecondarySpeechLine(12, 1);
		beginPrimarySpeechLine(12, 2, 499, 0xbd, 0x2a, 0x3f, 0x0e);
		return;
	}

	runActorReplacement(ActionOverlaySpec(9, kScene6020Chunk9DescriptorCount,
		kScene6020PickupForwardFrameMap, ARRAYSIZE(kScene6020PickupForwardFrameMap), kScene6020FrameMillis)
		.patchAt(6, 4));
	addInventoryItem(0x5e);
	applySceneStateToHotspotsAndPatches(4);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(12, 3);
}

void Scene6020::runUseItem39Overlay() {
	if (!hasInventoryItem(0x39) || !_vm->gameState().scene6030HannoverInterviewCompleted) {
		dispatchGenericSceneAction(0xe7);
		return;
	}

	runActorReplacement(ActionOverlaySpec(11, kScene6020Chunk11DescriptorCount,
		kScene6020PickupReverseFrameMap, ARRAYSIZE(kScene6020PickupReverseFrameMap), kScene6020FrameMillis));
	removeInventoryItem(0x39);
	_vm->gameState().scene6030CoffeeState = 1;
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(15, 0);
}

void Scene6020::runUseItem64Overlay() {
	if (!hasInventoryItem(0x64)) {
		beginSecondarySpeechLine(16, 0);
		return;
	}

	walkActiveActorTo(0x25b, 0x17e, 5, 0, false);
	beginSecondarySpeechLine(16, 0);
	runTaffyRatHandoffAnimation();
	removeInventoryItem(0x64);
	_soundBank0.playSample(1, 100);
	beginPrimarySpeechLineWithAnimationGroup(16, 1, 499, 0xbd, 0x2a, 0x3f, 0x0e, 4);
	runTaffyWalkOffAnimation();
	_vm->gameState().scene6020TaffyLeft = true;
	clearTaffyDepartureFrame();
	_taffyLayer.visible = false;
	_taffyAnimationState = 0;
	_taffyDeskMagnifierHidden = hasInventoryItem(kScene6020MagnifierInventoryItem);
	applySceneStateToHotspotsAndPatches(1);
	drawPlayableComposite();
	presentFrame();
	walkActiveActorTo(0x262, 0x184, 4, 0, false);
	beginSecondarySpeechLine(16, 2);
}

void Scene6020::runTaffyRatHandoffAnimation() {
	if (!loadTaffyDepartureResource())
		return;

	_taffyDepartureAnimationActive = true;
	_taffyLayer.visible = true;
	for (uint frame = 0; frame < ARRAYSIZE(kScene6020RatHandoffObjectFrames) && !Engine::shouldQuit(); ++frame) {
		_taffyLayer.setFrame(kScene6020RatHandoffTaffyFrames[frame]);
		setTaffyDepartureFrame(0, kScene6020RatHandoffDescriptorCount,
			kScene6020RatHandoffObjectFrames[frame]);
		if (frame == 3)
			setTaffyDeskMagnifierHidden(false);
		if (waitSceneMillis(kScene6020FrameMillis))
			break;
	}
	_taffyDepartureAnimationActive = false;
}

void Scene6020::runTaffyWalkOffAnimation() {
	if (!loadTaffyDepartureResource())
		return;

	_taffyDepartureAnimationActive = true;
	_taffyLayer.visible = false;
	for (uint frame = 0; frame < kScene6020WalkOffDescriptorCount && !Engine::shouldQuit(); ++frame) {
		setTaffyDepartureFrame(_taffyWalkOffResourceOffset, kScene6020WalkOffDescriptorCount, frame);
		if (waitSceneMillis(kScene6020FrameMillis))
			break;
	}
	_taffyDepartureAnimationActive = false;
}

void Scene6020::runLateSceneObjectAnimation() {
	if (!hasInventoryItem(0x5a)) {
		beginSecondarySpeechLine(18, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(16, kScene6020Chunk16DescriptorCount,
		kScene6020LateSceneObjectFrameMap, ARRAYSIZE(kScene6020LateSceneObjectFrameMap), kScene6020FrameMillis));
	beginSecondarySpeechLine(18, 1);
}

void Scene6020::runFinalSceneObjectAnimation() {
	beginSecondarySpeechLine(20, 0);
}

void Scene6020::runDialogueAndMaybeEnterScene6030() {
	Common::Array<DialogueChoiceRecord> records;
	initializeTaffyDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	const bool firstConversation = !_vm->gameState().scene6020TaffyKnown;
	beginSecondarySpeechLine(kScene6020TaffyDialogueStageId, firstConversation ? 0 : 1);
	runTaffyLookUpTransition();
	beginPrimarySpeechLine(kScene6020TaffyDialoguePrimaryRow, firstConversation ? 0 : 1,
		499, 0xbd, 0x2a, 0x3f, 0x0e);
	if (firstConversation) {
		_vm->gameState().scene6020TaffyKnown = true;
		applySceneStateToHotspotsAndPatches(1);
	}

	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene6020TaffyDialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene6020TaffyDialogueStageId, 6);
			beginPrimarySpeechLine(kScene6020TaffyDialoguePrimaryRow, 6,
				499, 0xbd, 0x2a, 0x3f, 0x0e);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene6020TaffyDialogueStageId, record.playerTextRowId);
		if (record.disableAfterUse == 3) {
			runExitToScene6030();
			return;
		}

		if (record.responseFrameIndex != kScene6020DialogueNoResponseFrame)
			beginPrimarySpeechLine(kScene6020TaffyDialoguePrimaryRow, record.responseFrameIndex,
				499, 0xbd, 0x2a, 0x3f, 0x0e);

		if (record.disableAfterUse != 0)
			record.enabled = 0;

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case kScene6020DialogueTransitionEnd:
			finished = true;
			break;
		case kScene6020DialogueTransitionDown:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case kScene6020DialogueTransitionUp:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 1;
			break;
		case kScene6020DialogueTransitionUpTwo:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 2;
			break;
		case kScene6020DialogueTransitionStay:
		default:
			break;
		}
	}
}

void Scene6020::initializeTaffyDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene6020TaffyDialogueChoiceRecordCount);

	// DAT_0050bb30: root choices for Taffy. Choice 0 enters Hannover's office while the original
	// one-shot late-scene guard is still clear.
	setTaffyDialogueRecord(records, 0, 1, 0, kScene6020DialogueTransitionStay, 2, 2, 3);
	setTaffyDialogueRecord(records, 1, 1, 0, kScene6020DialogueTransitionDown, 3, 3, 1);
	setTaffyDialogueRecord(records, 2, 1, 0, kScene6020DialogueTransitionStay, 4, 4, 1);
	setTaffyDialogueRecord(records, 3, 1, 0, kScene6020DialogueTransitionStay, 5, 5, 1);
	setTaffyDialogueRecord(records, 4, 1, 0, kScene6020DialogueTransitionEnd, 6, 6, 1);

	// Depth 1, node 0: follow-up choices opened by root choice 1.
	setTaffyDialogueRecord(records, 70, 1, 0, kScene6020DialogueTransitionStay, 7, 7, 1);
	setTaffyDialogueRecord(records, 71, 1, 0, kScene6020DialogueTransitionStay, 8, 8, 1);
	setTaffyDialogueRecord(records, 72, 1, 0, kScene6020DialogueTransitionUp, 9, 9, 1);
}

void Scene6020::setTaffyDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = enabled;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = 0xff;
}

void Scene6020::runTaffyLookUpTransition() {
	runTaffyFrameSequence(kScene6020TaffyLookUpFrames, ARRAYSIZE(kScene6020TaffyLookUpFrames));
}

void Scene6020::runTaffyFrameSequence(const byte *frames, uint frameCount) {
	if (!frames || frameCount == 0 || !_taffyLayer.visible || _vm->gameState().scene6020TaffyLeft)
		return;

	const bool previousManualAnimation = _taffyDepartureAnimationActive;
	_taffyDepartureAnimationActive = true;
	for (uint i = 0; i < frameCount && !Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++i) {
		_taffyLayer.setFrame(frames[i]);
		if (waitSceneMillis(kScene6020FrameMillis))
			break;
	}
	_taffyDepartureAnimationActive = previousManualAnimation;
}

void Scene6020::runExitToScene6010() {
	_vm->gameState().mainFlowStateId = kScene6010ReturnFromScene6020State;
}

void Scene6020::runExitToScene6030() {
	runTaffyFrameSequence(kScene6020TaffyExitIntroFrames, ARRAYSIZE(kScene6020TaffyExitIntroFrames));
	beginPrimarySpeechLineWithAnimationGroup(0x0e, 0, 499, 0xbd, 0x2a, 0x3f, 0x0e, 2);
	beginPrimarySpeechLineWithAnimationGroup(0x0e, 1, 0x215, 0x10e, 0x2a, 0x0e, 0x3f, 2);
	beginSecondarySpeechLine(0x0e, 2);
	runTaffyFrameSequence(kScene6020TaffyExitOutroFrames, ARRAYSIZE(kScene6020TaffyExitOutroFrames));
	walkActiveActorTo(0x0d3, 0x17b, 5, 0, false);
	runActorReplacement(ActionOverlaySpec(8, kScene6020Chunk8DescriptorCount,
		kScene6020SmallObjectFrameMap, ARRAYSIZE(kScene6020SmallObjectFrameMap), kScene6020FrameMillis)
		.soundAt(ARRAYSIZE(kScene6020SmallObjectFrameMap) - 1, 3));
	_vm->gameState().mainFlowStateId = kScene6030EntryState;
}

} // End of namespace Hollywood
