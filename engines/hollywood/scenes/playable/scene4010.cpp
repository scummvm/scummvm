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

#include "hollywood/scenes/playable/scene4010.h"

#include "common/system.h"
#include "graphics/pixelformat.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene4010EntryFromRightSideState = 0x0fab;
const uint16 kScene4010EntryFromLeftSideState = 0x0fac;
const uint16 kScene4010ExitState4020 = 0x0fb4;
const uint16 kScene4010ExitState4110 = 0x100f;
const uint16 kScene4010ViewportXOffset = 0x0068;
const uint16 kScene4010ViewportMaxXOffset = 0x00b8;
const uint kScene4010ActorBankTableEntry = 0x0000;
const uint kScene4010ActorPaletteTableEntry = 0x00cc;
const uint kScene4010Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4010SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4010RoomIdleFrameMillis = 75;
const uint32 kScene4010OverlayFrameMillis = 75;
const uint kScene4010VerbActionRecordSize = 4;
const uint kScene4010RoomIdleDescriptorCount = 0x14;
const uint kScene4010ExitOverlayDescriptorCount = 0x13;
const uint kScene4010Item3AOverlayDescriptorCount = 0x0e;
const uint kScene4010DestinationOverlayDescriptorCount = 0x11;
const uint kScene4010PillboxOverlayDescriptorCount = 0x0d;
const uint kScene4010HeckerDialogueChoiceRecordCount = 700;
const byte kScene4010Item3A = 0x3a;
const byte kScene4010PillboxItem = 0x3b;
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

const byte kScene4010RoomIdleFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 10, 11, 12, 0, 1, 2, 3, 13, 14,
	15, 15, 16, 17, 18, 19, 15, 14, 13, 3,
	2, 1, 0, 0
};

const byte kScene4010ExitOverlayFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18
};

const byte kScene4010Item3AFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13
};

const byte kScene4010DestinationFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 12, 13, 14, 15, 16, 11,
	10, 9, 8, 7, 6, 5, 4, 3, 2, 1,
	0
};

const byte kScene4010PillboxFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13
};

static PlayableSceneConfig scene4010Config() {
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
		_roomIdleChannel(),
		_roomIdleLayer(),
		_normalBaseFramebuffer(),
		_normalBaseFramebufferInitialized(false),
		_heckerAnimationState(0),
		_heckerLoopCount(0),
		_heckerAlternateSpeechPose(false),
		_heckerManualSequenceActive(false) {
	_normalBaseFramebuffer.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		Graphics::PixelFormat::createFormatCLUT8());
}

void Scene4010::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	initializeRoomIdleLayer();

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

void Scene4010::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (!alternateBackgroundActive())
		drawResourceSpriteLayer(_roomIdleLayer);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldY);
	drawActionOverlayLayer();
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

bool Scene4010::advanceCustomGameplayLoop(uint32 delta) {
	if (!alternateBackgroundActive() && !_primaryDialogueSpeechActive && !_heckerManualSequenceActive)
		advanceHeckerIdleLayer(delta);
	return false;
}

bool Scene4010::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a puente levadizo (go to drawbridge), or return from the opened-bridge view.
		if (alternateBackgroundActive())
			_vm->gameState().mainFlowStateId = kScene4010ExitState4110;
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
	case 307: // Ir a foso (go to moat): progressive exit to scene 4020.
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
	case 312: // Patched old-man/car response: talk suggestion outside, blocked car inside.
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
	case 317: // Coger pastillero (take pillbox): item 0x3b.
		takePillbox();
		return true;
	case 318: // Mirar pastillero (look pillbox): thrown out the window by the count.
		beginSecondarySpeechLine(16, 0);
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
		copyStepDeltas(0x0c, 0x17);
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 3 && nextRegion == 2) {
		copyStepDeltas(0x30, 0x3b);
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene4010::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;

	if (currentRegion == 3) {
		copyStepDeltas(0x30, 0x3b);
		requestedFacing = 4;
		restoredStepDeltas = true;
	}

	return restoredStepDeltas;
}

bool Scene4010::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	ensureNormalBaseFramebuffer();
	applyD01BackgroundForCurrentState();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene4070SlimmingTreatmentApplied && state.scene4010PillboxPickupState == 0)
		state.scene4010PillboxPickupState = 1;

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
			if (_sceneChunkTable.isValidChunk(10))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
		} else {
			removeColorMapItem(7);
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
	initializeRoomIdleLayer();
	return true;
}

AmbientAudioProfile Scene4010::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0b, 3, 20, 1, 0x0b, 5, 100, 50);
}

byte Scene4010::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return _heckerAlternateSpeechPose ? 0x14 : 0x1f;
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
	_roomIdleLayer.configure(6, kScene4010RoomIdleDescriptorCount,
		kScene4010RoomIdleFrameMap, ARRAYSIZE(kScene4010RoomIdleFrameMap));
	_roomIdleLayer.visible = !alternateBackgroundActive();
	_roomIdleChannel.reset(0, kScene4010RoomIdleFrameMillis);
	_heckerAnimationState = 0;
	_heckerLoopCount = 0;
	_heckerAlternateSpeechPose = false;
	_heckerManualSequenceActive = false;
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
			_heckerLoopCount = (byte)(_random.getRandomNumber(6) + 1);
		}
		break;
	case 2:
		setHeckerFrame(_roomIdleChannel.frameIndex + 1);
		if (_roomIdleChannel.frameIndex == 0x0c) {
			if (_heckerLoopCount > 0)
				--_heckerLoopCount;
			if (_heckerLoopCount == 0)
				_heckerAnimationState = 3;
			else
				setHeckerFrame(8);
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
}

void Scene4010::runEntryFromRightSide() {
	GameplayState &state = _vm->gameState();
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
	runActorReplacement(ActionOverlaySpec(17, kScene4010ExitOverlayDescriptorCount,
		kScene4010ExitOverlayFrameMap, ARRAYSIZE(kScene4010ExitOverlayFrameMap), kScene4010OverlayFrameMillis));
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
	_roomIdleLayer.setFrame(frameIndex);
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
	for (uint i = 0; i < 96 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++i) {
		if (_heckerAnimationState == 0 || _heckerAnimationState == 6 ||
				_heckerAnimationState == 8)
			return;
		if (waitSceneMillis(kScene4010RoomIdleFrameMillis))
			return;
	}
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

	const byte frames[] = { 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e };
	runHeckerFrameSequence(frames, ARRAYSIZE(frames));
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

	const byte frames[] = { 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e };
	runHeckerFrameSequence(frames, ARRAYSIZE(frames));
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
	const uint32 startMillis = g_system->getMillis();
	const bool started = startSecondarySpeechLine(rowIndex, frameIndex);
	const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
		MAX<uint32>(1200, (uint32)_speechOverlay.lines.size() * 1100);

	runHeckerDialoguePoseStart();

	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		const bool speechActive = _speech.isPlaying();
		const uint32 elapsed = g_system->getMillis() - startMillis;
		if (!speechActive && elapsed >= duration)
			break;

		const uint32 slice = speechActive ? 50 : MIN<uint32>(50, duration - elapsed);
		if (waitSceneMillis(slice))
			break;
	}

	_speech.stop();
	clearSpeechOverlay();
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
	record.reserved = 0xff;
	record.selectable = enabled != 0;
}

void Scene4010::runProgressiveExitSpeech() {
	GameplayState &state = _vm->gameState();
	const byte frame = MIN<byte>(state.scene4010ProgressiveExitSpeechState, 3);
	beginSecondarySpeechLine(6, frame);
	if (state.scene4010ProgressiveExitSpeechState > 1) {
		runActorReplacement(ActionOverlaySpec(16, kScene4010ExitOverlayDescriptorCount,
			kScene4010ExitOverlayFrameMap, ARRAYSIZE(kScene4010ExitOverlayFrameMap), kScene4010OverlayFrameMillis)
			.soundAt(11, 0x27)
			.noRedrawAtEnd()
			.startAt(1));
		state.mainFlowStateId = kScene4010ExitState4020;
	}
	if (state.scene4010ProgressiveExitSpeechState < 3)
		++state.scene4010ProgressiveExitSpeechState;
}

void Scene4010::takeAnimatedItem3A() {
	GameplayState &state = _vm->gameState();
	if (state.scene4010Item3APickupState == 0 || state.scene4010Item3APickupState == 3 ||
			hasInventoryItem(kScene4010Item3A))
		return;

	if (state.scene4010Item3APickupState == 1) {
		beginSecondarySpeechLine(13, 0);
	}

	beginSecondarySpeechLine(12, 0);
	state.scene4010Item3APickupState = 3;
	runActorReplacement(ActionOverlaySpec(9, kScene4010Item3AOverlayDescriptorCount,
		kScene4010Item3AFrameMap, ARRAYSIZE(kScene4010Item3AFrameMap), kScene4010OverlayFrameMillis)
		.patchAt(6, 3));
	addInventoryItem(kScene4010Item3A);
	_soundBank0.playSample(1, 100);
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
	runActorReplacement(ActionOverlaySpec(15, kScene4010DestinationOverlayDescriptorCount,
		kScene4010DestinationFrameMap, ARRAYSIZE(kScene4010DestinationFrameMap), kScene4010OverlayFrameMillis)
		.soundAt(20, 0, 0)
		.endAt(ARRAYSIZE(kScene4010DestinationFrameMap)));
	if (state.unlockTravelScreenDestination(kScene4010AustraliaDestinationId) ||
			state.hasTravelScreenDestination(kScene4010AustraliaDestinationId)) {
		state.scene4010DestinationUnlocked = true;
		_soundBank0.playSample(1, 100);
	}
}

void Scene4010::takePillbox() {
	GameplayState &state = _vm->gameState();
	if (state.scene4010PillboxPickupState != 1 || hasInventoryItem(kScene4010PillboxItem))
		return;

	state.scene4010PillboxPickupState = 2;
	runActorReplacement(ActionOverlaySpec(12, kScene4010PillboxOverlayDescriptorCount,
		kScene4010PillboxFrameMap, ARRAYSIZE(kScene4010PillboxFrameMap), kScene4010OverlayFrameMillis)
		.patchAt(6, 5));
	addInventoryItem(kScene4010PillboxItem);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene4010::ensureNormalBaseFramebuffer() {
	if (_normalBaseFramebufferInitialized)
		return;

	_normalBaseFramebuffer.copyFrom(_baseFramebufferOriginal);
	_normalBaseFramebufferInitialized = true;
}

void Scene4010::applyD01BackgroundForCurrentState() {
	if (alternateBackgroundActive()) {
		loadFixedChunk(5, _baseFramebuffer, kFrameBufferSize);
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

void Scene4010::copyStepDeltas(uint firstOffset, uint lastOffset) {
	for (uint offset = firstOffset; offset <= lastOffset &&
			offset < _actorPathStepDeltas.size() &&
			offset < ARRAYSIZE(kActorPathStepDeltaTableSet87); ++offset) {
		_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
	}
}

} // End of namespace Hollywood
