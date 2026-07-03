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

#include "hollywood/scenes/playable/scene2070.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene2070ArchiveName = "RESOURCE.B07";
const char *const kScene2070MusicArchiveName = "RESOURCE.M02";
const char *const kScene2070SoundArchiveName = "RESOURCE.S02";
const uint kScene2070InitialRequiredChunkCount = 11;
const uint kScene2070ArenaFirstChunk = 5;
const uint kScene2070ArenaLastChunk = 10;
const uint kScene2070StageIndex = 207;
const uint16 kScene2070FirstState = 0x0816;
const uint16 kScene2070LastState = 0x0817;
const uint16 kScene2050LabyrinthReturnState = 0x0803;
const uint16 kScene2060RightPassageState = 0x080f;
const uint16 kScene2080FirstState = 0x0820;
const uint16 kScene2070ViewportXOffset = 0x0068;
const uint16 kScene2070ViewportMaxXOffset = 0x00a8;
// The original calls LoadActorSpriteBankSetB4(), but the RESOURCE.000
// startup table maps that bank to offset/size table entry 0x0000.
const uint kScene2070ActorBankTableEntry = 0x0000;
const uint kScene2070ActorPaletteTableEntry = 0x00cc;
const uint kScene2070Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2070SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2070ForegroundFrameMillis = 75;
const uint32 kScene2070MemoryFrameMillis = 60;
const uint32 kScene2070OverlayFrameMillis = 75;
const uint kScene2070ForegroundDescriptorCount = 0x1a;
const uint kScene2070MemoryDescriptorCount = 0x5a;
const uint kScene2070InventoryOverlayDescriptorCount = 0x0d;
const uint kScene2070ExitVerbRecordIndex = 0x29;

const byte kScene2070ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene2070ForegroundFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25
};

const byte kScene2070SealMemoryFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
	27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,
	82, 83, 84, 85, 86, 87, 88, 89
};

const byte kScene2070InventoryOverlayFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

static_assert(ARRAYSIZE(kScene2070ForegroundFrameMap) == 0x1a, "Scene 2070 foreground frame map size changed");
static_assert(ARRAYSIZE(kScene2070SealMemoryFrameMap) == 0x62, "Scene 2070 seal memory frame map size changed");
static_assert(ARRAYSIZE(kScene2070InventoryOverlayFrameMap) == 14, "Scene 2070 inventory overlay frame map size changed");

static PlayableSceneConfig scene2070Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene2070ArchiveName;
	config.initialRequiredChunkCount = kScene2070InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene2070ArenaFirstChunk;
	config.arenaLastChunk = kScene2070ArenaLastChunk;
	config.stageIndex = kScene2070StageIndex;
	config.debugName = "Scene 2070";
	config.viewportXOffset = kScene2070ViewportXOffset;
	config.viewportMinXOffset = kScene2070ViewportXOffset;
	config.viewportMaxXOffset = kScene2070ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 2;
	config.actorBankTableEntry = kScene2070ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene2070ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene2070Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene2070SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene2070ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene2070ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 1;
	config.musicArchiveName = kScene2070MusicArchiveName;
	config.soundBank0ArchiveName = kScene2070SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene2070FirstState;
	config.mainFlowLastState = kScene2070LastState;
	return config;
}

Scene2070::Scene2070(HollywoodEngine *vm) :
		PlayableScene(vm, scene2070Config(), "scene2070", 0x0ef, 0x12c, 2, 0xfd, 0xfb),
		_foregroundChannel(),
		_foregroundLayer() {
	_foregroundLayer.configure(7, kScene2070ForegroundDescriptorCount,
		kScene2070ForegroundFrameMap, ARRAYSIZE(kScene2070ForegroundFrameMap));
}

bool Scene2070::hasCustomPreviewState() const {
	return true;
}

void Scene2070::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetForegroundLayer();
	applySceneStateToHotspotsAndPatches(0xff);

	const GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == 0x0817) {
		if (state.scene2070EntryProgress < 2) {
			_activeActorWorldX = 0x2b7;
			_activeActorWorldY = 0x13f;
			_activeActorFacing = 4;
		} else {
			_activeActorWorldX = 0x24d;
			_activeActorWorldY = 0x130;
			_activeActorFacing = 4;
		}
	} else {
		_activeActorWorldX = 0x0ef;
		_activeActorWorldY = 0x12c;
		_activeActorFacing = 2;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene2070::hasCustomComposite() const {
	return true;
}

void Scene2070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_foregroundLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
}

bool Scene2070::hasCustomEntrySequence() const {
	return true;
}

void Scene2070::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == 0x0817)
		runEntryFromRightPassage();
	else
		runEntryFromLabyrinth();
}

bool Scene2070::prepareCustomGameplayLoop() {
	resetForegroundLayer();
	return true;
}

bool Scene2070::advanceCustomGameplayLoop(uint32 delta) {
	advanceForegroundLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene2070::dispatchCustomSceneAction(uint16 handlerId) {
	GameplayState &state = _vm->gameState();

	switch (handlerId) {
	case 301: // Ir a laberinto/camara (go back): returns to B06 unless the seal route has been solved.
		state.mainFlowStateId = state.egyptSealPuzzleProgress == 2 ?
			kScene2050LabyrinthReturnState : kScene2060RightPassageState;
		return true;
	case 302: // Mirar/usar objeto B07 accion 302: scene speech row 2.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Mirar/usar objeto B07 accion 303: scene speech row 3.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Mirar/usar objeto B07 accion 304: scene speech row 4.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 305: // Mirar/usar objeto con estado de salida: scene speech row 5, variant 0/1.
		beginSecondarySpeechLine(5, state.scene2070SealExitPatchState != 0 ? 1 : 0);
		return true;
	case 306: // Mirar/usar objeto B07 accion 306: scene speech row 6.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 307: // Ir a salida del sello (go to seal exit): enter the next B08 scene.
		state.mainFlowStateId = kScene2080FirstState;
		return true;
	case 308: // Mirar/usar objeto con estado de salida: scene speech row 7, variant 0/1.
		beginSecondarySpeechLine(7, state.scene2070SealExitPatchState != 0 ? 1 : 0);
		return true;
	case 309: // Mirar/usar objeto bloqueado/desbloqueado: scene row 7 or shared row 8.
		if (state.scene2070SealExitPatchState == 0)
			beginSecondarySpeechLine(7, 0);
		else
			beginStaticSecondarySpeechLine(8, 0);
		return true;
	case 310: // Mirar/usar objeto bloqueado/desbloqueado: shared row 11 or row 19.
		if (state.scene2070SealExitPatchState == 0)
			beginStaticSecondarySpeechLine(0x0b, 0);
		else
			beginStaticSecondarySpeechLine(0x13, (byte)_random.getRandomNumber(1));
		return true;
	case 311: // Mirar/usar objeto B07 accion 311: scene speech row 8.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 312: // Mirar/usar objeto B07 accion 312: scene speech row 9.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 313: // Usar objeto de inventario en B07: overlay, then hide item 7 patch.
		runAnimatedInventoryStateChange();
		return true;
	case 314: // Coger/mirar objeto B07 accion 314: scene speech row 11.
		beginSecondarySpeechLine(11, 0);
		return true;
	default:
		return false;
	}
}

bool Scene2070::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x064, 0x2b7);
	if (targetY < 0x1df)
		++targetY;

	while (targetY < 0x1df) {
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
		++targetY;
	}

	if (walkableMaskAt(targetX, targetY) != 0)
		return true;

	while (targetY > 0) {
		--targetY;
		if (walkableMaskAt(targetX, targetY) != 0)
			return true;
	}

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene2070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if ((selector == 1 || selector == 0xff) && state.scene2070SealExitPatchState != 0) {
		if (_sceneChunkTable.isValidChunk(5))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _baseFramebuffer);
	}

	if (selector == 2 || selector == 0xff) {
		if (state.scene2070InnerPassagePatchState == 0) {
			for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
				if (_fullPaletteRegionMask[color] > 3)
					_fullPaletteRegionMask[color] = 1;
				if (_paletteMask[kSceneColorToItemMap + color] == 6)
					_paletteMask[kSceneColorToItemMap + color] = 0;
				if (_paletteMask[kSceneColorToItemMap + color] == 8)
					_paletteMask[kSceneColorToItemMap + color] = 5;
				if (_paletteMask[kSceneColorToItemMap + color] == 9)
					_paletteMask[kSceneColorToItemMap + color] = 4;
			}
			if (_routeSteps.size() > 0x1b5)
				_routeSteps[0x1b5] = 2;
			if (_routeSteps.size() > 0x331)
				_routeSteps[0x331] = 1;
			if (_actorDepthYThresholds.size() > 3) {
				_actorDepthYThresholds[3] = 0;
				_drawActorDepthYThresholds[3] = 0;
			}
		} else {
			if (_sceneChunkTable.isValidChunk(6))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _baseFramebuffer);
			for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
				if (_fullPaletteRegionMask[color] == 8)
					_fullPaletteRegionMask[color] = 0;
				if (_paletteMask[kSceneColorToItemMap + color] == 8 ||
						_paletteMask[kSceneColorToItemMap + color] == 9)
					_paletteMask[kSceneColorToItemMap + color] = 0;
			}
		}
	}

	if ((selector == 3 || selector == 0xff) && state.scene2070HiddenItemPatchState == 0) {
		setColorMapItem(7, 0);
		if (state.scene2070InnerPassagePatchState != 0 && _sceneChunkTable.isValidChunk(9))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[9], _baseFramebuffer);
	}

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);

	if (state.scene2070SealExitPatchState == 0) {
		_hotspots.setVerbActionHandlerByGlobalRecordIndex(kScene2070ExitVerbRecordIndex, 1);
	} else {
		ScenePoint point;
		point.x = 0x30e;
		point.y = 0x0ee;
		_hotspots.setActionTarget(5, point, point);
		_hotspots.setVerbActionHandlerByGlobalRecordIndex(kScene2070ExitVerbRecordIndex, 0x133);
	}

	return true;
}

AmbientAudioProfile Scene2070::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2070::resetForegroundLayer() {
	_foregroundChannel.reset(0, kScene2070ForegroundFrameMillis);
	_foregroundLayer.visible = true;
	_foregroundLayer.reset(0);
}

void Scene2070::advanceForegroundLayer(uint32 delta) {
	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		byte nextFrame = (byte)(_foregroundLayer.frameIndex + 1);
		if (nextFrame >= ARRAYSIZE(kScene2070ForegroundFrameMap))
			nextFrame = 0;
		_foregroundLayer.setFrame(nextFrame);
	}
}

void Scene2070::runEntryFromLabyrinth() {
	_activeActorWorldX = 0x04b;
	_activeActorWorldY = 0x110;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_foregroundLayer.setFrame(0);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	if (state.egyptSealPuzzleProgress == 1) {
		runSealMemoryEffect();
		state.egyptSealPuzzleProgress = 2;
	}

	beginSecondarySpeechLine(1, 1);
	runEntryPath(0x04b, 0x110, 2, 0x0ef, 0x12c);

	if (state.scene2070EntryProgress == 0) {
		beginSecondarySpeechLine(1, 0);
		state.scene2070EntryProgress = 1;
	}
}

void Scene2070::runEntryFromRightPassage() {
	_activeActorWorldX = 0x30e;
	_activeActorWorldY = 0x0ee;
	_activeActorFacing = 4;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_foregroundLayer.setFrame(0);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	if (state.scene2070EntryProgress < 2) {
		runEntryPathWithFinalFacing(0x30e, 0x0ee, 4, 0x2b7, 0x13f, 4, 0);
		beginStaticSecondarySpeechLine(0xd0, 0);
		state.scene2070EntryProgress = 2;
		return;
	}

	runEntryPath(0x30e, 0x0ee, 4, 0x24d, 0x130);
}

void Scene2070::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel) {
	_activeActorWorldX = startX;
	_activeActorWorldY = startY;
	_activeActorFacing = startFacing;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);

	drawPlayableComposite();
	presentFrame();
	walkActiveActorTo(targetX, targetY, finalFacing, finalCel, false);
	_activeActorFacing = finalFacing;
	_activeActorCel = finalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene2070::runSealMemoryEffect() {
	drawPlayableComposite();
	presentFrame();
	_soundBank0.playSample(0x24, 50);

	for (uint i = 0; i < ARRAYSIZE(kScene2070SealMemoryFrameMap) && !Engine::shouldQuit(); ++i) {
		if (i == 0x56)
			_soundBank0.playSample(0x2e, 50);
		drawClipFrameDelta(8, kScene2070MemoryDescriptorCount, kScene2070SealMemoryFrameMap[i]);
		presentFrame();
		if (waitSceneMillis(kScene2070MemoryFrameMillis))
			break;
	}

	GameplayState &state = _vm->gameState();
	state.scene2070SealExitPatchState = 1;
	applySceneStateToHotspotsAndPatches(1);
	drawPlayableComposite();
	presentFrame();
}

void Scene2070::runAnimatedInventoryStateChange() {
	beginSecondarySpeechLine(10, 0);
	runHiddenActorActionOverlay(10, kScene2070InventoryOverlayDescriptorCount,
		kScene2070InventoryOverlayFrameMap, ARRAYSIZE(kScene2070InventoryOverlayFrameMap),
		kScene2070OverlayFrameMillis);

	_soundBank0.playSample(1, 100);
	_vm->gameState().scene2070HiddenItemPatchState = 0;
	applySceneStateToHotspotsAndPatches(3);
}

void Scene2070::drawClipFrameDelta(byte chunkIndex, uint tableEntryCount, byte frameIndex) {
	if (!_sceneChunkTable.isValidChunk(chunkIndex))
		return;
	if (frameIndex >= tableEntryCount)
		return;

	const uint32 frameTableOffset = _resourceChunkOffsets[chunkIndex];
	const uint32 chunkEnd = frameTableOffset + _sceneChunkTable.sizes[chunkIndex];
	if (frameTableOffset > _resourceArena.size() || chunkEnd > _resourceArena.size() ||
			frameTableOffset + (uint32)tableEntryCount * 4 > chunkEnd)
		return;

	const uint32 tableEntryOffset = frameTableOffset + ((uint32)frameIndex * 4);
	if (tableEntryOffset + 4 > chunkEnd)
		return;

	const uint32 frameOffset = frameTableOffset + ((uint32)tableEntryCount * 4) +
		readUint32LE(_resourceArena, tableEntryOffset);
	if (frameOffset + 4 > chunkEnd)
		return;

	const uint16 firstRow = readUint16LE(_resourceArena, frameOffset);
	const uint16 lastRow = readUint16LE(_resourceArena, frameOffset + 2);
	if (firstRow > lastRow)
		return;

	uint cursor = frameOffset + 4;
	byte *pixels = framebufferPixels(_sceneFramebuffer);
	const uint size = framebufferByteCount();
	if (!pixels)
		return;

	for (uint row = firstRow; row <= lastRow && row < HollywoodEngine::kSceneBufferHeight; ++row) {
		if (cursor >= chunkEnd)
			return;

		byte runCount = _resourceArena[cursor++];
		for (; runCount != 0; --runCount) {
			if (cursor + 3 > chunkEnd)
				return;

			const uint x = readUint16LE(_resourceArena, cursor);
			const byte literalLength = _resourceArena[cursor + 2];
			const uint destinationOffset = row * HollywoodEngine::kSceneBufferWidth + x;

			if (literalLength == 0) {
				if (cursor + 5 > chunkEnd)
					return;

				const byte fillValue = _resourceArena[cursor + 3];
				const uint fillLength = _resourceArena[cursor + 4];
				cursor += 5;
				if (x < HollywoodEngine::kSceneBufferWidth && destinationOffset < size) {
					const uint copyLength = MIN<uint>(fillLength,
						HollywoodEngine::kSceneBufferWidth - x);
					if (destinationOffset + copyLength <= size)
						memset(pixels + destinationOffset, fillValue, copyLength);
				}
			} else {
				const uint literalOffset = cursor + 3;
				if (literalOffset + literalLength > chunkEnd)
					return;

				if (x < HollywoodEngine::kSceneBufferWidth && destinationOffset < size) {
					const uint copyLength = MIN<uint>((uint)literalLength,
						HollywoodEngine::kSceneBufferWidth - x);
					if (destinationOffset + copyLength <= size)
						memcpy(pixels + destinationOffset, _resourceArena.data() + literalOffset, copyLength);
				}
				cursor = literalOffset + literalLength;
			}
		}
	}
}

void Scene2070::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 2 || _walkablePaletteMask[i] == 3)
			_walkablePaletteMask[i] = 0;
	}
}

void Scene2070::setColorMapItem(byte sourceItem, byte destinationItem) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == sourceItem)
			_paletteMask[kSceneColorToItemMap + color] = destinationItem;
	}
}

} // End of namespace Hollywood
