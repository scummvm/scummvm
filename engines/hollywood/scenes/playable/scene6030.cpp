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

#include "hollywood/scenes/playable/scene6030.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene6020ReturnFromScene6030State = 0x1785;
const uint16 kScene6030ViewportXOffset = 0x0064;
const uint kScene6030ActorBankTableEntry = 0x0000;
const uint kScene6030ActorPaletteTableEntry = 0x00cc;
const uint kScene6030Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6030TaffyFrameMillis = 50;
const uint32 kScene6030PoseFrameMillis = 75;
const uint32 kScene6030SpeechFrameMillis = 125;
const uint32 kScene6030ActorPathFrameMillis = 60;
const uint kScene6030LargeLayerDescriptorCount = 0x1b;
const uint kScene6030BathroomExitDescriptorCount = 0x0e;
const uint kScene6030SmallLayerDescriptorCount = 3;
const uint kScene6030TaffyArrivalDescriptorCount = 0x45;
const uint kScene6030TaffyDepartureDescriptorCount = 0x2a;
const uint kScene6030SaxophoneDescriptorCount = 0x0c;
const byte kScene6030SaxophoneItemId = 0x5c;
const byte kScene6030PrimarySpeechTextColor = 0xfb;
const byte kScene6030DialogueStageId = 0x62;
const byte kScene6030DialoguePrimaryRow = 99;
const uint kScene6030DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene6030DialogueTransitionEnd = 0;
const byte kScene6030DialogueTransitionDown = 1;
const byte kScene6030DialogueTransitionUp = 2;
const byte kScene6030DialogueTransitionStay = 3;
const byte kScene6030DialogueTransitionUpTwo = 4;
const byte kScene6030DialogueNoResponseFrame = 0xff;

const byte kScene6030LargeLayerFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 11, 12, 13, 14,
	15, 16, 0, 17, 18, 19, 20, 21, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 21, 20, 19, 18, 17, 0, 23, 0, 23,
	24, 23, 0, 23, 24, 25, 24, 23, 24, 25, 26, 25, 24, 25, 26, 26,
	26, 26, 26, 0, 0, 0, 0, 0
};

const byte kScene6030SmallLayerFrameMap[] = {
	0, 1, 2
};

const byte kScene6030SaxophoneFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

enum Scene6030AnimationLayerId {
	kScene6030HannoverLayer,
	kScene6030BathroomExitLayer,
	kScene6030CoffeeCupLayer,
	kScene6030TaffyServiceLayer
};

enum Scene6030AnimationHookId {
	kScene6030TaffyPlacesCoffeeHook = 1,
	kScene6030HannoverRaisesCoffeeHook,
	kScene6030HannoverLowersCoffeeHook,
	kScene6030BathroomExitVolumeHook
};

const SceneAnimationLayerSpec kScene6030AnimationLayerSpecs[] = {
	{ kSceneAnimationBehindActors, 6, kScene6030LargeLayerDescriptorCount,
		kScene6030LargeLayerFrameMap, ARRAYSIZE(kScene6030LargeLayerFrameMap), true },
	{ kSceneAnimationBehindActors, 7, kScene6030BathroomExitDescriptorCount,
		nullptr, 0, false },
	{ kSceneAnimationBehindActors, 8, kScene6030SmallLayerDescriptorCount,
		kScene6030SmallLayerFrameMap, ARRAYSIZE(kScene6030SmallLayerFrameMap), true },
	{ kSceneAnimationInFrontOfActors, 9, kScene6030TaffyArrivalDescriptorCount,
		nullptr, 0, false }
};

static PlayableSceneConfig scene6030Config() {
	PlayableSceneConfig config(6030,
		SceneResourceLayout(13, 5, 12),
		SceneViewport(kScene6030ViewportXOffset),
		SceneActorPose(0x1e5, 0x186, 5));
	config.setActorResources(kScene6030ActorBankTableEntry, kScene6030ActorPaletteTableEntry);
	config.setTextResources(kScene6030Resource003RowsOffsetIndex, kScene6030SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene6030::Scene6030(HollywoodEngine *vm) :
		PlayableScene(vm, scene6030Config()),
		_hannoverIdleChannel(),
		_scriptedActorPathChannel(),
		_animationLayers(),
		_scriptedActorPathFrameIndex(0),
		_hannoverManualSequenceActive(false),
		_scriptedActorPathActive(false) {
	_animationLayers.configure(kScene6030AnimationLayerSpecs);
}

void Scene6030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	if (!hasSavedActiveActorPoseForCurrentState())
		_vm->gameState().scene6030HannoverAbsent = false;
	resetAnimationLayers();
	setActiveActorPose(0x1e5, 0x186, 5);
}

void Scene6030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawAnimationLayers(_animationLayers, kSceneAnimationBehindActors);
	if (_actionOverlayPlayer.isVisible() &&
			_actionOverlayPlayer.stratum == kSceneAnimationBehindActors)
		drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldX, activeWorldY);
	drawAnimationLayers(_animationLayers, kSceneAnimationInFrontOfActors);
	if (_actionOverlayPlayer.isVisible() &&
			_actionOverlayPlayer.stratum != kSceneAnimationBehindActors)
		drawActionOverlayLayer();
}

void Scene6030::runCustomEntrySequence() {
	_vm->gameState().scene6030HannoverAbsent = false;
	resetAnimationLayers();
	_soundBank0.playSample(4, 100);
	setActiveActorPose(0x384, 0x1de, 5);
	drawPlayableComposite();
	presentFrame();
	startScriptedActorPath(0x1e5, 0x186, 5);
	runEntryConversation();
}

bool Scene6030::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene6030::advanceCustomGameplayLoop(uint32 delta) {
	advanceScriptedActorPath(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else if (!_hannoverManualSequenceActive &&
			_animationLayers.layer(kScene6030HannoverLayer).visible)
		advanceHannoverLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene6030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar silla (look at chair).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar silla (use chair): Ron refuses to sit.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Mirar sillon de Hannover (look at Hannover's armchair).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Usar sillon de Hannover (use Hannover's armchair): Hannover may return.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 305: // Coger saxofon (take the saxophone).
		runSaxophonePickupSequence();
		return true;
	case 306: // Mirar saxofon (look at saxophone).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Usar saxofon (use saxophone): bad time to play.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Ir a oficina de Taffy / puerta (go to Taffy's office/door): return scene 6020.
		returnToScene6020();
		return true;
	case 309: // Mirar puerta/oficina de Taffy (look at door/Taffy's office).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar cafe (look at coffee), state-aware after margarita.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Usar/coger cafe (use/take coffee): Ron refuses.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Mirar cafe tras margarita (look at coffee after daisy effect).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313: // Mirar palos de golf (look at golf clubs).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314: // Mirar telefono (look at telephone).
		beginSecondarySpeechLine(12, 0);
		return true;
	case 315: // Usar telefono (use telephone): no need to call.
		beginSecondarySpeechLine(13, 0);
		return true;
	case 316: // Usar telefono (use telephone), duplicate scene action.
		beginSecondarySpeechLine(14, 0);
		return true;
	default:
		return false;
	}
}

bool Scene6030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector != 1 && selector != 0xff)
		return true;
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	if (_vm->gameState().scene6030SaxophoneTaken) {
		for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
			if (_paletteMaskOriginal[kSceneColorToItemMap + i] == 3)
				_paletteMask[kSceneColorToItemMap + i] = 0;
		}
		if (_sceneChunkTable.isValidChunk(5))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
	}

	rebuildWorkingWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

bool Scene6030::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene6030::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == 2)
		return 0x0b;
	return 0;
}

uint32 Scene6030::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene6030SpeechFrameMillis;
}

void Scene6030::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_animationLayers.setLayerFrame(kScene6030HannoverLayer, frameIndex);
}

void Scene6030::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	switch (hookId) {
	case kScene6030TaffyPlacesCoffeeHook:
	case kScene6030HannoverLowersCoffeeHook:
		_animationLayers.setLayerFrame(kScene6030CoffeeCupLayer, 1);
		break;
	case kScene6030HannoverRaisesCoffeeHook:
		_animationLayers.setLayerFrame(kScene6030CoffeeCupLayer, 2);
		break;
	case kScene6030BathroomExitVolumeHook:
		_speech.setVolume(25);
		break;
	default:
		PlayableScene::handleAnimationFrameHook(hookId, frame);
		break;
	}
}

AmbientAudioProfile Scene6030::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	if (_vm->gameState().scene6030CoffeeState == 2) {
		profile.soundMode = kAmbientSoundRandomRange;
		profile.soundFirstCueId = 0x11;
		profile.soundCueCount = 1;
		profile.soundVolumePercent = 50;
		profile.soundProbabilityModulus = 60;
	}
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene6030::rebuildWorkingWalkableMask() {
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 2)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene6030::resetAnimationLayers() {
	_hannoverIdleChannel.reset(0, kScene6030SpeechFrameMillis);
	_scriptedActorPathChannel.reset(0, kScene6030ActorPathFrameMillis);
	_animationLayers.setLayerFrame(kScene6030HannoverLayer, 0);
	_animationLayers.setLayerFrame(kScene6030BathroomExitLayer, 0);
	_animationLayers.setLayerFrame(kScene6030CoffeeCupLayer,
		_vm->gameState().scene6030HannoverAbsent ? 1 : 0);
	_animationLayers.setLayerFrame(kScene6030TaffyServiceLayer, 0);
	_animationLayers.setLayerVisible(kScene6030HannoverLayer,
		!_vm->gameState().scene6030HannoverAbsent);
	_animationLayers.setLayerVisible(kScene6030BathroomExitLayer, false);
	_animationLayers.setLayerVisible(kScene6030CoffeeCupLayer, true);
	_animationLayers.setLayerVisible(kScene6030TaffyServiceLayer, false);
	_scriptedActorPathFrameIndex = 0;
	_hannoverManualSequenceActive = false;
	_scriptedActorPathActive = false;
	_actorPathPlaybackActive = false;
}

void Scene6030::advanceHannoverLayer(uint32 delta) {
	ResourceSpriteLayer &hannoverLayer = _animationLayers.layer(kScene6030HannoverLayer);
	const uint frameCount = _hannoverIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (hannoverLayer.frameIndex == 0) {
			if (_random.getRandomNumber(14) == 0)
				hannoverLayer.setFrame(4);
		} else {
			hannoverLayer.setFrame(0);
		}
	}
}

void Scene6030::startScriptedActorPath(int targetX, int targetY, byte finalFacing) {
	queueActorPathWithPaletteRegionRouting(_activeActorWorldX, _activeActorWorldY,
		targetX, targetY, finalFacing, 0);
	_scriptedActorPathFrameIndex = 1;
	_scriptedActorPathChannel.reset(0, kScene6030ActorPathFrameMillis);
	_lastViewportScrollActorWorldX = _activeActorWorldX;
	_scriptedActorPathActive = _actorPathFrames.size() > 1;
	_actorPathPlaybackActive = _scriptedActorPathActive;
	if (!_scriptedActorPathActive) {
		if (!_actorPathFrames.empty()) {
			const ActorPathFrame &frame = _actorPathFrames.back();
			_activeActorWorldX = frame.worldX;
			_activeActorWorldY = frame.worldY;
			_activeActorFacing = frame.facing;
			_activeActorCel = frame.cel;
			_activeActorDrawOrderMode = frame.drawOrderMode;
		} else {
			setActiveActorPose(targetX, targetY, finalFacing);
		}
	}
}

void Scene6030::advanceScriptedActorPath(uint32 delta) {
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

void Scene6030::finishScriptedActorPath() {
	while (_scriptedActorPathActive && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10))
			break;
	}
	if (_scriptedActorPathActive && !animationPlaybackShouldStop() &&
			!_actorPathFrames.empty()) {
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

void Scene6030::drawForegroundBlocks(int activeWorldX, int activeWorldY) {
	if (activeWorldY < 0x184 && activeWorldX > 0x200 && _sceneChunkTable.isValidChunk(11))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _sceneFramebuffer);
}

void Scene6030::runEntryConversation() {
	GameplayState &state = _vm->gameState();
	if (!state.scene6030HannoverInterviewCompleted) {
		beginHannoverSpeechLine(0);
		finishScriptedActorPath();
		beginSecondarySpeechLine(kScene6030DialogueStageId, 0);
		beginHannoverSpeechLine(1);
		beginHannoverSpeechLine(3);
		runHannoverPoseTransition(false);
		beginHannoverSpeechLine(5, 2);
		runHannoverPoseTransition(true);
		beginHannoverSpeechLine(6);
		runTaffyCoffeeServiceSequence();
		if (animationPlaybackShouldStop())
			return;
		runHannoverDialogueMenu();
		state.scene6030HannoverInterviewCompleted = true;
		leaveHannoverOffice();
		return;
	}

	beginHannoverSpeechLine(2);
	finishScriptedActorPath();
	beginHannoverSpeechLine(state.scene6030CoffeeState < 2 ? 3 : 4);
	runHannoverPoseTransition(false);
	beginHannoverSpeechLine(5, 2);
	runHannoverPoseTransition(true);
	beginHannoverSpeechLine(7);
	runTaffyCoffeeServiceSequence();
	if (animationPlaybackShouldStop())
		return;
	if (state.scene6030CoffeeState == 0) {
		runHannoverDialogueMenu();
		leaveHannoverOffice();
		return;
	}

	runHannoverBathroomExitSequence();
	if (animationPlaybackShouldStop())
		return;
	if (state.scene6030CoffeeState == 1)
		state.scene6030CoffeeState = 2;
	walkActiveActorTo(0x1e5, 0x186, 1, 0, false);
	beginSecondarySpeechLine(10, 0);
}

void Scene6030::runHannoverDialogueMenu() {
	Common::Array<DialogueChoiceRecord> records;
	initializeHannoverDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene6030DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene6030DialogueStageId, 0x0e);
			beginHannoverSpeechLine(0x0e);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene6030DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene6030DialogueNoResponseFrame)
			beginHannoverSpeechLine(record.responseFrameIndex);

		if (record.disableAfterUse != 0)
			record.enabled = 0;

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case kScene6030DialogueTransitionEnd:
			finished = true;
			break;
		case kScene6030DialogueTransitionDown:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth + 1;
			break;
		case kScene6030DialogueTransitionUp:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 1;
			break;
		case kScene6030DialogueTransitionUpTwo:
			nodeIndex = record.nextNodeIndex;
			depthIndex = previousDepth - 2;
			break;
		case kScene6030DialogueTransitionStay:
		default:
			break;
		}
	}
}

void Scene6030::initializeHannoverDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene6030DialogueChoiceRecordCount);

	// DAT_0050cea0: Hannover interview root choices.
	setHannoverDialogueRecord(records, 0, 1, 0, kScene6030DialogueTransitionDown, 10, 10, 1);
	setHannoverDialogueRecord(records, 1, 1, 0, kScene6030DialogueTransitionStay, 11, 11, 1);
	setHannoverDialogueRecord(records, 2, 1, 0, kScene6030DialogueTransitionStay, 12, 12, 1);
	setHannoverDialogueRecord(records, 3, 1, 0, kScene6030DialogueTransitionStay, 13, 13, 1);
	setHannoverDialogueRecord(records, 4, 1, 0, kScene6030DialogueTransitionEnd, 14, 14, 1);

	// Depth 1, node 0.
	setHannoverDialogueRecord(records, 70, 1, 0, kScene6030DialogueTransitionStay, 15, 15, 1);
	setHannoverDialogueRecord(records, 71, 1, 0, kScene6030DialogueTransitionStay, 16, 16, 1);
	setHannoverDialogueRecord(records, 72, 1, 0, kScene6030DialogueTransitionStay, 17, 17, 1);
	setHannoverDialogueRecord(records, 73, 1, 0, kScene6030DialogueTransitionDown, 18, 18, 1);
	setHannoverDialogueRecord(records, 74, 1, 0, kScene6030DialogueTransitionUp, 19, 19, 0);

	// Depth 2, node 0.
	setHannoverDialogueRecord(records, 140, 1, 0, kScene6030DialogueTransitionStay, 20, 20, 1);
	setHannoverDialogueRecord(records, 141, 1, 0, kScene6030DialogueTransitionStay, 21, 21, 1);
	setHannoverDialogueRecord(records, 142, 1, 0, kScene6030DialogueTransitionStay, 22, 22, 1);
	setHannoverDialogueRecord(records, 143, 1, 0, kScene6030DialogueTransitionUp, 23, 19, 0);
}

void Scene6030::setHannoverDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
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

void Scene6030::beginHannoverSpeechLine(byte frameIndex, byte animationGroup) {
	beginPrimarySpeechLineWithAnimationGroup(kScene6030DialoguePrimaryRow, frameIndex,
		0x173, 0x0a9, 0x28, 0x16, 0x0b, animationGroup);
}

uint32 Scene6030::beginStaticHannoverSpeechLine(byte frameIndex, uint16 centerX, uint16 topY) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(kScene6030DialoguePrimaryRow, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return 0;

	const Common::String text = getResource003LargeTextRecord(textRecordId);
	if (text.empty())
		return 0;

	setPaletteEntry6Bit(kScene6030PrimarySpeechTextColor, 0x28, 0x16, 0x0b);
	_primarySpeechOverlay.visible = true;
	_primarySpeechOverlay.colorIndex = kScene6030PrimarySpeechTextColor;
	wrapActorSpeechText(text, centerX, _primarySpeechOverlay.lines);
	calculateSpeechOverlayBounds(_primarySpeechOverlay, centerX, topY, true, _activeActorWorldY);

	const bool started = voiceSampleId != 0 && _speech.playSample(voiceSampleId, 100);
	return started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(1200, MAX<byte>(1, continuationCount) * _primarySpeechOverlay.lines.size() * 1100);
}

void Scene6030::runHannoverFrameTransition(byte firstFrame, byte lastFrame, byte finalFrame,
		uint32 frameMillis) {
	const bool previousManualSequence = _hannoverManualSequenceActive;
	_hannoverManualSequenceActive = true;
	playAnimationTransition(_animationLayers, kScene6030HannoverLayer,
		AnimationTransition(firstFrame, lastFrame, finalFrame, frameMillis).unskippable());
	_hannoverManualSequenceActive = previousManualSequence;
}

void Scene6030::runHannoverPoseTransition(bool alternatePose) {
	runHannoverFrameTransition(alternatePose ? 0x10 : 5, alternatePose ? 0x12 : 0x0a,
		alternatePose ? 0 : 0x0b, kScene6030PoseFrameMillis);
}

void Scene6030::runTaffyCoffeeServiceSequence() {
	const uint deskOffset = 0x17d * HollywoodEngine::kSceneBufferWidth + 0x25b;
	const byte deskColor = savedFramebufferPixelAt(deskOffset);
	const bool hasDeskRegion = deskColor < _fullPaletteRegionMask.size();
	const byte previousDeskRegion = hasDeskRegion ? _fullPaletteRegionMask[deskColor] : 0;
	// The desk is not floor, but Ron must route onto it during this scripted walk.
	if (hasDeskRegion)
		_fullPaletteRegionMask[deskColor] = 1;

	startScriptedActorPath(0x25b, 0x17d, 4);
	_animationLayers.configureLayerResource(kScene6030TaffyServiceLayer, 9,
		kScene6030TaffyArrivalDescriptorCount, nullptr, 0);
	bool completed = playAnimationFrames(_animationLayers, kScene6030TaffyServiceLayer,
		AnimationFrameRange(0, 0x44, kScene6030TaffyFrameMillis)
			.unskippable().hookAt(0x42, kScene6030TaffyPlacesCoffeeHook));
	if (completed) {
		_animationLayers.configureLayerResource(kScene6030TaffyServiceLayer, 10,
			kScene6030TaffyDepartureDescriptorCount, nullptr, 0);
		completed = playAnimationFrames(_animationLayers, kScene6030TaffyServiceLayer,
			AnimationFrameRange(0, 0x29, kScene6030TaffyFrameMillis).unskippable());
	}
	_animationLayers.setLayerVisible(kScene6030TaffyServiceLayer, false);
	if (!completed) {
		_scriptedActorPathActive = false;
		_actorPathPlaybackActive = false;
		if (hasDeskRegion)
			_fullPaletteRegionMask[deskColor] = previousDeskRegion;
		return;
	}

	finishScriptedActorPath();
	if (animationPlaybackShouldStop()) {
		if (hasDeskRegion)
			_fullPaletteRegionMask[deskColor] = previousDeskRegion;
		return;
	}

	startScriptedActorPath(0x1e5, 0x186, 5);
	beginHannoverSpeechLine(8);
	finishScriptedActorPath();
	if (hasDeskRegion)
		_fullPaletteRegionMask[deskColor] = previousDeskRegion;
	if (animationPlaybackShouldStop())
		return;

	runHannoverCoffeeSequence();
}

void Scene6030::runHannoverCoffeeSequence() {
	const bool previousManualSequence = _hannoverManualSequenceActive;
	_hannoverManualSequenceActive = true;
	bool completed = playAnimationFrames(_animationLayers, kScene6030HannoverLayer,
		AnimationFrameRange(0x13, 0x16, kScene6030PoseFrameMillis)
			.unskippable().hookAt(0x16, kScene6030HannoverRaisesCoffeeHook));
	if (completed) {
		completed = playAnimationFrames(_animationLayers, kScene6030HannoverLayer,
			AnimationFrameRange(0x17, 0x29, kScene6030PoseFrameMillis)
				.unskippable().hookAt(0x29, kScene6030HannoverLowersCoffeeHook));
	}
	if (completed) {
		playAnimationFrames(_animationLayers, kScene6030HannoverLayer,
			AnimationFrameRange(0x2a, 0x2c, kScene6030PoseFrameMillis).unskippable());
	}
	_hannoverManualSequenceActive = previousManualSequence;
}

void Scene6030::runHannoverBathroomExitSequence() {
	const bool previousManualSequence = _hannoverManualSequenceActive;
	_hannoverManualSequenceActive = true;
	runHannoverFrameTransition(0x2d, 0x3e, 0x3e, kScene6030PoseFrameMillis);
	if (animationPlaybackShouldStop()) {
		_hannoverManualSequenceActive = previousManualSequence;
		return;
	}

	_animationLayers.setLayerVisible(kScene6030HannoverLayer, false);
	_animationLayers.setLayerVisible(kScene6030BathroomExitLayer, true);
	_animationLayers.setLayerFrame(kScene6030BathroomExitLayer, 0);

	const uint32 speechDuration = beginStaticHannoverSpeechLine(9, 500, 100);
	const bool completed = playAnimationFrames(_animationLayers, kScene6030BathroomExitLayer,
		AnimationFrameRange(0, 0x0d, kScene6030PoseFrameMillis)
			.unskippable().hookAt(0x0d, kScene6030BathroomExitVolumeHook));
	uint32 elapsed = kScene6030BathroomExitDescriptorCount * kScene6030PoseFrameMillis;
	while (completed && (_speech.isPlaying() || elapsed < speechDuration) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10))
			break;
		elapsed += 10;
	}

	_animationLayers.setLayerVisible(kScene6030BathroomExitLayer, false);
	_speech.stop();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
	if (completed && !animationPlaybackShouldStop())
		_vm->gameState().scene6030HannoverAbsent = true;
	_hannoverManualSequenceActive = previousManualSequence;
}

void Scene6030::runSaxophonePickupSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene6030SaxophoneTaken)
		return;

	const bool previousHideActiveActor = _actionOverlayPlayer.beginActorReplacement(12,
		kScene6030SaxophoneDescriptorCount, kScene6030SaxophoneFrameMap,
		ARRAYSIZE(kScene6030SaxophoneFrameMap));
	const bool completed = playAnimationFrames(_actionOverlayPlayer,
		AnimationFrameRange(1, 0x0c, kScene6030PoseFrameMillis).unskippable());
	_actionOverlayPlayer.finish(previousHideActiveActor);
	if (!completed)
		return;

	state.scene6030SaxophoneTaken = true;
	applySceneStateToHotspotsAndPatches(1);
	addInventoryItem(kScene6030SaxophoneItemId);
	_soundBank0.playSample(1, 100);
	beginSharedInventorySpeechLine(0x14, randomSharedInventorySpeechFrame(4));
}

void Scene6030::leaveHannoverOffice() {
	walkActiveActorTo(0x384, 0x1de, 5, 0, false);
	returnToScene6020();
}

void Scene6030::returnToScene6020() {
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene6020ReturnFromScene6030State;
}

} // End of namespace Hollywood
