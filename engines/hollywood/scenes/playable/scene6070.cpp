/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file for details.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hollywood/scenes/playable/scene6070.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint kScene6070StageIndex = 607;
const uint kScene6070AlternateStageIndex = 609;
const uint16 kScene6070FirstState = 0x17b6;
const uint16 kScene6070EntryFromScene6080State = 0x17b7;
const uint16 kScene6070ArrivalCutsceneState = 0x17b8;
const uint16 kScene6070AlternateCutsceneState = 0x17b9;
const uint16 kScene6070ReturnAfterRescueState = 0x17ba;
const uint16 kScene6070ReturnFromDialogueState = 0x17bb;
const uint16 kScene6050ReturnState = 0x17a4;
const uint16 kScene6080EntryState = 0x17c0;
const uint16 kScene6090EntryState = 0x17ca;
const uint16 kScene7100EntryState = 0x1bbc;
const uint16 kScene7100DialogueEntryState = 0x1bbd;
const uint16 kScene6070ViewportMinX = 0x0068;
const uint16 kScene6070ViewportMaxX = 0x0100;
const uint16 kScene6070ViewportFromScene6050 = 0x0100;
const uint16 kScene6070ViewportFromScene6080 = 0x0068;
const uint16 kScene6070ViewportFromScene7100 = 0x00a8;
const uint16 kScene6070ArrivalViewportMaxX = 0x0088;
const uint kScene6070ActorBankTableEntry = 0x0000;
const uint kScene6070ActorPaletteTableEntry = 0x00cc;
const uint kScene6070Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6070SueFrameMillis = 125;
const uint32 kScene6070SueSpeechCheckMillis = 40;
const uint32 kScene6070OverlayFrameMillis = 75;
const uint32 kScene6070DiscoveryFrameMillis = 40;
const uint32 kScene6070State609SlowFrameMillis = 125;
const uint32 kScene6070State609FastFrameMillis = 60;
const byte kScene6070SueIdleSpeechVolumePercent = 25;
const byte kScene6070SueSpeechGroup = 0;
const byte kScene6070State609PropSpeechGroup = 1;
const byte kScene6070State609NpcSpeechGroup = 2;
const byte kScene6070State609SueSpeechGroup = 3;
const byte kScene6070SueIdleSpeechGroup = 4;
const byte kScene6070TransferFrameHook = 1;
const uint kScene6070FixedGiveHandlerIndex = 0x110;
const uint16 kScene6070FixedGiveHandlerDefault = 0x79;

const byte kScene6070ArrivalFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12,
	12, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 51, 51};

const byte kScene6070TransferFrameMap[] = {
	0, 11, 10, 9, 8, 7, 6, 5, 5, 5, 4, 3, 2, 1, 0};

const byte kScene6070TransferSueFrameMap[] = {
	14, 14, 14, 14, 14, 14, 14, 14, 19, 20, 21, 22, 23, 24, 14};

const byte kScene6070State609PropFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	7, 6, 5, 0, 13, 14, 15, 16, 17, 18, 19, 0};

const byte kScene6070State609NpcFrameMap[] = {
	0, 5, 6, 7, 8, 9, 10, 11, 12, 7, 6, 5, 0};

struct Scene6070DialogueSeedRecord {
	uint16 index;
	byte enabled;
	byte nextNodeIndex;
	byte transitionMode;
	byte playerTextRowId;
	byte responseFrameIndex;
	byte disableAfterUse;
};

const Scene6070DialogueSeedRecord kScene6070SueDialogueSeedRecords[] = {
	{0, 1, 0, 0, 8, 8, 1},
	{1, 1, 0, 3, 4, 4, 1},
	{2, 1, 0, 3, 5, 5, 1},
	{3, 1, 0, 3, 6, 6, 1},
	{4, 1, 0, 3, 7, 7, 1},
	{5, 1, 0, 0, 9, 9, 1}};

static bool isScene6070AlternateCutscene(const HollywoodEngine *vm) {
	return vm->gameState().mainFlowStateId == kScene6070AlternateCutsceneState;
}

static uint16 scene6070InitialViewport(const HollywoodEngine *vm) {
	switch (vm->gameState().mainFlowStateId) {
	case kScene6070FirstState:
		return kScene6070ViewportFromScene6050;
	case kScene6070EntryFromScene6080State:
	case kScene6070AlternateCutsceneState:
		return kScene6070ViewportFromScene6080;
	default:
		return kScene6070ViewportFromScene7100;
	}
}

static PlayableSceneConfig scene6070Config(HollywoodEngine *vm) {
	PlayableSceneConfig config(6070,
		SceneResourceLayout(15, 5, 13),
		SceneViewport(scene6070InitialViewport(vm), kScene6070ViewportMinX, kScene6070ViewportMaxX),
		SceneActorPose(0x0fc, 0x156, 5));
	config.stageIndex = isScene6070AlternateCutscene(vm) ? kScene6070AlternateStageIndex : kScene6070StageIndex;
	config.setActorResources(kScene6070ActorBankTableEntry, kScene6070ActorPaletteTableEntry);
	config.setTextResources(kScene6070Resource003RowsOffsetIndex, kScene6070SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 6;
	return config;
}

Scene6070::Scene6070(HollywoodEngine *vm) :
		PlayableScene(vm, scene6070Config(vm)),
		_originalColorToItemMap(),
		_sueLayer(),
		_arrivalLayer(),
		_state609SueLayer(),
		_state609PropLayer(),
		_state609NpcLayer(),
		_sueIdleChannel(),
		_state609PropChannel(),
		_sueSpeechTimerAccumulator(0),
		_sueMode(0),
		_completedSueSpeechCount(0),
		_lastSueIdleSpeechFrame(0xff),
		_manualSequenceActive(false),
		_pendingRonRetort(false),
		_state609PropContinuous(false),
		_state609PropAlternatePose(false) {
	_sueLayer.configure(8, 25, nullptr, 0);
	_arrivalLayer.configure(9, 0x34, kScene6070ArrivalFrameMap,
							ARRAYSIZE(kScene6070ArrivalFrameMap));
	_state609SueLayer.configure(13, 5, nullptr, 0);
	_state609PropLayer.configure(12, 0x14, kScene6070State609PropFrameMap,
								 ARRAYSIZE(kScene6070State609PropFrameMap));
	_state609NpcLayer.configure(11, 0x0d, kScene6070State609NpcFrameMap,
								ARRAYSIZE(kScene6070State609NpcFrameMap));
}

int Scene6070::alternatePaletteResourceChunkIndex() const {
	return 14;
}

bool Scene6070::isAlternatePaletteResourceActive() const {
	return isScene6070AlternateCutscene(_vm);
}

bool Scene6070::shouldLoadArenaChunk(uint index) const {
	return index <= 10 || isScene6070AlternateCutscene(_vm);
}

void Scene6070::initializeCustomPreviewState() {
	GameplayState &state = _vm->gameState();
	const uint16 stateId = state.mainFlowStateId;
	if (stateId == kScene6070ArrivalCutsceneState)
		state.scene6070SuePresent = false;

	switch (stateId) {
	case kScene6070FirstState:
		_activeActorWorldX = 0x3b0;
		_activeActorWorldY = 0x0ae;
		_activeActorFacing = 4;
		break;
	case kScene6070EntryFromScene6080State:
		_activeActorWorldX = 0x0c5;
		_activeActorWorldY = 0x139;
		_activeActorFacing = 2;
		break;
	case kScene6070AlternateCutsceneState:
		_activeActorWorldX = 0x122;
		_activeActorWorldY = 0x150;
		_activeActorFacing = 2;
		break;
	case kScene6070ReturnAfterRescueState:
		_activeActorWorldX = 0x288;
		_activeActorWorldY = 0x19a;
		_activeActorFacing = 5;
		break;
	default:
		_activeActorWorldX = 0x0fc;
		_activeActorWorldY = 0x156;
		_activeActorFacing = 5;
		break;
	}

	resetSceneLayers();
	initializeDefaultPreviewState();
}

void Scene6070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
									int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
									byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
									byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();

	if (isScene6070AlternateCutscene(_vm)) {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel,
										  activeWorldX, activeWorldY, drawSecondaryActor, secondaryFacing,
										  secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		drawResourceSpriteLayer(_state609SueLayer);
		drawResourceSpriteLayer(_state609PropLayer);
		drawResourceSpriteLayer(_state609NpcLayer);
		return;
	}

	drawResourceSpriteLayer(_sueLayer);
	drawResourceSpriteLayer(_arrivalLayer);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel,
									  activeWorldX, activeWorldY, drawSecondaryActor, secondaryFacing,
									  secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();

	const int orderX = drawActiveActor ? activeWorldX : _activeActorWorldX;
	const int orderY = drawActiveActor ? activeWorldY : _activeActorWorldY;
	const byte orderMode = drawActiveActor ? actorDrawOrderMode : _activeActorDrawOrderMode;
	drawNormalForeground(orderX, orderY, orderMode);
}

void Scene6070::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene6070FirstState:
		_soundBank0.playSample(4, 100);
		runEntryPath(0x3b0, 0x0ae, 4, 0x2e3, 0x15a);
		break;
	case kScene6070EntryFromScene6080State:
		runEntryPath(0x0c5, 0x139, 2, 0x122, 0x162);
		break;
	case kScene6070ArrivalCutsceneState:
		runArrivalCutscene();
		break;
	case kScene6070AlternateCutsceneState:
		runState609Cutscene();
		break;
	case kScene6070ReturnAfterRescueState:
	case kScene6070ReturnFromDialogueState:
		drawPlayableComposite();
		runCurtainRevealFromBlack();
		break;
	default:
		drawPlayableComposite();
		presentFrame();
		break;
	}
}

bool Scene6070::shouldPresentPreviewBeforeEntrySequence() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return stateId < kScene6070ArrivalCutsceneState || stateId > kScene6070ReturnFromDialogueState;
}

bool Scene6070::prepareCustomGameplayLoop() {
	_sueIdleChannel.reset(_sueLayer.frameIndex, kScene6070SueFrameMillis);
	_state609PropChannel.reset(_state609PropLayer.frameIndex,
							   kScene6070State609SlowFrameMillis);
	_sueSpeechTimerAccumulator = 0;
	return true;
}

bool Scene6070::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);

	if (isScene6070AlternateCutscene(_vm))
		advanceState609Prop(delta);
	else
		advanceSueIdle(delta);

	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene6070::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Mirar puerta oscura (look at the dark door).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar/Abrir puerta oscura (use/open the dark door): it is locked.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Mirar celda (look at the cell): discover Sue on the first look.
		runSueDiscovery();
		return true;
	case 304: // Ir a escalera (go upstairs): return to the museum floor.
		_soundBank0.playSample(3, 100);
		state.mainFlowStateId = kScene6050ReturnState;
		return true;
	case 305: // Mirar escalera (look at the stairs).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Hablar con Sue (talk to Sue): open the rescue dialogue.
		runSueDialogue();
		return true;
	case 307: // Mirar Sue (look at Sue).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Dar objeto a Sue (give an inventory item to Sue).
		handleGiveItemToSue();
		return true;
	case 309: // Usar/Abrir puerta de la celda (use/open the cell door).
		if (state.scene6070CellDoorOpen)
			dispatchGenericSceneAction(8);
		else
			beginSecondarySpeechLine(2, 0);
		return true;
	case 310: // Cerrar puerta de la celda (close the cell door).
		dispatchGenericSceneAction(state.scene6070CellDoorOpen ? 19 : 11);
		return true;
	case 311: // Ir por puerta de la celda (go through the cell door).
		if (state.scene6070CellDoorOpen)
			state.mainFlowStateId = kScene6080EntryState;
		return true;
	default:
		return false;
	}
}

void Scene6070::prepareCustomActorPathRoute(int startX, int startY) {
	if (startX < 600) {
		if (_routeSteps.size() > 0x333) {
			_routeSteps[0x331] = startY < 400 ? 6 : 4;
			_routeSteps[0x332] = startY < 400 ? 5 : 3;
			_routeSteps[0x333] = 1;
		}
		return;
	}

	if (_routeSteps.size() > 0x1b7) {
		_routeSteps[0x1b5] = startY < 0x168 ? 5 : 3;
		_routeSteps[0x1b6] = startY < 0x168 ? 6 : 4;
		_routeSteps[0x1b7] = 2;
	}
}

bool Scene6070::customizeRouteSegment(byte currentRegion, byte nextRegion,
									  const ActorPathBuildState &state, const ScenePoint &boundary,
									  int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	if (currentRegion != 7 || nextRegion != 1)
		return false;

	for (uint i = 0; i < 0x0c && 0x30 + i < _actorPathStepDeltas.size(); ++i)
		_actorPathStepDeltas[0x30 + i] = kActorPathStepDeltaTableSet87[0x30 + i];
	requestedFacing = 4;
	restoredStepDeltas = true;
	return true;
}

bool Scene6070::customizeRouteFinal(byte currentRegion, byte targetRegion,
									const ActorPathBuildState &state, int targetX, int targetY,
									int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)targetRegion;
	(void)state;
	(void)targetY;
	if (targetX != 0x3b0)
		return false;

	for (uint i = 0; i < 0x0c && 0x0c + i < _actorPathStepDeltas.size(); ++i)
		_actorPathStepDeltas[0x0c + i] = kActorPathStepDeltaTableSet87[0x0c + i];
	requestedFacing = 1;
	restoredStepDeltas = true;
	return true;
}

bool Scene6070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	rememberOriginalColorMap();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(),
		   _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (!state.scene6070SuePresent)
		replaceColorMapItemFromOriginal(4, 1);

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		const byte originalRegion = _paletteMaskOriginal[color];
		byte region = originalRegion;
		if (state.scene6070CellDoorOpen) {
			if (originalRegion == 9)
				region = 0;
			else if (originalRegion == 10)
				region = 8;
		} else if (originalRegion == 9 || originalRegion == 10) {
			region = 2;
		}
		_paletteMask[color] = region;
		_fullPaletteRegionMask[color] = region;
	}

	if (state.scene6070CellDoorOpen && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);

	rebuildSceneWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (state.scene6070SuePresent)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x0c, 0);
	if (state.scene6070CellDoorOpen) {
		SceneActionTarget target = _hotspots.actionTarget(1);
		ScenePoint interactionPoint;
		interactionPoint.x = 0x0c5;
		interactionPoint.y = 0x139;
		_hotspots.setActionTarget(1, interactionPoint, target.approachPoint);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(9, 3);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x0c, 0);
	}

	_sueLayer.visible = state.scene6070SuePresent && !isScene6070AlternateCutscene(_vm);
	if (state.mainFlowStateId == kScene6070FirstState &&
		kScene6070FixedGiveHandlerIndex < GameplayState::kFixedInventoryActionTableEntryCount) {
		state.fixedInventoryVerbHandlerIdsByItemAndStrip[kScene6070FixedGiveHandlerIndex] = 1;
	}
	return true;
}

byte Scene6070::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	switch (animationGroup) {
	case kScene6070State609PropSpeechGroup:
		return _state609PropAlternatePose ? 8 : 0;
	case kScene6070State609NpcSpeechGroup:
		return 4;
	case kScene6070State609SueSpeechGroup:
		return 0;
	default:
		return _sueMode == 2 ? 0 : 14;
	}
}

uint32 Scene6070::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	return animationGroup == kScene6070State609NpcSpeechGroup ?
		kScene6070State609FastFrameMillis : kScene6070SueFrameMillis;
}

byte Scene6070::primarySpeechVolumePercent(byte animationGroup) const {
	return animationGroup == kScene6070SueIdleSpeechGroup ?
		kScene6070SueIdleSpeechVolumePercent : 100;
}

void Scene6070::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	switch (animationGroup) {
	case kScene6070State609PropSpeechGroup:
		_state609PropLayer.setFrame(frameIndex);
		break;
	case kScene6070State609NpcSpeechGroup:
		_state609NpcLayer.setFrame(frameIndex);
		break;
	case kScene6070State609SueSpeechGroup:
		_state609SueLayer.setFrame(frameIndex);
		break;
	default:
		_sueLayer.setFrame(frameIndex);
		break;
	}
}

void Scene6070::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	setPrimarySpeechAnimationFrame(animationGroup, baseFrame);
	if ((animationGroup != kScene6070SueSpeechGroup &&
			animationGroup != kScene6070SueIdleSpeechGroup) || _sueMode != 0)
		return;

	++_completedSueSpeechCount;
	if (_completedSueSpeechCount >= 5) {
		_completedSueSpeechCount = 0;
		_pendingRonRetort = true;
	}
}

void Scene6070::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId != kScene6070TransferFrameHook ||
		frame >= ARRAYSIZE(kScene6070TransferSueFrameMap))
		return;
	_sueLayer.setFrame(kScene6070TransferSueFrameMap[frame]);
}

bool Scene6070::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void Scene6070::runExitSideEffectsAfterLoop() {
	GameplayState &state = _vm->gameState();
	if (kScene6070FixedGiveHandlerIndex < GameplayState::kFixedInventoryActionTableEntryCount) {
		state.fixedInventoryVerbHandlerIdsByItemAndStrip[kScene6070FixedGiveHandlerIndex] =
			kScene6070FixedGiveHandlerDefault;
	}
	if (state.mainFlowStateId == kScene6050ReturnState ||
		state.mainFlowStateId == kScene6080EntryState)
		fadePaletteToBlack();
}

AmbientAudioProfile Scene6070::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene6070::resetSceneLayers() {
	_sueLayer.reset(14);
	_sueLayer.visible = _vm->gameState().scene6070SuePresent &&
						!isScene6070AlternateCutscene(_vm);
	_arrivalLayer.reset(0);
	_arrivalLayer.visible = false;
	_state609SueLayer.reset(0);
	_state609PropLayer.reset(0);
	_state609NpcLayer.reset(0);
	const bool alternate = isScene6070AlternateCutscene(_vm);
	_state609SueLayer.visible = alternate;
	_state609PropLayer.visible = alternate;
	_state609NpcLayer.visible = alternate;
	_sueIdleChannel.reset(14, kScene6070SueFrameMillis);
	_state609PropChannel.reset(0, kScene6070State609SlowFrameMillis);
	_sueSpeechTimerAccumulator = 0;
	_sueMode = alternate ? 2 : 0;
	_completedSueSpeechCount = 0;
	_lastSueIdleSpeechFrame = 0xff;
	_manualSequenceActive = false;
	_pendingRonRetort = false;
	_state609PropContinuous = false;
	_state609PropAlternatePose = false;
}

void Scene6070::advanceSueIdle(uint32 delta) {
	if (!_sueLayer.visible || _sueMode != 0 || _manualSequenceActive)
		return;

	_sueSpeechTimerAccumulator += delta;
	while (_sueSpeechTimerAccumulator >= kScene6070SueSpeechCheckMillis) {
		_sueSpeechTimerAccumulator -= kScene6070SueSpeechCheckMillis;
		if (_primaryDialogueSpeechActive || _speechOverlay.visible ||
				_actorPathPlaybackActive || _random.getRandomNumber(99) != 0)
			continue;

		byte speechFrame;
		do {
			speechFrame = (byte)_random.getRandomNumber(4);
		} while (speechFrame == _lastSueIdleSpeechFrame);
		_lastSueIdleSpeechFrame = speechFrame;
		_manualSequenceActive = true;
		beginPrimarySpeechLineWithAnimationGroup(9, speechFrame, 0x0e2, 0x08c,
			0x3f, 0x28, 0x32, kScene6070SueIdleSpeechGroup);
		_manualSequenceActive = false;
	}

	const uint ticks = _sueIdleChannel.consumeFrames(delta);
	for (uint tick = 0; tick < ticks; ++tick) {
		if (_sueLayer.frameIndex == 18)
			_sueLayer.setFrame(14);
		else if (!_primaryDialogueSpeechActive && _random.getRandomNumber(14) == 0)
			_sueLayer.setFrame(18);

		if (_pendingRonRetort && !_primaryDialogueSpeechActive &&
			!_speechOverlay.visible && !_actorPathPlaybackActive) {
			_pendingRonRetort = false;
			_manualSequenceActive = true;
			const ScenePoint point = _hotspots.actionTarget(4).approachPoint;
			_activeActorFacing = calculateFacingTowardPoint(_activeActorWorldX,
															_activeActorWorldY, point.x, point.y);
			_activeActorCel = 0;
			beginSecondarySpeechLine(9, 5);
			_manualSequenceActive = false;
			continue;
		}

	}
}

void Scene6070::advanceState609Prop(uint32 delta) {
	if (!_state609PropContinuous)
		return;

	const uint ticks = _state609PropChannel.consumeFrames(delta);
	for (uint tick = 0; tick < ticks; ++tick) {
		const byte frame = _state609PropLayer.frameIndex;
		_state609PropLayer.setFrame(frame == 24 ? 17 : (byte)(frame + 1));
	}
}

void Scene6070::drawNormalForeground(int actorWorldX, int actorWorldY,
									 byte actorDrawOrderMode) {
	uint chunkIndex = 0;
	if (actorDrawOrderMode == 8)
		chunkIndex = 7;
	else if ((actorWorldX < 0x242 && actorWorldY < 0x17a) ||
			 (actorWorldX > 0x241 && actorWorldY < 0x181))
		chunkIndex = 5;

	if (chunkIndex != 0 && _sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex],
							  _sceneFramebuffer);
}

void Scene6070::runArrivalCutscene() {
	drawPlayableComposite();
	if (runCurtainRevealFromBlack())
		return;

	_viewportMaxXOffset = kScene6070ArrivalViewportMaxX;
	_viewportXOffset = MIN<uint16>(_viewportXOffset, _viewportMaxXOffset);
	walkActiveActorTo(0x288, 0x19a, 5, 0);
	waitSceneMillis(2000, false);

	_arrivalLayer.visible = true;
	playAnimationFrames(_arrivalLayer,
		AnimationFrameRange(0, ARRAYSIZE(kScene6070ArrivalFrameMap) - 1,
			kScene6070OverlayFrameMillis));
	runCurtainClearToBlack();
	_vm->gameState().scene6070SuePresent = true;
	_vm->gameState().mainFlowStateId = kScene7100DialogueEntryState;
}

void Scene6070::runState609Cutscene() {
	_manualSequenceActive = true;
	drawPlayableComposite();
	presentFrame();

	playAnimationFrames(_state609PropLayer,
		AnimationFrameRange(4, 8, kScene6070State609SlowFrameMillis));
	_state609PropAlternatePose = true;
	beginPrimarySpeechLineWithAnimationGroup(15, 0, 0x17c, 0x0a8,
											 0x20, 0x32, 0, kScene6070State609PropSpeechGroup);
	playAnimationFrames(_state609PropLayer,
		AnimationFrameRange(12, 16, kScene6070State609SlowFrameMillis));
	_state609PropLayer.setFrame(0);
	_state609PropAlternatePose = false;

	_state609PropLayer.setFrame(17);
	_state609PropChannel.reset(17, kScene6070State609SlowFrameMillis);
	_state609PropContinuous = true;
	playAnimationFrames(_state609NpcLayer,
		AnimationFrameRange(1, 4, kScene6070State609FastFrameMillis));
	beginPrimarySpeechLineWithAnimationGroup(15, 1, 0x1c8, 0x096,
											 0x28, 0x16, 0x0b, kScene6070State609NpcSpeechGroup);
	playAnimationFrames(_state609NpcLayer,
		AnimationFrameRange(9, 12, kScene6070State609FastFrameMillis));
	_state609NpcLayer.setFrame(0);

	beginSecondarySpeechLine(15, 2);
	beginPrimarySpeechLineWithAnimationGroup(15, 3, 0x102, 0x096,
											 0x3f, 0x28, 0x32, kScene6070State609SueSpeechGroup);
	_state609PropContinuous = false;
	_state609PropLayer.setFrame(0);
	beginPrimarySpeechLineWithAnimationGroup(15, 4, 0x186, 0x096,
											 0x20, 0x32, 0, kScene6070State609PropSpeechGroup);

	runCurtainClearToBlack();
	_vm->gameState().mainFlowStateId = kScene6090EntryState;
	_manualSequenceActive = false;
}

void Scene6070::runSueDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializeSueDialogueRecords(records);
	_sueMode = 1;
	_sueLayer.setFrame(14);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(0x62, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(0x62, 9);
			beginPrimarySpeechLineWithAnimationGroup(99, 9, 0x0e2, 0x08c,
													 0x3f, 0x28, 0x32, kScene6070SueSpeechGroup);
			break;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 +
								 selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(0x62, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff) {
			beginPrimarySpeechLineWithAnimationGroup(99, record.responseFrameIndex,
													 0x0e2, 0x08c, 0x3f, 0x28, 0x32, kScene6070SueSpeechGroup);
		}
		if (record.disableAfterUse == 1)
			record.enabled = 0;

		if (record.transitionMode == 0) {
			if (record.responseFrameIndex == 8) {
				runCurtainClearToBlack();
				_vm->gameState().mainFlowStateId = kScene7100EntryState;
			}
			break;
		}
	}

	_sueMode = 0;
	_sueLayer.setFrame(14);
}

void Scene6070::initializeSueDialogueRecords(
	Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(10 * 10 * 7);
	for (uint i = 0; i < ARRAYSIZE(kScene6070SueDialogueSeedRecords); ++i) {
		const Scene6070DialogueSeedRecord &seed = kScene6070SueDialogueSeedRecords[i];
		DialogueChoiceRecord &record = records[seed.index];
		record.enabled = seed.enabled;
		record.nextNodeIndex = seed.nextNodeIndex;
		record.transitionMode = seed.transitionMode;
		record.playerTextRowId = seed.playerTextRowId;
		record.responseFrameIndex = seed.responseFrameIndex;
		record.disableAfterUse = seed.disableAfterUse;
		record.reserved = 0xff;
	}
}

void Scene6070::runSueDiscovery() {
	GameplayState &state = _vm->gameState();
	if (state.scene6070SuePresent || state.scene6070CellDoorOpen) {
		beginSecondarySpeechLine(5, 0);
		return;
	}

	_manualSequenceActive = true;
	beginSecondarySpeechLine(98, 0);
	state.scene6070SuePresent = true;
	applySceneStateToHotspotsAndPatches(0);
	playAnimationFrames(_sueLayer,
		AnimationFrameRange(0, 14, kScene6070DiscoveryFrameMillis));
	_sueMode = 1;
	beginPrimarySpeechLineWithAnimationGroup(99, 0, 0x0e2, 0x08c,
											 0x3f, 0x28, 0x32, kScene6070SueSpeechGroup);
	beginSecondarySpeechLine(98, 1);
	beginPrimarySpeechLineWithAnimationGroup(99, 1, 0x0e2, 0x08c,
											 0x3f, 0x28, 0x32, kScene6070SueSpeechGroup);
	beginSecondarySpeechLine(98, 2);
	beginPrimarySpeechLineWithAnimationGroup(99, 2, 0x0e2, 0x08c,
											 0x3f, 0x28, 0x32, kScene6070SueSpeechGroup);
	beginSecondarySpeechLine(98, 3);
	beginPrimarySpeechLineWithAnimationGroup(99, 3, 0x0e2, 0x08c,
											 0x3f, 0x28, 0x32, kScene6070SueSpeechGroup);
	_sueMode = 0;
	_sueLayer.setFrame(14);
	_manualSequenceActive = false;
}

void Scene6070::handleGiveItemToSue() {
	const byte ronItem = _lastInventoryPrimaryItemId;
	if (ronItem == 0x21) {
		beginSecondarySpeechLine(8, 0);
		return;
	}

	const byte sueItem = sueInventoryItemForRonItem(ronItem);
	if (sueItem == 0) {
		beginSecondarySpeechLine(7, (byte)_random.getRandomNumber(1));
		return;
	}

	beginSecondarySpeechLine(6, 0);
	_manualSequenceActive = true;
	runActorReplacement(ActionOverlaySpec(10, 0x0c, kScene6070TransferFrameMap,
									   ARRAYSIZE(kScene6070TransferFrameMap), kScene6070OverlayFrameMillis)
						 .hookEveryFrame(kScene6070TransferFrameHook));
	_sueLayer.setFrame(14);
	_manualSequenceActive = false;

	GameplayState &state = _vm->gameState();
	state.removeInventoryItem(0, ronItem);
	state.addInventoryItem(1, sueItem);
	_soundBank0.playSample(1, 100);
}

byte Scene6070::sueInventoryItemForRonItem(byte itemId) const {
	switch (itemId) {
	case 0x03:
		return 0x17;
	case 0x10:
		return 0x18;
	case 0x28:
		return 0x19;
	case 0x2d:
		return 0x1a;
	case 0x37:
		return 0x1b;
	case 0x41:
		return 0x1c;
	case 0x48:
		return 0x1d;
	case 0x56:
		return 0x1e;
	case 0x5a:
		return 0x1f;
	case 0x60:
		return 0x20;
	case 0x61:
		return 0x14;
	case 0x63:
		return 0x15;
	case 0x64:
		return 0x16;
	case 0x65:
		return 0x21;
	default:
		return 0;
	}
}

bool Scene6070::runCurtainRevealFromBlack() {
	Graphics::ManagedSurface savedScene;
	savedScene.copyFrom(_sceneFramebuffer);
	byte *destination = framebufferPixels(_sceneFramebuffer);
	const byte *source = framebufferPixels(savedScene);
	if (!destination || !source)
		return false;

	memset(destination, 0, framebufferByteCount());
	presentFrame();
	for (int sweep = 0x0dc; sweep >= 0 && !_vm->isSceneRestartRequested(); sweep -= 0x14) {
		const uint bandWidth = 0x14;
		const uint innerWidth = HollywoodEngine::kScreenWidth - 2 * sweep;
		const uint middleInset = sweep + bandWidth;
		const uint middleHeight = HollywoodEngine::kScreenHeight - 2 * middleInset;
		const uint leftX = _viewportXOffset + sweep;
		const uint rightX = leftX + innerWidth - bandWidth;

		for (uint row = 0; row < bandWidth; ++row) {
			memcpy(destination + (sweep + row) * HollywoodEngine::kSceneBufferWidth + leftX,
				   source + (sweep + row) * HollywoodEngine::kSceneBufferWidth + leftX,
				   innerWidth);
			const uint bottomY = HollywoodEngine::kScreenHeight - bandWidth - sweep + row;
			memcpy(destination + bottomY * HollywoodEngine::kSceneBufferWidth + leftX,
				   source + bottomY * HollywoodEngine::kSceneBufferWidth + leftX,
				   innerWidth);
		}
		for (uint row = 0; row < middleHeight; ++row) {
			const uint y = middleInset + row;
			memcpy(destination + y * HollywoodEngine::kSceneBufferWidth + leftX,
				   source + y * HollywoodEngine::kSceneBufferWidth + leftX, bandWidth);
			memcpy(destination + y * HollywoodEngine::kSceneBufferWidth + rightX,
				   source + y * HollywoodEngine::kSceneBufferWidth + rightX, bandWidth);
		}
		presentFrame();
		if (pollEvents(false))
			return true;
	}

	_sceneFramebuffer.copyRectToSurface(savedScene.rawSurface(), 0, 0,
										Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth,
													 HollywoodEngine::kSceneBufferHeight));
	presentFrame();
	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void Scene6070::runCurtainClearToBlack() {
	byte *pixels = framebufferPixels(_sceneFramebuffer);
	if (!pixels)
		return;

	for (uint sweep = 0; sweep < 0x0f0 && !_vm->isSceneRestartRequested(); sweep += 0x14) {
		const uint bandWidth = 0x14;
		const uint innerWidth = HollywoodEngine::kScreenWidth - 2 * sweep;
		const uint middleInset = sweep + bandWidth;
		const uint middleHeight = HollywoodEngine::kScreenHeight - 2 * middleInset;
		const uint leftX = _viewportXOffset + sweep;
		const uint rightX = leftX + innerWidth - bandWidth;

		for (uint row = 0; row < bandWidth; ++row) {
			memset(pixels + (sweep + row) * HollywoodEngine::kSceneBufferWidth + leftX,
				   0, innerWidth);
			memset(pixels + (HollywoodEngine::kScreenHeight - bandWidth - sweep + row) *
					HollywoodEngine::kSceneBufferWidth + leftX, 0, innerWidth);
		}
		for (uint row = 0; row < middleHeight; ++row) {
			const uint y = middleInset + row;
			memset(pixels + y * HollywoodEngine::kSceneBufferWidth + leftX, 0, bandWidth);
			memset(pixels + y * HollywoodEngine::kSceneBufferWidth + rightX, 0, bandWidth);
		}
		presentFrame();
		if (pollEvents(false))
			break;
	}

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	_displayPalette.markAllDirty();
	presentFrame();
}

void Scene6070::rememberOriginalColorMap() {
	if (!_originalColorToItemMap.empty() ||
		_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	_originalColorToItemMap.resize(kScenePaletteMapPageSize);
	memcpy(_originalColorToItemMap.data(),
		   _paletteMaskOriginal.data() + kSceneColorToItemMap,
		   _originalColorToItemMap.size());
}

void Scene6070::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_originalColorToItemMap.size() < kScenePaletteMapPageSize ||
		_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_originalColorToItemMap[color] == sourceItem)
			_paletteMask[kSceneColorToItemMap + color] = destinationItem;
	}
}

void Scene6070::rebuildSceneWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(),
		   _walkablePaletteMask.size());
	for (uint color = 0; color < _walkablePaletteMask.size(); ++color) {
		if (_walkablePaletteMask[color] > 6)
			_walkablePaletteMask[color] = 0;
	}
}

} // End of namespace Hollywood
