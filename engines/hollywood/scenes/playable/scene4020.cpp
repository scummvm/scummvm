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
#include "hollywood/scenes/playable/scene4020.h"

namespace Hollywood {

const uint16 kScene4020ReturnState = 0x0fb5;
const uint16 kScene4010ReturnState = 0x0fac;
const uint16 kScene4030FirstState = 0x0fbe;
const uint16 kScene4020ViewportXOffset = 0x0080;
const uint kScene4020ActorBankTableEntry = 0x0000;
const uint kScene4020ActorPaletteTableEntry = 0x00cc;
const uint kScene4020Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4020SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4020FrameMillis = 75;
const uint32 kScene4020AmbientCheckMillis = 250;
const uint kScene4020IdleChunk = 5;
const uint kScene4020IdleDescriptorCount = 0x1a;
const uint kScene4020GrateTransitionChunk = 6;
const uint kScene4020GrateTransitionDescriptorCount = 0x0d;
const uint kScene4020SkullcrackerChunk = 7;
const uint kScene4020SkullcrackerDescriptorCount = 0x10;
const byte kScene4020SkullcrackerItem = 0x20;

enum Scene4020LayerId {
	kScene4020IdleLayer
};

const byte kScene4020ReturnFromD03FrameMap[] = {
	12, 11, 10, 9, 8, 7, 6, 5, 4, 3,
	2, 1, 0
};

const byte kScene4020SkullcrackerFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 8, 9, 10,
	11, 12, 13, 14, 15, 8, 9, 10, 11, 12,
	13, 14, 15, 8, 9, 10, 11, 12, 13, 14,
	15, 8, 9, 10, 11, 12, 13, 14, 15, 6,
	4, 2, 0
};

const SceneLayerSpec kScene4020LayerSpecs[] = {
	{kSceneAnimationBehindActors, kScene4020IdleChunk,
		kScene4020IdleDescriptorCount, nullptr, 0, true, 0}
};

PlayableSceneConfig scene4020Config() {
	PlayableSceneConfig config(4020,
		SceneResourceLayout(5, 5, 7),
		SceneViewport(kScene4020ViewportXOffset, kScene4020ViewportXOffset, kScene4020ViewportXOffset),
		SceneActorPose(0x50, 0x173, 2));
	config.setActorResources(kScene4020ActorBankTableEntry, kScene4020ActorPaletteTableEntry);
	config.setTextResources(kScene4020Resource003RowsOffsetIndex, kScene4020SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 2;
	config.useActorDepthTest = true;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene4020::Scene4020(HollywoodEngine *vm) :
		PlayableScene(vm, scene4020Config()),
		_idleTrack(RealtimeAnimationTracks::kInvalidTrack) {
	_sceneLayers.configure(kScene4020LayerSpecs);
	_idleTrack = _realtimeAnimationTracks.addLoop(kScene4020IdleLayer,
		kScene4020FrameMillis, kScene4020IdleDescriptorCount);
}

void Scene4020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetIdleLayer();

	if (_vm->gameState().mainFlowStateId == kScene4020ReturnState)
		setActiveActorPose(0x265, 0x117, 4);
	else
		setActiveActorPose(0x50, 0x173, 2);
}
void Scene4020::runExitSideEffectsAfterLoop() {
	if (!didLeaveSceneAfterLoop())
		return;

	fadePaletteToBlack();
}

void Scene4020::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene4020ReturnState)
		runEntryFromScene4030();
	else
		runEntryFromScene4010();
}

bool Scene4020::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar zona del foso/entrada (look at moat/entry area).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Mirar camino/agua del foso (look at moat path/water).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Usar camino/mecanismo antes de abrirlo; after unlock, shared row 8 response.
		if (_vm->gameState().scene4020GateUnlocked)
			beginStaticSecondarySpeechLine(8, 0);
		else
			beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Usar/abrir blocked object: state-gated shared response.
		if (_vm->gameState().scene4020GateUnlocked)
			beginStaticSecondarySpeechLine(0x0b, 0);
		else
			beginStaticSecondarySpeechLine(0x13, (byte)_random.getRandomNumber(1));
		return true;
	case 305: // Mirar puerta/mecanismo (look at door/mechanism).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Ir/usar paso hacia mazmorra (go/use passage to dungeon).
		runExitToScene4030();
		return true;
	case 307: // Salir por el foso hacia exterior del castillo (exit moat to castle exterior).
		_vm->gameState().mainFlowStateId = kScene4010ReturnState;
		return true;
	case 308: // Usar el revienta-craneos para mantener abierta la reja (use the skullcracker to hold the grate open).
		useSkullcrackerOnGrate();
		return true;
	default:
		return false;
	}
}

bool Scene4020::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 3 && nextRegion == 4) {
		copyStepDeltas(0x0c, 0x17);
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 3 && nextRegion == 1) {
		copyStepDeltas(0x30, 0x3b);
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene4020::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
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

AmbientAudioProfile Scene4020::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene4020AmbientCheckMillis;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

void Scene4020::resetIdleLayer() {
	_realtimeAnimationTracks.reset(_idleTrack);
}

void Scene4020::setActiveActorPose(int x, int y, byte facing) {
	_activeActorWorldX = x;
	_activeActorWorldY = y;
	_activeActorFacing = facing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene4020::runEntryFromScene4010() {
	setActiveActorPose(0x50, 0x173, 2);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;

	walkActiveActorTo(0xce, 0x195, 0xff, 0, false);
	if (!_vm->gameState().scene4020FallReactionLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene4020FallReactionLineSeen = true;
	}
}

void Scene4020::runEntryFromScene4030() {
	setActiveActorPose(0x265, 0x117, 4);

	const bool previousHideActiveActor = _actionOverlayPlayer.beginActorReplacement(kScene4020GrateTransitionChunk,
		kScene4020GrateTransitionDescriptorCount, kScene4020ReturnFromD03FrameMap,
		ARRAYSIZE(kScene4020ReturnFromD03FrameMap));
	_actionOverlayPlayer.setFrame(0);
	drawPlayableComposite();
	BlockingSequence sequence(*this);
	sequence.fadeFromBlack()
		.layerFrames(_actionOverlayPlayer,
			AnimationFrameRange(1, ARRAYSIZE(kScene4020ReturnFromD03FrameMap) - 1,
				kScene4020FrameMillis));
	_actionOverlayPlayer.finish(previousHideActiveActor);
	if (!sequence.completed())
		return;

	sequence.actorPose(SceneActorPose(0x265, 0x117, 5))
		.actorPath(SceneActorPose(0x265, 0x117, 4));
}

void Scene4020::runExitToScene4030() {
	if (!_vm->gameState().scene4020GateUnlocked) {
		beginSecondarySpeechLine(5, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(kScene4020GrateTransitionChunk,
		kScene4020GrateTransitionDescriptorCount, kScene4020FrameMillis)
		.holdFirstFrame()
		.noRedrawAtEnd()
		.startAt(1));
	_vm->gameState().mainFlowStateId = kScene4030FirstState;
}

void Scene4020::useSkullcrackerOnGrate() {
	if (_vm->gameState().scene4020GateUnlocked) {
		beginSecondarySpeechLine(6, 0);
		return;
	}
	if (!hasInventoryItem(kScene4020SkullcrackerItem)) {
		beginSecondarySpeechLine(5, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(kScene4020SkullcrackerChunk, kScene4020SkullcrackerDescriptorCount,
		kScene4020SkullcrackerFrameMap, ARRAYSIZE(kScene4020SkullcrackerFrameMap), kScene4020FrameMillis)
		.loopingSoundAt(9, 0x39, 50)
		.stopSoundAt(0x31));
	_soundBank0.stop();
	removeInventoryItem(kScene4020SkullcrackerItem);
	_soundBank0.playSample(1, 100);
	_vm->gameState().scene4020GateUnlocked = true;
	beginSecondarySpeechLine(6, 0);
}

void Scene4020::copyStepDeltas(uint firstOffset, uint lastOffset) {
	for (uint offset = firstOffset; offset <= lastOffset &&
			offset < _actorPathStepDeltas.size() &&
			offset < ARRAYSIZE(kActorPathStepDeltaTableSet87); ++offset) {
		_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
	}
}

} // End of namespace Hollywood
