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

#include "hollywood/scenes/playable/scene4110.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene4110SecondEntryState = 0x100f;
const uint16 kScene4010EntryFromRightSideState = 0x0fab;
const uint16 kScene4100FirstState = 0x1004;
const uint16 kScene4110ViewportXOffset = 0x00c0;
const uint kScene4110ActorBankTableEntry = 0x0000;
const uint kScene4110ActorPaletteTableEntry = 0x00cc;
const uint kScene4110Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4110SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4110FrameMillis = 60;
const uint32 kScene4110BackgroundFrameMillis = 75;
const uint32 kScene4110BridgeShakeFrameMillis = 25;
const uint32 kScene4110AmbientCheckMillis = 250;
const byte kScene4110LoopAmbientFirstCue = 0x0b;
const byte kScene4110LoopAmbientCueCount = 3;
const byte kScene4110LoopAmbientVolumePercent = 20;
const byte kScene4110SpecialAmbientCue = 0x0e;
const byte kScene4110AmbientFirstCue = 0x0f;
const byte kScene4110AmbientCueCount = 8;
const byte kScene4110AmbientVolumePercent = 25;
const uint kScene4110PickupChunk = 5;
const uint kScene4110PickupDescriptorCount = 0x0c;
const uint kScene4110AlternateOverlayChunk = 6;
const uint kScene4110AlternateOverlayDescriptorCount = 0x0d;
const uint kScene4110AlternatePatchChunk = 7;
const uint kScene4110AlternateFinalPatchChunk = 8;
const uint kScene4110BackgroundChunk = 9;
const uint kScene4110BackgroundDescriptorCount = 10;
const uint kScene4110BackgroundLayerIndex = 0;
const uint kScene4110BridgeShakeLayerIndex = 1;
const uint kScene4110BridgeMainLayerIndex = 2;
const uint kScene4110CastleInteriorLookVerbRecordIndex = 0x2b;
const byte kScene4110StrawItem = 0x46;
const byte kScene4110BridgeShakeStartFrame = 10;
const byte kScene4110BridgeShakeMiddleFrame = 12;
const byte kScene4110BridgeShakeEndFrame = 11;
const byte kScene4110BridgeShakeRepeatCount = 40;
const byte kScene4110BridgeShakeStartOverlayFrame = 0x1a;
const byte kScene4110BridgeSoundOverlayFrame = 0x1c;
const byte kScene4110BridgeSoundCue = 0x31;
const byte kScene4110BridgeFinalSoundCue = 0x3b;

const byte kScene4110BackgroundFrameMap[] = {
	0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 2, 1, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 3, 2, 1,
	0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const byte kScene4110BackgroundSequenceLengths[] = {
	1, 3, 5, 11, 2
};

const byte kScene4110PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene4110AlternateFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 5, 6, 6, 6,
	6, 6, 5, 6, 6, 6, 6, 6, 5, 6, 7, 8, 9
};

static PlayableSceneConfig scene4110Config() {
	PlayableSceneConfig config(4110,
		SceneResourceLayout(5, 5, 9),
		SceneViewport(kScene4110ViewportXOffset, kScene4110ViewportXOffset, kScene4110ViewportXOffset),
		SceneActorPose(0x0118, 0x0180, 2));
	config.setActorResources(kScene4110ActorBankTableEntry, kScene4110ActorPaletteTableEntry);
	config.setTextResources(kScene4110Resource003RowsOffsetIndex, kScene4110SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	return config;
}

Scene4110::Scene4110(HollywoodEngine *vm) :
		PlayableScene(vm, scene4110Config()),
		_backgroundChannel(),
		_backgroundSequence(0),
		_backgroundFrameInSequence(0),
		_backgroundRepeatCounter(0),
		_bridgeSequenceActive(false),
		_ambientSoundTimerAccumulator(0),
		_lastAmbientLoopCue(0xff),
		_previousAmbientSoundCue(0xff) {
	_sceneLayers.configureLayer(kScene4110BackgroundLayerIndex, kSceneAnimationScenePlaced,
		kScene4110BackgroundChunk,
		kScene4110BackgroundDescriptorCount, nullptr, 0);
	_sceneLayers.configureLayer(kScene4110BridgeMainLayerIndex, kSceneAnimationScenePlaced,
		kScene4110AlternateOverlayChunk,
		kScene4110AlternateOverlayDescriptorCount,
		kScene4110AlternateFrameMap, ARRAYSIZE(kScene4110AlternateFrameMap));
	_sceneLayers.configureLayer(kScene4110BridgeShakeLayerIndex, kSceneAnimationScenePlaced,
		kScene4110AlternateOverlayChunk,
		kScene4110AlternateOverlayDescriptorCount, nullptr, 0);
}

void Scene4110::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetBackgroundLayer();
	resetAmbientSounds();
	applySceneStateToHotspotsAndPatches(0xff);

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene4110SecondEntryState) {
		_activeActorWorldX = 0x0253;
		_activeActorWorldY = 0x00f2;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x0051;
		_activeActorWorldY = 0x01a2;
		_activeActorFacing = 2;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene4110::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene4110::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	restoreSceneLayerBackground(kScene4110BackgroundLayerIndex, _baseFramebuffer);

	if (_bridgeSequenceActive) {
		restoreSceneLayerBackground(kScene4110BridgeShakeLayerIndex, _baseFramebuffer);
		restoreSceneLayerBackground(kScene4110BridgeMainLayerIndex, _baseFramebuffer);
		drawSceneLayer(kScene4110BridgeShakeLayerIndex);
		drawSceneLayer(kScene4110BackgroundLayerIndex);
		drawSceneLayer(kScene4110BridgeMainLayerIndex);
		return;
	}

	if (_actionOverlayPlayer.replacesActor()) {
		restoreResourceSpriteLayerBackground(_actionOverlayPlayer.layer, _baseFramebuffer);
		drawActionOverlayLayer();
		drawSceneLayer(kScene4110BackgroundLayerIndex);
		return;
	}

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawSceneLayer(kScene4110BackgroundLayerIndex);
	drawActionOverlayLayer();
}

void Scene4110::runCustomEntrySequence() {
	resetBackgroundLayer();
	resetAmbientSounds();
	applySceneStateToHotspotsAndPatches(0xff);

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene4110SecondEntryState) {
		if (revealEntryPose(0x0253, 0x00f2, 4))
			walkActiveActorTo(0x0168, 0x015e, 0xff, 0, false);
	} else {
		if (revealEntryPose(0x0051, 0x01a2, 2))
			walkActiveActorTo(0x0118, 0x0180, 0xff, 0, false);
	}
}

bool Scene4110::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void Scene4110::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
	stopAmbientSoundCues();
}

void Scene4110::prepareCustomGameplayLoop() {
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4110::advanceCustomGameplayLoop(uint32 delta) {
	advanceAmbientSounds(delta);
	advanceBackgroundLayer(delta);
}

bool Scene4110::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir al puente levadizo (go to drawbridge): leave only after it is lowered.
		if (bridgeOpened())
			runExitToScene4010();
		else
			beginSecondarySpeechLine(0, 0);
		return true;
	case 302: // Mirar puente levadizo (look at drawbridge): describe its current position.
		beginSecondarySpeechLine(1, bridgeOpened() ? 1 : 0);
		return true;
	case 303: // Abrir puente levadizo (open drawbridge): the wheel is required while it is raised.
		beginConditionalSpeechLine(2, 0, 7, 0);
		return true;
	case 304: // Cerrar puente levadizo (close drawbridge): it cannot be raised directly.
		beginConditionalSpeechLine(10, 0, 2, 0);
		return true;
	case 305: // Coger paja (take straw): item 0x46.
		takeStraw();
		return true;
	case 306: // Mirar paja (look at straw).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Mirar carro (look at cart).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Usar carro (use cart): Ron cannot move it alone.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Coger/usar escalera (take/use ladder): the cart blocks it.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar escalera (look at ladder).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Ir al interior del castillo (go inside castle): return to Scene 4100.
		_vm->gameState().mainFlowStateId = kScene4100FirstState;
		return true;
	case 312: // Mirar interior del castillo (look inside castle): dormant in original metadata.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 313: // Mirar rueda (look at wheel).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Usar rueda (use wheel): lower the drawbridge.
		runAlternateStateSequence();
		return true;
	default:
		return false;
	}
}

bool Scene4110::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	if (bridgeOpened() &&
			_sceneChunkTable.isValidChunk(kScene4110AlternateFinalPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4110AlternateFinalPatchChunk], _baseFramebuffer);

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (_vm->restoredContentEnabled())
		_hotspots.setVerbActionHandlerByGlobalRecordIndex(
			kScene4110CastleInteriorLookVerbRecordIndex, 312);
	patchActionMovementModes();
	return true;
}

AmbientAudioProfile Scene4110::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene4110AmbientCheckMillis;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

bool Scene4110::bridgeOpened() const {
	const GameplayState &state = _vm->gameState();
	return state.scene4110BridgeOpened || state.scene4010AlternateBackgroundState != 0;
}

bool Scene4110::revealEntryPose(int x, int y, byte facing) {
	setActiveActorPose(x, y, facing);
	drawPlayableComposite();
	return !fadePaletteFromBlack();
}

void Scene4110::resetBackgroundLayer() {
	_sceneLayers.setLayerVisible(kScene4110BackgroundLayerIndex, true);
	_sceneLayers.setLayerFrame(kScene4110BackgroundLayerIndex, 0);
	_sceneLayers.setLayerVisible(kScene4110BridgeMainLayerIndex, false);
	_sceneLayers.setLayerFrame(kScene4110BridgeMainLayerIndex, 0);
	_sceneLayers.setLayerVisible(kScene4110BridgeShakeLayerIndex, false);
	_sceneLayers.setLayerFrame(kScene4110BridgeShakeLayerIndex, kScene4110BridgeShakeStartFrame);
	_bridgeSequenceActive = false;
	_backgroundChannel.reset(0, kScene4110BackgroundFrameMillis);
	_backgroundSequence = 0;
	_backgroundFrameInSequence = 0;
	_backgroundRepeatCounter = 0;
}

void Scene4110::advanceBackgroundLayer(uint32 delta) {
	const uint frameCount = _backgroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		advanceBackgroundTick();
}

void Scene4110::advanceBackgroundTick() {
	if (_backgroundRepeatCounter == 0) {
		if (_random.getRandomNumber(49) == 0) {
			_backgroundSequence = (byte)_random.getRandomNumber(3);
			_backgroundRepeatCounter = (byte)(_random.getRandomNumber(9) + 1);
			_backgroundFrameInSequence = 1;
		} else if (_random.getRandomNumber(19) == 0) {
			_backgroundSequence = 4;
			_backgroundRepeatCounter = 1;
			_backgroundFrameInSequence = 1;
			_soundBank0.playSample(0x24, 25);
		} else {
			_sceneLayers.setVisibleLayerFrame(kScene4110BackgroundLayerIndex, 0);
			return;
		}
	} else if (_backgroundFrameInSequence >= kScene4110BackgroundSequenceLengths[_backgroundSequence]) {
		--_backgroundRepeatCounter;
		_backgroundSequence = _backgroundRepeatCounter == 0 ? 0 : (byte)_random.getRandomNumber(3);
		_backgroundFrameInSequence = 0;
	}

	const uint frameMapOffset = (uint)_backgroundSequence * 12 + _backgroundFrameInSequence;
	_sceneLayers.setVisibleLayerFrame(kScene4110BackgroundLayerIndex,
		kScene4110BackgroundFrameMap[frameMapOffset]);
	++_backgroundFrameInSequence;
}

void Scene4110::resetAmbientSounds() {
	_ambientSoundTimerAccumulator = 0;
	_lastAmbientLoopCue = 0xff;
	_previousAmbientSoundCue = 0xff;
}

void Scene4110::advanceAmbientSounds(uint32 delta) {
	updateAmbientLoopSound();
	_ambientSoundTimerAccumulator += delta;
	while (_ambientSoundTimerAccumulator >= kScene4110AmbientCheckMillis) {
		_ambientSoundTimerAccumulator -= kScene4110AmbientCheckMillis;
		SoundBank0Player &player = _additionalAmbientSoundBank0Slots[1];
		if (player.isPlaying() || _random.getRandomNumber(24) != 0)
			continue;

		if (_random.getRandomNumber(9) == 0) {
			player.playSample(kScene4110SpecialAmbientCue, 100);
			continue;
		}

		byte cue = _previousAmbientSoundCue;
		while (cue == _previousAmbientSoundCue)
			cue = (byte)(kScene4110AmbientFirstCue + _random.getRandomNumber(kScene4110AmbientCueCount - 1));
		_previousAmbientSoundCue = cue;
		player.playSample(cue, kScene4110AmbientVolumePercent);
	}
}

void Scene4110::updateAmbientLoopSound() {
	SoundBank0Player &player = _additionalAmbientSoundBank0Slots[0];
	if (player.isPlaying())
		return;

	byte cue = _lastAmbientLoopCue;
	while (cue == _lastAmbientLoopCue)
		cue = (byte)(kScene4110LoopAmbientFirstCue +
			_random.getRandomNumber(kScene4110LoopAmbientCueCount - 1));
	_lastAmbientLoopCue = cue;
	player.playSample(cue, kScene4110LoopAmbientVolumePercent);
}

void Scene4110::beginConditionalSpeechLine(uint16 falseRow, byte falseFrame, uint16 trueRow, byte trueFrame) {
	if (bridgeOpened())
		beginSecondarySpeechLine(trueRow, trueFrame);
	else
		beginSecondarySpeechLine(falseRow, falseFrame);
}

void Scene4110::runExitToScene4010() {
	GameplayState &state = _vm->gameState();
	walkActiveActorTo(0x0253, 0x00f2, 0xff, 0);
	state.mainFlowStateId = kScene4010EntryFromRightSideState;
}

void Scene4110::takeStraw() {
	GameplayState &state = _vm->gameState();
	if (state.scene4110StrawTaken || hasInventoryItem(kScene4110StrawItem)) {
		beginSecondarySpeechLine(3, 1);
		return;
	}

	beginSecondarySpeechLine(3, 0);
	runActorReplacement(ActionOverlaySpec(kScene4110PickupChunk, kScene4110PickupDescriptorCount,
		kScene4110PickupFrameMap, ARRAYSIZE(kScene4110PickupFrameMap), kScene4110FrameMillis)
		.frameRange(1, ARRAYSIZE(kScene4110PickupFrameMap)));
	addInventoryItem(kScene4110StrawItem);
	_soundBank0.playSample(1, 100);
	state.scene4110StrawTaken = true;
	applySceneStateToHotspotsAndPatches(0);
}

void Scene4110::runAlternateStateSequence() {
	GameplayState &state = _vm->gameState();
	if (bridgeOpened()) {
		beginSecondarySpeechLine(7, 0);
		return;
	}

	if (_sceneChunkTable.isValidChunk(kScene4110AlternatePatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4110AlternatePatchChunk], _baseFramebuffer);
	drawPlayableComposite();
	presentFrame();

	runBridgeOpeningOverlay();

	if (_sceneChunkTable.isValidChunk(kScene4110AlternateFinalPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4110AlternateFinalPatchChunk], _baseFramebuffer);

	const Common::Array<byte> savedPaletteRegionMask = _fullPaletteRegionMask;
	const Common::Array<byte> savedWalkablePaletteMask = _walkablePaletteMask;
	memset(_fullPaletteRegionMask.data(), 1, _fullPaletteRegionMask.size());
	memset(_walkablePaletteMask.data(), 1, _walkablePaletteMask.size());
	setActiveActorPose(0x0221, 0x011c, 4);
	walkActiveActorTo(0x01e6, 0x011c, 5, 0);
	_fullPaletteRegionMask = savedPaletteRegionMask;
	_walkablePaletteMask = savedWalkablePaletteMask;

	state.scene4010AlternateBackgroundState = 1;
	if (state.scene4010Item3APickupState == 0)
		state.scene4010Item3APickupState = 1;
	state.scene4110BridgeOpened = true;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4110::runBridgeOpeningOverlay() {
	_bridgeSequenceActive = true;
	_sceneLayers.setLayerVisible(kScene4110BridgeMainLayerIndex, true);
	_sceneLayers.setLayerFrame(kScene4110BridgeMainLayerIndex, 0);
	_sceneLayers.setLayerVisible(kScene4110BridgeShakeLayerIndex, true);
	_sceneLayers.setLayerFrame(kScene4110BridgeShakeLayerIndex, kScene4110BridgeShakeStartFrame);
	drawPlayableComposite();
	presentFrame();

	uint mainFrame = 0;
	uint32 mainFrameAccumulator = 0;
	uint32 shakeFrameMillis = kScene4110BridgeShakeFrameMillis;
	byte shakeFrame = kScene4110BridgeShakeStartFrame;
	byte shakeRepeatCounter = 0;

	while (mainFrame < ARRAYSIZE(kScene4110AlternateFrameMap) - 1 || shakeRepeatCounter != 0) {
		if (waitSceneMillis(shakeFrameMillis))
			break;

		bool frameChanged = false;
		if (mainFrame < ARRAYSIZE(kScene4110AlternateFrameMap) - 1) {
			mainFrameAccumulator += shakeFrameMillis;
			while (mainFrameAccumulator >= kScene4110FrameMillis &&
					mainFrame < ARRAYSIZE(kScene4110AlternateFrameMap) - 1) {
				mainFrameAccumulator -= kScene4110FrameMillis;
				++mainFrame;
				_sceneLayers.setVisibleLayerFrame(kScene4110BridgeMainLayerIndex, (byte)mainFrame);
				frameChanged = true;
				if (mainFrame == kScene4110BridgeShakeStartOverlayFrame)
					shakeRepeatCounter = kScene4110BridgeShakeRepeatCount;
				if (mainFrame == kScene4110BridgeSoundOverlayFrame)
					_soundBank0.playSample(kScene4110BridgeSoundCue, 100);
			}
		}

		if (shakeRepeatCounter != 0) {
			if (shakeFrame == kScene4110BridgeShakeStartFrame) {
				shakeFrame = kScene4110BridgeShakeMiddleFrame;
			} else if (shakeFrame == kScene4110BridgeShakeMiddleFrame) {
				shakeFrame = kScene4110BridgeShakeEndFrame;
			} else {
				shakeFrame = kScene4110BridgeShakeStartFrame;
				if (shakeRepeatCounter == 15)
					_soundBank0.playSample(kScene4110BridgeFinalSoundCue, 100);
				--shakeRepeatCounter;
				if (shakeRepeatCounter < 7)
					shakeFrameMillis += 7;
			}
			_sceneLayers.setVisibleLayerFrame(kScene4110BridgeShakeLayerIndex, shakeFrame);
			frameChanged = true;
		}

		if (frameChanged) {
			drawPlayableComposite();
			presentFrame();
		}
	}

	_bridgeSequenceActive = false;
	_sceneLayers.setLayerVisible(kScene4110BridgeMainLayerIndex, false);
	_sceneLayers.setLayerVisible(kScene4110BridgeShakeLayerIndex, false);
}

void Scene4110::patchActionMovementModes() {
	if (_vm->gameState().scene4110StrawTaken)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x13, 0);
	if (bridgeOpened())
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x35, 0);
}

} // End of namespace Hollywood
