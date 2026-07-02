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

#include "hollywood/scenes/playable/scene1070.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene1070ArchiveName = "RESOURCE.A07";
const char *const kScene1070MusicArchiveName = "RESOURCE.M01";
const char *const kScene1070SoundArchiveName = "RESOURCE.S01";
const uint kScene1070InitialRequiredChunkCount = 19;
const uint kScene1070ArenaFirstChunk = 5;
const uint kScene1070ArenaLastChunk = 18;
const uint kScene1070StageIndex = 107;
const uint16 kScene1070FirstState = 0x042e;
const uint16 kScene1070LastState = 0x0437;
const uint16 kScene1070ExitState1060 = 0x0425;
const uint16 kScene1070ExitStateNext = 0x0438;
const uint16 kScene1070ViewportXOffset = 0x00a0;
const uint16 kScene1070ViewportMinXOffset = 0x0068;
const uint16 kScene1070ViewportMaxXOffset = 0x00c8;
const uint kScene1070ActorBankTableEntry = 0x0000;
const uint kScene1070ActorPaletteTableEntry = 0x00cc;
const uint kScene1070Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene1070FrameMillis = 75;
const uint32 kScene1070BackFrameMillis = 60;
const uint32 kScene1070IdleFrameMillis = 125;
const uint kScene1070BackDescriptorCount = 0x14;
const uint kScene1070GhostDescriptorCount = 0x12;
const uint kScene1070SpencerDescriptorCount = 0x19;
const uint kScene1070MicrophoneDescriptorCount = 0x0c;
const uint kScene1070MicrophoneStandDescriptorCount = 0x0d;
const uint kScene1070BallDescriptorCount = 0x0b;
const byte kScene1070SpencerSpeechGroup = 1;
const byte kScene1070GhostSpeechGroup = 2;
const byte kScene1070FirstAmbientSoundCue = 0x1c;
const byte kScene1070AmbientSoundCueCount = 6;
const byte kScene1070FirstAmbientMusicCue = 0x0b;
const byte kScene1070AmbientMusicCueCount = 5;
const byte kScene1070AmbientSoundProbabilityModulus = 20;
const byte kScene1070AmbientMusicProbabilityModulus = 50;

const byte kScene1070ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene1070BackFrameMap[] = {
	0, 1, 2, 3, 2, 1, 0, 4, 5, 6, 5, 4, 0, 1, 2, 3,
	2, 1, 0, 7, 8, 9, 8, 7, 10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 3
};

const byte kScene1070GhostFrameMap[] = {
	0, 9, 10, 11, 12, 12, 13, 14, 15, 16, 12, 11, 10, 9, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 5, 6, 7, 8,
	7, 6, 5, 4, 5, 6, 7, 8, 7, 6, 5, 3, 2, 1, 0, 17
};

const byte kScene1070SpencerFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7, 6,
	5, 4, 3, 2, 1, 0, 10, 11, 12, 13, 14, 14, 15, 16, 17, 18,
	10, 19, 20, 21, 22, 23, 22, 21, 22, 23, 22, 21, 22, 23, 22, 21,
	20, 19, 10, 24, 14, 13, 12, 11, 10, 10
};

const byte kScene1070MicrophoneFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene1070MicrophoneStandFrameMap[] = {
	12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene1070BallFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 1, 0,
	1, 2, 1, 0, 1, 2, 1, 0, 1, 2, 1, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9, 10
};

static PlayableSceneConfig scene1070Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene1070ArchiveName;
	config.initialRequiredChunkCount = kScene1070InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene1070ArenaFirstChunk;
	config.arenaLastChunk = kScene1070ArenaLastChunk;
	config.stageIndex = kScene1070StageIndex;
	config.debugName = "Scene 1070";
	config.viewportXOffset = kScene1070ViewportXOffset;
	config.viewportMinXOffset = kScene1070ViewportMinXOffset;
	config.viewportMaxXOffset = kScene1070ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 1;
	config.actorBankTableEntry = kScene1070ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene1070ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene1070Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene1070SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene1070ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene1070ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 6;
	config.musicArchiveName = kScene1070MusicArchiveName;
	config.soundBank0ArchiveName = kScene1070SoundArchiveName;
	config.mainFlowFirstState = kScene1070FirstState;
	config.mainFlowLastState = kScene1070LastState;
	return config;
}

Scene1070::Scene1070(HollywoodEngine *vm) :
		PlayableScene(vm, scene1070Config(), "scene1070", 0x0aa, 0x1b3, 2, 0xfd, 0xfb),
		_backLayerChannel(),
		_ghostChannel(),
		_spencerChannel(),
		_backLayer(),
		_ghostLayer(),
		_spencerLayer(),
		_backLayerMode(0),
		_ghostMode(0),
		_spencerMode(0) {
	_backLayer.configure(9, kScene1070BackDescriptorCount,
		kScene1070BackFrameMap, ARRAYSIZE(kScene1070BackFrameMap));
	_ghostLayer.configure(10, kScene1070GhostDescriptorCount,
		kScene1070GhostFrameMap, ARRAYSIZE(kScene1070GhostFrameMap));
	_spencerLayer.configure(11, kScene1070SpencerDescriptorCount,
		kScene1070SpencerFrameMap, ARRAYSIZE(kScene1070SpencerFrameMap));
}

bool Scene1070::hasCustomPreviewState() const {
	return true;
}

void Scene1070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	if (_vm->gameState().mainFlowStateId == kScene1070FirstState) {
		_activeActorWorldX = 0x0aa;
		_activeActorWorldY = 0x1b3;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x16a;
		_activeActorWorldY = 0x141;
		_activeActorFacing = 4;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene1070::hasCustomComposite() const {
	return true;
}

void Scene1070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_backLayer);
	drawResourceSpriteLayer(_spencerLayer);
	drawResourceSpriteLayer(_ghostLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldX, activeWorldY);
}

bool Scene1070::hasCustomEntrySequence() const {
	return true;
}

void Scene1070::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene1070FirstState)
		runEntryPath(0x064, 0x1b3, 2, 0x0aa, 0x1b3);
	else
		runEntryPath(0x1df, 0x132, 4, 0x16a, 0x141);
	drawPlayableComposite();
	presentFrame();
}

bool Scene1070::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene1070::advanceCustomGameplayLoop(uint32 delta) {
	advanceBackLayer(delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	else {
		advanceGhostLayer(delta);
		advanceSpencerLayer(delta);
	}
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1070::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a salida izquierda (go to left exit).
		_vm->gameState().mainFlowStateId = kScene1070ExitState1060;
		return true;
	case 302: // Ir a puerta (go to door).
		if (_vm->gameState().scene1070DoorOpened)
			_vm->gameState().mainFlowStateId = kScene1070ExitStateNext;
		return true;
	case 303: // Mirar puerta (look at door).
		if (!_vm->gameState().scene1070DoorOpened)
			beginSecondarySpeechLine(1, 0);
		return true;
	case 304: // Usar/abrir puerta (use/open door).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Hablar con Spencer McDundee (talk to Spencer McDundee).
		runSpencerConversation();
		return true;
	case 306: // Mirar Spencer McDundee (look at Spencer McDundee).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 307: // Coger microfono (take microphone).
		handleMicrophonePickup();
		return true;
	case 308: // Mirar microfono (look at microphone).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 309: // Coger pie de micro (take microphone stand).
		handleMicStandPickup();
		return true;
	case 310: // Mirar pie de micro (look at microphone stand).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 311: // Hablar con Quasimodo (talk to Quasimodo).
		runQuasimodoConversation();
		return true;
	case 312: // Mirar Quasimodo (look at Quasimodo).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 313: // Mirar fantasma (look at ghost).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 314: // Mirar bateria (look at drums).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 315: // Usar bateria (use drums).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 316: // Coger cartel (take sign).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 317: // Mirar cartel (look at sign).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 318: // Coger cadena (take chain).
		beginSecondarySpeechLine(12, 0);
		return true;
	case 319: // Mirar cadena (look at chain).
		beginSecondarySpeechLine(13, 0);
		return true;
	case 320: // Mirar bola (look at ball).
		beginSecondarySpeechLine(14, 0);
		return true;
	case 321: // Usar serrucho con cadena (use saw with chain): releases the ball.
		handleBallChainPickup();
		return true;
	default:
		return false;
	}
}

bool Scene1070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene1070DoorOpened) {
		replaceColorMapItem(9, 2);
		if (_sceneChunkTable.isValidChunk(13))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);
		copyStageSmallRow(12, 2);
	}

	if (state.scene1070ChainRemoved)
		replaceColorMapItem(10, 0);

	if (state.scene1070SpiritBlockingHotspot)
		replaceColorMapItem(3, 0);

	if (state.scene1070MicrophoneStandTaken) {
		if (_sceneChunkTable.isValidChunk(17))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[17], _baseFramebuffer);
		replaceColorMapItem(5, 8);
	}

	if (state.scene1070MicrophoneTaken) {
		if (_sceneChunkTable.isValidChunk(15))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[15], _baseFramebuffer);
		replaceColorMapItem(4, 0);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (state.scene1070DoorOpened) {
		ScenePoint point;
		point.x = 0x1df;
		point.y = 0x132;
		_hotspots.setActionTarget(2, point, point);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x11, 3);
	}
	return true;
}

bool Scene1070::shouldAnimatePrimarySpeechLine() const {
	return true;
}

byte Scene1070::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	if (animationGroup == kScene1070SpencerSpeechGroup)
		return 0x1c;
	if (animationGroup == kScene1070GhostSpeechGroup)
		return 5;
	return 0;
}

void Scene1070::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	if (animationGroup == kScene1070SpencerSpeechGroup)
		_spencerLayer.setFrame(frameIndex);
	else if (animationGroup == kScene1070GhostSpeechGroup)
		_ghostLayer.setFrame(frameIndex);
}

AmbientAudioProfile Scene1070::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(kScene1070FirstAmbientSoundCue,
		kScene1070AmbientSoundCueCount, 5, kScene1070AmbientSoundProbabilityModulus,
		kScene1070FirstAmbientMusicCue, kScene1070AmbientMusicCueCount, 100,
		kScene1070AmbientMusicProbabilityModulus);
}

void Scene1070::resetAnimationLayers() {
	_backLayerChannel.reset(0, kScene1070BackFrameMillis);
	_ghostChannel.reset(0, kScene1070IdleFrameMillis);
	_spencerChannel.reset(0, kScene1070IdleFrameMillis);
	_backLayer.reset(_vm->gameState().scene1070ChainRemoved ? 0x18 : 0);
	_ghostLayer.reset(0);
	_spencerLayer.reset(0x1c);
	_backLayer.visible = true;
	_ghostLayer.visible = true;
	_spencerLayer.visible = true;
	_backLayerMode = 0;
	_ghostMode = 0;
	_spencerMode = 0;
}

void Scene1070::advanceBackLayer(uint32 delta) {
	const uint frameCount = _backLayerChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		const bool chainRemoved = _vm->gameState().scene1070ChainRemoved;
		const byte startFrame = chainRemoved ? 0x18 : 0;
		const byte endFrame = chainRemoved ? 0x22 : 0x17;
		if (_backLayer.frameIndex < endFrame)
			_backLayer.setFrame(_backLayer.frameIndex + 1);
		else
			_backLayer.setFrame(startFrame);
	}
}

void Scene1070::advanceGhostLayer(uint32 delta) {
	const uint frameCount = _ghostChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_ghostMode == 0) {
			if (_random.getRandomNumber(14) == 0) {
				_ghostLayer.setFrame(0x0e);
				_ghostMode = 1;
			} else if (_random.getRandomNumber(19) == 0) {
				_ghostLayer.setFrame(0x20);
				_ghostMode = 2;
			}
		} else if (_ghostMode == 1) {
			if (_ghostLayer.frameIndex < 0x0f)
				_ghostLayer.setFrame(_ghostLayer.frameIndex + 1);
			else {
				_ghostLayer.setFrame(0);
				_ghostMode = 0;
			}
		} else if (_ghostLayer.frameIndex < 0x2f) {
			_ghostLayer.setFrame(_ghostLayer.frameIndex + 1);
		} else {
			_ghostLayer.setFrame(0);
			_ghostMode = 0;
		}
	}
}

void Scene1070::advanceSpencerLayer(uint32 delta) {
	const uint frameCount = _spencerChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (_spencerMode == 0) {
			if (_random.getRandomNumber(14) == 0) {
				_spencerLayer.setFrame(0x33);
				_spencerMode = 1;
			} else if (_random.getRandomNumber(19) == 0) {
				_spencerLayer.setFrame(0x21);
				_spencerMode = 2;
			}
		} else if (_spencerMode == 1) {
			_spencerLayer.setFrame(0x1c);
			_spencerMode = 0;
		} else if (_spencerLayer.frameIndex < 0x32) {
			_spencerLayer.setFrame(_spencerLayer.frameIndex + 1);
		} else {
			_spencerLayer.setFrame(0x1c);
			_spencerMode = 0;
		}
	}
}

void Scene1070::drawForegroundBlocks(int activeWorldX, int activeWorldY) {
	if (activeWorldY < 0x138 && _sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (activeWorldX > 0x18a && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	if (activeWorldY < 0x132 && _sceneChunkTable.isValidChunk(8))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _sceneFramebuffer);
	if (activeWorldY <= 0x132 && activeWorldX >= 399 && _sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	if (_vm->gameState().scene1070MicrophoneTaken && _sceneChunkTable.isValidChunk(15))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[15], _sceneFramebuffer);
}

void Scene1070::replaceColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene1070::copyStageSmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene1070::runSpencerConversation() {
	GameplayState &state = _vm->gameState();
	const byte line = state.seenScene1070SpencerConversation ? 1 : 0;
	beginSecondarySpeechLine(0x60, line);
	beginPrimarySpeechLineWithAnimationGroup(0x61, line, 0x0f1, 0x0b8,
		0x3f, 0x0d, 0x0d, kScene1070SpencerSpeechGroup);
	state.seenScene1070SpencerConversation = true;
	if (state.scene1070SpencerDialogueState < 3)
		++state.scene1070SpencerDialogueState;
}

void Scene1070::runQuasimodoConversation() {
	GameplayState &state = _vm->gameState();
	const byte line = state.seenScene1070QuasimodoConversation ? 1 : 0;
	beginSecondarySpeechLine(0x62, line);
	beginPrimarySpeechLineWithAnimationGroup(0x63, line, 0x0a9, 0x0c8,
		0x20, 0x3f, 0, kScene1070GhostSpeechGroup);
	state.seenScene1070QuasimodoConversation = true;
}

void Scene1070::handleMicrophonePickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene1070MicrophoneTaken || hasInventoryItem(0x23))
		return;

	state.scene1070MicrophoneTaken = true;
	runOverlaySequence(14, kScene1070MicrophoneDescriptorCount, kScene1070MicrophoneFrameMap,
		ARRAYSIZE(kScene1070MicrophoneFrameMap), kScene1070FrameMillis, 5, 7);
	addInventoryItem(0x23);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene1070::handleMicStandPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene1070MicrophoneStandTaken || hasInventoryItem(0x24))
		return;

	state.scene1070MicrophoneStandTaken = true;
	runOverlaySequence(18, kScene1070MicrophoneStandDescriptorCount, kScene1070MicrophoneStandFrameMap,
		ARRAYSIZE(kScene1070MicrophoneStandFrameMap), kScene1070FrameMillis, 6, 6);
	addInventoryItem(0x24);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene1070::handleBallChainPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene1070ChainRemoved || hasInventoryItem(0x1e))
		return;

	dispatchGenericSceneAction(21);
	_soundBank0.playSampleLooping(0x22, 15);
	runOverlaySequence(12, kScene1070BallDescriptorCount, kScene1070BallFrameMap,
		ARRAYSIZE(kScene1070BallFrameMap), kScene1070FrameMillis, -1, 1);
	_soundBank0.stop();
	state.scene1070ChainRemoved = true;
	addInventoryItem(0x1e);
	_soundBank0.playSample(1, 100);
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(15, 0);
}

void Scene1070::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int patchFrame, byte patchSelector) {
	runConfiguredActionOverlay(chunkIndex, descriptorCount, frameMap, frameMapSize, frameMillis,
		kActionOverlayHideActiveActor, patchFrame, patchSelector);
}

} // End of namespace Hollywood
