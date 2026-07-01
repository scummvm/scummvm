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

#include "hollywood/scenes/playable/scene6010.h"

#include "common/debug.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene6010ArchiveName = "RESOURCE.F01";
const char *const kScene6010MusicArchiveName = "RESOURCE.M06";
const char *const kScene6010SoundArchiveName = "RESOURCE.S06";
const uint kScene6010InitialRequiredChunkCount = 19;
const uint kScene6010ArenaFirstChunk = 5;
const uint kScene6010ArenaLastChunk = 18;
const uint kScene6010StageIndex = 601;
const uint16 kScene6010FirstState = 0x177a;
const uint16 kScene6010LastState = 0x1783;
const uint16 kScene6010EntryState = 0x177a;
const uint16 kScene6011EntryState = 0x177b;
const uint16 kScene6012EntryState = 0x177c;
const uint16 kScene6013EntryState = 0x177d;
const uint16 kScene6020State = 0x1784;
const uint16 kScene6050State = 0x17a2;
const uint16 kScene6010ViewportXOffset = 0x0138;
const uint16 kScene6010ViewportMinXOffset = 0x00a8;
const uint16 kScene6010ViewportMaxXOffset = 0x0138;
const uint kScene6010ActorBankTableEntry = 0x0000;
const uint kScene6010ActorPaletteTableEntry = 0x00cc;
const uint kScene6010Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6010SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6010FrameMillis = 75;
const uint32 kScene6010ClipFrameMillis = 50;
const uint kScene6010ClipChunkIndex = 7;
const uint kScene6010ClipDescriptorCount = 0xda;
const uint kScene6010DoorRevealPrimaryDescriptorCount = 4;
const uint kScene6010DoorRevealSecondaryDescriptorCount = 0x13;
const uint kScene6010ExitDescriptorCount = 6;
const uint kScene6010Pickup59DescriptorCount = 0x0d;
const uint kScene6010Pickup58DescriptorCount = 0x0a;
const uint kScene6010PendingItem69DescriptorCount = 0x0e;

const byte kScene6010ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene6010ActorPathStepDeltaTableSet87[] = {
	6, 1, 1, 3, 3, 3, 7, 1, 0, 0, 4, 3,
	3, 2, 8, 6, 6, 7, 6, 4, 10, 3, 2, 9,
	8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10,
	4, 3, 3, 4, 0, 4, 4, 2, 0, 4, 2, 5,
	6, 10, 10, 4, 6, 4, 10, 8, 8, 7, 5, 10,
	6, 4, 10, 3, 2, 9, 3, 2, 8, 6, 6, 7
};

const byte kScene6010PendingItem69FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

const byte kScene6010DoorRevealPrimaryFrameMap[] = {
	0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0
};

const byte kScene6010DoorRevealSecondaryFrameMap[] = {
	0x0c, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 13, 14, 15, 16, 17, 18
};

const byte kScene6010ExitFrameMap[] = { 0, 1, 2, 3, 4, 5 };

const byte kScene6010Pickup59FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene6010Pickup58FrameMap[] = {
	0, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	1, 2, 1, 0, 1, 2, 1, 0, 1, 2,
	1, 0, 1, 2, 1, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9
};

static PlayableSceneConfig scene6010Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene6010ArchiveName;
	config.initialRequiredChunkCount = kScene6010InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene6010ArenaFirstChunk;
	config.arenaLastChunk = kScene6010ArenaLastChunk;
	config.stageIndex = kScene6010StageIndex;
	config.debugName = "Scene 6010";
	config.viewportXOffset = kScene6010ViewportXOffset;
	config.viewportMinXOffset = kScene6010ViewportMinXOffset;
	config.viewportMaxXOffset = kScene6010ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 6;
	config.actorBankTableEntry = kScene6010ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene6010ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene6010Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene6010SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene6010ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene6010ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 9;
	config.musicArchiveName = kScene6010MusicArchiveName;
	config.soundBank0ArchiveName = kScene6010SoundArchiveName;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene6010FirstState;
	config.mainFlowLastState = kScene6010LastState;
	return config;
}

Scene6010::Scene6010(HollywoodEngine *vm) :
		PlayableScene(vm, scene6010Config(), "scene6010", 0x327, 0x1c5, 5, 0xfd, 0xfb),
		_originalColorToItemMap(),
		_temporaryPrimaryLayer(),
		_temporarySecondaryLayer() {
}

bool Scene6010::hasCustomPreviewState() const {
	return true;
}

void Scene6010::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene6011EntryState) {
		_activeActorWorldX = 0x147;
		_activeActorWorldY = 0x0ed;
		_activeActorFacing = 2;
	} else if (stateId == kScene6012EntryState) {
		_activeActorWorldX = 0x2e2;
		_activeActorWorldY = 0x1ba;
		_activeActorFacing = 4;
	} else if (stateId == kScene6013EntryState) {
		_activeActorWorldX = 0x181;
		_activeActorWorldY = 0x171;
		_activeActorFacing = 2;
	} else {
		_activeActorWorldX = 0x327;
		_activeActorWorldY = 0x1c5;
		_activeActorFacing = 5;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene6010::hasCustomComposite() const {
	return true;
}

void Scene6010::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();
	updateSceneDepthThresholds(actorDrawOrderMode, activeWorldX, activeWorldY);
	drawTemporaryOverlayLayers();
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
}

bool Scene6010::hasCustomEntrySequence() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return stateId >= kScene6010EntryState && stateId <= kScene6013EntryState;
}

void Scene6010::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene6010EntryState:
		runEntryFromTravel();
		break;
	case kScene6011EntryState:
		runEntryAndPendingPickup();
		break;
	case kScene6012EntryState:
		runEntryFromRight();
		break;
	case kScene6013EntryState:
		runEntryCutsceneState();
		break;
	default:
		drawPlayableComposite();
		presentFrame();
		break;
	}
}

bool Scene6010::prepareCustomGameplayLoop() {
	_temporaryPrimaryLayer.visible = false;
	_temporarySecondaryLayer.visible = false;
	return true;
}

bool Scene6010::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene6010::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();
	switch (handlerId) {
	case 301: // Ir/usar puerta del plato (go/use studio stage door): guard/reveal branch.
		if (!state.scene6010StudioEntryUnlocked) {
			if (state.scene6010DoorActionState == 2) {
				beginSecondarySpeechLine(1, 7);
				walkActiveActorTo(0x3b0, 0x181, 0xff, 0, false);
				state.scene6010DoorActionState = 3;
			}
			state.mainFlowStateId = 0x1798;
			return true;
		}
		if (state.scene6010DoorActionState == 2) {
			beginSecondarySpeechLine(1, 6);
			return true;
		}
		if (state.scene6010DoorActionState == 0) {
			beginPrimarySpeechLine(1, 0, 0x3b6, 0x096, 0x3f, 0x20, 0);
			beginSecondarySpeechLine(1, 1);
			walkActiveActorTo(0x2e2, 0x1ba, 0xff, 0, false);
			walkActiveActorTo(0x2e2, 0x1ba, 1, 0, false);
			beginSecondarySpeechLine(1, 2);
			state.scene6010DoorActionState = 1;
			return true;
		}
		beginPrimarySpeechLine(1, 3, 0x3b6, 0x096, 0x3f, 0x20, 0);
		walkActiveActorTo(0x2e2, 0x1ba, 0xff, 0, false);
		walkActiveActorTo(0x2e2, 0x1ba, 1, 0, false);
		beginPrimarySpeechLine(1, 4, 0x3b6, 0x096, 0x3f, 0x20, 0);
		runDoorRevealOverlay();
		beginSecondarySpeechLine(1, 5);
		state.scene6010DoorActionState = 2;
		applySceneStateToHotspotsAndPatches(2);
		return true;
	case 302: // Mirar puerta/plato de cine (look at studio stage door).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Mirar puerta del museo de cera (look at wax museum door/sign).
		beginSecondarySpeechLine(3, 1);
		return true;
	case 304: // Ir a puerta del museo de cera (go to wax museum door): reports closed until the endgame entry.
		if (state.scene6010StudioEntryUnlocked)
			beginSecondarySpeechLine(4, 0);
		else
			runStudioClipSequence(true);
		return true;
	case 305: // Mirar oficinas de los estudios (look at studio offices), state-aware.
		beginSecondarySpeechLine(5, state.scene6010ExitOverlayPlayed ? 1 : 0);
		return true;
	case 306: // Ir a oficinas de los estudios (go to studio offices): scene 6020.
		runExitToScene6020Overlay();
		return true;
	case 307: // Usar/abrir oficinas (use/open offices): generic refusal.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Mirar cartel "Museo de los horrores Hannover" (look at museum sign).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Mirar buzon (look at mailbox): brochure slot or already empty.
		if (state.scene6010Item58Taken)
			beginSecondarySpeechLine(0x50, 1);
		else
			beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Usar/abrir buzon (use/open mailbox): locked.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Coger paquete del mensajero (take courier package): item 0x59.
		runPickupItem59Overlay();
		return true;
	case 312: // Mirar pelicula/cartel de la pelicula (look at movie/poster title).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 313: // Coger folleto del buzon (take mailbox brochure): item 0x58.
		runPickupItem58Overlay();
		return true;
	case 314: // Usar coche (use car): open Ron destination selector unless blocked.
		if (state.scene6010EndgameTravelExitBlocked) {
			beginSecondarySpeechLine(0, 1);
			return true;
		}
		state.requestTravelScreenSelection(6);
		return true;
	default:
		return false;
	}
}

bool Scene6010::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	if ((currentRegion == 9 && nextRegion == 3) ||
			(currentRegion == 3 && nextRegion == 4) ||
			(currentRegion == 6 && nextRegion == 5)) {
		copyStepDeltasFromSet87(0x30, 0x3b);
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}

	if ((currentRegion == 6 && nextRegion == 7) ||
			(currentRegion == 3 && nextRegion == 9) ||
			(currentRegion == 9 && nextRegion == 1)) {
		copyStepDeltasFromSet87(0x0c, 0x17);
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}

	if (currentRegion == 7 && nextRegion == 8) {
		requestedFacing = 1;
		return true;
	}

	return false;
}

bool Scene6010::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetY;
	(void)restoredStepDeltas;
	if (currentRegion == 8 && targetRegion == 8 && targetX < state.x) {
		requestedFacing = 5;
		return true;
	}

	return false;
}

bool Scene6010::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	rememberOriginalColorMap();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (selector == 0 || selector == 0xff) {
		if (!state.scene6010StudioEntryUnlocked) {
			if (_sceneChunkTable.isValidChunk(5))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
			if (_sceneChunkTable.isValidChunk(6))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
			for (uint i = 0; i < kScenePaletteMapPageSize && i < _paletteMask.size(); ++i) {
				if (_paletteMask[i] == 9) {
					_paletteMask[i] = 1;
					_fullPaletteRegionMask[i] = 1;
				}
			}
			replaceColorMapItemFromOriginal(4, 0);
		} else {
			for (uint i = 0; i < kScenePaletteMapPageSize && i < _paletteMask.size(); ++i) {
				if (_paletteMask[i] == 9) {
					_paletteMask[i] = 0;
					_fullPaletteRegionMask[i] = 0;
				}
			}
			_hotspots.setVerbMovementModeByGlobalRecordIndex(0x11, 0);
		}
	}

	if (selector == 1 || selector == 0xff) {
		replaceColorMapItemFromOriginal(6, state.scene6010Item59Visible ? 6 : 0);
		const uint patchChunk = state.scene6010Item59Visible ? 14 : 15;
		if (_sceneChunkTable.isValidChunk(patchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[patchChunk], _baseFramebuffer);
	}

	if ((selector == 2 || selector == 0xff) && state.scene6010DoorActionState == 2)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(9, 0);

	if ((selector == 4 || selector == 0xff) && state.scene6010EndgameTravelExitBlocked)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x39, 0);

	if (selector == 5 || selector == 0xff) {
		const uint patchChunk = state.scene6011PendingItem69Visible ? 10 : 11;
		if (_sceneChunkTable.isValidChunk(patchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[patchChunk], _baseFramebuffer);
	}

	rebuildStudioWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (state.scene6010StudioEntryUnlocked)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x11, 0);
	if (state.scene6010DoorActionState == 2)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(9, 0);
	if (state.scene6010EndgameTravelExitBlocked)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x39, 0);
	return true;
}

AmbientAudioProfile Scene6010::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene6010::updateSceneDepthThresholds(byte actorDrawOrderMode, int activeWorldX, int activeWorldY) {
	_drawActorDepthYThresholds = _actorDepthYThresholds;
	if (_drawActorDepthYThresholds.size() <= 3)
		return;

	if (actorDrawOrderMode == 1 && activeWorldY <= 0x21d && activeWorldX < 0x240) {
		_drawActorDepthYThresholds[1] = 0x03e7;
		_drawActorDepthYThresholds[2] = 0x03e7;
		_drawActorDepthYThresholds[3] = 0x03e7;
		return;
	}

	switch (actorDrawOrderMode) {
	case 3:
	case 8:
		_drawActorDepthYThresholds[1] = 0x03e7;
		_drawActorDepthYThresholds[2] = 0;
		_drawActorDepthYThresholds[3] = 0x03e7;
		break;
	case 5:
	case 6:
	case 7:
		_drawActorDepthYThresholds[1] = 0x03e7;
		_drawActorDepthYThresholds[2] = 0;
		_drawActorDepthYThresholds[3] = 0;
		break;
	default:
		break;
	}
}

void Scene6010::rebuildStudioWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 1 && _walkablePaletteMask[i] < 8)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene6010::rememberOriginalColorMap() {
	if (!_originalColorToItemMap.empty() ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	_originalColorToItemMap.resize(kScenePaletteMapPageSize);
	memcpy(_originalColorToItemMap.data(), _paletteMaskOriginal.data() + kSceneColorToItemMap,
		_originalColorToItemMap.size());
}

void Scene6010::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_originalColorToItemMap.size() < kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_originalColorToItemMap[i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene6010::copyStepDeltasFromSet87(uint firstOffset, uint lastOffset) {
	for (uint offset = firstOffset; offset <= lastOffset &&
			offset < _actorPathStepDeltas.size() &&
			offset < ARRAYSIZE(kScene6010ActorPathStepDeltaTableSet87); ++offset) {
		_actorPathStepDeltas[offset] = kScene6010ActorPathStepDeltaTableSet87[offset];
	}
}

void Scene6010::runEntryFromTravel() {
	runEntryPath(0x3b0, 0x1df, 5, 0x327, 0x1c5);
}

void Scene6010::runEntryAndPendingPickup() {
	_soundBank0.playSample(4, 100);
	runEntryPath(0x147, 0x0ed, 2, 0x147, 0x0ed);
	if (_vm->gameState().scene6011PendingItem69Visible)
		runPendingItem69PickupOverlay();
}

void Scene6010::runEntryFromRight() {
	runEntryPath(0x3b0, 0x181, 4, 0x2e2, 0x1ba);
}

void Scene6010::runEntryCutsceneState() {
	_vm->gameState().currentAmbientMusicCueId = 0x0b;
	_vm->gameplayMusic()->playMusicCue(0x0b, 100);
	runEntryPath(0x181, 0x171, 2, 0x181, 0x171);
	if (!_vm->gameState().scene6010StudioEntryUnlocked)
		runStudioClipSequence(false);
	else
		beginSecondarySpeechLine(4, 0);
}

void Scene6010::runStudioClipSequence(bool exitAfterPlayback) {
	drawPlayableComposite();
	presentFrame();

	Common::Array<byte> frameMap;
	frameMap.resize(0x113);
	for (uint i = 0; i < frameMap.size(); ++i) {
		frameMap[i] = 0xff;
		if (i >= 0x10 && i <= 0x70)
			frameMap[i] = (byte)(i - 0x10);
		else if (i >= 0x9b)
			frameMap[i] = (byte)(i - 0x3a);
	}

	for (uint i = 0; i < frameMap.size() && !Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++i) {
		if (frameMap[i] != 0xff) {
			drawClipFrameDelta(kScene6010ClipChunkIndex, kScene6010ClipDescriptorCount, frameMap[i]);
			presentFrame();
		}
		if (waitClipFrame(kScene6010ClipFrameMillis))
			break;
	}

	if (exitAfterPlayback) {
		_vm->gameState().mainFlowStateId = kScene6050State;
		return;
	}

	walkActiveActorTo(0x227, 0x19b, 0xff, 0, false);
}

void Scene6010::drawClipFrameDelta(uint chunkIndex, uint tableEntryCount, byte frameIndex) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return;

	const uint32 frameTableOffset = _resourceChunkOffsets[chunkIndex];
	const uint32 tableEntryOffset = frameTableOffset + ((uint32)frameIndex * 4);
	if (tableEntryOffset + 4 > _resourceArena.size())
		return;

	const uint32 frameOffset = frameTableOffset + ((uint32)tableEntryCount * 4) +
		readUint32LE(_resourceArena, tableEntryOffset);
	if (frameOffset + 4 > _resourceArena.size())
		return;

	const uint16 firstRow = readUint16LE(_resourceArena, frameOffset);
	const uint16 lastRow = readUint16LE(_resourceArena, frameOffset + 2);
	uint cursor = frameOffset + 4;
	byte *pixels = framebufferPixels(_sceneFramebuffer);
	const uint size = framebufferByteCount();

	for (uint row = firstRow; row <= lastRow && row < HollywoodEngine::kSceneBufferHeight; ++row) {
		if (cursor >= _resourceArena.size())
			return;

		byte runCount = _resourceArena[cursor++];
		for (; runCount != 0; --runCount) {
			if (cursor + 3 > _resourceArena.size())
				return;

			const uint x = readUint16LE(_resourceArena, cursor);
			const byte literalLength = _resourceArena[cursor + 2];
			const uint destinationOffset = row * HollywoodEngine::kSceneBufferWidth + x;
			if (destinationOffset >= size)
				return;

			if (literalLength == 0) {
				if (cursor + 5 > _resourceArena.size())
					return;

				const byte fillValue = _resourceArena[cursor + 3];
				const uint fillLength = _resourceArena[cursor + 4];
				cursor += 5;
				if (destinationOffset + fillLength <= size)
					memset(pixels + destinationOffset, fillValue, fillLength);
			} else {
				const uint literalOffset = cursor + 3;
				if (literalOffset + literalLength > _resourceArena.size())
					return;

				if (destinationOffset + literalLength <= size)
					memcpy(pixels + destinationOffset, _resourceArena.data() + literalOffset, literalLength);
				cursor = literalOffset + literalLength;
			}
		}
	}
}

bool Scene6010::waitClipFrame(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(true))
			return true;
		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		remaining -= slice;
	}
	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void Scene6010::runPendingItem69PickupOverlay() {
	beginSecondarySpeechLine(0x0c, 1);
	walkActiveActorTo(0x15d, 0x0ed, 1, 0, false);

	GameplayState &state = _vm->gameState();
	state.scene6011PendingItem69Visible = false;
	runConfiguredActionOverlay(12, kScene6010PendingItem69DescriptorCount,
		kScene6010PendingItem69FrameMap, ARRAYSIZE(kScene6010PendingItem69FrameMap),
		kScene6010FrameMillis, kActionOverlayHideActiveActor, 7, 5);

	addInventoryItem(0x69);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(0x0c, 0);
}

void Scene6010::runDoorRevealOverlay() {
	runLayeredOverlay(17, kScene6010DoorRevealPrimaryDescriptorCount,
		kScene6010DoorRevealPrimaryFrameMap, ARRAYSIZE(kScene6010DoorRevealPrimaryFrameMap),
		16, kScene6010DoorRevealSecondaryDescriptorCount,
		kScene6010DoorRevealSecondaryFrameMap, ARRAYSIZE(kScene6010DoorRevealSecondaryFrameMap),
		40, 13, 0x0c);
	_vm->gameState().scene6010Item59Visible = true;
	applySceneStateToHotspotsAndPatches(1);
	drawPlayableComposite();
	presentFrame();
}

void Scene6010::runExitToScene6020Overlay() {
	runConfiguredActionOverlay(9, kScene6010ExitDescriptorCount,
		kScene6010ExitFrameMap, ARRAYSIZE(kScene6010ExitFrameMap),
		kScene6010FrameMillis, kActionOverlayHideActiveActor, -1, 0,
		ARRAYSIZE(kScene6010ExitFrameMap) - 1, 3);
	_vm->gameState().scene6010ExitOverlayPlayed = true;
	_vm->gameState().mainFlowStateId = kScene6020State;
}

void Scene6010::runPickupItem59Overlay() {
	runLayeredOverlay(18, kScene6010Pickup59DescriptorCount,
		kScene6010Pickup59FrameMap, ARRAYSIZE(kScene6010Pickup59FrameMap),
		0, 0, nullptr, 0, kScene6010FrameMillis);
	addInventoryItem(0x59);
	_vm->gameState().scene6010Item59Visible = false;
	applySceneStateToHotspotsAndPatches(1);
	drawPlayableComposite();
	presentFrame();
	_soundBank0.playSample(1, 100);
	beginStaticSecondarySpeechLine(0xb6, 0);
}

void Scene6010::runPickupItem58Overlay() {
	GameplayState &state = _vm->gameState();
	if (state.scene6010Item58Taken) {
		beginSecondarySpeechLine(0x0b, 2);
		return;
	}

	beginSecondarySpeechLine(0x0b, 0);
	runLayeredOverlay(13, kScene6010Pickup58DescriptorCount,
		kScene6010Pickup58FrameMap, ARRAYSIZE(kScene6010Pickup58FrameMap),
		0, 0, nullptr, 0, kScene6010FrameMillis);
	addInventoryItem(0x58);
	state.scene6010Item58Taken = true;
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(0x0b, 1);
}

void Scene6010::runLayeredOverlay(uint primaryChunkIndex, uint primaryDescriptorCount,
		const byte *primaryFrameMap, uint primaryFrameMapSize,
		uint secondaryChunkIndex, uint secondaryDescriptorCount,
		const byte *secondaryFrameMap, uint secondaryFrameMapSize,
		uint32 frameMillis, int soundFrame, byte soundId) {
	_temporaryPrimaryLayer.configure(primaryChunkIndex, (uint16)primaryDescriptorCount,
		primaryFrameMap, primaryFrameMapSize);
	_temporaryPrimaryLayer.visible = primaryFrameMap != nullptr && primaryFrameMapSize != 0;
	if (secondaryFrameMap != nullptr && secondaryFrameMapSize != 0) {
		_temporarySecondaryLayer.configure(secondaryChunkIndex, (uint16)secondaryDescriptorCount,
			secondaryFrameMap, secondaryFrameMapSize);
		_temporarySecondaryLayer.visible = true;
	} else {
		_temporarySecondaryLayer.visible = false;
	}

	const uint frameCount = MAX<uint>(primaryFrameMapSize, secondaryFrameMapSize);
	const bool previousHideActor = _hideActiveActor;
	_hideActiveActor = true;
	for (uint frame = 0; frame < frameCount && !Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++frame) {
		if (_temporaryPrimaryLayer.visible)
			_temporaryPrimaryLayer.setFrame((byte)MIN<uint>(frame, primaryFrameMapSize - 1));
		if (_temporarySecondaryLayer.visible)
			_temporarySecondaryLayer.setFrame((byte)MIN<uint>(frame, secondaryFrameMapSize - 1));
		if (soundFrame >= 0 && (int)frame == soundFrame)
			_soundBank0.playSample(soundId, 80);
		if (waitSceneMillis(frameMillis))
			break;
	}
	_hideActiveActor = previousHideActor;
	_temporaryPrimaryLayer.visible = false;
	_temporarySecondaryLayer.visible = false;
	drawPlayableComposite();
	presentFrame();
}

void Scene6010::drawTemporaryOverlayLayers() {
	if (_temporarySecondaryLayer.visible)
		drawResourceSpriteLayer(_temporarySecondaryLayer);
	if (_temporaryPrimaryLayer.visible)
		drawResourceSpriteLayer(_temporaryPrimaryLayer);
}

} // End of namespace Hollywood
