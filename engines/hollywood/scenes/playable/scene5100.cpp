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

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene5100FirstState = 0x13ec;
const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5110FirstState = 0x13f6;
const uint16 kScene5120FirstState = 0x1400;
const uint16 kScene5100ReturnFrom5110State = 0x13ed;
const uint16 kScene5100ViewportXOffset = 0x00a0;
const uint kScene5100ActorBankTableEntry = 0x0000;
const uint kScene5100ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5100SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5100FrameMillis = 75;
const uint kScene5100FirstEntryDescriptorCount = 0x2f;
const uint kScene5100ReturnEntryDescriptorCount = 0x0e;
const uint kScene5100ElevatorDoorDescriptorCount = 0x0c;

const byte kScene5100ReturnEntryFrameMap[] = {
	13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kScene5100ElevatorCloseFrameMap[] = {
	11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kScene5100ElevatorOpenFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene5100TransitionFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

static Common::Array<byte> sequentialFrameMap(uint frameCount) {
	Common::Array<byte> frameMap;
	frameMap.resize(frameCount);
	for (uint i = 0; i < frameMap.size(); ++i)
		frameMap[i] = (byte)i;
	return frameMap;
}

static PlayableSceneConfig scene5100Config() {
	PlayableSceneConfig config(5100,
		SceneResourceLayout(5, 5, 9),
		SceneViewport(kScene5100ViewportXOffset, kScene5100ViewportXOffset, kScene5100ViewportXOffset),
		SceneActorPose(0x1c0, 0x199, 2));
	config.setActorResources(kScene5100ActorBankTableEntry, kScene5100ActorPaletteTableEntry);
	config.setTextResources(0, kScene5100SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = true;
	return config;
}

Scene5100::Scene5100(HollywoodEngine *vm) :
		PlayableScene(vm, scene5100Config()) {
}

void Scene5100::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
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

void Scene5100::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

void Scene5100::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene5100ReturnFrom5110State) {
		setActiveActorPose(0x163, 0x173, 2);
		drawPlayableComposite();
		presentFrame();
		runReturnEntryClip();
		runElevatorOpenClip();
		walkActiveActorTo(0x1c0, 0x199, 0xff, 0, false);
		return;
	}

	setActiveActorPose(0x3b0, 0x1ae, 4);
	drawPlayableComposite();
	presentFrame();

	runFirstEntryClip();
	walkActiveActorTo(0x2e8, 0x1ae, 0xff, 0, false);

	if (!state.scene5100EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5100EntryLineSeen = true;
	}
}

bool Scene5100::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
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

	ScenePoint interactionPoint;
	ScenePoint approachPoint;
	if (_vm->gameState().scene5100ButtonsUnlocked) {
		interactionPoint.x = 0x163;
		interactionPoint.y = 0x173;
		approachPoint.x = 0x163;
		approachPoint.y = 0x173;
	} else {
		interactionPoint.x = 0x156;
		interactionPoint.y = 0x191;
		approachPoint.x = 0x156;
		approachPoint.y = 0x191;
	}
	_hotspots.setActionTarget(5, interactionPoint, approachPoint);
	_hotspots.setActionTarget(6, interactionPoint, approachPoint);
	_hotspots.setActionTarget(7, interactionPoint, approachPoint);
	return true;
}

AmbientAudioProfile Scene5100::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 10, 25, 0x0b, 5, 100, 50);
}

void Scene5100::runFirstEntryClip() {
	const Common::Array<byte> frameMap = sequentialFrameMap(kScene5100FirstEntryDescriptorCount);
	runActorReplacement(ActionOverlaySpec(8, kScene5100FirstEntryDescriptorCount,
		frameMap.data(), frameMap.size(), kScene5100FrameMillis)
		.soundAt(0x1a, 0x16));
}

void Scene5100::runReturnEntryClip() {
	runActorReplacement(ActionOverlaySpec(9, kScene5100ReturnEntryDescriptorCount,
		kScene5100ReturnEntryFrameMap, ARRAYSIZE(kScene5100ReturnEntryFrameMap), kScene5100FrameMillis));
}

void Scene5100::runElevatorOpenClip() {
	_soundBank0.playSample(0x1d, 100);
	runActorReplacement(ActionOverlaySpec(7, kScene5100ElevatorDoorDescriptorCount,
		kScene5100ElevatorCloseFrameMap, ARRAYSIZE(kScene5100ElevatorCloseFrameMap), kScene5100FrameMillis));
}

void Scene5100::runElevatorButtonTransition(uint16 nextState) {
	_soundBank0.playSample(0x1d, 100);
	runActorReplacement(ActionOverlaySpec(7, kScene5100ElevatorDoorDescriptorCount,
		kScene5100ElevatorOpenFrameMap, ARRAYSIZE(kScene5100ElevatorOpenFrameMap), kScene5100FrameMillis));
	runActorReplacement(ActionOverlaySpec(9, kScene5100ReturnEntryDescriptorCount,
		kScene5100TransitionFrameMap, ARRAYSIZE(kScene5100TransitionFrameMap), kScene5100FrameMillis)
		.soundAt(0, 0x1e));
	_vm->gameState().scene5100ButtonsUnlocked = true;
	_vm->gameState().mainFlowStateId = nextState;
}

void Scene5100::runElevatorButtonUnlock() {
	_soundBank0.playSample(0x1d, 100);
	waitSceneMillis(1000);
	runElevatorOpenClip();
	walkActiveActorTo(0x1c0, 0x199, 0xff, 0, false);
	_vm->gameState().scene5100ButtonsUnlocked = true;
	applySceneStateToHotspotsAndPatches(1);
}

void Scene5100::runReturnToMineSwitches() {
	walkActiveActorTo(0x3b0, 0x1ae, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5100::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 1)
			_walkablePaletteMask[i] = 0;
	}
}

} // End of namespace Hollywood
