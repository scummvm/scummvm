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

#include "hollywood/scenes/playable/scene2110.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene2110ScriptedReturnState = 0x083f;
const uint16 kScene2100ReturnFromTreasureState = 0x0835;
const uint16 kScene2100LeftPassageState = 0x0836;
const uint16 kScene2110ViewportXOffset = 0x0000;
const uint16 kScene2110ViewportMaxXOffset = 0x0030;
const uint kScene2110ActorBankTableEntry = 0x0000;
const uint kScene2110ActorPaletteTableEntry = 0x00cc;
const uint kScene2110Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2110SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2110FrameMillis = 75;
const uint32 kScene2110ActorPathFrameMillis = 60;
const uint32 kScene2110PrimarySpeechFrameMillis = 125;
const uint32 kScene2110WaitSliceMillis = 10;
const byte kScene2110InvalidFacing = 0xff;
const byte kScene2110EntryLayerChunk = 5;
const byte kScene2110EntryLayerDescriptorCount = 0x1a;
const byte kScene2110AmbientChunk = 6;
const byte kScene2110AmbientDescriptorCount = 0x1a;
const byte kScene2110TreasureChunk = 7;
const byte kScene2110TreasureDescriptorCount = 0x13;
const byte kScene2110EntrySpeechBaseFrameA = 0x0e;
const byte kScene2110EntrySpeechBaseFrameB = 0x16;
const byte kScene2110TreasureSpeechBaseFrame = 0x0a;

enum Scene2110PrimarySpeechGroup {
	kScene2110EntrySpeechGroupA = 1,
	kScene2110EntrySpeechGroupB,
	kScene2110TreasureSpeechGroup
};

enum Scene2110AnimationHook {
	kScene2110EntryPathHook = 1,
	kScene2110TreasureSoundHook
};

const byte kScene2110EntryLayerFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 20, 19, 18,
	13
};

const byte kScene2110AmbientFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25
};

const byte kScene2110TreasureFrameMap[] = {
	0, 1, 2, 3, 4, 8, 9, 10, 9, 8,
	4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 4, 8, 9, 10, 9,
	8, 4, 3, 2, 1, 0
};

const byte kScene2110TreasureGrantItems[] = {
	0x30, 0x42, 0x4c
};

static_assert(ARRAYSIZE(kScene2110EntryLayerFrameMap) == 0x1f,
	"Scene 2110 entry-layer frame map size changed");
static_assert(ARRAYSIZE(kScene2110AmbientFrameMap) == 0x1a,
	"Scene 2110 ambient frame map size changed");
static_assert(ARRAYSIZE(kScene2110TreasureFrameMap) == 0x24,
	"Scene 2110 treasure frame map size changed");

static PlayableSceneConfig scene2110Config() {
	PlayableSceneConfig config(2110,
		SceneResourceLayout(8, 5, 7),
		SceneViewport(kScene2110ViewportXOffset, kScene2110ViewportXOffset, kScene2110ViewportMaxXOffset),
		SceneActorPose(0x24e, 0x122, 4));
	config.setActorResources(kScene2110ActorBankTableEntry, kScene2110ActorPaletteTableEntry);
	config.setTextResources(kScene2110Resource003RowsOffsetIndex, kScene2110SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = true;
	return config;
}

Scene2110::Scene2110(HollywoodEngine *vm) :
		PlayableScene(vm, scene2110Config()),
		_ambientChannel(),
		_entryIdleChannel(),
		_scriptedActorPathChannel(),
		_entryLayer(),
		_ambientLayer(),
		_treasureLayer(),
		_entryIdleActive(false),
		_scriptedActorPathActive(false),
		_scriptedActorPathFrameIndex(0) {
	_entryLayer.configure(kScene2110EntryLayerChunk, kScene2110EntryLayerDescriptorCount,
		kScene2110EntryLayerFrameMap, ARRAYSIZE(kScene2110EntryLayerFrameMap));
	_ambientLayer.configure(kScene2110AmbientChunk, kScene2110AmbientDescriptorCount,
		kScene2110AmbientFrameMap, ARRAYSIZE(kScene2110AmbientFrameMap));
	_treasureLayer.configure(kScene2110TreasureChunk, kScene2110TreasureDescriptorCount,
		kScene2110TreasureFrameMap, ARRAYSIZE(kScene2110TreasureFrameMap));
}

void Scene2110::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	if (_vm->gameState().mainFlowStateId == kScene2110ScriptedReturnState) {
		_activeActorWorldX = 0x320;
		_activeActorWorldY = 0x118;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x320;
		_activeActorWorldY = 0x104;
		_activeActorFacing = 4;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene2110::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_entryLayer);
	drawResourceSpriteLayer(_ambientLayer);
	drawActionOverlayLayer();

	if (_treasureLayer.visible) {
		drawResourceSpriteLayer(_treasureLayer);
	} else {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}
}

void Scene2110::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene2110ScriptedReturnState)
		runScriptedReturnToScene2100();
	else
		runEntryFromScene2100();
}

bool Scene2110::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

bool Scene2110::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff &&
		stateId != kScene2100ReturnFromTreasureState && !isMainFlowStateInScene(stateId);
}

void Scene2110::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

bool Scene2110::advanceCustomGameplayLoop(uint32 delta) {
	advanceAmbientLayer(delta);
	advanceEntryIdle(delta);
	advanceScriptedActorPath(delta);
	return false;
}

bool Scene2110::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a entrada (go to entrance): returns to the mummy office.
		runExitToScene2100();
		return true;
	case 302: // Mirar entrada (look at entrance): describe the passage back.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar trofeo (look at trophy): describe the trophy.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar base del trofeo (look at trophy base): read its inscription.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Abrir base del trofeo (open trophy base): reveal its reward.
		runTreasureGrantAction();
		return true;
	case 306: // Coger tesoros (take treasures): Ron cannot carry the hoard.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Mirar tesoros / pasta gansa (look at treasures/big money).
		beginSecondarySpeechLine(5, 0);
		return true;
	default:
		return false;
	}
}

bool Scene2110::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = MAX<int>(targetX, 0x102);

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

byte Scene2110::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	switch (animationGroup) {
	case kScene2110EntrySpeechGroupA:
		return kScene2110EntrySpeechBaseFrameA;
	case kScene2110EntrySpeechGroupB:
		return kScene2110EntrySpeechBaseFrameB;
	case kScene2110TreasureSpeechGroup:
		return kScene2110TreasureSpeechBaseFrame;
	default:
		return 0;
	}
}

byte Scene2110::primarySpeechAnimationFrameCount(byte animationGroup) const {
	return animationGroup == kScene2110TreasureSpeechGroup ? 4 : 5;
}

uint32 Scene2110::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene2110PrimarySpeechFrameMillis;
}

void Scene2110::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene2110TreasureSpeechGroup) {
		_treasureLayer.visible = true;
		_treasureLayer.setFrame(frameIndex);
		return;
	}

	_entryLayer.visible = true;
	_entryLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene2110::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2110::resetAnimationLayers() {
	_ambientChannel.reset(0, kScene2110FrameMillis);
	_entryIdleChannel.reset(kScene2110EntrySpeechBaseFrameA,
		kScene2110PrimarySpeechFrameMillis);
	_scriptedActorPathChannel.reset(0, kScene2110ActorPathFrameMillis);
	_entryLayer.visible = false;
	_entryLayer.reset(0);
	_ambientLayer.visible = true;
	_ambientLayer.reset(0);
	_treasureLayer.visible = false;
	_treasureLayer.reset(0);
	_entryIdleActive = false;
	_scriptedActorPathActive = false;
	_scriptedActorPathFrameIndex = 0;
}

void Scene2110::advanceAmbientLayer(uint32 delta) {
	const uint frameCount = _ambientChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		byte nextFrame = (byte)(_ambientLayer.frameIndex + 1);
		if (nextFrame >= ARRAYSIZE(kScene2110AmbientFrameMap))
			nextFrame = 0;
		_ambientLayer.setFrame(nextFrame);
	}
}

void Scene2110::advanceEntryIdle(uint32 delta) {
	if (!_entryIdleActive)
		return;

	const uint frameCount = _entryIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_entryLayer.frameIndex == kScene2110EntrySpeechBaseFrameA + 4) {
			_entryLayer.setFrame(kScene2110EntrySpeechBaseFrameA);
		} else if (_random.getRandomNumber(14) == 0) {
			_entryLayer.setFrame(kScene2110EntrySpeechBaseFrameA + 4);
		} else {
			_entryLayer.setFrame(kScene2110EntrySpeechBaseFrameA);
		}
	}
}

void Scene2110::startScriptedActorPath() {
	queueActorPathWithPaletteRegionRouting(_activeActorWorldX, _activeActorWorldY,
		0x263, 0x135, kScene2110InvalidFacing, 0);
	_scriptedActorPathFrameIndex = 1;
	_scriptedActorPathChannel.reset(0, kScene2110ActorPathFrameMillis);
	_lastViewportScrollActorWorldX = _activeActorWorldX;
	_scriptedActorPathActive = _actorPathFrames.size() > 1;
	_actorPathPlaybackActive = _scriptedActorPathActive;
	if (!_scriptedActorPathActive && !_actorPathFrames.empty()) {
		const ActorPathFrame &frame = _actorPathFrames.back();
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
	}
}

void Scene2110::advanceScriptedActorPath(uint32 delta) {
	if (!_scriptedActorPathActive)
		return;

	const uint frameCount = _scriptedActorPathChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount && _scriptedActorPathFrameIndex < _actorPathFrames.size(); ++i) {
		const ActorPathFrame &frame = _actorPathFrames[_scriptedActorPathFrameIndex++];
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
	}
	if (_scriptedActorPathFrameIndex >= _actorPathFrames.size()) {
		_scriptedActorPathActive = false;
		_actorPathPlaybackActive = false;
	}
}

void Scene2110::finishScriptedActorPath() {
	while (_scriptedActorPathActive && !animationPlaybackShouldStop()) {
		if (waitSceneMillis(kScene2110WaitSliceMillis, false))
			break;
	}

	if (_scriptedActorPathActive && !_actorPathFrames.empty()) {
		const ActorPathFrame &frame = _actorPathFrames.back();
		_activeActorWorldX = frame.worldX;
		_activeActorWorldY = frame.worldY;
		_activeActorFacing = frame.facing;
		_activeActorCel = frame.cel;
		_activeActorDrawOrderMode = frame.drawOrderMode;
	}
	_scriptedActorPathActive = false;
	_actorPathPlaybackActive = false;
}

void Scene2110::runEntryFromScene2100() {
	runEntryPathWithFinalFacing(0x320, 0x104, 4, 0x24e, 0x122, kScene2110InvalidFacing, 0);
	if (animationPlaybackShouldStop())
		return;

	GameplayState &state = _vm->gameState();
	if (!state.scene2110EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene2110EntryLineSeen = true;
	}
}

void Scene2110::runScriptedReturnToScene2100() {
	if (!runScriptedEntryOpening())
		return;

	runEntrySecondarySpeechLine(0);
	if (animationPlaybackShouldStop())
		return;
	if (!playAndPresentAnimationFrames(_entryLayer,
			AnimationFrameRange(0x12, 0x16, kScene2110FrameMillis).unskippable()))
		return;
	runEntryPrimarySpeechLine(1, kScene2110EntrySpeechGroupB);
	if (animationPlaybackShouldStop())
		return;
	if (!playAndPresentAnimationFrames(_entryLayer,
			AnimationFrameRange(0x1a, 0x1e, kScene2110FrameMillis).unskippable()))
		return;
	runEntrySecondarySpeechLine(2);
	if (animationPlaybackShouldStop())
		return;
	runEntryPrimarySpeechLine(3, kScene2110EntrySpeechGroupA);
	_entryLayer.visible = false;
	if (animationPlaybackShouldStop())
		return;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	invalidatePresentationPalette();
	presentFrame();

	GameplayState &state = _vm->gameState();
	state.mainFlowStateId = kScene2100ReturnFromTreasureState;
}

bool Scene2110::runScriptedEntryOpening() {
	setActiveActorPose(0x320, 0x118, 4);
	_entryLayer.visible = true;
	_entryLayer.reset(0);
	if (!playAndPresentAnimationFrames(_entryLayer,
			AnimationFrameRange(0, 0x0e, kScene2110FrameMillis)
				.hookAt(0x0a, kScene2110EntryPathHook).unskippable()))
		return false;
	finishScriptedActorPath();
	if (animationPlaybackShouldStop())
		return false;

	drawPlayableComposite();
	presentFrame();
	return true;
}

void Scene2110::runExitToScene2100() {
	GameplayState &state = _vm->gameState();
	state.mainFlowStateId = kScene2100LeftPassageState;
}

void Scene2110::runTreasureGrantAction() {
	GameplayState &state = _vm->gameState();
	const byte rewardIndex = state.frankensteinPartRewardIndex();
	const bool grantsReward = !state.scene2110TreasureGranted &&
		rewardIndex < ARRAYSIZE(kScene2110TreasureGrantItems);

	_treasureLayer.visible = true;
	_treasureLayer.reset(0);
	if (!playAndPresentAnimationFrames(_treasureLayer,
			AnimationFrameRange(0, 0x0a, kScene2110FrameMillis).unskippable())) {
		_treasureLayer.visible = false;
		return;
	}

	bool animationComplete = false;
	if (grantsReward) {
		runTreasurePrimarySpeechLine(0x16, (byte)(rewardIndex * 2));
		animationComplete = playAndPresentAnimationFrames(_treasureLayer,
			AnimationFrameRange(0x0d, 0x23, kScene2110FrameMillis)
				.hookAt(0x18, kScene2110TreasureSoundHook).unskippable());
	} else {
		runTreasurePrimarySpeechLine(0x66, 0);
		animationComplete = playAndPresentAnimationFrames(_treasureLayer,
			AnimationFrameRange(0x19, 0x23, kScene2110FrameMillis).unskippable());
	}

	_treasureLayer.visible = false;
	if (!animationComplete)
		return;
	if (!grantsReward) {
		drawPlayableComposite();
		presentFrame();
		return;
	}

	const byte itemId = kScene2110TreasureGrantItems[rewardIndex];
	if (!hasInventoryItem(itemId))
		addInventoryItem(itemId);
	setActiveActorPose(0x194, 0x155, 3);
	drawPlayableComposite();
	presentFrame();
	beginSecondarySpeechLine(0x16, (byte)(rewardIndex * 2 + 1));
	state.setFrankensteinPartRewardIndex(rewardIndex + 1);
	state.scene2110TreasureGranted = true;
}

void Scene2110::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel) {
	setActiveActorPose(startX, startY, startFacing);

	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;
	walkActiveActorTo(targetX, targetY, finalFacing, finalCel, false);
	if (finalFacing != kScene2110InvalidFacing)
		_activeActorFacing = finalFacing;
	_activeActorCel = finalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene2110::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene2110EntryPathHook && frame == 0x0a) {
		startScriptedActorPath();
	} else if (hookId == kScene2110TreasureSoundHook && frame == 0x18) {
		_soundBank0.playSample(1, 100);
	}
}

void Scene2110::runEntrySecondarySpeechLine(byte frameIndex) {
	_entryLayer.visible = true;
	_entryLayer.setFrame(kScene2110EntrySpeechBaseFrameA);
	_entryIdleChannel.reset(kScene2110EntrySpeechBaseFrameA,
		kScene2110PrimarySpeechFrameMillis);
	_entryIdleActive = true;
	beginSecondarySpeechLine(6, frameIndex);
	_entryIdleActive = false;
	_entryLayer.setFrame(kScene2110EntrySpeechBaseFrameA);
}

void Scene2110::runEntryPrimarySpeechLine(byte frameIndex, byte animationGroup) {
	beginPrimarySpeechLineWithAnimationGroup(6, frameIndex, 0x1f8, 0x46,
		0x32, 0x32, 0x3f, animationGroup);
}

void Scene2110::runTreasurePrimarySpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, 0x186, 0xbe,
		0x3f, 0x3f, 0x3f, kScene2110TreasureSpeechGroup);
}

} // End of namespace Hollywood
