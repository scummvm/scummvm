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

#include "hollywood/scenes/playable/scene6060.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

// F06 contains only the five fixed scene chunks, so its arena range is empty.
const uint16 kScene6060LastState = 0x17ad;
const uint16 kScene6100EntryState = 0x17d4;
const uint16 kScene6050ReturnState = 0x17a3;
const uint16 kScene6060EntryFromLobbyViewportX = 0x0080;
const uint16 kScene6060EntryFromScene6100ViewportX = 0x0000;
const uint16 kScene6060ViewportMinX = 0x0080;
const uint16 kScene6060ViewportMaxX = 0x0100;
const uint kScene6060ActorBankTableEntry = 0x0000;
const uint kScene6060ActorPaletteTableEntry = 0x00cc;
const uint kScene6060Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6060SpeechCueDescriptorTableOffset = 0x1135;

static PlayableSceneConfig scene6060Config() {
	PlayableSceneConfig config(6060,
		SceneResourceLayout(5, 5, 4),
		SceneViewport(kScene6060EntryFromLobbyViewportX, kScene6060ViewportMinX, kScene6060ViewportMaxX),
		SceneActorPose(0x2d0, 0x192, 5));
	config.setActorResources(kScene6060ActorBankTableEntry, kScene6060ActorPaletteTableEntry);
	config.setTextResources(kScene6060Resource003RowsOffsetIndex, kScene6060SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = true;
	return config;
}

Scene6060::Scene6060(HollywoodEngine *vm) :
		PlayableScene(vm, scene6060Config()) {
}

void Scene6060::initializeCustomPreviewState() {
	initializeDefaultPreviewState();

	if (_vm->gameState().mainFlowStateId == kScene6060LastState) {
		_activeActorWorldX = 0x168;
		_activeActorWorldY = 0x188;
		_activeActorFacing = 2;
		// The upper-floor return begins at the left edge of the lower museum room.
		_viewportXOffset = kScene6060EntryFromScene6100ViewportX;
	} else {
		_activeActorWorldX = 0x2d0;
		_activeActorWorldY = 0x192;
		_activeActorFacing = 5;
		_viewportXOffset = kScene6060EntryFromLobbyViewportX;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene6060::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene6060LastState) {
		runEntryPath(0x050, 0x0f1, 2, 0x168, 0x188);
		return;
	}

	runEntryPath(0x3b0, 0x1c2, 4, 0x2d0, 0x192);
}

bool Scene6060::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar vitrina del Hombre Lobo (look at the Werewolf display).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Mirar vitrina de Dracula (look at the Dracula display).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Mirar vitrina de la Momia (look at the Mummy display).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Ir a escalera (go upstairs): enter the museum's upper floor.
		_vm->gameState().mainFlowStateId = kScene6100EntryState;
		return true;
	case 305: // Mirar escalera (look at the staircase).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar letrero (look at the gallery sign).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Ir a salida del museo (go to museum exit): return to the lobby.
		_vm->gameState().mainFlowStateId = kScene6050ReturnState;
		return true;
	default:
		return false;
	}
}

bool Scene6060::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion != 2 || nextRegion != 1)
		return false;

	for (uint i = 0; i < 0x0c; ++i)
		_actorPathStepDeltas[0x18 + i] = kActorPathStepDeltaTableSetB4[0x24 + i];
	requestedFacing = 2;
	restoredStepDeltas = true;
	return true;
}

bool Scene6060::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)targetRegion;
	(void)state;
	(void)targetY;

	if (targetX != 0x050)
		return false;

	for (uint i = 0; i < 0x0c; ++i)
		_actorPathStepDeltas[0x3c + i] = kActorPathStepDeltaTableSetB4[i];
	requestedFacing = 5;
	restoredStepDeltas = true;
	return true;
}

bool Scene6060::shouldUseActorDepthTest(int actorWorldX, int actorWorldY) const {
	(void)actorWorldY;
	return actorWorldX <= 0x1a2 || actorWorldX >= 0x283;
}

bool Scene6060::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void Scene6060::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

AmbientAudioProfile Scene6060::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

} // End of namespace Hollywood
