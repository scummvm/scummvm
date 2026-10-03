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

#include "graphics/pixelformat.h"

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/scenes/playable/scene4010.h"
#include "hollywood/scenes/shared_frame_sequences.h"

namespace Hollywood {

const uint16 kScene4010EntryFromRightSideState = 0x0fab;
const uint16 kScene4010EntryFromLeftSideState = 0x0fac;
const uint16 kScene4010ExitState4020 = 0x0fb4;
const uint16 kScene4010ExitState4110 = 0x100f;
const uint16 kScene4010DemoExitState4030 = 0x0fbe;
const uint16 kScene4010DemoExitState4100 = 0x1004;
const uint16 kScene4010ViewportXOffset = 0x0068;
const uint16 kScene4010ViewportMaxXOffset = 0x00b8;
const uint kScene4010ActorBankTableEntry = 0x0000;
const uint kScene4010ActorPaletteTableEntry = 0x00cc;
const uint kScene4010Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4010SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4010RoomIdleFrameMillis = 75;
const uint32 kScene4010OverlayFrameMillis = 75;
const uint32 kScene4010PaletteCycleMillis = 300;
const uint32 kScene4010FirstEditionPaletteCycleMillis = 100;
const uint32 kScene4010AmbientCheckMillis = 250;
const uint32 kScene4010DemoAmbientCheckMillis = 10;
const uint kScene4010VerbActionRecordSize = 4;
const uint kScene4010HeckerUseVerbRecordIndex = 0x25;
const uint kScene4010RoomIdleDescriptorCount = 0x14;
const uint kScene4010ExitOverlayDescriptorCount = 0x13;
const uint kScene4010Item3AOverlayDescriptorCount = 0x0e;
const uint kScene4010DestinationOverlayDescriptorCount = 0x11;
const uint kScene4010PillboxOverlayDescriptorCount = 0x0d;
const uint kScene4010HeckerDialogueChoiceRecordCount = 700;
const byte kScene4010Item3A = 0x3a;
const byte kScene4010PillboxItem = 0x3b;
const byte kScene4010DemoLeverItem = 0x35;
const byte kScene4010AustraliaDestinationId = 4;
const byte kScene4010HeckerSpeechGroup = 0;
const byte kScene4010InvalidPrimarySpeechGroup = 0xff;
const byte kScene4010DefaultPrimarySpeechFrame = 7;
const byte kScene4010PrimarySpeechTextColor = 0xfb;
const byte kScene4010HeckerDialogueStageId = 0x62;
const byte kScene4010HeckerDialogueResponseRow = 99;
const uint16 kScene4010HeckerSpeechCenterX = 0x0154;
const uint16 kScene4010HeckerSpeechTopY = 0x00be;
const byte kScene4010HeckerSpeechRed = 0x20;
const byte kScene4010HeckerSpeechGreen = 0x30;
const byte kScene4010HeckerSpeechBlue = 0x3f;
const byte kScene4010PaletteCycleFirstColor = 0x80;
const byte kScene4010PaletteCycleLastColor = 0x9f;
const uint kScene4010RoomIdleLayer = 0;

struct Scene4010ReleaseProfile {
	uint16 drawbridgeExitState;
	uint16 moatExitState;
	byte thrownItemId;
	byte heckerLoopEndFrame;
	byte heckerLoopRestartFrame;
	byte secondaryAmbientProbabilityModulus;
	uint32 secondaryAmbientCheckMillis;
	uint32 paletteCycleMillis;
	byte framebufferChunkIndex;
	bool usesReducedFirstEntry;
	bool usesDirectRightEntry;
	bool usesDemoThrownItemLayout;
};

const Scene4010ReleaseProfile kScene4010FullGameProfile = {
	kScene4010ExitState4110, kScene4010ExitState4020, kScene4010PillboxItem,
	0x0c, 8, 25, kScene4010AmbientCheckMillis, kScene4010PaletteCycleMillis, 0, false, false, false
};

const Scene4010ReleaseProfile kScene4010FirstEditionProfile = {
	kScene4010ExitState4110, kScene4010ExitState4020, kScene4010PillboxItem,
	0x0c, 8, 25, kScene4010AmbientCheckMillis, kScene4010FirstEditionPaletteCycleMillis, 0, false, false, false
};

const Scene4010ReleaseProfile kScene4010SpanishDemoProfile = {
	kScene4010DemoExitState4100, kScene4010DemoExitState4030, kScene4010DemoLeverItem,
	0x0c, 8, 25, kScene4010AmbientCheckMillis, kScene4010PaletteCycleMillis, 5, false, true, true
};

const Scene4010ReleaseProfile kScene4010ItalianDemoProfile = {
	kScene4010DemoExitState4100, kScene4010DemoExitState4030, kScene4010DemoLeverItem,
	0x0f, 0x0b, 100, kScene4010DemoAmbientCheckMillis, kScene4010PaletteCycleMillis, 0, true, true, true
};

const Scene4010ReleaseProfile &scene4010ReleaseProfile(const HollywoodEngine *vm) {
	if (!vm->isDemo()) {
		if (vm->isFirstEdition())
			return kScene4010FirstEditionProfile;
		return kScene4010FullGameProfile;
	}
	return vm->getLanguage() == Common::IT_ITA ?
		kScene4010ItalianDemoProfile : kScene4010SpanishDemoProfile;
}

const byte kScene4010RoomIdleFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 10, 11, 12, 0, 1, 2, 3, 13, 14,
	15, 15, 16, 17, 18, 19, 15, 14, 13, 3,
	2, 1, 0, 0
};

const byte kScene4010HeckerResponsePoseFrames[] = {
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e
};

const SceneLayerSpec kScene4010LayerSpecs[] = {
	{kSceneAnimationBehindActors, 6, kScene4010RoomIdleDescriptorCount,
		kScene4010RoomIdleFrameMap, ARRAYSIZE(kScene4010RoomIdleFrameMap), true, 0}
};

PlayableSceneConfig scene4010Config() {
	PlayableSceneConfig config(4010,
		SceneResourceLayout(5, 6, 17),
		SceneViewport(kScene4010ViewportXOffset, kScene4010ViewportXOffset, kScene4010ViewportMaxXOffset),
		SceneActorPose(0x11a, 0x18d, 1));
	config.setActorResources(kScene4010ActorBankTableEntry, kScene4010ActorPaletteTableEntry);
	config.setTextResources(kScene4010Resource003RowsOffsetIndex, kScene4010SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene4010::Scene4010(HollywoodEngine *vm) :
		PlayableScene(vm, scene4010Config()),
		_releaseProfile(scene4010ReleaseProfile(vm)),
		_roomIdleChannel(),
		_paletteCycleChannel(),
		_secondaryAmbientChannel(),
		_normalBaseFramebuffer(),
		_normalBaseFramebufferInitialized(false),
		_heckerAnimationState(0),
		_heckerLoopCount(0),
		_previousSecondaryAmbientCue(0),
		_heckerAlternateSpeechPose(false),
		_heckerManualSequenceActive(false),
		_heckerPoseTransitionPending(false),
		_roomAnimationPaused(false) {
	_sceneLayers.configure(kScene4010LayerSpecs);
	_normalBaseFramebuffer.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		Graphics::PixelFormat::createFormatCLUT8());
}

uint Scene4010::framebufferResourceChunkIndex() const {
	return _releaseProfile.framebufferChunkIndex;
}

void Scene4010::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	initializeRoomIdleLayer();
	resetPaletteCycle();
	_secondaryAmbientChannel.reset(0, _releaseProfile.secondaryAmbientCheckMillis);
	_previousSecondaryAmbientCue = 0;

	switch (_vm->gameState().mainFlowStateId) {
	case kScene4010EntryFromRightSideState:
		setActiveActorPose(0xf5, 0x169, 1);
		break;
	case kScene4010EntryFromLeftSideState:
		setActiveActorPose(0x1ad, 0x1ce, 4);
		break;
	default:
		setActiveActorPose(0x11a, 0x18d, 1);
		break;
	}
}

void Scene4010::drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) {
	(void)activeWorldX;
	(void)actorDrawOrderMode;
	drawForegroundBlocks(activeWorldY);
}

void Scene4010::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene4010EntryFromRightSideState:
		runEntryFromRightSide();
		break;
	case kScene4010EntryFromLeftSideState:
		runEntryFromLeftSide();
		break;
	default:
		runFirstEntrySequence();
		break;
	}
}

void Scene4010::advanceCustomGameplayLoop(uint32 delta) {
	updateRoomAmbientAudio(delta);
	if (!_roomAnimationPaused)
		advancePaletteCycle(delta);
	if (!_roomAnimationPaused && !alternateBackgroundActive() && _sceneChunkTable.isValidChunk(6) &&
			!_primaryDialogueSpeechActive &&
			!_heckerManualSequenceActive)
		advanceHeckerIdleLayer(delta);
}

bool Scene4010::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a puente levadizo (go to drawbridge), or return from the opened-bridge view.
		if (alternateBackgroundActive())
			_vm->gameState().mainFlowStateId = _releaseProfile.drawbridgeExitState;
		else
			beginSecondarySpeechLine(0, 0);
		return true;
	case 302: // Mirar puente levadizo (look at drawbridge), state-aware.
		beginD01SpeechLine(1, 0, 1);
		return true;
	case 303: // Usar puente levadizo (use drawbridge): still raised.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Abrir puente levadizo (open drawbridge), state-aware.
		if (alternateBackgroundActive())
			beginSecondarySpeechLine(7, 0);
		else
			beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar ventana (look at window): too far to see.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Usar/cerrar ventana (use/close window): too far to reach.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Ir a foso (go to moat): progressive exit into the castle.
		runProgressiveExitSpeech();
		return true;
	case 308: // Mirar foso (look at moat): state-aware moat response.
		beginSecondarySpeechLine(7, _vm->gameState().scene4010ProgressiveExitSpeechState < 2 ? 0 : 1);
		return true;
	case 309: // Hablar con viejo algo trastornado / Dr. Hecker (talk to disturbed old man / Dr. Hecker).
		if (alternateBackgroundActive())
			beginSecondarySpeechLine(10, 0);
		else
			runHeckerDialogue();
		return true;
	case 310: // Intentar coger viejo algo trastornado / Dr. Hecker (try to take Dr. Hecker): refusal.
		beginD01SpeechLine(8, 0, 1);
		return true;
	case 311: // Mirar viejo algo trastornado / Dr. Hecker (look at disturbed old man / Dr. Hecker).
		beginD01SpeechLine(9, 0, 1);
		return true;
	case 312: // Restored: Usar Dr. Hecker (use Dr. Hecker), including while he is trapped under the bridge.
		if (alternateBackgroundActive())
			beginSecondarySpeechLine(8, 1);
		else
			beginSecondarySpeechLine(11, 0);
		return true;
	case 313: // Coger objeto brillante / camafeo de Samarkanda (take shiny object / Samarkanda cameo).
		takeAnimatedItem3A();
		return true;
	case 314: // Mirar objeto brillante / camafeo de Samarkanda (look at shiny object / Samarkanda cameo).
		handlePendingItem3A();
		return true;
	case 315: // Coger postal (take postal letter): trapped under the bridge.
		beginSecondarySpeechLine(14, 0);
		return true;
	case 316: // Mirar postal (look postal letter): unlock Australia destination when the clue is known.
		unlockDestinationFromRoomAction();
		return true;
	case 317: // Coger objeto arrojado (take thrown item): pillbox in full game, lever in demo.
		takeThrownItem();
		return true;
	case 318: // Mirar objeto arrojado (look at thrown item).
		beginSecondarySpeechLine(16, 0);
		return true;
	case 319: // Usar coche (use car): open Ron's destination selector.
		requestTravelScreenSelection(4);
		return true;
	default:
		return false;
	}
}

bool Scene4010::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	const int maxY = alternateBackgroundActive() ? 0x206 : 0x1df;
	if (targetY > maxY)
		targetY = maxY;
	return false;
}

bool Scene4010::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 3 && nextRegion == 4) {
		copyStepDeltas(0x0c, 0x0c, 0x0c);
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 3 && nextRegion == 2) {
		copyStepDeltas(0x30, 0x0c, 0x0c);
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene4010::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	if (selector == 0xff) {
		ensureNormalBaseFramebuffer();
		applyD01BackgroundForCurrentState();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		if (_releaseProfile.usesDemoThrownItemLayout)
			removeColorMapItem(0x0b);
	}

	GameplayState &state = _vm->gameState();

	if (selector == 0 || selector == 0xff) {
		if (alternateBackgroundActive()) {
			copySmallRow(0x171, 0x29);
			setScenePoint(kSceneItemInteractionPoints, 1, 0x02c4, 0x00f3);
			setScenePoint(kSceneItemInteractionPoints, 4, 0x00f5, 0x0169);
			setScenePoint(kSceneItemApproachPoints, 4, 0x0143, 0x0153);
			clearVerbActionRecord(0x0b);
			clearVerbActionRecord(0x0d);
			clearVerbActionRecord(0x0e);
			clearVerbActionRecord(0x0f);
			replaceColorMapItem(4, 0);
			replaceColorMapItem(9, 4);
			replaceColorMapItem(10, 4);
		} else {
			removeColorMapItem(6);
			removeColorMapItem(5);
			removeColorMapItem(9);
			replaceColorMapItem(10, 4);
		}
	}
	if ((selector == 2 || selector == 0xff) && state.scene4010EntryPathSpeechState == 1)
		copySmallRow(0x1c3, 0xa4);
	if (selector == 3 || selector == 0xff) {
		if (state.scene4010Item3APickupState == 1 && _sceneChunkTable.isValidChunk(7)) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
		} else if (state.scene4010Item3APickupState == 2) {
			copySmallRow(0x19a, 0xcd);
			if (_sceneChunkTable.isValidChunk(7))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
		} else if (state.scene4010Item3APickupState == 3) {
			if (_sceneChunkTable.isValidChunk(8))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
			replaceColorMapItem(5, 4);
		}
	}
	if (selector == 5 || selector == 0xff) {
		if (state.scene4010PillboxPickupState == 1) {
			// The Spanish demo already has the branch in its background.
			const bool spanishDemo = _vm->isDemo() && _vm->getLanguage() == Common::ES_ESP;
			if (!spanishDemo && _sceneChunkTable.isValidChunk(10))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
		} else {
			removeColorMapItem(7);
			if (_releaseProfile.usesDemoThrownItemLayout)
				applyBaseFramebufferPatch(11);
		}
	}

	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (alternateBackgroundActive()) {
			if (_walkablePaletteMask[i] == 3 || _walkablePaletteMask[i] == 5)
				_walkablePaletteMask[i] = 0;
		} else if (_walkablePaletteMask[i] > 1) {
			_walkablePaletteMask[i] = 0;
		}
	}

	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (_vm->restoredContentEnabled()) {
		_hotspots.setVerbActionHandlerByGlobalRecordIndex(kScene4010HeckerUseVerbRecordIndex, 312);
		_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene4010HeckerUseVerbRecordIndex, 0);
	}
	_sceneLayers.setLayerVisible(kScene4010RoomIdleLayer,
		!alternateBackgroundActive() && _sceneChunkTable.isValidChunk(6));
	return true;
}

AmbientAudioProfile Scene4010::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene4010AmbientCheckMillis;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

byte Scene4010::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return _heckerAlternateSpeechPose ? 0x14 : 0x1f;
}

byte Scene4010::primarySpeechAnimationFrameCount(byte animationGroup) const {
	(void)animationGroup;
	return _heckerAlternateSpeechPose ? 4 : 5;
}

void Scene4010::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	if (_heckerAlternateSpeechPose && frameIndex > 0x17)
		frameIndex = 0x14;
	setHeckerFrame(frameIndex);
}

void Scene4010::primarySpeechAnimationRestored(byte animationGroup, byte baseFrame) {
	(void)animationGroup;
	setHeckerFrame(baseFrame);
}

bool Scene4010::alternateBackgroundActive() const {
	return _vm->gameState().scene4010AlternateBackgroundState != 0;
}

void Scene4010::initializeRoomIdleLayer() {
	_sceneLayers.reset();
	_sceneLayers.setLayerVisible(kScene4010RoomIdleLayer,
		!alternateBackgroundActive() && _sceneChunkTable.isValidChunk(6));
	_roomIdleChannel.reset(0, kScene4010RoomIdleFrameMillis);
	_heckerAnimationState = 0;
	_heckerLoopCount = 0;
	_heckerAlternateSpeechPose = false;
	_heckerManualSequenceActive = false;
	_heckerPoseTransitionPending = false;
	_roomAnimationPaused = false;
}

void Scene4010::resetPaletteCycle() {
	_paletteCycleChannel.reset(0, _releaseProfile.paletteCycleMillis);
}

void Scene4010::advancePaletteCycle(uint32 delta) {
	const uint frameCount = _paletteCycleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		rotatePaletteCycle();
}

void Scene4010::rotatePaletteCycle() {
	const uint lastOffset = kScene4010PaletteCycleLastColor * 3;
	if (_paletteCurrent.size() <= lastOffset + 2)
		return;

	byte saved[3];
	memcpy(saved, &_paletteCurrent[lastOffset], sizeof(saved));
	for (uint color = kScene4010PaletteCycleLastColor; color > kScene4010PaletteCycleFirstColor; --color)
		memcpy(&_paletteCurrent[color * 3], &_paletteCurrent[(color - 1) * 3], sizeof(saved));
	memcpy(&_paletteCurrent[kScene4010PaletteCycleFirstColor * 3], saved, sizeof(saved));
	invalidatePresentationPalette();
}

void Scene4010::updateRoomAmbientAudio(uint32 delta) {
	if (!_ambientSoundBank0.isPlaying()) {
		_previousAmbientSoundCueId = _currentAmbientSoundCueId;
		do {
			_currentAmbientSoundCueId = (byte)(0x0b + _random.getRandomNumber(2));
		} while (_currentAmbientSoundCueId == _previousAmbientSoundCueId);
		_ambientSoundBank0.playSample(_currentAmbientSoundCueId, 20);
	}

	const uint checks = _secondaryAmbientChannel.consumeFrames(delta);
	for (uint check = 0; check < checks; ++check) {
		SoundBank0Player &player = _additionalAmbientSoundBank0Slots[0];
		if (player.isPlaying() || _releaseProfile.secondaryAmbientProbabilityModulus == 0 ||
				_random.getRandomNumber(_releaseProfile.secondaryAmbientProbabilityModulus - 1) != 0)
			continue;

		if (_random.getRandomNumber(9) == 0) {
			player.playSample(0x0e, 100);
			continue;
		}

		byte cue = 0;
		do {
			cue = (byte)(0x0f + _random.getRandomNumber(7));
		} while (cue == _previousSecondaryAmbientCue);
		_previousSecondaryAmbientCue = cue;
		player.playSample(cue, 25);
	}
}

void Scene4010::advanceHeckerIdleLayer(uint32 delta) {
	const uint frameCount = _roomIdleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		advanceHeckerIdleTick();
}

void Scene4010::advanceHeckerIdleTick() {
	switch (_heckerAnimationState) {
	case 0:
		if (_random.getRandomNumber(24) == 0) {
			setHeckerFrame(1);
			_heckerAnimationState = 1;
		} else if (_random.getRandomNumber(49) == 0) {
			setHeckerFrame(0x18);
			_heckerAnimationState = 5;
		}
		break;
	case 1:
		setHeckerFrame(_roomIdleChannel.frameIndex + 1);
		if (_roomIdleChannel.frameIndex == 8) {
			_heckerAnimationState = 2;
			_heckerLoopCount = _heckerPoseTransitionPending ? 1 :
				(byte)(_random.getRandomNumber(6) + 1);
			_soundBank0.playSample(0x17, 20, true);
		}
		break;
	case 2:
		setHeckerFrame(_roomIdleChannel.frameIndex + 1);
		if (_roomIdleChannel.frameIndex == _releaseProfile.heckerLoopEndFrame) {
			if (_heckerLoopCount > 0)
				--_heckerLoopCount;
			if (_heckerLoopCount == 0) {
				_heckerAnimationState = 3;
				_soundBank0.stop();
			} else {
				setHeckerFrame(_releaseProfile.heckerLoopRestartFrame);
			}
		}
		break;
	case 3:
		setHeckerFrame(_roomIdleChannel.frameIndex + 1);
		if (_roomIdleChannel.frameIndex == 0x14) {
			setHeckerFrame(0);
			_heckerAnimationState = 0;
		}
		break;
	case 5:
		setHeckerFrame(_roomIdleChannel.frameIndex + 1);
		if (_roomIdleChannel.frameIndex == 0x1e)
			_heckerAnimationState = 8;
		break;
	case 7:
		setHeckerFrame(_roomIdleChannel.frameIndex + 1);
		if (_roomIdleChannel.frameIndex == 0x2b) {
			setHeckerFrame(0);
			_heckerAnimationState = 0;
		}
		break;
	case 8:
		if (_roomIdleChannel.frameIndex == 0x1f) {
			if (_random.getRandomNumber(14) == 0)
				setHeckerFrame(0x23);
			else if (_random.getRandomNumber(9) == 0) {
				setHeckerFrame(0x24);
				_heckerAnimationState = 7;
			}
		} else {
			setHeckerFrame(0x1f);
		}
		break;
	default:
		break;
	}
}

void Scene4010::drawForegroundBlocks(int activeWorldY) {
	if (activeWorldY < 0x153 && _sceneChunkTable.isValidChunk(13))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _sceneFramebuffer);
	if (activeWorldY < 0x122 && _sceneChunkTable.isValidChunk(14))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[14], _sceneFramebuffer);
}

void Scene4010::runFirstEntrySequence() {
	GameplayState &state = _vm->gameState();
	runEntryPath(0x004c, 0x01c2, 2, 0x00c8, 0x01ae);
	if (_releaseProfile.usesReducedFirstEntry) {
		if (!state.scene4010FirstEntryConversationSeen) {
			beginSecondarySpeechLine(0, 0);
			state.scene4010FirstEntryConversationSeen = true;
		}
		return;
	}

	if (!state.scene4010FirstEntryConversationSeen && !alternateBackgroundActive()) {
		setHeckerFrame(0);
		_heckerAnimationState = 0;
		setActiveActorPose(0x00c8, 0x01ae, 1);
		runHeckerDialoguePoseStart();
		beginPrimarySpeechLine(0x61, 0, 0x0154, 0x00be, 0x20, 0x30, 0x3f);
		runEntryPath(0x00c8, 0x01ae, 1, 0x011a, 0x018d);
		setActiveActorPose(0x011a, 0x018d, 1);
		beginSecondarySpeechLine(0x60, 0);
		beginPrimarySpeechLine(0x61, 1, 0x0154, 0x00be, 0x20, 0x30, 0x3f);
		beginSecondarySpeechLine(0x60, 1);
		beginPrimarySpeechLine(0x61, 2, 0x0154, 0x00be, 0x20, 0x30, 0x3f);
		beginSecondarySpeechLine(0x60, 2);
		beginPrimarySpeechLine(0x61, 3, 0x0154, 0x00be, 0x20, 0x30, 0x3f);
		finishHeckerDialoguePose();
		state.scene4010FirstEntryConversationSeen = true;
	} else {
		setActiveActorPose(0x00c8, 0x01ae, 1);
	}

	if (_vm->isDemo() && _vm->getLanguage() == Common::ES_ESP)
		beginSecondarySpeechLine(0, 0);
}

void Scene4010::runEntryFromRightSide() {
	GameplayState &state = _vm->gameState();
	if (_releaseProfile.usesDirectRightEntry) {
		runEntryPath(0x02c4, 0x00f3, 4, 0x011d, 0x017d);
		return;
	}

	runEntryPath(0x02c4, 0x00f3, 4, state.scene4010EntryPathSpeechState < 2 ? 0x0238 : 0x011d,
		state.scene4010EntryPathSpeechState < 2 ? 0x0111 : 0x017d);
	if (state.scene4010EntryPathSpeechState < 2) {
		beginStaticSecondarySpeechLine(0xd0, 0);
		runEntryPath(0x0238, 0x0111, 4, 0x00f5, 0x0169);
		beginSecondarySpeechLine(1, 10);
		state.scene4010EntryPathSpeechState = 2;
	}
}

void Scene4010::runEntryFromLeftSide() {
	runEntryPath(0x01ad, 0x01ce, 4, 0x01ad, 0x01ce);
	runActorReplacement(ActionOverlaySpec(17, kScene4010ExitOverlayDescriptorCount, kScene4010OverlayFrameMillis)
		.holdFirstFrame()
		.startAt(1));
}

void Scene4010::setActiveActorPose(int x, int y, byte facing) {
	_activeActorWorldX = x;
	_activeActorWorldY = y;
	_activeActorFacing = facing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene4010::setHeckerFrame(byte frameIndex) {
	_roomIdleChannel.frameIndex = frameIndex;
	_sceneLayers.setLayerFrame(kScene4010RoomIdleLayer, frameIndex);
}

void Scene4010::runHeckerFrameSequence(const byte *frames, uint frameCount) {
	_heckerManualSequenceActive = true;
	for (uint i = 0; i < frameCount && !Engine::shouldQuit(); ++i) {
		setHeckerFrame(frames[i]);
		if (waitSceneMillis(kScene4010RoomIdleFrameMillis))
			break;
	}
	_heckerManualSequenceActive = false;
}

void Scene4010::waitForHeckerIdlePose() {
	_heckerPoseTransitionPending = true;
	_heckerLoopCount = 1;
	for (uint i = 0; i < 96 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++i) {
		if (_heckerAnimationState == 0 || _heckerAnimationState == 6 ||
				_heckerAnimationState == 8) {
			_heckerPoseTransitionPending = false;
			return;
		}
		if (waitSceneMillis(kScene4010RoomIdleFrameMillis)) {
			_heckerPoseTransitionPending = false;
			return;
		}
	}
	_heckerPoseTransitionPending = false;
}

void Scene4010::runHeckerDialoguePoseStart() {
	if (_heckerAnimationState == 8) {
		_heckerAnimationState = 6;
		return;
	}

	waitForHeckerIdlePose();
	if (_heckerAnimationState == 8) {
		_heckerAnimationState = 6;
		return;
	}

	runHeckerFrameSequence(kScene4010HeckerResponsePoseFrames,
		ARRAYSIZE(kScene4010HeckerResponsePoseFrames));
	_heckerAnimationState = 6;
}

void Scene4010::runHeckerRandomResponsePoseStart() {
	_heckerAlternateSpeechPose = _random.getRandomBit() != 0;
	if (!_heckerAlternateSpeechPose) {
		_heckerAnimationState = 6;
		return;
	}

	const byte frames[] = { 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a };
	runHeckerFrameSequence(frames, ARRAYSIZE(frames));
	_heckerAnimationState = 4;
}

void Scene4010::runHeckerResponsePoseEnd() {
	_heckerAnimationState = 6;
	if (!_heckerAlternateSpeechPose)
		return;

	runHeckerFrameSequence(kScene4010HeckerResponsePoseFrames,
		ARRAYSIZE(kScene4010HeckerResponsePoseFrames));
}

void Scene4010::finishHeckerDialoguePose() {
	if (_heckerAlternateSpeechPose) {
		_heckerAlternateSpeechPose = false;
		setHeckerFrame(0);
		_heckerAnimationState = 0;
		return;
	}

	const byte frames[] = { 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0 };
	runHeckerFrameSequence(frames, ARRAYSIZE(frames));
	_heckerAnimationState = 0;
}

void Scene4010::beginD01SpeechLine(uint16 rowIndex, byte normalFrame, byte alternateFrame) {
	const byte frame = alternateBackgroundActive() && alternateFrame != 0xff ? alternateFrame : normalFrame;
	beginSecondarySpeechLine(rowIndex, frame);
}

void Scene4010::beginSecondarySpeechLineAndEnterHeckerPose(uint16 rowIndex, byte frameIndex) {
	const bool started = startRealtimeSecondarySpeechLine(rowIndex, frameIndex, 0);
	runHeckerDialoguePoseStart();
	if (started)
		waitForRealtimeSpeech();
}

void Scene4010::beginHeckerSpeechLine(byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getStage003Cue(kScene4010HeckerDialogueResponseRow, frameIndex, textRecordId,
			continuationCount, voiceSampleId))
		return;

	setPaletteEntry6Bit(kScene4010PrimarySpeechTextColor, kScene4010HeckerSpeechRed,
		kScene4010HeckerSpeechGreen, kScene4010HeckerSpeechBlue);

	const byte baseFrame = primarySpeechAnimationBaseFrame(kScene4010HeckerSpeechGroup);
	_speechController.startPrimaryDialogueSpeech(kScene4010HeckerSpeechGroup, baseFrame);
	primarySpeechAnimationStarted(kScene4010HeckerSpeechGroup, baseFrame);
	setPrimarySpeechAnimationFrame(kScene4010HeckerSpeechGroup, baseFrame);

	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++part) {
		const Common::String text = getResource003LargeTextRecord(textRecordId + part);
		if (text.empty())
			continue;

		_primarySpeechOverlay.visible = true;
		_primarySpeechOverlay.colorIndex = kScene4010PrimarySpeechTextColor;
		wrapActorSpeechText(text, kScene4010HeckerSpeechCenterX, _primarySpeechOverlay.lines);
		calculateSpeechOverlayBounds(_primarySpeechOverlay, kScene4010HeckerSpeechCenterX,
			kScene4010HeckerSpeechTopY, true, _activeActorWorldY);

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _primarySpeechOverlay.lines.size() * 1100);
		const bool interrupted = waitForSpeechOrDelay(duration, false);

		_speech.stop();
		_primarySpeechOverlay.visible = false;
		_primarySpeechOverlay.lines.clear();
		if (interrupted)
			break;
	}

	setPrimarySpeechAnimationFrame(kScene4010HeckerSpeechGroup, baseFrame);
	_speechController.stopPrimaryDialogueSpeech(kScene4010InvalidPrimarySpeechGroup,
		kScene4010DefaultPrimarySpeechFrame);
	primarySpeechAnimationRestored(kScene4010HeckerSpeechGroup, baseFrame);
}

void Scene4010::runHeckerDialogue() {
	Common::Array<DialogueChoiceRecord> records;
	initializeHeckerDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	byte depthIndex = 0;
	byte nodeIndex = 0;
	bool finished = false;

	if (state.scene4010EntryPathSpeechState == 0) {
		beginSecondarySpeechLineAndEnterHeckerPose(kScene4010HeckerDialogueStageId, 0);
		beginHeckerSpeechLine(0);
		runHeckerResponsePoseEnd();
		state.scene4010EntryPathSpeechState = 1;
		applySceneStateToHotspotsAndPatches(2);
	} else {
		beginSecondarySpeechLineAndEnterHeckerPose(kScene4010HeckerDialogueStageId, 1);
		beginHeckerSpeechLine(1);
		runHeckerResponsePoseEnd();
	}

	while (!finished && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene4010HeckerDialogueStageId, records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginSecondarySpeechLine(kScene4010HeckerDialogueStageId, 6);
			beginHeckerSpeechLine(6);
			runHeckerResponsePoseEnd();
			break;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			break;

		DialogueChoiceRecord &record = records[recordIndex];
		beginSecondarySpeechLine(kScene4010HeckerDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != 0xff) {
			runHeckerRandomResponsePoseStart();
			beginHeckerSpeechLine(record.responseFrameIndex);
			if (record.transitionMode != 0)
				runHeckerResponsePoseEnd();
		}

		if (record.disableAfterUse == 1)
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

	finishHeckerDialoguePose();
}

void Scene4010::initializeHeckerDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene4010HeckerDialogueChoiceRecordCount);

	setHeckerDialogueRecord(records, 0, 1, 0, 1, 2, 2, 1);
	setHeckerDialogueRecord(records, 1, 1, 0, 3, 3, 3, 1);
	setHeckerDialogueRecord(records, 2, 1, 0, 3, 4, 4, 1);
	setHeckerDialogueRecord(records, 3, 1, 0, 3, 5, 5, 1);
	setHeckerDialogueRecord(records, 4, 1, 0, 0, 6, 6, 1);

	setHeckerDialogueRecord(records, 70, 1, 0, 1, 7, 7, 1);
	setHeckerDialogueRecord(records, 71, 1, 0, 3, 8, 8, 1);
	setHeckerDialogueRecord(records, 72, 1, 0, 3, 9, 9, 1);
	setHeckerDialogueRecord(records, 73, 1, 0, 3, 10, 10, 1);
	setHeckerDialogueRecord(records, 74, 1, 0, 2, 11, 11, 0);

	setHeckerDialogueRecord(records, 140, 1, 0, 1, 12, 12, 1);
	setHeckerDialogueRecord(records, 210, 1, 0, 4, 13, 13, 1);
}

void Scene4010::setHeckerDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
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
	record.selectable = enabled != 0;
}

void Scene4010::runProgressiveExitSpeech() {
	GameplayState &state = _vm->gameState();
	const byte frame = MIN<byte>(state.scene4010ProgressiveExitSpeechState, 3);
	BlockingSequence sequence(*this);
	sequence.secondarySpeech(6, frame);
	if (state.scene4010ProgressiveExitSpeechState > 1) {
		sequence.actorReplacement(ActionOverlaySpec(16, kScene4010ExitOverlayDescriptorCount,
				kScene4010OverlayFrameMillis)
				.holdFirstFrame()
				.soundAt(11, 0x27)
				.startAt(1))
			.commit(state.mainFlowStateId, _releaseProfile.moatExitState);
	}
	if (state.scene4010ProgressiveExitSpeechState < 3)
		++state.scene4010ProgressiveExitSpeechState;
}

void Scene4010::takeAnimatedItem3A() {
	GameplayState &state = _vm->gameState();
	if (state.scene4010Item3APickupState == 0 || state.scene4010Item3APickupState == 3 ||
			hasInventoryItem(kScene4010Item3A))
		return;

	BlockingSequence sequence(*this);
	if (state.scene4010Item3APickupState == 1)
		sequence.secondarySpeech(13, 0);

	sequence.secondarySpeech(12, 0)
		.commit(_roomAnimationPaused, true)
		.actorReplacement(ActionOverlaySpec(9, kScene4010Item3AOverlayDescriptorCount,
			kScene4010OverlayFrameMillis)
			.holdFirstFrame()
			.startAt(1)
			.resourcePatchAt(7, 8))
		.commit(_roomAnimationPaused, false)
		.commit(state.scene4010Item3APickupState, (byte)3)
		.framebufferPatch(3);
	addInventoryItem(kScene4010Item3A);
	sequence.sound(1);
}

void Scene4010::handlePendingItem3A() {
	GameplayState &state = _vm->gameState();
	if (state.scene4010Item3APickupState == 0)
		return;

	if (state.scene4010Item3APickupState == 1) {
		beginSecondarySpeechLine(13, 0);
		state.scene4010Item3APickupState = 2;
		applySceneStateToHotspotsAndPatches(3);
		return;
	}

	beginSecondarySpeechLine(13, 1);
}

void Scene4010::unlockDestinationFromRoomAction() {
	GameplayState &state = _vm->gameState();
	if (state.hasTravelScreenDestination(kScene4010AustraliaDestinationId))
		state.scene4010DestinationUnlocked = true;
	else if (state.scene4010DestinationUnlocked)
		state.scene4010DestinationUnlocked = false;

	if (!state.scene1050CharlieBogWerewolfClueHeard) {
		beginSecondarySpeechLine(15, 0);
		return;
	}
	if (state.scene4010DestinationUnlocked) {
		beginSecondarySpeechLine(14, 0);
		return;
	}

	beginSecondarySpeechLine(15, 1);
	runDestinationUnlockAnimation();
	if (unlockTravelDestination(kScene4010AustraliaDestinationId))
		state.scene4010DestinationUnlocked = true;
}

void Scene4010::runDestinationUnlockAnimation() {
	Common::Array<byte> frameMap;
	for (byte logicalFrame = 1; logicalFrame <= 0x0d; ++logicalFrame)
		frameMap.push_back(kTravelUnlockFrames[logicalFrame]);

	byte logicalFrame = 0x0d;
	const uint soundStartFrame = frameMap.size();
	for (uint hold = 0; hold < 25; ++hold) {
		byte nextFrame = logicalFrame;
		do {
			nextFrame = (byte)(0x0d + _random.getRandomNumber(4));
		} while (nextFrame == logicalFrame);
		logicalFrame = nextFrame;
		frameMap.push_back(kTravelUnlockFrames[logicalFrame]);
	}

	uint soundStopFrame = 0;
	while (logicalFrame < 0x1e) {
		if (logicalFrame == 0x14)
			soundStopFrame = frameMap.size();
		++logicalFrame;
		frameMap.push_back(kTravelUnlockFrames[logicalFrame]);
	}

	BlockingSequence(*this)
		.commit(_roomAnimationPaused, true)
		.actorReplacement(ActionOverlaySpec(15, kScene4010DestinationOverlayDescriptorCount,
			frameMap.data(), frameMap.size(), kScene4010OverlayFrameMillis)
			.loopingSoundAt(soundStartFrame, 0x38, 25)
			.stopSoundAt(soundStopFrame))
		.commit(_roomAnimationPaused, false)
		.stopSound();
}

void Scene4010::takeThrownItem() {
	GameplayState &state = _vm->gameState();
	if (state.scene4010PillboxPickupState != 1 || hasInventoryItem(_releaseProfile.thrownItemId))
		return;

	BlockingSequence sequence(*this);
	sequence.commit(_roomAnimationPaused, true)
		.actorReplacement(ActionOverlaySpec(12, kScene4010PillboxOverlayDescriptorCount,
			kScene4010OverlayFrameMillis)
			.holdFirstFrame()
			.startAt(1)
			.resourcePatchAt(7, 11))
		.commit(_roomAnimationPaused, false);
	applyBaseFramebufferPatch(11);
	sequence.commit(state.scene4010PillboxPickupState, (byte)2)
		.framebufferPatch(5);
	addInventoryItem(_releaseProfile.thrownItemId);
	sequence.sound(1);
	dispatchGenericSceneAction(21);
}

void Scene4010::applyBaseFramebufferPatch(uint chunkIndex) {
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _baseFramebuffer);
}

void Scene4010::ensureNormalBaseFramebuffer() {
	if (_normalBaseFramebufferInitialized)
		return;

	_normalBaseFramebuffer.copyFrom(_baseFramebufferOriginal);
	_normalBaseFramebufferInitialized = true;
}

void Scene4010::applyD01BackgroundForCurrentState() {
	if (alternateBackgroundActive()) {
		loadFixedChunk(5, _baseFramebuffer, kSceneBufferByteCount);
		_baseFramebufferOriginal.copyFrom(_baseFramebuffer);
		return;
	}

	_baseFramebuffer.copyFrom(_normalBaseFramebuffer);
	_baseFramebufferOriginal.copyFrom(_normalBaseFramebuffer);
}

void Scene4010::copySmallRow(uint sourceOffset, uint destinationOffset) {
	if (_stage003SmallRows.size() < sourceOffset + kStage003SmallRowSize ||
			_stage003SmallRows.size() < destinationOffset + kStage003SmallRowSize)
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene4010::setScenePoint(uint baseOffset, byte itemId, uint16 x, uint16 y) {
	const uint offset = baseOffset + itemId * sizeof(ScenePoint);
	if (_metadata.size() < offset + sizeof(ScenePoint))
		return;

	_metadata[offset] = x & 0xff;
	_metadata[offset + 1] = x >> 8;
	_metadata[offset + 2] = y & 0xff;
	_metadata[offset + 3] = y >> 8;
}

void Scene4010::clearVerbActionRecord(uint recordIndex) {
	const uint offset = kSceneVerbActionRecords + recordIndex * kScene4010VerbActionRecordSize;
	if (_metadata.size() < offset + kScene4010VerbActionRecordSize)
		return;

	memset(_metadata.data() + offset, 0, kScene4010VerbActionRecordSize);
}

void Scene4010::removeColorMapItem(byte itemId) {
	replaceColorMapItem(itemId, 0);
}

void Scene4010::replaceColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene4010::copyStepDeltas(uint targetOffset, uint sourceOffset, uint count) {
	for (uint i = 0; i < count && targetOffset + i < _actorPathStepDeltas.size() &&
			sourceOffset + i < ARRAYSIZE(kActorPathStepDeltaTableSet87); ++i) {
		_actorPathStepDeltas[targetOffset + i] = kActorPathStepDeltaTableSet87[sourceOffset + i];
	}
}

} // End of namespace Hollywood
