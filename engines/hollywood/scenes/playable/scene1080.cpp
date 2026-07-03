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

#include "hollywood/scenes/playable/scene1080.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene1080ArchiveName = "RESOURCE.A08";
const char *const kScene1080MusicArchiveName = "RESOURCE.M01";
const char *const kScene1080SoundArchiveName = "RESOURCE.S01";
const uint kScene1080InitialRequiredChunkCount = 12;
const uint kScene1080ArenaFirstChunk = 5;
const uint kScene1080ArenaLastChunk = 11;
const uint kScene1080StageIndex = 108;
const uint16 kScene1080FirstState = 0x0438;
const uint16 kScene1080LastState = 0x0441;
const uint16 kScene1080ExitStateBallroom = 0x042f;
const uint16 kScene1080ExitStatePantry = 0x0442;
const uint16 kScene1080ViewportXOffset = 0x00a0;
const uint16 kScene1080ViewportMinXOffset = 0x0068;
const uint16 kScene1080ViewportMaxXOffset = 0x00a8;
const uint kScene1080ActorBankTableEntry = 0x0000;
const uint kScene1080ActorPaletteTableEntry = 0x00cc;
const uint kScene1080Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1080SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene1080FrameMillis = 75;
const uint32 kScene1080FrancoisFrameMillis = 125;
const uint kScene1080ForegroundDescriptorCount = 5;
const uint kScene1080FrancoisDescriptorCount = 0x13;
const byte kScene1080FrancoisSpeechGroup = 1;
const byte kScene1080FirstAmbientSoundCue = 0x34;
const byte kScene1080AmbientSoundCueCount = 3;
const byte kScene1080FirstAmbientMusicCue = 0x0b;
const byte kScene1080AmbientMusicCueCount = 5;

const byte kScene1080ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene1080ForegroundFrameMap[] = { 0, 1, 2, 3, 4, 3, 2, 1, 0 };

const byte kScene1080FrancoisFrameMap[] = {
	0, 1, 2, 3, 16, 4, 17, 5, 4, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 17, 18
};

static PlayableSceneConfig scene1080Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene1080ArchiveName;
	config.initialRequiredChunkCount = kScene1080InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene1080ArenaFirstChunk;
	config.arenaLastChunk = kScene1080ArenaLastChunk;
	config.stageIndex = kScene1080StageIndex;
	config.debugName = "Scene 1080";
	config.viewportXOffset = kScene1080ViewportXOffset;
	config.viewportMinXOffset = kScene1080ViewportMinXOffset;
	config.viewportMaxXOffset = kScene1080ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 1;
	config.actorBankTableEntry = kScene1080ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene1080ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene1080Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene1080SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene1080ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene1080ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 6;
	config.musicArchiveName = kScene1080MusicArchiveName;
	config.soundBank0ArchiveName = kScene1080SoundArchiveName;
	config.mainFlowFirstState = kScene1080FirstState;
	config.mainFlowLastState = kScene1080LastState;
	return config;
}

Scene1080::Scene1080(HollywoodEngine *vm) :
		PlayableScene(vm, scene1080Config(), "scene1080", 0x1d3, 0x15c, 2, 0xfd, 0xfb),
		_foregroundChannel(),
		_francoisChannel(),
		_foregroundLayer(),
		_francoisLayer(),
		_foregroundMode(0),
		_francoisMode(0) {
	_foregroundLayer.configure(11, kScene1080ForegroundDescriptorCount,
		kScene1080ForegroundFrameMap, ARRAYSIZE(kScene1080ForegroundFrameMap));
	_francoisLayer.configure(8, kScene1080FrancoisDescriptorCount,
		kScene1080FrancoisFrameMap, ARRAYSIZE(kScene1080FrancoisFrameMap));
}

bool Scene1080::hasCustomPreviewState() const {
	return true;
}

void Scene1080::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	if (_vm->gameState().mainFlowStateId == kScene1080FirstState) {
		_activeActorWorldX = 0x1d3;
		_activeActorWorldY = 0x15c;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x1fd;
		_activeActorWorldY = 0x14d;
		_activeActorFacing = 4;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene1080::hasCustomComposite() const {
	return true;
}

void Scene1080::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_vm->gameState().scene1080FrancoisProgressState < 2)
		drawResourceSpriteLayer(_francoisLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldX, activeWorldY);
	drawResourceSpriteLayer(_foregroundLayer);
}

bool Scene1080::hasCustomEntrySequence() const {
	return true;
}

void Scene1080::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene1080FirstState) {
		runEntryPath(0x157, 0x0b4, 2, 0x1d3, 0x15c);
		if (!state.scene1080EntryLineSeen) {
			beginSecondarySpeechLine(0, 0);
			state.scene1080EntryLineSeen = true;
		}
	} else {
		runEntryPath(0x1fd, 0x14d, 4, 0x1fd, 0x14d);
	}
	drawPlayableComposite();
	presentFrame();
}

bool Scene1080::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene1080::advanceCustomGameplayLoop(uint32 delta) {
	advanceForegroundLayer(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else if (_vm->gameState().scene1080FrancoisProgressState < 2)
		advanceFrancoisLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1080::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a escalera (go to stairs).
		_vm->gameState().mainFlowStateId = kScene1080ExitStateBallroom;
		return true;
	case 302: // Mirar escalera (look at stairs).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Ir a despensa (go to pantry).
		if (_vm->gameState().scene1080FrancoisProgressState >= 2)
			_vm->gameState().mainFlowStateId = kScene1080ExitStatePantry;
		else
			beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar despensa (look at pantry).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Hablar con Francois (talk to Francois).
		runFrancoisConversation();
		return true;
	case 306: // Mirar Francois (look at Francois).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Coger/abrir cajas (take/open boxes).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Mirar cajas (look at boxes).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Coger lata derramada (take spilled can).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar lata derramada (look at spilled can).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Coger latas (take cans).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Mirar latas (look at cans).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313: // Mirar platos sucios (look at dirty plates).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314: // Usar globo inflado con gas con Francois (use gas-filled balloon with Francois).
		handleFrancoisDistraction();
		return true;
	default:
		return false;
	}
}

bool Scene1080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	applyKitchenItemMap();
	rebuildKitchenWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	if (_vm->gameState().scene1080FrancoisProgressState >= 2) {
		ScenePoint point;
		point.x = 0x02b9;
		point.y = 0x0123;
		_hotspots.setActionTarget(2, point, point);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x11, 1);
	}
	return true;
}

bool Scene1080::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene1080::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene1080FrancoisSpeechGroup)
		return 0;
	return 0;
}

void Scene1080::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene1080FrancoisSpeechGroup)
		_francoisLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene1080::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(kScene1080FirstAmbientSoundCue,
		kScene1080AmbientSoundCueCount, 20, 25,
		kScene1080FirstAmbientMusicCue, kScene1080AmbientMusicCueCount, 100, 50);
}

void Scene1080::resetAnimationLayers() {
	_foregroundChannel.reset(0, kScene1080FrameMillis);
	_francoisChannel.reset(0, kScene1080FrancoisFrameMillis);
	_foregroundLayer.reset(0);
	_francoisLayer.reset(0);
	_foregroundLayer.visible = true;
	_francoisLayer.visible = _vm->gameState().scene1080FrancoisProgressState < 2;
	_foregroundMode = 0;
	_francoisMode = 0;
}

void Scene1080::advanceForegroundLayer(uint32 delta) {
	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_foregroundMode == 0) {
			if (_random.getRandomNumber(29) == 0) {
				_foregroundMode = 1;
				_foregroundLayer.setFrame(0);
			}
		} else if (_foregroundLayer.frameIndex + 1 < ARRAYSIZE(kScene1080ForegroundFrameMap)) {
			_foregroundLayer.setFrame(_foregroundLayer.frameIndex + 1);
		} else {
			_foregroundLayer.setFrame(0);
			_foregroundMode = 0;
		}
	}
}

void Scene1080::advanceFrancoisLayer(uint32 delta) {
	const uint frameCount = _francoisChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_francoisMode == 0) {
			if (_random.getRandomNumber(19) == 0) {
				_francoisMode = 2;
				_francoisLayer.setFrame(8);
			} else {
				_francoisLayer.setFrame(_francoisLayer.frameIndex == 0 ? 4 : 0);
			}
		} else if (_francoisLayer.frameIndex < 0x12) {
			_francoisLayer.setFrame(_francoisLayer.frameIndex + 1);
		} else {
			_francoisLayer.setFrame(0);
			_francoisMode = 0;
		}
	}
}

void Scene1080::drawForegroundBlocks(int activeWorldX, int activeWorldY) {
	if (activeWorldY < 0x134 && activeWorldX < 0x1c7 && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	if (activeWorldY < 0x133 && activeWorldX > 0x23d && _sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene1080::rebuildKitchenWalkableMask() {
	const bool francoisGone = _vm->gameState().scene1080FrancoisProgressState >= 2;
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		const byte originalRegion = _paletteMaskOriginal[i];
		byte region = originalRegion;
		if (francoisGone && originalRegion == 6)
			region = 1;
		_fullPaletteRegionMask[i] = region;

		byte walkableRegion = region;
		if (walkableRegion > walkablePaletteMaxRegion() || walkableRegion == 2 || walkableRegion == 4 ||
				(!francoisGone && originalRegion == 6))
			walkableRegion = 0;
		_walkablePaletteMask[i] = walkableRegion;
	}
}

void Scene1080::applyKitchenItemMap() {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	const bool francoisGone = _vm->gameState().scene1080FrancoisProgressState >= 2;
	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
		byte item = originalItem;
		if (francoisGone) {
			if (originalItem == 8)
				item = 0;
			else if (originalItem == 3 || originalItem == 2)
				item = 2;
		} else {
			if (originalItem == 8 || originalItem == 3)
				item = 3;
			else if (originalItem == 2)
				item = 0;
		}
		_paletteMask[kSceneColorToItemMap + i] = item;
	}
}

void Scene1080::runFrancoisConversation() {
	GameplayState &state = _vm->gameState();
	const byte frame = state.scene1080FrancoisProgressState == 0 ? 0 : 1;
	beginSecondarySpeechLine(0x62, frame);
	beginPrimarySpeechLineWithAnimationGroup(99, frame, 0x022e, 0x0084,
		0x0d, 0x32, 0x3a, kScene1080FrancoisSpeechGroup);
	if (state.scene1080FrancoisProgressState == 0)
		state.scene1080FrancoisProgressState = 1;
}

void Scene1080::handleFrancoisDistraction() {
	GameplayState &state = _vm->gameState();
	if (state.scene1080FrancoisProgressState >= 2)
		return;

	beginSecondarySpeechLine(12, 0);
	beginSecondarySpeechLine(13, 0);
	beginSecondarySpeechLine(14, 0);
	beginSecondarySpeechLine(15, 0);
	if (hasInventoryItem(0x4d))
		removeInventoryItem(0x4d);
	if (!hasInventoryItem(0x1c))
		addInventoryItem(0x1c);
	state.scene1080FrancoisProgressState = 2;
	_francoisLayer.visible = false;
	applySceneStateToHotspotsAndPatches(1);
}

} // End of namespace Hollywood
