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

#include "hollywood/scenes/playable/scene3030.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const uint16 kScene3030FirstState = 0x0bd6;
const uint16 kScene3020EntryFromScene3030State = 0x0bcd;
const uint16 kScene3030ViewportXOffset = 0x0060;
const uint kScene3030ActorBankTableEntry = 0x0000;
const uint kScene3030ActorPaletteTableEntry = 0x00cc;
const uint kScene3030Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3030LoopFrameMillis = 150;
const uint32 kScene3030MachineFrameMillis = 75;
const uint32 kScene3030TransitionFrameMillis = 75;
const uint kScene3030LoopDescriptorCount = 0x0c;
const uint kScene3030MachineEffectDescriptorCount = 0x19;
const uint kScene3030MachineActionDescriptorCount = 0x0c;
const uint kScene3030MachineEffectLayer = 0;
const uint kScene3030MachineActionLayer = 1;
const uint kScene3030EntryTransitionChunk = 12;
const uint kScene3030EntryTransitionTableEntryCount = 0x20;
const byte kScene3030EntryTransitionFinalFrame = 0x1f;
const uint kScene3030ReturnTransitionChunk = 11;
const uint kScene3030ReturnTransitionTableEntryCount = 0x15;
const byte kScene3030ReturnTransitionFinalFrame = 0x14;
const byte kScene3030RequiredInventoryItem = 0x1f;
const byte kScene3030ResultInventoryItem = 0x41;

const byte kScene3030LoopFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene3030MachineEffectFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24
};

const byte kScene3030MachineActionFrameMap[] = {
	10, 10, 9, 8, 7, 6, 5, 4, 3, 2,
	1, 0, 0, 0, 0, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9, 10, 11
};

PlayableSceneConfig scene3030Config() {
	PlayableSceneConfig config(3030,
		SceneResourceLayout(11, 5, 10),
		SceneViewport(kScene3030ViewportXOffset, kScene3030ViewportXOffset, kScene3030ViewportXOffset),
		SceneActorPose(0x110, 0x18a, 2));
	config.setActorResources(kScene3030ActorBankTableEntry, kScene3030ActorPaletteTableEntry);
	config.setTextResources(kScene3030Resource003RowsOffsetIndex, kScene3030SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet00);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene3030::Scene3030(HollywoodEngine *vm) :
		PlayableScene(vm, scene3030Config()),
		_loopChannel(),
		_loopLayer(),
		_machineLayers(),
		_machineSequenceActive(false) {
	_loopLayer.configure(6, kScene3030LoopDescriptorCount,
		kScene3030LoopFrameMap, ARRAYSIZE(kScene3030LoopFrameMap));
}

void Scene3030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	setActiveActorPose(0x110, 0x18a, 2);
}

void Scene3030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;
	(void)activeWorldY;

	copyBaseFramebufferToSceneFramebuffer();
	if (_vm->gameState().windmillBladesMoving || _machineSequenceActive)
		drawResourceSpriteLayer(_loopLayer);
	if (_machineSequenceActive) {
		drawTransientLayers(_machineLayers);
		drawForegroundBlocks();
		return;
	}

	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks();
}

void Scene3030::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId != kScene3030FirstState) {
		PlayableScene::runCustomEntrySequence();
		return;
	}

	runEntryFromScene3020();
}

bool Scene3030::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene3030::advanceCustomGameplayLoop(uint32 delta) {
	if (_vm->gameState().windmillBladesMoving)
		advanceLoopingLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a camino anterior (go to previous forest area): return to scene 3020.
		runExitToScene3020();
		return true;
	case 302: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar/usar maquinaria (look/use machine), state-aware.
		beginSecondarySpeechLine(4, _vm->gameState().windmillBladesMoving ? 1 : 0);
		return true;
	case 307: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Mirar maquinaria/zona del claro (look at machine/clearing).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 312: // Usar objeto 0x1f con maquinaria: activates the windmill machinery.
		runMachineActivationSequence();
		return true;
	default:
		return false;
	}
}

bool Scene3030::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)targetX;
	(void)targetY;
	(void)restoredStepDeltas;

	if (currentRegion == 4 && state.drawOrderMode < 4) {
		requestedFacing = 1;
		return true;
	}

	return false;
}

bool Scene3030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 0) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		promoteMachineHotspots();
		rebuildWalkablePaletteMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

AmbientAudioProfile Scene3030::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	if (_vm->gameState().windmillBladesMoving) {
		profile.soundMode = kAmbientSoundLoop;
		profile.soundCueId = 0x0e;
		profile.soundVolumePercent = 30;
	}
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3030::resetAnimationLayers() {
	_loopChannel.reset(0, kScene3030LoopFrameMillis);
	_loopLayer.visible = true;
	_loopLayer.reset(0);
	_machineLayers.clear();
	_machineLayers.configureLayer(kScene3030MachineEffectLayer, 9, kScene3030MachineEffectDescriptorCount,
		kScene3030MachineEffectFrameMap, ARRAYSIZE(kScene3030MachineEffectFrameMap), false);
	_machineLayers.configureLayer(kScene3030MachineActionLayer, 10, kScene3030MachineActionDescriptorCount,
		kScene3030MachineActionFrameMap, ARRAYSIZE(kScene3030MachineActionFrameMap), false);
	_machineSequenceActive = false;
}

void Scene3030::advanceLoopingLayer(uint32 delta) {
	const uint frameCount = _loopChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		_loopChannel.frameIndex = _loopChannel.frameIndex + 1 < ARRAYSIZE(kScene3030LoopFrameMap) ?
			_loopChannel.frameIndex + 1 : 0;
		_loopLayer.setFrame(_loopChannel.frameIndex);
	}
}

void Scene3030::drawForegroundBlocks() {
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene3030::promoteMachineHotspots() {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	if (!_vm->gameState().scene3030MachineActivated) {
		for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
			if (_paletteMask[kSceneColorToItemMap + i] == 7)
				_paletteMask[kSceneColorToItemMap + i] = 2;
		}
		if (_sceneChunkTable.isValidChunk(7))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
		return;
	}

	if (_sceneChunkTable.isValidChunk(8))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == 2)
			_paletteMask[kSceneColorToItemMap + i] = 0;
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == 7)
			_paletteMask[kSceneColorToItemMap + i] = 3;
	}
}

void Scene3030::runEntryFromScene3020() {
	setActiveActorPose(0x110, 0x18a, 2);
	runDeltaTransitionClip(kScene3030EntryTransitionChunk,
		kScene3030EntryTransitionTableEntryCount, kScene3030EntryTransitionFinalFrame, false);
}

void Scene3030::runExitToScene3020() {
	drawPlayableComposite();
	presentFrame();
	runDeltaTransitionClip(kScene3030ReturnTransitionChunk,
		kScene3030ReturnTransitionTableEntryCount, kScene3030ReturnTransitionFinalFrame, true);
	_vm->gameState().mainFlowStateId = kScene3020EntryFromScene3030State;
}

void Scene3030::runDeltaTransitionClip(uint chunkIndex, uint tableEntryCount, byte finalFrameIndex,
		bool drawActorInBackground) {
	Common::Array<byte> clipData;
	if (!loadVariableChunk(chunkIndex, clipData))
		return;

	Graphics::ManagedSurface transitionBackground;
	transitionBackground.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	copyBaseFramebufferToSceneFramebuffer();
	// Only the return delta is encoded over the regular actor's starting pose.
	if (drawActorInBackground) {
		drawActiveAndSecondaryActorFrames(true, _activeActorFacing, _activeActorCel,
			_activeActorWorldX, _activeActorWorldY, false, 0, 0, 0, 0, -1);
	}
	drawForegroundBlocks();
	transitionBackground.copyRectToSurface(_sceneFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));

	uint32 frameAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	byte frameIndex = 0;

	drawDeltaTransitionFrame(clipData, tableEntryCount, frameIndex, *transitionBackground.surfacePtr());
	presentFrame();

	while (frameIndex < finalFrameIndex && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			break;

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		frameAccumulator += delta;
		if (_vm->gameState().windmillBladesMoving)
			advanceLoopingLayer(delta);
		updateAmbientAudioAndMusicCues(delta);

		bool frameDirty = false;
		// Each frame patches the previous result, so catch-up must apply every delta.
		while (frameAccumulator >= kScene3030TransitionFrameMillis && frameIndex < finalFrameIndex) {
			frameAccumulator -= kScene3030TransitionFrameMillis;
			++frameIndex;
			drawDeltaTransitionFrame(clipData, tableEntryCount, frameIndex,
				*transitionBackground.surfacePtr());
			frameDirty = true;
		}

		if (frameDirty)
			presentFrame();

		g_system->delayMillis(10);
	}
}

void Scene3030::drawDeltaTransitionFrame(const Common::Array<byte> &clipData, uint tableEntryCount,
		byte frameIndex, Graphics::Surface &transitionBackground) {
	_sceneFramebuffer.copyRectToSurface(transitionBackground, 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
	if (_vm->gameState().windmillBladesMoving)
		drawResourceSpriteLayer(_loopLayer);
	drawClipFrameDeltaToSurface(clipData, tableEntryCount, frameIndex, *_sceneFramebuffer.surfacePtr());
	drawClipFrameDeltaToSurface(clipData, tableEntryCount, frameIndex, transitionBackground);
}

void Scene3030::drawClipFrameDeltaToSurface(const Common::Array<byte> &clipData, uint tableEntryCount,
		byte frameIndex, Graphics::Surface &destination) {
	ResourceDeltaClipPlayer::drawFrame(clipData, 0, clipData.size(), tableEntryCount,
		frameIndex, (byte *)destination.getPixels(), destination.w, destination.h,
		destination.pitch, destination.pitch * destination.h);
}

void Scene3030::runMachineActivationSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene3030MachineActivated) {
		beginSecondarySpeechLine(4, 1);
		return;
	}
	if (!hasInventoryItem(kScene3030RequiredInventoryItem)) {
		beginSecondarySpeechLine(4, 0);
		return;
	}

	beginSecondarySpeechLine(11, 0);
	_machineSequenceActive = true;
	_machineLayers.setLayerVisible(kScene3030MachineEffectLayer, true);
	_machineLayers.setLayerVisible(kScene3030MachineActionLayer, true);
	_machineLayers.setLayerFrame(kScene3030MachineEffectLayer, 0);
	_machineLayers.setLayerFrame(kScene3030MachineActionLayer, 0);

	byte actionFrame = 0;
	byte effectFrame = 0;
	bool effectStarted = false;
	bool actionReleased = false;
	// Ron holds frame 15 until the machine effect reaches frame 5.
	while ((actionFrame + 1 < ARRAYSIZE(kScene3030MachineActionFrameMap) ||
			effectFrame + 1 < ARRAYSIZE(kScene3030MachineEffectFrameMap)) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (actionFrame + 1 < ARRAYSIZE(kScene3030MachineActionFrameMap) &&
				(actionFrame < 15 || actionReleased)) {
			++actionFrame;
			_machineLayers.setLayerFrame(kScene3030MachineActionLayer, actionFrame);
			if (actionFrame == 15) {
				effectStarted = true;
				if (_sceneChunkTable.isValidChunk(8))
					drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
			_soundBank0.playSample(0x1d, 100);
			}
		}

		if (effectStarted && effectFrame + 1 < ARRAYSIZE(kScene3030MachineEffectFrameMap)) {
			++effectFrame;
			_machineLayers.setLayerFrame(kScene3030MachineEffectLayer, effectFrame);
			if (effectFrame == 5) {
				actionReleased = true;
				state.scene3030MachineActivated = true;
				state.windmillBladesMoving = true;
				advanceLoopingLayer(kScene3030LoopFrameMillis);
			}
		}

		drawMachineSequenceFrame();
		if (waitSceneMillis(kScene3030MachineFrameMillis, false))
			break;
	}

	applySceneStateToHotspotsAndPatches(0);
	_machineSequenceActive = false;
	_machineLayers.setLayerVisible(kScene3030MachineEffectLayer, false);
	_machineLayers.setLayerVisible(kScene3030MachineActionLayer, false);
	removeInventoryItem(kScene3030RequiredInventoryItem);
	addInventoryItem(kScene3030ResultInventoryItem);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(11, 1);
	drawPlayableComposite();
	presentFrame();
}

void Scene3030::drawMachineSequenceFrame() {
	drawPlayableComposite();
	presentFrame();
}

} // End of namespace Hollywood
