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

#include "hollywood/scenes/playable/scene4110.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene4110ArchiveName = "RESOURCE.D11";
const char *const kScene4110MusicArchiveName = "RESOURCE.M04";
const char *const kScene4110SoundArchiveName = "RESOURCE.S04";
const uint kScene4110InitialRequiredChunkCount = 5;
const uint kScene4110ArenaFirstChunk = 5;
const uint kScene4110ArenaLastChunk = 9;
const uint kScene4110StageIndex = 411;
const uint16 kScene4110FirstState = 0x100e;
const uint16 kScene4110SecondEntryState = 0x100f;
const uint16 kScene4110LastState = 0x1017;
const uint16 kScene4010EntryFromRightSideState = 0x0fab;
const uint16 kScene4100FirstState = 0x1004;
const uint16 kScene4110ViewportXOffset = 0x00c0;
const uint kScene4110ActorBankTableEntry = 0x0000;
const uint kScene4110ActorPaletteTableEntry = 0x00cc;
const uint kScene4110Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4110SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4110FrameMillis = 75;
const uint32 kScene4110BackgroundFrameMillis = 75;
const uint kScene4110PickupChunk = 5;
const uint kScene4110PickupDescriptorCount = 0x0c;
const uint kScene4110AlternateOverlayChunk = 6;
const uint kScene4110AlternateOverlayDescriptorCount = 0x0d;
const uint kScene4110AlternatePatchChunk = 7;
const uint kScene4110AlternateFinalPatchChunk = 8;
const uint kScene4110BackgroundChunk = 9;
const uint kScene4110BackgroundDescriptorCount = 10;
const byte kScene4110InventoryItem46 = 0x46;

const byte kScene4110ActorPathStepDeltaTableSetB4[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene4110BackgroundFrameMap[] = {
	0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 2, 1, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 3, 2, 1,
	0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const byte kScene4110BackgroundSequenceLengths[] = {
	1, 3, 5, 11, 2
};

const byte kScene4110PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene4110AlternateFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 5, 6, 6, 6,
	6, 6, 5, 6, 6, 6, 6, 6, 5, 6, 7, 8, 9
};

static PlayableSceneConfig scene4110Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene4110ArchiveName;
	config.initialRequiredChunkCount = kScene4110InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene4110ArenaFirstChunk;
	config.arenaLastChunk = kScene4110ArenaLastChunk;
	config.stageIndex = kScene4110StageIndex;
	config.debugName = "Scene 4110";
	config.viewportXOffset = kScene4110ViewportXOffset;
	config.viewportMinXOffset = kScene4110ViewportXOffset;
	config.viewportMaxXOffset = kScene4110ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 4;
	config.actorBankTableEntry = kScene4110ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene4110ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene4110Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene4110SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene4110ActorPathStepDeltaTableSetB4;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene4110ActorPathStepDeltaTableSetB4);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene4110MusicArchiveName;
	config.soundBank0ArchiveName = kScene4110SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene4110FirstState;
	config.mainFlowLastState = kScene4110LastState;
	return config;
}

Scene4110::Scene4110(HollywoodEngine *vm) :
		PlayableScene(vm, scene4110Config(), "scene4110", 0x0118, 0x0180, 2, 0xfd, 0xfb),
		_backgroundLayer(),
		_backgroundChannel(),
		_backgroundSequence(0),
		_backgroundFrameInSequence(0),
		_backgroundRepeatCounter(0),
		_ambientLoopSound(),
		_lastAmbientLoopCue(0xff) {
	_backgroundLayer.configure(kScene4110BackgroundChunk, kScene4110BackgroundDescriptorCount,
		nullptr, 0);
	_ambientLoopSound.setArchive(Common::Path(kScene4110SoundArchiveName));
}

bool Scene4110::hasCustomPreviewState() const {
	return true;
}

void Scene4110::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetBackgroundLayer();
	applySceneStateToHotspotsAndPatches(0xff);

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene4110SecondEntryState) {
		_activeActorWorldX = 0x0253;
		_activeActorWorldY = 0x00f2;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x0051;
		_activeActorWorldY = 0x01a2;
		_activeActorFacing = 2;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene4110::hasCustomComposite() const {
	return true;
}

void Scene4110::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_actionOverlayVisible) {
		drawActionOverlayLayer();
		drawResourceSpriteLayer(_backgroundLayer);
		if (!_hideActiveActor) {
			drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
				drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
		}
		return;
	}

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawResourceSpriteLayer(_backgroundLayer);
	drawActionOverlayLayer();
}

bool Scene4110::hasCustomEntrySequence() const {
	return true;
}

void Scene4110::runCustomEntrySequence() {
	resetBackgroundLayer();
	applySceneStateToHotspotsAndPatches(0xff);

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene4110SecondEntryState) {
		runEntryPath(0x0253, 0x00f2, 4, 0x0168, 0x015e);
	} else {
		runEntryPath(0x0051, 0x01a2, 2, 0x0118, 0x0180);
	}
}

bool Scene4110::prepareCustomGameplayLoop() {
	resetBackgroundLayer();
	applySceneStateToHotspotsAndPatches(0xff);
	return true;
}

bool Scene4110::advanceCustomGameplayLoop(uint32 delta) {
	advanceBackgroundLayer(delta);
	updateAmbientLoopSound();
	return false;
}

bool Scene4110::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a salida hacia laboratorio de Hecker (go to Hecker lab exit) or blocked line.
		if (_vm->gameState().scene4110AlternateSceneState)
			runExitToScene4010();
		else
			beginSecondarySpeechLine(0, 0);
		return true;
	case 302: // Mirar salida/estatua segun estado (look at exit/statue depending on state).
		beginSecondarySpeechLine(1, _vm->gameState().scene4110AlternateSceneState ? 1 : 0);
		return true;
	case 303: // Mirar obstaculo del pasillo (look at corridor obstruction).
		beginConditionalSpeechLine(2, 0, 2, 0);
		return true;
	case 304: // Usar obstaculo del pasillo (use corridor obstruction), blocked after alternate gate.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Coger objeto 0x46 (take item 0x46).
		takeItem46();
		return true;
	case 306: // Mirar objeto del pasillo (look at corridor object).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Mirar pared/estatua del pasillo (look at corridor wall/statue).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Mirar puerta del pasillo (look at corridor door).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Mirar estado alternativo del pasillo (look at alternate corridor state).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar elemento secundario del pasillo (look at secondary corridor element).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Ir al pasillo anterior (go to previous corridor).
		_vm->gameState().mainFlowStateId = kScene4100FirstState;
		return true;
	case 312: // Accion sin efecto del pasillo (no-op corridor action).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 313: // Usar/intercambiar objeto comun con elemento del pasillo (use common item with corridor element).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Activar secuencia alternativa del pasillo (trigger alternate corridor sequence).
		runAlternateStateSequence();
		return true;
	default:
		return false;
	}
}

bool Scene4110::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	if (_vm->gameState().scene4110AlternateSceneState &&
			_sceneChunkTable.isValidChunk(kScene4110AlternateFinalPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4110AlternateFinalPatchChunk], _baseFramebuffer);

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	patchActionMovementModes();
	return true;
}

AmbientAudioProfile Scene4110::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0f, 8, 25, 25, 0x0b, 5, 100, 50);
}

void Scene4110::resetBackgroundLayer() {
	_backgroundLayer.visible = true;
	_backgroundLayer.reset(0);
	_backgroundChannel.reset(0, kScene4110BackgroundFrameMillis);
	_backgroundSequence = 0;
	_backgroundFrameInSequence = 0;
	_backgroundRepeatCounter = 0;
}

void Scene4110::advanceBackgroundLayer(uint32 delta) {
	const uint frameCount = _backgroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		advanceBackgroundTick();
}

void Scene4110::advanceBackgroundTick() {
	if (_backgroundRepeatCounter == 0) {
		if (_random.getRandomNumber(49) == 0) {
			_backgroundSequence = (byte)_random.getRandomNumber(3);
			_backgroundRepeatCounter = (byte)(_random.getRandomNumber(9) + 1);
			_backgroundFrameInSequence = 1;
		} else if (_random.getRandomNumber(19) == 0) {
			_backgroundSequence = 4;
			_backgroundRepeatCounter = 1;
			_backgroundFrameInSequence = 1;
			_soundBank0.playSample(0x24, 25);
		} else {
			_backgroundLayer.setFrame(0);
			return;
		}
	} else if (_backgroundFrameInSequence >= kScene4110BackgroundSequenceLengths[_backgroundSequence]) {
		--_backgroundRepeatCounter;
		_backgroundSequence = _backgroundRepeatCounter == 0 ? 0 : (byte)_random.getRandomNumber(3);
		_backgroundFrameInSequence = 0;
	}

	const uint frameMapOffset = (uint)_backgroundSequence * 12 + _backgroundFrameInSequence;
	_backgroundLayer.setFrame(kScene4110BackgroundFrameMap[frameMapOffset]);
	++_backgroundFrameInSequence;
}

void Scene4110::updateAmbientLoopSound() {
	if (_ambientLoopSound.isPlaying())
		return;

	byte cue = _lastAmbientLoopCue;
	while (cue == _lastAmbientLoopCue)
		cue = (byte)(0x0b + _random.getRandomNumber(2));
	_lastAmbientLoopCue = cue;
	_ambientLoopSound.playSample(cue, 20);
}

void Scene4110::beginConditionalSpeechLine(uint16 falseRow, byte falseFrame, uint16 trueRow, byte trueFrame) {
	if (_vm->gameState().scene4110AlternateSceneState)
		beginSecondarySpeechLine(trueRow, trueFrame);
	else
		beginSecondarySpeechLine(falseRow, falseFrame);
}

void Scene4110::runExitToScene4010() {
	walkActiveActorTo(0x0253, 0x00f2, 0xff, 0);
	_vm->gameState().mainFlowStateId = kScene4010EntryFromRightSideState;
}

void Scene4110::takeItem46() {
	GameplayState &state = _vm->gameState();
	if (state.scene4110Item46Taken || hasInventoryItem(kScene4110InventoryItem46)) {
		beginSecondarySpeechLine(3, 1);
		return;
	}

	beginSecondarySpeechLine(3, 0);
	runConfiguredActionOverlay(kScene4110PickupChunk, kScene4110PickupDescriptorCount,
		kScene4110PickupFrameMap, ARRAYSIZE(kScene4110PickupFrameMap),
		kScene4110FrameMillis, kActionOverlayHideActiveActor, -1, 0, -1, 0, 100,
		-1, 0, true, 1, ARRAYSIZE(kScene4110PickupFrameMap));
	addInventoryItem(kScene4110InventoryItem46);
	_soundBank0.playSample(1, 100);
	state.scene4110Item46Taken = true;
	applySceneStateToHotspotsAndPatches(0);
}

void Scene4110::runAlternateStateSequence() {
	GameplayState &state = _vm->gameState();
	if (state.scene4110AlternateSceneState) {
		beginSecondarySpeechLine(7, 0);
		return;
	}

	if (_sceneChunkTable.isValidChunk(kScene4110AlternatePatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4110AlternatePatchChunk], _baseFramebuffer);
	drawPlayableComposite();
	presentFrame();

	runConfiguredActionOverlay(kScene4110AlternateOverlayChunk, kScene4110AlternateOverlayDescriptorCount,
		kScene4110AlternateFrameMap, ARRAYSIZE(kScene4110AlternateFrameMap),
		kScene4110FrameMillis, kActionOverlayKeepActiveActorVisibility, -1, 0, 0x1c, 0x31, 100,
		-1, 0, true, 1, ARRAYSIZE(kScene4110AlternateFrameMap));

	if (_sceneChunkTable.isValidChunk(kScene4110AlternateFinalPatchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene4110AlternateFinalPatchChunk], _baseFramebuffer);

	_activeActorWorldX = 0x0221;
	_activeActorWorldY = 0x011c;
	_activeActorFacing = 4;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	walkActiveActorTo(0x01e6, 0x011c, 5, 0);

	state.scene4110AlternateSceneState = true;
	state.scene4110PostAlternateFlag = true;
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4110::patchActionMovementModes() {
	if (_vm->gameState().scene4110Item46Taken)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x13, 0);
	if (_vm->gameState().scene4110AlternateSceneState)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x35, 0);
}

} // End of namespace Hollywood
