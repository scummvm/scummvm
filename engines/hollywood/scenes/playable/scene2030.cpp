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

#include "hollywood/scenes/playable/scene2030.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene2030ArchiveName = "RESOURCE.B03";
const char *const kScene2030MusicArchiveName = "RESOURCE.M02";
const char *const kScene2030SoundArchiveName = "RESOURCE.S02";
const uint kScene2030InitialRequiredChunkCount = 11;
const uint kScene2030ArenaFirstChunk = 5;
const uint kScene2030ArenaLastChunk = 10;
const uint kScene2030StageIndex = 203;
const uint16 kScene2030FirstState = 0x07ee;
const uint16 kScene2030LastState = 0x07f7;
const uint16 kScene2030PyramidExitState = 0x07da;
const uint16 kScene2030SphinxExitState = 0x07f8;
const uint16 kScene2030EntryFromPyramidState = 0x07ef;
const uint16 kScene2030EntryFromSphinxState = 0x07f0;
const uint16 kScene2030ViewportXOffset = 0x0068;
const uint kScene2030ActorBankTableEntry = 0x0000;
const uint kScene2030ActorPaletteTableEntry = 0x00cc;
const uint kScene2030Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2030LeftMerchantFrameMillis = 300;
const uint32 kScene2030RightMerchantFrameMillis = 125;
const uint32 kScene2030MerchantCalloutCheckMillis = 75;
const uint32 kScene2030TransitionFrameMillis = 60;
const uint kScene2030LeftMerchantDescriptorCount = 0x1c;
const uint kScene2030RightMerchantDescriptorCount = 0x1e;
const uint kScene2030TransitionDescriptorCount = 0x8c;
const byte kScene2030TransitionFinalFrame = 0x8b;
const byte kScene2030PrimarySpeechTextColor = 0xfb;

enum Scene2030MerchantState {
	kScene2030MerchantIdle = 0,
	kScene2030MerchantCalloutOpen = 1,
	kScene2030MerchantCalloutClose = 2,
	kScene2030MerchantTalkOpen = 3,
	kScene2030MerchantTalkHold = 4,
	kScene2030MerchantTalkClose = 5,
	kScene2030MerchantCalloutReady = 6,
	kScene2030MerchantCalloutSpeaking = 7
};

enum Scene2030MerchantSide {
	kScene2030MerchantRight = 0,
	kScene2030MerchantLeft = 1,
	kScene2030MerchantNoCalloutSide = 2
};

enum Scene2030MerchantSpeechGroup {
	kScene2030RightMerchantIdleSpeech = 0,
	kScene2030LeftMerchantIdleSpeech = 1,
	kScene2030RightMerchantInteractiveSpeech = 2,
	kScene2030LeftMerchantInteractiveSpeech = 3
};

const byte kScene2030ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene2030LeftMerchantFrameMap[] = {
	0, 27, 27, 1, 2, 3, 3, 2, 1, 27, 27, 8,
	9, 10, 11, 11, 10, 9, 8, 27, 3, 4, 5, 6,
	7, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
	20, 19, 18, 22, 23, 24, 25, 26, 11, 0, 0, 0
};

const byte kScene2030RightMerchantFrameMap[] = {
	0, 29, 29, 1, 2, 3, 3, 2, 1, 29, 29, 8,
	9, 10, 11, 12, 13, 13, 12, 11, 10, 9, 8, 29,
	3, 4, 5, 6, 7, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 22, 21, 20, 24, 25, 26, 27, 28,
	13, 0, 0, 0, 0, 0, 0, 0
};

static PlayableSceneConfig scene2030Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene2030ArchiveName;
	config.initialRequiredChunkCount = kScene2030InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene2030ArenaFirstChunk;
	config.arenaLastChunk = kScene2030ArenaLastChunk;
	config.stageIndex = kScene2030StageIndex;
	config.debugName = "Scene 2030";
	config.viewportXOffset = kScene2030ViewportXOffset;
	config.viewportMinXOffset = kScene2030ViewportXOffset;
	config.viewportMaxXOffset = kScene2030ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 2;
	config.actorBankTableEntry = kScene2030ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene2030ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene2030Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene2030SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene2030ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene2030ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 6;
	config.musicArchiveName = kScene2030MusicArchiveName;
	config.soundBank0ArchiveName = kScene2030SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene2030FirstState;
	config.mainFlowLastState = kScene2030LastState;
	return config;
}

Scene2030::Scene2030(HollywoodEngine *vm) :
		PlayableScene(vm, scene2030Config(), "scene2030", 0x245, 0x167, 4, 0xfd, 0xfb),
		_leftMerchantChannel(),
		_rightMerchantChannel(),
		_leftMerchantLayer(),
		_rightMerchantLayer(),
		_leftMerchantState(0),
		_rightMerchantState(0),
		_merchantCalloutSide(kScene2030MerchantNoCalloutSide),
		_merchantCalloutSpeechActive(false),
		_merchantCalloutTimerAccumulator(0),
		_merchantCalloutSpeechElapsed(0),
		_merchantCalloutSpeechDuration(0) {
	_leftMerchantLayer.configure(5, kScene2030LeftMerchantDescriptorCount,
		kScene2030LeftMerchantFrameMap, ARRAYSIZE(kScene2030LeftMerchantFrameMap));
	_rightMerchantLayer.configure(6, kScene2030RightMerchantDescriptorCount,
		kScene2030RightMerchantFrameMap, ARRAYSIZE(kScene2030RightMerchantFrameMap));
}

bool Scene2030::hasCustomPreviewState() const {
	return true;
}

void Scene2030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetMerchantLayers();

	switch (_vm->gameState().mainFlowStateId) {
	case kScene2030EntryFromPyramidState:
		_activeActorWorldX = 200;
		_activeActorWorldY = 0x1b3;
		_activeActorFacing = 2;
		break;
	case kScene2030EntryFromSphinxState:
		_activeActorWorldX = 0x226;
		_activeActorWorldY = 0x130;
		_activeActorFacing = 3;
		break;
	default:
		_activeActorWorldX = 0x245;
		_activeActorWorldY = 0x167;
		_activeActorFacing = 4;
		break;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene2030::hasCustomComposite() const {
	return true;
}

void Scene2030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_rightMerchantLayer);
	drawResourceSpriteLayer(_leftMerchantLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
}

bool Scene2030::hasCustomEntrySequence() const {
	return true;
}

void Scene2030::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene2030EntryFromPyramidState:
		runEntryFromPyramid();
		break;
	case kScene2030EntryFromSphinxState:
		runEntryFromSphinx();
		break;
	default:
		runEntryFromChapterStart();
		break;
	}
}

bool Scene2030::prepareCustomGameplayLoop() {
	resetMerchantLayers();
	return true;
}

bool Scene2030::advanceCustomGameplayLoop(uint32 delta) {
	advanceMerchantLayers(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	advanceMerchantCalloutSpeech(delta);
	updateRandomMerchantCallouts(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2030::dispatchCustomSceneAction(uint16 handlerId) {
	stopMerchantCalloutSpeech();

	switch (handlerId) {
	case 301: // Ir a esfinge (go to sphinx): transition toward scene 2040.
		runSphinxExitTransition();
		return true;
	case 302: // Mirar esfinge (look at sphinx).
		beginSecondarySpeechLine(0, 0);
		return true;
	case 303: // Ir a piramide (go to pyramid): transition toward scene 2010.
		_vm->gameState().mainFlowStateId = kScene2030PyramidExitState;
		return true;
	case 304: // Hablar con mercader (talk to merchant): shop dialogue tree, partially restored.
		runMerchantShopPlaceholder();
		return true;
	case 305: // Mirar mercader (look at merchant): Suki's nose comparison.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 306: // Hablar con mercader (talk to merchant): fixed right-stall dialogue.
		runRightMerchantTalkSequence();
		return true;
	case 307: // Mirar mercader (look at merchant): Amy's belly comparison.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 308: // Mirar puesto (look at stall): cheap trinkets.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 309: // Mirar puesto (look at stall): merchant callout row.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 310: // Mirar puesto (look at stall): owner not visible.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 311: // Dar/usar item con mercader (give/use item with merchant): trade rejected.
		beginSecondarySpeechLine(9, 0);
		openMerchantForInteraction(true);
		runMerchantPrimarySpeechLine(9, (byte)(_random.getRandomNumber(1) + 1), true);
		closeMerchantAfterInteraction(true);
		return true;
	case 312: // Comprar articulo del mercader (buy merchant item 0x2b), not fully implemented.
		beginSecondarySpeechLine(9, 7);
		openMerchantForInteraction(true);
		runMerchantPrimarySpeechLine(9, 4, true);
		closeMerchantAfterInteraction(true);
		return true;
	case 313: // Comprar articulo del mercader (buy merchant item 0x2a), not fully implemented.
		beginSecondarySpeechLine(9, 7);
		openMerchantForInteraction(true);
		runMerchantPrimarySpeechLine(9, 3, true);
		closeMerchantAfterInteraction(true);
		return true;
	case 314: // Ir a avioneta (go to airplane): open travel destination selector.
		_vm->gameState().requestTravelScreenSelection(2);
		return true;
	default:
		return false;
	}
}

byte Scene2030::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	switch (animationGroup) {
	case kScene2030RightMerchantIdleSpeech:
		return 0x18;
	case kScene2030LeftMerchantIdleSpeech:
		return 0x14;
	case kScene2030RightMerchantInteractiveSpeech:
		return 0x1d;
	case kScene2030LeftMerchantInteractiveSpeech:
		return 0x19;
	default:
		return 0;
	}
}

void Scene2030::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (merchantSpeechGroupIsRight(animationGroup))
		_rightMerchantLayer.setFrame(frameIndex);
	else if (merchantSpeechGroupIsLeft(animationGroup))
		_leftMerchantLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene2030::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 1, 20, 50, 0x0b, 3, 100, 50);
}

void Scene2030::resetMerchantLayers() {
	_leftMerchantChannel.reset(1, kScene2030LeftMerchantFrameMillis);
	_rightMerchantChannel.reset(1, kScene2030RightMerchantFrameMillis);
	_leftMerchantState = 0;
	_rightMerchantState = 0;
	_merchantCalloutSide = kScene2030MerchantNoCalloutSide;
	_merchantCalloutSpeechActive = false;
	_merchantCalloutTimerAccumulator = 0;
	_merchantCalloutSpeechElapsed = 0;
	_merchantCalloutSpeechDuration = 0;
	_leftMerchantLayer.visible = true;
	_rightMerchantLayer.visible = true;
	_leftMerchantLayer.reset(1);
	_rightMerchantLayer.reset(1);
}

void Scene2030::advanceMerchantLayers(uint32 delta) {
	uint frameCount = _leftMerchantChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		advanceLeftMerchantTick();

	frameCount = _rightMerchantChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		advanceRightMerchantTick();
}

void Scene2030::advanceLeftMerchantTick() {
	if (_primaryDialogueSpeechActive && merchantSpeechGroupIsLeft(_primaryDialogueSpeechGroup))
		return;

	switch (_leftMerchantState) {
	case kScene2030MerchantIdle:
		if (_leftMerchantChannel.frameIndex == 1)
			_leftMerchantChannel.frameIndex = _random.getRandomNumber(14) == 0 ? 0 : 1;
		else
			_leftMerchantChannel.frameIndex = 1;
		break;
	case kScene2030MerchantCalloutOpen:
		if (_leftMerchantChannel.frameIndex < 5)
			++_leftMerchantChannel.frameIndex;
		else
			_leftMerchantState = kScene2030MerchantCalloutReady;
		break;
	case kScene2030MerchantCalloutClose:
		if (_leftMerchantChannel.frameIndex < 8)
			++_leftMerchantChannel.frameIndex;
		else {
			_merchantCalloutSide = kScene2030MerchantNoCalloutSide;
			_leftMerchantState = kScene2030MerchantIdle;
			_leftMerchantChannel.frameIndex = 1;
		}
		break;
	case kScene2030MerchantTalkOpen:
		if (_leftMerchantChannel.frameIndex < 0x0e)
			++_leftMerchantChannel.frameIndex;
		else
			_leftMerchantState = kScene2030MerchantTalkHold;
		break;
	case kScene2030MerchantTalkHold:
		if (_leftMerchantChannel.frameIndex == 0x19)
			_leftMerchantChannel.frameIndex = _random.getRandomNumber(14) == 0 ? 0x1d : 0x19;
		else
			_leftMerchantChannel.frameIndex = 0x19;
		break;
	case kScene2030MerchantTalkClose:
		if (_leftMerchantChannel.frameIndex > 0x12) {
			_leftMerchantState = kScene2030MerchantIdle;
			_leftMerchantChannel.frameIndex = 1;
		} else {
			++_leftMerchantChannel.frameIndex;
		}
		break;
	case kScene2030MerchantCalloutReady:
	case kScene2030MerchantCalloutSpeaking:
		break;
	default:
		_leftMerchantState = kScene2030MerchantIdle;
		_leftMerchantChannel.frameIndex = 1;
		break;
	}

	_leftMerchantLayer.setFrame(_leftMerchantChannel.frameIndex);
}

void Scene2030::advanceRightMerchantTick() {
	if (_primaryDialogueSpeechActive && merchantSpeechGroupIsRight(_primaryDialogueSpeechGroup))
		return;

	switch (_rightMerchantState) {
	case kScene2030MerchantIdle:
		if (_rightMerchantChannel.frameIndex == 1)
			_rightMerchantChannel.frameIndex = _random.getRandomNumber(14) == 0 ? 0 : 1;
		else
			_rightMerchantChannel.frameIndex = 1;
		break;
	case kScene2030MerchantCalloutOpen:
		if (_rightMerchantChannel.frameIndex < 5)
			++_rightMerchantChannel.frameIndex;
		else
			_rightMerchantState = kScene2030MerchantCalloutReady;
		break;
	case kScene2030MerchantCalloutClose:
		if (_rightMerchantChannel.frameIndex < 8)
			++_rightMerchantChannel.frameIndex;
		else {
			_merchantCalloutSide = kScene2030MerchantNoCalloutSide;
			_rightMerchantState = kScene2030MerchantIdle;
			_rightMerchantChannel.frameIndex = 1;
		}
		break;
	case kScene2030MerchantTalkOpen:
		if (_rightMerchantChannel.frameIndex < 0x10)
			++_rightMerchantChannel.frameIndex;
		else
			_rightMerchantState = kScene2030MerchantTalkHold;
		break;
	case kScene2030MerchantTalkHold:
		if (_rightMerchantChannel.frameIndex == 0x1d)
			_rightMerchantChannel.frameIndex = _random.getRandomNumber(14) == 0 ? 0x21 : 0x1d;
		else
			_rightMerchantChannel.frameIndex = 0x1d;
		break;
	case kScene2030MerchantTalkClose:
		if (_rightMerchantChannel.frameIndex > 0x16) {
			_rightMerchantState = kScene2030MerchantIdle;
			_rightMerchantChannel.frameIndex = 1;
		} else {
			++_rightMerchantChannel.frameIndex;
		}
		break;
	case kScene2030MerchantCalloutReady:
	case kScene2030MerchantCalloutSpeaking:
		break;
	default:
		_rightMerchantState = kScene2030MerchantIdle;
		_rightMerchantChannel.frameIndex = 1;
		break;
	}

	_rightMerchantLayer.setFrame(_rightMerchantChannel.frameIndex);
}

void Scene2030::updateRandomMerchantCallouts(uint32 delta) {
	if (_merchantCalloutSpeechActive || _primarySpeechOverlay.visible || _speechOverlay.visible)
		return;

	_merchantCalloutTimerAccumulator += delta;
	while (_merchantCalloutTimerAccumulator >= kScene2030MerchantCalloutCheckMillis) {
		_merchantCalloutTimerAccumulator -= kScene2030MerchantCalloutCheckMillis;

		if (_merchantCalloutSide == kScene2030MerchantRight) {
			if (_rightMerchantState == kScene2030MerchantCalloutReady &&
					!startMerchantCalloutSpeech(true)) {
				_rightMerchantState = kScene2030MerchantCalloutClose;
				_rightMerchantChannel.frameIndex = 6;
				_rightMerchantChannel.resetTimer();
				_rightMerchantLayer.setFrame(_rightMerchantChannel.frameIndex);
			}
			continue;
		}

		if (_merchantCalloutSide == kScene2030MerchantLeft) {
			if (_leftMerchantState == kScene2030MerchantCalloutReady &&
					!startMerchantCalloutSpeech(false)) {
				_leftMerchantState = kScene2030MerchantCalloutClose;
				_leftMerchantChannel.frameIndex = 6;
				_leftMerchantChannel.resetTimer();
				_leftMerchantLayer.setFrame(_leftMerchantChannel.frameIndex);
			}
			continue;
		}

		if (_actorPathPlaybackActive || _random.getRandomNumber(4) != 0)
			continue;

		if (_random.getRandomNumber(1) == 0) {
			_merchantCalloutSide = kScene2030MerchantRight;
			_rightMerchantState = kScene2030MerchantCalloutOpen;
			_rightMerchantChannel.frameIndex = 2;
			_rightMerchantChannel.resetTimer();
			_rightMerchantLayer.setFrame(_rightMerchantChannel.frameIndex);
		} else {
			_merchantCalloutSide = kScene2030MerchantLeft;
			_leftMerchantState = kScene2030MerchantCalloutOpen;
			_leftMerchantChannel.frameIndex = 2;
			_leftMerchantChannel.resetTimer();
			_leftMerchantLayer.setFrame(_leftMerchantChannel.frameIndex);
		}
	}
}

bool Scene2030::startMerchantCalloutSpeech(bool rightMerchant) {
	const uint16 rowIndex = rightMerchant ? 7 : 6;
	const byte frameIndex = rightMerchant ? (byte)_random.getRandomNumber(6) :
		(byte)_random.getRandomNumber(8);
	const uint16 centerX = rightMerchant ? 0x18c : 0x11b;
	const uint16 topY = rightMerchant ? 0x94 : 0x7b;
	const byte red = rightMerchant ? 0x36 : 3;
	const byte green = rightMerchant ? 0x12 : 0x3f;
	const byte blue = rightMerchant ? 0x3f : 0x20;
	const byte animationGroup = rightMerchant ? kScene2030RightMerchantIdleSpeech :
		kScene2030LeftMerchantIdleSpeech;

	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return false;
	(void)continuationCount;

	const Common::String text = getResource003LargeTextRecord(textRecordId);
	if (text.empty())
		return false;

	setPaletteEntry6Bit(kScene2030PrimarySpeechTextColor, red, green, blue);
	_primarySpeechOverlay.visible = true;
	_primarySpeechOverlay.colorIndex = kScene2030PrimarySpeechTextColor;
	wrapActorSpeechText(text, centerX, _primarySpeechOverlay.lines);
	calculateSpeechOverlayBounds(_primarySpeechOverlay, centerX, topY, true, _activeActorWorldY);

	const bool started = voiceSampleId != 0 && _speech.playSample(voiceSampleId, 100);
	_merchantCalloutSpeechDuration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(1200, _primarySpeechOverlay.lines.size() * 1100);
	_merchantCalloutSpeechElapsed = 0;
	_merchantCalloutSpeechActive = true;

	if (rightMerchant)
		_rightMerchantState = kScene2030MerchantCalloutSpeaking;
	else
		_leftMerchantState = kScene2030MerchantCalloutSpeaking;

	const byte baseFrame = primarySpeechAnimationBaseFrame(animationGroup);
	_speechController.startPrimaryDialogueSpeech(animationGroup, baseFrame);
	setPrimarySpeechAnimationFrame(animationGroup, baseFrame);
	return true;
}

void Scene2030::advanceMerchantCalloutSpeech(uint32 delta) {
	if (!_merchantCalloutSpeechActive)
		return;

	_merchantCalloutSpeechElapsed += delta;
	if (_speech.isPlaying() || _merchantCalloutSpeechElapsed < _merchantCalloutSpeechDuration)
		return;

	const bool rightMerchant = _merchantCalloutSide == kScene2030MerchantRight;
	_speech.stop();
	_primarySpeechOverlay.visible = false;
	_primarySpeechOverlay.lines.clear();
	_speechController.stopPrimaryDialogueSpeech(0xff, 7);
	_merchantCalloutSpeechActive = false;
	_merchantCalloutSpeechElapsed = 0;
	_merchantCalloutSpeechDuration = 0;

	if (rightMerchant) {
		_rightMerchantState = kScene2030MerchantCalloutClose;
		_rightMerchantChannel.frameIndex = 6;
		_rightMerchantChannel.resetTimer();
		_rightMerchantLayer.setFrame(_rightMerchantChannel.frameIndex);
	} else {
		_leftMerchantState = kScene2030MerchantCalloutClose;
		_leftMerchantChannel.frameIndex = 6;
		_leftMerchantChannel.resetTimer();
		_leftMerchantLayer.setFrame(_leftMerchantChannel.frameIndex);
	}
}

void Scene2030::stopMerchantCalloutSpeech() {
	if (_merchantCalloutSpeechActive) {
		_speech.stop();
		_primarySpeechOverlay.visible = false;
		_primarySpeechOverlay.lines.clear();
		_speechController.stopPrimaryDialogueSpeech(0xff, 7);
	}

	_merchantCalloutSpeechActive = false;
	_merchantCalloutSpeechElapsed = 0;
	_merchantCalloutSpeechDuration = 0;
	_merchantCalloutTimerAccumulator = 0;
	_merchantCalloutSide = kScene2030MerchantNoCalloutSide;

	if (_rightMerchantState == kScene2030MerchantCalloutOpen ||
			_rightMerchantState == kScene2030MerchantCalloutReady ||
			_rightMerchantState == kScene2030MerchantCalloutSpeaking ||
			_rightMerchantState == kScene2030MerchantCalloutClose) {
		_rightMerchantState = kScene2030MerchantIdle;
		_rightMerchantChannel.frameIndex = 1;
		_rightMerchantLayer.setFrame(1);
	}

	if (_leftMerchantState == kScene2030MerchantCalloutOpen ||
			_leftMerchantState == kScene2030MerchantCalloutReady ||
			_leftMerchantState == kScene2030MerchantCalloutSpeaking ||
			_leftMerchantState == kScene2030MerchantCalloutClose) {
		_leftMerchantState = kScene2030MerchantIdle;
		_leftMerchantChannel.frameIndex = 1;
		_leftMerchantLayer.setFrame(1);
	}
}

void Scene2030::openMerchantForInteraction(bool rightMerchant) {
	stopMerchantCalloutSpeech();

	if (rightMerchant) {
		_rightMerchantState = kScene2030MerchantTalkOpen;
		_rightMerchantChannel.frameIndex = 10;
		_rightMerchantChannel.resetTimer();
		_rightMerchantLayer.setFrame(_rightMerchantChannel.frameIndex);
	} else {
		_leftMerchantState = kScene2030MerchantTalkOpen;
		_leftMerchantChannel.frameIndex = 10;
		_leftMerchantChannel.resetTimer();
		_leftMerchantLayer.setFrame(_leftMerchantChannel.frameIndex);
	}

	waitForMerchantState(rightMerchant, kScene2030MerchantTalkHold);
}

void Scene2030::closeMerchantAfterInteraction(bool rightMerchant) {
	if (rightMerchant) {
		_rightMerchantState = kScene2030MerchantTalkClose;
		_rightMerchantChannel.frameIndex = 0x11;
		_rightMerchantChannel.resetTimer();
		_rightMerchantLayer.setFrame(_rightMerchantChannel.frameIndex);
	} else {
		_leftMerchantState = kScene2030MerchantTalkClose;
		_leftMerchantChannel.frameIndex = 0x0f;
		_leftMerchantChannel.resetTimer();
		_leftMerchantLayer.setFrame(_leftMerchantChannel.frameIndex);
	}

	waitForMerchantState(rightMerchant, kScene2030MerchantIdle);
}

void Scene2030::waitForMerchantState(bool rightMerchant, byte targetState) {
	uint32 lastMillis = g_system->getMillis();
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			break;

		const byte state = rightMerchant ? _rightMerchantState : _leftMerchantState;
		if (state == targetState)
			break;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		advanceMerchantLayers(delta);
		updateAmbientAudioAndMusicCues(delta);
		drawPlayableComposite();
		presentFrame();
		g_system->delayMillis(10);
	}
}

void Scene2030::runMerchantPrimarySpeechLine(uint16 rowIndex, byte frameIndex, bool rightMerchant) {
	if (rightMerchant) {
		beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, 0x18c, 0x94,
			0x36, 0x12, 0x3f, kScene2030RightMerchantInteractiveSpeech);
	} else {
		beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, 0x11b, 0x7b,
			3, 0x3f, 0x20, kScene2030LeftMerchantInteractiveSpeech);
	}
}

bool Scene2030::merchantSpeechGroupIsRight(byte animationGroup) const {
	return animationGroup == kScene2030RightMerchantIdleSpeech ||
		animationGroup == kScene2030RightMerchantInteractiveSpeech;
}

bool Scene2030::merchantSpeechGroupIsLeft(byte animationGroup) const {
	return animationGroup == kScene2030LeftMerchantIdleSpeech ||
		animationGroup == kScene2030LeftMerchantInteractiveSpeech;
}

void Scene2030::runEntryFromChapterStart() {
	runEntryPath(0x379, 0x155, 4, 0x245, 0x167);
}

void Scene2030::runEntryFromPyramid() {
	runEntryPath(100, 0x1b3, 2, 200, 0x1b3);
}

void Scene2030::runEntryFromSphinx() {
	_activeActorWorldX = 0x226;
	_activeActorWorldY = 0x130;
	_activeActorFacing = 3;
	_activeActorCel = 0;
	runTransitionClip(10);
	runEntryPath(0x226, 0x130, 3, 0x15e, 400);
}

void Scene2030::runSphinxExitTransition() {
	runTransitionClip(9);
	_vm->gameState().mainFlowStateId = kScene2030SphinxExitState;
}

void Scene2030::runTransitionClip(uint chunkIndex) {
	// RESOURCE.B03 transition frames are cumulative deltas; resetting to the
	// base room background per frame drops unchanged actor pixels.
	Graphics::ManagedSurface transitionBackground;
	transitionBackground.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	transitionBackground.copyRectToSurface(_baseFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));

	uint32 frameAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	byte frameIndex = 0;

	drawTransitionClipFrame(chunkIndex, frameIndex, *transitionBackground.surfacePtr());
	presentFrame();

	while (frameIndex < kScene2030TransitionFinalFrame && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			break;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;
		advanceMerchantLayers(delta);
		updateAmbientAudioAndMusicCues(delta);

		bool frameDirty = false;
		while (frameAccumulator >= kScene2030TransitionFrameMillis && frameIndex < kScene2030TransitionFinalFrame) {
			frameAccumulator -= kScene2030TransitionFrameMillis;
			++frameIndex;
			drawTransitionClipFrame(chunkIndex, frameIndex, *transitionBackground.surfacePtr());
			frameDirty = true;
		}

		if (frameDirty)
			presentFrame();

		g_system->delayMillis(10);
	}
}

void Scene2030::drawTransitionClipFrame(uint chunkIndex, byte frameIndex, Graphics::Surface &transitionBackground) {
	_sceneFramebuffer.copyRectToSurface(transitionBackground, 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	drawResourceSpriteLayer(_rightMerchantLayer);
	drawResourceSpriteLayer(_leftMerchantLayer);
	drawClipFrameDeltaToSurface(chunkIndex, kScene2030TransitionDescriptorCount, frameIndex, *_sceneFramebuffer.surfacePtr());
	drawClipFrameDeltaToSurface(chunkIndex, kScene2030TransitionDescriptorCount, frameIndex, transitionBackground);
}

void Scene2030::drawClipFrameDeltaToSurface(uint chunkIndex, uint tableEntryCount, byte frameIndex, Graphics::Surface &destination) {
	const uint32 frameTableOffset = _resourceChunkOffsets[chunkIndex];
	const uint32 tableEntryOffset = frameTableOffset + ((uint32)frameIndex * 4);
	if (tableEntryOffset + 4 > _resourceArena.size())
		return;

	const uint32 frameOffset = frameTableOffset + ((uint32)tableEntryCount * 4) +
		readUint32LE(_resourceArena, tableEntryOffset);
	if (frameOffset + 4 > _resourceArena.size())
		return;

	const uint16 firstRow = readUint16LE(_resourceArena, frameOffset);
	const uint16 lastRow = readUint16LE(_resourceArena, frameOffset + 2);
	uint cursor = frameOffset + 4;
	byte *pixels = (byte *)destination.getPixels();
	const uint size = destination.pitch * destination.h;

	for (uint row = firstRow; row <= lastRow && row < (uint)destination.h; ++row) {
		if (cursor >= _resourceArena.size())
			return;

		byte runCount = _resourceArena[cursor++];
		for (; runCount != 0; --runCount) {
			if (cursor + 3 > _resourceArena.size())
				return;

			const uint x = readUint16LE(_resourceArena, cursor);
			const byte literalLength = _resourceArena[cursor + 2];
			const uint destinationOffset = row * destination.pitch + x;
			if (destinationOffset >= size)
				return;

			if (literalLength == 0) {
				if (cursor + 5 > _resourceArena.size())
					return;

				const byte fillValue = _resourceArena[cursor + 3];
				const uint fillLength = _resourceArena[cursor + 4];
				cursor += 5;
				if (destinationOffset + fillLength <= size)
					memset(pixels + destinationOffset, fillValue, fillLength);
			} else {
				const uint literalOffset = cursor + 3;
				if (literalOffset + literalLength > _resourceArena.size())
					return;

				if (destinationOffset + literalLength <= size)
					memcpy(pixels + destinationOffset, _resourceArena.data() + literalOffset, literalLength);
				cursor = literalOffset + literalLength;
			}
		}
	}
}

void Scene2030::runMerchantShopPlaceholder() {
	beginSecondarySpeechLine(0x62, 0);
	openMerchantForInteraction(false);
	runMerchantPrimarySpeechLine(99, 0, false);
	closeMerchantAfterInteraction(false);
}

void Scene2030::runRightMerchantTalkSequence() {
	beginSecondarySpeechLine(8, 0);
	openMerchantForInteraction(true);
	runMerchantPrimarySpeechLine(8, 1, true);
	beginSecondarySpeechLine(8, 2);
	runMerchantPrimarySpeechLine(8, 3, true);
	beginSecondarySpeechLine(8, 4);
	runMerchantPrimarySpeechLine(8, 5, true);
	beginSecondarySpeechLine(8, 6);
	closeMerchantAfterInteraction(true);
}

} // End of namespace Hollywood
