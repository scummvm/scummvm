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

#include "hollywood/scenes/playable/scene6050.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

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
const uint kScene6050GuardDescriptorCount = 0x16;
const byte kScene6050DialogueStageId = 0x62;
const byte kScene6050GuardDialoguePrimaryRow = 99;
const uint kScene6050DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene6050DialogueTransitionEnd = 0;
const byte kScene6050DialogueTransitionStay = 3;
const byte kScene6050DialogueNoResponseFrame = 0xff;
const byte kScene6050GuardPassInventoryItem = 0x62;

const byte kScene6050GuardFrameMap[] = {
	0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	21, 21, 21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 0, 7, 8, 9, 10, 11, 15, 16, 17,
	18, 19, 0, 1, 2, 3, 4, 5, 6, 7, 7, 12, 13, 14, 15, 0,
	0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3,
	4, 5, 6, 7, 8, 9, 10, 11, 12
};

static PlayableSceneConfig scene6050Config() {
	PlayableSceneConfig config(6050,
		SceneResourceLayout(16, 5, 15),
		SceneViewport(kScene6050ViewportXOffset, kScene6050ViewportMinXOffset, kScene6050ViewportXOffset),
		SceneActorPose(0x307, 0x176, 4));
	config.setActorResources(kScene6050ActorBankTableEntry, kScene6050ActorPaletteTableEntry);
	config.setTextResources(kScene6050Resource003RowsOffsetIndex, kScene6050SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 2;
	return config;
}

Scene6050::Scene6050(HollywoodEngine *vm) :
		PlayableScene(vm, scene6050Config()),
		_originalColorToItemMap(),
		_guardChannel(),
		_guardLayer(),
		_guardAnimationState(0),
		_guardManualSequenceActive(false),
		_guardConversationActive(false) {
	_guardLayer.configure(7, kScene6050GuardDescriptorCount,
		kScene6050GuardFrameMap, ARRAYSIZE(kScene6050GuardFrameMap));
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
	if (activeWorldY < 0x152) {
		drawActionOverlayLayer();
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		drawResourceSpriteLayer(_guardLayer);
		if (_sceneChunkTable.isValidChunk(5))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
		return;
	}

	drawResourceSpriteLayer(_guardLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (activeWorldY < 0x176 && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

void Scene6050::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene6050FirstState:
		_vm->gameplayMusic()->playMusicCue(0x0e, 100);
		runEntryPath(0x3b0, 400, 4, 0x307, 0x176);
		break;
	case kScene6050FirstState + 1:
		runEntryPath(0x050, 0x14c, 2, 0x0ea, 0x15f);
		break;
	case kScene6050FirstState + 2:
		_soundBank0.playSample(4, 100);
		drawPlayableComposite();
		presentFrame();
		break;
	default:
		PlayableScene::runCustomEntrySequence();
		break;
	}
}

bool Scene6050::prepareCustomGameplayLoop() {
	resetGuardLayer();
	return true;
}

bool Scene6050::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive) {
		advancePrimaryDialogueSpeechFrame(delta);
	} else if (!_guardManualSequenceActive && _vm->gameState().scene6050GuardPresent) {
		advanceGuardIdleLayer(delta);
	}
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene6050::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 226: // Usar manuscrito/vitrina sin condicion (generic use): no need.
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
	case 305: // Mirar puerta principal (look at main door), state-aware with guard pass.
		beginSecondarySpeechLine(1, state.scene6050GuardAllowsEntry ? 6 : 4);
		return true;
	case 306: // Usar/abrir puerta principal (use/open main door): still closed.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Mirar puerta/interior (look at interior door/stairs), state-aware with guard pass.
		beginSecondarySpeechLine(state.scene6050GuardAllowsEntry ? 6 : 1,
			state.scene6050GuardAllowsEntry ? 0 : 4);
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
	case 313: // Usar objeto con vitrina/manuscrito (use item with display): no direct effect.
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Usar pase/disfraz para poder entrar al interior (use pass/disguise sequence).
		runMuseumInteriorUnlockSequence();
		return true;
	case 315: // Coger pase/manuscrito de la vitrina (take pass/manuscript from display).
		runDisplayCasePickup();
		return true;
	case 316: // Usar pase en salida vigilada (use pass at guarded exit): authorization sequence.
		runExitAuthorizationSequence();
		return true;
	default:
		return false;
	}
}

bool Scene6050::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	rememberOriginalColorMap();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (!state.scene6050GuardPresent) {
		replaceColorMapItemFromOriginal(7, 1);
		replaceColorMapItemFromOriginal(9, 0);
	} else {
		replaceColorMapItemFromOriginal(9, 7);
	}

	if (!state.scene6050DisplayCaseOpened && _sceneChunkTable.isValidChunk(12))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
	if (hasInventoryItem(kScene6050GuardPassInventoryItem))
		replaceColorMapItemFromOriginal(3, 0);

	rebuildMuseumWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	ScenePoint interactionPoint;
	interactionPoint.x = 0x114;
	interactionPoint.y = state.scene6050MuseumInteriorUnlocked ? 0x136 : 0x165;
	ScenePoint approachPoint = interactionPoint;
	_hotspots.setActionTarget(1, interactionPoint, approachPoint);
	return true;
}

bool Scene6050::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene6050::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return 0x3a;
}

void Scene6050::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_guardLayer.setFrame(frameIndex);
}

void Scene6050::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	(void)baseFrame;
	_guardLayer.setFrame((_guardConversationActive || _guardAnimationState == 2) ? 0x39 : 0);
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
	_guardChannel.reset(0, kScene6050GuardFrameMillis);
	_guardLayer.reset(0);
	_guardLayer.visible = _vm->gameState().scene6050GuardPresent;
	_guardAnimationState = 0;
	_guardManualSequenceActive = false;
	_guardConversationActive = false;
}

void Scene6050::advanceGuardIdleLayer(uint32 delta) {
	if (!_guardLayer.visible || _guardAnimationState == 2)
		return;

	const uint frameCount = _guardChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_guardAnimationState == 0) {
			if (_random.getRandomNumber(14) == 0) {
				_guardLayer.setFrame(1);
				_guardAnimationState = 1;
			}
			continue;
		}

		if (_guardLayer.frameIndex >= 0x27) {
			_guardLayer.setFrame(0);
			_guardAnimationState = 0;
		} else {
			_guardLayer.setFrame(_guardLayer.frameIndex + 1);
		}
	}
}

void Scene6050::runGuardFrameTransition(byte firstFrame, byte lastFrame, byte finalFrame) {
	if (!_vm->gameState().scene6050GuardPresent)
		return;

	const bool previousManual = _guardManualSequenceActive;
	_guardManualSequenceActive = true;
	const bool completed = playAnimationTransition(_guardLayer,
		AnimationTransition(firstFrame, lastFrame, finalFrame, kScene6050GuardFrameMillis));
	if (!completed && !Engine::shouldQuit())
		_guardLayer.setFrame(finalFrame);
	_guardManualSequenceActive = previousManual;
}

void Scene6050::runGuardLookUpTransition() {
	if (_guardAnimationState == 2)
		return;

	runGuardFrameTransition(0x32, 0x39, 0x39);
	_guardAnimationState = 2;
}

void Scene6050::runGuardLookDownTransition() {
	if (_guardAnimationState == 0)
		return;

	runGuardFrameTransition(0x39, 0x32, 0);
	_guardAnimationState = 0;
}

void Scene6050::beginGuardSpeechLine(byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(kScene6050GuardDialoguePrimaryRow, frameIndex,
		0x15e, 0x6e, 0x30, 0x3f, 0, 0);
}

void Scene6050::runGuardDialogue() {
	if (!_vm->gameState().scene6050GuardPresent) {
		beginSecondarySpeechLine(7, 0);
		return;
	}

	Common::Array<DialogueChoiceRecord> records;
	initializeGuardDialogueRecords(records);

	beginSecondarySpeechLine(kScene6050DialogueStageId, 0);
	runGuardLookUpTransition();
	_guardConversationActive = true;
	beginGuardSpeechLine(0);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene6050DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene6050DialogueStageId, 2);
			break;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene6050DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != kScene6050DialogueNoResponseFrame)
			beginGuardSpeechLine(record.responseFrameIndex);

		if (record.disableAfterUse != 0)
			record.enabled = 0;

		if (record.transitionMode == kScene6050DialogueTransitionEnd)
			finished = true;
	}

	_guardConversationActive = false;
	runGuardLookDownTransition();
}

void Scene6050::initializeGuardDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene6050DialogueChoiceRecordCount);

	// DAT_0050e228: guard dialogue root choices.
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
	record.reserved = 0xff;
}

void Scene6050::runInteriorBlockedByGuard() {
	if (!_vm->gameState().scene6050GuardPresent) {
		beginSecondarySpeechLine(9, 0);
		return;
	}

	runGuardLookUpTransition();
	_guardConversationActive = true;
	beginGuardSpeechLine(8);
	_guardConversationActive = false;
	runGuardLookDownTransition();
	runGuardDialogue();
}

void Scene6050::runMuseumInteriorUnlockSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene6050MuseumInteriorUnlocked) {
		beginSecondarySpeechLine(11, 4);
		return;
	}

	beginSecondarySpeechLine(11, 5);
	runGuardLookUpTransition();
	_guardConversationActive = true;
	beginGuardSpeechLine(11);
	beginSecondarySpeechLine(11, 0);
	beginGuardSpeechLine(12);
	beginSecondarySpeechLine(11, 1);
	beginGuardSpeechLine(13);
	beginSecondarySpeechLine(11, 2);
	beginGuardSpeechLine(14);
	beginSecondarySpeechLine(11, 3);
	_guardConversationActive = false;
	runGuardLookDownTransition();
	state.scene6050MuseumInteriorUnlocked = true;
	applySceneStateToHotspotsAndPatches(0);
}

void Scene6050::runDisplayCasePickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene6050GuardPresent) {
		beginSecondarySpeechLine(12, 2);
		return;
	}

	if (!state.scene6050DisplayCaseOpened) {
		beginSecondarySpeechLine(12, 0);
		state.scene6050DisplayCaseOpened = true;
		applySceneStateToHotspotsAndPatches(3);
		return;
	}

	if (hasInventoryItem(kScene6050GuardPassInventoryItem)) {
		dispatchGenericSceneAction(6);
		return;
	}

	addInventoryItem(kScene6050GuardPassInventoryItem);
	_soundBank0.playSample(1, 100);
	applySceneStateToHotspotsAndPatches(0xff);
	dispatchGenericSceneAction(21);
}

void Scene6050::runExitAuthorizationSequence() {
	GameplayState &state = _vm->gameState();
	if (!state.scene6050GuardAllowsEntry) {
		beginSecondarySpeechLine(13, 1);
		state.scene6050GuardAllowsEntry = true;
		applySceneStateToHotspotsAndPatches(0);
		return;
	}

	_soundBank0.playSample(3, 100);
	state.mainFlowStateId = kScene6070EntryState;
}

void Scene6050::rememberOriginalColorMap() {
	if (!_originalColorToItemMap.empty() ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	_originalColorToItemMap.resize(kScenePaletteMapPageSize);
	memcpy(_originalColorToItemMap.data(), _paletteMaskOriginal.data() + kSceneColorToItemMap,
		_originalColorToItemMap.size());
}

void Scene6050::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_originalColorToItemMap.size() < kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_originalColorToItemMap[i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene6050::rebuildMuseumWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 2)
			_walkablePaletteMask[i] = 0;
	}
}

} // End of namespace Hollywood
