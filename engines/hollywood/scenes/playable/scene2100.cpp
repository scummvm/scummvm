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

#include "hollywood/scenes/playable/scene2100.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene2100ArchiveName = "RESOURCE.B10";
const char *const kScene2100MusicArchiveName = "RESOURCE.M02";
const char *const kScene2100SoundArchiveName = "RESOURCE.S02";
const uint kScene2100InitialRequiredChunkCount = 17;
const uint kScene2100ArenaFirstChunk = 5;
const uint kScene2100ArenaLastChunk = 16;
const uint kScene2100StageIndex = 210;
const uint16 kScene2100FirstState = 0x0834;
const uint16 kScene2100ReturnFromTreasureState = 0x0835;
const uint16 kScene2100LeftPassageState = 0x0836;
const uint16 kScene2100LastState = 0x083d;
const uint16 kScene2110EntryState = 0x083e;
const uint16 kScene2110ScriptedReturnState = 0x083f;
const uint16 kScene2100ViewportXOffset = 0x00c8;
const uint16 kScene2100ViewportMaxXOffset = 0x0108;
const uint kScene2100ActorBankTableEntry = 0x0000;
const uint kScene2100ActorPaletteTableEntry = 0x00cc;
const uint kScene2100Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2100SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2100FrameMillis = 75;
const byte kScene2100InvalidFacing = 0xff;
const byte kScene2100ForegroundChunk = 5;
const byte kScene2100ForegroundDescriptorCount = 0x0d;
const byte kScene2100PickupChunk = 11;
const byte kScene2100PickupDescriptorCount = 0x0e;
const byte kScene2100TransitionChunk = 16;
const byte kScene2100TransitionDescriptorCount = 0x2f;
const byte kScene2100RaStaffItem = 0x2f;
const byte kScene2100MummyDialogueStageId = 0x62;
const byte kScene2100MummyPrimaryRow = 99;
const uint kScene2100MummyDialogueChoiceRecordCount = 10 * 10 * 7;

const byte kScene2100ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene2100ForegroundFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 8,
	9, 10, 11, 12, 7, 6, 5, 0
};

const byte kScene2100ForegroundSpeechFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 8,
	9, 10, 11, 12, 7, 6, 5, 0,
	0, 0, 0, 0, 0, 0, 1, 0,
	3, 2, 3, 1
};

const byte kScene2100PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13
};

const byte kScene2100TransitionFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 11, 8, 9,
	10, 25, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
	44, 45, 46
};

static_assert(ARRAYSIZE(kScene2100ActorPathStepDeltaTable) == 72,
	"Scene 2100 actor path table size changed");
static_assert(ARRAYSIZE(kScene2100ForegroundFrameMap) == 18,
	"Scene 2100 foreground frame map size changed");
static_assert(ARRAYSIZE(kScene2100ForegroundSpeechFrameMap) >= 0x1d,
	"Scene 2100 foreground speech frame map size changed");
static_assert(ARRAYSIZE(kScene2100PickupFrameMap) == 15,
	"Scene 2100 pickup frame map size changed");
static_assert(ARRAYSIZE(kScene2100TransitionFrameMap) == 0x35,
	"Scene 2100 transition frame map size changed");
static_assert(kScene2100TransitionDescriptorCount > 46,
	"Scene 2100 transition descriptor count is too small");

static PlayableSceneConfig scene2100Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene2100ArchiveName;
	config.initialRequiredChunkCount = kScene2100InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene2100ArenaFirstChunk;
	config.arenaLastChunk = kScene2100ArenaLastChunk;
	config.stageIndex = kScene2100StageIndex;
	config.debugName = "Scene 2100";
	config.viewportXOffset = kScene2100ViewportXOffset;
	config.viewportMinXOffset = kScene2100ViewportXOffset;
	config.viewportMaxXOffset = kScene2100ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 2;
	config.actorBankTableEntry = kScene2100ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene2100ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene2100Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene2100SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene2100ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene2100ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 1;
	config.musicArchiveName = kScene2100MusicArchiveName;
	config.soundBank0ArchiveName = kScene2100SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene2100FirstState;
	config.mainFlowLastState = kScene2100LastState;
	return config;
}

Scene2100::Scene2100(HollywoodEngine *vm) :
		PlayableScene(vm, scene2100Config(), "scene2100", 0x277, 0x168, 1, 0xfd, 0xfb),
		_foregroundLayer(),
		_transitionLayer(),
		_foregroundAlternateFrameSet(false) {
	_foregroundLayer.configure(kScene2100ForegroundChunk, kScene2100ForegroundDescriptorCount,
		kScene2100ForegroundFrameMap, ARRAYSIZE(kScene2100ForegroundFrameMap));
	_transitionLayer.configure(kScene2100TransitionChunk, kScene2100TransitionDescriptorCount,
		kScene2100TransitionFrameMap, ARRAYSIZE(kScene2100TransitionFrameMap));
}

bool Scene2100::hasCustomPreviewState() const {
	return true;
}

void Scene2100::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	const GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene2100ReturnFromTreasureState) {
		_activeActorWorldX = 0x1d0;
		_activeActorWorldY = 0x14c;
		_activeActorFacing = 4;
	} else if (state.mainFlowStateId == kScene2100LeftPassageState) {
		_activeActorWorldX = 0x107;
		_activeActorWorldY = 0x120;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x3b0;
		_activeActorWorldY = 0x1ba;
		_activeActorFacing = 4;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene2100::hasCustomComposite() const {
	return true;
}

void Scene2100::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_foregroundLayer);
	drawResourceSpriteLayer(_transitionLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
}

bool Scene2100::hasCustomEntrySequence() const {
	return true;
}

void Scene2100::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene2100ReturnFromTreasureState:
		runEntryFromScene2110();
		break;
	case kScene2100LeftPassageState:
		runEntryFromLeftPassage();
		break;
	default:
		runEntryFromScene2010();
		break;
	}
}

bool Scene2100::advanceCustomGameplayLoop(uint32 delta) {
	(void)delta;
	return false;
}

bool Scene2100::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();

	switch (handlerId) {
	case 301: // Mirar/usar puerta de piedra (look/use stone door): state-aware door response.
		beginSecondarySpeechLine(1, state.scene2100RaStaffTaken ? 0 : 1);
		return true;
	case 302: // Usar/abrir puerta de piedra (use/open stone door): no visible response.
		return true;
	case 303: // Mirar estantería (look at bookshelf): Egypt books and one odd novel.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar sillón (look at armchair): comfortable armchair.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Usar sillón (use armchair): Ron refuses to rest now.
		beginSecondarySpeechLine(4, 0);
		beginSecondarySpeechLine(4, 1);
		return true;
	case 306: // Usar bastón de Rá en puerta de piedra (use Ra staff on stone door): opens treasure room.
		runStoneDoorToTreasureRoom();
		return true;
	case 307: // Coger bastón de Rá (take Ra staff): grants the Ra staff.
		runRaStaffPickup();
		return true;
	case 308: // Hablar con la Momia (talk to the Mummy): dialogue tree.
		runMummyDialogue();
		return true;
	default:
		return false;
	}
}

bool Scene2100::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = MAX<int>(targetX, 0x118);

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene2100::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if ((selector == 1 || selector == 0xff) && state.scene2100RaStaffTaken) {
		if (_sceneChunkTable.isValidChunk(9))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
		removeColorMapItemFromOriginal(5);
	} else if ((selector == 1 || selector == 0xff) && _sceneChunkTable.isValidChunk(10)) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
	}

	if (selector == 4 || selector == 0xff) {
		const uint16 threshold = state.scene2100PassageOpen ? 0x145 : 10;
		if (_actorDepthYThresholds.size() > 2)
			_actorDepthYThresholds[2] = threshold;
		if (_drawActorDepthYThresholds.size() > 2)
			_drawActorDepthYThresholds[2] = threshold;

		const byte patchChunk = state.scene2100PassageOpen ? 13 : 8;
		if (_sceneChunkTable.isValidChunk(patchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[patchChunk], _baseFramebuffer);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

byte Scene2100::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return _foregroundAlternateFrameSet ? 9 : 0;
}

void Scene2100::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_foregroundLayer.visible = _vm->gameState().scene2100MummyBranchState != 1;
	_foregroundLayer.frameMap = kScene2100ForegroundSpeechFrameMap;
	_foregroundLayer.frameMapSize = ARRAYSIZE(kScene2100ForegroundSpeechFrameMap);
	_foregroundLayer.setFrame(frameIndex);
}

void Scene2100::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	setPrimarySpeechAnimationFrame(0, baseFrame);
	_foregroundLayer.frameMap = kScene2100ForegroundFrameMap;
	_foregroundLayer.frameMapSize = ARRAYSIZE(kScene2100ForegroundFrameMap);
}

AmbientAudioProfile Scene2100::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2100::resetAnimationLayers() {
	_foregroundLayer.visible = _vm->gameState().scene2100MummyBranchState != 1;
	_foregroundLayer.frameMap = kScene2100ForegroundFrameMap;
	_foregroundLayer.frameMapSize = ARRAYSIZE(kScene2100ForegroundFrameMap);
	_foregroundLayer.reset(0);
	_transitionLayer.visible = false;
	_transitionLayer.reset(0);
	_foregroundAlternateFrameSet = false;
}

void Scene2100::setForegroundFrame(byte frameIndex) {
	_foregroundLayer.visible = _vm->gameState().scene2100MummyBranchState != 1;
	_foregroundLayer.setFrame(frameIndex);
}

void Scene2100::runEntryFromScene2010() {
	runEntryPathWithFinalFacing(0x3b0, 0x1ba, 4, 0x277, 0x168, 1, 0);
	if (_vm->gameState().scene2100MummyBranchState == 0)
		runMummyDialogue();
}

void Scene2100::runEntryFromScene2110() {
	GameplayState &state = _vm->gameState();
	state.scene2100PassageOpen = false;
	applySceneStateToHotspotsAndPatches(4);
	runEntryPathWithFinalFacing(0x1d0, 0x14c, 4, 0x277, 0x168, 1, 0);
}

void Scene2100::runEntryFromLeftPassage() {
	GameplayState &state = _vm->gameState();
	state.scene2100PassageOpen = false;
	applySceneStateToHotspotsAndPatches(4);
	runEntryPathWithFinalFacing(0x107, 0x120, 4, 0x1f0, 0x17c, kScene2100InvalidFacing, 0);
}

void Scene2100::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel) {
	_activeActorWorldX = startX;
	_activeActorWorldY = startY;
	_activeActorFacing = startFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);

	drawPlayableComposite();
	presentFrame();
	walkActiveActorTo(targetX, targetY, finalFacing, finalCel, false);
	if (finalFacing != kScene2100InvalidFacing)
		_activeActorFacing = finalFacing;
	_activeActorCel = finalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene2100::runMummyDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializeMummyDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	if (!state.scene2100MummyGreetingSeen) {
		runMummyPrimarySpeechLine(0);
		beginMummyDialogueSecondarySpeechLine(kScene2100MummyDialogueStageId, 0);
		runMummyPrimarySpeechLine(1);
		state.scene2100MummyGreetingSeen = true;
	} else {
		beginMummyDialogueSecondarySpeechLine(kScene2100MummyDialogueStageId, 1);
		runMummyPrimarySpeechLine(2);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene2100MummyDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginMummyDialogueSecondarySpeechLine(kScene2100MummyDialogueStageId, 8);
			runMummyPrimarySpeechLine(9);
			return;
		}

		const uint recordIndex = ((uint)nodeIndex + (uint)depthIndex * 10) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			return;

		DialogueChoiceRecord &record = records[recordIndex];
		beginMummyDialogueSecondarySpeechLine(kScene2100MummyDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff)
			runMummyPrimarySpeechLine(record.responseFrameIndex);

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}
		if (record.disableAfterUse == 2 && !state.scene2100AfterlifeBranchUnlocked) {
			if (records.size() > 86) {
				records[86].enabled = 1;
				records[86].selectable = 1;
			}
			state.scene2100AfterlifeBranchUnlocked = true;
		} else if (record.disableAfterUse == 3 && state.scene2100MummyDialogueClueStage == 0) {
			if (records.size() > 3) {
				records[3].enabled = 1;
				records[3].selectable = 1;
			}
			state.scene2100MummyDialogueClueStage = 1;
			state.scene2100MarketBranchUnlocked = true;
		} else if (record.disableAfterUse == 4 && state.scene2100MummyDialogueClueStage == 1) {
			state.scene2100MummyDialogueClueStage = 2;
		} else if (record.disableAfterUse == 5) {
			runMummySpecialTransitionToScene2110();
			return;
		}

		switch (record.transitionMode) {
		case 0:
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
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = depthIndex > 1 ? (byte)(depthIndex - 2) : 0;
			break;
		case 5:
			nodeIndex = record.nextNodeIndex;
			depthIndex = depthIndex > 3 ? (byte)(depthIndex - 4) : 0;
			break;
		default:
			break;
		}
	}
}

void Scene2100::initializeMummyDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	const GameplayState &state = _vm->gameState();
	records.resize(kScene2100MummyDialogueChoiceRecordCount);

	setDialogueRecord(records, 0, 1, 0, 3, 2, 3, 1, 0xff);
	setDialogueRecord(records, 1, state.scene2100MarketBranchUnlocked ? 1 : 0, 2, 1, 3, 4, 1, 0xff);
	setDialogueRecord(records, 2, 1, 1, 1, 4, 5, 1, 0xff);
	setDialogueRecord(records, 3, state.scene2100MummyDialogueClueStage != 0 ? 1 : 0, 3, 1, 5, 6, 4, 0xff);
	setDialogueRecord(records, 4, 1, 0, 0, 7, 8, 5, 0xff);
	setDialogueRecord(records, 5, 1, 0, 0, 8, 9, 0, 0xff);
	setDialogueRecord(records, 70, 1, 0, 2, 9, 10, 1, 0xff);
	setDialogueRecord(records, 77, 1, 0, 2, 10, 11, 1, 0xff);
	setDialogueRecord(records, 140, 1, 2, 3, 11, 12, 2, 0xff);
	setDialogueRecord(records, 141, 1, 2, 3, 12, 13, 1, 0xff);
	setDialogueRecord(records, 142, state.scene2100AfterlifeBranchUnlocked ? 1 : 0, 0, 1, 13, 14, 3, 0xff);
	setDialogueRecord(records, 143, 1, 0, 2, 15, 16, 1, 0xff);
	setDialogueRecord(records, 210, 1, 0, 0, 17, 18, 1, 0xff);
}

void Scene2100::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
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
	record.selectable = enabled != 0;
}

void Scene2100::runMummyPrimarySpeechLine(byte frameIndex) {
	_foregroundLayer.frameMap = kScene2100ForegroundSpeechFrameMap;
	_foregroundLayer.frameMapSize = ARRAYSIZE(kScene2100ForegroundSpeechFrameMap);
	beginPrimarySpeechLineWithAnimationGroup(kScene2100MummyPrimaryRow, frameIndex,
		0x2e7, 0x83, 0x32, 0x32, 0x3f, 0);
	_foregroundLayer.frameMap = kScene2100ForegroundFrameMap;
	_foregroundLayer.frameMapSize = ARRAYSIZE(kScene2100ForegroundFrameMap);
}

void Scene2100::beginMummyDialogueSecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	beginSecondarySpeechLine(rowIndex, frameIndex);
}

void Scene2100::runMummySpecialTransitionToScene2110() {
	GameplayState &state = _vm->gameState();
	state.scene2100MummyBranchState = 1;
	state.scene2100PassageOpen = true;
	state.mainFlowStateId = kScene2110ScriptedReturnState;
}

void Scene2100::runStoneDoorToTreasureRoom() {
	if (!hasInventoryItem(kScene2100RaStaffItem)) {
		beginSecondarySpeechLine(1, 1);
		return;
	}

	walkActiveActorTo(0x24a, 0x163, 4, 0, false);
	_transitionLayer.visible = true;
	_transitionLayer.reset(0);

	for (byte frame = 1; frame < 4; ++frame) {
		_transitionLayer.setFrame(frame);
		if (waitSceneMillis(kScene2100FrameMillis))
			break;
	}

	for (byte speechFrame = 0; speechFrame < 4 && !_vm->isSceneRestartRequested(); ++speechFrame) {
		beginSecondarySpeechLine(5, speechFrame);
		const byte targetFrame = (byte)(8 + speechFrame * 5);
		runMappedLayerAnimation(_transitionLayer, targetFrame, kScene2100FrameMillis);
	}

	runMappedLayerAnimation(_transitionLayer, 0x34, kScene2100FrameMillis, -1, 0, 0, 0x1e, 0x10, 0x32);
	_transitionLayer.visible = false;

	GameplayState &state = _vm->gameState();
	state.scene2100PassageOpen = true;
	state.mainFlowStateId = kScene2110EntryState;
}

void Scene2100::runRaStaffPickup() {
	if (_vm->gameState().scene2100RaStaffTaken) {
		dispatchGenericSceneAction(6);
		return;
	}

	GameplayState &state = _vm->gameState();
	state.scene2100RaStaffTaken = true;

	ActionOverlayOptions options;
	options.actorVisibility = kActionOverlayHideActiveActor;
	options.statePatchFrame = 5;
	options.statePatchSelector = 1;
	runActionOverlay(kScene2100PickupChunk, kScene2100PickupDescriptorCount,
		kScene2100PickupFrameMap, ARRAYSIZE(kScene2100PickupFrameMap),
		kScene2100FrameMillis, options);

	applySceneStateToHotspotsAndPatches(1);
	if (!hasInventoryItem(kScene2100RaStaffItem))
		addInventoryItem(kScene2100RaStaffItem);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene2100::runMappedLayerAnimation(ResourceSpriteLayer &layer, byte targetFrame, uint32 frameMillis,
		int speechTriggerFrame, uint16 speechRow, byte speechFrame,
		int soundStartFrame, byte soundStartId, int soundStopFrame) {
	layer.visible = true;
	while (layer.frameIndex < targetFrame && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested()) {
		const byte nextFrame = (byte)(layer.frameIndex + 1);
		if ((int)nextFrame == speechTriggerFrame)
			startSecondarySpeechLine(speechRow, speechFrame);
		if ((int)nextFrame == soundStartFrame)
			_soundBank0.playSample(soundStartId, 100);
		if ((int)nextFrame == soundStopFrame)
			_soundBank0.stop();
		layer.setFrame(nextFrame);
		if (waitSceneMillis(frameMillis))
			break;
	}
}

void Scene2100::removeColorMapItemFromOriginal(byte itemId) {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
