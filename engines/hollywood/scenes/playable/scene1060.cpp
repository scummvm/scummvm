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

#include "hollywood/scenes/playable/scene1060.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene1060ArchiveName = "RESOURCE.A06";
const char *const kScene1060MusicArchiveName = "RESOURCE.M01";
const char *const kScene1060SoundArchiveName = "RESOURCE.S01";
const uint kScene1060InitialRequiredChunkCount = 15;
const uint kScene1060ArenaFirstChunk = 5;
const uint kScene1060ArenaLastChunk = 14;
const uint kScene1060StageIndex = 106;
const uint16 kScene1060FirstState = 0x0424;
const uint16 kScene1060LastState = 0x042d;
const uint16 kScene1060ExitState1040 = 0x0410;
const uint16 kScene1060ExitState1070 = 0x042e;
const uint16 kScene1060ViewportXOffset = 0x0068;
const uint16 kScene1060ViewportMinXOffset = 0x0068;
const uint16 kScene1060ViewportMaxXOffset = 0x0078;
const uint kScene1060ActorBankTableEntry = 0x0000;
const uint kScene1060ActorPaletteTableEntry = 0x00cc;
const uint kScene1060Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1060SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene1060FrameMillis = 75;
const uint32 kScene1060SlowFrameMillis = 150;
const uint kScene1060LargeBackgroundDescriptorCount = 0x27;
const uint kScene1060InvisibleManDescriptorCount = 0x16;
const uint kScene1060FlyDoctorDescriptorCount = 0x0e;
const uint kScene1060SmallLoopDescriptorCount = 5;
const uint kScene1060SmallTriggerDescriptorCount = 7;
const uint kScene1060SkullcrackerDescriptorCount = 0x0e;
const uint kScene1060PocketPaperDescriptorCount = 0x1e;
const byte kScene1060DoctorSpeechGroup = 1;
const byte kScene1060InvisibleManSpeechGroup = 2;
const byte kScene1060FirstAmbientMusicCue = 0x0b;
const byte kScene1060AmbientMusicCueCount = 5;
const byte kScene1060AmbientMusicProbabilityModulus = 50;

const byte kScene1060ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene1060LargeBackgroundFrameMap[] = {
	0, 1, 2, 3, 3, 3, 3, 4, 2, 1, 0, 0, 1, 2, 3, 5,
	6, 7, 7, 8, 9, 10, 11, 7, 12, 13, 14, 15, 16, 17, 32, 31,
	30, 29, 12, 7, 38, 37, 20, 19, 18, 18, 19, 20, 21, 21, 21, 21,
	22, 23, 24, 25, 26, 27, 28, 36, 35, 34, 33, 25, 26, 27, 28, 36,
	35, 34, 33, 25, 24, 23, 22, 21, 21, 21, 21, 20, 19, 18, 19, 20,
	21, 37, 38, 7
};

const byte kScene1060InvisibleManFrameMap[] = {
	13, 14, 15, 16, 17, 18, 19, 20, 21, 0,
	0, 1, 2, 3, 21, 20, 19, 18, 17, 13
};

const byte kScene1060FlyDoctorFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 8, 9,
	10, 11, 12, 13, 8, 9, 10, 11, 12, 13, 8, 9, 10, 11, 12, 13,
	7, 6, 5, 4, 0
};

const byte kScene1060SmallTriggerFrameMap[] = { 0, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2 };

const byte kScene1060SkullcrackerFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

const byte kScene1060PocketPaperFrameMap[] = {
	13, 13, 13, 13, 13, 13, 13, 13, 13, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 20, 20, 21, 22, 23, 24, 25, 26, 27,
	27, 27, 27, 27, 27, 27, 28, 29, 29, 29, 29, 29, 29, 29
};

Scene1060::Scene1060(HollywoodEngine *vm) :
		PlayableScene(vm, "scene1060", 0x0aa, 0x1b3, 2, 0xfd, 0xfb),
		_largeBackgroundChannel(),
		_invisibleManChannel(),
		_flyDoctorChannel(),
		_smallLoopChannel(),
		_smallTriggerChannel(),
		_largeBackgroundLayer(),
		_invisibleManLayer(),
		_flyDoctorLayer(),
		_smallLoopLayer(),
		_smallTriggerLayer(),
		_largeBackgroundMode(0),
		_largeBackgroundIdleCounter(0),
		_smallTriggerMode(0) {
	_largeBackgroundLayer.configure(9, kScene1060LargeBackgroundDescriptorCount,
		kScene1060LargeBackgroundFrameMap, ARRAYSIZE(kScene1060LargeBackgroundFrameMap));
	_invisibleManLayer.configure(5, kScene1060InvisibleManDescriptorCount,
		kScene1060InvisibleManFrameMap, ARRAYSIZE(kScene1060InvisibleManFrameMap));
	_flyDoctorLayer.configure(6, kScene1060FlyDoctorDescriptorCount,
		kScene1060FlyDoctorFrameMap, ARRAYSIZE(kScene1060FlyDoctorFrameMap));
	_smallLoopLayer.configure(8, kScene1060SmallLoopDescriptorCount, nullptr, 0);
	_smallTriggerLayer.configure(7, kScene1060SmallTriggerDescriptorCount,
		kScene1060SmallTriggerFrameMap, ARRAYSIZE(kScene1060SmallTriggerFrameMap));
}

const char *Scene1060::resourceArchiveName() const {
	return kScene1060ArchiveName;
}

uint Scene1060::sceneInitialRequiredChunkCount() const {
	return kScene1060InitialRequiredChunkCount;
}

uint Scene1060::sceneArenaFirstChunk() const {
	return kScene1060ArenaFirstChunk;
}

uint Scene1060::sceneArenaLastChunk() const {
	return kScene1060ArenaLastChunk;
}

uint Scene1060::sceneStageIndex() const {
	return kScene1060StageIndex;
}

const char *Scene1060::sceneDebugName() const {
	return "Scene 1060";
}

uint16 Scene1060::sceneViewportXOffset() const {
	return kScene1060ViewportXOffset;
}

uint16 Scene1060::sceneViewportMinXOffset() const {
	return kScene1060ViewportMinXOffset;
}

uint16 Scene1060::sceneViewportMaxXOffset() const {
	return kScene1060ViewportMaxXOffset;
}

byte Scene1060::inventoryOwnerIndex() const {
	return 0;
}

void Scene1060::initializeInventoryOwnerState() {
	GameplayState &state = _vm->gameState();
	state.initializeRonItemResourcePages();
	if (state.inventoryItemCountByOwner[0] == 0)
		state.initializeRonInventoryItems();
	state.currentInventoryOwnerIndex = 0;
	state.activeAudioChapterIndex = 1;
}

uint Scene1060::resource000ActorBankTableEntry() const {
	return kScene1060ActorBankTableEntry;
}

uint Scene1060::resource000ActorPaletteTableEntry() const {
	return kScene1060ActorPaletteTableEntry;
}

uint32 Scene1060::inventoryActionTableExtraOffset() const {
	return 0;
}

uint Scene1060::resource003InventoryRowsOffsetIndex() const {
	return kScene1060Resource003RowsOffsetIndex;
}

uint32 Scene1060::speechCueDescriptorTableOffset() const {
	return kScene1060SpeechCueDescriptorTableOffset;
}

const byte *Scene1060::actorPathStepDeltaTable() const {
	return kScene1060ActorPathStepDeltaTable;
}

uint Scene1060::actorPathStepDeltaTableSize() const {
	return ARRAYSIZE(kScene1060ActorPathStepDeltaTable);
}

byte Scene1060::walkablePaletteMaxRegion() const {
	return 6;
}

const char *Scene1060::musicArchiveName() const {
	return kScene1060MusicArchiveName;
}

const char *Scene1060::soundBank0ArchiveName() const {
	return kScene1060SoundArchiveName;
}

bool Scene1060::isMainFlowStateInScene(uint16 stateId) const {
	return stateId >= kScene1060FirstState && stateId <= kScene1060LastState;
}

bool Scene1060::hasCustomPreviewState() const {
	return true;
}

void Scene1060::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	if (_vm->gameState().mainFlowStateId == kScene1060FirstState) {
		_activeActorWorldX = 0x0aa;
		_activeActorWorldY = 0x1b3;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x28c;
		_activeActorWorldY = 0x1b3;
		_activeActorFacing = 4;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene1060::hasCustomComposite() const {
	return true;
}

void Scene1060::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_largeBackgroundLayer);
	drawResourceSpriteLayer(_invisibleManLayer);
	drawResourceSpriteLayer(_flyDoctorLayer);
	drawResourceSpriteLayer(_smallLoopLayer);
	drawResourceSpriteLayer(_smallTriggerLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
}

bool Scene1060::hasCustomEntrySequence() const {
	return true;
}

void Scene1060::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene1060FirstState) {
		runEntryPath(0x064, 0x1b3, 2, 0x0aa, 0x1b3);
		if (!_vm->gameState().seenScene1060EntryLine) {
			beginSecondarySpeechLine(0, 0);
			_vm->gameState().seenScene1060EntryLine = true;
		}
	} else {
		runEntryPath(0x2fb, 0x1b3, 4, 0x28c, 0x1b3);
	}
	drawPlayableComposite();
	presentFrame();
}

bool Scene1060::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	_soundBank0.playSampleLooping(0x15, 5);
	return true;
}

bool Scene1060::advanceCustomGameplayLoop(uint32 delta) {
	advanceLargeBackground(delta);
	advanceInvisibleMan(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else
		advanceFlyDoctor(delta);
	advanceSmallLoop(delta);
	advanceSmallTrigger(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1060::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a salida izquierda (go to left exit).
		_vm->gameState().mainFlowStateId = kScene1060ExitState1040;
		return true;
	case 302: // Ir a pasillo derecho / Junior (go to right hall / Junior).
		_vm->gameState().mainFlowStateId = kScene1060ExitState1070;
		return true;
	case 303: // Hablar con Junior (talk to Junior).
		runJuniorConversation();
		return true;
	case 304: // Mirar Junior (look at Junior).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Hablar con Dr. Mosca (talk to Dr. Fly).
		runDrMoscaConversation();
		return true;
	case 306: // Mirar Dr. Mosca (look at Dr. Fly).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Hablar con hombre invisible (talk to invisible man).
		runInvisibleManConversation();
		return true;
	case 308: // Mirar hombre invisible (look at invisible man).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 309: // Coger babas de mosca (take fly slime).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 310: // Mirar babas de mosca (look at fly slime).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 311: // Coger papel que asoma del bolsillo (take paper sticking out of pocket).
		handlePocketPaperTakeAction();
		return true;
	case 312: // Mirar papel que asoma del bolsillo (look at paper sticking out of pocket).
		handlePocketPaperLook();
		return true;
	case 313: // Coger revienta-craneos (take skullcracker).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Mirar revienta-craneos (look at skullcracker).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 315: // Coger restos de la fiesta (take party remains).
	case 316: // Mirar restos de la fiesta (look at party remains).
		beginSecondarySpeechLine(12, 0);
		return true;
	default:
		return false;
	}
}

bool Scene1060::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	_flyDoctorLayer.chunkIndex = state.scene1060FlyDoctorState == 2 ? 14 : 6;

	if (state.scene1060FlyDoctorState == 1)
		copyStageSmallRow(10, 7);

	if (state.scene1060FlyDoctorState == 2)
		replaceColorMapItem(7, 4);

	if (state.scene1060PartyRemainsState == 1)
		replaceColorMapItem(8, 3);

	if (state.scene1060FlyDoctorState < 2 && !state.scene1060FlySlimeHotspotActive)
		replaceColorMapItem(7, 4);

	if (state.scene1060PocketPaperTaken)
		replaceColorMapItem(7, 4);

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

bool Scene1060::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene1060::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene1060InvisibleManSpeechGroup)
		return 10;
	return 0;
}

void Scene1060::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene1060InvisibleManSpeechGroup)
		_invisibleManLayer.setFrame(frameIndex);
	else
		_flyDoctorLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene1060::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = kScene1060FirstAmbientMusicCue;
	profile.musicCueCount = kScene1060AmbientMusicCueCount;
	profile.musicProbabilityModulus = kScene1060AmbientMusicProbabilityModulus;
	profile.musicVolumePercent = 100;
	return profile;
}

void Scene1060::resetAnimationLayers() {
	GameplayState &state = _vm->gameState();
	_largeBackgroundChannel.reset(0, kScene1060FrameMillis);
	_invisibleManChannel.reset(0, kScene1060SlowFrameMillis);
	_flyDoctorChannel.reset(0, kScene1060FrameMillis);
	_smallLoopChannel.reset(0, kScene1060SlowFrameMillis);
	_smallTriggerChannel.reset(0, kScene1060FrameMillis);

	_largeBackgroundLayer.reset(state.scene1060PartyRemainsState == 1 ? 0x29 : 0);
	_invisibleManLayer.reset(0);
	_flyDoctorLayer.reset(0);
	_flyDoctorLayer.chunkIndex = state.scene1060FlyDoctorState == 2 ? 14 : 6;
	_smallLoopLayer.reset(0);
	_smallTriggerLayer.reset(0);
	_largeBackgroundLayer.visible = true;
	_invisibleManLayer.visible = true;
	_flyDoctorLayer.visible = true;
	_smallLoopLayer.visible = true;
	_smallTriggerLayer.visible = true;
	_largeBackgroundMode = 0;
	_largeBackgroundIdleCounter = 0;
	_smallTriggerMode = 0;
}

void Scene1060::advanceLargeBackground(uint32 delta) {
	const uint frameCount = _largeBackgroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_largeBackgroundMode == 0) {
			if (++_largeBackgroundIdleCounter < 0x22)
				continue;
			_largeBackgroundIdleCounter = 0;
			_largeBackgroundMode = 1;
			_largeBackgroundLayer.setFrame(_vm->gameState().scene1060PartyRemainsState == 1 ? 0x29 : 0);
			_soundBank0.playSample(0x0c, 60);
		} else {
			const byte endFrame = _vm->gameState().scene1060PartyRemainsState == 1 ? 0x4c : 0x0a;
			if (_largeBackgroundLayer.frameIndex < endFrame) {
				_largeBackgroundLayer.setFrame(_largeBackgroundLayer.frameIndex + 1);
			} else {
				_largeBackgroundMode = 0;
				_largeBackgroundLayer.setFrame(0);
			}
		}
	}
}

void Scene1060::advanceInvisibleMan(uint32 delta) {
	const uint frameCount = _invisibleManChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_invisibleManLayer.frameIndex < ARRAYSIZE(kScene1060InvisibleManFrameMap) - 1)
			_invisibleManLayer.setFrame(_invisibleManLayer.frameIndex + 1);
		else
			_invisibleManLayer.setFrame(0);
	}
}

void Scene1060::advanceFlyDoctor(uint32 delta) {
	const uint frameCount = _flyDoctorChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_flyDoctorLayer.frameIndex == 0 && _random.getRandomNumber(10) != 0)
			continue;
		if (_flyDoctorLayer.frameIndex < ARRAYSIZE(kScene1060FlyDoctorFrameMap) - 1)
			_flyDoctorLayer.setFrame(_flyDoctorLayer.frameIndex + 1);
		else
			_flyDoctorLayer.setFrame(0);
	}
}

void Scene1060::advanceSmallLoop(uint32 delta) {
	const uint frameCount = _smallLoopChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i)
		_smallLoopLayer.setFrame(_smallLoopLayer.frameIndex < 4 ? _smallLoopLayer.frameIndex + 1 : 0);
}

void Scene1060::advanceSmallTrigger(uint32 delta) {
	const uint frameCount = _smallTriggerChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_smallTriggerMode == 0) {
			if (_random.getRandomNumber(28) == 0) {
				_smallTriggerMode = 1;
				_smallTriggerLayer.setFrame(1);
			}
		} else if (_smallTriggerLayer.frameIndex < ARRAYSIZE(kScene1060SmallTriggerFrameMap) - 1) {
			_smallTriggerLayer.setFrame(_smallTriggerLayer.frameIndex + 1);
		} else {
			_smallTriggerLayer.setFrame(0);
			_smallTriggerMode = 0;
		}
	}
}

void Scene1060::replaceColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene1060::copyStageSmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene1060::runJuniorConversation() {
	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(1, 0);
	beginPrimarySpeechLineWithAnimationGroup(2, state.scene1060PartyRemainsState == 1 ? 3 : 0,
		0x235, 0x094, 0x3f, 0x20, 0, kScene1060DoctorSpeechGroup);
	beginSecondarySpeechLine(1, 1);
	beginPrimarySpeechLineWithAnimationGroup(2, state.scene1060PartyRemainsState == 1 ? 4 : 1,
		0x235, 0x094, 0x3f, 0x20, 0, kScene1060DoctorSpeechGroup);
	if (state.scene1060PartyRemainsState == 0) {
		state.scene1060PartyRemainsState = 1;
		applySceneStateToHotspotsAndPatches(5);
	}
	beginSecondarySpeechLine(1, 2);
}

void Scene1060::runDrMoscaConversation() {
	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(0x60, state.seenScene1060DoctorConversation ? 1 : 0);
	beginPrimarySpeechLineWithAnimationGroup(0x61, state.seenScene1060DoctorConversation ? 1 : 0,
		0x235, 0x094, 0x3f, 0x20, 0, kScene1060DoctorSpeechGroup);
	state.seenScene1060DoctorConversation = true;
}

void Scene1060::runInvisibleManConversation() {
	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(0x62, state.seenScene1060InvisibleManConversation ? 1 : 0);
	beginPrimarySpeechLineWithAnimationGroup(0x63, state.seenScene1060InvisibleManConversation ? 1 : 0,
		0x2aa, 0x0b8, 0, 0x3a, 0x28, kScene1060InvisibleManSpeechGroup);
	state.seenScene1060InvisibleManConversation = true;
	if (!state.scene1060PocketPaperTaken)
		handlePocketPaperPickup();
}

void Scene1060::handlePocketPaperPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene1060PocketPaperTaken || hasInventoryItem(0x21)) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	runOverlaySequence(11, kScene1060PocketPaperDescriptorCount, kScene1060PocketPaperFrameMap,
		ARRAYSIZE(kScene1060PocketPaperFrameMap), kScene1060FrameMillis);
	state.scene1060PocketPaperTaken = true;
	addInventoryItem(0x21);
	_soundBank0.playSample(1, 100);
	applySceneStateToHotspotsAndPatches(6);
}

void Scene1060::handlePocketPaperTakeAction() {
	GameplayState &state = _vm->gameState();
	if (state.scene1060FlyDoctorState == 2 || hasInventoryItem(0x22))
		return;

	state.scene1060FlyDoctorState = 2;
	runOverlaySequence(14, kScene1060SkullcrackerDescriptorCount, kScene1060SkullcrackerFrameMap,
		ARRAYSIZE(kScene1060SkullcrackerFrameMap), kScene1060FrameMillis, 4, 1);
	addInventoryItem(0x22);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(8, 0);
}

void Scene1060::handlePocketPaperLook() {
	GameplayState &state = _vm->gameState();
	if (state.scene1060FlyDoctorState == 0) {
		beginSecondarySpeechLine(9, 0);
		state.scene1060FlyDoctorState = 1;
		applySceneStateToHotspotsAndPatches(1);
		return;
	}
	beginSecondarySpeechLine(9, 0);
}

void Scene1060::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame, byte patchSelector) {
	ActionOverlayOptions options;
	options.actorVisibility = kActionOverlayHideActiveActor;
	if (patchFrame >= 0) {
		options.statePatchFrame = patchFrame;
		options.statePatchSelector = patchSelector;
	}
	runActionOverlay(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis, options);
}

} // End of namespace Hollywood
