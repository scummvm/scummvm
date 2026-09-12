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

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/scenes/playable/scene4040.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const uint16 kScene4040FirstState = 0x0fc8;
const uint16 kScene4040ReturnFromUpperExitState = 0x0fc9;
const uint16 kScene4030ReturnState = 0x0fbf;
const uint16 kScene4050FirstState = 0x0fd2;
const uint16 kScene4040ViewportXOffset = 0x0068;
const uint16 kScene4040ViewportMinXOffset = 0x0068;
const uint16 kScene4040ViewportMaxXOffset = 0x00b8;
const uint kScene4040ActorBankTableEntry = 0x0000;
const uint kScene4040ActorPaletteTableEntry = 0x00cc;
const uint kScene4040Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4040SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4040FrameMillis = 75;
const int kScene4040UpperExitReturnStartX = 0x028a;
const int kScene4040UpperExitReturnStartY = 0x0128;
const byte kScene4040UpperExitReturnFacing = 1;
const int kScene4040UpperExitReturnTargetX = 0x024b;
const int kScene4040UpperExitReturnTargetY = 0x014a;
const uint kScene4040CyclicBackgroundChunk = 8;
const uint kScene4040RandomBackgroundChunk = 13;
const uint kScene4040StairExitClipChunk = 9;
const uint kScene4040StairEntryClipChunk = 10;
const uint kScene4040DarkActorPaletteChunk = 11;
const uint kScene4040LightActorPaletteChunk = 12;
const uint kScene4040StairClipFrameCount = 0x3e;
const uint kScene4040BackgroundDescriptorCount = 0x1a;
const uint kScene4040RandomBackgroundLayer = 0;
const uint kScene4040CyclicBackgroundLayer = 1;
const uint kScene4040CandilOverlayChunk = 14;
const uint kScene4040CandilOverlayDescriptorCount = 9;
const byte kScene4040CandilItem = 0x3c;
const byte kScene4040DemoCordItem = 0x1b;
const byte kScene4040CandilSceneItem = 8;
const byte kScene4040PaletteOverrideColor = 0xfb;
const byte kScene4040ActorPaletteFirstColor = 0xd0;
const byte kScene4040ActorPaletteLastColor = 0xe8;
const uint32 kScene4040AmbientCheckMillis = 250;

const byte kScene4040EntryFootstepFrames[] = {
	3, 9, 15, 21, 27, 33, 39, 46, 52, 58
};

const byte kScene4040ExitFootstepFrames[] = {
	3, 9, 15, 23, 29, 35, 41, 47, 53, 59
};

const SceneLayerSpec kScene4040LayerSpecs[] = {
	{kSceneAnimationBehindActors, kScene4040RandomBackgroundChunk,
		kScene4040BackgroundDescriptorCount, nullptr, 0, true, 0},
	{kSceneAnimationBehindActors, kScene4040CyclicBackgroundChunk,
		kScene4040BackgroundDescriptorCount, nullptr, 0, true, 0}
};

PlayableSceneConfig scene4040Config() {
	PlayableSceneConfig config(4040,
		SceneResourceLayout(5, 5, 15),
		SceneViewport(kScene4040ViewportXOffset, kScene4040ViewportMinXOffset, kScene4040ViewportMaxXOffset),
		SceneActorPose(0x192, 0x0171, 2));
	config.setActorResources(kScene4040ActorBankTableEntry, kScene4040ActorPaletteTableEntry);
	config.setTextResources(kScene4040Resource003RowsOffsetIndex, kScene4040SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene4040::Scene4040(HollywoodEngine *vm) :
		PlayableScene(vm, scene4040Config()),
		_randomBackgroundChannel(),
		_cyclicBackgroundTrack(RealtimeAnimationTracks::kInvalidTrack),
		_randomBackgroundState(0),
		_randomBackgroundRepeatCount(0),
		_ambientEffectTimerAccumulator(0),
		_previousContinuousAmbientCue(0),
		_previousRandomAmbientCue(0) {
	_sceneLayers.configure(kScene4040LayerSpecs);
	_cyclicBackgroundTrack = _realtimeAnimationTracks.addLoop(kScene4040CyclicBackgroundLayer, kScene4040FrameMillis,
		kScene4040BackgroundDescriptorCount);
}

void Scene4040::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applyScenePaletteOverride();
	resetBackgroundLayers();
	if (_vm->gameState().mainFlowStateId == kScene4040ReturnFromUpperExitState) {
		_activeActorWorldX = kScene4040UpperExitReturnStartX;
		_activeActorWorldY = kScene4040UpperExitReturnStartY;
		_activeActorFacing = kScene4040UpperExitReturnFacing;
	} else {
		_activeActorWorldX = 0x192;
		_activeActorWorldY = 0x0171;
		_activeActorFacing = 2;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene4040::drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) {
	(void)activeWorldX;
	(void)actorDrawOrderMode;
	if (!_actionOverlayPlayer.replacesActor())
		drawForegroundBlocks(activeWorldY);
}

void Scene4040::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene4040ReturnFromUpperExitState) {
		runReturnFromUpperExitEntry();
		return;
	}

	if (state.mainFlowStateId != kScene4040FirstState)
		return;

	applyActorPaletteSpan(kScene4040DarkActorPaletteChunk);
	if (!runStairDeltaClip(kScene4040StairEntryClipChunk,
			kScene4040EntryFootstepFrames, ARRAYSIZE(kScene4040EntryFootstepFrames),
			53, kScene4040LightActorPaletteChunk, true) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		drawPlayableComposite();
		fadePaletteFromBlack();
	}
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	if (!state.scene4040EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene4040EntryLineSeen = true;
	}
}

void Scene4040::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

void Scene4040::prepareCustomGameplayLoop() {
	applyScenePaletteOverride();
}

void Scene4040::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientSounds(delta);
	advanceRandomBackground(delta);
}

bool Scene4040::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a salida superior (go to upper exit): toward scene 4050.
		_vm->gameState().mainFlowStateId = kScene4050FirstState;
		return true;
	case 302: // Mirar balcon (look at balcony).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Ir a balcon/escalera (go through balcony): return to dungeon.
		runStairReturnToDungeon();
		return true;
	case 304: // Mirar escalera inferior (look at lower stairs): leads to dungeons.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Mirar escudo (look at shield).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 306: // Mirar escalera cortada (look at cut stairs).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Mirar lanzas (look at spears).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Coger/usar cuerda (take/use rope): too damaged.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Mirar cuerda (look at rope).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Coger candil/cordon (take oil lamp/cord).
		takeCandil();
		return true;
	case 311: // Mirar candil/cordon (look at oil lamp/cord).
		beginSecondarySpeechLine(9, 0);
		return true;
	default:
		return false;
	}
}

bool Scene4040::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x11a, HollywoodEngine::kSceneBufferWidth - 1);
	targetY = CLIP<int>(targetY, 0, HollywoodEngine::kSceneBufferHeight - 1);

	if (targetY < 0x1df)
		++targetY;
	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;
	return true;
}

bool Scene4040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	applyScenePaletteOverride();

	if (_vm->gameState().scene4040CandilTaken) {
		removeColorMapItem(kScene4040CandilSceneItem);
		if (_sceneChunkTable.isValidChunk(15))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[15], _baseFramebuffer);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene4040::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene4040AmbientCheckMillis;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

void Scene4040::applyScenePaletteOverride() {
	setPaletteEntry6Bit(kScene4040PaletteOverrideColor, 0, 0x11, 0);
}

void Scene4040::resetBackgroundLayers() {
	_sceneLayers.reset();
	_realtimeAnimationTracks.reset(_cyclicBackgroundTrack);
	_randomBackgroundChannel.reset(0, kScene4040FrameMillis);
	_randomBackgroundState = 0;
	_randomBackgroundRepeatCount = 0;
}

void Scene4040::drawBackgroundLayers() {
	drawLayerStack(kSceneAnimationBehindActors);
}

void Scene4040::advanceRandomBackground(uint32 delta) {
	const uint randomFrameCount = _randomBackgroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < randomFrameCount; ++frame)
		advanceRandomBackgroundTick();
}

void Scene4040::advanceRandomBackgroundTick() {
	byte frameIndex = _randomBackgroundChannel.frameIndex;

	switch (_randomBackgroundState) {
	case 0:
		if (_random.getRandomNumber(14) == 0) {
			_randomBackgroundState = _random.getRandomBit() ? 2 : 1;
			_randomBackgroundRepeatCount = (byte)_random.getRandomNumber(9);
		}
		break;
	case 1:
		if (frameIndex == 0x19) {
			frameIndex = 0;
		} else {
			if (_randomBackgroundRepeatCount != 0) {
				if (frameIndex < 0x0e) {
					++frameIndex;
				} else if (_random.getRandomBit()) {
					_randomBackgroundState = 3;
					_randomBackgroundRepeatCount = 0;
					++frameIndex;
					break;
				} else {
					_randomBackgroundState = 2;
					frameIndex = 0x0d;
				}
				--_randomBackgroundRepeatCount;
			}
			if (_randomBackgroundRepeatCount == 0) {
				if (isRandomBackgroundHoldFrame(frameIndex))
					++frameIndex;
				else
					_randomBackgroundState = 0;
			}
		}
		break;
	case 2:
		if (_randomBackgroundRepeatCount != 0) {
			if (frameIndex == 0) {
				frameIndex = 1;
				_randomBackgroundState = 1;
			} else {
				--frameIndex;
			}
			--_randomBackgroundRepeatCount;
		}
		if (_randomBackgroundRepeatCount == 0) {
			if (isRandomBackgroundHoldFrame(frameIndex))
				--frameIndex;
			else
				_randomBackgroundState = 0;
		}
		break;
	case 3:
		if (frameIndex < 0x18)
			++frameIndex;
		else
			_randomBackgroundState = 4;
		break;
	case 4:
		if (_random.getRandomNumber(19) == 0) {
			frameIndex = 0x19;
			_randomBackgroundState = 1;
			_randomBackgroundRepeatCount = (byte)_random.getRandomNumber(14);
		}
		break;
	default:
		_randomBackgroundState = 0;
		break;
	}

	_randomBackgroundChannel.frameIndex = frameIndex;
	_sceneLayers.setLayerFrame(kScene4040RandomBackgroundLayer, frameIndex);
}

bool Scene4040::isRandomBackgroundHoldFrame(byte frameIndex) const {
	return frameIndex == 1 || frameIndex == 3 || frameIndex == 6 ||
		frameIndex == 8 || frameIndex == 0x0b || frameIndex == 0x0d;
}

void Scene4040::runReturnFromUpperExitEntry() {
	setActiveActorPose(kScene4040UpperExitReturnStartX, kScene4040UpperExitReturnStartY,
		kScene4040UpperExitReturnFacing);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;
	runEntryPath(kScene4040UpperExitReturnStartX, kScene4040UpperExitReturnStartY,
		kScene4040UpperExitReturnFacing, kScene4040UpperExitReturnTargetX,
		kScene4040UpperExitReturnTargetY);
}

void Scene4040::runStairReturnToDungeon() {
	const bool completed = runStairDeltaClip(kScene4040StairExitClipChunk,
		kScene4040ExitFootstepFrames, ARRAYSIZE(kScene4040ExitFootstepFrames),
		8, kScene4040DarkActorPaletteChunk, false);
	if (completed)
		_vm->gameState().mainFlowStateId = kScene4030ReturnState;
}

bool Scene4040::runStairDeltaClip(uint chunkIndex, const byte *footstepFrames,
		uint footstepFrameCount, int paletteFrame, uint paletteChunk, bool fadeIn) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex)) {
		warning("%s stair clip chunk %u is unavailable", sceneDebugName(), chunkIndex);
		return false;
	}

	uint frame = 0;
	for (; frame < kScene4040StairClipFrameCount && !Engine::shouldQuit() &&
			!_vm->isSceneRestartRequested(); ++frame) {
		if ((int)frame == paletteFrame)
			applyActorPaletteSpan(paletteChunk);
		if (containsFrame(footstepFrames, footstepFrameCount, (byte)frame))
			playResidentSoundEffect(1);
		if (!drawStairDeltaClipFrame(chunkIndex, (byte)frame)) {
			warning("%s failed to decode stair clip chunk %u frame %u",
				sceneDebugName(), chunkIndex, frame);
			return false;
		}
		if (frame == 0) {
			if (fadeIn && fadePaletteFromBlack())
				return false;
			if (!fadeIn)
				presentFrame();
			updateAmbientSounds(0);
		} else {
			presentFrame();
		}
		if (frame + 1 == kScene4040StairClipFrameCount)
			break;
		if (frame != 0 && waitDeltaClipFrameMillis(kScene4040FrameMillis))
			break;
		advanceTransitionSystems(kScene4040FrameMillis);
	}

	if (_skipRequested && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		for (++frame; frame < kScene4040StairClipFrameCount; ++frame) {
			if (!drawStairDeltaClipFrame(chunkIndex, (byte)frame))
				break;
			presentFrame();
		}
		consumeStepAdvanceRequest();
		_skipRequested = false;
	}

	return !Engine::shouldQuit() && !_vm->isSceneRestartRequested();
}

bool Scene4040::drawStairDeltaClipFrame(uint chunkIndex, byte frameIndex) {
	copyBaseFramebufferToSceneFramebuffer();
	drawBackgroundLayers();
	for (uint frame = 0; frame <= frameIndex; ++frame) {
		if (!drawResourceDeltaClipFrame(_resourceArena, _resourceChunkOffsets[chunkIndex],
				_sceneChunkTable.sizes[chunkIndex], kScene4040StairClipFrameCount,
				(byte)frame, framebufferPixels(_sceneFramebuffer), framebufferByteCount()))
			return false;
	}
	return true;
}

bool Scene4040::applyActorPaletteSpan(uint chunkIndex) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return false;

	const uint paletteOffset = kScene4040ActorPaletteFirstColor * 3;
	const uint paletteBytes = (kScene4040ActorPaletteLastColor -
		kScene4040ActorPaletteFirstColor + 1) * 3;
	if (_paletteCurrent.size() < paletteOffset + paletteBytes ||
			_paletteResource.size() < paletteOffset + paletteBytes ||
			_sceneChunkTable.sizes[chunkIndex] < paletteOffset + paletteBytes ||
			_resourceChunkOffsets[chunkIndex] > _resourceArena.size() ||
			paletteOffset + paletteBytes >
				_resourceArena.size() - _resourceChunkOffsets[chunkIndex]) {
		warning("%s actor palette chunk %u is truncated", sceneDebugName(), chunkIndex);
		return false;
	}

	const byte *source = _resourceArena.data() + _resourceChunkOffsets[chunkIndex] + paletteOffset;
	memcpy(_paletteCurrent.data() + paletteOffset, source, paletteBytes);
	memcpy(_paletteResource.data() + paletteOffset, source, paletteBytes);
	return true;
}

void Scene4040::advanceTransitionSystems(uint32 delta) {
	updateAmbientSounds(delta);
	updateAmbientAudioAndMusicCues(delta);
	_realtimeAnimationTracks.advance(_cyclicBackgroundTrack, delta, _random);
	advanceRandomBackground(delta);
}

void Scene4040::updateAmbientSounds(uint32 delta) {
	if (!_ambientSoundBank0.isPlaying()) {
		byte cueId = 0;
		do {
			cueId = (byte)(0x0b + _random.getRandomNumber(2));
		} while (cueId == _previousContinuousAmbientCue);
		_previousContinuousAmbientCue = cueId;
		_ambientSoundBank0.playSample(cueId, 10);
	}

	_ambientEffectTimerAccumulator += delta;
	while (_ambientEffectTimerAccumulator >= kScene4040AmbientCheckMillis) {
		_ambientEffectTimerAccumulator -= kScene4040AmbientCheckMillis;
		SoundBank0Player &player = _additionalAmbientSoundBank0Slots[0];
		if (player.isPlaying() || _random.getRandomNumber(24) != 0)
			continue;

		if (_random.getRandomNumber(9) == 0) {
			player.playSample(0x0e, 50);
			continue;
		}

		byte cueId = 0;
		do {
			cueId = (byte)(0x0f + _random.getRandomNumber(4));
		} while (cueId == _previousRandomAmbientCue);
		_previousRandomAmbientCue = cueId;
		player.playSample(cueId, 5);
	}
}

bool Scene4040::containsFrame(const byte *frames, uint frameCount, byte frame) const {
	for (uint i = 0; i < frameCount; ++i) {
		if (frames[i] == frame)
			return true;
	}
	return false;
}

void Scene4040::takeCandil() {
	GameplayState &state = _vm->gameState();
	if (state.scene4040CandilTaken) {
		beginSecondarySpeechLine(8, 0);
		return;
	}

	beginSecondarySpeechLine(8, 0);
	state.scene4040CandilTaken = true;
	const bool spanishDemo = _vm->isDemo() && _vm->getLanguage() == Common::ES_ESP;
	ActionOverlaySpec pickup(kScene4040CandilOverlayChunk,
		kScene4040CandilOverlayDescriptorCount, kScene4040FrameMillis);
	if (spanishDemo)
		pickup.holdFirstFrame().patchAt(4, 1);
	else
		pickup.patchAt(3, 1);
	runActorReplacement(pickup.noFinalFrameDelay());
	addInventoryItem(spanishDemo ? kScene4040DemoCordItem : kScene4040CandilItem);
	_soundBank0.playSample(1, 100);
}

void Scene4040::drawForegroundBlocks(int activeWorldY) {
	if (activeWorldY < 0x0189 && _sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
}

void Scene4040::removeColorMapItem(byte itemId) {
	replaceColorMapItem(itemId, 0);
}

void Scene4040::replaceColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

} // End of namespace Hollywood
