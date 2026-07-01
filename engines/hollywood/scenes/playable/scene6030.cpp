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

const char *const kScene6030ArchiveName = "RESOURCE.F03";
const char *const kScene6030MusicArchiveName = "RESOURCE.M06";
const char *const kScene6030SoundArchiveName = "RESOURCE.S06";
const uint kScene6030InitialRequiredChunkCount = 12;
const uint kScene6030ArenaFirstChunk = 5;
const uint kScene6030ArenaLastChunk = 11;
const uint kScene6030StageIndex = 603;
const uint16 kScene6030FirstState = 0x178e;
const uint16 kScene6030LastState = 0x1797;
const uint16 kScene6020ReturnFromScene6030State = 0x1785;
const uint16 kScene6030ViewportXOffset = 0x0064;
const uint kScene6030ActorBankTableEntry = 0x0000;
const uint kScene6030ActorPaletteTableEntry = 0x00cc;
const uint kScene6030Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6030FastFrameMillis = 50;
const uint32 kScene6030HannoverFrameMillis = 75;
const uint kScene6030LargeLayerDescriptorCount = 0x1b;
const uint kScene6030TaffyEntranceDescriptorCount = 0x0e;
const uint kScene6030SmallLayerDescriptorCount = 3;
const byte kScene6030TaffyEntranceChunkIndex = 7;
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

const byte kScene6030ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

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

const byte kScene6030TaffyEntranceFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

static PlayableSceneConfig scene6030Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene6030ArchiveName;
	config.initialRequiredChunkCount = kScene6030InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene6030ArenaFirstChunk;
	config.arenaLastChunk = kScene6030ArenaLastChunk;
	config.stageIndex = kScene6030StageIndex;
	config.debugName = "Scene 6030";
	config.viewportXOffset = kScene6030ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 6;
	config.actorBankTableEntry = kScene6030ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene6030ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene6030Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene6030SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene6030ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene6030ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene6030MusicArchiveName;
	config.soundBank0ArchiveName = kScene6030SoundArchiveName;
	config.mainFlowFirstState = kScene6030FirstState;
	config.mainFlowLastState = kScene6030LastState;
	return config;
}

Scene6030::Scene6030(HollywoodEngine *vm) :
		PlayableScene(vm, scene6030Config(), "scene6030", 0x1e5, 0x186, 5, 0xfd, 0xfb),
		_largeBackgroundChannel(),
		_taffyEntranceChannel(),
		_largeBackgroundLayer(),
		_smallForegroundLayer(),
		_taffyEntranceLayer(),
		_hannoverManualSequenceActive(false),
		_hannoverLayerSuppressed(false),
		_taffyEntranceSequenceActive(false) {
	_largeBackgroundLayer.configure(6, kScene6030LargeLayerDescriptorCount,
		kScene6030LargeLayerFrameMap, ARRAYSIZE(kScene6030LargeLayerFrameMap));
	_smallForegroundLayer.configure(8, kScene6030SmallLayerDescriptorCount,
		kScene6030SmallLayerFrameMap, ARRAYSIZE(kScene6030SmallLayerFrameMap));
	_taffyEntranceLayer.configure(kScene6030TaffyEntranceChunkIndex, kScene6030TaffyEntranceDescriptorCount,
		kScene6030TaffyEntranceFrameMap, ARRAYSIZE(kScene6030TaffyEntranceFrameMap));
}

bool Scene6030::hasCustomPreviewState() const {
	return true;
}

void Scene6030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	_activeActorWorldX = 0x1e5;
	_activeActorWorldY = 0x186;
	_activeActorFacing = 5;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene6030::hasCustomComposite() const {
	return true;
}

void Scene6030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (!_hannoverLayerSuppressed)
		drawResourceSpriteLayer(_largeBackgroundLayer);
	drawResourceSpriteLayer(_taffyEntranceLayer);
	drawResourceSpriteLayer(_smallForegroundLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldX, activeWorldY);
}

bool Scene6030::hasCustomEntrySequence() const {
	return true;
}

void Scene6030::runCustomEntrySequence() {
	_soundBank0.playSample(4, 100);
	runEntryPath(0x384, 0x1de, 5, 0x1e5, 0x186);
	runEntryConversation();
}

bool Scene6030::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene6030::advanceCustomGameplayLoop(uint32 delta) {
	if (_taffyEntranceSequenceActive)
		advanceTaffyEntranceLayer(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else if (!_hannoverManualSequenceActive && !_taffyEntranceSequenceActive)
		advanceHannoverLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene6030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301:
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302:
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303:
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304:
		beginSecondarySpeechLine(4, 0);
		return true;
	case 305: // Original patch-backed raw callback label.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306:
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307:
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Original raw callback label near the scene exit path.
		returnToScene6020();
		return true;
	case 309:
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310:
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311:
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312:
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313:
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314:
		beginSecondarySpeechLine(13, 0);
		return true;
	case 315:
		beginSecondarySpeechLine(14, 0);
		return true;
	case 316:
		beginSecondarySpeechLine(14, 0);
		return true;
	default:
		return false;
	}
}

bool Scene6030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

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

void Scene6030::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_largeBackgroundLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene6030::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
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
	_largeBackgroundChannel.reset(0, kScene6030HannoverFrameMillis);
	_taffyEntranceChannel.reset(0, kScene6030FastFrameMillis);
	_largeBackgroundLayer.reset(0);
	_smallForegroundLayer.reset(0);
	_taffyEntranceLayer.reset(0);
	_largeBackgroundLayer.visible = true;
	_smallForegroundLayer.visible = true;
	_taffyEntranceLayer.visible = false;
	_hannoverManualSequenceActive = false;
	_hannoverLayerSuppressed = _vm->gameState().scene6030CoffeeState >= 2;
	_taffyEntranceSequenceActive = false;
}

void Scene6030::advanceHannoverLayer(uint32 delta) {
	const uint frameCount = _largeBackgroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_largeBackgroundLayer.frameIndex == 0) {
			if (_random.getRandomNumber(14) == 0)
				_largeBackgroundLayer.setFrame(4);
		} else {
			_largeBackgroundLayer.setFrame(0);
		}
	}
}

void Scene6030::advanceTaffyEntranceLayer(uint32 delta) {
	if (!_taffyEntranceLayer.visible)
		return;

	const uint frameCount = _taffyEntranceChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_taffyEntranceLayer.frameIndex < 0x0d) {
			_taffyEntranceLayer.setFrame(_taffyEntranceLayer.frameIndex + 1);
		} else {
			_taffyEntranceSequenceActive = false;
			break;
		}
	}
}

void Scene6030::drawForegroundBlocks(int activeWorldX, int activeWorldY) {
	if (activeWorldY < 0x184 && activeWorldX > 0x200 && _sceneChunkTable.isValidChunk(11))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _sceneFramebuffer);
}

void Scene6030::runEntryConversation() {
	GameplayState &state = _vm->gameState();
	if (!state.scene6030HannoverInterviewCompleted) {
		beginHannoverSpeechLine(0);
		beginSecondarySpeechLine(kScene6030DialogueStageId, 0);
		beginHannoverSpeechLine(1);
		beginHannoverSpeechLine(3);
		runHannoverFrameRangeSequence(false);
		beginHannoverSpeechLine(5, 2);
		runHannoverFrameRangeSequence(true);
		beginHannoverSpeechLine(6);
		runHannoverDeskWalkSequence();
		beginHannoverSpeechLine(8);
		runHannoverDialogueMenu();
		state.scene6030HannoverInterviewCompleted = true;
		returnToScene6020();
		return;
	}

	beginHannoverSpeechLine(2);
	beginHannoverSpeechLine(state.scene6030CoffeeState < 2 ? 3 : 4);
	runHannoverFrameRangeSequence(false);
	beginHannoverSpeechLine(5, 2);
	runHannoverFrameRangeSequence(true);
	beginHannoverSpeechLine(7);
	runHannoverDeskWalkSequence();
	beginHannoverSpeechLine(8);
	if (state.scene6030CoffeeState == 0) {
		runHannoverDialogueMenu();
		returnToScene6020();
		return;
	}

	runTaffyEnteringAnimation();
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

void Scene6030::runHannoverFrameRange(byte firstFrame, byte lastFrame, byte finalFrame, uint32 frameMillis) {
	const bool previousManualSequence = _hannoverManualSequenceActive;
	_hannoverManualSequenceActive = true;
	_largeBackgroundLayer.setFrame(firstFrame);
	while (_largeBackgroundLayer.frameIndex < lastFrame &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(frameMillis))
			break;
		_largeBackgroundLayer.setFrame(_largeBackgroundLayer.frameIndex + 1);
	}
	if (!Engine::shouldQuit())
		_largeBackgroundLayer.setFrame(finalFrame);
	_hannoverManualSequenceActive = previousManualSequence;
}

void Scene6030::runHannoverFrameRangeSequence(bool alternatePose) {
	runHannoverFrameRange(alternatePose ? 0x10 : 5, alternatePose ? 0x12 : 0x0a,
		alternatePose ? 0 : 0x0b, kScene6030HannoverFrameMillis);
}

void Scene6030::runHannoverDeskWalkSequence() {
	walkActiveActorTo(0x25b, 0x17d, 4, 0, false);
	walkActiveActorTo(0x1e5, 0x186, 5, 0, false);
}

void Scene6030::runTaffyEnteringAnimation() {
	runHannoverFrameRange(0x2d, 0x3e, 0x3e, kScene6030FastFrameMillis);

	_hannoverLayerSuppressed = true;
	_taffyEntranceLayer.visible = true;
	_taffyEntranceLayer.reset(0);
	_taffyEntranceChannel.reset(0, kScene6030FastFrameMillis);
	_taffyEntranceSequenceActive = true;

	const uint32 speechDuration = beginStaticHannoverSpeechLine(9, 500, 100);
	uint32 elapsed = 0;
	bool departureVoiceVolumeAdjusted = false;
	while (((_taffyEntranceLayer.frameIndex < 0x0d) || _speech.isPlaying() || elapsed < speechDuration) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (waitSceneMillis(10))
			break;
		if (!departureVoiceVolumeAdjusted && _taffyEntranceLayer.frameIndex == 0x0d) {
			_speech.setVolume(25);
			departureVoiceVolumeAdjusted = true;
		}
		elapsed += 10;
	}

	_taffyEntranceSequenceActive = false;
	_taffyEntranceLayer.visible = false;
	_speech.stop();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
	_hannoverLayerSuppressed = true;
}

void Scene6030::returnToScene6020() {
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene6020ReturnFromScene6030State;
}

} // End of namespace Hollywood
