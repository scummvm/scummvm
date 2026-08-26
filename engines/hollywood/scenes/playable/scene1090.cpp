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

#include "hollywood/scenes/playable/scene1090.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene1090ExitStateKitchen = 0x0439;
const uint16 kScene1090ExitStateBallroom = 0x042f;
const uint16 kScene1090ViewportXOffset = 0x0028;
const uint16 kScene1090ViewportMinXOffset = 0x0028;
const uint16 kScene1090ViewportMaxXOffset = 0x0028;
const uint kScene1090ActorBankTableEntry = 0x0000;
const uint kScene1090ActorPaletteTableEntry = 0x00cc;
const uint kScene1090Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1090SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene1090FrameMillis = 75;
const uint kScene1090SwitchDescriptorCount = 6;
const uint kScene1090WrappedBrainPickupDescriptorCount = 0x0e;
const byte kScene1090LightSwitchHook = 1;

const byte kScene1090SwitchFrameMap[] = { 0, 1, 2, 3, 4, 3, 2, 1, 0, 5 };
const byte kScene1090WrappedBrainPickupFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

static PlayableSceneConfig scene1090Config() {
	PlayableSceneConfig config(1090,
		SceneResourceLayout(14, 5, 13),
		SceneViewport(kScene1090ViewportXOffset, kScene1090ViewportMinXOffset, kScene1090ViewportMaxXOffset),
		SceneActorPose(0x101, 0x15b, 2));
	config.setActorResources(kScene1090ActorBankTableEntry, kScene1090ActorPaletteTableEntry);
	config.setTextResources(kScene1090Resource003RowsOffsetIndex, kScene1090SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	return config;
}

Scene1090::Scene1090(HollywoodEngine *vm) :
		PlayableScene(vm, scene1090Config()),
		_darkPaletteResource(),
		_darkPaletteMask() {
}

void Scene1090::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
}

void Scene1090::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	runEntryPath(0x101, 0x15b, 2, 0x101, 0x15b);
	if (!state.scene1090EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene1090EntryLineSeen = true;
	}
	drawPlayableComposite();
	presentFrame();
}

bool Scene1090::prepareCustomGameplayLoop() {
	return true;
}

bool Scene1090::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene1090::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a cocina (go to kitchen).
		_vm->gameState().mainFlowStateId = kScene1090ExitStateKitchen;
		return true;
	case 302: // Mirar cocina (look at kitchen).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar interruptor (look at light switch).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Usar interruptor (use light switch).
		runSwitchAction();
		return true;
	case 305: // Ir a escaleras (go to stairs).
		_vm->gameState().mainFlowStateId = kScene1090ExitStateBallroom;
		return true;
	case 306: // Mirar escaleras (look at stairs).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 307: // Coger objeto envuelto en un trapo (take object wrapped in cloth).
		takeWrappedBrain();
		return true;
	case 308: // Mirar objeto envuelto en un trapo (look at object wrapped in cloth).
		revealWrappedBrain();
		return true;
	default:
		return false;
	}
}

bool Scene1090::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	rememberDarkResourceSet();
	applyActiveLightResourceSet();
	applyWrappedBrainPatch();
	rebuildPantryWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

void Scene1090::handleAnimationFrameHook(byte hookId, uint frame) {
	(void)frame;
	if (hookId != kScene1090LightSwitchHook)
		return;

	_vm->gameState().scene1090LightsOn = !_vm->gameState().scene1090LightsOn;
	applySceneStateToHotspotsAndPatches(1);
	invalidatePresentationPalette();
}

AmbientAudioProfile Scene1090::ambientAudioProfile() const {
	return createLoopingAmbientAudioProfile(100);
}

bool Scene1090::copyArenaChunkToFramebuffer(uint chunkIndex) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return false;

	const uint32 offset = _resourceChunkOffsets[chunkIndex];
	const uint32 chunkSize = _sceneChunkTable.sizes[chunkIndex];
	if (offset + chunkSize > _resourceArena.size())
		return false;

	byte *pixels = framebufferPixels(_baseFramebuffer);
	memset(pixels, 0, framebufferByteCount());
	memcpy(pixels, _resourceArena.data() + offset, MIN<uint32>(chunkSize, framebufferByteCount()));
	return true;
}

bool Scene1090::copyArenaChunkToFixedArray(uint chunkIndex, Common::Array<byte> &destination, uint destinationSize) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return false;

	const uint32 offset = _resourceChunkOffsets[chunkIndex];
	const uint32 chunkSize = _sceneChunkTable.sizes[chunkIndex];
	if (offset + chunkSize > _resourceArena.size())
		return false;

	destination.resize(destinationSize);
	memset(destination.data(), 0, destination.size());
	memcpy(destination.data(), _resourceArena.data() + offset, MIN<uint32>(chunkSize, destination.size()));
	return true;
}

bool Scene1090::copyArenaChunkToVariableArray(uint chunkIndex, Common::Array<byte> &destination) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return false;

	const uint32 offset = _resourceChunkOffsets[chunkIndex];
	const uint32 chunkSize = _sceneChunkTable.sizes[chunkIndex];
	if (offset + chunkSize > _resourceArena.size())
		return false;

	destination.resize(chunkSize);
	memcpy(destination.data(), _resourceArena.data() + offset, chunkSize);
	return true;
}

void Scene1090::rememberDarkResourceSet() {
	if (!_darkPaletteResource.empty())
		return;

	_darkPaletteResource = _paletteResource;
	_darkPaletteMask = _paletteMaskOriginal;
}

void Scene1090::applyActiveLightResourceSet() {
	if (_vm->gameState().scene1090LightsOn) {
		copyArenaChunkToFramebuffer(5);
		copyArenaChunkToFixedArray(6, _paletteResource, kPaletteSize);
		copyArenaChunkToVariableArray(7, _paletteMask);
	} else {
		restoreBaseFramebufferFromOriginal();
		if (!_darkPaletteResource.empty())
			_paletteResource = _darkPaletteResource;
		if (!_darkPaletteMask.empty())
			_paletteMask = _darkPaletteMask;
	}

	if (_paletteMask.size() >= kScenePaletteMapPageSize)
		memcpy(_fullPaletteRegionMask.data(), _paletteMask.data(), _fullPaletteRegionMask.size());
	if (_paletteMask.size() >= kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize)
		memcpy(_colorToActorDepthClassMap.data(),
			_paletteMask.data() + kSceneColorToActorDepthClassMap, _colorToActorDepthClassMap.size());

	_paletteCurrent = _paletteResource;
	loadResource000ActorPalette(_resource000OffsetTable);
	_panelArt.applyInteractiveObjectPalette(_paletteCurrent);
	_surfaceState.rebuildPresentationPaletteRemapTable();
}

void Scene1090::rebuildPantryWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > walkablePaletteMaxRegion())
			_walkablePaletteMask[i] = 0;
	}
}

void Scene1090::applyWrappedBrainPatch() {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	const bool hotspotActive = !_vm->gameState().scene1090LightsOn &&
		_vm->gameState().scene1090WrappedBrainState == 0;
	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMask[kSceneColorToItemMap + i] == 4)
			_paletteMask[kSceneColorToItemMap + i] = hotspotActive ? 4 : 0;
	}

	const uint patchChunk = _vm->gameState().scene1090WrappedBrainState != 0 ?
		(_vm->gameState().scene1090LightsOn ? 12 : 10) :
		(_vm->gameState().scene1090LightsOn ? 11 : 9);
	if (_sceneChunkTable.isValidChunk(patchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[patchChunk], _baseFramebuffer);
}

void Scene1090::runSwitchAction() {
	runActorReplacement(ActionOverlaySpec(8, kScene1090SwitchDescriptorCount,
		kScene1090SwitchFrameMap, ARRAYSIZE(kScene1090SwitchFrameMap), kScene1090FrameMillis)
		.hookAt(5, kScene1090LightSwitchHook));
}

void Scene1090::revealWrappedBrain() {
	beginSecondarySpeechLine(4, 0);
	if (_vm->gameState().scene1090WrappedBrainState == 0)
		_vm->gameState().scene1090WrappedBrainState = 1;
}

void Scene1090::takeWrappedBrain() {
	GameplayState &state = _vm->gameState();
	if (state.scene1090WrappedBrainState >= 2 || hasInventoryItem(0x25))
		return;

	if (state.scene1090WrappedBrainState == 0)
		revealWrappedBrain();
	runActorReplacement(13, kScene1090WrappedBrainPickupDescriptorCount,
		kScene1090WrappedBrainPickupFrameMap, ARRAYSIZE(kScene1090WrappedBrainPickupFrameMap),
		kScene1090FrameMillis);
	state.scene1090WrappedBrainState = 2;
	applySceneStateToHotspotsAndPatches(2);
	addInventoryItem(0x25);
	_soundBank0.playSample(1, 100);
	beginStaticSecondarySpeechLine(0x73, 0);
}

} // End of namespace Hollywood
