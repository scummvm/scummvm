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

#include "hollywood/scenes/playable/scene7040.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene7040Chunk10DescriptorCount = 5;
const uint16 kScene7040Chunk11DescriptorCount = 0x1f;
const uint16 kScene7040Chunk12DescriptorCount = 3;
const uint16 kScene7040Chunk13DescriptorCount = 0x1c;
const uint16 kScene7040Chunk14ActionDescriptorCount = 0x32;
const uint16 kScene7040Chunk14AltDescriptorCount = 0x38;
const uint16 kScene7040Chunk16DescriptorCount = 0x0b;
const uint16 kScene7040Chunk17DescriptorCount = 9;
const uint16 kScene7040Chunk18DescriptorCount = 0x0a;
const uint16 kScene7040State7041 = 0x1b81;
const uint16 kScene7040State7042 = 0x1b82;
const uint16 kScene7040ReturnState7031 = 0x1b77;
const uint16 kScene7040ExitState7050 = 0x1b8a;
const uint16 kScene7040ExitState7060 = 0x1b94;
const byte kScene7040Entry7040Facing = 1;
const int kScene7040Entry7040StartX = 100;
const int kScene7040Entry7040StartY = 0x1b1;
const int kScene7040Entry7040FirstTargetX = 0x14a;
const int kScene7040Entry7040FirstTargetY = 0x139;
const int kScene7040Entry7040RepeatTargetX = 0x16f;
const int kScene7040Entry7040RepeatTargetY = 0x177;
const byte kScene7040Entry7041Facing = 4;
const int kScene7040Entry7041StartX = 600;
const int kScene7040Entry7041StartY = 0x132;
const byte kScene7040Entry7042Facing = 4;
const int kScene7040Entry7042StartX = 0x322;
const int kScene7040Entry7042StartY = 0x1c9;
const int kScene7040Entry7042TargetX = 0x29e;
const int kScene7040Entry7042TargetY = 0x1cc;
const int kScene7040MajorHotspotOverlayEndX = 0x179;
const int kScene7040MajorHotspotOverlayEndY = 0x113;
const byte kScene7040MajorHotspotOverlayEndFacing = 5;
const byte kScene7040MajorHotspotReturnStartFacing = 4;
const int kScene7040MajorHotspotReturnX = 0x10d;
const int kScene7040MajorHotspotReturnY = 0x124;
const uint32 kScene7040Chunk11FrameMillis = 75;
const uint32 kScene7040Chunk14FrameMillis = 75;
const uint32 kScene7040Chunk16FrameMillis = 75;
const uint32 kScene7040Chunk17FrameMillis = 125;
const uint kScene7040ColorToItemMapOffset = 0x100;
const uint kScene7040ColorMapSize = 0x100;
const byte kScene7040InvalidFacing = 0xff;
const byte kScene7040DialogueStageId = 0x62;
const byte kScene7040DialoguePrimaryRow = 99;
const uint16 kScene7040DialoguePrimaryCenterX = 0x1c2;
const uint16 kScene7040DialoguePrimaryTopY = 0x73;
const byte kScene7040DialoguePrimaryRed = 0x3f;
const byte kScene7040DialoguePrimaryGreen = 0x32;
const byte kScene7040DialoguePrimaryBlue = 0x0c;
const byte kScene7040JosephNormalSpeechGroup = 0;
const byte kScene7040JosephRevealSpeechGroupA = 1;
const byte kScene7040JosephRevealSpeechGroupB = 2;
const byte kScene7040ActionSpeechGroupA = 3;
const byte kScene7040ActionSpeechGroupB = 4;
const byte kScene7040FrankieSpeechGroupA = 5;
const byte kScene7040FrankieSpeechGroupB = 6;
const byte kScene7040PrimarySpeechFrameCount = 5;
const byte kScene7040Chunk14ActionHook = 1;
const byte kScene7040Chunk14AltHook = 2;
const uint kScene7040Chunk17Layer = 0;
const uint kScene7040Chunk16Layer = 1;
const uint kScene7040Chunk12Layer = 2;
const uint kScene7040Chunk11Layer = 3;
const uint kScene7040Chunk14ActionLayer = 4;
const uint kScene7040Chunk14AltLayer = 5;
const byte kScene7040JosephNormalSpeechBaseFrame = 7;
const byte kScene7040JosephRevealSpeechBaseFrameA = 0x12;
const byte kScene7040JosephRevealSpeechBaseFrameB = 0x1a;
const byte kScene7040ActionSpeechBaseFrameA = 0x11;
const byte kScene7040ActionSpeechBaseFrameB = 0x6b;
const byte kScene7040FrankieSpeechBaseFrameA = 0x14;
const byte kScene7040FrankieSpeechBaseFrameB = 0x1c;
const uint kScene7040DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene7040Chunk11FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 1, 28, 29, 30, 0, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
	27, 1, 0, 0, 0, 0, 0, 0
};
const byte kScene7040Chunk14ActionFrameMap[] = {
	49, 49, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 14, 18, 19, 20, 21, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 34, 33, 36, 35, 34, 35, 36, 34, 33, 36, 35,
	33, 36, 35, 33, 34, 35, 36, 34, 33, 34, 35, 36, 34, 33, 36, 35,
	34, 33, 34, 35, 36, 34, 33, 36, 35, 34, 35, 36, 34, 33, 36, 35,
	33, 36, 35, 33, 34, 35, 36, 34, 33, 34, 35, 36, 34, 33, 36, 35,
	34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 21, 22, 23, 24, 21, 37,
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 0, 0, 0, 0
};
const byte kScene7040Chunk14AltFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	27, 26, 25, 24, 19, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 55, 55, 0
};
const byte kScene7040Chunk16PostItemFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 5, 4, 5, 6, 5, 4, 5, 6,
	5, 4, 5, 6, 5, 4, 5, 6, 5, 4, 3, 2, 1, 0, 7, 8,
	9, 10, 7, 0, 0, 0, 0, 0, 0, 0
};
const byte kScene7040MajorHotspotFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 6, 7, 8, 7, 6, 6, 7, 8,
	9, 8, 7, 6, 6, 7, 8, 9, 10, 9, 8, 7, 6, 6, 7, 8,
	9, 10, 11, 11, 11, 11, 12, 13, 14, 15, 16, 17, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27
};
const byte kScene7040Chunk10ExitFrameMap[] = { 0, 0, 1, 2, 3, 4 };
const byte kScene7040Chunk18PickupItem0FFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};
const SceneAnimationLayerSpec kScene7040AnimationLayerSpecs[] = {
	{ kSceneAnimationBehindActors, 17, kScene7040Chunk17DescriptorCount, nullptr, 0, false },
	{ kSceneAnimationBehindActors, 16, kScene7040Chunk16DescriptorCount,
		kScene7040Chunk16PostItemFrameMap,
		ARRAYSIZE(kScene7040Chunk16PostItemFrameMap), false },
	{ kSceneAnimationBehindActors, 12, kScene7040Chunk12DescriptorCount, nullptr, 0, false },
	{ kSceneAnimationBehindActors, 11, kScene7040Chunk11DescriptorCount, kScene7040Chunk11FrameMap,
		ARRAYSIZE(kScene7040Chunk11FrameMap), true },
	{ kSceneAnimationBehindActors, 14, kScene7040Chunk14ActionDescriptorCount,
		kScene7040Chunk14ActionFrameMap,
		ARRAYSIZE(kScene7040Chunk14ActionFrameMap), false },
	{ kSceneAnimationBehindActors, 14, kScene7040Chunk14AltDescriptorCount, kScene7040Chunk14AltFrameMap,
		ARRAYSIZE(kScene7040Chunk14AltFrameMap), false }
};

static PlayableSceneConfig scene7040Config() {
	PlayableSceneConfig config(7040,
		SceneResourceLayout(20, 5, 18),
		SceneViewport(0xc8),
		SceneActorPose(0x14a, 0x139, 1));
	return config;
}

Scene7040::Scene7040(HollywoodEngine *vm) :
		PlayableScene(vm, scene7040Config()),
		_postItemIdleState(0),
		_primarySpeechLeadInTicks(0),
		_primarySpeechLastMouthFrameOffset(0),
		_animationLayers() {
	_preItemIdleAnimation.configure(kScene7040Chunk11FrameMillis, 0, 1, 0, 6, 0x0e, 0x31);
	_postItemAnimation.reset(1, kScene7040Chunk16FrameMillis);
	_chunk17Animation.reset(0, kScene7040Chunk17FrameMillis);
}

int Scene7040::alternatePaletteResourceChunkIndex() const {
	return 19;
}

bool Scene7040::isAlternatePaletteResourceActive() const {
	return _vm->gameState().reviewedFrankensteinNote;
}

bool Scene7040::shouldConvertSavedFramebufferFF() const {
	return true;
}

bool Scene7040::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene7040::initializeCustomPreviewState() {
	_primaryLeftSpeechLastFrame = 0;
	_primaryDialogueSpeechLastFrame = 7;
	_actionOverlayPlayer.reset();
	_postItemIdleState = 0;
	_preItemIdleAnimation.reset();
	_postItemAnimation.reset(1, kScene7040Chunk16FrameMillis);
	_chunk17Animation.reset(0, kScene7040Chunk17FrameMillis);
	configureAnimationLayers();
	_primarySpeechLeadInTicks = 0;
	_primarySpeechLastMouthFrameOffset = 0;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_ambientMusicTimerAccumulator = 0;
	_secondaryActorTimerAccumulator = 0;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	setActiveActorPose(kScene7040Entry7040FirstTargetX, kScene7040Entry7040FirstTargetY, kScene7040Entry7040Facing);
	_secondaryActorFrame = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene7040::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();
	syncAnimationLayerFrames();
	if (_actionOverlayPlayer.replacesActor()) {
		drawResourceSpriteLayer(_animationLayers.layer(kScene7040Chunk12Layer));
		if (isAlternatePaletteResourceActive()) {
			drawResourceSpriteLayer(_animationLayers.layer(kScene7040Chunk17Layer));
			drawResourceSpriteLayer(_animationLayers.layer(kScene7040Chunk16Layer));
		}
		drawActionOverlayLayer();
		if (!isAlternatePaletteResourceActive())
			drawResourceSpriteLayer(_animationLayers.layer(kScene7040Chunk11Layer));
	} else {
		drawAnimationLayers(_animationLayers, kSceneAnimationBehindActors);
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}

	uint blockChunk = 5;
	if (actorDrawOrderMode == 2 || actorDrawOrderMode == 3) {
		blockChunk = activeWorldY <= 0x15f ? 6 : 0;
	} else if (actorDrawOrderMode == 6) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
		blockChunk = _vm->gameState().officeNotePickupState == 1 ? 9 : 0;
	}
	if (blockChunk != 0)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[blockChunk], _sceneFramebuffer);
}

void Scene7040::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene7040State7041) {
		_soundBank0.playSample(4, 100);
		runEntryPath(kScene7040Entry7041StartX, kScene7040Entry7041StartY, kScene7040Entry7041Facing,
			kScene7040Entry7041StartX, kScene7040Entry7041StartY);
		return;
	}

	if (state.mainFlowStateId == kScene7040State7042) {
		runEntryPath(kScene7040Entry7042StartX, kScene7040Entry7042StartY, kScene7040Entry7042Facing,
			kScene7040Entry7042TargetX, kScene7040Entry7042TargetY);
		return;
	}

	const bool seenJosephGuestListGreeting = state.seenJosephGuestListGreeting;
	const int targetX = seenJosephGuestListGreeting ?
		kScene7040Entry7040RepeatTargetX : kScene7040Entry7040FirstTargetX;
	const int targetY = seenJosephGuestListGreeting ?
		kScene7040Entry7040RepeatTargetY : kScene7040Entry7040FirstTargetY;
	runEntryPath(kScene7040Entry7040StartX, kScene7040Entry7040StartY,
		kScene7040Entry7040Facing, targetX, targetY);
	if (!seenJosephGuestListGreeting && !shouldStopJosephGuestListGreeting())
		runJosephGuestListGreeting();
}

void Scene7040::runJosephGuestListGreeting() {
	beginSecondarySpeechLine(kScene7040DialogueStageId, 6);
	if (shouldStopJosephGuestListGreeting())
		return;
	_preItemIdleAnimation.state = 2;
	waitPreItemIdleSequence();
	if (shouldStopJosephGuestListGreeting())
		return;
	_preItemIdleAnimation.state = 3;
	beginPrimarySpeechLine(kScene7040DialoguePrimaryRow, 6, kScene7040DialoguePrimaryCenterX,
		kScene7040DialoguePrimaryTopY, kScene7040DialoguePrimaryRed, kScene7040DialoguePrimaryGreen,
		kScene7040DialoguePrimaryBlue);
	if (shouldStopJosephGuestListGreeting())
		return;
	beginSecondarySpeechLine(kScene7040DialogueStageId, 7);
	if (shouldStopJosephGuestListGreeting())
		return;
	beginPrimarySpeechLine(kScene7040DialoguePrimaryRow, 7, kScene7040DialoguePrimaryCenterX,
		kScene7040DialoguePrimaryTopY, kScene7040DialoguePrimaryRed, kScene7040DialoguePrimaryGreen,
		kScene7040DialoguePrimaryBlue);
	if (shouldStopJosephGuestListGreeting())
		return;
	_preItemIdleAnimation.reset();
	_vm->gameState().seenJosephGuestListGreeting = true;
}

void Scene7040::waitPreItemIdleSequence() {
	while (_preItemIdleAnimation.state != 0 && _preItemIdleAnimation.state != 3 && !Engine::shouldQuit()) {
		if (waitSceneMillis(kScene7040Chunk11FrameMillis))
			break;
	}
}

bool Scene7040::shouldStopJosephGuestListGreeting() {
	if (!_skipRequested && !Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		return false;

	_preItemIdleAnimation.reset();
	return true;
}

bool Scene7040::prepareCustomGameplayLoop() {
	resetTransientAnimationLayers();
	return true;
}

bool Scene7040::advanceCustomGameplayLoop(uint32 delta) {
	if (_vm->gameState().reviewedFrankensteinNote)
		advanceChunk16PostItemAnimation(delta);
	else if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceChunk11PreItemIdleAnimation(delta);

	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene7040::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)restoredStepDeltas;
	if (currentRegion == 3 && nextRegion == 3 && state.x < boundary.x && boundary.y <= state.y) {
		requestedFacing = 1;
		return true;
	}
	return false;
}

bool Scene7040::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)restoredStepDeltas;
	if (currentRegion == 3 && targetRegion == 3 && state.x < targetX && targetY <= state.y) {
		requestedFacing = 1;
		return true;
	}
	return false;
}

bool Scene7040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0 || selector == 0xff) {
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());

		for (uint i = 0; i < _fullPaletteRegionMask.size(); ++i) {
			if (_paletteMaskOriginal[i] == 7)
				_fullPaletteRegionMask[i] = _vm->gameState().reviewedFrankensteinNote ? 0 : 1;
		}

		if (_paletteMaskOriginal.size() >= kScene7040ColorToItemMapOffset + kScene7040ColorMapSize &&
				_paletteMask.size() >= kScene7040ColorToItemMapOffset + kScene7040ColorMapSize) {
			for (uint i = 0; i < kScene7040ColorMapSize; ++i) {
				const byte originalItem = _paletteMaskOriginal[kScene7040ColorToItemMapOffset + i];
				if (!_vm->gameState().reviewedFrankensteinNote) {
					if (originalItem == 9)
						_paletteMask[kScene7040ColorToItemMapOffset + i] = 2;
					if (originalItem == 10)
						_paletteMask[kScene7040ColorToItemMapOffset + i] = 0;
				} else {
					if (originalItem == 8)
						_paletteMask[kScene7040ColorToItemMapOffset + i] = 0;
					if (originalItem == 9 || originalItem == 10)
						_paletteMask[kScene7040ColorToItemMapOffset + i] = 8;
				}
			}
		}

		if (_vm->gameState().reviewedFrankensteinNote) {
			if (_metadata.size() >= kSceneItemInteractionPoints + 9 * 4 &&
					_metadata.size() >= kSceneItemFacing + 3) {
				const uint item2Interaction = kSceneItemInteractionPoints + 2 * 4;
				_metadata[item2Interaction] = 0xf2;
				_metadata[item2Interaction + 1] = 0;
				_metadata[item2Interaction + 2] = 0x46;
				_metadata[item2Interaction + 3] = 1;
				const uint item8Interaction = kSceneItemInteractionPoints + 8 * 4;
				_metadata[item8Interaction] = 0xf2;
				_metadata[item8Interaction + 1] = 0;
				_metadata[item8Interaction + 2] = 0x46;
				_metadata[item8Interaction + 3] = 1;
				_metadata[kSceneItemFacing + 2] = 1;
			}
		}

		rebuildWalkablePaletteMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	if (selector == 3 || selector == 0xff) {
		restoreBaseFramebufferFromOriginal();

		if (_vm->gameState().officeNotePickupState == 1)
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
		else
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
	}

	return true;
}

byte Scene7040::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	switch (animationGroup) {
	case kScene7040JosephRevealSpeechGroupA:
		return kScene7040JosephRevealSpeechBaseFrameA;
	case kScene7040JosephRevealSpeechGroupB:
		return kScene7040JosephRevealSpeechBaseFrameB;
	case kScene7040ActionSpeechGroupA:
		return kScene7040ActionSpeechBaseFrameA;
	case kScene7040ActionSpeechGroupB:
		return kScene7040ActionSpeechBaseFrameB;
	case kScene7040FrankieSpeechGroupA:
		return kScene7040FrankieSpeechBaseFrameA;
	case kScene7040FrankieSpeechGroupB:
		return kScene7040FrankieSpeechBaseFrameB;
	default:
		return kScene7040JosephNormalSpeechBaseFrame;
	}
}

void Scene7040::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	switch (animationGroup) {
	case kScene7040ActionSpeechGroupA:
	case kScene7040ActionSpeechGroupB:
		setChunk14ActionVisible(true);
		setChunk14ActionFrame(frameIndex);
		break;
	case kScene7040FrankieSpeechGroupA:
	case kScene7040FrankieSpeechGroupB:
		configureChunk14AltLayer(15, true);
		setChunk14AltFrame(frameIndex);
		break;
	default:
		_preItemIdleAnimation.setFrame(frameIndex);
		break;
	}
}

void Scene7040::primarySpeechAnimationStarted(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	(void)baseFrame;

	_primarySpeechLeadInTicks = 0;
}

void Scene7040::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	(void)baseFrame;

	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	drawPlayableComposite();
	presentFrame();
}

void Scene7040::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene7040Chunk14ActionHook)
		applyChunk14ActionSideEffects((byte)(frame - 1));
	else if (hookId == kScene7040Chunk14AltHook)
		applyChunk14AltSideEffects((byte)(frame - 1));
}

void Scene7040::advanceChunk11PreItemIdleAnimation(uint32 delta) {
	_preItemIdleAnimation.advance(_random, delta);
}

void Scene7040::advancePrimaryDialogueSpeechFrame(uint32 delta) {
	_primaryDialogueSpeechTimerAccumulator += delta;
	while (_primaryDialogueSpeechTimerAccumulator >= kScene7040Chunk14FrameMillis) {
		_primaryDialogueSpeechTimerAccumulator -= kScene7040Chunk14FrameMillis;
		if (_primarySpeechLeadInTicks < 3) {
			++_primarySpeechLeadInTicks;
			continue;
		}

		const byte baseFrame = primarySpeechAnimationBaseFrame(_primaryDialogueSpeechGroup);
		const byte nextFrame = (byte)(baseFrame +
			pickPrimarySpeechFrameExcluding(kScene7040PrimarySpeechFrameCount, _primarySpeechLastMouthFrameOffset));
		_primarySpeechLastMouthFrameOffset = (byte)(nextFrame - baseFrame);
		_primaryDialogueSpeechLastFrame = nextFrame;
		setPrimarySpeechAnimationFrame(_primaryDialogueSpeechGroup, nextFrame);
	}
}

byte Scene7040::pickPrimarySpeechFrameExcluding(byte frameCount, byte previousFrame) {
	if (frameCount <= 1)
		return 0;

	byte nextFrame = previousFrame;
	for (uint attempt = 0; attempt < 8 && nextFrame == previousFrame; ++attempt)
		nextFrame = (byte)_random.getRandomNumber(frameCount - 1);

	if (nextFrame == previousFrame)
		nextFrame = (byte)((previousFrame + 1) % frameCount);

	return nextFrame;
}

void Scene7040::advanceChunk16PostItemAnimation(uint32 delta) {
	for (uint frame = 0; frame < _postItemAnimation.consumeFrames(delta); ++frame) {
		switch (_postItemIdleState) {
		case 0:
			++_postItemAnimation.frameIndex;
			if (_postItemAnimation.frameIndex >= 5)
				_postItemIdleState = 1;
			break;
		case 1:
			++_postItemAnimation.frameIndex;
			if (_postItemAnimation.frameIndex >= 0x1a)
				_postItemIdleState = 2;
			break;
		case 2:
			++_postItemAnimation.frameIndex;
			if (_postItemAnimation.frameIndex >= 0x1e)
				_postItemIdleState = 3;
			break;
		default:
			if (_random.getRandomNumber(0x0e) == 0)
				_postItemAnimation.frameIndex = 0x22;
			else
				_postItemAnimation.frameIndex = 0x1e;
			break;
		}
		if (_postItemAnimation.frameIndex >= ARRAYSIZE(kScene7040Chunk16PostItemFrameMap))
			_postItemAnimation.frameIndex = 1;
	}

	for (uint frame = 0; frame < _chunk17Animation.consumeFrames(delta); ++frame) {
		if (_postItemIdleState > 1 || _chunk17Animation.frameIndex != 0)
			_chunk17Animation.frameIndex = _chunk17Animation.frameIndex == 8 ? 0 :
				(byte)(_chunk17Animation.frameIndex + 1);
	}
}

bool Scene7040::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a terraza (go to terrace)
		handleActionSlot00ReturnToG03();
		return true;
	case 302: // Mirar puerta (look at door)
		handleActionSlot01ProgressSpeech();
		return true;
	case 303: // Usar/Abrir puerta (use/open door)
		handleActionSlot02MajorHotspotAction();
		return true;
	case 304: // Ir a escalera (go to stairs)
		handleActionSlot03TransitionToState7060();
		return true;
	case 305: // Mirar escalera (look at stairs)
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar puerta (look at door)
		handleActionSlot05ExitProgressSpeech();
		return true;
	case 307: // Usar/Abrir puerta (use/open door)
		handleActionSlot06TransitionToG05();
		return true;
	case 308: // Mirar estatua (look at statue)
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Mirar florero (look at vase)
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Coger libreta (take notebook)
		handleActionSlot09PickupItem0FThenExit();
		return true;
	case 311: // Usar libreta (use notebook)
		handleActionSlot10CommonSpeech();
		return true;
	case 312: // Mirar libreta (look at notebook)
		handleActionHandler312ProgressSpeech();
		return true;
	case 313: // Hablar con Joseph (talk to Joseph)
		handleActionHandler313ConversationGate();
		return true;
	case 314: // Mirar Joseph (look at Joseph)
		handleActionHandler314FrankensteinNoteSpeech();
		return true;
	case 315: // Coger florero (take vase)
		handleActionHandler315PickupItem0C();
		return true;
	default:
		return false;
	}
}

void Scene7040::handleActionSlot00ReturnToG03() {
	_vm->gameState().mainFlowStateId = kScene7040ReturnState7031;
}

void Scene7040::handleActionSlot01ProgressSpeech() {
	beginSecondarySpeechLine(1, _vm->gameState().officeStatueActionProgress == 0 ? 0 : 1);
}

void Scene7040::handleActionSlot02MajorHotspotAction() {
	GameplayState &state = _vm->gameState();
	if (state.reviewedFrankensteinNote) {
		beginSecondarySpeechLine(3, 0x0b);
		return;
	}

	setChunk12OverlayVisible(true);
	if (state.officeStatueActionProgress == 2) {
		runActorReplacement(ActionOverlaySpec(13, kScene7040Chunk13DescriptorCount,
			kScene7040MajorHotspotFrameMap, ARRAYSIZE(kScene7040MajorHotspotFrameMap), kScene7040Chunk14FrameMillis)
			.noRedrawAtEnd()
			.endAt(0x2d));
		_soundBank0.playSample(0x15, 100);
		runMajorHotspotFrankensteinBranch();
		setChunk12OverlayVisible(true);
		runActorReplacement(ActionOverlaySpec(13, kScene7040Chunk13DescriptorCount,
			kScene7040MajorHotspotFrameMap, ARRAYSIZE(kScene7040MajorHotspotFrameMap), kScene7040Chunk14FrameMillis)
			.noRedrawAtEnd()
			.frameRange(0x35, ARRAYSIZE(kScene7040MajorHotspotFrameMap)));
	} else {
		runActorReplacement(ActionOverlaySpec(13, kScene7040Chunk13DescriptorCount,
			kScene7040MajorHotspotFrameMap, ARRAYSIZE(kScene7040MajorHotspotFrameMap), kScene7040Chunk14FrameMillis)
			.soundAt(0x2c, 0x15)
			.noRedrawAtEnd());
	}
	setChunk12OverlayVisible(false);

	runMajorHotspotReturnPath(state.officeStatueActionProgress == 2 ? 4 : 5);
	switch (state.officeStatueActionProgress) {
	case 0:
		beginSecondarySpeechLine(2, 0);
		state.officeStatueActionProgress = 1;
		break;
	case 1:
		beginSecondarySpeechLine(2, 1);
		state.officeStatueActionProgress = 2;
		break;
	case 2:
		beginSecondarySpeechLine(3, 10);
		state.officeStatueActionProgress = 3;
		state.frankensteinNoteOverlayMode = 1;
		break;
	default:
		beginSecondarySpeechLine(2, 2);
		break;
	}
}

void Scene7040::handleActionSlot03TransitionToState7060() {
	_vm->gameState().mainFlowStateId = kScene7040ExitState7060;
}

void Scene7040::handleActionSlot05ExitProgressSpeech() {
	beginSecondarySpeechLine(5, _vm->gameState().openedOfficeClosetDoor ? 1 : 0);
}

void Scene7040::handleActionSlot06TransitionToG05() {
	runActorReplacement(10, kScene7040Chunk10DescriptorCount, kScene7040Chunk10ExitFrameMap,
		ARRAYSIZE(kScene7040Chunk10ExitFrameMap), kScene7040Chunk14FrameMillis);
	_vm->gameState().openedOfficeClosetDoor = true;
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene7040ExitState7050;
}

void Scene7040::handleActionSlot09PickupItem0FThenExit() {
	GameplayState &state = _vm->gameState();
	if (state.officeStatueActionProgress <= 2 || state.officeNotePickupState == 2) {
		beginSecondarySpeechLine(9, 0);
		return;
	}
	if (!state.reviewedFrankensteinNote) {
		beginSecondarySpeechLine(8, 0);
		return;
	}

	beginSecondarySpeechLine(8, 1);
	runActorReplacement(18, kScene7040Chunk18DescriptorCount, kScene7040Chunk18PickupItem0FFrameMap,
		ARRAYSIZE(kScene7040Chunk18PickupItem0FFrameMap), kScene7040Chunk14FrameMillis);
	addInventoryItem(0x0f);
	_soundBank0.playSample(1, 100);
	state.officeNotePickupState = 2;
	beginSecondarySpeechLine(8, 2);
	walkActiveActorTo(600, 0x132, kScene7040InvalidFacing, 0);
	handleActionSlot06TransitionToG05();
}

void Scene7040::handleActionSlot10CommonSpeech() {
	beginSecondarySpeechLine(9, 0);
}

void Scene7040::handleActionHandler312ProgressSpeech() {
	GameplayState &state = _vm->gameState();
	if (state.officeStatueActionProgress == 3)
		beginSecondarySpeechLine(10, state.officeNotePickupState >= 2 ? 1 : 0);
	else
		beginStaticSecondarySpeechLine(0x2d, 0);
}

void Scene7040::handleActionHandler313ConversationGate() {
	if (_vm->gameState().reviewedFrankensteinNote) {
		beginSecondarySpeechLine(11, 2);
		return;
	}
	runDialogueMenuRow98();
}

void Scene7040::handleActionHandler314FrankensteinNoteSpeech() {
	beginSecondarySpeechLine(11, _vm->gameState().reviewedFrankensteinNote ? 1 : 0);
}

void Scene7040::handleActionHandler315PickupItem0C() {
	if (hasInventoryItem(0x0c))
		return;

	addInventoryItem(0x0c);
	_soundBank0.playSample(1, 100);
}

void Scene7040::runDialogueMenuRow98() {
	Common::Array<DialogueChoiceRecord> records;
	initializeDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;

	beginSecondarySpeechLine(kScene7040DialogueStageId, 0);
	_preItemIdleAnimation.setStateAndFrame(3, 7);
	beginPrimarySpeechLine(kScene7040DialoguePrimaryRow, 0, kScene7040DialoguePrimaryCenterX,
		kScene7040DialoguePrimaryTopY, kScene7040DialoguePrimaryRed, kScene7040DialoguePrimaryGreen,
		kScene7040DialoguePrimaryBlue);

	while (!finished && !Engine::shouldQuit()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene7040DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene7040DialogueStageId, 5);
			beginPrimarySpeechLine(kScene7040DialoguePrimaryRow, 5, kScene7040DialoguePrimaryCenterX,
				kScene7040DialoguePrimaryTopY, kScene7040DialoguePrimaryRed, kScene7040DialoguePrimaryGreen,
				kScene7040DialoguePrimaryBlue);
			_preItemIdleAnimation.reset();
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene7040DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff) {
			beginPrimarySpeechLine(kScene7040DialoguePrimaryRow, record.responseFrameIndex,
				kScene7040DialoguePrimaryCenterX, kScene7040DialoguePrimaryTopY, kScene7040DialoguePrimaryRed,
				kScene7040DialoguePrimaryGreen, kScene7040DialoguePrimaryBlue);
		}

		if (record.disableAfterUse == 1)
			record.enabled = 0;

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			finished = true;
			break;
		case 1:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 1;
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 2;
			break;
		default:
			break;
		}
	}

	_preItemIdleAnimation.reset();
}

void Scene7040::initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene7040DialogueChoiceRecordCount);

	records[0].transitionMode = 3;
	records[0].playerTextRowId = 1;
	records[0].responseFrameIndex = 1;
	records[0].disableAfterUse = 1;
	records[0].reserved = 0xff;

	records[1].transitionMode = 3;
	records[1].playerTextRowId = 2;
	records[1].responseFrameIndex = 2;
	records[1].disableAfterUse = 1;
	records[1].reserved = 0xff;

	records[2].enabled = 1;
	records[2].transitionMode = 3;
	records[2].playerTextRowId = 3;
	records[2].responseFrameIndex = 3;
	records[2].disableAfterUse = 1;
	records[2].reserved = 0xff;

	records[3].transitionMode = 3;
	records[3].playerTextRowId = 4;
	records[3].responseFrameIndex = 4;
	records[3].disableAfterUse = 1;
	records[3].reserved = 0xff;

	records[4].enabled = 1;
	records[4].transitionMode = 0;
	records[4].playerTextRowId = 5;
	records[4].responseFrameIndex = 5;
	records[4].reserved = 0xff;

	const GameplayState &state = _vm->gameState();
	if (state.officeStatueActionProgress != 0)
		records[0].enabled = 1;
	if (state.officeStatueActionProgress == 3) {
		if (!state.hasInventoryItem(state.currentInventoryOwnerIndex, 6))
			records[1].enabled = 1;
		if (state.officeNotePickupState != 2)
			records[3].enabled = 1;
	}
}

void Scene7040::runMajorHotspotFrankensteinBranch() {
	const bool previousHideActiveActor = _hideActiveActor;
	const byte previousPreItemIdleState = _preItemIdleAnimation.state;
	const uint previousAltChunkIndex = chunk14AltChunkIndex();
	_hideActiveActor = true;
	_preItemIdleAnimation.state = 3;
	setChunk12OverlayVisible(true);
	setChunk12OverlayFrame(0);
	setChunk14ActionVisible(true);
	setChunk14AltVisible(false);

	runChunk14ActionRange(0, 0x10);
	beginPrimarySpeechLineWithAnimationGroup(3, 0, 0x154, 0x5f, 0x20, 0, 0x3f,
		kScene7040ActionSpeechGroupA);
	beginPrimarySpeechLineWithAnimationGroup(3, 1, 0x1c2, 0x73, 0x3f, 0x32, 0x0c,
		kScene7040JosephNormalSpeechGroup);
	runChunk14ActionRange(0x15, 0x61);
	_vm->gameState().officeNotePickupState = 1;
	applySceneStateToHotspotsAndPatches(3);
	runChunk14ActionRange(0x61, 0x6b);
	beginPrimarySpeechLineWithAnimationGroup(3, 2, 0x16d, 0x69, 0x20, 0, 0x3f,
		kScene7040ActionSpeechGroupB);
	runChunk14ActionRange(0x6f, 0x7c);
	setChunk14ActionVisible(false);
	setChunk12OverlayVisible(false);
	setChunk12OverlayFrame(0);

	configureChunk14AltLayer(15, false);
	runChunk11Range(0x0b, 0x12);
	beginPrimarySpeechLineWithAnimationGroup(3, 3, 0x1a9, 0x82, 0x3f, 0x32, 0x0c,
		kScene7040JosephRevealSpeechGroupA);
	setChunk14AltVisible(true);
	runChunk14AltRange(15, 0, 0x14);
	beginPrimarySpeechLineWithAnimationGroup(3, 4, 0x136, 0x6e, 0x0a, 0x3f, 0,
		kScene7040FrankieSpeechGroupA);
	runChunk11Range(0x16, 0x1a);
	beginPrimarySpeechLineWithAnimationGroup(3, 5, 0x1a9, 0x82, 0x3f, 0x32, 0x0c,
		kScene7040JosephRevealSpeechGroupB);
	runChunk11Range(0x1e, 0x21);
	runChunk14AltRange(15, 0x18, 0x1c);
	beginPrimarySpeechLineWithAnimationGroup(3, 6, 0x14f, 0x73, 0x0a, 0x3f, 0,
		kScene7040FrankieSpeechGroupB);
	runChunk14AltRange(15, 0x20, 0x25);
	beginPrimarySpeechLineWithAnimationGroup(3, 7, 0x1c2, 0x73, 0x3f, 0x32, 0x0c,
		kScene7040JosephNormalSpeechGroup);
	runChunk14AltRange(15, 0x18, 0x1c);
	beginPrimarySpeechLineWithAnimationGroup(3, 8, 0x14f, 0x73, 0x0a, 0x3f, 0,
		kScene7040FrankieSpeechGroupB);
	runChunk14AltRange(15, 0x25, 0x3f);
	setChunk14AltVisible(false);
	configureChunk14AltLayer(previousAltChunkIndex, false);
	beginPrimarySpeechLineWithAnimationGroup(3, 9, 0x1c2, 0x73, 0x3f, 0x32, 0x0c,
		kScene7040JosephNormalSpeechGroup);

	_preItemIdleAnimation.setFrame(0);
	setChunk14ActionVisible(false);
	setChunk14AltVisible(false);
	setChunk12OverlayVisible(false);
	_preItemIdleAnimation.state = previousPreItemIdleState;
	_hideActiveActor = previousHideActiveActor;
}

void Scene7040::runMajorHotspotReturnPath(byte finalFacing) {
	setActiveActorPose(kScene7040MajorHotspotOverlayEndX, kScene7040MajorHotspotOverlayEndY, kScene7040MajorHotspotOverlayEndFacing);

	walkActiveActorTo(kScene7040MajorHotspotOverlayEndX, kScene7040MajorHotspotOverlayEndY,
		kScene7040MajorHotspotReturnStartFacing, 0, false);
	walkActiveActorTo(kScene7040MajorHotspotReturnX, kScene7040MajorHotspotReturnY, finalFacing, 0, false);
}

void Scene7040::runChunk11Range(byte firstFrame, byte endFrame) {
	const byte previousPreItemIdleState = _preItemIdleAnimation.state;
	_preItemIdleAnimation.state = 3;
	playAnimationFrames(_preItemIdleAnimation,
		AnimationFrameRange(firstFrame + 1, endFrame, kScene7040Chunk11FrameMillis));
	_preItemIdleAnimation.state = previousPreItemIdleState;
}

void Scene7040::runChunk14ActionRange(byte firstFrame, byte endFrame) {
	setChunk14ActionVisible(true);
	playAnimationFrames(_animationLayers, kScene7040Chunk14ActionLayer,
		AnimationFrameRange(firstFrame + 1, endFrame, kScene7040Chunk14FrameMillis)
			.hookEveryFrame(kScene7040Chunk14ActionHook));
}

void Scene7040::runChunk14AltRange(uint chunkIndex, byte firstFrame, byte endFrame) {
	configureChunk14AltLayer(chunkIndex, true);
	playAnimationFrames(_animationLayers, kScene7040Chunk14AltLayer,
		AnimationFrameRange(firstFrame + 1, endFrame, kScene7040Chunk14FrameMillis)
			.hookEveryFrame(kScene7040Chunk14AltHook));
}

void Scene7040::applyChunk14ActionSideEffects(byte frameIndex) {
	switch (frameIndex) {
	case 0:
		_soundBank0.playSample(3, 100);
		setChunk12OverlayVisible(true);
		setChunk12OverlayFrame(1);
		break;
	case 1:
		setChunk12OverlayVisible(true);
		setChunk12OverlayFrame(2);
		break;
	case 8:
		setChunk12OverlayVisible(true);
		setChunk12OverlayFrame(1);
		break;
	case 9:
		_soundBank0.playSample(4, 100);
		setChunk12OverlayVisible(true);
		setChunk12OverlayFrame(0);
		break;
	case 0x0a:
		setChunk12OverlayVisible(false);
		break;
	case 0x22:
		_soundBank0.playSample(0x16, 50);
		break;
	case 0x6a:
		_soundBank0.stop();
		break;
	default:
		break;
	}
}

void Scene7040::applyChunk14AltSideEffects(byte frameIndex) {
	switch (frameIndex) {
	case 0x2b:
		_soundBank0.playSample(0x17, 50);
		_vm->gameState().officeNotePickupState = 0;
		applySceneStateToHotspotsAndPatches(3);
		break;
	case 0x37:
		_soundBank0.playSample(3, 100);
		setChunk12OverlayVisible(true);
		setChunk12OverlayFrame(1);
		break;
	case 0x38:
		setChunk12OverlayVisible(true);
		setChunk12OverlayFrame(2);
		break;
	case 0x3d:
		_soundBank0.playSample(3, 100);
		setChunk12OverlayVisible(true);
		setChunk12OverlayFrame(1);
		break;
	case 0x3e:
		_soundBank0.playSample(4, 100);
		setChunk12OverlayVisible(true);
		setChunk12OverlayFrame(0);
		break;
	case 0x3f:
		setChunk12OverlayVisible(false);
		break;
	default:
		break;
	}
}

void Scene7040::configureAnimationLayers() {
	_animationLayers.configure(kScene7040AnimationLayerSpecs);
	const bool postItemMode = _vm->gameState().reviewedFrankensteinNote;
	_animationLayers.setLayerVisible(kScene7040Chunk17Layer, postItemMode);
	_animationLayers.setLayerVisible(kScene7040Chunk16Layer, postItemMode);
	_animationLayers.setLayerVisible(kScene7040Chunk11Layer, !postItemMode);
}

void Scene7040::resetTransientAnimationLayers() {
	setChunk12OverlayVisible(false);
	setChunk12OverlayFrame(0);
	setChunk14ActionVisible(false);
	setChunk14ActionFrame(0);
	configureChunk14AltLayer(14, false);
}

void Scene7040::syncAnimationLayerFrames() {
	_animationLayers.setLayerFrame(kScene7040Chunk17Layer, _chunk17Animation.frameIndex);
	_animationLayers.setLayerFrame(kScene7040Chunk16Layer, _postItemAnimation.frameIndex);
	_animationLayers.setLayerFrame(kScene7040Chunk11Layer, _preItemIdleAnimation.channel.frameIndex);
}

void Scene7040::setChunk12OverlayVisible(bool visible) {
	_animationLayers.setLayerVisible(kScene7040Chunk12Layer, visible);
}

void Scene7040::setChunk12OverlayFrame(byte frameIndex) {
	_animationLayers.setLayerFrame(kScene7040Chunk12Layer, frameIndex);
}

void Scene7040::setChunk14ActionVisible(bool visible) {
	_animationLayers.setLayerVisible(kScene7040Chunk14ActionLayer, visible);
}

void Scene7040::setChunk14ActionFrame(byte frameIndex) {
	_animationLayers.setLayerFrame(kScene7040Chunk14ActionLayer, frameIndex);
}

void Scene7040::configureChunk14AltLayer(uint chunkIndex, bool visible) {
	_animationLayers.configureLayerResource(kScene7040Chunk14AltLayer, chunkIndex,
		kScene7040Chunk14AltDescriptorCount, kScene7040Chunk14AltFrameMap,
		ARRAYSIZE(kScene7040Chunk14AltFrameMap), visible);
}

void Scene7040::setChunk14AltVisible(bool visible) {
	_animationLayers.setLayerVisible(kScene7040Chunk14AltLayer, visible);
}

void Scene7040::setChunk14AltFrame(byte frameIndex) {
	_animationLayers.setLayerFrame(kScene7040Chunk14AltLayer, frameIndex);
}

uint Scene7040::chunk14AltChunkIndex() const {
	if (!_animationLayers.hasLayer(kScene7040Chunk14AltLayer))
		return 14;
	return _animationLayers.layer(kScene7040Chunk14AltLayer).chunkIndex;
}

void Scene7040::runExitSideEffectsAfterLoop() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene7040ExitState7050 &&
			state.reviewedFrankensteinNote && state.officeNotePickupState == 2) {
		state.reviewedFrankensteinNote = false;
	}
}

} // End of namespace Hollywood
