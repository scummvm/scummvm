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

#include "hollywood/scenes/playable/scene6100.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene6060ReturnState = 0x17ad;
const uint kScene6100ActorBankTableEntry = 0;
const uint kScene6100ActorPaletteTableEntry = 0x00cc;
const uint kScene6100Resource003RowsOffsetIndex = 0;
const uint32 kScene6100SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6100AnimationFrameMillis = 75;
const uint32 kScene6100SpeechFrameMillis = 125;
const uint32 kScene6100DepartureFrameMillis = 40;
const byte kScene6100CharlieSpeechGroup = 0;
const byte kScene6100LetterSpeechGroup = 1;
const uint16 kScene6100CharlieDialogueRow = 99;
const uint16 kScene6100EnvelopeRow = 9;
const byte kScene6100BriefcaseItem = 0x65;
const byte kScene6100BillyFordEnvelopeItem = 0x69;

const byte kScene6100CharlieFrameMap[] = {
	3, 48, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 19, 18, 17, 12, 25, 26, 27,
	28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 38, 40, 38, 40, 38, 40, 38, 40, 38, 40, 38,
	40, 38, 40, 38, 40, 38, 40, 38, 40, 38, 40, 38,
	40, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47
};

const byte kScene6100BriefcaseFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene6100LetterFrameMap[] = {
	0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

enum Scene6100LayerId {
	kScene6100CharlieLayer,
	kScene6100LetterLayer,
	kScene6100DepartureLayer
};

const SceneLayerSpec kScene6100LayerSpecs[] = {
	{kSceneAnimationInFrontOfActors, 5, 0x31, kScene6100CharlieFrameMap,
		ARRAYSIZE(kScene6100CharlieFrameMap), false, 2},
	{kSceneAnimationInFrontOfActors, 8, 0x0c, kScene6100LetterFrameMap,
		ARRAYSIZE(kScene6100LetterFrameMap), false, 0},
	{kSceneAnimationBehindActors, 6, 0x17, nullptr, 0, false, 0}
};

static PlayableSceneConfig scene6100Config() {
	PlayableSceneConfig config(6100,
		SceneResourceLayout(10, 5, 9),
		SceneViewport(0x00a0, 0, 0x00a0),
		SceneActorPose(0x308, 0x1df, 4));
	config.setActorResources(kScene6100ActorBankTableEntry, kScene6100ActorPaletteTableEntry);
	config.setTextResources(kScene6100Resource003RowsOffsetIndex, kScene6100SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 4;
	config.useActorDepthTest = true;
	return config;
}

Scene6100::Scene6100(HollywoodEngine *vm) :
		PlayableScene(vm, scene6100Config()),
		_charlieIdleChannel(),
		_charlieConversationChannel(),
		_charlieIdleState(2),
		_charliePose(0),
		_charlieManualSequenceActive(false),
		_charlieConversationActive(false) {
	_sceneLayers.configure(kScene6100LayerSpecs);
}

void Scene6100::initializeCustomPreviewState() {
	_activeActorWorldX = 0x308;
	_activeActorWorldY = 0x1df;
	_activeActorFacing = 4;
	_viewportXOffset = 0x00a0;
	resetSceneLayers();
	initializeDefaultPreviewState();
}

void Scene6100::resetSceneLayers() {
	_sceneLayers.reset();
	_charlieIdleChannel.reset(2, kScene6100AnimationFrameMillis);
	_charlieConversationChannel.reset(2, kScene6100SpeechFrameMillis);
	_charlieIdleState = 2;
	_charliePose = 0;
	_charlieManualSequenceActive = false;
	_charlieConversationActive = false;
}

void Scene6100::runCustomEntrySequence() {
	runEntryPath(0x308, 0x1df, 4, 0x26c, 0x177);

	GameplayState &state = _vm->gameState();
	if (state.scene6100EntryLineSeen)
		return;

	state.scene6100EntryLineSeen = true;
	beginSecondarySpeechLine(0, 0);
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;
	walkActiveActorTo(0x14f, 0x1df, 5, 0, false);
	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		runCharlieDialogue();
}

void Scene6100::prepareCustomGameplayLoop() {
	const byte charlieFrame = _sceneLayers.layerFrame(kScene6100CharlieLayer);
	_charlieIdleChannel.reset(charlieFrame, kScene6100AnimationFrameMillis);
	_charlieConversationChannel.reset(charlieFrame, kScene6100SpeechFrameMillis);
	_sceneLayers.setLayerVisible(kScene6100LetterLayer, false);
	_sceneLayers.setLayerVisible(kScene6100DepartureLayer, false);
	_charlieManualSequenceActive = false;
	_charlieConversationActive = false;
}

void Scene6100::advanceCustomGameplayLoop(uint32 delta) {
	if (!_primaryDialogueSpeechActive && !_charlieManualSequenceActive) {
		if (_charlieConversationActive)
			advanceCharlieConversationIdle(delta);
		else
			advanceCharlieIdle(delta);
	}
}

void Scene6100::advancePrimarySpeechAnimation(uint32 delta) {
	if (!_primaryDialogueSpeechActive)
		return;
	if (_primaryDialogueSpeechGroup == kScene6100LetterSpeechGroup)
		advanceLetterReadingSpeech(delta);
	else
		PlayableScene::advancePrimarySpeechAnimation(delta);
}

void Scene6100::advanceCharlieIdle(uint32 delta) {
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene6100CharlieLayer);
	if (!layer.visible)
		return;

	const uint ticks = _charlieIdleChannel.consumeFrames(delta);
	for (uint tick = 0; tick < ticks; ++tick) {
		switch (_charlieIdleState) {
		case 0:
			if (_random.getRandomNumber(14) == 0) {
				layer.setFrame(1);
				_charlieIdleState = 1;
			} else if (_random.getRandomNumber(19) == 0) {
				layer.setFrame(2);
				_charlieIdleState = 2;
			}
			break;
		case 1:
			layer.setFrame(0);
			_charlieIdleState = 0;
			break;
		case 2:
			if (layer.frameIndex == 7) {
				if (_random.getRandomNumber(19) == 0) {
					layer.setFrame(0);
					_charlieIdleState = 0;
				} else {
					layer.setFrame(2);
				}
			} else {
				layer.setFrame(layer.frameIndex + 1);
			}
			break;
		default:
			_charlieIdleState = 0;
			break;
		}
	}
}

void Scene6100::advanceCharlieConversationIdle(uint32 delta) {
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene6100CharlieLayer);
	if (!layer.visible)
		return;

	const byte baseFrame = _charliePose == 0 ? 16 : (_charliePose == 1 ? 24 : 78);
	const byte specialFrame = baseFrame + 4;
	const uint ticks = _charlieConversationChannel.consumeFrames(delta);
	for (uint tick = 0; tick < ticks; ++tick) {
		if (layer.frameIndex == specialFrame)
			layer.setFrame(baseFrame);
		else if (_random.getRandomNumber(14) == 0)
			layer.setFrame(specialFrame);
	}
}

void Scene6100::advanceLetterReadingSpeech(uint32 delta) {
	ResourceSpriteLayer &layer = _sceneLayers.layer(kScene6100CharlieLayer);
	_primaryDialogueSpeechTimerAccumulator += delta;
	while (_primaryDialogueSpeechTimerAccumulator >= kScene6100SpeechFrameMillis) {
		_primaryDialogueSpeechTimerAccumulator -= kScene6100SpeechFrameMillis;
		if (layer.frameIndex == 42)
			layer.setFrame(41);
		else if (_random.getRandomNumber(2) == 0)
			layer.setFrame(42);
	}
}

bool Scene6100::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar vitrina vacía (look at the empty display case).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Mirar gárgola (look at the gargoyle).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Hablar con Charlie (talk to Charlie).
		runCharlieDialogue();
		return true;
	case 304: // Mirar Charlie (look at Charlie).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar letrero (look at the sign).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar vitrina de la Criatura de la Laguna Negra (look at its display).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Ir a escalera (go downstairs): return to the lower museum floor.
		_vm->gameState().mainFlowStateId = kScene6060ReturnState;
		return true;
	case 308: // Mirar escalera (look at the staircase).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Coger maletín de Charlie (take Charlie's briefcase).
		takeCharlieBriefcase();
		return true;
	case 310: // Mirar maletín de Charlie (look at Charlie's briefcase).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Dar sobre de Billy Ford a Charlie (give Billy Ford's envelope to Charlie).
		giveBillyFordEnvelopeToCharlie();
		return true;
	default:
		return false;
	}
}

bool Scene6100::customizeRouteSegment(byte currentRegion, byte nextRegion,
		const ActorPathBuildState &state, const ScenePoint &boundary,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	if (currentRegion != 3 || nextRegion != 1)
		return false;

	for (uint i = 0; i < 0x0c && 0x30 + i < _actorPathStepDeltas.size(); ++i)
		_actorPathStepDeltas[0x30 + i] = kActorPathStepDeltaTableSetB4[i];
	requestedFacing = 4;
	restoredStepDeltas = true;
	return true;
}

bool Scene6100::customizeRouteFinal(byte currentRegion, byte targetRegion,
		const ActorPathBuildState &state, int targetX, int targetY,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)targetRegion;
	(void)state;
	if (targetX != 0x308 || targetY != 0x1df)
		return false;

	for (uint i = 0; i < 0x0c && 0x0c + i < _actorPathStepDeltas.size(); ++i)
		_actorPathStepDeltas[0x0c + i] = kActorPathStepDeltaTableSetB4[0x24 + i];
	requestedFacing = 1;
	restoredStepDeltas = true;
	return true;
}

bool Scene6100::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_fullPaletteRegionMask.size() < kScenePaletteMapPageSize ||
			_walkablePaletteMask.size() < kScenePaletteMapPageSize)
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());

	const GameplayState &state = _vm->gameState();
	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		const byte originalRegion = _paletteMaskOriginal[color];
		byte fullRegion = originalRegion;
		if (originalRegion == 4)
			fullRegion = state.scene6100CharlieState == 0 ? 1 : 0;

		_fullPaletteRegionMask[color] = fullRegion;
		_walkablePaletteMask[color] = fullRegion == 3 ? 0 : fullRegion;
		_paletteMask[color] = _walkablePaletteMask[color];

		const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + color];
		byte item = originalItem;
		if (state.scene6100CharlieState == 0 && originalItem == 3)
			item = 0;
		if (!state.scene6100BriefcasePresent && originalItem == 7)
			item = 0;
		_paletteMask[kSceneColorToItemMap + color] = item;
	}

	if (!state.scene6100BriefcasePresent && _sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);

	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (state.scene6100CharlieState == 0) {
		ScenePoint interactionPoint;
		interactionPoint.x = 0x0e4;
		interactionPoint.y = 0x1df;
		SceneActionTarget target = _hotspots.actionTarget(2);
		_hotspots.setActionTarget(2, interactionPoint, target.approachPoint);
		target = _hotspots.actionTarget(7);
		_hotspots.setActionTarget(7, interactionPoint, target.approachPoint);
	}

	_sceneLayers.setLayerVisible(kScene6100CharlieLayer,
		state.scene6100CharlieState != 0);
	return true;
}

byte Scene6100::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene6100LetterSpeechGroup)
		return 41;
	return _charliePose == 0 ? 16 : (_charliePose == 1 ? 24 : 78);
}

uint32 Scene6100::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene6100SpeechFrameMillis;
}

void Scene6100::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_sceneLayers.setVisibleLayerFrame(kScene6100CharlieLayer, frameIndex);
}

void Scene6100::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	_sceneLayers.setVisibleLayerFrame(kScene6100CharlieLayer, baseFrame);
	_charlieConversationChannel.resetTimer();
}

void Scene6100::enterCharlieDialoguePose() {
	_charlieManualSequenceActive = true;
	const byte frameIndex = _sceneLayers.layerFrame(kScene6100CharlieLayer);
	if (frameIndex < 16) {
		playAnimationFrames(kScene6100CharlieLayer,
			AnimationFrameRange(frameIndex + 1, 16,
				kScene6100AnimationFrameMillis));
	}
	_charliePose = 0;
	_charlieManualSequenceActive = false;
}

void Scene6100::switchCharlieToAlternatePose() {
	_charlieManualSequenceActive = true;
	playAnimationFrames(kScene6100CharlieLayer,
		AnimationFrameRange(20, 24, kScene6100AnimationFrameMillis));
	_charliePose = 1;
	_charlieManualSequenceActive = false;
}

void Scene6100::returnCharlieToDialoguePose() {
	_charlieManualSequenceActive = true;
	playAnimationFrames(kScene6100CharlieLayer,
		AnimationFrameRange(28, 32, kScene6100AnimationFrameMillis));
	_charliePose = 0;
	_charlieManualSequenceActive = false;
}

void Scene6100::finishCharlieDialoguePose() {
	_charlieConversationActive = false;
	_charlieManualSequenceActive = true;
	_sceneLayers.setLayerFrame(kScene6100CharlieLayer, 16);
	playAnimationFrames(kScene6100CharlieLayer,
		AnimationFrameRange(15, 7, kScene6100AnimationFrameMillis));
	_charlieIdleState = 0;
	_charlieIdleChannel.reset(_sceneLayers.layerFrame(kScene6100CharlieLayer),
		kScene6100AnimationFrameMillis);
	_charlieManualSequenceActive = false;
}

void Scene6100::beginCharlieSpeechLine(uint16 rowIndex, byte frameIndex, byte animationGroup) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, 0x0fa, 0x131,
		6, 0x3f, 0x2d, animationGroup);
}

void Scene6100::runCharlieDialogue() {
	if (_vm->gameState().scene6100CharlieState == 0)
		return;

	Common::Array<DialogueChoiceRecord> records;
	initializeCharlieDialogueRecords(records);
	GameplayState &state = _vm->gameState();
	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;

	const byte greetingFrame = state.scene6100CharlieState == 2 ? 1 : 0;
	beginSecondarySpeechLine(0x62, greetingFrame);
	enterCharlieDialoguePose();
	_charlieConversationActive = true;
	beginCharlieSpeechLine(kScene6100CharlieDialogueRow, greetingFrame, kScene6100CharlieSpeechGroup);
	if (state.scene6100CharlieState != 2)
		state.scene6100CharlieState = 2;

	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(0x62, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(0x62, 6);
			beginCharlieSpeechLine(kScene6100CharlieDialogueRow, 6, kScene6100CharlieSpeechGroup);
			break;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(0x62, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff) {
			const bool alternatePose = _random.getRandomNumber(1) != 0;
			if (!alternatePose && _charliePose != 0)
				returnCharlieToDialoguePose();
			else if (alternatePose && _charliePose == 0)
				switchCharlieToAlternatePose();

			beginCharlieSpeechLine(kScene6100CharlieDialogueRow, record.responseFrameIndex,
				kScene6100CharlieSpeechGroup);
			if (_charliePose != 0)
				returnCharlieToDialoguePose();
		}

		if (record.disableAfterUse == 1) {
			record.enabled = 0;
			record.selectable = 0;
		}

		switch (record.transitionMode) {
		case 0:
			finished = true;
			break;
		case 1:
			nodeIndex = record.nextNodeIndex;
			if (depthIndex < 9)
				++depthIndex;
			else
				finished = true;
			break;
		case 2:
			nodeIndex = record.nextNodeIndex;
			if (depthIndex != 0)
				--depthIndex;
			else
				finished = true;
			break;
		case 3:
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			if (depthIndex >= 2)
				depthIndex -= 2;
			else
				finished = true;
			break;
		default:
			break;
		}
	}

	finishCharlieDialoguePose();
}

void Scene6100::initializeCharlieDialogueRecords(
		Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(10 * 10 * 7);

	setCharlieDialogueRecord(records, 0, 1, 0, 3, 2, 2, 1);
	setCharlieDialogueRecord(records, 1, 1, 0, 1, 3, 3, 1);
	setCharlieDialogueRecord(records, 2, 1, 1, 1, 4, 4, 1);
	setCharlieDialogueRecord(records, 3, 1, 0, 3, 5, 5, 1);
	setCharlieDialogueRecord(records, 4, 1, 0, 0, 6, 6, 1);

	setCharlieDialogueRecord(records, 70, 1, 0, 3, 7, 7, 1);
	setCharlieDialogueRecord(records, 71, 1, 0, 3, 8, 8, 1);
	setCharlieDialogueRecord(records, 72, 1, 0, 3, 9, 9, 1);
	setCharlieDialogueRecord(records, 73, 1, 0, 2, 10, 10, 1);

	setCharlieDialogueRecord(records, 77, 1, 1, 3, 11, 11, 1);
	setCharlieDialogueRecord(records, 78, 1, 1, 3, 12, 12, 1);
	setCharlieDialogueRecord(records, 79, 1, 1, 3, 13, 13, 1);
	setCharlieDialogueRecord(records, 80, 1, 0, 2, 14, 14, 1);
}

void Scene6100::setCharlieDialogueRecord(Common::Array<DialogueChoiceRecord> &records,
		uint index, byte enabled, byte nextNodeIndex, byte transitionMode,
		byte playerTextRowId, byte responseFrameIndex, byte disableAfterUse) const {
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
	record.selectable = enabled != 0;
}

void Scene6100::takeCharlieBriefcase() {
	GameplayState &state = _vm->gameState();
	if (state.scene6100CharlieState != 0) {
		beginSecondarySpeechLine(7, 0);
		return;
	}
	if (!state.scene6100BriefcasePresent)
		return;

	BlockingSequence sequence(*this);
	sequence.secondarySpeech(7, 1)
		.commit(_charlieManualSequenceActive, true)
		.actorReplacement(ActionOverlaySpec(9, 0x0d, kScene6100BriefcaseFrameMap,
			ARRAYSIZE(kScene6100BriefcaseFrameMap), kScene6100AnimationFrameMillis)
			.drawAt(kSceneAnimationInFrontOfActors)
			.commitAt(7, state.scene6100BriefcasePresent, false)
			.patchAt(7, 2))
		.commit(_charlieManualSequenceActive, false);
	if (state.scene6100BriefcasePresent) {
		sequence.commit(state.scene6100BriefcasePresent, false)
			.framebufferPatch(2);
	}
	addInventoryItem(kScene6100BriefcaseItem);
	sequence.sound(1);
}

void Scene6100::giveBillyFordEnvelopeToCharlie() {
	GameplayState &state = _vm->gameState();
	if (state.scene6100CharlieState == 0 ||
			!hasInventoryItem(kScene6100BillyFordEnvelopeItem))
		return;

	BlockingSequence sequence(*this);
	sequence.secondarySpeech(9, 0)
		.commit(_charlieManualSequenceActive, true);
	enterCharlieDialoguePose();
	sequence.commit(_charlieManualSequenceActive, true);
	beginCharlieSpeechLine(kScene6100EnvelopeRow, 1, kScene6100CharlieSpeechGroup);

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	_sceneLayers.showLayerAtFrame(kScene6100LetterLayer, 0);
	sequence.layerFrames(kScene6100LetterLayer,
			AnimationFrameRange(0, 7, kScene6100AnimationFrameMillis))
		.layerFrames(kScene6100CharlieLayer,
			AnimationFrameRange(32, 41, kScene6100AnimationFrameMillis))
		.layerFrames(kScene6100LetterLayer,
			AnimationFrameRange(7, 12, kScene6100AnimationFrameMillis));
	_sceneLayers.setLayerVisible(kScene6100LetterLayer, false);
	_hideActiveActor = previousHideActiveActor;

	removeInventoryItem(kScene6100BillyFordEnvelopeItem);
	sequence.sound(1);
	beginCharlieSpeechLine(kScene6100EnvelopeRow, 2, kScene6100LetterSpeechGroup);
	sequence.layerFrames(kScene6100CharlieLayer,
			AnimationFrameRange(42, 78, kScene6100AnimationFrameMillis))
		.commit(_charliePose, (byte)2);
	beginCharlieSpeechLine(kScene6100EnvelopeRow, 3, kScene6100CharlieSpeechGroup);

	_sceneLayers.setLayerVisible(kScene6100CharlieLayer, false);
	_sceneLayers.showLayerAtFrame(kScene6100DepartureLayer, 0);
	sequence.layerFrames(kScene6100DepartureLayer,
		AnimationFrameRange(0, 22, kScene6100DepartureFrameMillis));
	_sceneLayers.setLayerVisible(kScene6100DepartureLayer, false);
	sequence.commit(state.scene6100CharlieState, (byte)0)
		.framebufferPatch(1)
		.commit(_charlieManualSequenceActive, false)
		.actorPath(SceneActorPose(0x14f, 0x1df, 1))
		.secondarySpeech(9, 4);
}

bool Scene6100::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void Scene6100::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

AmbientAudioProfile Scene6100::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

} // End of namespace Hollywood
