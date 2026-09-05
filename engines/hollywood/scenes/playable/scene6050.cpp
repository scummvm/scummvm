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

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/scenes/playable/scene6050.h"

namespace Hollywood {

const uint16 kScene6050FirstState = 0x17a2;
const uint16 kScene6010ReturnFromScene6050State = 0x177d;
const uint16 kScene6060EntryState = 0x17ac;
const uint16 kScene6070EntryState = 0x17b6;
const uint16 kScene6050ViewportXOffset = 0x0100;
const uint16 kScene6050ViewportMinXOffset = 0x0080;
const uint kScene6050ActorBankTableEntry = 0x0000;
const uint kScene6050ActorPaletteTableEntry = 0x00cc;
const uint kScene6050Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6050SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6050GuardFrameMillis = 125;
const uint32 kScene6050GuardSpeechFrameMillis = 75;
const uint32 kScene6050ScriptFrameMillis = 40;
const uint32 kScene6050SecondaryScriptFrameMillis = 60;
const uint kScene6050GuardDescriptorCount = 0x16;
const uint kScene6050DoorSequenceDescriptorCount = 0x0b;
const uint kScene6050InteriorUnlockDescriptorCount = 0x0d;
const uint kScene6050DisplaySecondaryDescriptorCount = 0x0f;
const uint kScene6050DisplayDescriptorCount = 0x14;
const uint kScene6050DisplayPickupDescriptorCount = 0x0d;
const byte kScene6050DialogueStageId = 0x62;
const byte kScene6050GuardDialoguePrimaryRow = 99;
const uint kScene6050DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene6050DialogueTransitionEnd = 0;
const byte kScene6050DialogueTransitionStay = 3;
const byte kScene6050DialogueNoResponseFrame = 0xff;
const byte kScene6050DiamondInventoryItem = 0x4a;
const byte kScene6050WireInventoryItem = 0x5f;
const byte kScene6050WaxBallInventoryItem = 0x62;
const byte kScene6050DisplayCaseHook = 1;

enum {
	kScene6050GuardLayer,
	kScene6050SecondaryScriptLayer,
	kScene6050ScriptLayer
};

const byte kScene6050GuardFrameMap[] = {
	0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	21, 21, 21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 0, 7, 8, 9, 10, 11, 15, 16, 17,
	18, 19, 0, 1, 2, 3, 4, 5, 6, 7, 7, 12, 13, 14, 15
};

const byte kScene6050DoorSequenceFrameMap[] = {
	0, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 1, 0, 1, 2, 1, 0,
	1, 2, 1, 0, 1, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const byte kScene6050InteriorUnlockFrameMap[] = {
	0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12
};

const byte kScene6050DisplayFrameMap[] = {
	0, 18, 17, 16, 15, 14, 15, 16, 17, 18, 1, 2, 3, 4, 5, 6, 7, 7,
	8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 6, 5, 4, 3, 2, 1,
	18, 17, 16, 15, 14, 15, 16, 17, 18, 19
};

const byte kScene6050DisplaySecondaryFrameMap[] = {
	0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7
};

const SceneLayerSpec kScene6050LayerSpecs[] = {
	{ kSceneAnimationScenePlaced, 7, kScene6050GuardDescriptorCount,
		kScene6050GuardFrameMap, ARRAYSIZE(kScene6050GuardFrameMap), false, 0 },
	{ kSceneAnimationScenePlaced, 0, 0, nullptr, 0, false, 0 },
	{ kSceneAnimationScenePlaced, 0, 0, nullptr, 0, false, 0 }
};

PlayableSceneConfig scene6050Config() {
	PlayableSceneConfig config(6050,
		SceneResourceLayout(16, 5, 15),
		SceneViewport(kScene6050ViewportXOffset, kScene6050ViewportMinXOffset, kScene6050ViewportXOffset),
		SceneActorPose(0x307, 0x176, 4));
	config.setActorResources(kScene6050ActorBankTableEntry, kScene6050ActorPaletteTableEntry);
	config.setTextResources(kScene6050Resource003RowsOffsetIndex, kScene6050SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 2;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene6050::Scene6050(HollywoodEngine *vm) :
		PlayableScene(vm, scene6050Config()),
		_guardChannel(),
		_guardBlinkChannel(),
		_displayCaseSecondaryChannel(),
		_guardAnimationState(0),
		_guardManualSequenceActive(false),
		_guardGesturing(false),
		_scriptAnimationActive(false),
		_displayCaseSecondaryActive(false),
		_entryWalkPending(false) {
	_sceneLayers.configure(kScene6050LayerSpecs);
}

void Scene6050::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetGuardLayer();
	switch (_vm->gameState().mainFlowStateId) {
	case kScene6050FirstState + 1:
		_activeActorWorldX = 0x0ea;
		_activeActorWorldY = 0x15f;
		_activeActorFacing = 2;
		break;
	case kScene6050FirstState + 2:
		_activeActorWorldX = 0x2f2;
		_activeActorWorldY = 0x136;
		_activeActorFacing = 4;
		break;
	default:
		_activeActorWorldX = 0x307;
		_activeActorWorldY = 0x176;
		_activeActorFacing = 4;
		break;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene6050::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_scriptAnimationActive) {
		drawSceneLayer(kScene6050GuardLayer);
		drawSceneLayer(kScene6050SecondaryScriptLayer);
		drawSceneLayer(kScene6050ScriptLayer);
		return;
	}

	if (activeWorldY < 0x152) {
		drawActionOverlayLayer();
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		drawSceneLayer(kScene6050GuardLayer);
		if (_sceneChunkTable.isValidChunk(5))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
		return;
	}

	drawSceneLayer(kScene6050GuardLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (activeWorldY < 0x176 && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

void Scene6050::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene6050FirstState:
		_vm->gameState().currentAmbientMusicCueId = 0x0e;
		_vm->gameplayMusic()->playMusicCue(0x0e, 100);
		setActiveActorPose(0x3b0, 400, 4);
		_entryWalkPending = true;
		break;
	case kScene6050FirstState + 1:
		setActiveActorPose(0x050, 0x14c, 2);
		_entryWalkPending = true;
		break;
	case kScene6050FirstState + 2:
		_soundBank0.playSample(4, 100);
		setActiveActorPose(0x2f2, 0x136, 4);
		break;
	default:
		break;
	}
	centerViewportOnActor(_activeActorWorldX);
	drawPlayableComposite();
	fadePaletteFromBlack();
}

void Scene6050::runExitSideEffectsAfterLoop() {
	if (!didLeaveSceneAfterLoop())
		return;

	clearSpeechOverlay();
	drawPlayableComposite();
	if (fadePaletteToBlack())
		return;
	while (_speech.isPlaying() && !animationPlaybackShouldStop()) {
		if (waitDeltaClipFrameMillis(20))
			break;
	}
}

void Scene6050::prepareCustomGameplayLoop() {
	resetGuardLayer();
	if (_entryWalkPending) {
		_entryWalkPending = false;
		if (_vm->gameState().mainFlowStateId == kScene6050FirstState)
			startPlayerDirectedActorPath(0x307, 0x176, 0xff, 0, 0);
		else
			startPlayerDirectedActorPath(0x0ea, 0x15f, 0xff, 0, 0);
	}
}

void Scene6050::advanceCustomGameplayLoop(uint32 delta) {
	if (!_primaryDialogueSpeechActive && !_guardManualSequenceActive &&
			_vm->gameState().scene6050GuardPresent) {
		advanceGuardIdleLayer(delta);
	}
	if (_displayCaseSecondaryActive)
		advanceDisplayCaseSecondaryLayer(delta);
}

bool Scene6050::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 226: // Usar (use): no need.
		beginStaticSecondarySpeechLine(0xd5, (byte)_random.getRandomNumber(1));
		return true;
	case 227: // Coger manuscrito protegido (take protected manuscript): cannot take it.
		beginStaticSecondarySpeechLine(0xd6, (byte)_random.getRandomNumber(1));
		return true;
	case 301: // Mirar vitrina de Dr. Jekyll y Mr. Hyde (look at Jekyll/Hyde display).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Mirar vitrina del monstruo beisbolista (look at baseball-monster display).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Mirar manuscrito/cartel de la vitrina (look at manuscript/sign).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Ir a salida (go to exit): return to the studio lot.
		state.mainFlowStateId = kScene6010ReturnFromScene6050State;
		return true;
	case 305: // Mirar puerta al piso inferior (look at downstairs door).
		beginSecondarySpeechLine(state.scene6050GuardAllowsEntry ? 6 : 4, 0);
		return true;
	case 306: // Usar/abrir puerta principal (use/open main door): still closed.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar puerta al piso superior (look at upstairs door).
		beginSecondarySpeechLine(4, state.scene6050GuardAllowsEntry ? 1 : 0);
		return true;
	case 308: // Usar/abrir segunda puerta (use/open second door): still closed.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 309: // Hablar con guardia (talk to guard).
		runGuardDialogue();
		return true;
	case 310: // Mirar guardia (look at guard).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 311: // Ir al interior del museo (go to museum interior): guard or unlocked transition.
		if (state.scene6050MuseumInteriorUnlocked)
			state.mainFlowStateId = kScene6060EntryState;
		else
			runInteriorBlockedByGuard();
		return true;
	case 312: // Mirar interior del museo (look at museum interior).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 313: // Dar pase de Taffy al guardia (give Taffy's pass to guard).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Dar pase de Davenport al guardia (give Davenport's pass to guard).
		runMuseumInteriorUnlockSequence();
		return true;
	case 315: // Usar diamante con vitrina (cut the baseball display's glass).
		runDisplayCasePickup();
		return true;
	case 316: // Usar alambre con puerta al piso inferior (pick the downstairs lock).
		runExitAuthorizationSequence();
		return true;
	case 317: // Usar alambre con puerta al piso superior (try the upstairs lock).
		runInteriorDoorWireSequence();
		return true;
	default:
		return false;
	}
}

bool Scene6050::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x114, HollywoodEngine::kSceneBufferWidth - 1);
	targetY = CLIP<int>(targetY, 0, 0x1df);
	while (targetY < 0x1df) {
		++targetY;
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
	}
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;
	return true;
}

bool Scene6050::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return true;

	const GameplayState &state = _vm->gameState();
	if (selector == 0xff) {
		restoreBaseFramebufferFromOriginal();
		_paletteMask = _paletteMaskOriginal;
		rebuildWalkablePaletteMask();
	}
	if (selector == 1 || selector == 0xff) {
		for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
			const byte item = _paletteMaskOriginal[kSceneColorToItemMap + i];
			if (item == 7)
				_paletteMask[kSceneColorToItemMap + i] = state.scene6050GuardPresent ? 7 : 1;
			else if (item == 9)
				_paletteMask[kSceneColorToItemMap + i] = state.scene6050GuardPresent ? 7 : 0;
		}
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		_sceneLayers.setLayerVisible(kScene6050GuardLayer, state.scene6050GuardPresent);
		if (state.scene6050GuardPresent)
			_hotspots.setRelationMovementMode(kScene6050DiamondInventoryItem, 2, 1, 0);
	}
	if (selector == 0 || selector == 1 || selector == 0xff) {
		ScenePoint interactionPoint;
		interactionPoint.x = 0x114;
		interactionPoint.y = state.scene6050MuseumInteriorUnlocked ? 0x136 : 0x165;
		_hotspots.setActionInteraction(1, interactionPoint, state.scene6050MuseumInteriorUnlocked ? 1 : 0);
	}
	if (state.scene6050DisplayCaseOpened) {
		if ((selector == 3 || selector == 0xff) && _sceneChunkTable.isValidChunk(12))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
		if (selector == 1 || selector == 3 || selector == 0xff) {
			_hotspots.setRelationActionHandler(kScene6050DiamondInventoryItem, 2, 1, 226);
			_hotspots.setRelationMovementMode(kScene6050DiamondInventoryItem, 2, 1, 0);
		}
	}
	return true;
}

bool Scene6050::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene6050::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return _guardGesturing ? 45 : 40;
}

void Scene6050::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_sceneLayers.setLayerFrame(kScene6050GuardLayer, frameIndex);
}

void Scene6050::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	_sceneLayers.setLayerFrame(kScene6050GuardLayer, baseFrame);
	_guardBlinkChannel.resetTimer();
}

void Scene6050::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	if (hookId == kScene6050DisplayCaseHook) {
		_displayCaseSecondaryChannel.reset(7, kScene6050SecondaryScriptFrameMillis);
		_displayCaseSecondaryActive = true;
	}
}

AmbientAudioProfile Scene6050::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene6050::resetGuardLayer() {
	_sceneLayers.configure(kScene6050LayerSpecs);
	_guardChannel.reset(0, kScene6050GuardFrameMillis);
	_guardBlinkChannel.reset(0, kScene6050GuardSpeechFrameMillis);
	_sceneLayers.setLayerVisible(kScene6050GuardLayer,
		_vm->gameState().scene6050GuardPresent);
	_guardAnimationState = 0;
	_guardManualSequenceActive = false;
	_guardGesturing = false;
	_scriptAnimationActive = false;
	_displayCaseSecondaryActive = false;
}

void Scene6050::advanceGuardIdleLayer(uint32 delta) {
	ResourceSpriteLayer &guardLayer = _sceneLayers.layer(kScene6050GuardLayer);
	if (!guardLayer.visible)
		return;
	if (_guardAnimationState == 2) {
		const byte baseFrame = primarySpeechAnimationBaseFrame(0);
		const uint frameCount = _guardBlinkChannel.consumeFrames(delta);
		for (uint i = 0; i < frameCount; ++i) {
			if (guardLayer.frameIndex == baseFrame + 4)
				guardLayer.setFrame(baseFrame);
			else if (_random.getRandomNumber(14) == 0)
				guardLayer.setFrame(baseFrame + 4);
		}
		return;
	}

	const uint frameCount = _guardChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_guardAnimationState == 0) {
			if (_random.getRandomNumber(14) == 0) {
				guardLayer.setFrame(1);
				_guardAnimationState = 1;
			}
			continue;
		}

		if (guardLayer.frameIndex >= 0x27) {
			guardLayer.setFrame(0);
			_guardAnimationState = 0;
		} else {
			guardLayer.setFrame(guardLayer.frameIndex + 1);
		}
	}
}

bool Scene6050::runGuardFrameTransition(byte firstFrame, byte lastFrame, byte finalFrame) {
	if (!_vm->gameState().scene6050GuardPresent)
		return false;

	const bool previousManual = _guardManualSequenceActive;
	_guardManualSequenceActive = true;
	const bool completed = playAndPresentAnimationFrames(kScene6050GuardLayer,
		AnimationFrameRange(firstFrame, lastFrame, kScene6050GuardFrameMillis)
			.unskippable().noFinalFrameDelay());
	if (completed)
		_sceneLayers.setLayerFrame(kScene6050GuardLayer, finalFrame);
	_guardManualSequenceActive = previousManual;
	_guardBlinkChannel.resetTimer();
	return completed;
}

bool Scene6050::runGuardLookUpTransition() {
	if (_guardAnimationState == 2)
		return !animationPlaybackShouldStop();

	if (!runGuardFrameTransition(0x32, 0x39, 0x39))
		return false;
	_guardAnimationState = 2;
	return true;
}

bool Scene6050::runGuardLookDownTransition() {
	if (_guardAnimationState == 0)
		return !animationPlaybackShouldStop();

	if (!runGuardFrameTransition(0x39, 0x32, 0))
		return false;
	_guardAnimationState = 0;
	_guardGesturing = false;
	return true;
}

bool Scene6050::runGuardGestureTransition(bool raised) {
	if (_guardGesturing == raised)
		return !animationPlaybackShouldStop();
	if (!runGuardFrameTransition(raised ? 0x3a : 0x3e,
			raised ? 0x3e : 0x3a, raised ? 0x3e : 0x3a))
		return false;
	_guardGesturing = raised;
	return true;
}

void Scene6050::beginGuardSpeechLine(byte frameIndex) {
	beginSceneGuardSpeechLine(kScene6050GuardDialoguePrimaryRow, frameIndex);
}

void Scene6050::beginSceneGuardSpeechLine(uint16 rowIndex, byte frameIndex) {
	if (animationPlaybackShouldStop())
		return;
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex,
		0x15e, 0x6e, 0x30, 0x3f, 0, 0);
}

void Scene6050::advanceDisplayCaseSecondaryLayer(uint32 delta) {
	ResourceSpriteLayer &secondaryLayer = _sceneLayers.layer(kScene6050SecondaryScriptLayer);
	const uint frameCount = _displayCaseSecondaryChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (secondaryLayer.frameIndex >= 14) {
			applyDisplayCaseBackdropPatch();
			_displayCaseSecondaryActive = false;
			break;
		}
		secondaryLayer.setFrame(secondaryLayer.frameIndex + 1);
	}
}

void Scene6050::applyDisplayCaseBackdropPatch() {
	if (_sceneChunkTable.isValidChunk(11))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
}

void Scene6050::clearScriptLayers() {
	_displayCaseSecondaryActive = false;
	clearSceneLayer(kScene6050SecondaryScriptLayer);
	clearSceneLayer(kScene6050ScriptLayer);
	_scriptAnimationActive = false;
}

void Scene6050::runGuardDialogue() {
	if (!_vm->gameState().scene6050GuardPresent) {
		beginSecondarySpeechLine(7, 0);
		return;
	}

	Common::Array<DialogueChoiceRecord> records;
	initializeGuardDialogueRecords(records);

	beginSecondarySpeechLine(kScene6050DialogueStageId, 0);
	if (!runGuardLookUpTransition())
		return;
	_guardGesturing = false;
	beginGuardSpeechLine(0);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene6050DialogueStageId, records, depthIndex, nodeIndex);
		if (animationPlaybackShouldStop())
			return;
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene6050DialogueStageId, 2);
			break;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene6050DialogueStageId, record.playerTextRowId);
		if (animationPlaybackShouldStop())
			return;
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != kScene6050DialogueNoResponseFrame) {
			if (!runGuardGestureTransition(_random.getRandomNumber(1) != 0))
				return;
			beginGuardSpeechLine(record.responseFrameIndex);
			if (!runGuardGestureTransition(false))
				return;
		}

		if (record.disableAfterUse != 0)
			record.enabled = 0;

		if (record.transitionMode == kScene6050DialogueTransitionEnd)
			finished = true;
	}

	runGuardLookDownTransition();
}

void Scene6050::initializeGuardDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene6050DialogueChoiceRecordCount);

	// Guard dialogue root choices.
	setGuardDialogueRecord(records, 0, 1, 0, kScene6050DialogueTransitionStay, 1, 1, 1);
	setGuardDialogueRecord(records, 1, 1, 0, kScene6050DialogueTransitionStay, 3, 3, 1);
	setGuardDialogueRecord(records, 2, 1, 0, kScene6050DialogueTransitionStay, 4, 4, 1);
	setGuardDialogueRecord(records, 3, 1, 0, kScene6050DialogueTransitionStay, 5, 5, 1);
	setGuardDialogueRecord(records, 4, 1, 0, kScene6050DialogueTransitionEnd, 2, 0, 1);
}

void Scene6050::setGuardDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
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
}

void Scene6050::runInteriorBlockedByGuard() {
	if (!_vm->gameState().scene6050GuardPresent) {
		beginSecondarySpeechLine(9, 0);
		return;
	}

	if (!runGuardLookUpTransition())
		return;
	_guardGesturing = false;
	beginSceneGuardSpeechLine(8, 0);
	BlockingSequence sequence(*this);
	sequence.actorPath(SceneActorPose(0x114, 0x165, 1));
	if (sequence.completed())
		runGuardDialogue();
}

void Scene6050::runMuseumInteriorUnlockSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene6050MuseumInteriorUnlocked) {
		beginSecondarySpeechLine(11, 4);
		return;
	}

	beginSecondarySpeechLine(11, 5);
	if (!runGuardLookUpTransition())
		return;
	_guardGesturing = false;
	_scriptAnimationActive = true;
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene6050ScriptLayer, 10,
		kScene6050InteriorUnlockDescriptorCount,
		kScene6050InteriorUnlockFrameMap,
		AnimationFrameRange(0, 12, kScene6050ScriptFrameMillis).unskippable(), false);
	if (sequence.completed())
		beginSceneGuardSpeechLine(11, 0);
	sequence.resourceLayerFrames(kScene6050ScriptLayer, 10,
		kScene6050InteriorUnlockDescriptorCount,
		kScene6050InteriorUnlockFrameMap,
		AnimationFrameRange(13, ARRAYSIZE(kScene6050InteriorUnlockFrameMap) - 1,
			kScene6050ScriptFrameMillis).unskippable().noFinalFrameDelay());
	clearScriptLayers();
	if (!sequence.completed())
		return;

	sequence.secondarySpeech(11, 1);
	if (!runGuardGestureTransition(true))
		return;
	beginSceneGuardSpeechLine(11, 2);
	if (!runGuardGestureTransition(false))
		return;
	sequence.secondarySpeech(11, 3);
	if (runGuardLookDownTransition())
		sequence.commit(state.scene6050MuseumInteriorUnlocked, true).framebufferPatch(0);
}

void Scene6050::runDisplayCasePickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene6050GuardPresent) {
		beginSecondarySpeechLine(12, 2);
		return;
	}

	if (!state.scene6050DisplayCaseOpened) {
		_scriptAnimationActive = true;
		_sceneLayers.setLayerResource(kScene6050SecondaryScriptLayer, 13,
			kScene6050DisplaySecondaryDescriptorCount, nullptr, 0);
		_sceneLayers.setLayerVisible(kScene6050SecondaryScriptLayer, false);
		BlockingSequence sequence(*this);
		sequence.resourceLayerFrames(kScene6050ScriptLayer, 14,
			kScene6050DisplayDescriptorCount,
			kScene6050DisplayFrameMap,
			AnimationFrameRange(0, ARRAYSIZE(kScene6050DisplayFrameMap) - 1,
				kScene6050ScriptFrameMillis)
				.unskippable().noFinalFrameDelay()
				.layerVisibleAt(16, kScene6050SecondaryScriptLayer, true)
				.visibleMappedLayerFrames(kScene6050SecondaryScriptLayer,
					kScene6050DisplaySecondaryFrameMap,
					ARRAYSIZE(kScene6050DisplaySecondaryFrameMap), 16)
				.hookAt(30, kScene6050DisplayCaseHook)
				.resourcePatchAt(31, 11));
		clearScriptLayers();
		if (!sequence.completed())
			return;

		sequence.actorPath(SceneActorPose(0x1a6, 0x15e, 3))
			.secondarySpeech(12, 0)
			.actorPath(SceneActorPose(0x1d0, 0x143, 1));
		if (!sequence.completed())
			return;

		_scriptAnimationActive = true;
		sequence.resourceLayerFrames(kScene6050ScriptLayer, 15,
			kScene6050DisplayPickupDescriptorCount,
			AnimationFrameRange(kScene6050DisplayPickupDescriptorCount,
				kScene6050ScriptFrameMillis).holdFirstFrame().unskippable().noFinalFrameDelay()
				.commitAt(6, state.scene6050DisplayCaseOpened, true).patchAt(6, 3));
		clearScriptLayers();
		if (!sequence.completed())
			return;

		addInventoryItem(kScene6050WaxBallInventoryItem);
		sequence.sound(1)
			.actorPath(SceneActorPose(0x1a6, 0x15e, 3))
			.secondarySpeech(12, 1);
		return;
	}

	beginStaticSecondarySpeechLine(0xd5, (byte)_random.getRandomNumber(1));
}

void Scene6050::runExitAuthorizationSequence() {
	GameplayState &state = _vm->gameState();
	_scriptAnimationActive = true;
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene6050ScriptLayer, 8,
		kScene6050DoorSequenceDescriptorCount,
		kScene6050DoorSequenceFrameMap,
		AnimationFrameRange(0, 24, kScene6050ScriptFrameMillis).unskippable(), false);
	if (!sequence.completed()) {
		clearScriptLayers();
		return;
	}

	if (state.inventoryItemResourcePageByOwnerAndItemId[0][kScene6050WireInventoryItem] == 0x40 &&
			!state.scene6050GuardAllowsEntry) {
		startSecondarySpeechLine(13, 1);
		state.scene6050GuardAllowsEntry = true;
	}

	if (state.scene6050GuardAllowsEntry) {
		sequence.resourceLayerFrames(kScene6050ScriptLayer, 8,
			kScene6050DoorSequenceDescriptorCount,
			kScene6050DoorSequenceFrameMap,
			AnimationFrameRange(25, 27, kScene6050ScriptFrameMillis).unskippable(), false);
		if (!sequence.completed()) {
			clearScriptLayers();
			return;
		}
		sequence.sound(3)
			.commit(state.mainFlowStateId, kScene6070EntryState);
		return;
	}

	sequence.resourceLayerFrames(kScene6050ScriptLayer, 8,
		kScene6050DoorSequenceDescriptorCount,
		kScene6050DoorSequenceFrameMap,
		AnimationFrameRange(25, ARRAYSIZE(kScene6050DoorSequenceFrameMap) - 1,
			kScene6050ScriptFrameMillis).unskippable().noFinalFrameDelay());
	clearScriptLayers();
	sequence.secondarySpeech(13, 0);
}

void Scene6050::runInteriorDoorWireSequence() {
	_scriptAnimationActive = true;
	BlockingSequence sequence(*this);
	sequence.resourceLayerFrames(kScene6050ScriptLayer, 9,
		kScene6050DoorSequenceDescriptorCount,
		kScene6050DoorSequenceFrameMap,
		AnimationFrameRange(0, ARRAYSIZE(kScene6050DoorSequenceFrameMap) - 1,
			kScene6050ScriptFrameMillis).unskippable().noFinalFrameDelay());
	clearScriptLayers();
	sequence.secondarySpeech(13, 0);
}

} // End of namespace Hollywood
