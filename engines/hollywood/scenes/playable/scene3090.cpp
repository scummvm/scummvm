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

#include "hollywood/scenes/playable/scene3090.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene3080ReturnFromScene3090State = 0x0c0a;
const uint16 kScene3090ViewportXOffset = 0x0098;
const uint16 kScene3090ViewportMinXOffset = 0x0068;
const uint16 kScene3090ViewportMaxXOffset = 0x0098;
const uint kScene3090ActorBankTableEntry = 0x0000;
const uint kScene3090ActorPaletteTableEntry = 0x00cc;
const uint kScene3090Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3090SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3090FrontFrameMillis = 100;
const uint32 kScene3090BlindManFrameMillis = 100;
const uint32 kScene3090BlindManSpeechFrameMillis = 100;
const uint32 kScene3090PuzzleFrameMillis = 125;
const uint32 kScene3090OverlayFrameMillis = 75;
const uint kScene3090FrontDescriptorCount = 0x1a;
const uint kScene3090BlindManDescriptorCount = 0x24;
const uint kScene3090PuzzleDescriptorCount = 0x26;
const uint kScene3090PickupDescriptorCount = 0x0c;
const uint kScene3090StrawDescriptorCount = 0x0d;
const uint kScene3090SaxophoneDescriptorCount = 0x0c;
const byte kScene3090DialogueStageId = 0x62;
const byte kScene3090DialoguePrimaryRow = 99;
const uint kScene3090DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene3090PrimarySpeechRed = 0x2c;
const byte kScene3090PrimarySpeechGreen = 0x3f;
const byte kScene3090PrimarySpeechBlue = 0x13;
const uint16 kScene3090PrimarySpeechCenterX = 0x154;
const uint16 kScene3090PrimarySpeechTopY = 0x0ac;
const byte kScene3090SecretDiarySourceRow = 8;
const byte kScene3090BookDestinationRow = 1;
const byte kScene3090SaltShakerItemId = 0x36;
const byte kScene3090DowsingRodItemId = 0x37;
const byte kScene3090StrawItemId = 0x46;
const byte kScene3090SaxophoneItemId = 0x5c;
const byte kScene3090BlindManSpeechNormalBaseFrame = 0;
const byte kScene3090BlindManSpeechSaxophoneBaseFrame = 0x24;
const byte kScene3090BlindManPostConversationNormalFrame = 4;
const byte kScene3090BlindManPostConversationSaxophoneFrame = 0x28;
const byte kScene3090BlindManNoPreviousSpeechFrame = 0xff;

const byte kScene3090FrontFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25
};

const byte kScene3090PuzzleFrameMap[] = {
	24, 25, 26, 21, 22, 23, 18, 19, 20, 15,
	16, 17, 12, 13, 14, 9, 10, 11, 6, 7,
	8, 3, 4, 5, 0, 1, 2, 27, 28, 29,
	28, 29, 28, 27, 30, 31, 32, 33, 34, 35,
	36, 37
};

const byte kScene3090BlindManFrameMap[] = {
	0, 1, 2, 3, 0, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 11, 10, 9, 8, 7, 6,
	5, 4, 0, 0, 13, 14, 15, 16, 17, 18,
	17, 16, 15, 14, 13, 0, 19, 20, 21, 22,
	19, 23, 24, 25, 26, 27, 28, 29, 28, 27,
	26, 25, 24, 23, 19, 19, 30, 31, 32, 33,
	34, 35, 34, 33, 32, 31, 30
};

const byte kScene3090PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene3090StrawFrameMap[] = {
	11, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 12
};

const byte kScene3090SaxophoneFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0
};

static PlayableSceneConfig scene3090Config() {
	PlayableSceneConfig config(3090,
		SceneResourceLayout(20, 5, 19),
		SceneViewport(kScene3090ViewportXOffset, kScene3090ViewportMinXOffset, kScene3090ViewportMaxXOffset),
		SceneActorPose(600, 0x145, 4));
	config.setActorResources(kScene3090ActorBankTableEntry, kScene3090ActorPaletteTableEntry);
	config.setTextResources(kScene3090Resource003RowsOffsetIndex, kScene3090SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene3090::Scene3090(HollywoodEngine *vm) :
		PlayableScene(vm, scene3090Config()),
		_frontChannel(),
		_blindManChannel(),
		_puzzleChannel(),
		_frontLayer(),
		_blindManLayer(),
		_puzzleLayer(),
		_puzzleLayerTriggered(false),
		_dialogueMenuActive(false),
		_blindManConversationActive(false),
		_blindManSpeechActive(false),
		_blindManSpeechLastRandomFrame(kScene3090BlindManNoPreviousSpeechFrame),
		_blindManSpeechTimerAccumulator(0) {
	_frontLayer.configure(9, kScene3090FrontDescriptorCount,
		kScene3090FrontFrameMap, ARRAYSIZE(kScene3090FrontFrameMap));
	_blindManLayer.configure(11, kScene3090BlindManDescriptorCount,
		kScene3090BlindManFrameMap, ARRAYSIZE(kScene3090BlindManFrameMap));
	_puzzleLayer.configure(12, kScene3090PuzzleDescriptorCount,
		kScene3090PuzzleFrameMap, ARRAYSIZE(kScene3090PuzzleFrameMap));
}

void Scene3090::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	setActiveActorPose(600, 0x145, 4);
}

void Scene3090::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	_frontLayer.chunkIndex = _vm->gameState().scene3090WindowOpenSequenceState == 0 ? 9 : 10;
	drawResourceSpriteLayer(_frontLayer);
	if (_puzzleLayer.visible)
		drawResourceSpriteLayer(_puzzleLayer);
	drawResourceSpriteLayer(_blindManLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldY);
}

void Scene3090::runCustomEntrySequence() {
	runEntryPath(600, 0x145, 4, 600, 0x145);
	if (!_vm->gameState().scene3090EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene3090EntryLineSeen = true;
	}
}

bool Scene3090::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene3090::advanceCustomGameplayLoop(uint32 delta) {
	advanceFrontLayer(delta);
	if (_blindManSpeechActive)
		advanceBlindManSpeechAnimation(delta);
	else if (!_blindManConversationActive && !_dialogueMenuActive)
		advanceBlindManLayer(delta);
	advancePuzzleLayer(delta);
	if (!_blindManSpeechActive && !_blindManConversationActive && !_dialogueMenuActive)
		updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3090::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Coger libro (take book): Ron cannot do it while the blind man is there.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Mirar libro / diario secreto (look at book / secret diary).
		beginSecondarySpeechLine(2, state.scene3090SecretDiaryPuzzleStage == 0 ? 0 : 1);
		return true;
	case 303: // Hablar con hombre ciego (talk to blind man).
		runBlindManConversation();
		setBlindManPostConversationFrame();
		return true;
	case 304: // Mirar hombre ciego (look at blind man).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar ventana (look at window).
		beginSecondarySpeechLine(4, state.scene3090WindowOpenSequenceState == 0 ? 1 : 0);
		return true;
	case 306: // Abrir ventana (open window).
		if (state.scene3090WindowOpenSequenceState == 0)
			beginSecondarySpeechLine(5, 0);
		else
			dispatchGenericSceneAction(20);
		return true;
	case 307: // Cerrar ventana (close window).
		dispatchGenericSceneAction(state.scene3090WindowOpenSequenceState == 0 ? 12 : 20);
		return true;
	case 308: // Ir a exterior (go outside): return to the brook outside the cabin.
		runExitToScene3080();
		return true;
	case 309: // Mirar exterior (look outside).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 310: // Mirar chimenea (look at fireplace).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 311: // Coger salero (take salt shaker).
		runSaltShakerPickup();
		return true;
	case 312: // Mirar salero/sal (look at salt shaker/salt).
		beginSecondarySpeechLine(8, 1);
		return true;
	case 313: // Coger varita (take dowsing rod).
		runDowsingRodPickup();
		return true;
	case 314: // Mirar varita (look at dowsing rod).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 315: // Usar paja en chimenea (use straw in fireplace).
		runUseStrawInFireplace();
		return true;
	case 316: // Dar saxofon al hombre ciego (give saxophone to blind man).
		runSaxophoneHandoff();
		return true;
	default:
		return false;
	}
}

bool Scene3090::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetX < 0x18f)
		targetX = 0x18f;
	if (targetX > 0x2a4)
		targetX = 0x2a4;

	while (targetY < 0x1df) {
		++targetY;
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
	}

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		--targetY;
	}

	return true;
}

bool Scene3090::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 1 || selector == 2 || selector == 4 || selector == 5) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

		const GameplayState &state = _vm->gameState();
		if (state.scene3090SecretDiaryPuzzleStage < 2) {
			if (!state.scene3090BlindManPlayingSaxophone) {
				removeColorMapItem(8);
			} else if (state.scene3090SecretDiaryPuzzleProgress < 9) {
				removeColorMapItem(1);
			} else {
				removeColorMapItem(1);
				replaceColorMapItemFromOriginal(8, 1);
			}
		}
		if (state.scene3090SecretDiaryPuzzleStage == 1)
			copySmallTextRow(kScene3090SecretDiarySourceRow, kScene3090BookDestinationRow);
		if (state.scene3090SecretDiaryPuzzleStage == 2) {
			removeColorMapItem(1);
			removeColorMapItem(8);
		}

		if (state.scene3090WindowOpenSequenceState == 0) {
			if (_sceneChunkTable.isValidChunk(6))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
			replaceActorPaletteClassFromOriginal(4, 2);
		} else {
			if (_sceneChunkTable.isValidChunk(5))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
			replaceActorPaletteClassFromOriginal(4, 1);
		}

		if (state.scene3090SaltShakerTaken) {
			removeColorMapItem(6);
			if (_sceneChunkTable.isValidChunk(7))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
		}

		if (state.scene3090DowsingRodTaken) {
			removeColorMapItem(7);
			if (_sceneChunkTable.isValidChunk(8))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		}

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

AmbientAudioProfile Scene3090::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.soundMode = kAmbientSoundLoop;
	profile.soundCueId = 0x1c;
	profile.soundVolumePercent = _vm->gameState().scene3090BlindManPlayingSaxophone ? 0x50 : 0x28;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = _vm->gameState().scene3090BlindManPlayingSaxophone ? 0x11 : 0x10;
	profile.musicCueCount = 1;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3090::resetAnimationLayers() {
	GameplayState &state = _vm->gameState();
	_frontChannel.reset(0, kScene3090FrontFrameMillis);
	const byte blindManFrame = state.scene3090BlindManPlayingSaxophone ? 0x2b : 7;
	_blindManChannel.reset(blindManFrame, kScene3090BlindManFrameMillis);
	const byte puzzleFrame = (byte)(state.scene3090SecretDiaryPuzzleProgress * 3);
	_puzzleChannel.reset(puzzleFrame, kScene3090PuzzleFrameMillis);
	_frontLayer.visible = true;
	_blindManLayer.visible = true;
	_puzzleLayer.visible = state.scene3090SecretDiaryPuzzleStage < 2;
	_frontLayer.reset(0);
	_blindManLayer.reset(blindManFrame);
	_puzzleLayer.reset(puzzleFrame);
	_puzzleLayerTriggered = false;
	_dialogueMenuActive = false;
	_blindManConversationActive = false;
	_blindManSpeechActive = false;
	_blindManSpeechLastRandomFrame = kScene3090BlindManNoPreviousSpeechFrame;
	_blindManSpeechTimerAccumulator = 0;
}

void Scene3090::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 3 || _walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}
}

void Scene3090::removeColorMapItem(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal.size() > kSceneColorToItemMap + i &&
				_paletteMaskOriginal[kSceneColorToItemMap + i] == itemId)
			_paletteMask[kSceneColorToItemMap + i] = 0;
	}
}

void Scene3090::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene3090::replaceActorPaletteClassFromOriginal(byte sourceClass, byte destinationClass) {
	if (_paletteMask.size() < kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToActorDepthClassMap + i] == sourceClass)
			_paletteMask[kSceneColorToActorDepthClassMap + i] = destinationClass;
	}
}

void Scene3090::copySmallTextRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene3090::advanceFrontLayer(uint32 delta) {
	const uint frameCount = _frontChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		_frontChannel.frameIndex = _frontChannel.frameIndex == 25 ? 0 : _frontChannel.frameIndex + 1;
		_frontLayer.chunkIndex = _vm->gameState().scene3090WindowOpenSequenceState == 0 ? 9 : 10;
		_frontLayer.setFrame(_frontChannel.frameIndex);
	}
}

void Scene3090::advanceBlindManLayer(uint32 delta) {
	const uint frameCount = _blindManChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		GameplayState &state = _vm->gameState();
		++_blindManChannel.frameIndex;
		if (!state.scene3090BlindManPlayingSaxophone) {
			if (_blindManChannel.frameIndex == 0x13)
				_blindManChannel.frameIndex = 7;
			if (_blindManChannel.frameIndex == 0x0d)
				triggerPuzzleLayer();
		} else {
			if (_blindManChannel.frameIndex == 0x33)
				_blindManChannel.frameIndex = 0x2b;
			if (_blindManChannel.frameIndex == 0x2f)
				triggerPuzzleLayer();
		}
		_blindManLayer.setFrame(_blindManChannel.frameIndex);
	}
}

void Scene3090::beginBlindManSpeechAnimation() {
	_blindManSpeechActive = true;
	_blindManSpeechLastRandomFrame = kScene3090BlindManNoPreviousSpeechFrame;
	_blindManSpeechTimerAccumulator = 0;

	const byte frame = blindManSpeechBaseFrame();
	_blindManChannel.frameIndex = frame;
	_blindManLayer.setFrame(frame);
}

void Scene3090::advanceBlindManSpeechAnimation(uint32 delta) {
	_blindManSpeechTimerAccumulator += delta;
	while (_blindManSpeechTimerAccumulator >= kScene3090BlindManSpeechFrameMillis) {
		_blindManSpeechTimerAccumulator -= kScene3090BlindManSpeechFrameMillis;

		byte randomFrame = _blindManSpeechLastRandomFrame;
		for (uint attempt = 0; attempt < 8 && randomFrame == _blindManSpeechLastRandomFrame; ++attempt)
			randomFrame = (byte)_random.getRandomNumber(3);
		if (randomFrame == _blindManSpeechLastRandomFrame)
			randomFrame = (byte)((_blindManSpeechLastRandomFrame + 1) & 3);

		_blindManSpeechLastRandomFrame = randomFrame;
		_blindManChannel.frameIndex = (byte)(blindManSpeechBaseFrame() + randomFrame);
		_blindManLayer.setFrame(_blindManChannel.frameIndex);
	}
}

void Scene3090::endBlindManSpeechAnimation() {
	_blindManSpeechActive = false;
	_blindManSpeechLastRandomFrame = kScene3090BlindManNoPreviousSpeechFrame;
	_blindManSpeechTimerAccumulator = 0;

	const byte frame = blindManSpeechBaseFrame();
	_blindManChannel.frameIndex = frame;
	_blindManLayer.setFrame(frame);
}

byte Scene3090::blindManSpeechBaseFrame() const {
	return _vm->gameState().scene3090BlindManPlayingSaxophone ?
		kScene3090BlindManSpeechSaxophoneBaseFrame : kScene3090BlindManSpeechNormalBaseFrame;
}

void Scene3090::setBlindManPostConversationFrame() {
	const byte frame = _vm->gameState().scene3090BlindManPlayingSaxophone ?
		kScene3090BlindManPostConversationSaxophoneFrame : kScene3090BlindManPostConversationNormalFrame;
	_blindManChannel.frameIndex = frame;
	_blindManLayer.setFrame(frame);
}

void Scene3090::advancePuzzleLayer(uint32 delta) {
	if (!_puzzleLayer.visible || !_puzzleLayerTriggered)
		return;

	const uint frameCount = _puzzleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		GameplayState &state = _vm->gameState();
		if (!state.scene3090BlindManPlayingSaxophone) {
			++_puzzleChannel.frameIndex;
			if (_puzzleChannel.frameIndex == 3) {
				_puzzleChannel.frameIndex = 0;
				_puzzleLayerTriggered = false;
			}
		} else if (state.scene3090SecretDiaryPuzzleProgress < 9) {
			++_puzzleChannel.frameIndex;
			if ((uint)state.scene3090SecretDiaryPuzzleProgress * 3 + 3 == _puzzleChannel.frameIndex) {
				++state.scene3090SecretDiaryPuzzleProgress;
				_puzzleLayerTriggered = false;
				applySceneStateToHotspotsAndPatches(1);
			}
		} else if (state.scene3090WindowOpenSequenceState == 0) {
			++_puzzleChannel.frameIndex;
			if (_puzzleChannel.frameIndex == 0x1e) {
				_puzzleChannel.frameIndex = 0x1b;
				_puzzleLayerTriggered = false;
			}
		} else if (state.scene3090WindowOpenSequenceState == 1) {
			if (_puzzleChannel.frameIndex < 0x29) {
				++_puzzleChannel.frameIndex;
			} else {
				_puzzleLayerTriggered = false;
				state.scene3090SecretDiaryPuzzleStage = 2;
				state.scene3080FrankensteinDiaryRevealed = true;
				applySceneStateToHotspotsAndPatches(1);
			}
		}
		_puzzleLayer.visible = state.scene3090SecretDiaryPuzzleStage < 2;
		_puzzleLayer.setFrame(_puzzleChannel.frameIndex);
	}
}

void Scene3090::triggerPuzzleLayer() {
	if (_vm->gameState().scene3090SecretDiaryPuzzleStage < 2) {
		_puzzleLayerTriggered = true;
		_puzzleLayer.visible = true;
	}
	_soundBank0.playSample(0x16, 0x14);
}

void Scene3090::runExitToScene3080() {
	_vm->gameState().mainFlowStateId = kScene3080ReturnFromScene3090State;
}

void Scene3090::runBlindManConversation() {
	GameplayState &state = _vm->gameState();
	Common::Array<DialogueChoiceRecord> records;
	initializeDialogueRecords(records);

	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;
	_blindManConversationActive = true;
	_vm->gameplayMusic()->stop();

	if (!state.scene3090BlindManConversationSeen) {
		beginSecondarySpeechLine(kScene3090DialogueStageId, 0);
		beginBlindManResponse(1);
		state.scene3090BlindManConversationSeen = true;
	} else {
		beginSecondarySpeechLine(kScene3090DialogueStageId, 1);
		beginBlindManResponse(2);
	}

	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		_dialogueMenuActive = true;
		const byte selectedChoice = menu.choose(kScene3090DialogueStageId, records, depthIndex, nodeIndex);
		_dialogueMenuActive = false;
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene3090DialogueStageId, 7);
			beginBlindManResponse(8);
			_blindManConversationActive = false;
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene3090DialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff)
			beginBlindManResponse(record.responseFrameIndex);

		if (record.disableAfterUse != 0)
			record.enabled = 0;
		if (record.disableAfterUse == 3 && state.scene3090SecretDiaryPuzzleStage == 0) {
			state.scene3090SecretDiaryPuzzleStage = 1;
			applySceneStateToHotspotsAndPatches(1);
			if (records.size() > 0x1f8)
				records[0x1f8].enabled = 1;
		}
		if (record.disableAfterUse == 4)
			state.scene3090DialogueMentionedBlindManLaxative = true;

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

	_blindManConversationActive = false;
}

void Scene3090::initializeDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene3090DialogueChoiceRecordCount);

	setDialogueRecord(records, 0, 1, 0, 3, 2, 3, 1);
	setDialogueRecord(records, 1, 1, 0, 1, 3, 4, 1);
	setDialogueRecord(records, 2, _vm->gameState().scene3090DialogueMentionedBlindManLaxative ? 1 : 0, 1, 1, 4, 5, 1);
	setDialogueRecord(records, 3, 1, 2, 1, 5, 6, 1);
	setDialogueRecord(records, 4, _vm->gameState().scene3090SecretDiaryPuzzleStage != 0 ? 1 : 0, 3, 1, 6, 7, 1);
	setDialogueRecord(records, 5, 1, 0, 3, 22, 23, 2);
	setDialogueRecord(records, 6, 1, 0, 0, 7, 8, 0);

	setDialogueRecord(records, 70, 1, 0, 3, 8, 9, 3);
	setDialogueRecord(records, 71, 1, 0, 3, 9, 10, 1);
	setDialogueRecord(records, 72, 0, 0, 3, 10, 11, 1);
	setDialogueRecord(records, 73, 1, 0, 2, 11, 12, 0);

	setDialogueRecord(records, 140, 1, 1, 3, 12, 13, 1);
	setDialogueRecord(records, 141, 1, 1, 3, 13, 14, 4);
	setDialogueRecord(records, 142, 1, 0, 2, 14, 15, 0);

	setDialogueRecord(records, 210, 1, 2, 3, 15, 16, 1);
	setDialogueRecord(records, 211, 1, 2, 3, 16, 17, 1);
	setDialogueRecord(records, 212, 1, 0, 2, 17, 18, 0);

	setDialogueRecord(records, 280, 1, 3, 3, 18, 19, 1);
	setDialogueRecord(records, 281, 1, 3, 3, 19, 20, 1);
	setDialogueRecord(records, 282, 1, 3, 3, 20, 21, 1);
	setDialogueRecord(records, 283, 1, 0, 2, 21, 22, 0);
}

void Scene3090::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
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

void Scene3090::beginBlindManResponse(byte frameIndex) {
	_vm->gameplayMusic()->stop();
	beginBlindManSpeechAnimation();
	beginPrimarySpeechLine(kScene3090DialoguePrimaryRow, frameIndex,
		kScene3090PrimarySpeechCenterX, kScene3090PrimarySpeechTopY,
		kScene3090PrimarySpeechRed, kScene3090PrimarySpeechGreen, kScene3090PrimarySpeechBlue);
	endBlindManSpeechAnimation();
}

void Scene3090::runSaltShakerPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene3090SaltShakerTaken) {
		beginSecondarySpeechLine(8, 1);
		return;
	}

	state.scene3090SaltShakerTaken = true;
	runActorReplacement(ActionOverlaySpec(16, kScene3090PickupDescriptorCount,
		kScene3090PickupFrameMap, ARRAYSIZE(kScene3090PickupFrameMap), kScene3090OverlayFrameMillis));
	applySceneStateToHotspotsAndPatches(4);
	addInventoryItem(kScene3090SaltShakerItemId);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene3090::runDowsingRodPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene3090DowsingRodTaken) {
		beginSecondarySpeechLine(9, 0);
		return;
	}

	state.scene3090DowsingRodTaken = true;
	runActorReplacement(ActionOverlaySpec(16, kScene3090PickupDescriptorCount,
		kScene3090PickupFrameMap, ARRAYSIZE(kScene3090PickupFrameMap), kScene3090OverlayFrameMillis));
	applySceneStateToHotspotsAndPatches(5);
	addInventoryItem(kScene3090DowsingRodItemId);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene3090::runUseStrawInFireplace() {
	GameplayState &state = _vm->gameState();
	if (!hasInventoryItem(kScene3090StrawItemId)) {
		dispatchGenericSceneAction(2);
		return;
	}

	beginSecondarySpeechLine(10, 0);
	runActorReplacement(ActionOverlaySpec(14, kScene3090StrawDescriptorCount,
		kScene3090StrawFrameMap, ARRAYSIZE(kScene3090StrawFrameMap), kScene3090OverlayFrameMillis)
		.soundAt(7, 1));
	removeInventoryItem(kScene3090StrawItemId);
	waitSceneMillis(750);
	_vm->gameplayMusic()->stop();
	beginBlindManSpeechAnimation();
	beginPrimarySpeechLine(10, 1, kScene3090PrimarySpeechCenterX, kScene3090PrimarySpeechTopY,
		kScene3090PrimarySpeechRed, kScene3090PrimarySpeechGreen, kScene3090PrimarySpeechBlue);
	endBlindManSpeechAnimation();
	state.scene3080WindowOpened = true;
	state.scene3080ChimneySmokeAnimationChanged = true;
	state.scene3090WindowOpenSequenceState = 1;
	_puzzleChannel.frameIndex = 0x1b;
	_puzzleLayer.setFrame(_puzzleChannel.frameIndex);
	_puzzleLayerTriggered = true;
	applySceneStateToHotspotsAndPatches(2);
	setBlindManPostConversationFrame();
}

void Scene3090::runSaxophoneHandoff() {
	GameplayState &state = _vm->gameState();
	if (!hasInventoryItem(kScene3090SaxophoneItemId) || state.scene3090BlindManPlayingSaxophone) {
		dispatchGenericSceneAction(13);
		return;
	}

	walkActiveActorTo(0x178, 0x171, 5, 0, false);
	beginSecondarySpeechLine(11, 0);
	beginBlindManResponse(1);
	beginSecondarySpeechLine(11, 1);
	runActorReplacement(ActionOverlaySpec(15, kScene3090SaxophoneDescriptorCount,
		kScene3090SaxophoneFrameMap, ARRAYSIZE(kScene3090SaxophoneFrameMap), kScene3090OverlayFrameMillis));
	removeInventoryItem(kScene3090SaxophoneItemId);
	state.scene3090BlindManPlayingSaxophone = true;
	state.currentAmbientMusicCueId = 0x11;
	_blindManChannel.frameIndex = 0x2b;
	_blindManLayer.setFrame(_blindManChannel.frameIndex);
	_puzzleChannel.frameIndex = (byte)(state.scene3090SecretDiaryPuzzleProgress * 3);
	_puzzleLayer.setFrame(_puzzleChannel.frameIndex);
	applySceneStateToHotspotsAndPatches(1);
	if (_vm->restoredContentEnabled())
		beginSecondarySpeechLine(11, 2);
}

void Scene3090::drawForegroundBlocks(int activeWorldY) {
	uint chunkIndex = 17;
	if (activeWorldY < 0x14b)
		chunkIndex = _vm->gameState().scene3090SaltShakerTaken ? 19 : 18;
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

} // End of namespace Hollywood
