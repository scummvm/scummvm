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

#include "hollywood/scenes/playable/scene3100.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene3080ReturnFromScene3100State = 0x0c09;
const uint16 kScene3100ViewportXOffset = 0x0028;
const uint16 kScene3100ViewportMinXOffset = 0x0000;
const uint16 kScene3100ViewportMaxXOffset = 0x0028;
const uint kScene3100ActorBankTableEntry = 0x0000;
const uint kScene3100ActorPaletteTableEntry = 0x00cc;
const uint kScene3100Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3100SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3100CabinFrameMillis = 125;
const uint32 kScene3100AlternateFrameMillis = 125;
const uint32 kScene3100OverlayFrameMillis = 75;
const uint kScene3100CabinDescriptorCount = 0x16;
const uint kScene3100AlternateDescriptorCount = 0x10;
const uint kScene3100ObjectOverlayDescriptorCount = 0x0e;
const uint kScene3100ExchangeOverlayDescriptorCount = 8;
const byte kScene3100DialogueStageId = 0x62;
const byte kScene3100DialoguePrimaryRow = 99;
const uint kScene3100DialogueChoiceRecordCount = 10 * 10 * 7;
const uint16 kScene3100PrimarySpeechCenterX = 0x1ff;
const uint16 kScene3100PrimarySpeechTopY = 0x0b6;
const byte kScene3100PrimarySpeechRed = 0x3f;
const byte kScene3100PrimarySpeechGreen = 0x20;
const byte kScene3100PrimarySpeechBlue = 0x3f;
const byte kScene3100PickupItem38 = 0x38;
const byte kScene3100PickupItem39 = 0x39;

const byte kScene3100CabinFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
};

const byte kScene3100AlternateFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15
};

const byte kScene3100ObjectPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

const byte kScene3100ExchangePickupFrameMap[] = {
	6, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7
};

const byte kScene3100ResolutionFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

static PlayableSceneConfig scene3100Config() {
	PlayableSceneConfig config(3100,
		SceneResourceLayout(13, 5, 12),
		SceneViewport(kScene3100ViewportXOffset, kScene3100ViewportMinXOffset, kScene3100ViewportMaxXOffset),
		SceneActorPose(0x276, 0x1c2, 5));
	config.setActorResources(kScene3100ActorBankTableEntry, kScene3100ActorPaletteTableEntry);
	config.setTextResources(kScene3100Resource003RowsOffsetIndex, kScene3100SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene3100::Scene3100(HollywoodEngine *vm) :
		PlayableScene(vm, scene3100Config()),
		_cabinChannel(),
		_alternateChannel(),
		_cabinLayer(),
		_alternateLayer(),
		_dialogueMenuActive(false) {
	_cabinLayer.configure(6, kScene3100CabinDescriptorCount,
		kScene3100CabinFrameMap, ARRAYSIZE(kScene3100CabinFrameMap));
	_alternateLayer.configure(12, kScene3100AlternateDescriptorCount,
		kScene3100AlternateFrameMap, ARRAYSIZE(kScene3100AlternateFrameMap));
}

void Scene3100::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	setActiveActorPose(0x276, 0x1c2, 5);
}

void Scene3100::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_vm->gameState().scene3100GirlConversationState < 2)
		drawResourceSpriteLayer(_cabinLayer);
	else
		drawResourceSpriteLayer(_alternateLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene3100::runCustomEntrySequence() {
	if (_vm->gameState().scene3100CabinVisited && hasSavedActiveActorPoseForCurrentState()) {
		restoreActiveActorPoseFromGameState();
		drawPlayableComposite();
		presentFrame();
		return;
	}

	runEntryPath(800, 0x1d6, 5, 0x276, 0x1c2);
	_activeActorFacing = 5;
	_vm->gameState().scene3100CabinVisited = true;
}

bool Scene3100::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene3100::advanceCustomGameplayLoop(uint32 delta) {
	advanceCabinLayers(delta);
	if (!_dialogueMenuActive)
		updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3100::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Ir a exterior de la cabaña (go outside): return to scene 3080.
		runExitToScene3080();
		return true;
	case 302: // Mirar niña (look at girl): state-aware girl description.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Hablar con ocupante de la cabaña (talk to cabin occupant).
		runCabinConversation();
		_cabinChannel.frameIndex = 5;
		_cabinLayer.setFrame(5);
		return true;
	case 304: // Mirar ocupante/estado de la cabaña (look at cabin occupant/state).
		beginSecondarySpeechLine(1, state.scene3100GirlConversationState == 0 ? 0 : 1);
		return true;
	case 305: // Coger margarita revelada tras la conversacion (take revealed daisy): adds item 0x39.
		runObjectPickup();
		return true;
	case 306: // Mirar/coger margarita (look/take daisy): flower from the girl.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 307: // Mirar planta (look at plant): living sap source.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 308: // Mirar tronco (look at log).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 309: // Mirar rio (look at river).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 310: // Coger savia de la planta (take plant sap): adds item 0x38.
		runExchangePickup();
		return true;
	default:
		return false;
	}
}

bool Scene3100::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 0 || selector == 1 || selector == 8) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

		const GameplayState &state = _vm->gameState();
		if (state.scene3100GirlConversationState == 2)
			removeColorMapItem(2);
		if (state.scene3100DaisyVisible && !state.scene3100DaisyTaken) {
			if (_sceneChunkTable.isValidChunk(9))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
			replaceColorMapItemFromOriginal(3, 3);
		} else {
			if (_sceneChunkTable.isValidChunk(10))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
			removeColorMapItem(3);
		}
		if (state.scene3100SapSyringeTaken)
			removeColorMapItem(8);

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

AmbientAudioProfile Scene3100::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.soundMode = kAmbientSoundLoop;
	profile.soundCueId = 0x18;
	profile.soundVolumePercent = 1; // Legacy 0x78 is about -40 dB.
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3100::resetAnimationLayers() {
	const GameplayState &state = _vm->gameState();
	const byte cabinFrame = state.scene3100GirlConversationState < 2 ? 6 : 15;
	_cabinChannel.reset(cabinFrame, kScene3100CabinFrameMillis);
	_alternateChannel.reset(15, kScene3100AlternateFrameMillis);
	_cabinLayer.visible = true;
	_alternateLayer.visible = true;
	_cabinLayer.reset(cabinFrame);
	_alternateLayer.reset(15);
	_dialogueMenuActive = false;
}

void Scene3100::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}
}

void Scene3100::removeColorMapItem(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == itemId)
			_paletteMask[kSceneColorToItemMap + i] = 0;
	}
}

void Scene3100::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene3100::advanceCabinLayers(uint32 delta) {
	GameplayState &state = _vm->gameState();
	if (state.scene3100GirlConversationState < 2) {
		const uint frameCount = _cabinChannel.consumeFrames(delta);
		for (uint frame = 0; frame < frameCount; ++frame) {
			if (_cabinChannel.frameIndex < 6 || _cabinChannel.frameIndex >= 15)
				_cabinChannel.frameIndex = 6;
			else
				++_cabinChannel.frameIndex;
			_cabinLayer.setFrame(_cabinChannel.frameIndex);
		}
		return;
	}

	const uint frameCount = _alternateChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_alternateChannel.frameIndex >= 15) {
			if (_random.getRandomNumber(19) == 0)
				_alternateChannel.frameIndex = 0;
		} else {
			++_alternateChannel.frameIndex;
		}
		_alternateLayer.setFrame(_alternateChannel.frameIndex);
	}
}

void Scene3100::runExitToScene3080() {
	_vm->gameState().mainFlowStateId = kScene3080ReturnFromScene3100State;
}

void Scene3100::runCabinConversation() {
	GameplayState &state = _vm->gameState();
	Common::Array<DialogueChoiceRecord> records;
	initializeDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	_vm->gameplayMusic()->stop();

	if (state.scene3100GirlConversationState == 0) {
		beginSecondarySpeechLine(kScene3100DialogueStageId, 0);
		beginCabinPrimaryResponse(0);
		beginSecondarySpeechLine(kScene3100DialogueStageId, 1);
		beginCabinPrimaryResponse(1);
		state.scene3100GirlConversationState = 1;
	} else {
		beginSecondarySpeechLine(kScene3100DialogueStageId, 2);
		beginCabinPrimaryResponse(2);
	}

	if (state.scene3100GirlDialogueRepeatCounter != 0 && records.size() > 2)
		records[2].responseFrameIndex = state.scene3100GirlDialogueRepeatCounter + 10;

	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		_dialogueMenuActive = true;
		const byte selectedChoice = menu.choose(kScene3100DialogueStageId, records, depthIndex, nodeIndex);
		_dialogueMenuActive = false;
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene3100DialogueStageId, 6);
			beginCabinPrimaryResponse(6);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene3100DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff)
			beginCabinPrimaryResponse(record.responseFrameIndex);

		if (record.disableAfterUse == 1)
			record.enabled = 0;
		if (record.disableAfterUse == 2) {
			if (state.scene3100GirlDialogueRepeatCounter < 9) {
				const byte previousCounter = state.scene3100GirlDialogueRepeatCounter;
				state.scene3100GirlDialogueRepeatCounter++;
				record.responseFrameIndex = previousCounter + 0x0b;
			} else {
				record.transitionMode = 0;
				runConversationResolutionSequence();
			}
		}

		const byte previousDepth = depthIndex;
		switch (record.transitionMode) {
		case 0:
			finished = true;
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
}

void Scene3100::initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene3100DialogueChoiceRecordCount);

	setDialogueRecord(records, 0, 1, 0, 1, 3, 3, 1);
	setDialogueRecord(records, 1, 1, 0, 3, 4, 4, 1);
	setDialogueRecord(records, 2, 1, 0, 3, 5, 5, 2);
	setDialogueRecord(records, 3, 1, 0, 0, 6, 6, 0);

	setDialogueRecord(records, 70, 1, 0, 3, 7, 7, 1);
	setDialogueRecord(records, 71, 1, 0, 3, 8, 8, 1);
	setDialogueRecord(records, 72, 1, 0, 3, 9, 9, 1);
	setDialogueRecord(records, 73, 1, 0, 2, 10, 10, 0);
}

void Scene3100::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
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

void Scene3100::beginCabinPrimaryResponse(byte frameIndex) {
	_vm->gameplayMusic()->stop();
	beginPrimarySpeechLine(kScene3100DialoguePrimaryRow, frameIndex,
		kScene3100PrimarySpeechCenterX, kScene3100PrimarySpeechTopY,
		kScene3100PrimarySpeechRed, kScene3100PrimarySpeechGreen, kScene3100PrimarySpeechBlue);
}

void Scene3100::runConversationResolutionSequence() {
	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(kScene3100DialogueStageId, 0x0b);
	runSceneOverlay(ActionOverlaySpec(8, 0x0b,
		kScene3100ResolutionFrameMap, ARRAYSIZE(kScene3100ResolutionFrameMap), kScene3100OverlayFrameMillis)
		.soundAt(5, 0x19));
	state.scene3100GirlConversationState = 2;
	state.scene3100DaisyVisible = true;
	_alternateChannel.frameIndex = 15;
	_alternateLayer.setFrame(15);
	applySceneStateToHotspotsAndPatches(0xff);
	beginSecondarySpeechLine(kScene3100DialogueStageId, 0x0c);
}

void Scene3100::runObjectPickup() {
	GameplayState &state = _vm->gameState();
	if (!state.scene3100DaisyVisible || state.scene3100DaisyTaken) {
		beginSecondarySpeechLine(2, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(11, kScene3100ObjectOverlayDescriptorCount,
		kScene3100ObjectPickupFrameMap, ARRAYSIZE(kScene3100ObjectPickupFrameMap), kScene3100OverlayFrameMillis)
		.patchAt(7, 1)
		.soundAt(7, 1));
	state.scene3100DaisyVisible = false;
	state.scene3100DaisyTaken = true;
	applySceneStateToHotspotsAndPatches(1);
	addInventoryItem(kScene3100PickupItem39);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(3, 0);
}

void Scene3100::runExchangePickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene3100SapSyringeTaken) {
		beginSecondarySpeechLine(7, 0);
		return;
	}

	beginSecondarySpeechLine(7, 0);
	runActorReplacement(ActionOverlaySpec(7, kScene3100ExchangeOverlayDescriptorCount,
		kScene3100ExchangePickupFrameMap, ARRAYSIZE(kScene3100ExchangePickupFrameMap), kScene3100OverlayFrameMillis));
	state.scene3100SapSyringeTaken = true;
	applySceneStateToHotspotsAndPatches(8);
	addInventoryItem(kScene3100PickupItem38);
	removeInventoryItem(8);
	_soundBank0.playSample(1, 100);
}

} // End of namespace Hollywood
