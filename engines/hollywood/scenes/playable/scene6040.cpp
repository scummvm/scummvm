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

#include "hollywood/scenes/playable/scene6040.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene6040ArchiveName = "RESOURCE.F04";
const char *const kScene6040MusicArchiveName = "RESOURCE.M06";
const char *const kScene6040SoundArchiveName = "RESOURCE.S06";
const uint kScene6040InitialRequiredChunkCount = 14;
const uint kScene6040ArenaFirstChunk = 5;
const uint kScene6040ArenaLastChunk = 13;
const uint kScene6040StageIndex = 604;
const uint16 kScene6040FirstState = 0x1798;
const uint16 kScene6040LastState = 0x17a1;
const uint16 kScene6010ReturnFromScene6040State = 0x177c;
const uint16 kScene6040ViewportXOffset = 0x00c8;
const uint16 kScene6040ViewportMaxXOffset = 0x0110;
const uint kScene6040ActorBankTableEntry = 0x0000;
const uint kScene6040ActorPaletteTableEntry = 0x00cc;
const uint kScene6040Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene6040SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene6040FrameMillis = 75;
const uint32 kScene6040LeftToggleMillis = 500;
const uint32 kScene6040RightToggleMillis = 625;
const uint kScene6040ToggleDescriptorCount = 2;
const uint kScene6040WireOverlayDescriptorCount = 12;
const uint kScene6040PaintOverlayDescriptorCount = 10;
const byte kScene6040PaintInventoryItem = 0x60;
const byte kScene6040LooseWireInventoryItem = 0x61;
const byte kScene6040CutWireInventoryItem = 0x5f;

const byte kScene6040ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene6040ToggleFrameMap[] = { 0, 1 };

const byte kScene6040PaintPickupFrameMap[] = {
	9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};

const byte kScene6040WireInspectFrameMap[] = {
	0x0b, 0, 1, 2, 3, 4, 3, 2, 3, 4,
	3, 2, 3, 4, 3, 2, 1, 0, 0x0b
};

const byte kScene6040WireCutFrameMap[] = {
	0x0b, 10, 8, 7, 6, 5, 4, 3, 2, 3, 4, 3, 2,
	3, 4, 3, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0x0b
};

const byte kScene6040WirePickupFrameMap[] = {
	0x0b, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0x0b
};

static PlayableSceneConfig scene6040Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene6040ArchiveName;
	config.initialRequiredChunkCount = kScene6040InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene6040ArenaFirstChunk;
	config.arenaLastChunk = kScene6040ArenaLastChunk;
	config.stageIndex = kScene6040StageIndex;
	config.debugName = "Scene 6040";
	config.viewportXOffset = kScene6040ViewportXOffset;
	config.viewportMinXOffset = kScene6040ViewportXOffset;
	config.viewportMaxXOffset = kScene6040ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 6;
	config.actorBankTableEntry = kScene6040ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene6040ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene6040Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene6040SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene6040ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene6040ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene6040MusicArchiveName;
	config.soundBank0ArchiveName = kScene6040SoundArchiveName;
	config.mainFlowFirstState = kScene6040FirstState;
	config.mainFlowLastState = kScene6040LastState;
	return config;
}

Scene6040::Scene6040(HollywoodEngine *vm) :
		PlayableScene(vm, scene6040Config(), "scene6040", 0x172, 0x1c2, 2, 0xfd, 0xfb),
		_originalColorToItemMap(),
		_leftToggleChannel(),
		_rightToggleChannel(),
		_leftToggleLayer(),
		_rightToggleLayer() {
	_leftToggleLayer.configure(12, kScene6040ToggleDescriptorCount,
		kScene6040ToggleFrameMap, ARRAYSIZE(kScene6040ToggleFrameMap));
	_rightToggleLayer.configure(13, kScene6040ToggleDescriptorCount,
		kScene6040ToggleFrameMap, ARRAYSIZE(kScene6040ToggleFrameMap));
}

bool Scene6040::hasCustomPreviewState() const {
	return true;
}

void Scene6040::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	_activeActorWorldX = 0x172;
	_activeActorWorldY = 0x1c2;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene6040::hasCustomComposite() const {
	return true;
}

void Scene6040::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_leftToggleLayer);
	drawResourceSpriteLayer(_rightToggleLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldY);
}

bool Scene6040::hasCustomEntrySequence() const {
	return true;
}

bool Scene6040::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene6040::runCustomEntrySequence() {
	_activeActorWorldX = 100;
	_activeActorWorldY = 0x1dc;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;

	runEntryPath(100, 0x1dc, 2, 0x172, 0x1c2);
	if (!_vm->gameState().scene6040EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene6040EntryLineSeen = true;
	}
}

bool Scene6040::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene6040::advanceCustomGameplayLoop(uint32 delta) {
	advanceToggleLayer(_leftToggleChannel, _leftToggleLayer, delta);
	advanceToggleLayer(_rightToggleChannel, _rightToggleLayer, delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene6040::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 205: // Mirar tapa/resto pequeno (look at small remnant): Ron inventory-style description.
		beginStaticSecondarySpeechLine(0xc0, 0);
		return true;
	case 301: // Ir a exterior (go to exterior): return to the studio lot.
		_vm->gameState().mainFlowStateId = kScene6010ReturnFromScene6040State;
		return true;
	case 302: // Mirar decorado (look at dinosaur set).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar camara de 16 mm (look at 16mm camera).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Coger/usar camara alta (take/use high camera): unreachable.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar camara en grua (look at camera on crane).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar baules y cajas (look at trunks and boxes).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Abrir baules y cajas (open trunks and boxes).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Coger/usar hacha prehistorica/cachiporras (take/use prop weapons).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Mirar hacha prehistorica/cachiporras (look at prop weapons).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Coger bote de pintura (take paint can).
		runPaintCanPickup();
		return true;
	case 311: // Mirar bote de pintura (look at paint can).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 312: // Coger alambre (take wire): cannot detach it yet.
		runWireInspectionAnimation();
		return true;
	case 313: // Mirar alambre (look at wire).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 314: // Usar alambre (use wire with hands): too stiff to bend.
		beginSecondarySpeechLine(12, 0);
		return true;
	case 315: // Usar herramienta cortante con alambre (use cutting tool with wire).
		runCutWireWithTool();
		return true;
	case 316: // Coger trozo de alambre suelto (take loosened wire piece).
		runWirePickup();
		return true;
	default:
		return false;
	}
}

bool Scene6040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	rememberOriginalColorMap();
	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (state.scene6040PaintCanTaken || hasInventoryItem(kScene6040PaintInventoryItem)) {
		state.scene6040PaintCanTaken = true;
		if (_sceneChunkTable.isValidChunk(8))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		replaceColorMapItemFromOriginal(12, 5);
		replaceColorMapItemFromOriginal(9, 0);
	} else {
		replaceColorMapItemFromOriginal(12, 9);
	}

	if (state.scene6040WireState == 0) {
		replaceColorMapItemFromOriginal(10, 11);
	} else if (state.scene6040WireState == 1) {
		if (_sceneChunkTable.isValidChunk(9))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
		replaceColorMapItemFromOriginal(10, 10);
		replaceColorMapItemFromOriginal(11, 0);
	} else {
		state.scene6040WireState = 2;
		if (_sceneChunkTable.isValidChunk(7))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _baseFramebuffer);
		replaceColorMapItemFromOriginal(10, 0);
		replaceColorMapItemFromOriginal(11, 0);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene6040::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene6040::resetAnimationLayers() {
	_leftToggleChannel.reset(0, kScene6040LeftToggleMillis);
	_rightToggleChannel.reset(0, kScene6040RightToggleMillis);
	_leftToggleLayer.reset(0);
	_rightToggleLayer.reset(0);
	_leftToggleLayer.visible = true;
	_rightToggleLayer.visible = true;
}

void Scene6040::advanceToggleLayer(TimedAnimationChannel &channel, ResourceSpriteLayer &layer, uint32 delta) {
	const uint frameCount = channel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i)
		layer.setFrame(layer.frameIndex == 0 ? 1 : 0);
}

void Scene6040::drawForegroundBlocks(int activeWorldY) {
	if (activeWorldY < 0x1a1 && _sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

void Scene6040::runPaintCanPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene6040PaintCanTaken || hasInventoryItem(kScene6040PaintInventoryItem)) {
		dispatchGenericSceneAction(6);
		return;
	}

	state.scene6040PaintCanTaken = true;
	runActionOverlay(ActionOverlaySpec(11, kScene6040PaintOverlayDescriptorCount,
		kScene6040PaintPickupFrameMap, ARRAYSIZE(kScene6040PaintPickupFrameMap), kScene6040FrameMillis)
		.hideActor()
		.patchAt(5, 1));
	addInventoryItem(kScene6040PaintInventoryItem);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene6040::runWireInspectionAnimation() {
	runActionOverlay(ActionOverlaySpec(10, kScene6040WireOverlayDescriptorCount,
		kScene6040WireInspectFrameMap, ARRAYSIZE(kScene6040WireInspectFrameMap), kScene6040FrameMillis)
		.hideActor());
	beginSecondarySpeechLine(10, 0);
}

void Scene6040::runCutWireWithTool() {
	GameplayState &state = _vm->gameState();
	if (state.scene6040WireState != 1) {
		beginSecondarySpeechLine(2, 0);
		return;
	}

	beginSecondarySpeechLine(12, 0);
	state.scene6040WireState = 2;
	runActionOverlay(ActionOverlaySpec(10, kScene6040WireOverlayDescriptorCount,
		kScene6040WireCutFrameMap, ARRAYSIZE(kScene6040WireCutFrameMap), kScene6040FrameMillis)
		.hideActor()
		.patchAt(19, 2));
	removeInventoryItem(kScene6040LooseWireInventoryItem);
	addInventoryItem(kScene6040CutWireInventoryItem);
	_soundBank0.playSample(1, 100);
}

void Scene6040::runWirePickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene6040WireState != 0 || hasInventoryItem(kScene6040LooseWireInventoryItem)) {
		dispatchGenericSceneAction(6);
		return;
	}

	state.scene6040WireState = 1;
	runActionOverlay(ActionOverlaySpec(10, kScene6040WireOverlayDescriptorCount,
		kScene6040WirePickupFrameMap, ARRAYSIZE(kScene6040WirePickupFrameMap), kScene6040FrameMillis)
		.hideActor()
		.patchAt(6, 2));
	addInventoryItem(kScene6040LooseWireInventoryItem);
	_soundBank0.playSample(1, 100);
	dispatchGenericSceneAction(21);
}

void Scene6040::rememberOriginalColorMap() {
	if (!_originalColorToItemMap.empty() ||
			_paletteMaskOriginal.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	_originalColorToItemMap.resize(kScenePaletteMapPageSize);
	memcpy(_originalColorToItemMap.data(), _paletteMaskOriginal.data() + kSceneColorToItemMap,
		_originalColorToItemMap.size());
}

void Scene6040::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_originalColorToItemMap.size() < kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_originalColorToItemMap[i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

} // End of namespace Hollywood
