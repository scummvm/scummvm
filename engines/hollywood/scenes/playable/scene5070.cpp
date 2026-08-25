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

#include "hollywood/scenes/playable/scene5070.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene5070ArchiveName = "RESOURCE.E07";
const char *const kScene5070MusicArchiveName = "RESOURCE.M05";
const char *const kScene5070SoundArchiveName = "RESOURCE.S05";
const uint kScene5070InitialRequiredChunkCount = 5;
const uint kScene5070ArenaFirstChunk = 5;
const uint kScene5070ArenaLastChunk = 11;
const uint kScene5070StageIndex = 507;
const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5070ViewportXOffset = 0x00a0;
const uint16 kScene5070ViewportMaxXOffset = 0x00c8;
const uint kScene5070ActorBankTableEntry = 0x0000;
const uint kScene5070ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5070FrameMillis = 75;
const uint32 kScene5070MineCartFrameMillis = 50;
const uint kScene5070MineCartDescriptorCount = 0x30;
const uint kScene5070AviatorCapPickupDescriptorCount = 0x0c;
const uint kScene5070ShovelPickupDescriptorCount = 0x0d;
const byte kScene5070ShovelSceneItem = 2;
const byte kScene5070HangingSceneItem = 6;
const byte kScene5070AviatorCapUnidentifiedState = 1;
const byte kScene5070AviatorCapIdentifiedState = 2;
const byte kScene5070AviatorCapTakenState = 0;
const byte kScene5070AviatorCapInventoryItem = 0x4f;
const byte kScene5070ShovelInventoryItem = 0x50;
const byte kScene5070AviatorCapNameRow = 7;
const byte kScene5070HangingItemNameRow = 6;
const uint kScene5070HangingItemVerbRecordIndex = 0x34;

const byte kScene5070ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene5070AviatorCapPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const byte kScene5070ShovelPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

static Common::Array<byte> sequentialFrameMap(uint frameCount) {
	Common::Array<byte> frameMap;
	frameMap.resize(frameCount);
	for (uint i = 0; i < frameMap.size(); ++i)
		frameMap[i] = (byte)i;
	return frameMap;
}

PlayableSceneConfig scene5070Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene5070ArchiveName;
	config.initialRequiredChunkCount = kScene5070InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene5070ArenaFirstChunk;
	config.arenaLastChunk = kScene5070ArenaLastChunk;
	config.stageIndex = kScene5070StageIndex;
	config.debugName = "Scene 5070";
	config.viewportXOffset = kScene5070ViewportXOffset;
	config.viewportMinXOffset = kScene5070ViewportXOffset;
	config.viewportMaxXOffset = kScene5070ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 5;
	config.actorBankTableEntry = kScene5070ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene5070ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = 0;
	config.speechCueDescriptorTableOffset = kScene5070SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene5070ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene5070ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 3;
	config.musicArchiveName = kScene5070MusicArchiveName;
	config.soundBank0ArchiveName = kScene5070SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = false;
	return config;
}

Scene5070::Scene5070(HollywoodEngine *vm) :
		PlayableScene(vm, scene5070Config(), "scene5070", 0x2e3, 0x1d6, 5, 0xfd, 0xfb) {
}

void Scene5070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);

	setActiveActorPose(0x2e3, 0x1d6, 5);
}

void Scene5070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();
	drawHangingItemForeground();
}

void Scene5070::runCustomEntrySequence() {
	setActiveActorPose(0x3ab, 0x1df, 4);
	drawPlayableComposite();
	presentFrame();

	runMineCartEntryClip();
	setActiveActorPose(0x2e3, 0x1d6, 5);
	drawPlayableComposite();
	presentFrame();
}

bool Scene5070::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene5070::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();

	switch (handlerId) {
	case 301: // Ir a vagoneta/salida (go to cart/exit): return to mine switches.
		runExitToMineSwitches();
		return true;
	case 302: // Coger pala (take shovel): grants inventory item 0x50.
		runShovelPickup();
		return true;
	case 303: // Mirar pala/túnel cercano (look at shovel/near tunnel).
		beginSecondarySpeechLine(0, 0);
		return true;
	case 304: // Ir a/abrir túnel o caseta (go/open tunnel or shed): Ron refuses to leave the cart.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 305: // Mirar algo colgado/caseta (look at hanging object/shed): identifies the aviator cap.
		beginSecondarySpeechLine(2, 0);
		if (state.scene5070AviatorCapState == kScene5070AviatorCapUnidentifiedState) {
			state.scene5070AviatorCapState = kScene5070AviatorCapIdentifiedState;
			applySceneStateToHotspotsAndPatches(1);
		}
		return true;
	case 306: // Coger gorro de aviador (take aviator cap): grants inventory item 0x4f.
		runAviatorCapPickup();
		return true;
	case 307: // Mirar gorro de aviador (look at aviator cap).
		beginSecondarySpeechLine(3, 0);
		return true;
	default:
		return false;
	}
}

bool Scene5070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene5070AviatorCapState > kScene5070AviatorCapIdentifiedState)
		state.scene5070AviatorCapState = kScene5070AviatorCapUnidentifiedState;

	if ((selector == 0 || selector == 0xff) && (state.scene5070ShovelTaken || hasInventoryItem(kScene5070ShovelInventoryItem))) {
		if (_sceneChunkTable.isValidChunk(6))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5070ShovelSceneItem);
	}
	if (selector == 1 || selector == 0xff) {
		if (state.scene5070AviatorCapState == kScene5070AviatorCapTakenState || hasInventoryItem(kScene5070AviatorCapInventoryItem)) {
			if (_sceneChunkTable.isValidChunk(5))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
			clearSceneItemFromColorMap(kScene5070HangingSceneItem);
		} else if (state.scene5070AviatorCapState == kScene5070AviatorCapIdentifiedState) {
			copyStageSmallRow(kScene5070HangingItemNameRow, kScene5070AviatorCapNameRow);
		}
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (state.scene5070AviatorCapState == kScene5070AviatorCapIdentifiedState)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(kScene5070HangingItemVerbRecordIndex, 0);
	return true;
}

AmbientAudioProfile Scene5070::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 75, 25, 0x0b, 5, 100, 50);
}

void Scene5070::runMineCartEntryClip() {
	const Common::Array<byte> frameMap = sequentialFrameMap(kScene5070MineCartDescriptorCount);
	runActionOverlay(ActionOverlaySpec(9, kScene5070MineCartDescriptorCount,
		frameMap.data(), frameMap.size(), kScene5070MineCartFrameMillis)
		.hideActor()
		.soundAt(0x23, 0x16));
}

void Scene5070::runExitToMineSwitches() {
	walkActiveActorTo(0x3ab, 0x1df, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5070::runShovelPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5070ShovelTaken || hasInventoryItem(kScene5070ShovelInventoryItem)) {
		beginSecondarySpeechLine(0, 0);
		return;
	}

	runActionOverlay(ActionOverlaySpec(8, kScene5070ShovelPickupDescriptorCount,
		kScene5070ShovelPickupFrameMap, ARRAYSIZE(kScene5070ShovelPickupFrameMap), kScene5070FrameMillis)
		.hideActor()
		.patchAt(6, 0));
	addInventoryItem(kScene5070ShovelInventoryItem);
	_soundBank0.playSample(1, 100);
	state.scene5070ShovelTaken = true;
	applySceneStateToHotspotsAndPatches(0);
}

void Scene5070::runAviatorCapPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5070AviatorCapState == kScene5070AviatorCapTakenState || hasInventoryItem(kScene5070AviatorCapInventoryItem)) {
		beginSecondarySpeechLine(3, 0);
		return;
	}

	runActionOverlay(ActionOverlaySpec(7, kScene5070AviatorCapPickupDescriptorCount,
		kScene5070AviatorCapPickupFrameMap, ARRAYSIZE(kScene5070AviatorCapPickupFrameMap), kScene5070FrameMillis)
		.hideActor());
	addInventoryItem(kScene5070AviatorCapInventoryItem);
	_soundBank0.playSample(1, 100);
	state.scene5070AviatorCapState = kScene5070AviatorCapTakenState;
	applySceneStateToHotspotsAndPatches(1);
}

void Scene5070::drawHangingItemForeground() {
	const byte state = _vm->gameState().scene5070AviatorCapState;
	const uint chunkIndex = (state == kScene5070AviatorCapTakenState ||
		hasInventoryItem(kScene5070AviatorCapInventoryItem)) ? 11 : 10;
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

void Scene5070::copyStageSmallRow(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene5070::clearSceneItemFromColorMap(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
