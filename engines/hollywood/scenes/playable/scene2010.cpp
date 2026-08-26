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

#include "hollywood/scenes/playable/scene2010.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene2010EntryFromB02State = 0x07db;
const uint16 kScene2010PatchedEntryState = 0x07dc;
const uint16 kScene2020EntryFromPyramidExteriorState = 0x07e4;
const uint16 kScene2030EntryFromPyramidState = 0x07ef;
const uint16 kScene2100EntryState = 0x0834;
const uint16 kScene2010ViewportXOffset = 0x00e8;
const uint kScene2010ActorBankTableEntry = 0x0000;
const uint kScene2010ActorPaletteTableEntry = 0x00cc;
const uint kScene2010Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2010SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2010OverlayFrameMillis = 75;
const uint kScene2010FirstOverlayDescriptorCount = 0x0a;
const uint kScene2010SecondOverlayDescriptorCount = 0x10;
const uint kScene2010ActorOverlayDescriptorCount = 0x1c;

const byte kScene2010FirstOverlayFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kScene2010SecondOverlayFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

const byte kScene2010ActorOverlayFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14,
	14, 14, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27
};

static_assert(ARRAYSIZE(kScene2010FirstOverlayFrameMap) == 19, "Scene 2010 anilla first overlay frame map size changed");
static_assert(ARRAYSIZE(kScene2010SecondOverlayFrameMap) == 16, "Scene 2010 anilla second overlay frame map size changed");
static_assert(ARRAYSIZE(kScene2010ActorOverlayFrameMap) == 32, "Scene 2010 anilla actor overlay frame map size changed");
static_assert(kScene2010FirstOverlayDescriptorCount > 9, "Scene 2010 anilla first overlay descriptor count is too small");
static_assert(kScene2010SecondOverlayDescriptorCount > 15, "Scene 2010 anilla second overlay descriptor count is too small");
static_assert(kScene2010ActorOverlayDescriptorCount > 27, "Scene 2010 anilla actor overlay descriptor count is too small");

static PlayableSceneConfig scene2010Config() {
	PlayableSceneConfig config(2010,
		SceneResourceLayout(11, 5, 10),
		SceneViewport(kScene2010ViewportXOffset, kScene2010ViewportXOffset, kScene2010ViewportXOffset),
		SceneActorPose(0x2e9, 0x1a0, 4));
	config.setActorResources(kScene2010ActorBankTableEntry, kScene2010ActorPaletteTableEntry);
	config.setTextResources(kScene2010Resource003RowsOffsetIndex, kScene2010SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 6;
	config.useActorDepthTest = true;
	return config;
}

Scene2010::Scene2010(HollywoodEngine *vm) :
		PlayableScene(vm, scene2010Config()) {
}

void Scene2010::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);

	switch (_vm->gameState().mainFlowStateId) {
	case kScene2010EntryFromB02State:
		_activeActorWorldX = 0x28a;
		_activeActorWorldY = 0x19a;
		_activeActorFacing = 1;
		break;
	case kScene2010PatchedEntryState:
		_activeActorWorldX = 0x21c;
		_activeActorWorldY = 0x126;
		_activeActorFacing = 2;
		break;
	default:
		_activeActorWorldX = 0x2e9;
		_activeActorWorldY = 0x1a0;
		_activeActorFacing = 4;
		break;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene2010::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
}

void Scene2010::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene2010EntryFromB02State:
		runEntryFromB02();
		break;
	case kScene2010PatchedEntryState:
		runPatchedEntrySequence();
		break;
	default:
		runEntryFromMarket();
		break;
	}
}

bool Scene2010::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2010::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();

	switch (handlerId) {
	case 301: // Ir al mercado egipcio (go to Egyptian market): return to scene 2030.
		state.mainFlowStateId = kScene2030EntryFromPyramidState;
		return true;
	case 302: // Mirar mercado/rampa (look at market/ramp): warns that the pyramid ramps are slippery.
		beginSecondarySpeechLine(1, 1);
		return true;
	case 303: // Ir a rampa (go to ramp): enter the pyramid/B02 scene through scene 2020.
		state.scene2010B02EntranceUsed = true;
		state.mainFlowStateId = kScene2020EntryFromPyramidExteriorState;
		return true;
	case 304: // Mirar anilla (look at ring/doorbell): identifies it as the doorbell.
		beginSecondarySpeechLine(2, 1);
		return true;
	case 305: // Usar anilla (use ring/doorbell): play the doorbell/gatekeeper sequence and enter scene 2100.
		runLongSequenceToScene2100();
		return true;
	case 306: // Mirar puerta (look at door): Ron says it must weigh a ton.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Abrir puerta (open door): cannot be opened from outside.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 308: // Coger jeroglifico (take hieroglyphic): impossible because it is carved in stone.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 309: // Mirar jeroglifico (look at hieroglyphic): illegible but decorative.
		beginSecondarySpeechLine(7, 0);
		return true;
	default:
		return false;
	}
}

bool Scene2010::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 5 && nextRegion == 4) {
		copyStepDeltasFromB4(0x0c, 0x17, 0x00);
		requestedFacing = 1;
		restoredStepDeltas = true;
		return true;
	}
	if ((currentRegion == 6 && nextRegion == 4) ||
			(currentRegion == 3 && nextRegion == 2)) {
		copyStepDeltasFromB4(0x3c, 0x47, 0x00);
		requestedFacing = 5;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 3 && nextRegion == 4) {
		copyStepDeltasFromB4(0x18, 0x23, 0x24);
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene2010::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;

	if (currentRegion == 5) {
		copyStepDeltasFromB4(0x30, 0x3b, 0x24);
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}
	if (currentRegion == 6) {
		copyStepDeltasFromB4(0x18, 0x23, 0x24);
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene2010::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	if (selector == 0 || selector == 0xff) {
		restoreBaseFramebufferFromOriginal();
		const uint paletteMaskBytes = MIN<uint>(_paletteMask.size(), _paletteMaskOriginal.size());
		const uint fullPaletteMaskBytes = MIN<uint>(_fullPaletteRegionMask.size(), _paletteMaskOriginal.size());
		if (paletteMaskBytes != 0)
			memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), paletteMaskBytes);
		if (_paletteMask.size() > paletteMaskBytes)
			memset(_paletteMask.data() + paletteMaskBytes, 0, _paletteMask.size() - paletteMaskBytes);
		if (fullPaletteMaskBytes != 0)
			memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), fullPaletteMaskBytes);
		if (_fullPaletteRegionMask.size() > fullPaletteMaskBytes)
			memset(_fullPaletteRegionMask.data() + fullPaletteMaskBytes, 0,
				_fullPaletteRegionMask.size() - fullPaletteMaskBytes);
	}

	if ((selector == 2 || selector == 0xff) && _vm->gameState().scene2010B02EntranceUsed)
		copyStageSmallRow(6, 2);

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene2010::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 1, 20, 50, 0x0b, 3, 100, 50);
}

void Scene2010::rebuildWalkableMask() {
	const uint maskBytes = MIN<uint>(_walkablePaletteMask.size(), _fullPaletteRegionMask.size());
	if (maskBytes != 0)
		memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), maskBytes);
	if (_walkablePaletteMask.size() > maskBytes)
		memset(_walkablePaletteMask.data() + maskBytes, 0, _walkablePaletteMask.size() - maskBytes);

	for (uint i = 0; i < maskBytes; ++i) {
		if (_walkablePaletteMask[i] != 1 && _walkablePaletteMask[i] != 4)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene2010::copyStageSmallRow(byte sourceRow, byte destinationRow) {
	const uint sourceOffset = (uint)sourceRow * kStage003SmallRowSize;
	const uint destinationOffset = (uint)destinationRow * kStage003SmallRowSize;
	if (sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene2010::copyStepDeltasFromB4(uint targetFirstOffset, uint targetLastOffset, uint sourceFirstOffset) {
	for (uint targetOffset = targetFirstOffset, sourceOffset = sourceFirstOffset;
			targetOffset <= targetLastOffset &&
			targetOffset < _actorPathStepDeltas.size() &&
			sourceOffset < ARRAYSIZE(kActorPathStepDeltaTableSetB4);
			++targetOffset, ++sourceOffset) {
		_actorPathStepDeltas[targetOffset] = kActorPathStepDeltaTableSetB4[sourceOffset];
	}
}

void Scene2010::runEntryFromMarket() {
	runEntryPath(0x364, 0x1df, 5, 0x2e9, 0x1a0);
	_activeActorCel = 0;

	GameplayState &state = _vm->gameState();
	if (!state.scene2010EntryLineSeen) {
		beginSecondarySpeechLine(1, 0);
		state.scene2010EntryLineSeen = true;
	}
}

void Scene2010::runEntryFromB02() {
	runEntryPath(0x23a, 0x1df, 1, 0x28a, 0x19a);
	_activeActorCel = 0;
}

void Scene2010::runPatchedEntrySequence() {
	if (_sceneChunkTable.isValidChunk(8))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);

	runEntryPath(0x130, 0x105, 2, 0x21c, 0x126);
	_soundBank0.playSample(0x0c, 100);

	if (_sceneChunkTable.isValidChunk(10))
		runSceneOverlay(10, kScene2010SecondOverlayDescriptorCount,
			kScene2010SecondOverlayFrameMap, ARRAYSIZE(kScene2010SecondOverlayFrameMap),
			kScene2010OverlayFrameMillis);

	if (_sceneChunkTable.isValidChunk(9)) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
		drawPlayableComposite();
		presentFrame();
	}

	GameplayState &state = _vm->gameState();
	if (state.scene2010TravelReturnSpeechState == 1) {
		beginStaticSecondarySpeechLine(0xd4, state.ronTapeRecorderState);
		if (state.ronTapeRecorderState < 0xff)
			++state.ronTapeRecorderState;
		state.scene2010TravelReturnSpeechState = 2;
	}
}

void Scene2010::runLongSequenceToScene2100() {
	if (_sceneChunkTable.isValidChunk(5)) {
		runActorReplacement(ActionOverlaySpec(5, kScene2010FirstOverlayDescriptorCount,
			kScene2010FirstOverlayFrameMap, ARRAYSIZE(kScene2010FirstOverlayFrameMap),
			kScene2010OverlayFrameMillis)
			.soundAt(10, 0x0b));
	}

	if (_sceneChunkTable.isValidChunk(6)) {
		runActorReplacement(ActionOverlaySpec(6, kScene2010SecondOverlayDescriptorCount,
			kScene2010SecondOverlayFrameMap, ARRAYSIZE(kScene2010SecondOverlayFrameMap),
			kScene2010OverlayFrameMillis)
			.soundAt(0, 0x0c));
	}

	if (_sceneChunkTable.isValidChunk(7)) {
		runSceneOverlay(7, kScene2010ActorOverlayDescriptorCount,
			kScene2010ActorOverlayFrameMap, ARRAYSIZE(kScene2010ActorOverlayFrameMap),
			kScene2010OverlayFrameMillis);
	}

	GameplayState &state = _vm->gameState();
	beginSecondarySpeechLine(3, state.scene2010LongSequenceFirstSpeechSeen ? 2 : 1);
	state.scene2010LongSequenceFirstSpeechSeen = true;
	state.mainFlowStateId = kScene2100EntryState;
}

} // End of namespace Hollywood
