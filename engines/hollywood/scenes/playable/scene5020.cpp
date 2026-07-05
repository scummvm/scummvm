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

#include "hollywood/scenes/playable/scene5020.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const char *const kScene5020ArchiveName = "RESOURCE.E02";
const char *const kScene5020MusicArchiveName = "RESOURCE.M05";
const char *const kScene5020SoundArchiveName = "RESOURCE.S05";
const uint kScene5020InitialRequiredChunkCount = 5;
const uint kScene5020ArenaFirstChunk = 5;
const uint kScene5020ArenaLastChunk = 10;
const uint kScene5020StageIndex = 502;
const uint16 kScene5020State = 0x139c;
const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5020ViewportXOffset = 0x0048;
const uint16 kScene5020ViewportMinXOffset = 0x0000;
const uint16 kScene5020ViewportMaxXOffset = 0x0048;
const uint kScene5020ActorBankTableEntry = 0x0000;
const uint kScene5020ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5020SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5020OverlayFrameMillis = 75;
const byte kScene5020WoodenPlankItem = 0x47;
const byte kScene5020TakenSceneItemId = 4;
const byte kScene5020RenamedSmallRow = 7;
const byte kScene5020WallClueSmallRow = 8;
const uint kScene5020PickupOverlayChunk = 10;
const uint kScene5020PickupOverlayDescriptorCount = 0x0d;

const byte kScene5020ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene5020PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

PlayableSceneConfig scene5020Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene5020ArchiveName;
	config.initialRequiredChunkCount = kScene5020InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene5020ArenaFirstChunk;
	config.arenaLastChunk = kScene5020ArenaLastChunk;
	config.stageIndex = kScene5020StageIndex;
	config.debugName = "Scene 5020";
	config.viewportXOffset = kScene5020ViewportXOffset;
	config.viewportMinXOffset = kScene5020ViewportMinXOffset;
	config.viewportMaxXOffset = kScene5020ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 5;
	config.actorBankTableEntry = kScene5020ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene5020ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = 0;
	config.speechCueDescriptorTableOffset = kScene5020SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene5020ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene5020ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 3;
	config.musicArchiveName = kScene5020MusicArchiveName;
	config.soundBank0ArchiveName = kScene5020SoundArchiveName;
	config.mainFlowFirstState = kScene5020State;
	config.mainFlowLastState = kScene5020State;
	return config;
}

Scene5020::Scene5020(HollywoodEngine *vm) :
		PlayableScene(vm, scene5020Config(), "scene5020", 0x27d, 0x16c, 4, 0xfd, 0xfb) {
}

bool Scene5020::hasCustomPreviewState() const {
	return true;
}

void Scene5020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);

	_activeActorWorldX = 0x27d;
	_activeActorWorldY = 0x16c;
	_activeActorFacing = 4;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene5020::hasCustomEntrySequence() const {
	return true;
}

void Scene5020::runCustomEntrySequence() {
	runEntryPath(0x348, 0x186, 4, 0x27d, 0x16c);
	_activeActorFacing = 4;
	_activeActorCel = 0;
}

bool Scene5020::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a salida/vagoneta (go to exit/mine cart): return to mine switches.
		runExitToMineSwitches();
		return true;
	case 302: // Coger madero (take wooden plank): grants inventory item 0x47.
		runPickupWoodenPlank();
		return true;
	case 303: // Mirar madero (look at wooden plank): Ron says it is a good piece of wood.
		beginSecondarySpeechLine(0, 0);
		return true;
	case 304: // Mirar/coger tunel deteriorado (look/take deteriorated tunnel support).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 305: // Coger pala encajada (take stuck shovel).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 306: // Mirar pala (look at shovel).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 307: // Mirar caja/caja de explosivos (look at box/explosives crate): first look reveals dynamite.
		beginSecondarySpeechLine(4, _vm->gameState().scene5020ExplosivesCrateIdentified ? 1 : 0);
		_vm->gameState().scene5020ExplosivesCrateIdentified = true;
		applySceneStateToHotspotsAndPatches(0);
		return true;
	case 308: // Abrir caja de explosivos (open explosives crate): lid is nailed shut.
		beginSecondarySpeechLine(5, 0);
		return true;
	default:
		return false;
	}
}

bool Scene5020::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if ((selector == 0 || selector == 0xff) && state.scene5020ExplosivesCrateIdentified)
		copyStageSmallRow(kScene5020RenamedSmallRow, kScene5020WallClueSmallRow);
	if ((selector == 1 || selector == 0xff) && (state.scene5020WoodenPlankTaken || hasInventoryItem(kScene5020WoodenPlankItem))) {
		if (_sceneChunkTable.isValidChunk(8))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5020TakenSceneItemId);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene5020::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x25, 3, 100, 20, 0x0b, 5, 100, 20);
}

void Scene5020::runExitToMineSwitches() {
	walkActiveActorTo(0x348, 0x186, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5020::runPickupWoodenPlank() {
	GameplayState &state = _vm->gameState();
	if (state.scene5020WoodenPlankTaken || hasInventoryItem(kScene5020WoodenPlankItem)) {
		beginSecondarySpeechLine(0, 0);
		return;
	}

	runHiddenActorActionOverlay(kScene5020PickupOverlayChunk, kScene5020PickupOverlayDescriptorCount,
		kScene5020PickupFrameMap, ARRAYSIZE(kScene5020PickupFrameMap), kScene5020OverlayFrameMillis);
	addInventoryItem(kScene5020WoodenPlankItem);
	_soundBank0.playSample(1, 100);
	state.scene5020WoodenPlankTaken = true;
	applySceneStateToHotspotsAndPatches(1);
}

void Scene5020::copyStageSmallRow(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene5020::clearSceneItemFromColorMap(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
