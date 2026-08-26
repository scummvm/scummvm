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

#include "hollywood/scenes/playable/scene5060.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene5010ReturnState = 0x1393;
const uint kScene5060ActorBankTableEntry = 0x0000;
const uint kScene5060ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5060SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5060FrameMillis = 75;
const uint32 kScene5060MineCartFrameMillis = 50;
const uint kScene5060MineCartDescriptorCount = 0x63;
const uint kScene5060RockPickupDescriptorCount = 0x0e;
const byte kScene5060RockSceneItem = 4;
const byte kScene5060RockInventoryItem = 0x4e;
const byte kScene5060GasSourceInventoryItem = 0x1c;
const byte kScene5060GasFilledInventoryItem = 0x4d;

const byte kScene5060RockPickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

static Common::Array<byte> sequentialFrameMap(uint frameCount) {
	Common::Array<byte> frameMap;
	frameMap.resize(frameCount);
	for (uint i = 0; i < frameMap.size(); ++i)
		frameMap[i] = (byte)i;
	return frameMap;
}

PlayableSceneConfig scene5060Config() {
	PlayableSceneConfig config(5060,
		SceneResourceLayout(5, 5, 8),
		SceneViewport(0),
		SceneActorPose(0x1fe, 0x17c, 4));
	config.setActorResources(kScene5060ActorBankTableEntry, kScene5060ActorPaletteTableEntry);
	config.setTextResources(0, kScene5060SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	return config;
}

Scene5060::Scene5060(HollywoodEngine *vm) :
		PlayableScene(vm, scene5060Config()) {
}

void Scene5060::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);

	setActiveActorPose(0x1fe, 0x17c, 4);
}

void Scene5060::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();
}

void Scene5060::runCustomEntrySequence() {
	setActiveActorPose(0x2d9, 0x19b, 4);
	drawPlayableComposite();
	presentFrame();

	runMineCartEntryClip();
	walkActiveActorTo(0x1fe, 0x17c, 4, 0, false);

	GameplayState &state = _vm->gameState();
	if (!state.scene5060EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5060EntryLineSeen = true;
	}
}

bool Scene5060::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene5060::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Usar túnel/vagoneta (use tunnel/cart): return to mine switches.
		runExitToMineSwitches();
		return true;
	case 302: // Mirar/oler grieta (look/smell crack): Ron notices gas.
		beginSecondarySpeechLine(1, 0);
		_vm->gameState().scene5060GasSmelled = true;
		return true;
	case 303: // Coger piedra (take rock): grants inventory item 0x4e.
		runRockPickup();
		return true;
	case 304: // Mirar piedra (look at rock): Ron calls it a good stone.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Usar globo/recipiente con grieta de gas: converts item 0x1c into item 0x4d.
		runGasInventoryAction();
		return true;
	case 306: // Usar globo ya hinchado con grieta de gas: it cannot be inflated further.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Usar objeto inadecuado con grieta de gas: not the right fuel.
		beginSecondarySpeechLine(5, 0);
		return true;
	default:
		return false;
	}
}

bool Scene5060::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if ((selector == 1 || selector == 0xff) && (state.scene5060RockTaken || hasInventoryItem(kScene5060RockInventoryItem))) {
		if (_sceneChunkTable.isValidChunk(8))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5060RockSceneItem);
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene5060::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 75, 25, 0x0b, 5, 100, 50);
}

void Scene5060::runMineCartEntryClip() {
	const Common::Array<byte> frameMap = sequentialFrameMap(kScene5060MineCartDescriptorCount);
	runActorReplacement(ActionOverlaySpec(5, kScene5060MineCartDescriptorCount,
		frameMap.data(), frameMap.size(), kScene5060MineCartFrameMillis)
		.soundAt(0x3c, 0x16));
}

void Scene5060::runExitToMineSwitches() {
	walkActiveActorTo(0x2d9, 0x19b, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5060::runRockPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene5060RockTaken || hasInventoryItem(kScene5060RockInventoryItem)) {
		beginSecondarySpeechLine(2, 0);
		return;
	}

	runActorReplacement(ActionOverlaySpec(7, kScene5060RockPickupDescriptorCount,
		kScene5060RockPickupFrameMap, ARRAYSIZE(kScene5060RockPickupFrameMap), kScene5060FrameMillis)
		.patchAt(6, 1));
	addInventoryItem(kScene5060RockInventoryItem);
	_soundBank0.playSample(1, 100);
	state.scene5060RockTaken = true;
	applySceneStateToHotspotsAndPatches(1);
}

void Scene5060::runGasInventoryAction() {
	GameplayState &state = _vm->gameState();
	if (!state.scene5060GasSmelled) {
		beginSecondarySpeechLine(1, 1);
		state.scene5060GasSmelled = true;
	}

	const byte sourceItem = _lastInventoryPrimaryItemId != 0 ? _lastInventoryPrimaryItemId : kScene5060GasSourceInventoryItem;
	if (sourceItem == kScene5060GasFilledInventoryItem || hasInventoryItem(kScene5060GasFilledInventoryItem)) {
		beginSecondarySpeechLine(4, 0);
		return;
	}
	if (sourceItem != kScene5060GasSourceInventoryItem || !hasInventoryItem(sourceItem)) {
		beginSecondarySpeechLine(5, 0);
		return;
	}

	beginSecondarySpeechLine(3, 0);
	removeInventoryItem(sourceItem);
	addInventoryItem(kScene5060GasFilledInventoryItem);
	_soundBank0.playSample(1, 100);
}

void Scene5060::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 2 && _walkablePaletteMask[i] < 8)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene5060::clearSceneItemFromColorMap(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
