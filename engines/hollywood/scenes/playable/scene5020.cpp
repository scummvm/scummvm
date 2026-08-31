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

const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5020ViewportXOffset = 0x0048;
const uint16 kScene5020ViewportMinXOffset = 0x0000;
const uint16 kScene5020ViewportMaxXOffset = 0x0048;
const uint kScene5020ActorBankTableEntry = 0x0000;
const uint kScene5020ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5020SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5020OverlayFrameMillis = 75;
const uint32 kScene5020MineCartTimerNumerator = 75;
const byte kScene5020WoodenPlankItem = 0x47;
const byte kScene5020TakenSceneItemId = 4;
const byte kScene5020RenamedSmallRow = 7;
const byte kScene5020WallClueSmallRow = 8;
const uint kScene5020MineCartOverlayChunk = 9;
const uint kScene5020MineCartDescriptorCount = 0x38;
const byte kScene5020MineCartStopSoundFrame = 0x32;
const uint kScene5020PickupOverlayChunk = 10;
const uint kScene5020PickupOverlayDescriptorCount = 0x0d;
const int kScene5020EntryStartX = 0x348;
const int kScene5020EntryStartY = 0x186;
const int kScene5020EntryTargetX = 0x27d;
const int kScene5020EntryTargetY = 0x16c;
const int kScene5020MinimumWalkX = 0x50;
const int kScene5020MaximumWalkY = 0x1df;

const byte kScene5020MineCartDelayBuckets[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2,
	3, 3, 3, 3,
	4, 4, 4, 4,
	5, 5, 5, 5,
	6, 6, 6, 6,
	7, 7, 7, 7,
	8, 8, 8, 8,
	9, 9, 9, 9,
	10, 10, 10, 10,
	11, 11, 11, 11,
	12, 12, 12, 12
};

const byte kScene5020PickupFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

PlayableSceneConfig scene5020Config() {
	PlayableSceneConfig config(5020,
		SceneResourceLayout(5, 5, 10),
		SceneViewport(kScene5020ViewportXOffset, kScene5020ViewportMinXOffset, kScene5020ViewportMaxXOffset),
		SceneActorPose(0x27d, 0x16c, 4));
	config.setActorResources(kScene5020ActorBankTableEntry, kScene5020ActorPaletteTableEntry);
	config.setTextResources(0, kScene5020SpeechCueDescriptorTableOffset);
	return config;
}

Scene5020::Scene5020(HollywoodEngine *vm) :
		PlayableScene(vm, scene5020Config()),
		_mineCartLayer() {
	_mineCartLayer.configure(kScene5020MineCartOverlayChunk,
		kScene5020MineCartDescriptorCount, nullptr, 0);
}

void Scene5020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	_mineCartLayer.visible = false;
	_mineCartLayer.reset(0);

	setActiveActorPose(kScene5020EntryStartX, kScene5020EntryStartY, 4);
}

void Scene5020::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel,
		int activeWorldX, int activeWorldY, bool drawSecondaryActor, byte secondaryFacing,
		byte secondaryFrame, int secondaryWorldX, int secondaryWorldY, byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForeground(actorDrawOrderMode, activeWorldY);
	drawActionOverlayLayer();
	drawResourceSpriteLayer(_mineCartLayer);
}

bool Scene5020::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene5020::runCustomEntrySequence() {
	runMineCartArrival();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	runEntryPath(kScene5020EntryStartX, kScene5020EntryStartY, 4,
		kScene5020EntryTargetX, kScene5020EntryTargetY);
}

bool Scene5020::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene5020::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
}

void Scene5020::advanceCustomGameplayLoop(uint32 delta) {
	(void)delta;
	ensureAmbientSoundCuePlaying(1, 0x0c, 10);
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

bool Scene5020::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = MAX<int>(targetX, kScene5020MinimumWalkX);
	if (targetY < kScene5020MaximumWalkY)
		++targetY;

	while (targetY < kScene5020MaximumWalkY && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return true;
}

bool Scene5020::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if ((currentRegion == 4 && nextRegion == 5) ||
			(currentRegion == 5 && nextRegion == 6) ||
			(currentRegion == 6 && nextRegion == 7) ||
			(currentRegion == 7 && nextRegion == 8)) {
		requestedFacing = 4;
		copySlopeStepDeltasFromSet5A(0x30);
		restoredStepDeltas = true;
		return true;
	}
	if ((currentRegion == 8 && nextRegion == 7) ||
			(currentRegion == 7 && nextRegion == 6) ||
			(currentRegion == 6 && nextRegion == 5) ||
			(currentRegion == 5 && nextRegion == 4)) {
		requestedFacing = 1;
		copySlopeStepDeltasFromSet5A(0x0c);
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene5020::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if (state.scene5020ExplosivesCrateIdentified)
		copyStageSmallRow(kScene5020RenamedSmallRow, kScene5020WallClueSmallRow);
	if (state.scene5020WoodenPlankTaken || hasInventoryItem(kScene5020WoodenPlankItem)) {
		if (_sceneChunkTable.isValidChunk(8))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5020TakenSceneItemId);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	if (state.scene5020ExplosivesCrateIdentified)
		_hotspots.setVerbMovementModeByGlobalRecordIndex(0x3c, 0);
	return true;
}

AmbientAudioProfile Scene5020::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 10, 25, 0x0b, 5, 100, 50);
}

byte Scene5020::ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const {
	if (cueId == 0x10)
		return 2;
	if (cueId == 0x14)
		return 100;
	return defaultVolumePercent;
}

void Scene5020::runMineCartArrival() {
	if (!_sceneChunkTable.isValidChunk(kScene5020MineCartOverlayChunk))
		return;

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	_mineCartLayer.visible = true;
	_mineCartLayer.reset(0);
	drawPlayableComposite();
	presentFrame();
	if (fadePaletteFromBlack()) {
		_mineCartLayer.visible = false;
		_hideActiveActor = previousHideActiveActor;
		return;
	}

	_soundBank0.playSample(0x18, 100);
	for (uint frame = 0; frame < ARRAYSIZE(kScene5020MineCartDelayBuckets) &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++frame) {
		const byte delayBucket = kScene5020MineCartDelayBuckets[frame];
		const uint32 frameMillis = kScene5020MineCartTimerNumerator /
			MAX<uint32>(1, 13 - delayBucket);
		if (waitSceneMillis(frameMillis, false))
			break;

		const byte nextFrame = (byte)(frame + 1);
		_mineCartLayer.setFrame(nextFrame);
		if (nextFrame == kScene5020MineCartStopSoundFrame)
			_soundBank0.playSample(0x16, 100);
		drawPlayableComposite();
		presentFrame();
	}

	_mineCartLayer.visible = false;
	_hideActiveActor = previousHideActiveActor;
	drawPlayableComposite();
	presentFrame();
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

	runActorReplacement(kScene5020PickupOverlayChunk, kScene5020PickupOverlayDescriptorCount,
		kScene5020PickupFrameMap, ARRAYSIZE(kScene5020PickupFrameMap), kScene5020OverlayFrameMillis);
	addInventoryItem(kScene5020WoodenPlankItem);
	_soundBank0.playSample(1, 100);
	state.scene5020WoodenPlankTaken = true;
	applySceneStateToHotspotsAndPatches(1);
}

void Scene5020::drawForeground(byte actorDrawOrderMode, int actorWorldY) {
	uint chunkIndex = 0;
	if (actorDrawOrderMode == 1 || actorDrawOrderMode == 2) {
		chunkIndex = 5;
	} else if (actorDrawOrderMode == 3 || actorDrawOrderMode == 4) {
		chunkIndex = actorWorldY > 0x174 ? 7 : 6;
	} else if (actorDrawOrderMode >= 5 && actorDrawOrderMode <= 8) {
		chunkIndex = 7;
	}

	if (chunkIndex != 0 && _sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

void Scene5020::copySlopeStepDeltasFromSet5A(uint firstOffset) {
	for (uint i = 0; i < 0x0c && firstOffset + i < _actorPathStepDeltas.size() &&
			firstOffset + i < ARRAYSIZE(kActorPathStepDeltaTableSet5A); ++i)
		_actorPathStepDeltas[firstOffset + i] = kActorPathStepDeltaTableSet5A[firstOffset + i];
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
