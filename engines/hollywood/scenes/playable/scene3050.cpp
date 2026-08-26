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

#include "hollywood/scenes/playable/scene3050.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene3050EntryFromScene3010State = 0x0bea;
const uint16 kScene3050EntryFromScene3060State = 0x0beb;
const uint16 kScene3010EntryFromScene3050State = 0x0bc4;
const uint16 kScene3060State = 0x0bf4;
const uint16 kScene3050ViewportXOffset = 0x00a0;
const uint16 kScene3050ViewportMinXOffset = 0x0068;
const uint16 kScene3050ViewportMaxXOffset = 0x00a0;
const uint kScene3050ActorBankTableEntry = 0x0000;
const uint kScene3050ActorPaletteTableEntry = 0x00cc;
const uint kScene3050Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3050SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3050BackgroundFrameMillis = 25;
const uint32 kScene3050ForegroundFrameMillis = 100;
const uint32 kScene3050DialogueFrameMillis = 125;
const uint kScene3050BackgroundDescriptorCount = 0x1e;
const uint kScene3050ForegroundActorDescriptorCount = 0x17;
const byte kScene3050DialogueStageId = 0x62;
const byte kScene3050DialoguePrimaryRow = 99;
const uint kScene3050DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene3050CaptionSourceRow = 10;
const byte kScene3050CaptionDestinationRow = 3;
const byte kScene3050IgorIdleSpeechRow = 0x0b;
const byte kScene3050IgorIdleSpeechFrameCount = 5;
const byte kScene3050IgorInitialIdleCounter = 7;
const byte kScene3050DialogueBaseFrame = 0x18;
const byte kScene3050DialogueBlinkFrame = 0x1c;
const byte kScene3050PrimarySpeechTextColor = 0xfb;
const byte kScene3050InvalidPrimarySpeechGroup = 0xff;
const byte kScene3050DefaultPrimarySpeechFrame = 7;
const byte kScene3050DialogueTransitionEnd = 0;
const byte kScene3050DialogueTransitionStay = 3;
const uint32 kScene3050MinimumSpeechMillis = 750;
const uint32 kScene3050FallbackSpeechMillis = 1200;

const byte kScene3050BackgroundFrameMap[] = {
	0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27, 27, 28, 28, 28, 29, 29, 29, 29,
	29, 29, 29, 29, 28, 28, 28, 27, 27, 26,
	25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6,
	5, 4, 3, 2, 2, 1, 1, 1, 0, 0,
	0, 0
};

const byte kScene3050ForegroundActorFrameMap[] = {
	11, 11, 11, 12, 13, 14, 15, 0, 1, 2,
	3, 3, 3, 4, 5, 6, 7, 8, 9, 10,
	10, 16, 17, 18, 18, 19, 20, 21, 22, 18,
	17, 16, 10, 11, 3
};

static PlayableSceneConfig scene3050Config() {
	PlayableSceneConfig config(3050,
		SceneResourceLayout(8, 5, 7),
		SceneViewport(kScene3050ViewportXOffset, kScene3050ViewportMinXOffset, kScene3050ViewportMaxXOffset),
		SceneActorPose(0x2a4, 0x19e, 5));
	config.setActorResources(kScene3050ActorBankTableEntry, kScene3050ActorPaletteTableEntry);
	config.setTextResources(kScene3050Resource003RowsOffsetIndex, kScene3050SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet00);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene3050::Scene3050(HollywoodEngine *vm) :
		PlayableScene(vm, scene3050Config()),
		_backgroundChannel(),
		_foregroundActorChannel(),
		_dialogueActorChannel(),
		_backgroundLayer(),
		_foregroundActorLayer(),
		_foregroundActorMode(0),
		_foregroundActorIdleCounter(0),
		_foregroundActorIdleSpeechFrame(0),
		_foregroundActorIdleSpeechActive(false),
		_foregroundActorManualSequenceActive(false),
		_dialogueMenuActive(false),
		_foregroundActorIdleSpeechTimer(0),
		_foregroundActorIdleSpeechDuration(0) {
	_backgroundLayer.configure(7, kScene3050BackgroundDescriptorCount,
		kScene3050BackgroundFrameMap, ARRAYSIZE(kScene3050BackgroundFrameMap));
	_foregroundActorLayer.configure(6, kScene3050ForegroundActorDescriptorCount,
		kScene3050ForegroundActorFrameMap, ARRAYSIZE(kScene3050ForegroundActorFrameMap));
}

void Scene3050::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	if (_vm->gameState().mainFlowStateId == kScene3050EntryFromScene3060State) {
		_activeActorWorldX = 0x102;
		_activeActorWorldY = 0x147;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x2a4;
		_activeActorWorldY = 0x19e;
		_activeActorFacing = 5;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene3050::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_backgroundLayer);
	drawResourceSpriteLayer(_foregroundActorLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks();
}

void Scene3050::runCustomEntrySequence() {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId != kScene3050EntryFromScene3010State && stateId != kScene3050EntryFromScene3060State) {
		PlayableScene::runCustomEntrySequence();
		return;
	}

	if (stateId == kScene3050EntryFromScene3060State)
		runEntryFromScene3060();
	else
		runEntryFromScene3010();
}

bool Scene3050::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	rebuildWalkableMask();
	return true;
}

bool Scene3050::advanceCustomGameplayLoop(uint32 delta) {
	advanceBackgroundLayer(delta);
	updateForegroundActorIdleSpeech(delta);
	if (_dialogueMenuActive)
		advanceDialogueActorLayer(delta);
	else
		advanceForegroundActorLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3050::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Ir a exterior (go outside): return toward scene 3010.
		runExitToScene3010();
		return true;
	case 302: // Ir a escalera / coger paraguas (go to stairs / take umbrella): Igor blocks Ron with dialogue.
		runDialogueAction(false);
		return true;
	case 303: // Mirar escalera (look at stairs): describes the upstairs access.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 304: // Mirar sala (look at room): identifies it as biblioteca (library).
		beginSecondarySpeechLine(2, 0);
		if (!state.scene3050LibraryCaptionRevealed) {
			state.scene3050LibraryCaptionRevealed = true;
			applySceneStateToHotspotsAndPatches(1);
		}
		return true;
	case 305: // Ir a sala/biblioteca (go to room/library): transition toward scene 3060.
		state.mainFlowStateId = kScene3060State;
		return true;
	case 306: // Mirar reloj de pared (look at wall clock).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 307: // Hablar con Igor (talk to Igor): Ron greets Igor, then opens dialogue.
		runDialogueAction(true);
		return true;
	case 308: // Mirar Igor (look at Igor).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 309: // Mirar armarito (look at small cupboard).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 310: // Abrir armarito (open small cupboard).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 311: // Mirar sofa (look at sofa).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 312: // Usar sofa (use sofa).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 313: // Mirar estante (look at shelf).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 314: // Mirar paraguas (look at umbrella).
		beginSecondarySpeechLine(10, 0);
		return true;
	default:
		return false;
	}
}

bool Scene3050::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 1 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		if (_vm->gameState().scene3050LibraryCaptionRevealed)
			copyCaptionRow(kScene3050CaptionSourceRow, kScene3050CaptionDestinationRow);
		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

byte Scene3050::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return kScene3050DialogueBaseFrame;
}

void Scene3050::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_dialogueActorChannel.frameIndex = frameIndex;
	_foregroundActorChannel.frameIndex = frameIndex;
	_foregroundActorLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene3050::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3050::resetAnimationLayers() {
	_backgroundChannel.reset(0, kScene3050BackgroundFrameMillis);
	_foregroundActorChannel.reset(0, kScene3050ForegroundFrameMillis);
	_dialogueActorChannel.reset(kScene3050DialogueBaseFrame, kScene3050DialogueFrameMillis);
	_backgroundLayer.visible = true;
	_foregroundActorLayer.visible = true;
	_backgroundLayer.reset(0);
	_foregroundActorLayer.reset(0);
	_foregroundActorMode = 0;
	_foregroundActorIdleCounter = kScene3050IgorInitialIdleCounter;
	_foregroundActorIdleSpeechFrame = 0;
	_foregroundActorIdleSpeechActive = false;
	_foregroundActorManualSequenceActive = false;
	_dialogueMenuActive = false;
	_foregroundActorIdleSpeechTimer = 0;
	_foregroundActorIdleSpeechDuration = 0;
}

void Scene3050::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 3 || _walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}
}

void Scene3050::copyCaptionRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene3050::advanceBackgroundLayer(uint32 delta) {
	const uint frameCount = _backgroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		_backgroundChannel.frameIndex = _backgroundChannel.frameIndex + 1 < ARRAYSIZE(kScene3050BackgroundFrameMap) ?
			_backgroundChannel.frameIndex + 1 : 0;
		_backgroundLayer.setFrame(_backgroundChannel.frameIndex);
	}
}

void Scene3050::advanceForegroundActorLayer(uint32 delta) {
	if (_foregroundActorManualSequenceActive)
		return;

	const uint frameCount = _foregroundActorChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_foregroundActorMode == 3) {
			if (_foregroundActorChannel.frameIndex < 0x20) {
				++_foregroundActorChannel.frameIndex;
			} else {
				_foregroundActorMode = 1;
				_foregroundActorChannel.frameIndex = 0;
			}
			_foregroundActorLayer.setFrame(_foregroundActorChannel.frameIndex);
			continue;
		}

		if (_primaryDialogueSpeechActive) {
			_foregroundActorMode = 0;
			_foregroundActorChannel.frameIndex = _foregroundActorChannel.frameIndex < 0x13 ?
				_foregroundActorChannel.frameIndex + 1 : 0;
			_foregroundActorLayer.setFrame(_foregroundActorChannel.frameIndex);
			continue;
		}

		if (_foregroundActorMode == 1) {
			if (_foregroundActorChannel.frameIndex == 2) {
				if (_random.getRandomNumber(14) == 0) {
					_foregroundActorChannel.frameIndex = 0x21;
				} else if (_foregroundActorIdleCounter < 10) {
					++_foregroundActorIdleCounter;
				} else {
					_foregroundActorIdleCounter = 0;
					_foregroundActorMode = 0;
					startForegroundActorIdleSpeech(_foregroundActorIdleSpeechFrame);
				}
			} else {
				_foregroundActorChannel.frameIndex = 2;
			}
			_foregroundActorLayer.setFrame(_foregroundActorChannel.frameIndex);
			continue;
		}

		if (_foregroundActorMode == 2) {
			if (_foregroundActorChannel.frameIndex == 0x0c) {
				if (_random.getRandomNumber(14) == 0) {
					_foregroundActorChannel.frameIndex = 0x22;
				} else if (_foregroundActorIdleCounter < 10) {
					++_foregroundActorIdleCounter;
				} else {
					_foregroundActorIdleCounter = 0;
					_foregroundActorMode = 0;
					startForegroundActorIdleSpeech(_foregroundActorIdleSpeechFrame);
				}
			} else {
				_foregroundActorChannel.frameIndex = 0x0c;
			}
			_foregroundActorLayer.setFrame(_foregroundActorChannel.frameIndex);
			continue;
		}

		if (_foregroundActorChannel.frameIndex == 2) {
			_foregroundActorMode = 1;
		} else if (_foregroundActorChannel.frameIndex == 0x0c) {
			_foregroundActorMode = 2;
		} else {
			_foregroundActorChannel.frameIndex = _foregroundActorChannel.frameIndex < 0x13 ?
				_foregroundActorChannel.frameIndex + 1 : 0;
		}
		_foregroundActorLayer.setFrame(_foregroundActorChannel.frameIndex);
	}
}

void Scene3050::advanceDialogueActorLayer(uint32 delta) {
	const uint frameCount = _dialogueActorChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		byte nextFrame = kScene3050DialogueBaseFrame;
		if (_primaryDialogueSpeechActive) {
			nextFrame = _speechController.advancePrimaryDialogueSpeechFrame(
				_random, kScene3050DialogueBaseFrame, primarySpeechAnimationFrameCount(0));
		} else if (_dialogueActorChannel.frameIndex == kScene3050DialogueBaseFrame &&
				_random.getRandomNumber(14) == 0) {
			nextFrame = kScene3050DialogueBlinkFrame;
		}
		setPrimarySpeechAnimationFrame(0, nextFrame);
	}
}

void Scene3050::updateForegroundActorIdleSpeech(uint32 delta) {
	if (!_foregroundActorIdleSpeechActive)
		return;

	if (_speechOverlay.visible || _actionOverlayPlayer.isVisible() || _actorPathPlaybackActive) {
		finishForegroundActorIdleSpeech();
		return;
	}

	_foregroundActorIdleSpeechTimer += delta;
	if (!_speech.isPlaying() && _foregroundActorIdleSpeechTimer >= _foregroundActorIdleSpeechDuration)
		finishForegroundActorIdleSpeech();
}

bool Scene3050::canStartForegroundActorIdleSpeech() const {
	return !_foregroundActorIdleSpeechActive && !_speech.isPlaying() &&
		!_speechOverlay.visible && !_primarySpeechOverlay.visible &&
		!_actionOverlayPlayer.isVisible() && !_actorPathPlaybackActive &&
		!_foregroundActorManualSequenceActive && !_dialogueMenuActive;
}

void Scene3050::startForegroundActorIdleSpeech(byte frameIndex) {
	if (!canStartForegroundActorIdleSpeech())
		return;

	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(kScene3050IgorIdleSpeechRow, frameIndex, textRecordId,
			continuationCount, voiceSampleId))
		return;
	(void)continuationCount;

	const Common::String text = getResource003LargeTextRecord(textRecordId);
	if (text.empty())
		return;

	setPaletteEntry6Bit(kScene3050PrimarySpeechTextColor, 0x3f, 0x26, 0x38);
	_primarySpeechOverlay.visible = true;
	_primarySpeechOverlay.colorIndex = kScene3050PrimarySpeechTextColor;
	wrapActorSpeechText(text, 0x1c0, _primarySpeechOverlay.lines);
	calculateSpeechOverlayBounds(_primarySpeechOverlay, 0x1c0, 0x091, true, _activeActorWorldY);

	const bool started = voiceSampleId != 0 && _speech.playSample(voiceSampleId, 100);
	_foregroundActorIdleSpeechDuration = started ?
		MAX<uint32>(_speech.lastSampleDurationMillis(), kScene3050MinimumSpeechMillis) :
		MAX<uint32>(kScene3050FallbackSpeechMillis, _primarySpeechOverlay.lines.size() * 1100);
	_foregroundActorIdleSpeechTimer = 0;
	_foregroundActorIdleSpeechActive = true;
	_foregroundActorIdleSpeechFrame = (frameIndex + 1) % kScene3050IgorIdleSpeechFrameCount;
	_speechController.startPrimaryDialogueSpeech(0, primarySpeechAnimationBaseFrame(0));
}

void Scene3050::finishForegroundActorIdleSpeech() {
	if (!_foregroundActorIdleSpeechActive)
		return;

	_foregroundActorIdleSpeechActive = false;
	_foregroundActorIdleSpeechTimer = 0;
	_foregroundActorIdleSpeechDuration = 0;
	_speechController.clearPrimaryOverlay();
	if (_primaryDialogueSpeechActive)
		_speechController.stopPrimaryDialogueSpeech(kScene3050InvalidPrimarySpeechGroup,
			kScene3050DefaultPrimarySpeechFrame);
}

void Scene3050::drawForegroundBlocks() {
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene3050::runEntryFromScene3010() {
	_soundBank0.playSample(4, 100);
	runEntryPath(0x388, 0x1c6, 5, 0x2a4, 0x19e);
	_activeActorFacing = 5;
	_activeActorCel = 0;
	if (!_vm->gameState().scene3050EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene3050EntryLineSeen = true;
	}
}

void Scene3050::runEntryFromScene3060() {
	runEntryPath(0x064, 0x11e, 2, 0x102, 0x147);
}

void Scene3050::runExitToScene3010() {
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene3010EntryFromScene3050State;
}

void Scene3050::runDialogueAction(bool playOpeningLine) {
	finishForegroundActorIdleSpeech();
	if (playOpeningLine)
		beginSecondarySpeechLine(kScene3050DialogueStageId, 0);

	runForegroundActorDialoguePose();
	runDialogueMenuRow98();
	_foregroundActorChannel.frameIndex = 0x1d;
	_foregroundActorMode = 3;
	_foregroundActorLayer.setFrame(_foregroundActorChannel.frameIndex);
}

void Scene3050::runForegroundActorDialoguePose() {
	_foregroundActorChannel.frameIndex = _foregroundActorMode == 2 ? 0x0c : 2;
	_foregroundActorMode = 0;
	_foregroundActorLayer.setFrame(_foregroundActorChannel.frameIndex);
	drawPlayableComposite();
	presentFrame();

	_foregroundActorManualSequenceActive = true;
	while (_foregroundActorChannel.frameIndex < 0x17 && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		++_foregroundActorChannel.frameIndex;
		_foregroundActorLayer.setFrame(_foregroundActorChannel.frameIndex);
		if (waitSceneMillis(kScene3050ForegroundFrameMillis))
			break;
	}
	_foregroundActorManualSequenceActive = false;
}

void Scene3050::runDialogueMenuRow98() {
	Common::Array<DialogueChoiceRecord> records;
	initializeDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;

	_dialogueMenuActive = true;
	_dialogueActorChannel.reset(kScene3050DialogueBaseFrame, kScene3050DialogueFrameMillis);
	beginDialogueResponse(0);

	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene3050DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene3050DialogueStageId, 7);
			beginDialogueResponse(7);
			_dialogueMenuActive = false;
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene3050DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff)
			beginDialogueResponse(record.responseFrameIndex);

		if (record.disableAfterUse == 1)
			record.enabled = 0;

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			finished = true;
			break;
		case 3:
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
	_dialogueMenuActive = false;
}

void Scene3050::initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene3050DialogueChoiceRecordCount);

	setDialogueRecord(records, 0, 1, 1, kScene3050DialogueTransitionStay, 1);
	setDialogueRecord(records, 1, 2, 2, kScene3050DialogueTransitionStay, 1);
	setDialogueRecord(records, 2, 3, 3, kScene3050DialogueTransitionStay, 1);
	setDialogueRecord(records, 3, 4, 4, kScene3050DialogueTransitionStay, 1);
	setDialogueRecord(records, 4, 5, 5, kScene3050DialogueTransitionStay, 1);
	setDialogueRecord(records, 5, 6, 6, kScene3050DialogueTransitionStay, 1);
	setDialogueRecord(records, 6, 7, 7, kScene3050DialogueTransitionEnd, 0);
}

void Scene3050::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte playerTextRowId, byte responseFrameIndex, byte transitionMode, byte disableAfterUse) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = 1;
	record.nextNodeIndex = 0;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = 0xff;
}

void Scene3050::beginDialogueResponse(byte frameIndex) {
	finishForegroundActorIdleSpeech();
	beginPrimarySpeechLine(kScene3050DialoguePrimaryRow, frameIndex, 0x1c0, 0x091, 0x3f, 0x26, 0x38);
}

} // End of namespace Hollywood
