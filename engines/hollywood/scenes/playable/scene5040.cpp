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

#include "hollywood/scenes/playable/scene5040.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const char *const kScene5040ArchiveName = "RESOURCE.E04";
const char *const kScene5040MusicArchiveName = "RESOURCE.M05";
const char *const kScene5040SoundArchiveName = "RESOURCE.S05";
const uint kScene5040InitialRequiredChunkCount = 5;
const uint kScene5040ArenaFirstChunk = 5;
const uint kScene5040ArenaLastChunk = 17;
const uint kScene5040StageIndex = 504;
const uint16 kScene5040FirstState = 0x13b0;
const uint16 kScene5040LastState = 0x13b9;
const uint16 kScene5010ReturnState = 0x1393;
const uint kScene5040ActorBankTableEntry = 0x0000;
const uint kScene5040ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5040SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5040FrameMillis = 75;
const uint32 kScene5040MineCartFrameMillis = 40;
const uint kScene5040KarlDescriptorCount = 0x2e;
const uint kScene5040MineCartDescriptorCount = 0x59;
const byte kScene5040KarlSpeechGroup = 0;
const byte kScene5040KarlSpeechBaseFrame = 0x4c;
const byte kScene5040KarlDialogueStageId = 0x62;
const byte kScene5040KarlPrimaryRow = 99;
const byte kScene5040DialogueNoResponseFrame = 0xff;
const uint kScene5040KarlDialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene5040LooseObjectItem = 0x0d;
const byte kScene5040OldSockItem = 0x0c;
const byte kScene5040WandItem = 0x0b;
const byte kScene5040KarlPrizeItem = 0x4a;

enum {
	kScene5040DialogueTransitionEnd = 0,
	kScene5040DialogueTransitionDown = 1,
	kScene5040DialogueTransitionUp = 2,
	kScene5040DialogueTransitionStay = 3,
	kScene5040DialogueTransitionUpTwo = 4
};

const byte kScene5040ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene5040KarlFrameMap[] = {
	0, 43, 44, 45, 0, 1, 2, 3, 4, 3, 4, 3, 4, 3, 4, 3,
	4, 3, 2, 1, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14,
	13, 12, 11, 10, 9, 8, 7, 6, 5, 0, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, 26, 27, 28, 0, 29, 30, 31, 32, 33, 34, 34,
	34, 34, 35, 36, 37, 0, 37, 38, 39, 40, 41, 42, 20, 21, 22, 21
};

const byte kScene5040Chunk12FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene5040Chunk15FrameMap[] = {
	13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

PlayableSceneConfig scene5040Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene5040ArchiveName;
	config.initialRequiredChunkCount = kScene5040InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene5040ArenaFirstChunk;
	config.arenaLastChunk = kScene5040ArenaLastChunk;
	config.stageIndex = kScene5040StageIndex;
	config.debugName = "Scene 5040";
	config.viewportXOffset = 0;
	config.viewportMinXOffset = 0;
	config.viewportMaxXOffset = 0;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 5;
	config.actorBankTableEntry = kScene5040ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene5040ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = 0;
	config.speechCueDescriptorTableOffset = kScene5040SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene5040ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene5040ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene5040MusicArchiveName;
	config.soundBank0ArchiveName = kScene5040SoundArchiveName;
	config.mainFlowFirstState = kScene5040FirstState;
	config.mainFlowLastState = kScene5040LastState;
	return config;
}

Scene5040::Scene5040(HollywoodEngine *vm) :
		PlayableScene(vm, scene5040Config(), "scene5040", 0x218, 0x161, 5, 0xfd, 0xfb),
		_karlIdleChannel(),
		_karlLayer(),
		_karlIdleFrame(0),
		_karlIdleEndFrame(0),
		_karlIdleActive(false) {
	_karlLayer.configure(9, kScene5040KarlDescriptorCount,
		kScene5040KarlFrameMap, ARRAYSIZE(kScene5040KarlFrameMap));
}

bool Scene5040::hasCustomPreviewState() const {
	return true;
}

void Scene5040::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	_activeActorWorldX = _vm->gameState().scene5040EntryLineSeen ? 0x19e : 0x218;
	_activeActorWorldY = _vm->gameState().scene5040EntryLineSeen ? 0x172 : 0x161;
	_activeActorFacing = _vm->gameState().scene5040EntryLineSeen ? 4 : 5;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene5040::hasCustomComposite() const {
	return true;
}

void Scene5040::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_karlLayer.visible && _vm->gameState().scene5040MineGalleryState < 2)
		drawResourceSpriteLayer(_karlLayer);

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (activeWorldY < 0x138 && _sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(16))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[16], _sceneFramebuffer);
	drawActionOverlayLayer();
}

bool Scene5040::hasCustomEntrySequence() const {
	return true;
}

void Scene5040::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();

	if (state.scene5040SpecialTransitionState == 1) {
		_activeActorWorldX = 0x32d;
		_activeActorWorldY = 0x12c;
		_activeActorFacing = 4;
		_activeActorCel = 0;
		_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
		drawPlayableComposite();
		presentFrame();
		runMineCartEntryClip();
		walkActiveActorTo(0x238, 0x145, 5, 0, false);
		beginSecondarySpeechLine(8, 0);
		runConfiguredActionOverlay(15, 0x0e, kScene5040Chunk15FrameMap,
			ARRAYSIZE(kScene5040Chunk15FrameMap), kScene5040FrameMillis,
			kActionOverlayHideActiveActor, 6, 6, -1, 0, 100, -1, 0, true);
		addInventoryItem(kScene5040KarlPrizeItem);
		_soundBank0.playSample(1, 100);
		state.scene5040SpecialTransitionState = 2;
		walkActiveActorTo(0x208, 0x15e, 4, 0, false);
		return;
	}

	if (state.mainFlowStateId == kScene5040FirstState) {
		_activeActorWorldX = 0x32d;
		_activeActorWorldY = 0x110;
		_activeActorFacing = 4;
		_activeActorCel = 0;
		_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
		drawPlayableComposite();
		presentFrame();

		runMineCartEntryClip();
		if (state.scene5040EntryLineSeen) {
			walkActiveActorTo(0x19e, 0x172, 4, 0, false);
		} else {
			walkActiveActorTo(0x218, 0x161, 5, 0, false);
			beginSecondarySpeechLine(0, 0);
			state.scene5040EntryLineSeen = true;
		}
		return;
	}

	runEntryPath(0x0f6, 0x0e9, 2, 0x1c3, 0x15e);
	_activeActorFacing = 2;
	_activeActorCel = 0;
}

bool Scene5040::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene5040::advanceCustomGameplayLoop(uint32 delta) {
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceKarlLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene5040::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a vagoneta/salida (go to mine cart/exit): return to mine switches.
		runExitToMineSwitches();
		return true;
	case 302: // Mirar/usar túnel (look/use tunnel): generic mine-gallery response.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar minero/Karl Hecker (look at miner/Karl Hecker).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 304: // Hablar con minero/Karl Hecker (talk to miner/Karl Hecker).
		runKarlConversation();
		return true;
	case 305: // Mirar boquete/caja (look at hole/box): identifies what is inside.
		beginSecondarySpeechLine(10, _vm->gameState().scene5040OldSockTaken ? 1 : 0);
		return true;
	case 306: // Coger boquete/caja (take from hole/box): first loose object pickup.
		runLooseObjectPickup();
		return true;
	case 307: // Usar/coger minero (use/take miner): Ron avoids angering him.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Mirar varita (look at wand): it locates diamonds.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Mirar pico (look at pickaxe): standard miner tool.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 312: // Unused no-op scene action slot in original callback table.
		return true;
	case 313: // Usar calcetín viejo con caja/boquete: reveals the small key state.
		beginSecondarySpeechLine(10, 1);
		return true;
	case 314: // Mirar/coger calcetín viejo (look/take old sock): patched generic speech row 11.
		beginSecondarySpeechLine(11, 0);
		return true;
	case 315: // Coger calcetín viejo/llave (take old sock/key): grants inventory item 0x0c.
		runOldSockPickup();
		return true;
	case 316: // Mirar llave (look at key): key is inside the old sock.
		beginSecondarySpeechLine(10, 1);
		return true;
	case 317: // Usar varita con Karl/minero (use wand with Karl): return to switch room with mine-transport state 4.
		runSpecialMineExitWithWand();
		return true;
	default:
		return false;
	}
}

bool Scene5040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if (selector == 1 || selector == 0xff) {
		switch (state.scene5040MineGalleryState) {
		case 0:
			remapSceneColors(3, 0);
			remapSceneColors(4, 4);
			remapSceneColors(0x0e, 4);
			remapSceneColors(0x0f, 0);
			remapSceneColors(0x10, 0);
			remapSceneColors(0x11, 0);
			break;
		case 1:
			remapSceneColors(3, 0);
			remapSceneColors(4, 0);
			remapSceneColors(0x0e, 4);
			remapSceneColors(0x0f, 4);
			remapSceneColors(0x10, 4);
			remapSceneColors(0x11, 0);
			break;
		case 2:
			remapSceneColors(3, 3);
			remapSceneColors(4, 0);
			remapSceneColors(0x0e, 0);
			remapSceneColors(0x0f, 0);
			remapSceneColors(0x10, 3);
			remapSceneColors(0x11, 4);
			if (_sceneChunkTable.isValidChunk(6))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
			if (_sceneChunkTable.isValidChunk(8))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
			break;
		default:
			break;
		}
	}
	if ((selector == 2 || selector == 0xff) && state.scene5040LooseObjectTaken) {
		remapSceneColors(5, 0);
		if (state.scene5040MineGalleryState != 1)
			remapSceneColors(0x0d, 0);
	}
	if ((selector == 3 || selector == 0xff) && state.scene5040OldSockTaken) {
		remapSceneColors(6, 0);
		if (state.scene5040MineGalleryState != 1)
			remapSceneColors(0x0b, 0);
		remapSceneColors(0x0c, 0);
		clearSceneItemFromColorMap(6);
	}
	if (selector == 4 || selector == 0xff) {
		switch (state.scene5040DialState) {
		case 1:
			remapSceneColors(0x12, 9);
			break;
		case 3:
			remapSceneColors(0x12, 10);
			break;
		default:
			remapSceneColors(0x12, 8);
			break;
		}
	}
	if ((selector == 5 || selector == 0xff) && state.scene5040KarlDialogueIntroSeen)
		copyStageSmallRow(4, 11);
	if ((selector == 6 || selector == 0xff) && state.scene5040SpecialTransitionState == 1 &&
			_sceneChunkTable.isValidChunk(13))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene5040::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 75, 25, 0x0b, 5, 100, 50);
}

byte Scene5040::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	return animationGroup == kScene5040KarlSpeechGroup ? kScene5040KarlSpeechBaseFrame : 0;
}

void Scene5040::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene5040KarlSpeechGroup)
		_karlLayer.setFrame(frameIndex);
}

void Scene5040::resetAnimationLayers() {
	_karlIdleChannel.reset(0, kScene5040FrameMillis);
	_karlLayer.visible = _vm->gameState().scene5040MineGalleryState < 2;
	_karlLayer.reset(_vm->gameState().scene5040MineGalleryState == 1 ? 0x49 : 0);
	_karlIdleFrame = _karlLayer.frameIndex;
	_karlIdleEndFrame = _karlIdleFrame;
	_karlIdleActive = false;
}

void Scene5040::advanceKarlLayer(uint32 delta) {
	if (!_karlLayer.visible)
		return;

	const uint consumedFrames = _karlIdleChannel.consumeFrames(delta);
	for (uint i = 0; i < consumedFrames; ++i) {
		if (!_karlIdleActive) {
			if (_random.getRandomNumber(24) != 0)
				return;

			const byte sequence = (byte)_random.getRandomNumber(2);
			_karlIdleFrame = sequence == 0 ? 4 : 21;
			_karlIdleEndFrame = sequence == 0 ? 20 : 41;
			_karlIdleActive = true;
		} else if (_karlIdleFrame >= _karlIdleEndFrame) {
			_karlIdleFrame = 0;
			_karlIdleEndFrame = 0;
			_karlIdleActive = false;
		} else {
			++_karlIdleFrame;
		}
		_karlLayer.setFrame(_karlIdleFrame);
	}
}

void Scene5040::runMineCartEntryClip() {
	Common::Array<byte> frameMap;
	frameMap.resize(kScene5040MineCartDescriptorCount);
	for (uint i = 0; i < frameMap.size(); ++i)
		frameMap[i] = (byte)i;

	runConfiguredActionOverlay(17, kScene5040MineCartDescriptorCount,
		frameMap.data(), frameMap.size(), kScene5040MineCartFrameMillis,
		kActionOverlayHideActiveActor, -1, 0, 0x32, 0x16);
}

void Scene5040::runExitToMineSwitches() {
	walkActiveActorTo(0x0f6, 0x0e9, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5040::runKarlConversation() {
	Common::Array<DialogueChoiceRecord> records;
	initializeKarlDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	if (!state.scene5040KarlDialogueIntroSeen) {
		beginSecondarySpeechLine(kScene5040KarlDialogueStageId, 0);
		beginKarlSpeechLine(0);
		beginSecondarySpeechLine(kScene5040KarlDialogueStageId, 1);
		beginKarlSpeechLine(1);
		state.scene5040KarlDialogueIntroSeen = true;
		applySceneStateToHotspotsAndPatches(5);
	} else {
		beginSecondarySpeechLine(kScene5040KarlDialogueStageId, 0);
		beginKarlSpeechLine(0);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene5040KarlDialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene5040KarlDialogueStageId, 7);
			beginKarlSpeechLine(7);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			return;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene5040KarlDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene5040DialogueNoResponseFrame)
			beginKarlSpeechLine(record.responseFrameIndex);

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}

		if (applyKarlDialogueTransition(record, depthIndex, nodeIndex))
			return;
	}
}

void Scene5040::runLooseObjectPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5040LooseObjectTaken || hasInventoryItem(kScene5040LooseObjectItem)) {
		beginSecondarySpeechLine(12, 0);
		return;
	}

	runHiddenActorActionOverlay(12, 0x0c, kScene5040Chunk12FrameMap,
		ARRAYSIZE(kScene5040Chunk12FrameMap), kScene5040FrameMillis);
	addInventoryItem(kScene5040LooseObjectItem);
	_soundBank0.playSample(1, 100);
	state.scene5040LooseObjectTaken = true;
	applySceneStateToHotspotsAndPatches(2);
}

void Scene5040::runOldSockPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5040OldSockTaken || hasInventoryItem(kScene5040OldSockItem)) {
		beginSecondarySpeechLine(11, 0);
		return;
	}

	runHiddenActorActionOverlay(12, 0x0c, kScene5040Chunk12FrameMap,
		ARRAYSIZE(kScene5040Chunk12FrameMap), kScene5040FrameMillis);
	addInventoryItem(kScene5040OldSockItem);
	_soundBank0.playSample(1, 100);
	state.scene5040OldSockTaken = true;
	applySceneStateToHotspotsAndPatches(3);
}

void Scene5040::runSpecialMineExitWithWand() {
	GameplayState &state = _vm->gameState();
	if (!hasInventoryItem(kScene5040WandItem)) {
		beginSecondarySpeechLine(15, 2);
		return;
	}

	removeInventoryItem(kScene5040WandItem);
	beginSecondarySpeechLine(15, 0);
	state.scene5040SpecialTransitionState = 1;
	state.scene5040MineGalleryState = 2;
	state.scene5010MineTransportState = 4;
	state.mainFlowStateId = kScene5010ReturnState;
}

void Scene5040::initializeKarlDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene5040KarlDialogueChoiceRecordCount);

	// DAT_005091e0: root choices for Karl Hecker.
	setKarlDialogueRecord(records, 0, 0, kScene5040DialogueTransitionDown, 3, 3, 1); // ¿Qué está haciendo aquí?
	setKarlDialogueRecord(records, 1, 0, kScene5040DialogueTransitionStay, 4, 4, 1); // ¿Ha visto al hombre lobo?
	setKarlDialogueRecord(records, 2, 0, kScene5040DialogueTransitionStay, 5, 5, 1); // ¿Cuánto tiempo ha pasado en estas galerías?
	setKarlDialogueRecord(records, 3, 0, kScene5040DialogueTransitionStay, 6, 6, 1); // He oído hablar de usted y sus hermanos.
	setKarlDialogueRecord(records, 4, 0, kScene5040DialogueTransitionEnd, 7, 7, 0); // Bueno, le dejo ya.

	// Depth 1, node 0: follow-up choices after Karl explains he is looking for the diamond.
	setKarlDialogueRecord(records, 70, 0, kScene5040DialogueTransitionStay, 8, 8, 1); // Difícil encontrarlo en las galerías.
	setKarlDialogueRecord(records, 71, 0, kScene5040DialogueTransitionStay, 9, 9, 1); // Por qué es importante el diamante.
	setKarlDialogueRecord(records, 72, 0, kScene5040DialogueTransitionStay, 10, 10, 1); // Qué tipo de sorpresas.
	setKarlDialogueRecord(records, 73, 0, kScene5040DialogueTransitionUp, 11, 11, 0); // Espero que tenga suerte.
}

void Scene5040::setKarlDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = 1;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = 0xff;
	record.selectable = 1;
}

bool Scene5040::applyKarlDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) const {
	const byte previousDepth = depthIndex;
	switch (record.transitionMode) {
	case kScene5040DialogueTransitionEnd:
		return true;
	case kScene5040DialogueTransitionDown:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth + 1;
		break;
	case kScene5040DialogueTransitionUp:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth == 0 ? 0 : (byte)(previousDepth - 1);
		break;
	case kScene5040DialogueTransitionUpTwo:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
		break;
	case kScene5040DialogueTransitionStay:
	default:
		break;
	}

	return false;
}

void Scene5040::beginKarlSpeechLine(byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(kScene5040KarlPrimaryRow, frameIndex,
		0x1b7, 0x067, 0x20, 0x30, 0x3f, kScene5040KarlSpeechGroup);
}

void Scene5040::copyStageSmallRow(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene5040::remapSceneColors(byte sourceColor, byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[color] == sourceColor)
			_paletteMask[kSceneColorToItemMap + color] = itemId;
	}
}

void Scene5040::clearSceneItemFromColorMap(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
