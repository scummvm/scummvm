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

const uint16 kScene4040FirstState = 0x0fc8;
const uint16 kScene4040ReturnFromUpperExitState = 0x0fc9;
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
const int kScene4040UpperExitReturnStartX = 0x028a;
const int kScene4040UpperExitReturnStartY = 0x0128;
const byte kScene4040UpperExitReturnFacing = 1;
const int kScene4040UpperExitReturnTargetX = 0x024b;
const int kScene4040UpperExitReturnTargetY = 0x014a;
const uint kScene4040CyclicBackgroundChunk = 8;
const uint kScene4040RandomBackgroundChunk = 13;
const uint kScene4040StairOverlayChunk = 9;
const uint kScene4040StairOverlayDescriptorCount = 0x3e;
const uint kScene4040BackgroundDescriptorCount = 0x1a;
const uint kScene4040CandilOverlayChunk = 14;
const uint kScene4040CandilOverlayDescriptorCount = 9;
const byte kScene4040CandilItem = 0x3c;
const byte kScene4040CandilSceneItem = 8;
const byte kScene4040PaletteOverrideColor = 0xfb;

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
	PlayableSceneConfig config(4040,
		SceneResourceLayout(5, 5, 15),
		SceneViewport(kScene4040ViewportXOffset, kScene4040ViewportMinXOffset, kScene4040ViewportMaxXOffset),
		SceneActorPose(0x192, 0x0171, 2));
	config.setActorResources(kScene4040ActorBankTableEntry, kScene4040ActorPaletteTableEntry);
	config.setTextResources(kScene4040Resource003RowsOffsetIndex, kScene4040SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene4040::Scene4040(HollywoodEngine *vm) :
		PlayableScene(vm, scene4040Config()),
		_cyclicBackgroundChannel(),
		_randomBackgroundChannel(),
		_cyclicBackgroundLayer(),
		_randomBackgroundLayer(),
		_randomBackgroundState(0),
		_randomBackgroundRepeatCount(0) {
}

void Scene4040::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applyScenePaletteOverride();
	resetBackgroundLayers();
	if (_vm->gameState().mainFlowStateId == kScene4040ReturnFromUpperExitState) {
		_activeActorWorldX = kScene4040UpperExitReturnStartX;
		_activeActorWorldY = kScene4040UpperExitReturnStartY;
		_activeActorFacing = kScene4040UpperExitReturnFacing;
	} else {
		_activeActorWorldX = 0x192;
		_activeActorWorldY = 0x0171;
		_activeActorFacing = 2;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene4040::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawBackgroundLayers();
	if (_actionOverlayPlayer.replacesActor()) {
		drawActionOverlayLayer();
		return;
	}
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldY);
	drawActionOverlayLayer();
}

void Scene4040::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene4040ReturnFromUpperExitState) {
		runReturnFromUpperExitEntry();
		return;
	}

	if (_vm->gameState().mainFlowStateId == kScene4040FirstState && !state.scene4040EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene4040EntryLineSeen = true;
	}
}

bool Scene4040::prepareCustomGameplayLoop() {
	applyScenePaletteOverride();
	resetBackgroundLayers();
	return true;
}

bool Scene4040::advanceCustomGameplayLoop(uint32 delta) {
	advanceBackgroundLayers(delta);
	return false;
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
	applyScenePaletteOverride();

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

void Scene4040::applyScenePaletteOverride() {
	setPaletteEntry6Bit(kScene4040PaletteOverrideColor, 0, 0x11, 0);
}

void Scene4040::resetBackgroundLayers() {
	_cyclicBackgroundLayer.configure(kScene4040CyclicBackgroundChunk, kScene4040BackgroundDescriptorCount, nullptr, 0);
	_cyclicBackgroundLayer.visible = true;
	_cyclicBackgroundLayer.setFrame(0);
	_cyclicBackgroundChannel.reset(0, kScene4040FrameMillis);

	_randomBackgroundLayer.configure(kScene4040RandomBackgroundChunk, kScene4040BackgroundDescriptorCount, nullptr, 0);
	_randomBackgroundLayer.visible = true;
	_randomBackgroundLayer.setFrame(0);
	_randomBackgroundChannel.reset(0, kScene4040FrameMillis);
	_randomBackgroundState = 0;
	_randomBackgroundRepeatCount = 0;
}

void Scene4040::drawBackgroundLayers() {
	drawResourceSpriteLayer(_randomBackgroundLayer);
	drawResourceSpriteLayer(_cyclicBackgroundLayer);
}

void Scene4040::advanceBackgroundLayers(uint32 delta) {
	const uint cyclicFrameCount = _cyclicBackgroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < cyclicFrameCount; ++frame) {
		if (_cyclicBackgroundChannel.frameIndex == 0x19)
			_cyclicBackgroundChannel.frameIndex = 0;
		else
			++_cyclicBackgroundChannel.frameIndex;
		_cyclicBackgroundLayer.setFrame(_cyclicBackgroundChannel.frameIndex);
	}

	const uint randomFrameCount = _randomBackgroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < randomFrameCount; ++frame)
		advanceRandomBackgroundTick();
}

void Scene4040::advanceRandomBackgroundTick() {
	byte frameIndex = _randomBackgroundChannel.frameIndex;

	switch (_randomBackgroundState) {
	case 0:
		if (_random.getRandomNumber(14) == 0) {
			_randomBackgroundState = _random.getRandomBit() ? 2 : 1;
			_randomBackgroundRepeatCount = (byte)_random.getRandomNumber(9);
		}
		break;
	case 1:
		if (frameIndex == 0x19) {
			frameIndex = 0;
		} else {
			if (_randomBackgroundRepeatCount != 0) {
				if (frameIndex < 0x0e) {
					++frameIndex;
				} else if (_random.getRandomBit()) {
					_randomBackgroundState = 3;
					_randomBackgroundRepeatCount = 0;
					++frameIndex;
					break;
				} else {
					_randomBackgroundState = 2;
					frameIndex = 0x0d;
				}
				--_randomBackgroundRepeatCount;
			}
			if (_randomBackgroundRepeatCount == 0) {
				if (isRandomBackgroundHoldFrame(frameIndex))
					++frameIndex;
				else
					_randomBackgroundState = 0;
			}
		}
		break;
	case 2:
		if (_randomBackgroundRepeatCount != 0) {
			if (frameIndex == 0) {
				frameIndex = 1;
				_randomBackgroundState = 1;
			} else {
				--frameIndex;
			}
			--_randomBackgroundRepeatCount;
		}
		if (_randomBackgroundRepeatCount == 0) {
			if (isRandomBackgroundHoldFrame(frameIndex))
				--frameIndex;
			else
				_randomBackgroundState = 0;
		}
		break;
	case 3:
		if (frameIndex < 0x18)
			++frameIndex;
		else
			_randomBackgroundState = 4;
		break;
	case 4:
		if (_random.getRandomNumber(19) == 0) {
			frameIndex = 0x19;
			_randomBackgroundState = 1;
			_randomBackgroundRepeatCount = (byte)_random.getRandomNumber(14);
		}
		break;
	default:
		_randomBackgroundState = 0;
		break;
	}

	_randomBackgroundChannel.frameIndex = frameIndex;
	_randomBackgroundLayer.setFrame(frameIndex);
}

bool Scene4040::isRandomBackgroundHoldFrame(byte frameIndex) const {
	return frameIndex == 1 || frameIndex == 3 || frameIndex == 6 ||
		frameIndex == 8 || frameIndex == 0x0b || frameIndex == 0x0d;
}

void Scene4040::runReturnFromUpperExitEntry() {
	runEntryPath(kScene4040UpperExitReturnStartX, kScene4040UpperExitReturnStartY,
		kScene4040UpperExitReturnFacing, kScene4040UpperExitReturnTargetX,
		kScene4040UpperExitReturnTargetY);
}

void Scene4040::runStairReturnToDungeon() {
	runActorReplacement(ActionOverlaySpec(kScene4040StairOverlayChunk, kScene4040StairOverlayDescriptorCount,
		kScene4040StairOverlayFrameMap, ARRAYSIZE(kScene4040StairOverlayFrameMap), kScene4040FrameMillis));
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
	runActorReplacement(ActionOverlaySpec(kScene4040CandilOverlayChunk, kScene4040CandilOverlayDescriptorCount,
		kScene4040CandilFrameMap, ARRAYSIZE(kScene4040CandilFrameMap), kScene4040FrameMillis)
		.patchAt(4, 1));
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
