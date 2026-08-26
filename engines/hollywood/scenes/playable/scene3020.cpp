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

#include "hollywood/scenes/playable/scene3020.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene3020EntryFromScene3010State = 0x0bcc;
const uint16 kScene3020EntryFromScene3030State = 0x0bcd;
const uint16 kScene3010EntryFromScene3020State = 0x0bc3;
const uint16 kScene3030State = 0x0bd6;
const uint16 kScene3020ViewportXOffset = 0x0064;
const uint kScene3020ActorBankTableEntry = 0x0000;
const uint kScene3020ActorPaletteTableEntry = 0x00cc;
const uint kScene3020Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3020SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3020LoopFrameMillis = 150;
const uint32 kScene3020PickupFrameMillis = 75;
const uint32 kScene3020TransitionFrameMillis = 75;
const uint kScene3020LoopDescriptorCount = 0x1e;
const uint kScene3020PickupDescriptorCount = 0x0e;
const uint kScene3020ReturnTransitionChunk = 12;
const uint kScene3020ReturnTransitionDescriptorCount = 0x43;
const byte kScene3020ReturnTransitionFinalFrame = 0x42;
const byte kScene3020PickupInventoryItem = 0x31;

const byte kScene3020LoopFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29
};

const byte kScene3020PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13
};

PlayableSceneConfig scene3020Config() {
	PlayableSceneConfig config(3020,
		SceneResourceLayout(11, 5, 10),
		SceneViewport(kScene3020ViewportXOffset, kScene3020ViewportXOffset, kScene3020ViewportXOffset),
		SceneActorPose(0x0cf, 0x152, 4));
	config.setActorResources(kScene3020ActorBankTableEntry, kScene3020ActorPaletteTableEntry);
	config.setTextResources(kScene3020Resource003RowsOffsetIndex, kScene3020SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet00);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene3020::Scene3020(HollywoodEngine *vm) :
		PlayableScene(vm, scene3020Config()),
		_loopChannel(),
		_loopLayer() {
	_loopLayer.configure(7, kScene3020LoopDescriptorCount,
		kScene3020LoopFrameMap, ARRAYSIZE(kScene3020LoopFrameMap));
}

void Scene3020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	if (_vm->gameState().mainFlowStateId == kScene3020EntryFromScene3030State) {
		_activeActorWorldX = 0x13d;
		_activeActorWorldY = 0x138;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x0cf;
		_activeActorWorldY = 0x152;
		_activeActorFacing = 4;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene3020::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_loopLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldY);
}

void Scene3020::runCustomEntrySequence() {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId != kScene3020EntryFromScene3010State && stateId != kScene3020EntryFromScene3030State) {
		PlayableScene::runCustomEntrySequence();
		return;
	}

	if (stateId == kScene3020EntryFromScene3030State)
		runEntryFromScene3030();
	else
		runEntryFromScene3010();
}

bool Scene3020::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	rebuildWalkableMask();
	return true;
}

bool Scene3020::advanceCustomGameplayLoop(uint32 delta) {
	if (_vm->gameState().windmillBladesMoving)
		advanceLoopingLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3020::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 144: // Mirar maza (look at mace): scene metadata reuses Ron's inventory look handler.
		beginStaticSecondarySpeechLine(0x85, 0);
		return true;
	case 301: // Ir a exterior del molino (go to windmill exterior): return to scene 3010.
		_vm->gameState().mainFlowStateId = kScene3010EntryFromScene3020State;
		return true;
	case 302: // Mirar camino/bosque (look at path/forest).
		beginSecondarySpeechLine(0, 0);
		return true;
	case 303: // Coger maza (take mace): pickup inventory item 0x31.
		runPickupMace();
		return true;
	case 304: // Ir a claro/maquinaria (go to next forest area): transition to scene 3030.
		_vm->gameState().mainFlowStateId = kScene3030State;
		return true;
	case 305: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 306: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 307: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 308: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 309: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 310: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 311: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 312: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 313: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 314: // Mirar zona del camino (look at scene object).
		beginSecondarySpeechLine(10, 0);
		return true;
	default:
		return false;
	}
}

bool Scene3020::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;
	(void)restoredStepDeltas;

	if (currentRegion == 3) {
		requestedFacing = 1;
		return true;
	}

	return false;
}

bool Scene3020::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 0 || selector == 1) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

		if (_vm->gameState().scene3020MaceTaken) {
			if (_sceneChunkTable.isValidChunk(8))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
			removeTakenItemHotspots();
		}

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

AmbientAudioProfile Scene3020::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	if (_vm->gameState().windmillBladesMoving) {
		profile.soundMode = kAmbientSoundLoop;
		profile.soundCueId = 0x0b;
		profile.soundVolumePercent = 30;
	}
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3020::resetAnimationLayers() {
	_loopChannel.reset(0, kScene3020LoopFrameMillis);
	_loopLayer.visible = true;
	_loopLayer.reset(0);
}

void Scene3020::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 1 && _walkablePaletteMask[i] < 4)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene3020::advanceLoopingLayer(uint32 delta) {
	const uint frameCount = _loopChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		_loopChannel.frameIndex = _loopChannel.frameIndex + 1 < ARRAYSIZE(kScene3020LoopFrameMap) ?
			_loopChannel.frameIndex + 1 : 0;
		_loopLayer.setFrame(_loopChannel.frameIndex);
	}
}

void Scene3020::drawForegroundBlocks(int activeWorldY) {
	uint chunkIndex = 5;
	if (activeWorldY < 0x125)
		chunkIndex = _vm->gameState().scene3020MaceTaken ? 6 : 10;

	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

void Scene3020::removeTakenItemHotspots() {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == 2)
			_paletteMask[kSceneColorToItemMap + i] = 0;
	}
}

void Scene3020::runEntryFromScene3010() {
	runEntryPath(0x0ff, 0x109, 4, 0x0cf, 0x152);
}

void Scene3020::runEntryFromScene3030() {
	setActiveActorPose(0x13d, 0x138, 4);
	runDescriptorTransitionClip(kScene3020ReturnTransitionChunk,
		kScene3020ReturnTransitionDescriptorCount, kScene3020ReturnTransitionFinalFrame);
}

void Scene3020::runDescriptorTransitionClip(uint chunkIndex, uint descriptorCount, byte finalFrameIndex) {
	Common::Array<byte> clipData;
	if (!loadVariableChunk(chunkIndex, clipData))
		return;

	uint32 frameAccumulator = 0;
	uint32 lastMillis = g_system->getMillis();
	byte frameIndex = 0;

	drawDescriptorTransitionFrame(clipData, descriptorCount, frameIndex);
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
		while (frameAccumulator >= kScene3020TransitionFrameMillis && frameIndex < finalFrameIndex) {
			frameAccumulator -= kScene3020TransitionFrameMillis;
			++frameIndex;
			frameDirty = true;
		}

		if (frameDirty) {
			drawDescriptorTransitionFrame(clipData, descriptorCount, frameIndex);
			presentFrame();
		}

		g_system->delayMillis(10);
	}
}

void Scene3020::drawDescriptorTransitionFrame(const Common::Array<byte> &clipData, uint descriptorCount, byte frameIndex) {
	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_loopLayer);
	// The continuation descriptors contain Ron and their own occlusion.
	drawStripSpriteFrame(clipData, 0, 0, descriptorCount, frameIndex, _sceneFramebuffer);
}

void Scene3020::runPickupMace() {
	if (_vm->gameState().scene3020MaceTaken) {
		beginSecondarySpeechLine(1, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(9, kScene3020PickupDescriptorCount,
		kScene3020PickupFrameMap, ARRAYSIZE(kScene3020PickupFrameMap), kScene3020PickupFrameMillis)
		.patchAt(7, 1));
	_vm->gameState().scene3020MaceTaken = true;
	applySceneStateToHotspotsAndPatches(1);
	addInventoryItem(kScene3020PickupInventoryItem);
	_soundBank0.playSample(1, 100);
}

} // End of namespace Hollywood
