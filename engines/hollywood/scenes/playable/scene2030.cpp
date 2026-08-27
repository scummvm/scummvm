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

#include "hollywood/gameplay/dialogue_menu.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const uint16 kScene2030PyramidExitState = 0x07da;
const uint16 kScene2030SphinxExitState = 0x07f8;
const uint16 kScene2030EntryFromPyramidState = 0x07ef;
const uint16 kScene2030EntryFromSphinxState = 0x07f0;
const uint16 kScene2030ViewportXOffset = 0x0068;
const uint kScene2030ActorBankTableEntry = 0x0000;
const uint kScene2030ActorPaletteTableEntry = 0x00cc;
const uint kScene2030Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2030MerchantFrameMillis = 75;
const uint32 kScene2030MerchantSpeechFrameMillis = 125;
const uint32 kScene2030MerchantCalloutCheckMillis = 300;
const uint32 kScene2030OverlayFrameMillis = 75;
const uint32 kScene2030TransitionFrameMillis = 60;
const uint kScene2030LeftMerchantDescriptorCount = 0x1c;
const uint kScene2030RightMerchantDescriptorCount = 0x1e;
const uint kScene2030TransitionDescriptorCount = 0x8c;
const uint kScene2030SphinxExitTransitionChunk = 9;
const uint kScene2030SphinxReturnTransitionChunk = 10;
const uint kScene2030RightMerchantTradeDescriptorCount = 0x0c;
const uint kScene2030LeftMerchantPurchaseDescriptorCount = 0x0c;
const byte kScene2030TransitionFinalFrame = 0x8b;
const byte kScene2030PrimarySpeechTextColor = 0xfb;
const byte kScene2030ShopDialogueStageId = 0x62;
const byte kScene2030ShopPrimaryRow = 99;
const uint kScene2030ShopDialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene2030MoneyItem = 0x29;
const byte kScene2030SeedsItem = 0x2b;
const byte kScene2030MerchantItem2A = 0x2a;
const byte kScene2030RightMerchantSaleItemA = 0x49;
const byte kScene2030RightMerchantSaleItemB = 0x3f;
const uint16 kScene2030SeedsPrice = 0x00fa;
const uint16 kScene2030MerchantItem2APrice = 0x0352;
const uint16 kScene2030RightMerchantSaleValueA = 0x02bc;
const uint16 kScene2030RightMerchantSaleValueB = 0x0190;
const byte kScene2030MerchantCalloutSpeechVolumePercent = 25;

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

const byte kScene2030RightStallTradeFrameMap[] = {
	10, 10, 9, 8, 9, 10, 11
};

const byte kScene2030RightMerchantBuyOverlayFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 4, 4,
	4, 4, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene2030RightMerchantBuyPoseFrameMap[] = {
	34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
	44, 45, 46, 47, 48, 48, 48, 48, 48, 48
};

const byte kScene2030LeftMerchantPurchaseOverlayFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 4, 4,
	4, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene2030LeftMerchantPurchasePoseFrameMap[] = {
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 44, 44, 44, 44
};

static_assert(ARRAYSIZE(kScene2030RightMerchantBuyOverlayFrameMap) ==
	ARRAYSIZE(kScene2030RightMerchantBuyPoseFrameMap),
	"Scene 2030 right-merchant handoff frame maps differ in size");
static_assert(ARRAYSIZE(kScene2030LeftMerchantPurchaseOverlayFrameMap) ==
	ARRAYSIZE(kScene2030LeftMerchantPurchasePoseFrameMap),
	"Scene 2030 left-merchant handoff frame maps differ in size");

struct Scene2030SynchronizedOverlayTarget {
	Scene2030SynchronizedOverlayTarget(ActionOverlayPlayer &newOverlay,
			ResourceSpriteLayer &newMerchantLayer, TimedAnimationChannel &newMerchantChannel,
			const byte *newMerchantFrameMap, uint newFrameCount) :
			overlay(newOverlay),
			merchantLayer(newMerchantLayer),
			merchantChannel(newMerchantChannel),
			merchantFrameMap(newMerchantFrameMap),
			frameCount(newFrameCount) {
	}

	void setFrame(byte frame) {
		overlay.setFrame(frame);
		if (frame >= frameCount)
			return;

		merchantChannel.frameIndex = merchantFrameMap[frame];
		merchantLayer.setFrame(merchantChannel.frameIndex);
	}

	ActionOverlayPlayer &overlay;
	ResourceSpriteLayer &merchantLayer;
	TimedAnimationChannel &merchantChannel;
	const byte *merchantFrameMap;
	uint frameCount;
};

static PlayableSceneConfig scene2030Config() {
	PlayableSceneConfig config(2030,
		SceneResourceLayout(11, 5, 10),
		SceneViewport(kScene2030ViewportXOffset, kScene2030ViewportXOffset, kScene2030ViewportXOffset),
		SceneActorPose(0x245, 0x167, 4));
	config.setActorResources(kScene2030ActorBankTableEntry, kScene2030ActorPaletteTableEntry);
	config.setTextResources(kScene2030Resource003RowsOffsetIndex, kScene2030SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 6;
	config.useActorDepthTest = true;
	return config;
}

Scene2030::Scene2030(HollywoodEngine *vm) :
		PlayableScene(vm, scene2030Config()),
		_leftMerchantChannel(),
		_rightMerchantChannel(),
		_leftMerchantLayer(),
		_rightMerchantLayer(),
		_leftMerchantState(0),
		_rightMerchantState(0),
		_merchantCalloutSide(kScene2030MerchantNoCalloutSide),
		_merchantInteractionActive(false),
		_leftMerchantSequenceLocked(false),
		_rightMerchantSequenceLocked(false),
		_merchantCalloutSpeechActive(false),
		_merchantCalloutTimerAccumulator(0),
		_merchantCalloutSpeechElapsed(0),
		_merchantCalloutSpeechDuration(0) {
	_leftMerchantLayer.configure(5, kScene2030LeftMerchantDescriptorCount,
		kScene2030LeftMerchantFrameMap, ARRAYSIZE(kScene2030LeftMerchantFrameMap));
	_rightMerchantLayer.configure(6, kScene2030RightMerchantDescriptorCount,
		kScene2030RightMerchantFrameMap, ARRAYSIZE(kScene2030RightMerchantFrameMap));
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

bool Scene2030::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

bool Scene2030::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void Scene2030::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
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
	case 304: // Hablar con mercader izquierdo (talk to left merchant): shop dialogue and purchases.
		runMerchantShopDialogue();
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
		runRightMerchantRejectedTradeSequence();
		return true;
	case 312: // Vender calcetín remendado al mercader derecho (sell patched sock to right merchant): grants money.
		runRightMerchantSaleSequence(kScene2030RightMerchantSaleItemA, kScene2030RightMerchantSaleValueA, 4, 5);
		return true;
	case 313: // Vender camisón de Sherilyn al mercader derecho (sell Sherilyn nightgown): grants money.
		runRightMerchantSaleSequence(kScene2030RightMerchantSaleItemB, kScene2030RightMerchantSaleValueB, 3, 6);
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

uint32 Scene2030::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene2030MerchantSpeechFrameMillis;
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
	_leftMerchantChannel.reset(1, kScene2030MerchantFrameMillis);
	_rightMerchantChannel.reset(1, kScene2030MerchantFrameMillis);
	_leftMerchantState = 0;
	_rightMerchantState = 0;
	_merchantCalloutSide = kScene2030MerchantNoCalloutSide;
	_merchantInteractionActive = false;
	_leftMerchantSequenceLocked = false;
	_rightMerchantSequenceLocked = false;
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
	if (!_leftMerchantSequenceLocked) {
		for (uint frame = 0; frame < frameCount; ++frame)
			advanceLeftMerchantTick();
	}

	frameCount = _rightMerchantChannel.consumeFrames(delta);
	if (!_rightMerchantSequenceLocked) {
		for (uint frame = 0; frame < frameCount; ++frame)
			advanceRightMerchantTick();
	}
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
	if (_merchantInteractionActive || _merchantCalloutSpeechActive ||
			_primarySpeechOverlay.visible || _speechOverlay.visible)
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

	const bool started = voiceSampleId != 0 &&
		_speech.playSample(voiceSampleId, kScene2030MerchantCalloutSpeechVolumePercent);
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
	_merchantInteractionActive = true;

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
	_merchantInteractionActive = false;
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

void Scene2030::beginSecondarySpeechLineAndOpenMerchant(uint16 rowIndex, byte frameIndex,
		bool rightMerchant) {
	stopMerchantCalloutSpeech();

	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId) ||
			MAX<byte>(1, continuationCount) != 1) {
		beginSecondarySpeechLine(rowIndex, frameIndex);
		openMerchantForInteraction(rightMerchant);
		return;
	}

	const uint32 startMillis = g_system->getMillis();
	const bool started = startSecondarySpeechLine(rowIndex, frameIndex);
	const bool hasSubtitle = !_speechOverlay.lines.empty();
	const uint32 durationMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(1200, (uint32)_speechOverlay.lines.size() * 1100);

	openMerchantForInteraction(rightMerchant);
	if (started || hasSubtitle)
		waitForStartedSecondarySpeech(startMillis, durationMillis);
}

void Scene2030::waitForStartedSecondarySpeech(uint32 startMillis, uint32 durationMillis) {
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		const bool speechActive = _speech.isPlaying();
		const uint32 elapsed = g_system->getMillis() - startMillis;
		if (!speechActive && elapsed >= durationMillis)
			break;

		const uint32 slice = speechActive ? 50 : MIN<uint32>(50, durationMillis - elapsed);
		if (waitSceneMillis(slice))
			break;
	}

	_speech.stop();
	clearSpeechOverlay();
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
	runEntryPathWithFade(0x379, 0x155, 4, 0x245, 0x167);
}

void Scene2030::runEntryFromPyramid() {
	runEntryPathWithFade(100, 0x1b3, 2, 200, 0x1b3);
}

void Scene2030::runEntryFromSphinx() {
	setActiveActorPose(0x226, 0x130, 3, 0);
	runTransitionClip(kScene2030SphinxReturnTransitionChunk, false, true);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
	runEntryPath(0x226, 0x130, 3, 0x15e, 400);
}

void Scene2030::runEntryPathWithFade(int startX, int startY, byte startFacing,
		int targetX, int targetY) {
	setActiveActorPose(startX, startY, startFacing, 0);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;
	walkActiveActorTo(targetX, targetY, 0xff, 0, false);
}

void Scene2030::runSphinxExitTransition() {
	runTransitionClip(kScene2030SphinxExitTransitionChunk, true, false);
	_vm->gameState().mainFlowStateId = kScene2030SphinxExitState;
}

void Scene2030::runTransitionClip(uint chunkIndex, bool includeActiveActor, bool fadeIn) {
	// The forward clip inherits Ron's pose; the reverse clip supplies every actor pixel.
	Graphics::ManagedSurface transitionBackground;
	transitionBackground.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	copyBaseFramebufferToSceneFramebuffer();
	if (includeActiveActor) {
		drawActiveAndSecondaryActorFrames(!_hideActiveActor, _activeActorFacing, _activeActorCel,
			_activeActorWorldX, _activeActorWorldY, false, 0, 0, 0, 0, -1);
	}
	transitionBackground.copyRectToSurface(_sceneFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));

	uint32 frameAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	byte frameIndex = 0;

	drawTransitionClipFrame(chunkIndex, frameIndex, *transitionBackground.surfacePtr());
	if (fadeIn) {
		if (fadePaletteFromBlack())
			return;
	} else {
		presentFrame();
	}

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
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return;

	ResourceDeltaClipPlayer::drawFrame(_resourceArena, _resourceChunkOffsets[chunkIndex],
		_sceneChunkTable.sizes[chunkIndex], tableEntryCount, frameIndex,
		(byte *)destination.getPixels(), destination.w, destination.h, destination.pitch,
		destination.pitch * destination.h);
}

void Scene2030::runMerchantShopDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializeMerchantShopDialogueRecords(records);

	beginSecondarySpeechLine(kScene2030ShopDialogueStageId, 0);
	openMerchantForInteraction(false);
	runMerchantPrimarySpeechLine(kScene2030ShopPrimaryRow, 0, false);

	byte selectedProduct = 0;
	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene2030ShopDialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene2030ShopDialogueStageId, 3);
			runMerchantPrimarySpeechLine(kScene2030ShopPrimaryRow, 3, false);
			break;
		}

		const uint recordIndex = ((uint)nodeIndex + (uint)depthIndex * 10) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene2030ShopDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff)
			runMerchantPrimarySpeechLine(kScene2030ShopPrimaryRow, record.responseFrameIndex, false);

		if (record.disableAfterUse != 0)
			record.enabled = 0;
		if (record.disableAfterUse == 2) {
			selectedProduct = record.playerTextRowId;
			records[71].enabled = merchantShopProductPrice(selectedProduct) <=
				_vm->gameState().ronEgyptianMoneyAmount ? 1 : 0;
		}
		if (record.disableAfterUse == 3)
			runLeftMerchantPurchase(selectedProduct);

		switch (record.transitionMode) {
		case 0:
			closeMerchantAfterInteraction(false);
			return;
		case 1:
			nodeIndex = record.nextNodeIndex;
			++depthIndex;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			if (depthIndex != 0)
				--depthIndex;
			break;
		case 3:
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = depthIndex > 1 ? (byte)(depthIndex - 2) : 0;
			break;
		default:
			closeMerchantAfterInteraction(false);
			return;
		}
	}

	closeMerchantAfterInteraction(false);
}

void Scene2030::initializeMerchantShopDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.resize(kScene2030ShopDialogueChoiceRecordCount);
	setMerchantShopDialogueRecord(records, 0, 1, 0, 1, 1, 1, 2, 0xff);
	setMerchantShopDialogueRecord(records, 1, 1, 0, 1, 2, 2, 2, 0xff);
	setMerchantShopDialogueRecord(records, 2, 1, 0, 1, 7, 6, 2, 0xff);
	setMerchantShopDialogueRecord(records, 3, 1, 0, 3, 8, 7, 1, 0xff);
	setMerchantShopDialogueRecord(records, 4, 1, 0, 1, 9, 8, 2, 0xff);
	setMerchantShopDialogueRecord(records, 5, 1, 0, 0, 3, 3, 0, 0);
	setMerchantShopDialogueRecord(records, 70, 1, 0, 0, 4, 4, 1, 0xff);
	setMerchantShopDialogueRecord(records, 71, 0, 0, 0, 5, 5, 3, 0xff);
	setMerchantShopDialogueRecord(records, 72, 1, 0, 0, 6, 3, 1, 0xff);

	const GameplayState &state = _vm->gameState();
	if (state.scene2030SeedOfferState != 1) {
		records[0].enabled = 0;
		records[4].enabled = 0;
	}
	if (state.scene2030MerchantItem2AOfferState != 1)
		records[1].enabled = 0;
}

void Scene2030::setMerchantShopDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte enabled, byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte reserved) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = enabled;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = reserved;
}

uint16 Scene2030::merchantShopProductPrice(byte productRowId) const {
	switch (productRowId) {
	case 1:
		return kScene2030SeedsPrice;
	case 2:
		return kScene2030MerchantItem2APrice;
	default:
		return 0xffff;
	}
}

void Scene2030::runLeftMerchantPurchase(byte productRowId) {
	const uint16 price = merchantShopProductPrice(productRowId);
	if (price == 0xffff || _vm->gameState().ronEgyptianMoneyAmount < price)
		return;

	runSynchronizedMerchantOverlay(8, kScene2030LeftMerchantPurchaseDescriptorCount,
		kScene2030LeftMerchantPurchaseOverlayFrameMap,
		kScene2030LeftMerchantPurchasePoseFrameMap,
		ARRAYSIZE(kScene2030LeftMerchantPurchaseOverlayFrameMap), false);

	GameplayState &state = _vm->gameState();
	if (productRowId == 1) {
		addInventoryItem(kScene2030SeedsItem);
		state.scene2030SeedOfferState = 2;
	} else if (productRowId == 2) {
		addInventoryItem(kScene2030MerchantItem2A);
		state.scene2030MerchantItem2AOfferState = 2;
	}
	subtractEgyptianMoney(price);
	_soundBank0.playSample(1, 100);
}

void Scene2030::subtractEgyptianMoney(uint16 amount) {
	GameplayState &state = _vm->gameState();
	if (amount >= state.ronEgyptianMoneyAmount) {
		state.ronEgyptianMoneyAmount = 0;
		removeInventoryItem(kScene2030MoneyItem);
		return;
	}

	state.ronEgyptianMoneyAmount -= amount;
}

void Scene2030::runRightMerchantTalkSequence() {
	GameplayState &state = _vm->gameState();
	const bool firstConversation = !state.scene2030RightMerchantConversationSeen;
	if (firstConversation) {
		beginSecondarySpeechLineAndOpenMerchant(8, 0, true);
		runMerchantPrimarySpeechLine(8, 1, true);
		beginSecondarySpeechLine(8, 2);
		runMerchantPrimarySpeechLine(8, 3, true);
		beginSecondarySpeechLine(8, 4);
		runMerchantPrimarySpeechLine(8, 5, true);
		beginSecondarySpeechLine(8, 6);
	} else {
		beginSecondarySpeechLineAndOpenMerchant(8, 7, true);
		runMerchantPrimarySpeechLine(8, 8, true);
		runMerchantPrimarySpeechLine(8, 9, true);
		beginSecondarySpeechLine(8, 10);
	}
	closeMerchantAfterInteraction(true);
	if (firstConversation)
		state.scene2030RightMerchantConversationSeen = true;
}

void Scene2030::runRightMerchantRejectedTradeSequence() {
	beginSecondarySpeechLineAndOpenMerchant(9, 0, true);
	runRightStallTradeOverlay();
	runMerchantPrimarySpeechLine(9, (byte)(_random.getRandomNumber(1) + 1), true);
	closeMerchantAfterInteraction(true);
}

void Scene2030::runRightStallTradeOverlay() {
	runActorReplacement(7, kScene2030RightMerchantTradeDescriptorCount,
		kScene2030RightStallTradeFrameMap, ARRAYSIZE(kScene2030RightStallTradeFrameMap),
		kScene2030OverlayFrameMillis);
}

void Scene2030::runRightMerchantBuyItemOverlay() {
	runSynchronizedMerchantOverlay(7, kScene2030RightMerchantTradeDescriptorCount,
		kScene2030RightMerchantBuyOverlayFrameMap, kScene2030RightMerchantBuyPoseFrameMap,
		ARRAYSIZE(kScene2030RightMerchantBuyOverlayFrameMap), true);
}

void Scene2030::runSynchronizedMerchantOverlay(uint chunkIndex, uint descriptorCount,
		const byte *overlayFrameMap, const byte *merchantFrameMap, uint frameCount,
		bool rightMerchant) {
	if (frameCount == 0)
		return;

	ResourceSpriteLayer &merchantLayer = rightMerchant ? _rightMerchantLayer : _leftMerchantLayer;
	TimedAnimationChannel &merchantChannel = rightMerchant ? _rightMerchantChannel : _leftMerchantChannel;
	if (rightMerchant)
		_rightMerchantSequenceLocked = true;
	else
		_leftMerchantSequenceLocked = true;

	const bool previousHideActiveActor = _actionOverlayPlayer.beginActorReplacement(
		chunkIndex, descriptorCount, overlayFrameMap, frameCount);
	Scene2030SynchronizedOverlayTarget target(_actionOverlayPlayer, merchantLayer,
		merchantChannel, merchantFrameMap, frameCount);
	playAndPresentAnimationFrames(target, AnimationFrameRange(0, frameCount - 1,
		kScene2030OverlayFrameMillis));
	_actionOverlayPlayer.finish(previousHideActiveActor);

	if (rightMerchant)
		_rightMerchantSequenceLocked = false;
	else
		_leftMerchantSequenceLocked = false;
	drawPlayableComposite();
	presentFrame();
}

void Scene2030::runRightMerchantSaleSequence(byte soldItemId, uint16 moneyAmount, byte merchantFrameIndex, byte secondaryFrameIndex) {
	if (!hasInventoryItem(soldItemId)) {
		runRightMerchantRejectedTradeSequence();
		return;
	}

	beginSecondarySpeechLineAndOpenMerchant(9, 7, true);
	runRightStallTradeOverlay();
	runMerchantPrimarySpeechLine(9, merchantFrameIndex, true);
	beginSecondarySpeechLine(9, secondaryFrameIndex);
	runRightMerchantBuyItemOverlay();
	addEgyptianMoney(moneyAmount);
	removeInventoryItem(soldItemId);
	_soundBank0.playSample(1, 100);
	closeMerchantAfterInteraction(true);
}

void Scene2030::addEgyptianMoney(uint16 amount) {
	GameplayState &state = _vm->gameState();
	if (state.ronEgyptianMoneyAmount == 0 && !hasInventoryItem(kScene2030MoneyItem))
		addInventoryItem(kScene2030MoneyItem);

	const uint32 total = (uint32)state.ronEgyptianMoneyAmount + amount;
	state.ronEgyptianMoneyAmount = (uint16)MIN<uint32>(total, 0xffff);
}

} // End of namespace Hollywood
