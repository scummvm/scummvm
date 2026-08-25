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

#include "hollywood/scenes/playable/scene1010.h"

#include "common/debug.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene1010LeftEntryState = 0x03f2;
const uint16 kScene1010RightEntryState = 0x03f3;
const uint16 kScene1010CenterEntryState = 0x03f4;
const uint16 kScene1010ExitState1020 = 0x03fc;
const uint16 kScene1010ExitState1030 = 0x0406;
const uint16 kScene1010ViewportXOffset = 0x00a0;
const uint16 kScene1010ViewportMinXOffset = 0x0060;
const uint16 kScene1010ViewportMaxXOffset = 0x0128;
const uint kScene1010ActorBankTableEntry = 0x0000;
const uint kScene1010ActorPaletteTableEntry = 0x00cc;
const uint kScene1010Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1010SpeechCueDescriptorTableOffset = 0x1135;
const int kScene1010LeftEntryStartX = 0x0dc;
const int kScene1010LeftEntryStartY = 0x159;
const int kScene1010LeftEntryTargetX = 0x226;
const int kScene1010LeftEntryTargetY = 0x1cc;
const byte kScene1010LeftEntryFacing = 1;
const int kScene1010RightEntryStartX = 0x3b0;
const int kScene1010RightEntryStartY = 0x1ad;
const int kScene1010RightEntryTargetX = 0x307;
const int kScene1010RightEntryTargetY = 0x1bb;
const byte kScene1010RightEntryFacing = 4;
const int kScene1010CenterEntryStartX = 0x2ab;
const int kScene1010CenterEntryStartY = 0x1df;
const int kScene1010CenterEntryTargetX = 0x276;
const int kScene1010CenterEntryTargetY = 0x1c0;
const byte kScene1010CenterEntryFacing = 5;
const int kScene1010SpecialFacingTargetX = 0x3b4;
const int kScene1010SpecialFacingTargetY = 0x1ac;
const byte kScene1010FirstAmbientSoundCue = 0x25;
const byte kScene1010AmbientSoundCueCount = 7;
const byte kScene1010FirstAmbientMusicCue = 0x0b;
const byte kScene1010AmbientMusicCueCount = 5;
const byte kScene1010AmbientSoundProbabilityModulus = 25;
const byte kScene1010AmbientMusicProbabilityModulus = 50;
const int kScene1010ForegroundLeftXThreshold = 500;
const int kScene1010ForegroundLeftYThreshold = 0x1cd;
const int kScene1010ForegroundRightXThreshold = 0x28a;
const int kScene1010ForegroundRightYThreshold = 0x1cb;
const uint32 kScene1010SceneActorBlinkFrameMillis = 75;
const uint kScene1010SceneActorBlinkDescriptorCount = 0x0c;

const byte kScene1010SceneActorBlinkFrameMap[] = {
	0, 11, 0, 1, 2, 1, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 10, 10, 10, 10, 9, 8,
	7, 6, 5, 4, 3, 2, 1, 0
};

static PlayableSceneConfig scene1010Config() {
	PlayableSceneConfig config(1010,
		SceneResourceLayout(9, 5, 8),
		SceneViewport(kScene1010ViewportXOffset, kScene1010ViewportMinXOffset, kScene1010ViewportMaxXOffset),
		SceneActorPose(kScene1010CenterEntryTargetX, kScene1010CenterEntryTargetY, kScene1010CenterEntryFacing));
	config.setActorResources(kScene1010ActorBankTableEntry, kScene1010ActorPaletteTableEntry);
	config.setTextResources(kScene1010Resource003RowsOffsetIndex, kScene1010SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.loadActorDepthTables = false;
	return config;
}

Scene1010::Scene1010(HollywoodEngine *vm) :
		PlayableScene(vm, scene1010Config()),
		_sceneActorBlinkTimerAccumulator(0),
		_sceneActorBlinkFrameIndex(0),
		_sceneActorBlinkPatternMode(0) {
}

void Scene1010::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	_sceneActorBlinkTimerAccumulator = 0;
	_sceneActorBlinkFrameIndex = 0;
	_sceneActorBlinkPatternMode = 0;
	if (_vm->gameState().mainFlowStateId == kScene1010LeftEntryState) {
		_activeActorWorldX = kScene1010LeftEntryTargetX;
		_activeActorWorldY = kScene1010LeftEntryTargetY;
		_activeActorFacing = kScene1010LeftEntryFacing;
	} else if (_vm->gameState().mainFlowStateId == kScene1010RightEntryState) {
		_activeActorWorldX = kScene1010RightEntryTargetX;
		_activeActorWorldY = kScene1010RightEntryTargetY;
		_activeActorFacing = kScene1010RightEntryFacing;
	} else {
		_activeActorWorldX = kScene1010CenterEntryTargetX;
		_activeActorWorldY = kScene1010CenterEntryTargetY;
		_activeActorFacing = kScene1010CenterEntryFacing;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene1010::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	drawActionOverlayLayer();

	if (!drawActiveActor)
		return;

	if (activeWorldY < kScene1010ForegroundLeftYThreshold && activeWorldX < kScene1010ForegroundLeftXThreshold)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);

	if (activeWorldX > kScene1010ForegroundRightXThreshold && activeWorldY < kScene1010ForegroundRightYThreshold) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _sceneFramebuffer);
	}

	drawSceneActorBlinkFrame();
}

void Scene1010::runCustomEntrySequence() {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId != kScene1010LeftEntryState &&
		stateId != kScene1010RightEntryState &&
		stateId != kScene1010CenterEntryState) {
		PlayableScene::runCustomEntrySequence();
		return;
	}

	if (stateId == kScene1010LeftEntryState) {
		runEntryPath(kScene1010LeftEntryStartX, kScene1010LeftEntryStartY,
			kScene1010LeftEntryFacing, kScene1010LeftEntryTargetX, kScene1010LeftEntryTargetY);
		return;
	}

	if (stateId == kScene1010RightEntryState) {
		runEntryPath(kScene1010RightEntryStartX, kScene1010RightEntryStartY,
			kScene1010RightEntryFacing, kScene1010RightEntryTargetX, kScene1010RightEntryTargetY);
		return;
	}

	runEntryPath(kScene1010CenterEntryStartX, kScene1010CenterEntryStartY,
		kScene1010CenterEntryFacing, kScene1010CenterEntryTargetX, kScene1010CenterEntryTargetY);
	_activeActorFacing = kScene1010CenterEntryFacing;
	_activeActorCel = 0;
	drawPlayableComposite();
	presentFrame();
	if (!_vm->gameState().scene1010EntryLineSeen) {
		_vm->gameState().scene1010EntryLineSeen = true;
		beginSecondarySpeechLine(0, 0);
	}
}

bool Scene1010::prepareCustomGameplayLoop() {
	_sceneActorBlinkTimerAccumulator = 0;
	return true;
}

bool Scene1010::advanceCustomGameplayLoop(uint32 delta) {
	advanceSceneActorBlinkAnimation(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1010::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a bodega / sala de fiesta (go to cellar/party room): enter scene 1030.
		_vm->gameState().mainFlowStateId = kScene1010ExitState1030;
		return true;
	case 302: // Mirar escaleras (look at stairs): they lead to the mansion.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar caseta de perro (look at doghouse): empty doghouse.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Ir a bodega exterior / rejilla (go to cellar exterior/grate): enter scene 1020.
		_vm->gameState().mainFlowStateId = kScene1010ExitState1020;
		return true;
	case 305: // Mirar edificio (look at distant building): too far from here.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 306: // Usar coche (use car): open Ron destination selector once unlocked.
		if (!_vm->gameState().ronTravelScreenUnlocked) {
			beginSecondarySpeechLine(1, 4);
			return true;
		}
		_vm->gameState().requestTravelScreenSelection(1);
		return true;
	default:
		return false;
	}
}

bool Scene1010::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	if (targetX < 0x1cf)
		targetX = 0x1cf;

	while (targetY < 0x1df) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _fullPaletteRegionMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		++targetY;
	}

	while (targetY > 0) {
		const uint offset = targetY * HollywoodEngine::kSceneBufferWidth + targetX;
		if (isFramebufferOffsetValid(offset) && _fullPaletteRegionMask[savedFramebufferPixelAt(offset)] != 0)
			return true;
		--targetY;
	}

	return true;
}

bool Scene1010::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	if (currentRegion == 3 && nextRegion == 2) {
		for (uint offset = 0x0c; offset <= 0x17 && offset < _actorPathStepDeltas.size(); ++offset)
			_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene1010::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)state;
	if (currentRegion == 3) {
		for (uint offset = 0x30; offset <= 0x3b && offset < _actorPathStepDeltas.size(); ++offset)
			_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
		requestedFacing = 4;
		restoredStepDeltas = true;
	}

	if (targetX == kScene1010SpecialFacingTargetX && targetY == kScene1010SpecialFacingTargetY)
		requestedFacing = 1;

	return restoredStepDeltas || requestedFacing >= 0;
}

AmbientAudioProfile Scene1010::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(kScene1010FirstAmbientSoundCue,
		kScene1010AmbientSoundCueCount, 15, kScene1010AmbientSoundProbabilityModulus,
		kScene1010FirstAmbientMusicCue, kScene1010AmbientMusicCueCount, 100,
		kScene1010AmbientMusicProbabilityModulus);
}

void Scene1010::advanceSceneActorBlinkAnimation(uint32 delta) {
	_sceneActorBlinkTimerAccumulator += delta;
	if (_sceneActorBlinkTimerAccumulator < kScene1010SceneActorBlinkFrameMillis)
		return;
	_sceneActorBlinkTimerAccumulator %= kScene1010SceneActorBlinkFrameMillis;

	if (_sceneActorBlinkPatternMode != 0) {
		if (_sceneActorBlinkPatternMode == 1) {
			if (_sceneActorBlinkFrameIndex > 5) {
				_sceneActorBlinkPatternMode = 0;
				return;
			}
		} else if (_sceneActorBlinkPatternMode == 2) {
			if (_sceneActorBlinkFrameIndex > 0x1e) {
				_sceneActorBlinkPatternMode = 0;
				return;
			}
		}
		++_sceneActorBlinkFrameIndex;
		return;
	}

	if (_sceneActorBlinkFrameIndex != 0) {
		_sceneActorBlinkFrameIndex = 0;
		return;
	}

	if (_random.getRandomNumber(14) == 0) {
		_sceneActorBlinkFrameIndex = 1;
		return;
	}

	if (_random.getRandomNumber(19) != 0)
		return;

	if (_random.getRandomNumber(1) != 0) {
		_sceneActorBlinkFrameIndex = 7;
		_sceneActorBlinkPatternMode = 2;
	} else {
		_sceneActorBlinkFrameIndex = 2;
		_sceneActorBlinkPatternMode = 1;
	}
}

void Scene1010::drawSceneActorBlinkFrame() {
	if (_sceneActorBlinkFrameIndex >= ARRAYSIZE(kScene1010SceneActorBlinkFrameMap))
		return;

	drawStripSpriteFrame(_resourceArena, _resourceChunkOffsets[5], 0,
		kScene1010SceneActorBlinkDescriptorCount,
		kScene1010SceneActorBlinkFrameMap[_sceneActorBlinkFrameIndex], _sceneFramebuffer);
}

} // End of namespace Hollywood
