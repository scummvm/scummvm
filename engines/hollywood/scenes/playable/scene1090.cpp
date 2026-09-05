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

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/scenes/playable/scene1090.h"

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

const byte kScene1090SwitchFrameMap[] = { 0, 0, 1, 2, 3, 4, 3, 2, 1, 0, 5 };
const int8 kScene1090LitActorPaletteDeltas[] = { -6, -6, -13, -18, -5, -23 };
const int8 kScene1090DarkActorPaletteDeltas[] = { -25, -20, -30, -35, -5, -40 };

PlayableSceneConfig scene1090Config() {
	PlayableSceneConfig config(1090,
		SceneResourceLayout(14, 5, 13),
		SceneViewport(kScene1090ViewportXOffset, kScene1090ViewportMinXOffset, kScene1090ViewportMaxXOffset),
		SceneActorPose(0x101, 0x15b, 2));
	config.setActorResources(kScene1090ActorBankTableEntry, kScene1090ActorPaletteTableEntry);
	config.setTextResources(kScene1090Resource003RowsOffsetIndex, kScene1090SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = true;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene1090::Scene1090(HollywoodEngine *vm) :
		PlayableScene(vm, scene1090Config()),
		_litPaletteResource() {
}

void Scene1090::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	setActiveActorPose(0x101, 0x15b, 2);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;
	if (!state.scene1090EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene1090EntryLineSeen = true;
	}
	drawPlayableComposite();
	presentFrame();
}

void Scene1090::runExitSideEffectsAfterLoop() {
	if (didLeaveSceneAfterLoop())
		fadePaletteToBlack();
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
		beginSecondarySpeechLine(2, _vm->gameState().scene1090LightsOff ? 0 : 1);
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

bool Scene1090::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x76, 0x1ac);
	targetY = CLIP<int>(targetY, 0, 0x1df);
	while (targetY < 0x1df) {
		++targetY;
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
	}
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;
	return true;
}

bool Scene1090::customizeRouteSegment(byte currentRegion, byte nextRegion,
		const ActorPathBuildState &state, const ScenePoint &boundary,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;
	(void)restoredStepDeltas;
	if (currentRegion == 2) {
		if (nextRegion == 1)
			requestedFacing = 2;
		else if (nextRegion > 2)
			requestedFacing = 5;
	}
	return requestedFacing >= 0;
}

bool Scene1090::customizeRouteFinal(byte currentRegion, byte targetRegion,
		const ActorPathBuildState &state, int targetX, int targetY,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)currentRegion;
	(void)state;
	(void)targetX;
	(void)targetY;
	(void)restoredStepDeltas;
	if (targetRegion == 4)
		requestedFacing = 1;
	return requestedFacing >= 0;
}

bool Scene1090::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	if (selector == 0xff) {
		rememberLitResourceSet();
		applyActiveLightResourceSet();
		rebuildPantryWalkableMask();
	}
	if (selector == 1 || selector == 0xff) {
		applyActorLightPaletteAdjustments();
		updateActorPaletteForWorldPoint(_activeActorWorldX, _activeActorWorldY);
	}
	if (selector == 2 || selector == 0xff) {
		applyWrappedBrainPatch();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}
	return true;
}

AmbientAudioProfile Scene1090::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
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

void Scene1090::rememberLitResourceSet() {
	if (!_litPaletteResource.empty())
		return;

	_litPaletteResource = _paletteResource;
}

void Scene1090::applyActiveLightResourceSet() {
	if (_vm->gameState().scene1090LightsOff) {
		copyArenaChunkToFramebuffer(5);
		copyArenaChunkToFixedArray(6, _paletteResource, kPaletteSize);
		copyArenaChunkToVariableArray(7, _paletteMask);
	} else {
		restoreBaseFramebufferFromOriginal();
		if (!_litPaletteResource.empty())
			_paletteResource = _litPaletteResource;
		_paletteMask = _paletteMaskOriginal;
	}

	if (_paletteMask.size() >= kScenePaletteMapPageSize)
		memcpy(_fullPaletteRegionMask.data(), _paletteMask.data(), _fullPaletteRegionMask.size());
	if (_paletteMask.size() >= kSceneColorToActorDepthClassMap + kScenePaletteMapPageSize)
		memcpy(_colorToActorDepthClassMap.data(),
			_paletteMask.data() + kSceneColorToActorDepthClassMap, _colorToActorDepthClassMap.size());

	_paletteCurrent = _paletteResource;
	loadResource000ActorPalette(_resource000OffsetTable);
	_panelArt.applyInteractiveObjectPalette(_paletteCurrent);
	if (_paletteMask.size() >= kScenePresentationPaletteRemapMap + kScenePaletteMapPageSize) {
		memcpy(_presentationPaletteRemapTable.data(),
			_paletteMask.data() + kScenePresentationPaletteRemapMap, _presentationPaletteRemapTable.size());
	} else {
		_surfaceState.rebuildPresentationPaletteRemapTable();
	}
}

void Scene1090::applyActorLightPaletteAdjustments() {
	if (_metadata.size() < kPaletteAdjustTable + 6)
		return;

	const bool lightsOff = _vm->gameState().scene1090LightsOff;
	const int8 *deltas = lightsOff ? kScene1090DarkActorPaletteDeltas : kScene1090LitActorPaletteDeltas;
	for (uint i = 0; i < ARRAYSIZE(kScene1090LitActorPaletteDeltas); ++i)
		_metadata[kPaletteDeltaTable + i + 1] = (byte)deltas[i];
	_metadata[kPaletteAdjustTable + 2] = lightsOff ? 2 : 1;
	_metadata[kPaletteAdjustTable + 3] = (byte)-12;
	_metadata[kPaletteAdjustTable + 4] = 2;
	_metadata[kPaletteAdjustTable + 5] = (byte)-12;
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

	const bool hotspotActive = !_vm->gameState().scene1090LightsOff &&
		_vm->gameState().scene1090WrappedBrainState == 0;
	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_paletteMaskOriginal[kSceneColorToItemMap + i] == 4)
			_paletteMask[kSceneColorToItemMap + i] = hotspotActive ? 4 : 0;
	}

	const uint patchChunk = _vm->gameState().scene1090WrappedBrainState != 0 ?
		(_vm->gameState().scene1090LightsOff ? 12 : 10) :
		(_vm->gameState().scene1090LightsOff ? 11 : 9);
	if (_sceneChunkTable.isValidChunk(patchChunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[patchChunk], _baseFramebuffer);
}

void Scene1090::runSwitchAction() {
	GameplayState &state = _vm->gameState();
	runActorReplacement(ActionOverlaySpec(8, kScene1090SwitchDescriptorCount,
		kScene1090SwitchFrameMap, ARRAYSIZE(kScene1090SwitchFrameMap), kScene1090FrameMillis)
		.commitAt(6, state.scene1090LightsOff, !state.scene1090LightsOff)
		.patchAt(6, 0xff)
		.invalidatePaletteAt(6)
		.noFinalFrameDelay()
		.unskippable());
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
	if (animationPlaybackShouldStop())
		return;

	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(13, kScene1090WrappedBrainPickupDescriptorCount,
			kScene1090FrameMillis)
			.holdFirstFrame()
			.resourcePatchAt(4, 10)
			.noFinalFrameDelay()
			.unskippable());
	if (!sequence.completed())
		return;

	sequence.commit(state.scene1090WrappedBrainState, (byte)2)
		.framebufferPatch(2);
	addInventoryItem(0x25);
	sequence.sound(1);
	if (sequence.completed())
		beginStaticSecondarySpeechLine(0x73, 0);
}

} // End of namespace Hollywood
