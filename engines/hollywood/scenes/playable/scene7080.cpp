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

#include "hollywood/scenes/playable/scene7080.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene7080FirstState = 0x1ba8;
const uint16 kScene7080LastState = 0x1bb1;
const uint16 kScene7080BackToG07State = 0x1ba0;
const uint kScene7080InitialRequiredChunkCount = 9;
const uint kScene7080ArenaFirstChunk = 5;
const uint kScene7080ArenaLastChunk = 8;
const uint kScene7080StageIndex = 708;
const uint16 kScene7080ViewportXOffset = 0x68;
const int kScene7080EntryX = 0x1db;
const int kScene7080EntryY = 0x102;
const byte kScene7080EntryFacing = 2;
const uint16 kScene7080Chunk6DescriptorCount = 4;
const uint16 kScene7080Chunk7DescriptorCount = 0x0b;
const uint32 kScene7080FrameMillis = 75;
const uint32 kScene7080AmbientCheckMillis = 250;
const byte kScene7080TableItemColorId = 6;
const byte kScene7080PostPickupTableItemId = 4;
const byte kScene7080BackToG07FrameMap[] = {
	0, 1, 2, 3
};
const byte kScene7080PickupItem13FrameMap[] = {
	0, 6, 7, 8, 9, 10, 1, 2, 3, 3, 4, 5, 0
};

Scene7080::Scene7080(HollywoodEngine *vm) :
		SuePlayableScene(vm, "scene7080", kScene7080EntryX, kScene7080EntryY,
			kScene7080EntryFacing, 0xfd, 0xfb),
		_ambientTimerAccumulator(0) {
}

const char *Scene7080::resourceArchiveName() const {
	return "RESOURCE.G08";
}

uint Scene7080::sceneInitialRequiredChunkCount() const {
	return kScene7080InitialRequiredChunkCount;
}

uint Scene7080::sceneArenaFirstChunk() const {
	return kScene7080ArenaFirstChunk;
}

uint Scene7080::sceneArenaLastChunk() const {
	return kScene7080ArenaLastChunk;
}

uint Scene7080::sceneStageIndex() const {
	return kScene7080StageIndex;
}

const char *Scene7080::sceneDebugName() const {
	return "Scene 7080";
}

uint16 Scene7080::sceneViewportXOffset() const {
	return kScene7080ViewportXOffset;
}

bool Scene7080::isMainFlowStateInScene(uint16 stateId) const {
	return stateId >= kScene7080FirstState && stateId <= kScene7080LastState;
}

bool Scene7080::hasCustomPreviewState() const {
	return true;
}

void Scene7080::initializeCustomPreviewState() {
	_actionOverlayVisible = false;
	_actionOverlayChunkIndex = 0;
	_actionOverlayDescriptorCount = 0;
	_actionOverlayFrameIndex = 0;
	_hideActiveActor = false;
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_ambientTimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	_activeActorWorldX = kScene7080EntryX;
	_activeActorWorldY = kScene7080EntryY;
	_activeActorFacing = kScene7080EntryFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

bool Scene7080::hasCustomComposite() const {
	return true;
}

void Scene7080::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)drawSecondaryActor;
	(void)secondaryFacing;
	(void)secondaryFrame;
	(void)secondaryWorldX;
	(void)secondaryWorldY;
	(void)actorDrawOrderMode;

	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	if (drawActiveActor)
		drawActiveActorFrame(activeFacing, activeCel, activeWorldX, activeWorldY, -1);

	if (_actionOverlayVisible) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
			_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
	}

	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

bool Scene7080::hasCustomEntrySequence() const {
	return true;
}

void Scene7080::runCustomEntrySequence() {
	_soundBank0.playSample(4, 100);
	_activeActorWorldX = kScene7080EntryX;
	_activeActorWorldY = kScene7080EntryY;
	_activeActorFacing = kScene7080EntryFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	if (!state.g08IntroSeen) {
		beginSecondarySpeechLine(0, 0);
		state.g08IntroSeen = true;
	}
}

bool Scene7080::advanceCustomGameplayLoop(uint32 delta) {
	updateSceneAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene7080::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301:
		handleBackToG07();
		return true;
	case 302:
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303:
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304:
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305:
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306:
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307:
		handlePickupItem13();
		return true;
	case 308:
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309:
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310:
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311:
		beginSecondarySpeechLine(9, 0);
		return true;
	default:
		return false;
	}
}

bool Scene7080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 1 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		if (!_baseFramebufferOriginal.empty())
			memcpy(_baseFramebuffer.data(), _baseFramebufferOriginal.data(), _baseFramebuffer.size());

		GameplayState &state = _vm->gameState();
		if (!state.g08Item13OnTable)
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize &&
				_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
			for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
				const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
				if (!state.g08Item13OnTable && originalItem == kScene7080TableItemColorId)
					_paletteMask[kSceneColorToItemMap + i] = kScene7080PostPickupTableItemId;
			}
		}

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}
	return true;
}

void Scene7080::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 1)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene7080::updateSceneAmbientAudioAndMusicCues(uint32 delta) {
	_ambientTimerAccumulator += delta;
	if (_ambientTimerAccumulator < kScene7080AmbientCheckMillis)
		return;
	_ambientTimerAccumulator %= kScene7080AmbientCheckMillis;

	if (!_ambientSoundBank0.isPlaying())
		_ambientSoundBank0.playSample(0x0b, 50);

	if (_vm->gameplayMusic()->isPlaying())
		return;

	GameplayState &state = _vm->gameState();
	if (state.currentRandomAmbientMusicTrackId != 0x0f) {
		_previousAmbientMusicTrackId = state.currentRandomAmbientMusicTrackId;
		state.currentRandomAmbientMusicTrackId = 0x0f;
		_vm->gameplayMusic()->playMusicCue(state.currentRandomAmbientMusicTrackId, 50);
		return;
	}

	byte nextTrack = 0;
	do {
		nextTrack = (byte)(0x0c + _random.getRandomNumber(2));
	} while (nextTrack == _previousAmbientMusicTrackId);

	_previousAmbientMusicTrackId = state.currentRandomAmbientMusicTrackId;
	state.currentRandomAmbientMusicTrackId = nextTrack;
	_vm->gameplayMusic()->playMusicCue(state.currentRandomAmbientMusicTrackId, 50);
}

void Scene7080::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int statePatchFrame) {
	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	_actionOverlayVisible = true;
	_actionOverlayChunkIndex = (byte)chunkIndex;
	_actionOverlayDescriptorCount = (byte)descriptorCount;
	for (uint frame = 0; frame < frameMapSize && !Engine::shouldQuit(); ++frame) {
		_actionOverlayFrameIndex = frameMap[frame];
		if (statePatchFrame >= 0 && (int)frame == statePatchFrame) {
			_vm->gameState().g08Item13OnTable = false;
			applySceneStateToHotspotsAndPatches(1);
		}
		if (waitSceneMillis(frameMillis))
			break;
	}
	_actionOverlayVisible = false;
	_actionOverlayFrameIndex = 0;
	_hideActiveActor = previousHideActiveActor;
	drawPlayableComposite();
	presentFrame();
}

void Scene7080::handleBackToG07() {
	runOverlaySequence(6, kScene7080Chunk6DescriptorCount,
		kScene7080BackToG07FrameMap, ARRAYSIZE(kScene7080BackToG07FrameMap),
		kScene7080FrameMillis);
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene7080BackToG07State;
}

void Scene7080::handlePickupItem13() {
	dispatchGenericSceneAction(19);
	runOverlaySequence(7, kScene7080Chunk7DescriptorCount,
		kScene7080PickupItem13FrameMap, ARRAYSIZE(kScene7080PickupItem13FrameMap),
		kScene7080FrameMillis, 3);
	addInventoryItem(0x13);
	_soundBank0.playSample(1, 100);
}

} // End of namespace Hollywood
