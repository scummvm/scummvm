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

#include "common/system.h"

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/scenes/playable/scene2110.h"

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
const uint kScene2110EntryLayer = 0;
const uint kScene2110AmbientLayer = 1;
const uint kScene2110TreasureLayer = 2;

enum Scene2110PrimarySpeechGroup {
	kScene2110EntrySpeechGroupA = 1,
	kScene2110EntrySpeechGroupB,
	kScene2110TreasureSpeechGroup
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

const SceneLayerSpec kScene2110LayerSpecs[] = {
	{kSceneAnimationBehindActors, kScene2110EntryLayerChunk,
		kScene2110EntryLayerDescriptorCount, kScene2110EntryLayerFrameMap,
		ARRAYSIZE(kScene2110EntryLayerFrameMap), false, 0},
	{kSceneAnimationBehindActors, kScene2110AmbientChunk,
		kScene2110AmbientDescriptorCount, kScene2110AmbientFrameMap,
		ARRAYSIZE(kScene2110AmbientFrameMap), true, 0},
	{kSceneAnimationActorReplacement, kScene2110TreasureChunk,
		kScene2110TreasureDescriptorCount, kScene2110TreasureFrameMap,
		ARRAYSIZE(kScene2110TreasureFrameMap), false, 0}
};

const byte kScene2110TreasureGrantItems[] = {
	0x30, 0x42, 0x4c
};

PlayableSceneConfig scene2110Config() {
	PlayableSceneConfig config(2110,
		SceneResourceLayout(8, 5, 7),
		SceneViewport(kScene2110ViewportXOffset, kScene2110ViewportXOffset, kScene2110ViewportMaxXOffset),
		SceneActorPose(0x24e, 0x122, 4));
	config.setActorResources(kScene2110ActorBankTableEntry, kScene2110ActorPaletteTableEntry);
	config.setTextResources(kScene2110Resource003RowsOffsetIndex, kScene2110SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = true;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene2110::Scene2110(HollywoodEngine *vm) :
		PlayableScene(vm, scene2110Config()),
		_entryIdleChannel(),
		_ambientTrack(RealtimeAnimationTracks::kInvalidTrack),
		_entryIdleActive(false) {
	_sceneLayers.configure(kScene2110LayerSpecs);
	_ambientTrack = _realtimeAnimationTracks.addFrameMap(kScene2110AmbientLayer, kScene2110FrameMillis);
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

void Scene2110::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene2110ScriptedReturnState)
		runScriptedReturnToScene2100();
	else
		runEntryFromScene2100();
}

void Scene2110::runExitSideEffectsAfterLoop() {
	if (!didLeaveSceneAfterLoop() ||
		_vm->gameState().mainFlowStateId == kScene2100ReturnFromTreasureState)
		return;

	fadePaletteToBlack();
}

void Scene2110::advanceCustomGameplayLoop(uint32 delta) {
	advanceEntryIdle(delta);
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
		_sceneLayers.setLayerVisible(kScene2110TreasureLayer, true);
		_sceneLayers.setLayerFrame(kScene2110TreasureLayer, frameIndex);
		return;
	}

	_sceneLayers.setLayerVisible(kScene2110EntryLayer, true);
	_sceneLayers.setLayerFrame(kScene2110EntryLayer, frameIndex);
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
	_sceneLayers.reset();
	_realtimeAnimationTracks.reset(_ambientTrack);
	_entryIdleChannel.reset(kScene2110EntrySpeechBaseFrameA,
		kScene2110PrimarySpeechFrameMillis);
	_entryIdleActive = false;
}

void Scene2110::advanceEntryIdle(uint32 delta) {
	if (!_entryIdleActive)
		return;

	const uint frameCount = _entryIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_sceneLayers.layerFrame(kScene2110EntryLayer) == kScene2110EntrySpeechBaseFrameA + 4) {
			_sceneLayers.setLayerFrame(kScene2110EntryLayer, kScene2110EntrySpeechBaseFrameA);
		} else if (_random.getRandomNumber(14) == 0) {
			_sceneLayers.setLayerFrame(kScene2110EntryLayer, kScene2110EntrySpeechBaseFrameA + 4);
		} else {
			_sceneLayers.setLayerFrame(kScene2110EntryLayer, kScene2110EntrySpeechBaseFrameA);
		}
	}
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

	BlockingSequence sequence(*this);
	runEntrySecondarySpeechLine(0);
	sequence.presentedLayerFrames(kScene2110EntryLayer,
		AnimationFrameRange(0x12, 0x16, kScene2110FrameMillis).unskippable());
	if (!sequence.completed())
		return;
	runEntryPrimarySpeechLine(1, kScene2110EntrySpeechGroupB);
	sequence.presentedLayerFrames(kScene2110EntryLayer,
		AnimationFrameRange(0x1a, 0x1e, kScene2110FrameMillis).unskippable());
	if (!sequence.completed())
		return;
	runEntrySecondarySpeechLine(2);
	if (!sequence.completed())
		return;
	runEntryPrimarySpeechLine(3, kScene2110EntrySpeechGroupA);
	_sceneLayers.setLayerVisible(kScene2110EntryLayer, false);
	if (!sequence.completed())
		return;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	invalidatePresentationPalette();
	presentFrame();

	sequence.commit(_vm->gameState().mainFlowStateId, kScene2100ReturnFromTreasureState);
}

bool Scene2110::runScriptedEntryOpening() {
	setActiveActorPose(0x320, 0x118, 4);
	_sceneLayers.setLayerVisible(kScene2110EntryLayer, true);
	_sceneLayers.resetLayer(kScene2110EntryLayer, 0);
	BlockingSequence sequence(*this);
	sequence.presentedLayerFrames(kScene2110EntryLayer,
		AnimationFrameRange(0, 0x0e, kScene2110FrameMillis)
			.actorPathAt(0x0a, 0x263, 0x135, kScene2110InvalidFacing, 0,
				kScene2110ActorPathFrameMillis)
			.unskippable());
	if (!sequence.completed())
		return false;
	finishConcurrentActorPath();
	if (!sequence.completed())
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

	_sceneLayers.setLayerVisible(kScene2110TreasureLayer, true);
	_sceneLayers.resetLayer(kScene2110TreasureLayer, 0);
	BlockingSequence sequence(*this);
	sequence.presentedLayerFrames(kScene2110TreasureLayer,
		AnimationFrameRange(0, 0x0a, kScene2110FrameMillis).unskippable());
	if (!sequence.completed()) {
		_sceneLayers.setLayerVisible(kScene2110TreasureLayer, false);
		return;
	}

	if (grantsReward) {
		runTreasurePrimarySpeechLine(0x16, (byte)(rewardIndex * 2));
		sequence.presentedLayerFrames(kScene2110TreasureLayer,
			AnimationFrameRange(0x0d, 0x23, kScene2110FrameMillis)
				.soundAt(0x18, 1).unskippable());
	} else {
		runTreasurePrimarySpeechLine(0x66, 0);
		sequence.presentedLayerFrames(kScene2110TreasureLayer,
			AnimationFrameRange(0x19, 0x23, kScene2110FrameMillis).unskippable());
	}

	_sceneLayers.setLayerVisible(kScene2110TreasureLayer, false);
	if (!sequence.completed())
		return;
	if (!grantsReward) {
		drawPlayableComposite();
		presentFrame();
		return;
	}

	const byte itemId = kScene2110TreasureGrantItems[rewardIndex];
	if (!hasInventoryItem(itemId))
		addInventoryItem(itemId);
	sequence.actorPose(SceneActorPose(0x194, 0x155, 3));
	drawPlayableComposite();
	presentFrame();
	sequence.secondarySpeech(0x16, (byte)(rewardIndex * 2 + 1));
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

void Scene2110::runEntrySecondarySpeechLine(byte frameIndex) {
	_sceneLayers.setLayerVisible(kScene2110EntryLayer, true);
	_sceneLayers.setLayerFrame(kScene2110EntryLayer, kScene2110EntrySpeechBaseFrameA);
	_entryIdleChannel.reset(kScene2110EntrySpeechBaseFrameA,
		kScene2110PrimarySpeechFrameMillis);
	_entryIdleActive = true;
	beginSecondarySpeechLine(6, frameIndex);
	_entryIdleActive = false;
	_sceneLayers.setLayerFrame(kScene2110EntryLayer, kScene2110EntrySpeechBaseFrameA);
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
