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

#include "hollywood/scenes/playable/scene5110.h"

#include "engines/engine.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene5100ReturnState = 0x13ed;
const uint16 kScene5120FirstState = 0x1400;
const uint16 kScene5110ViewportXOffset = 0x0068;
const uint kScene5110ActorBankTableEntry = 0x0000;
const uint kScene5110ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5110SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5110FrameMillis = 75;
const uint kScene5110PickupDescriptorCount = 0x0c;
const uint kScene5110WerewolfDialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene5110UnderwearInventoryItem = 0x53;
const byte kScene5110BottleInventoryItem = 0x54;
const byte kScene5110MirrorInventoryItem = 0x55;
const byte kScene5110UnderwearSceneItem = 5;
const byte kScene5110BottleSceneItem = 6;
const byte kScene5110MirrorSceneItem = 12;
const byte kScene5110DialogueStageId = 0x62;
const byte kScene5110SalonResponseRow = 99;
const byte kScene5110DialogueNoResponseFrame = 0xff;

enum {
	kScene5110DialogueTransitionEnd = 0,
	kScene5110DialogueTransitionDown = 1,
	kScene5110DialogueTransitionUp = 2,
	kScene5110DialogueTransitionStay = 3,
	kScene5110DialogueTransitionUpTwo = 4
};

enum {
	kScene5110PatchUnderwear = 1,
	kScene5110PatchBottle = 2,
	kScene5110PatchMirror = 3
};

const byte kScene5110PickupFrameMap[] = {
	11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

static PlayableSceneConfig scene5110Config() {
	PlayableSceneConfig config(5110,
		SceneResourceLayout(5, 5, 0x19),
		SceneViewport(kScene5110ViewportXOffset, kScene5110ViewportXOffset, kScene5110ViewportXOffset),
		SceneActorPose(0x0b4, 0x174, 5));
	config.setActorResources(kScene5110ActorBankTableEntry, kScene5110ActorPaletteTableEntry);
	config.setTextResources(0, kScene5110SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	return config;
}

Scene5110::Scene5110(HollywoodEngine *vm) :
		PlayableScene(vm, scene5110Config()) {
}

void Scene5110::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);

	setActiveActorPose(0x0b4, 0x174, 5);
}

void Scene5110::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawStaticRoomLayers(activeWorldY);
	drawActionOverlayLayer();
}

void Scene5110::runCustomEntrySequence() {
	setActiveActorPose(0x084, 0x169, 2);
	drawPlayableComposite();
	presentFrame();

	runEntryElevatorSequence();
	walkActiveActorTo(0x0b4, 0x174, 0xff, 0, false);

	GameplayState &state = _vm->gameState();
	if (!state.scene5110IntroSeen && state.scene5110SalonTransformState < 2) {
		beginSalonPrimarySpeechLine(0, 0);
		beginSecondarySpeechLine(0, 1);
		beginSalonPrimarySpeechLine(0, 2);
		state.scene5110IntroSeen = true;
	}
}

bool Scene5110::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene5110::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar ascensor (look at elevator).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Usar boton superior (use upper button): exits to scene 5120.
		runButtonExitToState(kScene5120FirstState);
		return true;
	case 303: // Usar boton central (use middle button): elevator returns to this room.
		runButtonReturnSequence();
		return true;
	case 304: // Usar boton inferior (use lower button): exits back to scene 5100.
		runButtonExitToState(kScene5100ReturnState);
		return true;
	case 305: // Coger calzoncillo (take underwear): grants item 0x53.
		runUnderwearPickup();
		return true;
	case 306: // Mirar calzoncillo (look at underwear).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 307: // Coger frasco (take bottle): grants item 0x54 after the salon is transformed.
		runBottlePickup();
		return true;
	case 308: // Mirar frasco (look at bottle): identifies it before pickup.
		if (_vm->gameState().scene5110BottleState == 0) {
			beginSecondarySpeechLine(4, 0);
			_vm->gameState().scene5110BottleState = 1;
			applySceneStateToHotspotsAndPatches(2);
		} else {
			beginSecondarySpeechLine(4, 1);
		}
		return true;
	case 309: // Hablar con hombre lobo (talk to werewolf): opens the werewolf dialogue.
		runWerewolfDialogue();
		return true;
	case 310: // Mirar hombre lobo (look at werewolf).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 311: // Hablar con chica haciendo la manicura (talk to manicure girl).
		runManicureGirlTalk();
		return true;
	case 312: // Mirar chica haciendo la manicura (look at manicure girl).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 313: // Hablar con chica del jacuzzi (talk to jacuzzi girl).
		runJacuzziGirlTalk();
		return true;
	case 314: // Mirar chica del jacuzzi (look at jacuzzi girl).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 315: // Ir a tocador/espejo (go to dressing table/mirror area).
		runMirrorApproach();
		return true;
	case 316: // Mirar jacuzzi (look at jacuzzi).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 317: // Coger espejo (take mirror): grants item 0x55 after the salon is transformed.
		runMirrorPickup();
		return true;
	case 318: // Mirar/usar espejo (look/use mirror).
		beginSecondarySpeechLine(13, 0);
		return true;
	case 319: // Mirar jacuzzi despues del dialogo (look at jacuzzi after dialogue state change).
		runJacuzziInspection();
		return true;
	case 320: // Mirar tratamiento capilar, primera zona (look at hair treatment, first area).
		beginSecondarySpeechLine(16, 0);
		return true;
	case 321: // Mirar tratamiento capilar, segunda zona (look at hair treatment, second area).
		beginSecondarySpeechLine(17, 0);
		return true;
	case 322: // Mirar tratamiento capilar, tercera zona (look at hair treatment, third area).
		beginSecondarySpeechLine(18, 0);
		return true;
	case 323: // Mirar tratamiento capilar, cuarta zona (look at hair treatment, fourth area).
		beginSecondarySpeechLine(15, 0);
		return true;
	case 324: // Hablar con tratamiento capilar (talk to hair treatment customer).
		runHairTreatmentTalk();
		return true;
	default:
		return false;
	}
}

bool Scene5110::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x084, 0x34f);

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene5110::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	GameplayState &state = _vm->gameState();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	if (state.scene5110UnderwearTaken || hasInventoryItem(kScene5110UnderwearInventoryItem)) {
		state.scene5110UnderwearTaken = true;
		if (_sceneChunkTable.isValidChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5110UnderwearSceneItem);
	}

	if (state.scene5110BottleState == 2 || hasInventoryItem(kScene5110BottleInventoryItem)) {
		state.scene5110BottleState = 2;
		if (_sceneChunkTable.isValidChunk(5))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5110BottleSceneItem);
	}

	if (state.scene5110MirrorTaken || hasInventoryItem(kScene5110MirrorInventoryItem)) {
		state.scene5110MirrorTaken = true;
		if (_sceneChunkTable.isValidChunk(13))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5110MirrorSceneItem);
	}

	if (state.scene5110SalonTransformState < 2) {
		// The original aliases several transformed-salon labels back to
		// "chica del jacuzzi" until the later hair-treatment state is reached.
		replaceColorMapItemFromOriginal(14, 9);
		replaceColorMapItemFromOriginal(15, 9);
		replaceColorMapItemFromOriginal(16, 9);
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

bool Scene5110::shouldConvertSavedFramebufferFF() const {
	return true;
}

AmbientAudioProfile Scene5110::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 75, 25, 0x0b, 5, 100, 50);
}

void Scene5110::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	switch (hookId) {
	case kScene5110PatchUnderwear:
		if (_sceneChunkTable.isValidChunk(11))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[11], _baseFramebuffer);
		break;
	case kScene5110PatchBottle:
		if (_sceneChunkTable.isValidChunk(5))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
		break;
	case kScene5110PatchMirror:
		if (_sceneChunkTable.isValidChunk(13))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);
		break;
	default:
		break;
	}
}

void Scene5110::runEntryElevatorSequence() {
	_soundBank0.playSample(0x1d, 100);
	waitSceneMillis(kScene5110FrameMillis * 5);
	if (_sceneChunkTable.isValidChunk(23))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[23], _baseFramebuffer);
	_soundBank0.playSample(0x1d, 100);
	waitSceneMillis(kScene5110FrameMillis * 5);
}

void Scene5110::runButtonExitToState(uint16 nextState) {
	_soundBank0.playSample(0x1d, 100);
	waitSceneMillis(kScene5110FrameMillis * 5);
	if (_sceneChunkTable.isValidChunk(24))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[24], _baseFramebuffer);
	_soundBank0.playSample(0x1e, 100);
	_vm->gameState().scene5110ElevatorTransitionSeen = true;
	_vm->gameState().mainFlowStateId = nextState;
}

void Scene5110::runButtonReturnSequence() {
	_soundBank0.playSample(0x1d, 100);
	waitSceneMillis(kScene5110FrameMillis * 5);
	_vm->gameState().scene5110ElevatorTransitionSeen = true;
	applySceneStateToHotspotsAndPatches(4);
	walkActiveActorTo(0x0b4, 0x174, 0xff, 0, false);
}

void Scene5110::runUnderwearPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5110UnderwearTaken || hasInventoryItem(kScene5110UnderwearInventoryItem)) {
		beginSecondarySpeechLine(2, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(12, kScene5110PickupDescriptorCount,
		kScene5110PickupFrameMap, ARRAYSIZE(kScene5110PickupFrameMap), kScene5110FrameMillis)
		.hookAt(6, kScene5110PatchUnderwear));
	state.scene5110UnderwearTaken = true;
	applySceneStateToHotspotsAndPatches(1);
	addInventoryItem(kScene5110UnderwearInventoryItem);
	_soundBank0.playSample(1, 100);
}

void Scene5110::runBottlePickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5110BottleState == 2 || hasInventoryItem(kScene5110BottleInventoryItem)) {
		beginSecondarySpeechLine(3, 2);
		return;
	}
	if (state.scene5110SalonTransformState < 2) {
		beginSecondarySpeechLine(3, 0);
		return;
	}
	if (state.scene5110BottleState == 0) {
		beginSecondarySpeechLine(4, 0);
		state.scene5110BottleState = 1;
		applySceneStateToHotspotsAndPatches(2);
	}

	beginSecondarySpeechLine(3, 1);
	runActorReplacement(ActionOverlaySpec(6, kScene5110PickupDescriptorCount,
		kScene5110PickupFrameMap, ARRAYSIZE(kScene5110PickupFrameMap), kScene5110FrameMillis)
		.hookAt(6, kScene5110PatchBottle));
	state.scene5110BottleState = 2;
	applySceneStateToHotspotsAndPatches(2);
	addInventoryItem(kScene5110BottleInventoryItem);
	_soundBank0.playSample(1, 100);
}

void Scene5110::runWerewolfDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializeWerewolfDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	if (!state.scene5110IntroSeen) {
		beginSecondarySpeechLine(kScene5110DialogueStageId, 0);
		beginSalonPrimarySpeechLine(kScene5110SalonResponseRow, 0);
		state.scene5110IntroSeen = true;
	} else {
		beginSecondarySpeechLine(kScene5110DialogueStageId, 1);
		beginSalonPrimarySpeechLine(kScene5110SalonResponseRow, 1);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene5110DialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene5110DialogueStageId, 7);
			beginSalonPrimarySpeechLine(kScene5110SalonResponseRow, 7);
			return;
		}

		const uint recordIndex = ((uint)nodeIndex + (uint)depthIndex * 10) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			return;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene5110DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene5110DialogueNoResponseFrame)
			beginSalonPrimarySpeechLine(kScene5110SalonResponseRow, record.responseFrameIndex);

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}
		if (record.disableAfterUse == 2 && state.scene5110JacuzziInspectionState == 0)
			state.scene5110JacuzziInspectionState = 1;

		if (applyWerewolfDialogueTransition(record, depthIndex, nodeIndex))
			return;
	}
}

void Scene5110::runManicureGirlTalk() {
	beginSecondarySpeechLine(6, 0);
	beginSalonPrimarySpeechLine(6, 1);
}

void Scene5110::runJacuzziGirlTalk() {
	beginSecondarySpeechLine(6, 0);
	beginSalonPrimarySpeechLine(8, 0);
}

void Scene5110::runMirrorApproach() {
	if (_vm->gameState().scene5110SalonTransformState < 2) {
		beginSecondarySpeechLine(10, 0);
		return;
	}

	walkActiveActorTo(0x30d, 0x0f5, 0xff, 0, false);
	beginSecondarySpeechLine(10, 1);
	walkActiveActorTo(0x258, 0x101, 0xff, 0, false);
	beginSecondarySpeechLine(10, 2);
}

void Scene5110::runMirrorPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5110MirrorTaken || hasInventoryItem(kScene5110MirrorInventoryItem)) {
		beginSecondarySpeechLine(13, 0);
		return;
	}
	if (state.scene5110SalonTransformState < 2) {
		beginSecondarySpeechLine(12, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(14, kScene5110PickupDescriptorCount,
		kScene5110PickupFrameMap, ARRAYSIZE(kScene5110PickupFrameMap), kScene5110FrameMillis)
		.hookAt(6, kScene5110PatchMirror));
	state.scene5110MirrorTaken = true;
	applySceneStateToHotspotsAndPatches(5);
	addInventoryItem(kScene5110MirrorInventoryItem);
	_soundBank0.playSample(1, 100);
}

void Scene5110::runJacuzziInspection() {
	GameplayState &state = _vm->gameState();
	if (state.scene5110JacuzziInspectionState == 0) {
		runJacuzziGirlTalk();
		return;
	}
	if (state.scene5110JacuzziInspectionState == 1) {
		beginSecondarySpeechLine(14, 0);
		beginSalonPrimarySpeechLine(14, 1);
		state.scene5110JacuzziInspectionState = 2;
		return;
	}
	beginSecondarySpeechLine(14, 2);
}

void Scene5110::runHairTreatmentTalk() {
	beginSecondarySpeechLine(6, 0);
	beginSalonPrimarySpeechLine(19, (byte)_random.getRandomNumber(1));
}

void Scene5110::beginSalonPrimarySpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLine(rowIndex, frameIndex, 599, 0x00b4, 0x17, 0x33, 0x2c);
}

void Scene5110::initializeWerewolfDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene5110WerewolfDialogueChoiceRecordCount);

	// DAT_0050a5c8: root choices for the werewolf in the spa.
	setWerewolfDialogueRecord(records, 0, 1, 0, kScene5110DialogueTransitionDown, 2, 2, 1);
	setWerewolfDialogueRecord(records, 1, 1, 0, kScene5110DialogueTransitionStay, 3, 3, 1);
	setWerewolfDialogueRecord(records, 2, 1, 0, kScene5110DialogueTransitionStay, 4, 4, 1);
	setWerewolfDialogueRecord(records, 3, 1, 0, kScene5110DialogueTransitionStay, 5, 5, 1);
	setWerewolfDialogueRecord(records, 4,
		_vm->gameState().scene5110WerewolfDialogueChoiceUnlocked ? 1 : 0,
		0, kScene5110DialogueTransitionStay, 6, 6, 2);
	setWerewolfDialogueRecord(records, 5, 1, 0, kScene5110DialogueTransitionEnd, 7, 7, 0);

	// Depth 1, node 0: follow-ups after asking why the werewolf is here.
	setWerewolfDialogueRecord(records, 70, 1, 0, kScene5110DialogueTransitionStay, 8, 8, 1);
	setWerewolfDialogueRecord(records, 71, 1, 0, kScene5110DialogueTransitionStay, 9, 9, 1);
	setWerewolfDialogueRecord(records, 72, 1, 0, kScene5110DialogueTransitionStay, 10, 10, 1);
	setWerewolfDialogueRecord(records, 73, 1, 0, kScene5110DialogueTransitionUp, 11, 11, 0);
}

void Scene5110::setWerewolfDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
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
	record.selectable = 1;
}

bool Scene5110::applyWerewolfDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) {
	const byte previousDepth = depthIndex;
	switch (record.transitionMode) {
	case kScene5110DialogueTransitionEnd:
		return true;
	case kScene5110DialogueTransitionDown:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth + 1;
		break;
	case kScene5110DialogueTransitionUp:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth == 0 ? 0 : (byte)(previousDepth - 1);
		break;
	case kScene5110DialogueTransitionUpTwo:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
		break;
	case kScene5110DialogueTransitionStay:
	default:
		break;
	}

	return false;
}

void Scene5110::drawStaticRoomLayers(int activeWorldY) {
	const GameplayState &state = _vm->gameState();
	drawSpriteFrame(7, 4, 0);

	if (state.scene5110SalonTransformState < 2) {
		drawSpriteFrame(16, 8, 0);
		drawSpriteFrame(19, 1, 0);
		drawSpriteFrame(8, 5, 1);
		drawSpriteFrame(17, 7, 0);
		drawSpriteFrame(9, 10, 0);
		drawSpriteFrame(10, 5, 0);
		if (_sceneChunkTable.isValidChunk(20))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[20], _sceneFramebuffer);
		if (activeWorldY > 0x109 && _sceneChunkTable.isValidChunk(21))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[21], _sceneFramebuffer);
		drawSpriteFrame(15, 9, 0);
		drawSpriteFrame(18, 7, 0);
	} else {
		if (_sceneChunkTable.isValidChunk(20))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[20], _sceneFramebuffer);
		if (_sceneChunkTable.isValidChunk(25))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[25], _sceneFramebuffer);
		if (activeWorldY > 0x109 && _sceneChunkTable.isValidChunk(21))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[21], _sceneFramebuffer);
	}
}

void Scene5110::drawSpriteFrame(uint chunkIndex, uint descriptorCount, byte frameIndex) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return;

	ResourceSpriteLayer layer;
	layer.visible = true;
	layer.chunkIndex = chunkIndex;
	layer.descriptorCount = descriptorCount;
	layer.frameIndex = frameIndex;
	drawResourceSpriteLayer(layer);
}

void Scene5110::clearSceneItemFromColorMap(byte itemId) {
	replaceColorMapItemFromOriginal(itemId, 0);
}

void Scene5110::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + color] == sourceItem)
			_paletteMask[kSceneColorToItemMap + color] = destinationItem;
	}
}

void Scene5110::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 20)
			_walkablePaletteMask[i] = 0;
	}
}

} // End of namespace Hollywood
