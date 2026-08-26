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

#include "hollywood/scenes/playable/scene3040.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene3040State = 0x0be0;
const uint16 kScene3010EntryFromScene3040State = 0x0bc3;
const uint16 kScene3040ViewportXOffset = 0x0060;
const uint kScene3040ActorBankTableEntry = 0x0000;
const uint kScene3040ActorPaletteTableEntry = 0x00cc;
const uint kScene3040Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3040SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3040LoopFrameMillis = 125;
const uint32 kScene3040ForegroundFrameMillis = 75;
const uint32 kScene3040ForegroundIdleFrameMillis = 150;
const uint kScene3040ForegroundActorDescriptorCount = 0x14;
const uint kScene3040LoopDescriptorCount = 8;
const byte kScene3040HiddenObjectItemId = 3;
const byte kScene3040HiddenObjectPatchChunk = 7;
const byte kScene3040HiddenObjectPatchHook = 1;

const byte kScene3040ForegroundFrameMap[] = {
	0, 1, 2, 1, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 0, 14, 15, 16, 17, 18,
	19
};

const byte kScene3040LoopFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7
};

static PlayableSceneConfig scene3040Config() {
	PlayableSceneConfig config(3040,
		SceneResourceLayout(8, 5, 7),
		SceneViewport(kScene3040ViewportXOffset, kScene3040ViewportXOffset, kScene3040ViewportXOffset),
		SceneActorPose(0x210, 0x139, 2));
	config.setActorResources(kScene3040ActorBankTableEntry, kScene3040ActorPaletteTableEntry);
	config.setTextResources(kScene3040Resource003RowsOffsetIndex, kScene3040SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet00);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

static void drawLooseSpriteFrame(const Common::Array<byte> &resource, uint32 baseOffset,
		uint16 descriptorCount, uint16 descriptorIndex, Graphics::ManagedSurface &destination) {
	const uint entryOffset = baseOffset + kFrameDescriptorSize * descriptorIndex;
	if (entryOffset + kFrameDescriptorSize > resource.size())
		return;

	const uint16 spanCount = readUint16LE(resource, entryOffset + 12);
	uint cursor = baseOffset + kFrameDescriptorSize * descriptorCount + readUint32LE(resource, entryOffset);
	if (cursor > resource.size())
		return;

	for (uint spanIndex = 0; spanIndex < spanCount; ++spanIndex) {
		if (cursor + 5 > resource.size())
			return;

		const uint32 packedDestination = readUint32LE(resource, cursor);
		const int dataLength = resource[cursor + 4];
		cursor += 5;

		if (cursor + dataLength > resource.size())
			return;

		const int x = packedDestination & 0xffff;
		const int y = (int)((packedDestination >> 16) & 0xffff);
		if (y >= 0 && y < destination.h && x >= 0 && x < destination.w) {
			const int drawWidth = MIN<int>(dataLength, destination.w - x);
			if (drawWidth > 0)
				destination.copyRectToSurface(resource.data() + cursor, dataLength, x, y, drawWidth, 1);
		}

		cursor += dataLength;
	}
}

Scene3040::Scene3040(HollywoodEngine *vm) :
		PlayableScene(vm, scene3040Config()),
		_loopChannel(),
		_foregroundActorChannel(),
		_foregroundActorLayer(),
		_loopLayer(),
		_foregroundActorBlinkActive(false),
		_foregroundActionActive(false) {
	_foregroundActorLayer.configure(5, kScene3040ForegroundActorDescriptorCount,
		kScene3040ForegroundFrameMap, ARRAYSIZE(kScene3040ForegroundFrameMap));
	_loopLayer.configure(6, kScene3040LoopDescriptorCount,
		kScene3040LoopFrameMap, ARRAYSIZE(kScene3040LoopFrameMap));
}

void Scene3040::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	setActiveActorPose(0x210, 0x139, 2);
}

void Scene3040::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)drawActiveActor;
	(void)activeFacing;
	(void)activeCel;
	(void)activeWorldX;
	(void)activeWorldY;
	(void)drawSecondaryActor;
	(void)secondaryFacing;
	(void)secondaryFrame;
	(void)secondaryWorldX;
	(void)secondaryWorldY;
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawLooseResourceSpriteLayer(_foregroundActorLayer);
	drawLooseResourceSpriteLayer(_loopLayer);
}

void Scene3040::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId != kScene3040State) {
		PlayableScene::runCustomEntrySequence();
		return;
	}

	runEntryPath(0x210, 0x139, 2, 0x210, 0x139);
	resetAnimationLayers();
	drawPlayableComposite();
	presentFrame();

	if (!_vm->gameState().scene3040EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene3040EntryLineSeen = true;
	}
}

bool Scene3040::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	rebuildWalkableMask();
	return true;
}

bool Scene3040::advanceCustomGameplayLoop(uint32 delta) {
	advanceLoopingLayer(delta);
	advanceForegroundActorLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3040::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar tejado (look at roof).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Ir al suelo (go to ground): return toward scene 3010.
		runExitToScene3010();
		return true;
	case 303: // Coger artilugio del Dr. Mosca (pick up Dr. Mosca's device).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar artilugio del Dr. Mosca (look at Dr. Mosca's device).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Usar artilugio del Dr. Mosca (use Dr. Mosca's device).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 89: // Usar artilugio del Dr. Mosca (use device from inventory): install it here.
	case 306: // Usar artilugio del Dr. Mosca con tejado (use device on roof): install it.
		runInventoryPatchAction();
		return true;
	default:
		return false;
	}
}

bool Scene3040::shouldPlayGameplayClickPath() const {
	return false;
}

bool Scene3040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0xff || selector == 0 || selector == 1) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		updateHiddenObjectHotspots();
		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}

	return true;
}

AmbientAudioProfile Scene3040::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene3040::resetAnimationLayers() {
	_loopChannel.reset(0, kScene3040LoopFrameMillis);
	_foregroundActorChannel.reset(0, kScene3040ForegroundIdleFrameMillis);
	_foregroundActorLayer.visible = true;
	_loopLayer.visible = true;
	_foregroundActorLayer.reset(0);
	_loopLayer.reset(0);
	_foregroundActorBlinkActive = false;
	_foregroundActionActive = false;
}

void Scene3040::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}
}

void Scene3040::advanceLoopingLayer(uint32 delta) {
	const uint frameCount = _loopChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		_loopChannel.frameIndex = _loopChannel.frameIndex + 1 < ARRAYSIZE(kScene3040LoopFrameMap) ?
			_loopChannel.frameIndex + 1 : 0;
		_loopLayer.setFrame(_loopChannel.frameIndex);
	}
}

void Scene3040::advanceForegroundActorLayer(uint32 delta) {
	if (_foregroundActionActive || _actorPathPlaybackActive)
		return;

	const uint frameCount = _foregroundActorChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_speechOverlay.visible) {
			byte nextFrame;
			do {
				nextFrame = (byte)_random.getRandomNumber(4);
			} while (nextFrame == _foregroundActorLayer.frameIndex);
			_foregroundActorLayer.setFrame(nextFrame);
			_foregroundActorBlinkActive = false;
			continue;
		}

		if (_foregroundActorLayer.frameIndex != 0) {
			_foregroundActorLayer.setFrame(0);
			_foregroundActorBlinkActive = false;
			continue;
		}

		if (!_foregroundActorBlinkActive && _random.getRandomNumber(14) == 0) {
			_foregroundActorLayer.setFrame(4);
			_foregroundActorBlinkActive = true;
		}
	}
}

void Scene3040::drawLooseResourceSpriteLayer(const ResourceSpriteLayer &layer) {
	if (!layer.visible || layer.chunkIndex >= HollywoodEngine::kResourceChunkCount ||
			!_sceneChunkTable.isValidChunk(layer.chunkIndex))
		return;

	drawLooseSpriteFrame(_resourceArena, _resourceChunkOffsets[layer.chunkIndex],
		layer.descriptorCount, layer.descriptorIndex(), _sceneFramebuffer);
}

void Scene3040::updateHiddenObjectHotspots() {
	if (_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == kScene3040HiddenObjectItemId) {
			_paletteMask[kSceneColorToItemMap + i] =
				_vm->gameState().scene3040HiddenObjectVisible ? kScene3040HiddenObjectItemId : 0;
		}
	}

	if (_vm->gameState().scene3040HiddenObjectVisible)
		applyHiddenObjectPatch();
}

void Scene3040::runExitToScene3010() {
	_foregroundActionActive = true;
	playAnimationFrames(_foregroundActorLayer,
		AnimationFrameRange(0x0e, 0x14, kScene3040ForegroundFrameMillis));
	_foregroundActionActive = false;
	_vm->gameState().mainFlowStateId = kScene3010EntryFromScene3040State;
}

void Scene3040::runInventoryPatchAction() {
	beginSecondarySpeechLine(1, 5);
	_foregroundActionActive = true;
	playAnimationFrames(_foregroundActorLayer,
		AnimationFrameRange(4, 0x0e, kScene3040ForegroundFrameMillis)
			.hookAt(0x0b, kScene3040HiddenObjectPatchHook));
	_foregroundActionActive = false;

	const byte inventoryItem = selectedInventoryItemForPatchAction();
	if (inventoryItem != 0)
		removeInventoryItem(inventoryItem);

	_vm->gameState().scene3040HiddenObjectVisible = true;
	applySceneStateToHotspotsAndPatches(1);
	_soundBank0.playSample(1, 100);
	drawPlayableComposite();
	presentFrame();
}

void Scene3040::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene3040HiddenObjectPatchHook && frame == 0x0b) {
		_vm->gameState().scene3040HiddenObjectVisible = true;
		applySceneStateToHotspotsAndPatches(1);
	}
}

void Scene3040::applyHiddenObjectPatch() {
	if (_sceneChunkTable.isValidChunk(kScene3040HiddenObjectPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene3040HiddenObjectPatchChunk], _baseFramebuffer);
}

byte Scene3040::selectedInventoryItemForPatchAction() const {
	if (_lastInventoryPrimaryItemId != 0)
		return _lastInventoryPrimaryItemId;
	return _lastInventoryActionItemId;
}

} // End of namespace Hollywood
