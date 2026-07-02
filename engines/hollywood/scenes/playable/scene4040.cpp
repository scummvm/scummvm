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

#include "hollywood/scenes/playable/scene4040.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene4040ArchiveName = "RESOURCE.D04";
const char *const kScene4040MusicArchiveName = "RESOURCE.M04";
const char *const kScene4040SoundArchiveName = "RESOURCE.S04";
const uint kScene4040InitialRequiredChunkCount = 5;
const uint kScene4040ArenaFirstChunk = 5;
const uint kScene4040ArenaLastChunk = 15;
const uint kScene4040StageIndex = 404;
const uint16 kScene4040FirstState = 0x0fc8;
const uint16 kScene4040LastState = 0x0fd1;
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
const uint kScene4040StairOverlayChunk = 9;
const uint kScene4040StairOverlayDescriptorCount = 0x3e;
const uint kScene4040CandilOverlayChunk = 14;
const uint kScene4040CandilOverlayDescriptorCount = 9;
const byte kScene4040CandilItem = 0x3c;
const byte kScene4040CandilSceneItem = 8;

const byte kScene4040ActorPathStepDeltaTableSetB4[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene4040StairOverlayFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
	60, 61
};

const byte kScene4040CandilFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8
};

PlayableSceneConfig scene4040Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene4040ArchiveName;
	config.initialRequiredChunkCount = kScene4040InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene4040ArenaFirstChunk;
	config.arenaLastChunk = kScene4040ArenaLastChunk;
	config.stageIndex = kScene4040StageIndex;
	config.debugName = "Scene 4040";
	config.viewportXOffset = kScene4040ViewportXOffset;
	config.viewportMinXOffset = kScene4040ViewportMinXOffset;
	config.viewportMaxXOffset = kScene4040ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 4;
	config.actorBankTableEntry = kScene4040ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene4040ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene4040Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene4040SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene4040ActorPathStepDeltaTableSetB4;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene4040ActorPathStepDeltaTableSetB4);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene4040MusicArchiveName;
	config.soundBank0ArchiveName = kScene4040SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = false;
	config.mainFlowFirstState = kScene4040FirstState;
	config.mainFlowLastState = kScene4040LastState;
	return config;
}

Scene4040::Scene4040(HollywoodEngine *vm) :
		PlayableScene(vm, scene4040Config(), "scene4040", 0x192, 0x0171, 2, 0xfd, 0xfb) {
}

bool Scene4040::hasCustomPreviewState() const {
	return true;
}

void Scene4040::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	_activeActorWorldX = 0x192;
	_activeActorWorldY = 0x0171;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene4040::hasCustomComposite() const {
	return true;
}

void Scene4040::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldY);
	drawActionOverlayLayer();
}

bool Scene4040::hasCustomEntrySequence() const {
	return true;
}

void Scene4040::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (_vm->gameState().mainFlowStateId == kScene4040FirstState && !state.seenScene4040EntryLine) {
		beginSecondarySpeechLine(0, 0);
		state.seenScene4040EntryLine = true;
	}
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
	case 310: // Coger candil (take oil lamp): item 0x3c.
		takeCandil();
		return true;
	case 311: // Mirar candil (look at oil lamp).
		beginSecondarySpeechLine(9, 0);
		return true;
	default:
		return false;
	}
}

bool Scene4040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

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
	return createRandomAmbientAudioProfile(0x0b, 3, 20, 1, 0x0b, 5, 100, 50);
}

void Scene4040::runStairReturnToDungeon() {
	runConfiguredActionOverlay(kScene4040StairOverlayChunk, kScene4040StairOverlayDescriptorCount,
		kScene4040StairOverlayFrameMap, ARRAYSIZE(kScene4040StairOverlayFrameMap),
		kScene4040FrameMillis, kActionOverlayHideActiveActor);
	_vm->gameState().mainFlowStateId = kScene4030ReturnState;
}

void Scene4040::takeCandil() {
	GameplayState &state = _vm->gameState();
	if (state.scene4040CandilTaken) {
		beginSecondarySpeechLine(8, 0);
		return;
	}

	beginSecondarySpeechLine(8, 0);
	state.scene4040CandilTaken = true;
	runConfiguredActionOverlay(kScene4040CandilOverlayChunk, kScene4040CandilOverlayDescriptorCount,
		kScene4040CandilFrameMap, ARRAYSIZE(kScene4040CandilFrameMap),
		kScene4040FrameMillis, kActionOverlayKeepActiveActorVisibility, 4, 1);
	addInventoryItem(kScene4040CandilItem);
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
