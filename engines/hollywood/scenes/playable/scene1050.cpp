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

#include "hollywood/scenes/playable/scene1050.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene1050ExitState1040FromDoor = 0x0411;
const uint16 kScene1050ViewportXOffset = 0x0068;
const uint16 kScene1050ViewportMinXOffset = 0x0068;
const uint16 kScene1050ViewportMaxXOffset = 0x00a0;
const uint kScene1050ActorBankTableEntry = 0x0000;
const uint kScene1050ActorPaletteTableEntry = 0x00cc;
const uint kScene1050Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1050SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene1050FrameMillis = 75;
const uint32 kScene1050SmallOverlayFrameMillis = 125;
const uint32 kScene1050LargeOverlayFrameMillis = 75;
const uint kScene1050SmallOverlayDescriptorCount = 5;
const uint kScene1050LargeOverlayDescriptorCount = 0x3e;
const uint kScene1050DoorOverlayDescriptorCount = 6;
const uint kScene1050JacketOverlayDescriptorCount = 0x0e;
const uint kScene1050SuitcaseOverlayDescriptorCount = 0x0b;
const uint kScene1050TravelOverlayDescriptorCount = 0x11;
const byte kScene1050DialogueStageId = 0x62;
const byte kScene1050DialoguePrimaryRow = 99;
const uint16 kScene1050DialoguePrimaryCenterX = 0x1d6;
const uint16 kScene1050DialoguePrimaryTopY = 0x95;
const uint16 kScene1050DialoguePrimaryAltCenterX = 0x1cb;
const uint16 kScene1050DialoguePrimaryAltTopY = 0x96;
const byte kScene1050DialoguePrimaryRed = 6;
const byte kScene1050DialoguePrimaryGreen = 0x3f;
const byte kScene1050DialoguePrimaryBlue = 0x2d;
const byte kScene1050PrimarySpeechNormalGroup = 1;
const byte kScene1050PrimarySpeechAltGroup = 2;
const uint kScene1050DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene1050FirstAmbientSoundCue = 0x25;
const byte kScene1050AmbientSoundCueCount = 7;
const byte kScene1050FirstAmbientMusicCue = 0x0b;
const byte kScene1050AmbientMusicCueCount = 5;
const byte kScene1050AmbientSoundProbabilityModulus = 25;
const byte kScene1050AmbientMusicProbabilityModulus = 50;

const byte kScene1050SmallOverlayFrameMap[] = {
	0, 0, 1, 1, 2, 2, 3, 3, 4, 4,
	4, 4, 4, 4, 4, 4, 3, 2, 1, 0,
	0, 0, 0, 0, 0
};

const byte kScene1050LargeOverlayFrameMap[] = {
	0, 1, 2, 3, 60, 18, 19, 20, 21, 21, 22, 23,
	24, 61, 20, 19, 18, 0, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 16, 15, 14, 13,
	12, 11, 10, 9, 8, 7, 6, 5, 4, 0, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	0, 25, 26, 25, 0, 41, 42, 43, 44, 45, 44, 45,
	44, 45, 44, 45, 46, 45, 46, 45, 46, 45, 46, 47,
	48, 48, 48, 48, 48, 48, 49, 50, 51, 52, 53, 54,
	55, 56, 57, 58, 59
};

const byte kScene1050DoorFrameMap[] = { 0, 1, 2, 3, 4, 5 };
const byte kScene1050SuitcaseFrameMap[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
const byte kScene1050TravelFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 12, 13,
	14, 15, 16, 11, 10, 9, 8, 7,
	6, 5, 4, 3, 2, 1, 0
};

const byte kScene1050JacketFirstFrameMap[] = {
	0, 13, 12, 11, 10, 9, 8, 7,
	6, 5, 5, 5, 4, 3, 2, 1,
	0
};

const byte kScene1050JacketFirstLargeOverlayFrameMap[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 61, 62, 63, 64, 64, 64,
	64
};

const byte kScene1050JacketSecondFrameMap[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 3,
	4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 0
};

const byte kScene1050JacketSecondLargeOverlayFrameMap[] = {
	0, 46, 47, 48, 49, 50, 51, 52,
	53, 54, 55, 56, 57, 57, 57, 57,
	57, 57, 58, 59, 60, 60, 60, 60,
	60, 60, 60
};

static PlayableSceneConfig scene1050Config() {
	PlayableSceneConfig config(1050,
		SceneResourceLayout(14, 5, 13),
		SceneViewport(kScene1050ViewportXOffset, kScene1050ViewportMinXOffset, kScene1050ViewportMaxXOffset),
		SceneActorPose(0x07f, 0x174, 2));
	config.setActorResources(kScene1050ActorBankTableEntry, kScene1050ActorPaletteTableEntry);
	config.setTextResources(kScene1050Resource003RowsOffsetIndex, kScene1050SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 6;
	return config;
}

Scene1050::Scene1050(HollywoodEngine *vm) :
		PlayableScene(vm, scene1050Config()),
		_smallOverlayChannel(),
		_largeOverlayChannel(),
		_smallOverlayLayer(),
		_largeOverlayLayer(),
		_largeOverlayMode(0),
		_largeOverlayActionLocked(false) {
	_smallOverlayLayer.configure(7, kScene1050SmallOverlayDescriptorCount,
		kScene1050SmallOverlayFrameMap, ARRAYSIZE(kScene1050SmallOverlayFrameMap));
	_largeOverlayLayer.configure(8, kScene1050LargeOverlayDescriptorCount,
		kScene1050LargeOverlayFrameMap, ARRAYSIZE(kScene1050LargeOverlayFrameMap));
	_smallOverlayLayer.visible = true;
	_largeOverlayLayer.visible = true;
}

void Scene1050::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	_smallOverlayChannel.reset(0, kScene1050SmallOverlayFrameMillis);
	_largeOverlayChannel.reset(0, kScene1050LargeOverlayFrameMillis);
	_smallOverlayLayer.reset(0);
	_largeOverlayLayer.reset(0);
	_smallOverlayLayer.visible = true;
	_largeOverlayLayer.visible = true;
	_largeOverlayMode = 0;
	_largeOverlayActionLocked = false;
	setActiveActorPose(0x07f, 0x174, 2);
}

void Scene1050::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_largeOverlayLayer);
	if (_actionOverlayPlayer.layer.chunkIndex == 12)
		drawActionOverlayLayer();
	drawResourceSpriteLayer(_smallOverlayLayer);
	if (_actionOverlayPlayer.layer.chunkIndex != 12)
		drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

void Scene1050::runCustomEntrySequence() {
	_soundBank0.playSample(4, 100);
	runEntryPath(0x050, 0x174, 2, 0x07f, 0x174);
	drawPlayableComposite();
	presentFrame();
}

bool Scene1050::prepareCustomGameplayLoop() {
	_smallOverlayChannel.reset(_smallOverlayLayer.frameIndex, kScene1050SmallOverlayFrameMillis);
	_largeOverlayChannel.reset(_largeOverlayLayer.frameIndex, kScene1050LargeOverlayFrameMillis);
	return true;
}

bool Scene1050::advanceCustomGameplayLoop(uint32 delta) {
	advanceSmallOverlay(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else if (!_largeOverlayActionLocked)
		advanceLargeOverlay(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1050::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar puerta (look at door).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar/abrir puerta (use/open door).
		runDoorBackToGorillaRoomAction();
		return true;
	case 303: // Hablar con tipo del guardarropa (talk to cloakroom attendant).
		runCloakroomAttendantConversation();
		applySceneStateToHotspotsAndPatches(1);
		return true;
	case 304: // Mirar tipo del guardarropa (look at cloakroom attendant).
		beginSecondarySpeechLine(2, _vm->gameState().scene1050CloakroomAttendantConversationSeen ? 1 : 0);
		return true;
	case 305: // Coger chaqueta (take jacket).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 306: // Mirar chaqueta (look at jacket).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Mirar cortina (look at curtain).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Usar objeto especial con cortina/chaqueta.
		handleJacketExchange();
		return true;
	case 309: // Hablar con Jack el destripador (talk to Jack the Ripper).
		handleJackTalkLine();
		return true;
	case 310: // Mirar Jack el destripador (look at Jack the Ripper).
		handleJackLookLine();
		return true;
	case 311: // Coger maletín (take suitcase).
		handleSuitcasePickup();
		return true;
	case 312: // Mirar maletín (look at suitcase).
		beginSecondarySpeechLine(10, 0);
		return true;
	default:
		return false;
	}
}

bool Scene1050::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene1050SuitcaseTaken &&
			_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize &&
			_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
		for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
			if (_paletteMaskOriginal[kSceneColorToItemMap + i] == 6)
				_paletteMask[kSceneColorToItemMap + i] = 0;
		}
	}

	if (state.scene1050SuitcaseTaken && _sceneChunkTable.isValidChunk(11))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);

	if (state.scene1050CloakroomAttendantConversationSeen)
		copyStageSmallRow(7, 2);

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

bool Scene1050::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene1050::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene1050PrimarySpeechAltGroup)
		return 9;
	return 0;
}

void Scene1050::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_largeOverlayLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene1050::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(kScene1050FirstAmbientSoundCue,
		kScene1050AmbientSoundCueCount, 15, kScene1050AmbientSoundProbabilityModulus,
		kScene1050FirstAmbientMusicCue, kScene1050AmbientMusicCueCount, 100,
		kScene1050AmbientMusicProbabilityModulus);
}

void Scene1050::runDoorBackToGorillaRoomAction() {
	runOverlaySequence(9, kScene1050DoorOverlayDescriptorCount, kScene1050DoorFrameMap,
		ARRAYSIZE(kScene1050DoorFrameMap), kScene1050FrameMillis);
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene1050ExitState1040FromDoor;
}

void Scene1050::runCloakroomAttendantConversation() {
	Common::Array<DialogueChoiceRecord> records;
	initializeDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	GameplayState &state = _vm->gameState();
	const bool firstConversation = !state.scene1050CloakroomAttendantConversationSeen;

	beginSecondarySpeechLine(kScene1050DialogueStageId, firstConversation ? 0 : 1);
	beginCloakroomAttendantSpeechLine(firstConversation ? 0 : 1, _random.getRandomBit() != 0);
	state.scene1050CloakroomAttendantConversationSeen = true;
	applySceneStateToHotspotsAndPatches(1);

	while (!finished && !Engine::shouldQuit()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene1050DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene1050DialogueStageId, 7);
			beginCloakroomAttendantSpeechLine(7, _random.getRandomBit() != 0);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene1050DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0 && record.responseFrameIndex != 0xff)
			beginCloakroomAttendantSpeechLine(record.responseFrameIndex, _random.getRandomBit() != 0);

		handleDialogueEffect(record.disableAfterUse);
		if (record.disableAfterUse != 0)
			record.enabled = 0;

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

void Scene1050::initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene1050DialogueChoiceRecordCount);

	records[0].enabled = 1;
	records[0].transitionMode = 1;
	records[0].playerTextRowId = 2;
	records[0].responseFrameIndex = 2;
	records[0].disableAfterUse = 1;
	records[0].reserved = 0xff;

	records[1].enabled = 1;
	records[1].transitionMode = 3;
	records[1].playerTextRowId = 3;
	records[1].responseFrameIndex = 3;
	records[1].disableAfterUse = 1;
	records[1].reserved = 0xff;

	records[2].enabled = 1;
	records[2].transitionMode = 3;
	records[2].playerTextRowId = 4;
	records[2].responseFrameIndex = 4;
	records[2].disableAfterUse = 1;
	records[2].reserved = 0xff;

	records[3].enabled = 1;
	records[3].transitionMode = 3;
	records[3].playerTextRowId = 5;
	records[3].responseFrameIndex = 5;
	records[3].disableAfterUse = 1;
	records[3].reserved = 0xff;

	records[4].enabled = 1;
	records[4].nextNodeIndex = 1;
	records[4].transitionMode = 1;
	records[4].playerTextRowId = 6;
	records[4].responseFrameIndex = 6;
	records[4].disableAfterUse = 1;
	records[4].reserved = 0xff;

	records[5].enabled = 1;
	records[5].transitionMode = 0;
	records[5].playerTextRowId = 7;
	records[5].responseFrameIndex = 7;
	records[5].disableAfterUse = 1;
	records[5].reserved = 0xff;

	records[70].enabled = 1;
	records[70].transitionMode = 1;
	records[70].playerTextRowId = 8;
	records[70].responseFrameIndex = 8;
	records[70].disableAfterUse = 10;
	records[70].reserved = 0xff;

	records[77].enabled = 1;
	records[77].nextNodeIndex = 1;
	records[77].transitionMode = 3;
	records[77].playerTextRowId = 9;
	records[77].responseFrameIndex = 9;
	records[77].disableAfterUse = 4;
	records[77].reserved = 0xff;

	records[78].enabled = 1;
	records[78].nextNodeIndex = 1;
	records[78].transitionMode = 3;
	records[78].playerTextRowId = 10;
	records[78].responseFrameIndex = 10;
	records[78].disableAfterUse = 3;
	records[78].reserved = 0xff;

	records[79].enabled = 1;
	records[79].nextNodeIndex = 1;
	records[79].transitionMode = 3;
	records[79].playerTextRowId = 11;
	records[79].responseFrameIndex = 11;
	records[79].disableAfterUse = 5;
	records[79].reserved = 0xff;

	records[80].enabled = 1;
	records[80].nextNodeIndex = 1;
	records[80].transitionMode = 3;
	records[80].playerTextRowId = 12;
	records[80].responseFrameIndex = 12;
	records[80].disableAfterUse = 2;
	records[80].reserved = 0xff;

	records[81].enabled = 1;
	records[81].nextNodeIndex = 1;
	records[81].transitionMode = 3;
	records[81].playerTextRowId = 13;
	records[81].responseFrameIndex = 13;
	records[81].disableAfterUse = 1;
	records[81].reserved = 0xff;

	records[82].enabled = 1;
	records[82].nextNodeIndex = 1;
	records[82].transitionMode = 3;
	records[82].playerTextRowId = 14;
	records[82].responseFrameIndex = 14;
	records[82].disableAfterUse = 1;
	records[82].reserved = 0xff;

	records[83].enabled = 1;
	records[83].transitionMode = 2;
	records[83].playerTextRowId = 15;
	records[83].responseFrameIndex = 15;
	records[83].disableAfterUse = 1;
	records[83].reserved = 0xff;

	records[140].enabled = 1;
	records[140].transitionMode = 4;
	records[140].playerTextRowId = 16;
	records[140].responseFrameIndex = 16;
	records[140].disableAfterUse = 1;
	records[140].reserved = 0xff;
}

void Scene1050::beginCloakroomAttendantSpeechLine(byte frameIndex, bool alternatePose) {
	const byte group = alternatePose ? kScene1050PrimarySpeechAltGroup : kScene1050PrimarySpeechNormalGroup;
	if (alternatePose)
		runLargeOverlayPoseTransition(1, 5);

	beginPrimarySpeechLineWithAnimationGroup(kScene1050DialoguePrimaryRow, frameIndex,
		alternatePose ? kScene1050DialoguePrimaryAltCenterX : kScene1050DialoguePrimaryCenterX,
		alternatePose ? kScene1050DialoguePrimaryAltTopY : kScene1050DialoguePrimaryTopY,
		kScene1050DialoguePrimaryRed, kScene1050DialoguePrimaryGreen,
		kScene1050DialoguePrimaryBlue, group);

	if (alternatePose)
		runLargeOverlayPoseTransition(2, 0x0e);
	else {
		_largeOverlayLayer.setFrame(0);
		_largeOverlayMode = 0;
	}
}

void Scene1050::handleDialogueEffect(byte effectId) {
	GameplayState &state = _vm->gameState();
	switch (effectId) {
	case 2:
		if (!state.hasTravelScreenDestination(1)) {
			runTravelUnlockEffect(1);
			state.unlockTravelScreenDestination(1);
		}
		break;
	case 3:
		if (!state.hasTravelScreenDestination(2)) {
			runTravelUnlockEffect(2);
			state.unlockTravelScreenDestination(2);
		}
		break;
	case 4:
		if (!state.hasTravelScreenDestination(3)) {
			runTravelUnlockEffect(3);
			state.unlockTravelScreenDestination(3);
		}
		break;
	case 5:
		state.scene1050CharlieBogWerewolfClueHeard = true;
		break;
	case 10:
		runDialogueEffectTen();
		break;
	default:
		break;
	}
}

void Scene1050::runDialogueEffectTen() {
	_largeOverlayMode = 6;
	_largeOverlayLayer.setFrame(0x41);
	_largeOverlayChannel.resetTimer();
	_soundBank0.playSample(0x0b, 100);

	while (_largeOverlayMode == 6 && !Engine::shouldQuit()) {
		if (waitSceneMillis(kScene1050FrameMillis))
			break;
	}

	_largeOverlayMode = 0;
	_largeOverlayLayer.setFrame(0);
	waitSceneMillis(1000);
}

void Scene1050::runTravelUnlockEffect(byte travelSlotId) {
	(void)travelSlotId;
	beginStaticSecondarySpeechLine(0xdb, 0);
	_soundBank0.playSampleLooping(0x32, 25);
	runOverlaySequence(13, kScene1050TravelOverlayDescriptorCount, kScene1050TravelFrameMap,
		ARRAYSIZE(kScene1050TravelFrameMap), kScene1050FrameMillis);
	_soundBank0.stop();
	_soundBank0.playSample(1, 100);
}

void Scene1050::handleJacketExchange() {
	finishLargeOverlayIdleSequence();
	runSynchronizedOverlaySequence(10, kScene1050JacketOverlayDescriptorCount,
		kScene1050JacketFirstFrameMap, kScene1050JacketFirstLargeOverlayFrameMap,
		ARRAYSIZE(kScene1050JacketFirstFrameMap), kScene1050FrameMillis);
	beginPrimarySpeechLine(kScene1050DialoguePrimaryRow, 6, kScene1050DialoguePrimaryCenterX,
		kScene1050DialoguePrimaryTopY, kScene1050DialoguePrimaryRed,
		kScene1050DialoguePrimaryGreen, kScene1050DialoguePrimaryBlue);
	runSynchronizedOverlaySequence(10, kScene1050JacketOverlayDescriptorCount,
		kScene1050JacketSecondFrameMap, kScene1050JacketSecondLargeOverlayFrameMap,
		ARRAYSIZE(kScene1050JacketSecondFrameMap), kScene1050FrameMillis);
	removeInventoryItem(0x22);
	addInventoryItem(0x1d);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(6, 2);
	_largeOverlayLayer.setFrame(0);
	_largeOverlayMode = 0;
}

void Scene1050::handleJackTalkLine() {
	if (!_vm->gameState().scene1050JackLookedAt)
		handleJackLookLine();
	beginSecondarySpeechLine(7, 0);
}

void Scene1050::handleJackLookLine() {
	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(8, state.scene1050JackLookedAt ? 1 : 0);
	state.scene1050JackLookedAt = true;
}

void Scene1050::handleSuitcasePickup() {
	if (hasInventoryItem(0x19))
		return;

	runOverlaySequence(12, kScene1050SuitcaseOverlayDescriptorCount, kScene1050SuitcaseFrameMap,
		ARRAYSIZE(kScene1050SuitcaseFrameMap), kScene1050FrameMillis);
	_vm->gameState().scene1050SuitcaseTaken = true;
	applySceneStateToHotspotsAndPatches(0);
	addInventoryItem(0x19);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(9, 0);
}

void Scene1050::runLargeOverlayPoseTransition(byte mode, byte startFrame) {
	_largeOverlayMode = mode;
	_largeOverlayLayer.setFrame(startFrame);
	_largeOverlayChannel.resetTimer();

	while (_largeOverlayMode == mode && !Engine::shouldQuit()) {
		if (waitSceneMillis(kScene1050FrameMillis))
			break;
	}

	if (_largeOverlayMode == mode) {
		_largeOverlayLayer.setFrame(mode == 1 ? 9 : 0);
		_largeOverlayMode = 0;
	}
}

void Scene1050::finishLargeOverlayIdleSequence() {
	if (_largeOverlayMode < 3 || _largeOverlayMode > 5)
		return;

	while (_largeOverlayMode >= 3 && _largeOverlayMode <= 5 && !Engine::shouldQuit()) {
		advanceSmallOverlay(kScene1050FrameMillis);
		advanceLargeOverlay(kScene1050FrameMillis, true);
		drawPlayableComposite();
		presentFrame();
		if (waitSceneMillis(kScene1050FrameMillis))
			break;
	}
}

void Scene1050::runSynchronizedOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *actionFrameMap,
		const byte *largeOverlayFrameMap, uint frameMapSize, uint32 frameMillis) {
	if (frameMapSize == 0)
		return;

	const bool previousLargeOverlayActionLocked = _largeOverlayActionLocked;
	_largeOverlayActionLocked = true;
	_largeOverlayMode = 0;
	const bool previousHideActiveActor = _actionOverlayPlayer.beginActorReplacement(chunkIndex,
		descriptorCount, actionFrameMap, frameMapSize);

	for (uint frame = 0; frame < frameMapSize && !Engine::shouldQuit(); ++frame) {
		_actionOverlayPlayer.setFrame(frame);
		_largeOverlayLayer.setFrame(largeOverlayFrameMap[frame]);
		if (waitSceneMillis(frameMillis))
			break;
	}

	_actionOverlayPlayer.finish(previousHideActiveActor);
	_largeOverlayActionLocked = previousLargeOverlayActionLocked;
}

void Scene1050::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame) {
	runActorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount,
		frameMap, frameMapSize, frameMillis)
		.patchAt(patchFrame, 0xff));
}

void Scene1050::advanceSmallOverlay(uint32 delta) {
	const uint frameCount = _smallOverlayChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_smallOverlayLayer.frameIndex < 0x18) {
			if (_smallOverlayLayer.frameIndex == 0x0f)
				_soundBank0.playSample((byte)(0x0f + _random.getRandomNumber(2)), 30);
			_smallOverlayLayer.setFrame(_smallOverlayLayer.frameIndex + 1);
		} else {
			_smallOverlayLayer.setFrame(0);
		}
	}
}

void Scene1050::advanceLargeOverlay(uint32 delta) {
	advanceLargeOverlay(delta, false);
}

void Scene1050::advanceLargeOverlay(uint32 delta, bool forceFinish) {
	const uint frameCount = _largeOverlayChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_largeOverlayMode == 0) {
			if (_largeOverlayLayer.frameIndex == 0) {
				if (_random.getRandomNumber(14) == 0) {
					_largeOverlayLayer.setFrame(4);
				} else if (_random.getRandomNumber(19) == 0) {
					_largeOverlayMode = 3;
					_largeOverlayLayer.setFrame(0x11);
				}
			} else {
				_largeOverlayLayer.setFrame(0);
			}
		} else if (_largeOverlayMode == 3) {
			if (_largeOverlayLayer.frameIndex < 0x1a) {
				_largeOverlayLayer.setFrame(_largeOverlayLayer.frameIndex + 1);
			} else {
				_largeOverlayMode = 4;
			}
		} else if (_largeOverlayMode == 4) {
			if (_largeOverlayLayer.frameIndex < 0x23) {
				_largeOverlayLayer.setFrame(_largeOverlayLayer.frameIndex + 1);
			} else if (forceFinish || _random.getRandomNumber(5) == 0) {
				_largeOverlayLayer.setFrame(0x24);
				_largeOverlayMode = 5;
			} else {
				_largeOverlayLayer.setFrame(0x1d);
			}
		} else if (_largeOverlayMode == 5) {
			if (_largeOverlayLayer.frameIndex <= 0x2c) {
				_largeOverlayLayer.setFrame(_largeOverlayLayer.frameIndex + 1);
			} else {
				_largeOverlayLayer.setFrame(0);
				_largeOverlayMode = 0;
			}
		} else if (_largeOverlayMode == 1) {
			if (_largeOverlayLayer.frameIndex < 8 && !_primaryDialogueSpeechActive) {
				_largeOverlayLayer.setFrame(_largeOverlayLayer.frameIndex + 1);
			} else {
				_largeOverlayLayer.setFrame(9);
				_largeOverlayMode = 0;
			}
		} else if (_largeOverlayMode == 2) {
			if (_largeOverlayLayer.frameIndex > 0x10 || _primaryDialogueSpeechActive) {
				_largeOverlayLayer.setFrame(0);
				_largeOverlayMode = 0;
			} else {
				_largeOverlayLayer.setFrame(_largeOverlayLayer.frameIndex + 1);
			}
		} else if (_largeOverlayMode == 6) {
			if (_largeOverlayLayer.frameIndex > 100) {
				_largeOverlayLayer.setFrame(0);
				_largeOverlayMode = 0;
			} else {
				if (_largeOverlayLayer.frameIndex == 0x59)
					_soundBank0.playSample(0x0e, 100);
				_largeOverlayLayer.setFrame(_largeOverlayLayer.frameIndex + 1);
			}
		}
	}
}

void Scene1050::copyStageSmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

} // End of namespace Hollywood
