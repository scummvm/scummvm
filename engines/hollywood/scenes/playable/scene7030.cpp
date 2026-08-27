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

#include "hollywood/scenes/playable/scene7030.h"

#include "common/debug.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene7030ViewportXOffset = 0x60;
const uint16 kScene7030State7031 = 0x1b77;
const uint16 kScene7030ExitState7040 = 0x1b80;
const uint16 kScene7030ReturnState7011 = 0x1b63;
const uint16 kScene7030Chunk5DescriptorCount = 9;
const uint16 kScene7030Chunk6DescriptorCount = 0x10;
const uint16 kScene7030Chunk7DescriptorCount = 0x0d;
const uint16 kScene7030Chunk10DescriptorCount = 0x0a;
const uint16 kScene7030Chunk11DescriptorCount = 0x20;
const byte kScene7030AmbientMusicCueStillFrame = 0x0f;
const byte kScene7030Entry7030Facing = 4;
const int kScene7030Entry7030StartX = 0x312;
const int kScene7030Entry7030StartY = 0x19d;
const int kScene7030Entry7030TargetX = 0x1fa;
const int kScene7030Entry7030TargetY = 0x142;
const byte kScene7030Entry7031Facing = 2;
const int kScene7030Entry7031StartX = 0x60;
const int kScene7030Entry7031StartY = 0x10e;
const int kScene7030Entry7031TargetX = 0x133;
const int kScene7030Entry7031TargetY = 0x134;
const uint32 kScene7030Chunk5FrameMillis = 75;
const uint32 kScene7030Chunk5FastFrameMillis = 60;
const uint32 kScene7030Chunk6FrameMillis = 125;
const uint32 kScene7030SecondaryActorFrameMillis = 150;
const uint32 kScene7030AmbientMusicCheckMillis = 250;
const byte kScene7030BaseAmbientSoundCue = 0x0b;
const byte kScene7030SecondaryAmbientSoundCue = 0x0c;
const uint kScene7030ColorToItemMapOffset = 0x100;
const uint kScene7030ColorMapSize = 0x100;
const byte kScene7030PunchBowlGlassPatchHook = 1;
const uint kScene7030Chunk6IdleLayerA = 0;
const uint kScene7030Chunk6IdleLayerB = 1;
const uint kScene7030Chunk6IdleLayerC = 2;
const uint kScene7030Chunk6IdleLayerD = 3;
const uint kScene7030Chunk5Layer = 4;
const byte kScene7030Chunk5FrameMap[] = {
	0, 0, 1, 2, 3, 4, 3, 2, 3, 4, 3, 2, 1, 0, 5, 6,
	7, 8, 7, 6, 7, 8, 7, 6, 5
};
const byte kScene7030Chunk7PickupBoneFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};
const byte kScene7030Chunk10PickupItem0CFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 9, 0
};
const byte kScene7030Chunk11ExchangeItem0CFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 25, 25, 25, 25, 25, 25,
	25, 25, 25, 25, 26, 27, 28, 29, 30, 31, 13, 12, 11, 10, 9, 8,
	7, 6, 5, 4, 3, 2, 1, 0
};
const SceneAnimationLayerSpec kScene7030AnimationLayerSpecs[] = {
	{ kSceneAnimationBehindActors,
		6, kScene7030Chunk6DescriptorCount, nullptr, 0, true },
	{ kSceneAnimationBehindActors,
		6, kScene7030Chunk6DescriptorCount, nullptr, 0, true },
	{ kSceneAnimationBehindActors,
		6, kScene7030Chunk6DescriptorCount, nullptr, 0, true },
	{ kSceneAnimationBehindActors,
		6, kScene7030Chunk6DescriptorCount, nullptr, 0, true },
	{ kSceneAnimationBehindActors,
		5, kScene7030Chunk5DescriptorCount, kScene7030Chunk5FrameMap,
		ARRAYSIZE(kScene7030Chunk5FrameMap), false }
};
const byte kScene7030Route6StepDeltas[] = {
	5, 5, 5, 6, 2, 3, 4, 8, 10, 6, 3, 5
};

static PlayableSceneConfig scene7030Config() {
	PlayableSceneConfig config(7030,
		SceneResourceLayout(12, 5, 11),
		SceneViewport(kScene7030ViewportXOffset),
		SceneActorPose(0x1fa, 0x142, 4));
	config.useActorDepthTest = true;
	return config;
}

Scene7030::Scene7030(HollywoodEngine *vm) :
		PlayableScene(vm, scene7030Config()),
		_chunk9AmbientDecisionCounter(0),
		_chunk5FrameDirection(1),
		_chunk5Animation(),
		_chunk6Animation(),
		_chunk6IdlePairA(),
		_chunk6IdlePairB(),
		_animationLayers() {
}

void Scene7030::initializeCustomPreviewState() {
	GameplayState &state = _vm->gameState();

	configureAnimationLayers();
	setChunk5Frame(1);
	_chunk5FrameDirection = 1;
	_chunk6IdlePairA.configure(0, 4, AlternatingRandomFramePair::kFirstFrame);
	_chunk6IdlePairB.configure(8, 0x0c, AlternatingRandomFramePair::kSecondFrame);
	_primaryLeftSpeechLastFrame = 0;
	_actionOverlayPlayer.reset();
	const bool pairASecondPhase = _random.getRandomNumber(1) != 0;
	const bool pairBSecondPhase = _random.getRandomNumber(1) != 0;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_chunk6IdlePairA.reset(pairASecondPhase, _chunk6IdlePairA.randomPhaseTicks(_random));
	_chunk6IdlePairB.reset(pairBSecondPhase, _chunk6IdlePairB.randomPhaseTicks(_random));
	_animationLayers.setLayerFrame(kScene7030Chunk6IdleLayerA, _chunk6IdlePairA.firstFrame);
	_animationLayers.setLayerFrame(kScene7030Chunk6IdleLayerB, _chunk6IdlePairA.secondFrame);
	_animationLayers.setLayerFrame(kScene7030Chunk6IdleLayerC, _chunk6IdlePairB.firstFrame);
	_animationLayers.setLayerFrame(kScene7030Chunk6IdleLayerD, _chunk6IdlePairB.secondFrame);
	_chunk9AmbientDecisionCounter = 0;
	_chunk5Animation.reset(1, kScene7030Chunk5FrameMillis);
	_chunk6Animation.reset(0, kScene7030Chunk6FrameMillis);
	_ambientMusicTimerAccumulator = 0;
	_secondaryActorTimerAccumulator = 0;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	setActiveActorPose(kScene7030Entry7030TargetX, kScene7030Entry7030TargetY, kScene7030Entry7030Facing);
	_secondaryActorFrame = 0;
	memset(_inventoryItems, 0, sizeof(_inventoryItems));
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	_sceneStateFlags[0] = state.humeroBarrierState;
	_sceneStateFlags[1] = state.humeroBonePickupState;
	_sceneStateFlags[2] = state.punchBowlGlassPatchState;
	if (!hasInventoryItem(0x0c) && !hasInventoryItem(0x0d) && !hasInventoryItem(0x11) &&
			_sceneStateFlags[2] == 0) {
		_sceneStateFlags[2] = 2;
		state.punchBowlGlassPatchState = _sceneStateFlags[2];
	}
	_animationLayers.setLayerVisible(kScene7030Chunk5Layer, _sceneStateFlags[0] != 0);
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene7030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();
	drawAnimationLayers(_animationLayers, kSceneAnimationBehindActors);

	updateActorDepthThresholds(actorDrawOrderMode);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	drawActionOverlayLayer();
}

bool Scene7030::shouldDrawSecondaryActorInPlayableComposite() const {
	return _speechOverlay.visible && !_actionOverlayPlayer.replacesActor();
}

void Scene7030::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene7030State7031) {
		runEntryPath(kScene7030Entry7031StartX, kScene7030Entry7031StartY, kScene7030Entry7031Facing,
			kScene7030Entry7031TargetX, kScene7030Entry7031TargetY);
		return;
	}

	runEntryPath(kScene7030Entry7030StartX, kScene7030Entry7030StartY, kScene7030Entry7030Facing,
		kScene7030Entry7030TargetX, kScene7030Entry7030TargetY);
}

bool Scene7030::advanceCustomGameplayLoop(uint32 delta) {
	for (uint frame = _chunk5Animation.consumeFrames(delta); frame != 0; --frame)
		advanceChunk5AmbientOverlay();

	for (uint frame = _chunk6Animation.consumeFrames(delta); frame != 0; --frame)
		advanceChunk6IdleFrames();

	if (_primaryLeftSpeechActive) {
		_primaryLeftSpeechTimerAccumulator += delta;
		while (_primaryLeftSpeechTimerAccumulator >= kScene7030SecondaryActorFrameMillis) {
			_primaryLeftSpeechTimerAccumulator -= kScene7030SecondaryActorFrameMillis;
			advancePrimaryLeftSpeechFrame();
		}
	}

	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene7030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 0: // Shared default/no-op action slot.
	case 1: // Ir a Húmero/hueso/vaso/ponchera (go to Húmero/bone/glass/punch bowl)
	case 3: // Usar objetos sin efecto en la escena (use items with no scene effect)
	case 9: // Abrir puerta (open door)
	case 13: // Dar objetos a Húmero (give items to Húmero)
	case 18: // Cerrar puerta / usar objeto sin efecto (close door / no-effect item use)
	case 24: // Coger ponchera (take punch bowl)
		return dispatchGenericSceneAction(handlerId);
	case 301: // Ir a puerta (go to door)
		handleActionSlot00TransitionToG04();
		break;
	case 302: // Mirar puerta (look at door)
		handleActionSlot01SecondarySpeech();
		break;
	case 303: // Ir a escalera (go to stairs)
		handleActionSlot02TransitionToG01Alt();
		break;
	case 304: // Mirar escalera (look at stairs)
		handleActionSlot03SecondarySpeech();
		break;
	case 305: // Hablar con Húmero (talk to Húmero)
		handleActionSlot04SecondarySpeech();
		break;
	case 306: // Mirar Húmero (look at Húmero)
		handleActionSlot05ToggleSceneState0Speech();
		break;
	case 309: // Mirar vaso (look at glass)
		handleActionSlot08CommonSpeech();
		break;
	case 310: // Mirar ponchera (look at punch bowl)
		handleActionSlot09CommonSpeech();
		break;
	case 311: // Usar ponchera (use punch bowl)
		handleActionSlot10CommonSpeech();
		break;
	case 312: // Usar florero con ponchera (use vase with punch bowl)
		break;
	case 313: // Usar vaso con ponchera (use glass with punch bowl)
		handleActionHandler313ExchangeItem0CFor0D();
		break;
	case 314: // Coger Húmero/hueso (take Húmero/bone)
		handleActionHandler314PickupBone();
		break;
	case 315: // Coger vaso (take glass)
		handleActionHandler315PickupItem0C();
		break;
	case 316: // Usar vaso con ponchera (use glass with punch bowl)
		handleActionHandler316SecondarySpeech();
		break;
	default:
		warning("Unhandled %s action handler %u", sceneDebugName(), handlerId);
		break;
	}
	return true;
}

bool Scene7030::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetX > 0x242)
		targetX = 0x242;

	while (targetY < 0x1df) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		++targetY;
	}

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		--targetY;
	}
	return true;
}

bool Scene7030::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)restoredStepDeltas;
	if (currentRegion == 6 && nextRegion == 1) {
		requestedFacing = 4;
		return true;
	}
	if (currentRegion == 1 && nextRegion == 6 && state.x < boundary.x && boundary.y <= state.y) {
		requestedFacing = 1;
		return true;
	}
	return false;
}

bool Scene7030::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	if (currentRegion == 6 && targetRegion == 6) {
		memcpy(_actorPathStepDeltas.data() + 12, kScene7030Route6StepDeltas,
			ARRAYSIZE(kScene7030Route6StepDeltas));
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 1 && targetRegion == 1 && state.x < targetX && targetY <= state.y) {
		requestedFacing = 1;
		return true;
	}
	return false;
}

bool Scene7030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0 || selector == 0xff) {
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		for (uint i = 0; i < _fullPaletteRegionMask.size(); ++i) {
			if (_paletteMaskOriginal[i] == 7)
				_fullPaletteRegionMask[i] = _sceneStateFlags[0] == 1 ? 0 : 1;
		}

		if (_sceneStateFlags[0] != 1 && _routeBoundaryPoints.size() > 0x100) {
			_routeBoundaryPoints[0x4b].x = 0x149;
			_routeBoundaryPoints[0x4b].y = 0x136;
			_routeBoundaryPoints[0x4c].x = 0x16f;
			_routeBoundaryPoints[0x4c].y = 0x13f;
			_routeBoundaryPoints[0xff].x = 0x14a;
			_routeBoundaryPoints[0xff].y = 0x136;
			_routeBoundaryPoints[0x100].x = 0x170;
			_routeBoundaryPoints[0x100].y = 0x13e;
		}

		rebuildWalkablePaletteMask();
	}

	if (selector == 2 || selector == 0xff) {
		restoreBaseFramebufferFromOriginal();

		if (_sceneStateFlags[2] != 0)
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		else
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);

		if (_paletteMaskOriginal.size() >= kScene7030ColorToItemMapOffset + kScene7030ColorMapSize &&
				_paletteMask.size() >= kScene7030ColorToItemMapOffset + kScene7030ColorMapSize) {
			memcpy(_paletteMask.data() + kScene7030ColorToItemMapOffset,
				_paletteMaskOriginal.data() + kScene7030ColorToItemMapOffset, kScene7030ColorMapSize);
			if (_sceneStateFlags[2] == 0) {
				for (uint i = 0; i < kScene7030ColorMapSize; ++i) {
					if (_paletteMaskOriginal[kScene7030ColorToItemMapOffset + i] == 5)
						_paletteMask[kScene7030ColorToItemMapOffset + i] = 0;
				}
			}
		}

		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}
	return true;
}

void Scene7030::setPrimaryLeftSpeechFrame(byte frameIndex) {
	setChunk5Frame(0x0b + frameIndex);
}

void Scene7030::updateAmbientAudioAndMusicCues(uint32 delta) {
	ensureAmbientSoundCuePlaying(1, kScene7030BaseAmbientSoundCue, 75);
	ensureAmbientSoundCuePlaying(2, kScene7030SecondaryAmbientSoundCue, 50);

	_ambientMusicTimerAccumulator += delta;
	if (_ambientMusicTimerAccumulator < kScene7030AmbientMusicCheckMillis)
		return;
	_ambientMusicTimerAccumulator %= kScene7030AmbientMusicCheckMillis;

	if (_vm->gameplayMusic()->isPlaying())
		return;

	GameplayState &state = _vm->gameState();
	if (state.currentAmbientMusicCueId != kScene7030AmbientMusicCueStillFrame) {
		_previousAmbientMusicTrackId = state.currentAmbientMusicCueId;
		state.currentAmbientMusicCueId = kScene7030AmbientMusicCueStillFrame;
		setChunk5Frame(0);
		_chunk5Animation.frameMillis = kScene7030Chunk5FrameMillis;
		_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, 75);
		return;
	}

	byte nextTrack = 0;
	do {
		nextTrack = (byte)(0x0c + _random.getRandomNumber(2));
	} while (nextTrack == _previousAmbientMusicTrackId);

	_previousAmbientMusicTrackId = state.currentAmbientMusicCueId;
	state.currentAmbientMusicCueId = nextTrack;
	setChunk5Frame(1);
	_chunk5FrameDirection = 1;
	_chunk5Animation.frameMillis = nextTrack == 0x0e ?
		kScene7030Chunk5FastFrameMillis : kScene7030Chunk5FrameMillis;
	_vm->gameplayMusic()->playMusicCue(state.currentAmbientMusicCueId, 75);
}

void Scene7030::advanceChunk5AmbientOverlay() {
	if (_vm->gameState().currentAmbientMusicCueId == kScene7030AmbientMusicCueStillFrame) {
		if (chunk5Frame() < ARRAYSIZE(kScene7030Chunk5FrameMap) &&
				kScene7030Chunk5FrameMap[chunk5Frame()] != 0) {
			if ((chunk5Frame() % 12) < 7)
				setChunk5Frame(chunk5Frame() - 1);
			else
				setChunk5Frame(chunk5Frame() + 1);
		}
		return;
	}

	if (_chunk9AmbientDecisionCounter == 0) {
		_chunk9AmbientDecisionCounter = (byte)(_random.getRandomNumber(3) + 1);
		_chunk5FrameDirection = _random.getRandomNumber(1) == 0 ? -1 : 1;
	}
	setChunk5Frame((byte)((int)chunk5Frame() + _chunk5FrameDirection));
	if (chunk5Frame() == 0)
		setChunk5Frame(0x18);
	else if (chunk5Frame() == 0x19)
		setChunk5Frame(1);
	--_chunk9AmbientDecisionCounter;
}

void Scene7030::advanceChunk6IdleFrames() {
	_chunk6IdlePairA.advance(_random);
	_chunk6IdlePairB.advance(_random);
	_animationLayers.setLayerFrame(kScene7030Chunk6IdleLayerA, _chunk6IdlePairA.firstFrame);
	_animationLayers.setLayerFrame(kScene7030Chunk6IdleLayerB, _chunk6IdlePairA.secondFrame);
	_animationLayers.setLayerFrame(kScene7030Chunk6IdleLayerC, _chunk6IdlePairB.firstFrame);
	_animationLayers.setLayerFrame(kScene7030Chunk6IdleLayerD, _chunk6IdlePairB.secondFrame);
}

void Scene7030::configureAnimationLayers() {
	_animationLayers.configure(kScene7030AnimationLayerSpecs);
}

void Scene7030::setChunk5Frame(byte frameIndex) {
	_animationLayers.setLayerFrame(kScene7030Chunk5Layer, frameIndex);
}

byte Scene7030::chunk5Frame() const {
	return _animationLayers.layerFrame(kScene7030Chunk5Layer);
}

void Scene7030::runPunchBowlPatchOverlay(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, uint32 frameMillis, int statePatchFrame) {
	const byte hookId = statePatchFrame >= 0 ? kScene7030PunchBowlGlassPatchHook : 0;
	runActorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount,
		frameMap, frameMapSize, frameMillis)
		.hookAt(statePatchFrame, hookId));
}

void Scene7030::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;

	if (hookId == kScene7030PunchBowlGlassPatchHook) {
		_sceneStateFlags[2] = 0;
		applySceneStateToHotspotsAndPatches(2);
	}
}

void Scene7030::handleActionSlot00TransitionToG04() {
	_vm->gameState().mainFlowStateId = kScene7030ExitState7040;
}

void Scene7030::handleActionSlot01SecondarySpeech() {
	beginSecondarySpeechLine(1, 0);
}

void Scene7030::handleActionSlot02TransitionToG01Alt() {
	_vm->gameState().mainFlowStateId = kScene7030ReturnState7011;
}

void Scene7030::handleActionSlot03SecondarySpeech() {
	beginSecondarySpeechLine(2, 0);
}

void Scene7030::handleActionSlot04SecondarySpeech() {
	beginSecondarySpeechLine(3, 0);
}

void Scene7030::handleActionSlot05ToggleSceneState0Speech() {
	if (_sceneStateFlags[0] == 1) {
		beginSecondarySpeechLine(4, 0);
		_sceneStateFlags[0] = 2;
		_vm->gameState().humeroBarrierState = _sceneStateFlags[0];
		applySceneStateToHotspotsAndPatches(0);
	} else {
		beginSecondarySpeechLine(4, 1);
	}
}

void Scene7030::handleActionSlot08CommonSpeech() {
	beginSecondarySpeechLine(7, 0);
	_sceneStateFlags[2] = 2;
	_vm->gameState().punchBowlGlassPatchState = _sceneStateFlags[2];
	applySceneStateToHotspotsAndPatches(2);
}

void Scene7030::handleActionSlot09CommonSpeech() {
	beginSecondarySpeechLine(8, 0);
}

void Scene7030::handleActionSlot10CommonSpeech() {
	beginSecondarySpeechLine(9, 0);
}

void Scene7030::handleActionHandler313ExchangeItem0CFor0D() {
	const bool speechStarted = startSecondarySpeechLine(10, 0);
	runPunchBowlPatchOverlay(11, kScene7030Chunk11DescriptorCount,
		kScene7030Chunk11ExchangeItem0CFrameMap, ARRAYSIZE(kScene7030Chunk11ExchangeItem0CFrameMap),
		kScene7030Chunk5FrameMillis);
	if (speechStarted)
		waitForSpeechOrDelay(0, false);
	clearSpeechOverlay();
	_speech.stop();
	removeInventoryItem(0x0c);
	addInventoryItem(0x0d);
	_vm->gameState().inventoryPanelDirty = true;
	_soundBank0.playSample(1, 100);
}

void Scene7030::handleActionHandler314PickupBone() {
	if (hasInventoryItem(0x0b)) {
		beginSecondarySpeechLine(5, 2);
		return;
	}
	if (_vm->gameState().currentAmbientMusicCueId != kScene7030AmbientMusicCueStillFrame) {
		beginSecondarySpeechLine(5, 3);
		return;
	}

	beginSecondarySpeechLine(5, 0);
	runPunchBowlPatchOverlay(7, kScene7030Chunk7DescriptorCount,
		kScene7030Chunk7PickupBoneFrameMap, ARRAYSIZE(kScene7030Chunk7PickupBoneFrameMap),
		kScene7030Chunk5FrameMillis);
	addInventoryItem(0x0b);
	_vm->gameState().inventoryPanelDirty = true;
	_soundBank0.playSample(1, 100);
	_sceneStateFlags[1] = 1;
	_vm->gameState().humeroBonePickupState = _sceneStateFlags[1];
	beginSecondarySpeechLine(5, 1);
}

void Scene7030::handleActionHandler315PickupItem0C() {
	if (_sceneStateFlags[2] == 1) {
		handleActionSlot08CommonSpeech();
		clearSpeechOverlay();
		_speech.stop();
	}

	runPunchBowlPatchOverlay(10, kScene7030Chunk10DescriptorCount,
		kScene7030Chunk10PickupItem0CFrameMap, ARRAYSIZE(kScene7030Chunk10PickupItem0CFrameMap),
		kScene7030Chunk5FrameMillis, 3);
	_vm->gameState().punchBowlGlassPatchState = _sceneStateFlags[2];
	addInventoryItem(0x0c);
	_vm->gameState().inventoryPanelDirty = true;
	_soundBank0.playSample(1, 100);
}

void Scene7030::handleActionHandler316SecondarySpeech() {
	beginSecondarySpeechLine(10, 1);
}

} // End of namespace Hollywood
