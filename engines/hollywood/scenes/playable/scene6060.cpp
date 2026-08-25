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

const char *const kScene6060ArchiveName = "RESOURCE.F06";
const char *const kScene6060MusicArchiveName = "RESOURCE.M06";
const char *const kScene6060SoundArchiveName = "RESOURCE.S06";
const uint kScene6060InitialRequiredChunkCount = 5;
// F06 contains only the five fixed scene chunks, so its arena range is empty.
const uint kScene6060ArenaFirstChunk = 5;
const uint kScene6060ArenaLastChunk = 4;
const uint kScene6060StageIndex = 606;
const uint16 kScene6060FirstState = 0x17ac;
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

const byte kScene6060ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

static PlayableSceneConfig scene6060Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene6060ArchiveName;
	config.initialRequiredChunkCount = kScene6060InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene6060ArenaFirstChunk;
	config.arenaLastChunk = kScene6060ArenaLastChunk;
	config.stageIndex = kScene6060StageIndex;
	config.debugName = "Scene 6060";
	config.viewportXOffset = kScene6060EntryFromLobbyViewportX;
	config.viewportMinXOffset = kScene6060ViewportMinX;
	config.viewportMaxXOffset = kScene6060ViewportMaxX;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 6;
	config.actorBankTableEntry = kScene6060ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene6060ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene6060Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene6060SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene6060ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene6060ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 1;
	config.musicArchiveName = kScene6060MusicArchiveName;
	config.soundBank0ArchiveName = kScene6060SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene6060FirstState;
	config.mainFlowLastState = kScene6060LastState;
	return config;
}

Scene6060::Scene6060(HollywoodEngine *vm) :
		PlayableScene(vm, scene6060Config(), "scene6060", 0x2d0, 0x192, 5, 0xfd, 0xfb) {
}

bool Scene6060::hasCustomPreviewState() const {
	return true;
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

bool Scene6060::hasCustomEntrySequence() const {
	return true;
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
		_actorPathStepDeltas[0x18 + i] = kScene6060ActorPathStepDeltaTable[0x24 + i];
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
		_actorPathStepDeltas[0x3c + i] = kScene6060ActorPathStepDeltaTable[i];
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
