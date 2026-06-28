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

#include "hollywood/scenes/playable/scene7090.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene7090FirstState = 0x1bb2;
const uint16 kScene7090LastState = 0x1bbb;
const uint16 kScene7090BackToG07State = 0x1ba0;
const uint16 kScene7090ExitToG10State = 0x1bbc;
const uint kScene7090InitialRequiredChunkCount = 11;
const uint kScene7090ArenaFirstChunk = 5;
const uint kScene7090ArenaLastChunk = 10;
const uint kScene7090StageIndex = 709;
const uint16 kScene7090ViewportXOffset = 0x68;
const int kScene7090EntryX = 0x1dd;
const int kScene7090EntryY = 0x101;
const byte kScene7090EntryFacing = 2;
const byte kScene7090IntroTurnFacing = 4;
const uint16 kScene7090Chunk9DescriptorCount = 4;
const uint16 kScene7090Chunk10DescriptorCount = 0x16;
const uint32 kScene7090FrameMillis = 75;
const uint32 kScene7090AmbientCheckMillis = 250;
const uint kScene7090Item08VerbRecordIndex = 0x45;
const uint kScene7090ActorPaletteOffset = 0x270;
const uint kScene7090ActorPaletteColorCount = 0x19;
const int kScene7090ForegroundYThreshold = 0x132;
const int kScene7090GatedActionTargetX = 0x2ce;
const int kScene7090GatedActionTargetY = 0x11b;
const byte kScene7090GatedActionTargetFacing = 5;
const int kScene7090GatedActionReturnX = 0x281;
const int kScene7090GatedActionReturnY = 0x10d;
const byte kScene7090BackToG07FrameMap[] = {
	0, 1, 2, 3
};
const byte kScene7090GatedActionFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21
};

Scene7090::Scene7090(HollywoodEngine *vm) :
		SuePlayableScene(vm, "scene7090", kScene7090EntryX, kScene7090EntryY,
			kScene7090EntryFacing, 0xfd, 0xfb),
		_ambientTimerAccumulator(0),
		_prePatchChunk7Visible(false) {
}

const char *Scene7090::resourceArchiveName() const {
	return "RESOURCE.G09";
}

uint Scene7090::sceneInitialRequiredChunkCount() const {
	return kScene7090InitialRequiredChunkCount;
}

uint Scene7090::sceneArenaFirstChunk() const {
	return kScene7090ArenaFirstChunk;
}

uint Scene7090::sceneArenaLastChunk() const {
	return kScene7090ArenaLastChunk;
}

uint Scene7090::sceneStageIndex() const {
	return kScene7090StageIndex;
}

const char *Scene7090::sceneDebugName() const {
	return "Scene 7090";
}

uint16 Scene7090::sceneViewportXOffset() const {
	return kScene7090ViewportXOffset;
}

bool Scene7090::isMainFlowStateInScene(uint16 stateId) const {
	return stateId >= kScene7090FirstState && stateId <= kScene7090LastState;
}

bool Scene7090::hasCustomPreviewState() const {
	return true;
}

void Scene7090::initializeCustomPreviewState() {
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
	_prePatchChunk7Visible = false;
	_activeActorWorldX = kScene7090EntryX;
	_activeActorWorldY = kScene7090EntryY;
	_activeActorFacing = kScene7090EntryFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
	darkenActorPaletteRange();
}

bool Scene7090::hasCustomComposite() const {
	return true;
}

void Scene7090::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	if (_prePatchChunk7Visible)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	if (_actionOverlayVisible) {
		drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[_actionOverlayChunkIndex], 0,
			_actionOverlayDescriptorCount, _actionOverlayFrameIndex, _sceneFramebuffer);
	}

	if (activeWorldY < kScene7090ForegroundYThreshold)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

bool Scene7090::hasCustomEntrySequence() const {
	return true;
}

void Scene7090::runCustomEntrySequence() {
	_soundBank0.playSample(4, 100);
	_activeActorWorldX = kScene7090EntryX;
	_activeActorWorldY = kScene7090EntryY;
	_activeActorFacing = kScene7090EntryFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	if (!state.g09IntroSeen) {
		walkActiveActorTo(kScene7090EntryX, kScene7090EntryY, kScene7090IntroTurnFacing, 0);
		const byte pathFacing = _activeActorFacing;
		const byte pathCel = _activeActorCel;
		_activeActorFacing = kScene7090EntryFacing;
		_activeActorCel = 0;
		beginSecondarySpeechLine(0, 0);
		_activeActorFacing = pathFacing;
		_activeActorCel = pathCel;
		state.g09IntroSeen = true;
	}
}

bool Scene7090::advanceCustomGameplayLoop(uint32 delta) {
	updateSceneAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene7090::dispatchCustomSceneAction(uint16 handlerId) {
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
		_vm->gameState().mainFlowStateId = kScene7090ExitToG10State;
		return true;
	case 307:
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308:
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309:
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310:
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311:
		handleGatedAction();
		return true;
	default:
		return false;
	}
}

bool Scene7090::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 1 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		if (!_baseFramebufferOriginal.empty())
			memcpy(_baseFramebuffer.data(), _baseFramebufferOriginal.data(), _baseFramebuffer.size());

		GameplayState &state = _vm->gameState();
		if (state.g09PatchState) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		}

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize &&
				_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
			for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
				const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
				if (state.g09PatchState) {
					if (originalItem == 9 || originalItem == 0x0b)
						_paletteMask[kSceneColorToItemMap + i] = 8;
					else if (originalItem == 0x0a)
						_paletteMask[kSceneColorToItemMap + i] = 2;
					else if (originalItem == 0x0c)
						_paletteMask[kSceneColorToItemMap + i] = 0;
				} else {
					if (originalItem == 9)
						_paletteMask[kSceneColorToItemMap + i] = 2;
					else if (originalItem == 0x0a || originalItem == 0x0c)
						_paletteMask[kSceneColorToItemMap + i] = 8;
					else if (originalItem == 0x0b)
						_paletteMask[kSceneColorToItemMap + i] = 0;
				}
			}
		}

		const uint interactionOffset = kSceneItemInteractionPoints + 8 * sizeof(ScenePoint);
		if (_metadata.size() >= interactionOffset + sizeof(ScenePoint)) {
			const uint16 x = state.g09PatchState ? 0x1fc : 0x245;
			const uint16 y = state.g09PatchState ? 0x110 : 0x11f;
			_metadata[interactionOffset] = x & 0xff;
			_metadata[interactionOffset + 1] = x >> 8;
			_metadata[interactionOffset + 2] = y & 0xff;
			_metadata[interactionOffset + 3] = y >> 8;
		}

		const uint approachOffset = kSceneItemApproachPoints + 8 * sizeof(ScenePoint);
		if (_metadata.size() >= approachOffset + sizeof(ScenePoint)) {
			const uint16 x = state.g09PatchState ? 0x23a : 0x288;
			const uint16 y = state.g09PatchState ? 0x096 : 0x0a8;
			_metadata[approachOffset] = x & 0xff;
			_metadata[approachOffset + 1] = x >> 8;
			_metadata[approachOffset + 2] = y & 0xff;
			_metadata[approachOffset + 3] = y >> 8;
		}

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
		if (state.g09PatchState)
			_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene7090Item08VerbRecordIndex, 0);
	}
	return true;
}

void Scene7090::darkenActorPaletteRange() {
	if (_paletteCurrent.size() < kScene7090ActorPaletteOffset + kScene7090ActorPaletteColorCount * 3)
		return;

	for (uint color = 0; color < kScene7090ActorPaletteColorCount; ++color) {
		const uint offset = kScene7090ActorPaletteOffset + color * 3;
		for (uint component = 1; component < 3; ++component) {
			const uint paletteOffset = offset + component;
			_paletteCurrent[paletteOffset] -= _paletteCurrent[paletteOffset] >> 2;
		}
	}
}

void Scene7090::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 3)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene7090::updateSceneAmbientAudioAndMusicCues(uint32 delta) {
	_ambientTimerAccumulator += delta;
	if (_ambientTimerAccumulator < kScene7090AmbientCheckMillis)
		return;
	_ambientTimerAccumulator %= kScene7090AmbientCheckMillis;

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

void Scene7090::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis) {
	_actionOverlayVisible = true;
	_actionOverlayChunkIndex = (byte)chunkIndex;
	_actionOverlayDescriptorCount = (byte)descriptorCount;
	for (uint frame = 0; frame < frameMapSize && !Engine::shouldQuit(); ++frame) {
		_actionOverlayFrameIndex = frameMap[frame];
		if (waitSceneMillis(frameMillis))
			break;
	}
	_actionOverlayVisible = false;
	_actionOverlayFrameIndex = 0;
	drawPlayableComposite();
	presentFrame();
}

void Scene7090::handleBackToG07() {
	runOverlaySequence(9, kScene7090Chunk9DescriptorCount,
		kScene7090BackToG07FrameMap, ARRAYSIZE(kScene7090BackToG07FrameMap),
		kScene7090FrameMillis);
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene7090BackToG07State;
}

void Scene7090::handleGatedAction() {
	GameplayState &state = _vm->gameState();
	if (state.g09PatchState) {
		beginSecondarySpeechLine(10, 3);
		return;
	}

	if (!state.g09ActionGate) {
		beginSecondarySpeechLine(10, 0);
		return;
	}

	beginSecondarySpeechLine(10, 1);
	walkActiveActorTo(kScene7090GatedActionTargetX, kScene7090GatedActionTargetY,
		kScene7090GatedActionTargetFacing, 0);

	_prePatchChunk7Visible = true;
	_actionOverlayVisible = true;
	_actionOverlayChunkIndex = 10;
	_actionOverlayDescriptorCount = kScene7090Chunk10DescriptorCount;
	for (uint frame = 0; frame < ARRAYSIZE(kScene7090GatedActionFrameMap) && !Engine::shouldQuit(); ++frame) {
		_actionOverlayFrameIndex = kScene7090GatedActionFrameMap[frame];
		if (frame == 3)
			_soundBank0.playSample(0x1b, 100);
		if (frame == 0x12)
			_soundBank0.stop();
		if (waitSceneMillis(kScene7090FrameMillis))
			break;
	}
	_actionOverlayVisible = false;
	_actionOverlayFrameIndex = 0;
	_prePatchChunk7Visible = false;

	state.g09PatchState = true;
	applySceneStateToHotspotsAndPatches(1);
	walkActiveActorTo(kScene7090GatedActionReturnX, kScene7090GatedActionReturnY,
		kScene7090GatedActionTargetFacing, 0);
	beginSecondarySpeechLine(10, 2);
}

} // End of namespace Hollywood
