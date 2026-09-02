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

#include "hollywood/scenes/playable/scene5100.h"

#include "engines/engine.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene5100FirstState = 0x13ec;
const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5110FirstState = 0x13f6;
const uint16 kScene5120FirstState = 0x1400;
const uint16 kScene5100ReturnState = 0x13ed;
const uint16 kScene5100ViewportXOffset = 0x00a0;
const uint kScene5100ActorBankTableEntry = 0x0000;
const uint kScene5100ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5100SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5100CartTimingNumeratorMillis = 75;
const uint32 kScene5100ElevatorFrameMillis = 125;
const uint32 kScene5100ElevatorOpenLeadInMillis = 500;
const uint32 kScene5100ElevatorWaitMillis = 1000;
const uint kScene5100FirstEntryDescriptorCount = 0x2f;
const uint kScene5100ReturnEntryDescriptorCount = 0x0e;
const uint kScene5100ElevatorDoorDescriptorCount = 0x0c;

const byte kScene5100CartSpeedBuckets[] = {
	1, 1, 1, 1, 1,
	2, 2, 2, 2, 2,
	3, 3, 3, 3, 3,
	4, 4, 4, 4, 4,
	5, 5, 5, 5, 5,
	6, 6, 6, 6, 6,
	7, 7, 7, 7, 7,
	8, 8, 8, 8, 8,
	9, 9, 9, 9, 9, 9
};

const byte kScene5100AmbientSoundVolumes[] = {
	10, 10, 10, 2, 10, 10, 10, 100
};

enum Scene5100LayerId {
	kScene5100MineCartLayer,
	kScene5100ElevatorTravelLayer,
	kScene5100ElevatorDoorLayer
};

const SceneLayerSpec kScene5100LayerSpecs[] = {
	{kSceneAnimationInFrontOfActors, 8, kScene5100FirstEntryDescriptorCount,
		nullptr, 0, false, 0},
	{kSceneAnimationInFrontOfActors, 9, kScene5100ReturnEntryDescriptorCount,
		nullptr, 0, false, 0},
	{kSceneAnimationInFrontOfActors, 7, kScene5100ElevatorDoorDescriptorCount,
		nullptr, 0, false, 0}
};

static PlayableSceneConfig scene5100Config() {
	PlayableSceneConfig config(5100,
		SceneResourceLayout(5, 5, 9),
		SceneViewport(kScene5100ViewportXOffset, kScene5100ViewportXOffset, kScene5100ViewportXOffset),
		SceneActorPose(0x1c0, 0x199, 2));
	config.setActorResources(kScene5100ActorBankTableEntry, kScene5100ActorPaletteTableEntry);
	config.setTextResources(0, kScene5100SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = false;
	return config;
}

Scene5100::Scene5100(HollywoodEngine *vm) :
		PlayableScene(vm, scene5100Config()),
		_elevatorDoorChannel(),
		_mineCartRumbleActive(false),
		_elevatorDoorClosing(false) {
	_sceneLayers.configure(kScene5100LayerSpecs);
}

void Scene5100::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	_sceneLayers.reset();
	_elevatorDoorChannel.reset(0, kScene5100ElevatorFrameMillis);
	_mineCartRumbleActive = false;
	_elevatorDoorClosing = false;
	applySceneStateToHotspotsAndPatches(0xff);

	if (_vm->gameState().mainFlowStateId == kScene5100FirstState) {
		_activeActorWorldX = 0x2e8;
		_activeActorWorldY = 0x1ae;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x1c0;
		_activeActorWorldY = 0x199;
		_activeActorFacing = 2;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene5100::drawCustomForegroundComposite(int activeWorldX, int activeWorldY) {
	(void)activeWorldX;
	(void)activeWorldY;
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

bool Scene5100::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene5100::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene5100ReturnState) {
		setActiveActorPose(0x163, 0x173, 2);
		if (!runReturnEntryClip() || !runElevatorDoorOpen())
			return;
		walkActiveActorTo(0x1c0, 0x199, 0xff, 0, false);
		return;
	}
	if (state.mainFlowStateId != kScene5100FirstState) {
		drawPlayableComposite();
		presentFrame();
		return;
	}

	setActiveActorPose(0x3b0, 0x1ae, 4);
	if (!runFirstEntryClip())
		return;
	walkActiveActorTo(0x2e8, 0x1ae, 0xff, 0, false);

	if (!state.scene5100EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5100EntryLineSeen = true;
	}
}

void Scene5100::advanceCustomGameplayLoop(uint32 delta) {
	ensureAmbientSoundCuePlaying(1, 0x0c, 10);
	if (_mineCartRumbleActive && !_soundBank0.isPlaying())
		_soundBank0.playSample(0x18, 100);
	advanceElevatorDoor(delta);
}

bool Scene5100::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Usar vagoneta (use mine cart): return to the mine switch panel.
		runReturnToMineSwitches();
		return true;
	case 302: // Mirar ascensor (look at elevator).
		beginSecondarySpeechLine(1, 1);
		return true;
	case 303: // Usar boton superior (use upper button): exits to scene 5120.
		runElevatorButtonTransition(kScene5120FirstState);
		return true;
	case 304: // Usar boton central (use middle button): exits to the spa room, scene 5110.
		runElevatorButtonTransition(kScene5110FirstState);
		return true;
	case 305: // Usar boton inferior (use lower button): returns the elevator and unlocks button default verbs.
		runElevatorButtonUnlock();
		return true;
	default:
		return false;
	}
}

bool Scene5100::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x156, 0x3b0);

	if (targetY < 0x1df)
		++targetY;
	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene5100::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	if (_vm->gameState().scene5100ButtonsUnlocked && _metadata.size() > kSceneItemDefaultStrip + 7) {
		_metadata[kSceneItemDefaultStrip + 5] = 5;
		_metadata[kSceneItemDefaultStrip + 6] = 5;
		_metadata[kSceneItemDefaultStrip + 7] = 5;
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	updateElevatorButtonActionTargets(false);
	return true;
}

void Scene5100::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

AmbientAudioProfile Scene5100::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 10, 25, 0x0b, 5, 100, 50);
}

byte Scene5100::ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const {
	if (cueId >= 0x0d && cueId <= 0x14)
		return kScene5100AmbientSoundVolumes[cueId - 0x0d];
	return defaultVolumePercent;
}

void Scene5100::handleLeftClick(const GameplayLoopCursorState &state) {
	updateElevatorButtonActionTargets(state.currentStrip == 5);
	PlayableScene::handleLeftClick(state);
}

bool Scene5100::runFirstEntryClip() {
	if (!_sceneChunkTable.isValidChunk(8)) {
		drawPlayableComposite();
		return !fadePaletteFromBlack();
	}

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	_sceneLayers.showLayerAtFrame(kScene5100MineCartLayer, 0);
	ensureAmbientSoundCuePlaying(1, 0x0c, 10);
	_mineCartRumbleActive = true;
	_soundBank0.playSample(0x18, 100);
	drawPlayableComposite();
	if (fadePaletteFromBlack()) {
		_mineCartRumbleActive = false;
		_sceneLayers.setLayerVisible(kScene5100MineCartLayer, false);
		_hideActiveActor = previousHideActiveActor;
		return false;
	}

	for (uint frame = 1; frame < kScene5100FirstEntryDescriptorCount; ++frame) {
		const uint32 frameMillis = kScene5100CartTimingNumeratorMillis /
			(13 - kScene5100CartSpeedBuckets[frame - 1]);
		if (waitSceneMillis(frameMillis, false)) {
			_mineCartRumbleActive = false;
			_sceneLayers.setLayerVisible(kScene5100MineCartLayer, false);
			_hideActiveActor = previousHideActiveActor;
			return false;
		}

		_sceneLayers.setLayerFrame(kScene5100MineCartLayer, (byte)frame);
		if (frame == 0x1a) {
			_mineCartRumbleActive = false;
			_soundBank0.playSample(0x16, 100);
		}
		drawPlayableComposite();
		presentFrame();
	}

	_mineCartRumbleActive = false;
	_sceneLayers.setLayerVisible(kScene5100MineCartLayer, false);
	_hideActiveActor = previousHideActiveActor;
	return true;
}

bool Scene5100::runReturnEntryClip() {
	if (!_sceneChunkTable.isValidChunk(9)) {
		drawPlayableComposite();
		return !fadePaletteFromBlack();
	}

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	_sceneLayers.setLayerVisible(kScene5100ElevatorDoorLayer, false);
	_sceneLayers.showLayerAtFrame(kScene5100ElevatorTravelLayer,
		kScene5100ReturnEntryDescriptorCount - 1);
	drawPlayableComposite();
	if (fadePaletteFromBlack()) {
		_sceneLayers.setLayerVisible(kScene5100ElevatorTravelLayer, false);
		_hideActiveActor = previousHideActiveActor;
		return false;
	}

	if (!playAndPresentAnimationFrames(kScene5100ElevatorTravelLayer,
			AnimationFrameRange(kScene5100ReturnEntryDescriptorCount - 1, 0,
				kScene5100ElevatorFrameMillis).unskippable())) {
		_sceneLayers.setLayerVisible(kScene5100ElevatorTravelLayer, false);
		_hideActiveActor = previousHideActiveActor;
		return false;
	}

	_sceneLayers.setLayerVisible(kScene5100ElevatorTravelLayer, false);
	_hideActiveActor = previousHideActiveActor;
	_sceneLayers.showLayerAtFrame(kScene5100ElevatorDoorLayer,
		kScene5100ElevatorDoorDescriptorCount - 1);
	drawPlayableComposite();
	presentFrame();
	return true;
}

bool Scene5100::runElevatorDoorClose() {
	_soundBank0.playSample(0x1d, 100, true);
	if (!_sceneChunkTable.isValidChunk(7))
		return true;
	ResourceSpriteLayer &doorLayer = _sceneLayers.layer(kScene5100ElevatorDoorLayer);

	if (doorLayer.visible && !_elevatorDoorClosing &&
			doorLayer.frameIndex == kScene5100ElevatorDoorDescriptorCount - 1)
		return true;

	byte firstFrame = 0;
	if (doorLayer.visible)
		firstFrame = MIN<byte>(doorLayer.frameIndex,
			kScene5100ElevatorDoorDescriptorCount - 1);
	else
		_sceneLayers.showLayerAtFrame(kScene5100ElevatorDoorLayer, 0);

	if (_elevatorDoorClosing) {
		const uint32 elapsed = MIN<uint32>(_elevatorDoorChannel.timerAccumulator,
			kScene5100ElevatorFrameMillis);
		_elevatorDoorClosing = false;
		if (waitSceneMillis(kScene5100ElevatorFrameMillis - elapsed, false))
			return false;
		if (firstFrame == kScene5100ElevatorDoorDescriptorCount - 1)
			return true;
		++firstFrame;
		doorLayer.setFrame(firstFrame);
		drawPlayableComposite();
		presentFrame();
	}

	_elevatorDoorClosing = false;
	const bool completed = playAndPresentAnimationFrames(kScene5100ElevatorDoorLayer,
		AnimationFrameRange(firstFrame, kScene5100ElevatorDoorDescriptorCount - 1,
			kScene5100ElevatorFrameMillis).unskippable());
	_elevatorDoorChannel.reset(kScene5100ElevatorDoorDescriptorCount - 1,
		kScene5100ElevatorFrameMillis);
	return completed;
}

bool Scene5100::runElevatorDoorOpen() {
	if (!_sceneChunkTable.isValidChunk(7))
		return true;

	_elevatorDoorClosing = false;
	_sceneLayers.showLayerAtFrame(kScene5100ElevatorDoorLayer,
		kScene5100ElevatorDoorDescriptorCount - 1);
	_soundBank0.playSample(0x1d, 100, true);
	if (waitSceneMillis(kScene5100ElevatorOpenLeadInMillis, false))
		return false;

	const bool completed = playAndPresentAnimationFrames(kScene5100ElevatorDoorLayer,
		AnimationFrameRange(kScene5100ElevatorDoorDescriptorCount - 1, 0,
			kScene5100ElevatorFrameMillis).unskippable());
	_soundBank0.stop();
	_sceneLayers.setLayerVisible(kScene5100ElevatorDoorLayer, false);
	return completed;
}

bool Scene5100::runElevatorTravel() {
	_soundBank0.playSample(0x1e, 100, true);
	if (_vm->gameState().mainFlowStateId != kScene5100FirstState &&
			waitSceneMillis(kScene5100ElevatorWaitMillis, false))
		return false;
	if (!_sceneChunkTable.isValidChunk(9))
		return true;

	_elevatorDoorClosing = false;
	_sceneLayers.setLayerVisible(kScene5100ElevatorDoorLayer, false);
	_sceneLayers.showLayerAtFrame(kScene5100ElevatorTravelLayer, 0);
	_hideActiveActor = true;
	return playAndPresentAnimationFrames(kScene5100ElevatorTravelLayer,
		AnimationFrameRange(0, kScene5100ReturnEntryDescriptorCount - 1,
			kScene5100ElevatorFrameMillis).unskippable().noFinalFrameDelay());
}

void Scene5100::runElevatorButtonTransition(uint16 nextState) {
	drawPlayableComposite();
	presentFrame();
	if (!runElevatorDoorClose() || !runElevatorTravel())
		return;
	_vm->gameState().scene5100ButtonsUnlocked = true;
	_vm->gameState().mainFlowStateId = nextState;
}

void Scene5100::runElevatorButtonUnlock() {
	drawPlayableComposite();
	presentFrame();
	if (!runElevatorDoorClose() || waitSceneMillis(kScene5100ElevatorWaitMillis, false) ||
			!runElevatorDoorOpen())
		return;
	_soundBank0.stop();
	walkActiveActorTo(0x1c0, 0x199, 0xff, 0, false);
	_vm->gameState().scene5100ButtonsUnlocked = true;
	applySceneStateToHotspotsAndPatches(1);
}

void Scene5100::runReturnToMineSwitches() {
	walkActiveActorTo(0x3b0, 0x1ae, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5100::advanceElevatorDoor(uint32 delta) {
	ResourceSpriteLayer &doorLayer = _sceneLayers.layer(kScene5100ElevatorDoorLayer);
	if (!_elevatorDoorClosing) {
		if (!doorLayer.visible && _activeActorDrawOrderMode == 2 && _activeActorFacing == 5) {
			_sceneLayers.showLayerAtFrame(kScene5100ElevatorDoorLayer, 0);
			_elevatorDoorChannel.reset(0, kScene5100ElevatorFrameMillis);
			_elevatorDoorClosing = true;
		}
		return;
	}

	const uint frameCount = _elevatorDoorChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (doorLayer.frameIndex == kScene5100ElevatorDoorDescriptorCount - 1) {
			_elevatorDoorClosing = false;
			break;
		}
		doorLayer.setFrame(doorLayer.frameIndex + 1);
		_elevatorDoorChannel.frameIndex = doorLayer.frameIndex;
	}
}

void Scene5100::updateElevatorButtonActionTargets(bool useStrip) {
	ScenePoint interactionPoint;
	interactionPoint.x = useStrip ? 0x163 : 0x156;
	interactionPoint.y = useStrip ? 0x173 : 0x191;
	const byte facing = useStrip ? 2 : 1;
	_hotspots.setActionInteraction(5, interactionPoint, facing);
	_hotspots.setActionInteraction(6, interactionPoint, facing);
	_hotspots.setActionInteraction(7, interactionPoint, facing);
}

void Scene5100::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 1)
			_walkablePaletteMask[i] = 0;
	}
}

} // End of namespace Hollywood
