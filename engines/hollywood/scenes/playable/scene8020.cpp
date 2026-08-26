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

#include "hollywood/scenes/playable/scene8020.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene8010ReturnEntryState = 0x1f4b;
const uint kScene8020ActorBankTableEntry = 0x0000;
const uint kScene8020ActorPaletteTableEntry = 0x00cc;
const uint kScene8020Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene8020SpeechCueDescriptorTableOffset = 0x1135;
const int kScene8020EntryStartX = 0x2e4;
const int kScene8020EntryStartY = 0x1a3;
const int kScene8020EntryTargetX = 0x21c;
const int kScene8020EntryTargetY = 0x191;
const byte kScene8020EntryFacing = 5;
const uint32 kScene8020FrameMillis = 75;
const uint kScene8020ForegroundDescriptorCount = 0x13;
const uint kScene8020ForegroundChunk = 5;
const uint kScene8020SecondaryPatchChunk = 6;
const uint kScene8020Pickup5dPatchChunk = 7;
const uint kScene8020TransformationPatchChunk = 8;
const uint kScene8020Pickup6cPatchChunk = 9;
const uint kScene8020InventoryItem6cOverlayChunk = 10;
const uint kScene8020InventoryItem5dOverlayChunk = 11;
const uint kScene8020InventoryItemOverlayDescriptorCount = 8;
const uint kScene8020InventoryItem5dOverlayDescriptorCount = 0x0d;
const byte kScene8020ForegroundObjectOriginalItem = 3;
const byte kScene8020ForegroundObjectMovedItem = 4;
const byte kScene8020SecondaryObjectItem = 5;
const byte kScene8020InventoryItem6c = 0x6c;
const byte kScene8020InventoryItem5d = 0x5d;
const byte kScene8020InventoryItem4f = 0x4f;
const byte kScene8020OverlayHookPickup6c = 1;
const byte kScene8020OverlayHookPickup5d = 2;
const byte kScene8020OverlayHookRemove6c = 3;
const byte kScene8020TransformationHook = 4;

const byte kScene8020ForegroundFrameMap[] = {
	0, 0, 1, 1, 2, 2, 1, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 13, 18, 17, 18, 13, 14, 15
};

const byte kScene8020Pickup6cFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 0
};

const byte kScene8020Pickup5dFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const byte kScene8020Reverse6cFrameMap[] = {
	0, 7, 6, 5, 4, 3, 2, 1, 0
};

PlayableSceneConfig scene8020Config() {
	PlayableSceneConfig config(8020,
		SceneResourceLayout(5, 5, 11),
		SceneViewport(0),
		SceneActorPose(kScene8020EntryTargetX, kScene8020EntryTargetY, kScene8020EntryFacing));
	config.setActorResources(kScene8020ActorBankTableEntry, kScene8020ActorPaletteTableEntry);
	config.setTextResources(kScene8020Resource003RowsOffsetIndex, kScene8020SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet00);
	config.walkablePaletteMaxRegion = 20;
	config.loadActorDepthTables = false;
	return config;
}

Scene8020::Scene8020(HollywoodEngine *vm) :
		PlayableScene(vm, scene8020Config()),
		_foregroundLayer(),
		_foregroundChannel(),
		_originalColorToItemMap(),
		_foregroundAnimationState(0),
		_foregroundRepeatCount(0),
		_foregroundSequenceLocked(false) {
}

void Scene8020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetForegroundLayer();
	setActiveActorPose(kScene8020EntryTargetX, kScene8020EntryTargetY, kScene8020EntryFacing);
}

void Scene8020::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_vm->gameState().scene8020ForegroundObjectState != 2)
		drawResourceSpriteLayer(_foregroundLayer);
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawActionOverlayLayer();
}

void Scene8020::runCustomEntrySequence() {
	runEntryPath(kScene8020EntryStartX, kScene8020EntryStartY, kScene8020EntryFacing,
		kScene8020EntryTargetX, kScene8020EntryTargetY);

	GameplayState &state = _vm->gameState();
	if (!state.seenScene8020EntryLine) {
		beginSecondarySpeechLine(0, 0);
		state.seenScene8020EntryLine = true;
	}
}

bool Scene8020::prepareCustomGameplayLoop() {
	return true;
}

bool Scene8020::advanceCustomGameplayLoop(uint32 delta) {
	if (!_foregroundSequenceLocked && _vm->gameState().scene8020ForegroundObjectState != 2)
		advanceForegroundLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene8020::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a camino (go to path): return to lake unless the egg belongs in the nest.
		if (_vm->gameState().scene8020ForegroundObjectState == 2)
			beginSecondarySpeechLine(1, 1);
		else
			runExitToScene8010();
		return true;
	case 302: // Mirar camino (look at path): it leads to the lake shore.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Mirar nido (look at nest): huge bird nest.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Coger huevo (take egg): pick up item 0x6c.
		runPickupInventoryItem6cSequence();
		return true;
	case 305: // Abrir huevo (open egg): let nature take its course.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 306: // Coger polluelo (take chick): cannot take it.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 307: // Mirar polluelo (look at chick): it is hungry.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 308: // Coger cáscara de huevo (take eggshell): pick up item 0x5d.
		runPickupInventoryItem5dSequence();
		return true;
	case 309: // Original slot 08 no-op: produces no text or action.
		return true;
	case 310: // Usar gorro de aviador con huevo/nido (use cap with egg/nest): hatch the egg.
		runForegroundTransformationSequence();
		return true;
	case 311: // Usar huevo con nido (use egg with nest): put egg back/remove item 0x6c.
		runRemoveInventoryItem6cSequence();
		return true;
	case 312: // Dar comida equivocada al polluelo (give wrong food to chick).
		beginSecondarySpeechLine(9, 0);
		return true;
	default:
		return false;
	}
}

bool Scene8020::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	const int minimumX = _vm->gameState().scene8020ForegroundObjectState == 1 ? 0x13d : 0x118;
	targetX = CLIP<int>(targetX, minimumX, HollywoodEngine::kSceneBufferWidth - 1);
	targetY = CLIP<int>(targetY, 0, HollywoodEngine::kSceneBufferHeight - 1);

	if (targetY < 0x1df) {
		const int downY = targetY + 1;
		if (isWalkableAt(targetX, downY)) {
			targetY = downY;
			return true;
		}
	}

	if (targetY > 0) {
		const int upY = targetY - 1;
		if (isWalkableAt(targetX, upY)) {
			targetY = upY;
			return true;
		}
	}

	while (targetY < 0x1df) {
		if (isWalkableAt(targetX, targetY))
			return true;
		++targetY;
	}

	while (targetY > 0) {
		if (isWalkableAt(targetX, targetY))
			return true;
		--targetY;
	}

	return true;
}

bool Scene8020::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	if (_originalColorToItemMap.empty() &&
			_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
		_originalColorToItemMap.resize(kScenePaletteMapPageSize);
		memcpy(_originalColorToItemMap.data(), _paletteMaskOriginal.data() + kSceneColorToItemMap,
			_originalColorToItemMap.size());
	}

	bool handled = false;
	if (selector == 0 || selector == 0xff) {
		restoreBaseFramebufferFromOriginal();
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		handled = true;
	}

	if (selector == 1 || selector == 0xff) {
		const byte state = _vm->gameState().scene8020ForegroundObjectState;
		if (state == 0) {
			replaceColorMapItemFromOriginal(kScene8020ForegroundObjectOriginalItem,
				kScene8020ForegroundObjectOriginalItem);
		} else if (state == 1) {
			for (uint i = 0; i < _fullPaletteRegionMask.size() && i < _paletteMask.size(); ++i) {
				if (_fullPaletteRegionMask[i] == 1) {
					_fullPaletteRegionMask[i] = 0;
					_paletteMask[i] = 0;
				}
			}
			replaceColorMapItemFromOriginal(kScene8020ForegroundObjectOriginalItem,
				kScene8020ForegroundObjectMovedItem);
			ScenePoint point = { 0x13d, 0x00fc };
			_hotspots.setActionTarget(2, point, point);
			_hotspots.setActionTarget(4, point, point);
		} else if (state == 2) {
			replaceColorMapItemFromOriginal(kScene8020ForegroundObjectOriginalItem, 0);
		}
		_hotspots.setVerbMovementModeByGlobalRecordIndex(9, state == 2 ? 0 : 1);
		handled = true;
	}

	if (selector == 2 || selector == 0xff) {
		if (_vm->gameState().scene8020SecondaryObjectVisible) {
			replaceColorMapItemFromOriginal(kScene8020SecondaryObjectItem, kScene8020SecondaryObjectItem);
			if (_sceneChunkTable.isValidChunk(kScene8020SecondaryPatchChunk))
				drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene8020SecondaryPatchChunk],
					_baseFramebuffer);
		} else {
			replaceColorMapItemFromOriginal(kScene8020SecondaryObjectItem, 0);
		}
		handled = true;
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return handled;
}

AmbientAudioProfile Scene8020::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x12, 6, 50, 1, 0x0c, 1, 100, 50);
}

void Scene8020::handleAnimationFrameHook(byte hookId, uint frame) {
	GameplayState &state = _vm->gameState();
	if (hookId == kScene8020OverlayHookPickup6c && frame == 3) {
		if (_sceneChunkTable.isValidChunk(kScene8020Pickup6cPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene8020Pickup6cPatchChunk],
				_baseFramebuffer);
		state.scene8020ForegroundObjectState = 2;
		applySceneStateToHotspotsAndPatches(1);
		return;
	}

	if (hookId == kScene8020OverlayHookPickup5d && frame == 6) {
		if (_sceneChunkTable.isValidChunk(kScene8020Pickup5dPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene8020Pickup5dPatchChunk],
				_baseFramebuffer);
		return;
	}

	if (hookId == kScene8020OverlayHookRemove6c && frame == 4) {
		state.scene8020ForegroundObjectState = 0;
		applySceneStateToHotspotsAndPatches(1);
		return;
	}

	if (hookId == kScene8020TransformationHook) {
		if (frame == 0x0b) {
			_soundBank0.playSample(0x1b, 100);
		} else if (frame == 0x12) {
			_soundBank0.playSample(0x1c, 50);
			if (_sceneChunkTable.isValidChunk(kScene8020TransformationPatchChunk)) {
				drawResourceBlockList(_resourceArena,
					_resourceChunkOffsets[kScene8020TransformationPatchChunk], _baseFramebuffer);
			}
		}
	}
}

void Scene8020::resetForegroundLayer() {
	_foregroundLayer.configure(kScene8020ForegroundChunk, kScene8020ForegroundDescriptorCount,
		kScene8020ForegroundFrameMap, ARRAYSIZE(kScene8020ForegroundFrameMap));
	_foregroundLayer.visible = true;
	_foregroundChannel.reset(0, kScene8020FrameMillis);
	_foregroundSequenceLocked = false;

	const byte state = _vm->gameState().scene8020ForegroundObjectState;
	if (state == 1) {
		_foregroundLayer.reset(0x17);
		_foregroundAnimationState = 3;
	} else {
		_foregroundLayer.reset(0);
		_foregroundAnimationState = 0;
	}
	_foregroundRepeatCount = 0;
}

void Scene8020::advanceForegroundLayer(uint32 delta) {
	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		switch (_foregroundAnimationState) {
		case 0:
			if (_random.getRandomNumber(29) == 0) {
				_foregroundRepeatCount = (byte)(_random.getRandomNumber(7) + 3);
				_foregroundAnimationState = (byte)(_random.getRandomBit() ? 2 : 1);
				_soundBank0.playSample(_foregroundAnimationState == 1 ? 0x19 : 0x1a,
					_foregroundAnimationState == 1 ? 30 : 100, true);
			}
			break;
		case 1:
			if (_foregroundLayer.frameIndex == 3) {
				_foregroundLayer.setFrame(0);
				if (_foregroundRepeatCount == 0) {
					_foregroundAnimationState = 0;
					_soundBank0.stop();
				} else {
					--_foregroundRepeatCount;
					_soundBank0.playSample(0x19, 30, true);
				}
			} else {
				_foregroundLayer.setFrame(_foregroundLayer.frameIndex + 1);
			}
			break;
		case 2:
			if (_foregroundLayer.frameIndex == 7) {
				_foregroundLayer.setFrame(0);
				if (_foregroundRepeatCount == 0) {
					_foregroundAnimationState = 0;
					_soundBank0.stop();
				} else {
					--_foregroundRepeatCount;
					_soundBank0.playSample(0x1a, 100, true);
				}
			} else {
				_foregroundLayer.setFrame(_foregroundLayer.frameIndex + 1);
			}
			break;
		case 3:
			if (_random.getRandomNumber(29) == 0) {
				_foregroundAnimationState = 5;
			} else if (_random.getRandomNumber(9) == 0) {
				_foregroundLayer.setFrame(0x18);
				_foregroundAnimationState = 4;
			}
			break;
		case 4:
			_foregroundLayer.setFrame(0x17);
			_foregroundAnimationState = 3;
			break;
		case 5:
			if (_foregroundLayer.frameIndex == 0x21) {
				_foregroundLayer.setFrame(0x17);
				_foregroundAnimationState = 3;
			} else {
				_foregroundLayer.setFrame(_foregroundLayer.frameIndex + 1);
				if (_foregroundLayer.frameIndex == 0x19)
					_soundBank0.playSample(0x18, 100);
			}
			break;
		default:
			_foregroundAnimationState = 0;
			_foregroundLayer.setFrame(0);
			break;
		}
	}
}

bool Scene8020::isWalkableAt(int x, int y) const {
	if (x < 0 || y < 0 || x >= HollywoodEngine::kSceneBufferWidth || y >= HollywoodEngine::kSceneBufferHeight)
		return false;
	const uint offset = y * HollywoodEngine::kSceneBufferWidth + x;
	return isFramebufferOffsetValid(offset) && _walkablePaletteMask[savedFramebufferPixelAt(offset)] != 0;
}

void Scene8020::replaceColorMapItemFromOriginal(byte sourceItem, byte destinationItem) {
	if (_originalColorToItemMap.size() < kScenePaletteMapPageSize ||
			_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
		if (_originalColorToItemMap[i] == sourceItem)
			_paletteMask[kSceneColorToItemMap + i] = destinationItem;
	}
}

void Scene8020::runExitToScene8010() {
	_vm->gameState().mainFlowStateId = kScene8010ReturnEntryState;
	_vm->gameState().activeActorPoseValid = false;
}

void Scene8020::runPickupInventoryItem6cSequence() {
	_foregroundLayer.setFrame(0);
	_foregroundAnimationState = 0;
	_foregroundRepeatCount = 0;
	_soundBank0.stop();

	runActorReplacement(ActionOverlaySpec(kScene8020InventoryItem6cOverlayChunk, kScene8020InventoryItemOverlayDescriptorCount,
		kScene8020Pickup6cFrameMap, ARRAYSIZE(kScene8020Pickup6cFrameMap), kScene8020FrameMillis)
		.hookAt(3, kScene8020OverlayHookPickup6c));

	addInventoryItem(kScene8020InventoryItem6c);
	_soundBank0.playSample(1, 100);
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(4, 0);
}

void Scene8020::runPickupInventoryItem5dSequence() {
	runActorReplacement(ActionOverlaySpec(kScene8020InventoryItem5dOverlayChunk, kScene8020InventoryItem5dOverlayDescriptorCount,
		kScene8020Pickup5dFrameMap, ARRAYSIZE(kScene8020Pickup5dFrameMap), kScene8020FrameMillis)
		.hookAt(6, kScene8020OverlayHookPickup5d));

	addInventoryItem(kScene8020InventoryItem5d);
	_soundBank0.playSample(1, 100);
	_vm->gameState().scene8020SecondaryObjectVisible = false;
	applySceneStateToHotspotsAndPatches(2);
	beginSecondarySpeechLine(0x14, (byte)_random.getRandomNumber(4));
}

void Scene8020::runRemoveInventoryItem6cSequence() {
	runActorReplacement(ActionOverlaySpec(kScene8020InventoryItem6cOverlayChunk, kScene8020InventoryItemOverlayDescriptorCount,
		kScene8020Reverse6cFrameMap, ARRAYSIZE(kScene8020Reverse6cFrameMap), kScene8020FrameMillis)
		.hookAt(4, kScene8020OverlayHookRemove6c));

	removeInventoryItem(kScene8020InventoryItem6c);
	_soundBank0.playSample(1, 100);
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(1, 1);
}

void Scene8020::runForegroundTransformationSequence() {
	beginSecondarySpeechLine(8, 0);
	beginSecondarySpeechLine(8, 1);
	walkActiveActorTo(0x118, 0x00eb, 5, 0, false);

	runActorReplacement(ActionOverlaySpec(kScene8020InventoryItem6cOverlayChunk, kScene8020InventoryItemOverlayDescriptorCount,
		kScene8020Reverse6cFrameMap, ARRAYSIZE(kScene8020Reverse6cFrameMap), kScene8020FrameMillis)
		.hookAt(4, kScene8020OverlayHookRemove6c));

	_foregroundAnimationState = 2;
	_foregroundRepeatCount = 4;
	_soundBank0.playSample(0x1a, 100, true);
	removeInventoryItem(kScene8020InventoryItem6c);
	removeInventoryItem(kScene8020InventoryItem4f);
	_soundBank0.playSample(1, 100);

	walkActiveActorTo(0x15e, 0x0104, 0xff, 0, false);
	walkActiveActorTo(0x15e, 0x0104, 5, 0, false);

	_foregroundLayer.setFrame(7);
	_soundBank0.playSample(0x1a, 100);
	_foregroundSequenceLocked = true;
	playAnimationFrames(_foregroundLayer,
		AnimationFrameRange(7, 0x17, kScene8020FrameMillis)
			.hookEveryFrame(kScene8020TransformationHook));
	_foregroundSequenceLocked = false;

	GameplayState &state = _vm->gameState();
	state.scene8020ForegroundObjectState = 1;
	state.scene8020SecondaryObjectVisible = true;
	applySceneStateToHotspotsAndPatches(0xff);
	resetForegroundLayer();
	beginSecondarySpeechLine(8, 2);
}

} // End of namespace Hollywood
