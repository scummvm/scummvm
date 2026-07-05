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

#include "hollywood/scenes/playable/scene2020.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene2020ArchiveName = "RESOURCE.B02";
const char *const kScene2020MusicArchiveName = "RESOURCE.M02";
const char *const kScene2020SoundArchiveName = "RESOURCE.S02";
const uint kScene2020InitialRequiredChunkCount = 19;
const uint kScene2020ArenaFirstChunk = 5;
const uint kScene2020ArenaLastChunk = 18;
const uint kScene2020StageIndex = 202;
const uint16 kScene2020FirstState = 0x07e4;
const uint16 kScene2020LastState = 0x07ed;
const uint16 kScene2020PrincessExitState = 0x07e5;
const uint16 kScene2010ReturnFromScene2020State = 0x07db;
const uint16 kScene2020PrincessExitNextState = 0x082b;
const uint kScene2020ActorBankTableEntry = 0x0000;
const uint kScene2020ActorPaletteTableEntry = 0x00cc;
const uint kScene2020Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2020SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2020PoolFrameMillis = 150;
const uint32 kScene2020TigerFrameMillis = 75;
const uint32 kScene2020PrincessFrameMillis = 75;
const uint32 kScene2020OverlayFrameMillis = 75;
const byte kScene2020PrincessDialogueStageId = 0x62;
const byte kScene2020PrincessDialoguePrimaryRow = 99;
const uint kScene2020PrincessDialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene2020DialogueNoResponseFrame = 0xff;
const byte kScene2020PoolDescriptorCount = 3;
const byte kScene2020TigerDescriptorCount = 0x10;
const byte kScene2020PrincessDescriptorCount = 0x21;
const byte kScene2020PickupDescriptorCount = 9;
const byte kScene2020TigerToothPickupDescriptorCount = 0x0d;
const byte kScene2020TigerItemOverlayDescriptorCount = 10;
const byte kScene2020HatInventoryItem = 0x28;
const byte kScene2020SunglassesInventoryItem = 0x27;
const byte kScene2020TigerToothInventoryItem = 0x26;
const byte kScene2020SteakInventoryItem = 0x45;
const byte kScene2020LabInventoryItem = 0x11;

enum Scene2020OverlayHook {
	kScene2020HatPickupPatchHook = 1,
	kScene2020SunglassesPickupPatchHook = 2,
	kScene2020TigerToothPickupPatchHook = 3
};

const byte kScene2020ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene2020PoolFrameMap[] = {
	0, 1, 0, 2
};

const byte kScene2020TigerFrameMap[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	3, 4, 5, 0, 6, 7, 8, 8, 7, 6, 0
};

const byte kScene2020PrincessFrameMap[] = {
	0, 1, 2, 1, 0, 3, 4, 3, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	0, 0, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 0, 0, 0, 0, 0, 0
};

const byte kScene2020PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8
};

const byte kScene2020TigerToothPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene2020TigerItemOverlayFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};

static_assert(ARRAYSIZE(kScene2020PoolFrameMap) == 4, "Scene 2020 pool frame map size changed");
static_assert(ARRAYSIZE(kScene2020TigerFrameMap) == 23, "Scene 2020 tiger frame map size changed");
static_assert(ARRAYSIZE(kScene2020PrincessFrameMap) == 56, "Scene 2020 princess frame map size changed");
static_assert(ARRAYSIZE(kScene2020PickupFrameMap) == 10, "Scene 2020 pickup frame map size changed");
static_assert(ARRAYSIZE(kScene2020TigerToothPickupFrameMap) == 14, "Scene 2020 tooth pickup frame map size changed");
static_assert(ARRAYSIZE(kScene2020TigerItemOverlayFrameMap) == 11, "Scene 2020 tiger item overlay frame map size changed");

static PlayableSceneConfig scene2020Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene2020ArchiveName;
	config.initialRequiredChunkCount = kScene2020InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene2020ArenaFirstChunk;
	config.arenaLastChunk = kScene2020ArenaLastChunk;
	config.stageIndex = kScene2020StageIndex;
	config.debugName = "Scene 2020";
	config.viewportXOffset = 0;
	config.viewportMinXOffset = 0;
	config.viewportMaxXOffset = 0;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 2;
	config.actorBankTableEntry = kScene2020ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene2020ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene2020Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene2020SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene2020ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene2020ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 6;
	config.musicArchiveName = kScene2020MusicArchiveName;
	config.soundBank0ArchiveName = kScene2020SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene2020FirstState;
	config.mainFlowLastState = kScene2020LastState;
	return config;
}

Scene2020::Scene2020(HollywoodEngine *vm) :
		PlayableScene(vm, scene2020Config(), "scene2020", 0x320, 0x1b1, 4, 0xfd, 0xfb),
		_poolChannel(),
		_tigerChannel(),
		_princessChannel(),
		_poolLayer(),
		_tigerLayer(),
		_princessLayer(),
		_tigerAnimationState(0),
		_princessAnimationState(0),
		_princessSpeechTransitionActive(false) {
	_poolLayer.configure(5, kScene2020PoolDescriptorCount,
		kScene2020PoolFrameMap, ARRAYSIZE(kScene2020PoolFrameMap));
	_tigerLayer.configure(6, kScene2020TigerDescriptorCount,
		kScene2020TigerFrameMap, ARRAYSIZE(kScene2020TigerFrameMap));
	_princessLayer.configure(11, kScene2020PrincessDescriptorCount,
		kScene2020PrincessFrameMap, ARRAYSIZE(kScene2020PrincessFrameMap));
}

bool Scene2020::hasCustomPreviewState() const {
	return true;
}

void Scene2020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	_activeActorWorldX = 0x190;
	_activeActorWorldY = 0x1b1;
	_activeActorFacing = 4;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene2020::hasCustomComposite() const {
	return true;
}

void Scene2020::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_poolLayer);
	drawResourceSpriteLayer(_tigerLayer);
	drawResourceSpriteLayer(_princessLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
}

bool Scene2020::hasCustomEntrySequence() const {
	return true;
}

void Scene2020::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene2020PrincessExitState) {
		runPrincessExitCutscene();
		return;
	}

	runEntryFromExterior();
}

bool Scene2020::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene2020::advanceCustomGameplayLoop(uint32 delta) {
	advancePoolLayer(delta);
	advanceTigerLayer(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advancePrincessIdleLayer(delta, true);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2020::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Hablar con princesa Amesis-Huni (talk to Princess Amesis-Huni): run her dialogue tree.
		runPrincessDialogue();
		return true;
	case 302: // Mirar princesa Amesis-Huni (look at Princess Amesis-Huni).
		beginSecondarySpeechLine(1, 1);
		return true;
	case 303: // Coger pamela (take hat): only possible after the princess leaves it behind.
		runHatPickup();
		return true;
	case 304: // Mirar pamela (look at hat): state-aware line while worn/left behind.
		beginSecondarySpeechLine(2, _vm->gameState().scene2020PrincessGone ? 1 : 0);
		return true;
	case 305: // Coger diente de tigre (take tiger tooth): pick the tooth after it is exposed.
		runTigerToothPickup();
		return true;
	case 306: // Mirar diente de tigre (look at tiger tooth).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Mirar tigre (look at tiger): changes after the tooth puzzle starts.
		beginSecondarySpeechLine(5, _vm->gameState().scene2020TigerToothState == 0 ? 0 : 1);
		return true;
	case 308: // Mirar/usar piscina (look/use pool).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Ir a entrada de la pirámide (go to pyramid entrance): return to scene 2010.
		_vm->gameState().mainFlowStateId = kScene2010ReturnFromScene2020State;
		return true;
	case 310: // Mirar tienda (look at tent).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 311: // Coger gafas de sol (take sunglasses): only possible after the princess leaves them behind.
		runSunglassesPickup();
		return true;
	case 312: // Mirar gafas de sol (look at sunglasses): changes after the tooth puzzle starts.
		beginSecondarySpeechLine(9, _vm->gameState().scene2020TigerToothState == 0 ? 0 : 1);
		return true;
	case 313: // Usar carne/filete con tigre (use steak with tiger): distracts the tiger and consumes the steak.
		runSteakOnTigerSequence();
		return true;
	case 314: // Usar filete "durillo" con tigre (use tough steak with tiger): exposes the tiger tooth.
		runLabItemOnTigerSequence();
		return true;
	default:
		return false;
	}
}

bool Scene2020::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if (selector == 1 || selector == 0xff) {
		if (!state.scene2020PrincessGone) {
			replaceColorMapItem(10, 1);
		} else {
			replaceColorMapItem(1, 0);
			uint patchChunk = 10;
			if (state.scene2020HatPresent && state.scene2020SunglassesPresent)
				patchChunk = 7;
			else if (state.scene2020HatPresent)
				patchChunk = 8;
			else if (state.scene2020SunglassesPresent)
				patchChunk = 9;
			if (_sceneChunkTable.isValidChunk(patchChunk))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[patchChunk], _baseFramebuffer);
		}
	}

	if (selector == 2 || selector == 0xff) {
		if (state.scene2020TigerToothState == 1) {
			restoreOriginalColorMapItem(3);
			if (_sceneChunkTable.isValidChunk(16))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[16], _baseFramebuffer);
		} else {
			replaceColorMapItem(3, 0);
			if (_sceneChunkTable.isValidChunk(17))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[17], _baseFramebuffer);
		}
	}

	if (selector == 3 || selector == 0xff) {
		if (!state.scene2020HatPresent) {
			replaceColorMapItem(2, 0);
			replaceColorMapItem(9, 0);
			replaceColorMapItem(10, 0);
		} else if (!state.scene2020PrincessGone) {
			replaceColorMapItem(9, 0);
			replaceColorMapItem(10, 0);
		} else {
			replaceColorMapItem(2, 0);
			remapOriginalColorMapItems(9, 10, 2);
			if (!state.scene2020SunglassesPresent) {
				for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
					if (originalColorMapItemAt(i) == 12)
						_paletteMask[kSceneColorToItemMap + i] = 2;
				}
			}
		}
	}

	if (selector == 4 || selector == 0xff) {
		if (!state.scene2020SunglassesPresent) {
			replaceColorMapItem(8, 0);
			replaceColorMapItem(11, 0);
			replaceColorMapItem(12, 0);
		} else if (!state.scene2020PrincessGone) {
			replaceColorMapItem(11, 0);
			replaceColorMapItem(12, 0);
		} else {
			replaceColorMapItem(8, 0);
			remapOriginalColorMapItems(11, 12, 8);
		}
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	if (state.scene2020PrincessGone && state.scene2020HatPresent) {
		ScenePoint interactionPoint;
		ScenePoint approachPoint;
		interactionPoint.x = 0x00ba;
		interactionPoint.y = 0x01a8;
		approachPoint.x = 0x00a2;
		approachPoint.y = 0x0180;
		_hotspots.setActionTarget(2, interactionPoint, approachPoint);
	}
	if (state.scene2020PrincessGone && state.scene2020SunglassesPresent) {
		ScenePoint interactionPoint;
		ScenePoint approachPoint;
		interactionPoint.x = 0x00ba;
		interactionPoint.y = 0x01a8;
		approachPoint.x = 0x00a2;
		approachPoint.y = 0x0180;
		_hotspots.setActionTarget(8, interactionPoint, approachPoint);
	}
	return true;
}

byte Scene2020::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return 0x20;
}

void Scene2020::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	if (!_vm->gameState().scene2020PrincessGone)
		_princessLayer.setFrame(frameIndex);
}

void Scene2020::handleActionOverlayFrameHook(byte hookId, uint frame) {
	(void)frame;
	drawPickupPatch(hookId);
}

AmbientAudioProfile Scene2020::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x16, 5, 10, 1, 0x0b, 3, 100, 50);
}

void Scene2020::resetAnimationLayers() {
	_poolChannel.reset(0, kScene2020PoolFrameMillis);
	_tigerChannel.reset(0, kScene2020TigerFrameMillis);
	_princessChannel.reset(0, kScene2020PrincessFrameMillis);
	_poolLayer.visible = true;
	_tigerLayer.visible = true;
	_princessLayer.visible = !_vm->gameState().scene2020PrincessGone;
	_poolLayer.reset(0);
	_tigerLayer.reset(0);
	_princessLayer.reset(0);
	_tigerAnimationState = 0;
	_princessAnimationState = 0;
	_princessSpeechTransitionActive = false;
}

void Scene2020::advancePoolLayer(uint32 delta) {
	const uint frameCount = _poolChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		_poolChannel.frameIndex = _poolChannel.frameIndex < 3 ? _poolChannel.frameIndex + 1 : 0;
		_poolLayer.setFrame(_poolChannel.frameIndex);
	}
}

void Scene2020::advanceTigerLayer(uint32 delta) {
	const uint frameCount = _tigerChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_tigerAnimationState == 0) {
			if (_tigerLayer.frameIndex < 0x0e) {
				_tigerLayer.setFrame(_tigerLayer.frameIndex + 1);
			} else if (_random.getRandomNumber(0x1d) == 0) {
				_tigerAnimationState = 1;
				_tigerLayer.setFrame(_tigerLayer.frameIndex + 1);
			} else {
				_tigerLayer.setFrame(0);
			}
		} else if (_tigerAnimationState == 1) {
			if (_tigerLayer.frameIndex < 0x12) {
				_tigerLayer.setFrame(_tigerLayer.frameIndex + 1);
			} else if (_random.getRandomNumber(0x31) == 0) {
				_tigerAnimationState = 2;
				_tigerLayer.setFrame(_tigerLayer.frameIndex + 1);
			}
		} else if (_tigerLayer.frameIndex < 0x16) {
			_tigerLayer.setFrame(_tigerLayer.frameIndex + 1);
		} else {
			_tigerAnimationState = 0;
			_tigerLayer.setFrame(0);
		}
	}
}

void Scene2020::advancePrincessIdleLayer(uint32 delta, bool canStartLongSequence) {
	if (_vm->gameState().scene2020PrincessGone || _princessSpeechTransitionActive)
		return;

	const uint frameCount = _princessChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_princessAnimationState == 0) {
			if (_princessLayer.frameIndex < 7) {
				_princessLayer.setFrame(_princessLayer.frameIndex + 1);
			} else if (canStartLongSequence && _random.getRandomNumber(0x13) == 0) {
				_princessAnimationState = 1;
				_princessLayer.setFrame(_princessLayer.frameIndex + 1);
			} else {
				_princessLayer.setFrame(0);
			}
		} else if (_princessLayer.frameIndex < 0x18) {
			_princessLayer.setFrame(_princessLayer.frameIndex + 1);
		} else {
			_princessAnimationState = 0;
			_princessLayer.setFrame(0);
		}
	}
}

void Scene2020::runEntryFromExterior() {
	runEntryPath(0x320, 0x1b1, 4, 0x190, 0x1b1);

	GameplayState &state = _vm->gameState();
	if (!state.scene2020EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene2020EntryLineSeen = true;
	}
}

void Scene2020::runPrincessExitCutscene() {
	resetAnimationLayers();
	_activeActorWorldX = 0x320;
	_activeActorWorldY = 0x1b1;
	_activeActorFacing = 4;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);

	drawPlayableComposite();
	presentFrame();
	runPrincessFrameSequence(0, 0x19, 1);
	runPrincessFrameSequence(0, 0x19, 1);

	GameplayState &state = _vm->gameState();
	state.scene2020PrincessGone = true;
	_princessLayer.visible = false;
	_soundBank0.playSample(0x10, 100);
	applySceneStateToHotspotsAndPatches(0xff);
	drawPlayableComposite();
	presentFrame();
	waitSceneMillis(500);
	state.mainFlowStateId = kScene2020PrincessExitNextState;
}

void Scene2020::runPrincessFrameSequence(byte firstFrame, byte lastFrame, int step) {
	_princessSpeechTransitionActive = true;
	for (int frame = firstFrame; !Engine::shouldQuit() && !_vm->isSceneRestartRequested();) {
		_princessLayer.setFrame((byte)frame);
		drawPlayableComposite();
		presentFrame();
		if (waitSceneMillis(kScene2020PrincessFrameMillis))
			break;
		if (frame == lastFrame)
			break;
		frame += step;
	}
	_princessSpeechTransitionActive = false;
}

void Scene2020::runPrincessSpeechTransition(bool opening) {
	if (_vm->gameState().scene2020PrincessGone)
		return;

	_princessSpeechTransitionActive = true;
	if (opening) {
		for (byte frame = 0x19; frame <= 0x1f && !Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++frame) {
			_princessLayer.setFrame(frame);
			if (waitSceneMillis(kScene2020PrincessFrameMillis))
				break;
		}
	} else {
		for (byte frame = 0x20; frame >= 0x19 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested(); --frame) {
			_princessLayer.setFrame(frame);
			if (waitSceneMillis(kScene2020PrincessFrameMillis))
				break;
			if (frame == 0x19)
				break;
		}
	}
	_princessSpeechTransitionActive = false;
}

void Scene2020::beginPrincessSpeechLine(byte frameIndex) {
	runPrincessSpeechTransition(true);
	beginPrimarySpeechLineWithAnimationGroup(kScene2020PrincessDialoguePrimaryRow, frameIndex,
		0x00ab, 0x00e5, 0x3f, 0x20, 0x34, 0);
	runPrincessSpeechTransition(false);
}

void Scene2020::runPrincessDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializePrincessDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	if (!state.scene2020PrincessConversationSeen) {
		for (byte frame = 0; frame < 3; ++frame) {
			beginSecondarySpeechLine(kScene2020PrincessDialogueStageId, frame);
			beginPrincessSpeechLine(frame);
		}
		state.scene2020PrincessConversationSeen = true;
	} else {
		beginSecondarySpeechLine(kScene2020PrincessDialogueStageId, 3);
		beginPrincessSpeechLine(3);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene2020PrincessDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene2020PrincessDialogueStageId, 7);
			beginPrincessSpeechLine(7);
			return;
		}

		const uint recordIndex = ((uint)nodeIndex + (uint)depthIndex * 10) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			return;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene2020PrincessDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene2020DialogueNoResponseFrame)
			beginPrincessSpeechLine(record.responseFrameIndex);

		if (record.disableAfterUse == 1)
			record.enabled = 0;

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
		case 3:
			break;
		case 4:
			nodeIndex = record.nextNodeIndex;
			depthIndex = depthIndex > 1 ? (byte)(depthIndex - 2) : 0;
			break;
		default:
			break;
		}
	}
}

void Scene2020::initializePrincessDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.resize(kScene2020PrincessDialogueChoiceRecordCount);
	setDialogueRecord(records, 0, 1, 0, 1, 4, 4, 1, 0xff);
	setDialogueRecord(records, 1, 1, 0, 3, 5, 5, 1, 0xff);
	setDialogueRecord(records, 2, 1, 0, 3, 6, 6, 1, 0xff);
	setDialogueRecord(records, 3, 1, 0, 0, 7, 7, 0, 0xff);
	setDialogueRecord(records, 70, 1, 0, 1, 8, 8, 1, 0xff);
	setDialogueRecord(records, 71, 1, 0, 3, 9, 9, 1, 0xff);
	setDialogueRecord(records, 72, 1, 0, 3, 10, 10, 1, 0xff);
	setDialogueRecord(records, 73, 1, 0, 2, 11, 11, 0, 0xff);
	setDialogueRecord(records, 140, 1, 0, 3, 12, 12, 1, 0xff);
	setDialogueRecord(records, 141, 1, 0, 3, 13, 13, 1, 0xff);
	setDialogueRecord(records, 142, 1, 0, 2, 11, 11, 0, 0xff);
}

void Scene2020::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
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

void Scene2020::runHatPickup() {
	GameplayState &state = _vm->gameState();
	if (!state.scene2020PrincessGone) {
		beginSecondarySpeechLine(3, 0);
		return;
	}
	if (!state.scene2020HatPresent) {
		dispatchGenericSceneAction(6);
		return;
	}

	runActionOverlay(ActionOverlaySpec(12, kScene2020PickupDescriptorCount,
		kScene2020PickupFrameMap, ARRAYSIZE(kScene2020PickupFrameMap), kScene2020OverlayFrameMillis)
		.hideActor()
		.hookAt(3, kScene2020HatPickupPatchHook));
	state.scene2020HatPresent = false;
	applySceneStateToHotspotsAndPatches(0xff);
	addInventoryItem(kScene2020HatInventoryItem);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene2020::runSunglassesPickup() {
	GameplayState &state = _vm->gameState();
	if (!state.scene2020PrincessGone) {
		beginSecondarySpeechLine(8, 0);
		return;
	}
	if (!state.scene2020SunglassesPresent) {
		dispatchGenericSceneAction(6);
		return;
	}

	runActionOverlay(ActionOverlaySpec(12, kScene2020PickupDescriptorCount,
		kScene2020PickupFrameMap, ARRAYSIZE(kScene2020PickupFrameMap), kScene2020OverlayFrameMillis)
		.hideActor()
		.hookAt(3, kScene2020SunglassesPickupPatchHook));
	state.scene2020SunglassesPresent = false;
	applySceneStateToHotspotsAndPatches(0xff);
	addInventoryItem(kScene2020SunglassesInventoryItem);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene2020::runTigerToothPickup() {
	if (_vm->gameState().scene2020TigerToothState != 1) {
		beginSecondarySpeechLine(4, 0);
		return;
	}

	runActionOverlay(ActionOverlaySpec(18, kScene2020TigerToothPickupDescriptorCount,
		kScene2020TigerToothPickupFrameMap, ARRAYSIZE(kScene2020TigerToothPickupFrameMap), kScene2020OverlayFrameMillis)
		.hideActor()
		.hookAt(7, kScene2020TigerToothPickupPatchHook));
	_vm->gameState().scene2020TigerToothState = 2;
	applySceneStateToHotspotsAndPatches(2);
	addInventoryItem(kScene2020TigerToothInventoryItem);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene2020::runSteakOnTigerSequence() {
	beginSecondarySpeechLine(10, 0);
	runTigerItemOverlaySequence();
	removeInventoryItem(kScene2020SteakInventoryItem);
	_soundBank0.playSample(1, 100);
}

void Scene2020::runLabItemOnTigerSequence() {
	beginSecondarySpeechLine(11, 0);
	runTigerItemOverlaySequence();
	_vm->gameState().scene2020TigerToothState = 1;
	applySceneStateToHotspotsAndPatches(2);
	removeInventoryItem(kScene2020LabInventoryItem);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(11, 1);
}

void Scene2020::runTigerItemOverlaySequence() {
	runActionOverlay(ActionOverlaySpec(14, kScene2020TigerItemOverlayDescriptorCount,
		kScene2020TigerItemOverlayFrameMap, ARRAYSIZE(kScene2020TigerItemOverlayFrameMap), kScene2020OverlayFrameMillis)
		.hideActor()
		.soundAt(8, 0x2a));
	_tigerAnimationState = 0;
	_tigerLayer.setFrame(0);
}

void Scene2020::drawPickupPatch(byte hookId) {
	uint patchChunk = 0;
	const GameplayState &state = _vm->gameState();
	switch (hookId) {
	case kScene2020HatPickupPatchHook:
		patchChunk = state.scene2020SunglassesPresent ? 9 : 10;
		break;
	case kScene2020SunglassesPickupPatchHook:
		patchChunk = state.scene2020HatPresent ? 8 : 10;
		break;
	case kScene2020TigerToothPickupPatchHook:
		patchChunk = 17;
		break;
	default:
		return;
	}

	if (_sceneChunkTable.isValidChunk(patchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[patchChunk], _baseFramebuffer);
}

void Scene2020::replaceColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene2020::restoreOriginalColorMapItem(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (originalColorMapItemAt(i) == itemId)
			_paletteMask[kSceneColorToItemMap + i] = itemId;
	}
}

void Scene2020::remapOriginalColorMapItems(byte firstSourceItem, byte secondSourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		const byte originalItem = originalColorMapItemAt(i);
		if (originalItem == firstSourceItem || originalItem == secondSourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

byte Scene2020::originalColorMapItemAt(uint paletteIndex) const {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			paletteIndex >= kScenePaletteMapPageSize)
		return 0;

	return _paletteMaskOriginal[kSceneColorToItemMap + paletteIndex];
}

} // End of namespace Hollywood
