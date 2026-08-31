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
const uint32 kScene8020AmbientCheckMillis = 250;
const uint kScene8020ForegroundDescriptorCount = 0x13;
const uint kScene8020ForegroundChunk = 5;
const uint kScene8020SecondaryPatchChunk = 6;
const uint kScene8020Pickup5dPatchChunk = 7;
const uint kScene8020Pickup6cPatchChunk = 9;
const uint kScene8020InventoryItem6cOverlayChunk = 10;
const uint kScene8020InventoryItem5dOverlayChunk = 11;
const uint kScene8020InventoryItemOverlayDescriptorCount = 8;
const uint kScene8020InventoryItem5dOverlayDescriptorCount = 0x0d;
const byte kScene8020ForegroundObjectOriginalItem = 3;
const byte kScene8020ChickSceneItem = 4;
const byte kScene8020SecondaryObjectItem = 5;
const byte kScene8020InventoryItem6c = 0x6c;
const byte kScene8020InventoryItem5d = 0x5d;
const byte kScene8020InventoryItem4f = 0x4f;
const byte kScene8020OverlayHookPickup6c = 1;
const byte kScene8020OverlayHookPickup5d = 2;
const byte kScene8020OverlayHookRemove6c = 3;
const byte kScene8020TransformationHook = 4;
const byte kScene8020OverlayHookTransformationReverse = 5;

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

const uint kScene8020ForegroundLayer = 0;
const SceneLayerSpec kScene8020LayerSpecs[] = {
	{kSceneAnimationScenePlaced, kScene8020ForegroundChunk,
		kScene8020ForegroundDescriptorCount, kScene8020ForegroundFrameMap,
		ARRAYSIZE(kScene8020ForegroundFrameMap), true, 0}
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
		_foregroundChannel(),
		_secondaryAmbientChannel(),
		_originalColorToItemMap(),
		_foregroundAnimationState(0),
		_foregroundRepeatCount(0),
		_previousPrimaryAmbientCue(0),
		_previousSecondaryAmbientCue(0),
		_foregroundSequenceLocked(false) {
	_sceneLayers.configure(kScene8020LayerSpecs);
}

void Scene8020::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetForegroundLayer();
	setActiveActorPose(kScene8020EntryStartX, kScene8020EntryStartY, kScene8020EntryFacing);
}

bool Scene8020::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

bool Scene8020::shouldRunExitSideEffectsAfterLoop() const {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	return !Engine::shouldQuit() && stateId != 0xff && !isMainFlowStateInScene(stateId);
}

void Scene8020::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
	_soundBank0.stop();
	stopAmbientSoundCues();
}

void Scene8020::drawCustomBackgroundComposite(int activeWorldX, int activeWorldY) {
	(void)activeWorldX;
	(void)activeWorldY;
	if (_vm->gameState().scene8020ForegroundObjectState != 2)
		drawResourceSpriteLayer(_sceneLayers.layer(kScene8020ForegroundLayer));
}

void Scene8020::runCustomEntrySequence() {
	setActiveActorPose(kScene8020EntryStartX, kScene8020EntryStartY, kScene8020EntryFacing);
	drawPlayableComposite();
	if (fadePaletteFromBlack())
		return;
	if (!walkActiveActorTo(kScene8020EntryTargetX, kScene8020EntryTargetY,
			kScene8020EntryFacing, 0, false))
		return;

	GameplayState &state = _vm->gameState();
	if (!state.seenScene8020EntryLine) {
		beginSecondarySpeechLine(0, 0);
		state.seenScene8020EntryLine = true;
	}
}

void Scene8020::advanceCustomGameplayLoop(uint32 delta) {
	if (!_foregroundSequenceLocked && _vm->gameState().scene8020ForegroundObjectState != 2)
		advanceForegroundLayer(delta);
}

bool Scene8020::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a camino (go to path): return to lake unless the egg belongs in the nest.
		if (_vm->gameState().scene8020ForegroundObjectState == 2)
			beginSecondarySpeechLine(1, 0);
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
	case 310: // Usar huevo con gorro de aviador (use egg with aviator cap): hatch it in the nest.
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

	int lowerY = targetY;
	int upperY = targetY;
	do {
		if (lowerY < HollywoodEngine::kSceneBufferHeight - 1) {
			++lowerY;
			if (isWalkableAt(targetX, lowerY))
				targetY = lowerY;
		}
		if (upperY > 0) {
			--upperY;
			if (isWalkableAt(targetX, upperY))
				targetY = upperY;
		}
		if (lowerY == HollywoodEngine::kSceneBufferHeight - 1 && upperY == 0)
			break;
	} while (!isWalkableAt(targetX, targetY));

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
				kScene8020ChickSceneItem);
		} else if (state == 2) {
			replaceColorMapItemFromOriginal(kScene8020ForegroundObjectOriginalItem, 0);
		}
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

	const byte foregroundState = _vm->gameState().scene8020ForegroundObjectState;
	if (foregroundState == 1) {
		const ScenePoint interactionPoint = { 0x13d, 0x00fc };
		SceneActionTarget target = _hotspots.actionTarget(2);
		_hotspots.setActionTarget(2, interactionPoint, target.approachPoint);
		target = _hotspots.actionTarget(4);
		_hotspots.setActionTarget(4, interactionPoint, target.approachPoint);
	}
	_hotspots.setVerbMovementModeByGlobalRecordIndex(9, foregroundState == 2 ? 0 : 1);
	if (_vm->restoredContentEnabled()) {
		for (byte itemId = GameplayState::kInventoryFirstSlot;
				itemId < GameplayState::kInventoryOwnerSlotStride; ++itemId) {
			if (_hotspots.relationActionRecord(itemId,
					kScene8020ChickSceneItem, 2).actionHandlerId == 0) {
				_hotspots.setRelationActionHandler(itemId,
					kScene8020ChickSceneItem, 2, 312);
			}
		}
	}
	return handled;
}

void Scene8020::handleAnimationFrameHook(byte hookId, uint frame) {
	GameplayState &state = _vm->gameState();
	if (hookId == kScene8020OverlayHookPickup6c && frame == 4) {
		if (_sceneChunkTable.isValidChunk(kScene8020Pickup6cPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene8020Pickup6cPatchChunk],
				_baseFramebuffer);
		state.scene8020ForegroundObjectState = 2;
		return;
	}

	if (hookId == kScene8020OverlayHookPickup5d && frame == 7) {
		if (_sceneChunkTable.isValidChunk(kScene8020Pickup5dPatchChunk))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[kScene8020Pickup5dPatchChunk],
				_baseFramebuffer);
		return;
	}

	if (hookId == kScene8020OverlayHookRemove6c && frame == 5) {
		state.scene8020ForegroundObjectState = 0;
		return;
	}

	if (hookId == kScene8020OverlayHookTransformationReverse && frame == 5) {
		state.scene8020ForegroundObjectState = 0;
		_foregroundAnimationState = 2;
		_foregroundRepeatCount = 100;
		return;
	}

	if (hookId == kScene8020TransformationHook) {
		if (frame == 0x0c) {
			_soundBank0.playSample(0x1b, 100);
		} else if (frame == 0x13) {
			_soundBank0.playSample(0x1c, 50);
			if (_sceneChunkTable.isValidChunk(kScene8020SecondaryPatchChunk)) {
				drawResourceBlockList(_resourceArena,
					_resourceChunkOffsets[kScene8020SecondaryPatchChunk], _baseFramebuffer);
			}
		}
	}
}

void Scene8020::resetForegroundLayer() {
	_sceneLayers.reset();
	_foregroundChannel.reset(0, kScene8020FrameMillis);
	_secondaryAmbientChannel.reset(0, kScene8020AmbientCheckMillis);
	_foregroundSequenceLocked = false;

	const byte state = _vm->gameState().scene8020ForegroundObjectState;
	if (state == 1) {
		_sceneLayers.layer(kScene8020ForegroundLayer).reset(0x17);
		_foregroundAnimationState = 3;
	} else {
		_sceneLayers.layer(kScene8020ForegroundLayer).reset(0);
		_foregroundAnimationState = 0;
	}
	_foregroundRepeatCount = 0;
	_previousPrimaryAmbientCue = 0;
	_previousSecondaryAmbientCue = 0;
}

void Scene8020::advanceForegroundLayer(uint32 delta) {
	ResourceSpriteLayer &foregroundLayer = _sceneLayers.layer(kScene8020ForegroundLayer);
	const uint frameCount = _foregroundChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		switch (_foregroundAnimationState) {
		case 0:
			if (_random.getRandomNumber(29) == 0) {
				_foregroundRepeatCount = (byte)(_random.getRandomNumber(7) + 3);
				_foregroundAnimationState = (byte)(_random.getRandomBit() ? 2 : 1);
				_soundBank0.playSample(_foregroundAnimationState == 1 ? 0x19 : 0x1a,
					_foregroundAnimationState == 1 ? 30 : 100);
			}
			break;
		case 1:
			if (foregroundLayer.frameIndex == 3) {
				foregroundLayer.setFrame(0);
				if (_foregroundRepeatCount == 0) {
					_foregroundAnimationState = 0;
					_soundBank0.stop();
				} else {
					--_foregroundRepeatCount;
					if (!_soundBank0.isPlaying())
						_soundBank0.playSample(0x19, 30);
				}
			} else {
				foregroundLayer.setFrame(foregroundLayer.frameIndex + 1);
			}
			break;
		case 2:
			if (foregroundLayer.frameIndex == 7) {
				foregroundLayer.setFrame(0);
				if (_foregroundRepeatCount == 0) {
					_foregroundAnimationState = 0;
					_soundBank0.stop();
				} else {
					--_foregroundRepeatCount;
					if (!_soundBank0.isPlaying())
						_soundBank0.playSample(0x1a, 100);
				}
			} else {
				foregroundLayer.setFrame(foregroundLayer.frameIndex + 1);
			}
			break;
		case 3:
			if (_random.getRandomNumber(29) == 0) {
				_foregroundAnimationState = 5;
			} else if (_random.getRandomNumber(9) == 0) {
				foregroundLayer.setFrame(0x18);
				_foregroundAnimationState = 4;
			}
			break;
		case 4:
			foregroundLayer.setFrame(0x17);
			_foregroundAnimationState = 3;
			break;
		case 5:
			if (foregroundLayer.frameIndex == 0x21) {
				foregroundLayer.setFrame(0x17);
				_foregroundAnimationState = 3;
			} else {
				foregroundLayer.setFrame(foregroundLayer.frameIndex + 1);
				if (foregroundLayer.frameIndex == 0x19)
					_soundBank0.playSample(0x18, 100);
			}
			break;
		default:
			_foregroundAnimationState = 0;
			foregroundLayer.setFrame(0);
			break;
		}
	}
}

void Scene8020::advanceAmbientAudio(uint32 delta) {
	if (!_ambientSoundBank0.isPlaying()) {
		byte cue = 0;
		do {
			cue = (byte)(0x12 + _random.getRandomNumber(5));
		} while (cue == _previousPrimaryAmbientCue);
		_previousPrimaryAmbientCue = cue;
		_ambientSoundBank0.playSample(cue, 50);
	}

	const uint checks = _secondaryAmbientChannel.consumeFrames(delta);
	for (uint check = 0; check < checks; ++check) {
		SoundBank0Player &player = _additionalAmbientSoundBank0Slots[0];
		if (!player.isPlaying() && _random.getRandomNumber(29) == 0) {
			byte cue = 0;
			do {
				cue = (byte)(0x0c + _random.getRandomNumber(5));
			} while (cue == _previousSecondaryAmbientCue);
			_previousSecondaryAmbientCue = cue;
			player.playSample(cue, 50);
		}

		if (!_vm->gameplayMusic()->isPlaying() && _random.getRandomNumber(49) == 0) {
			_vm->gameState().currentAmbientMusicCueId = 0x0c;
			_vm->gameplayMusic()->playMusicCue(0x0c, 100);
		}
	}
}

bool Scene8020::waitForForegroundAnimationIdle() {
	while (_foregroundAnimationState != 0) {
		if (waitSceneMillis(kScene8020FrameMillis, false))
			return false;
	}
	drawPlayableComposite();
	presentFrame();
	return !Engine::shouldQuit() && !_vm->isSceneRestartRequested();
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
	_sceneLayers.layer(kScene8020ForegroundLayer).setFrame(0);
	_foregroundAnimationState = 0;
	_foregroundRepeatCount = 0;
	_soundBank0.stop();

	runActorReplacement(ActionOverlaySpec(kScene8020InventoryItem6cOverlayChunk, kScene8020InventoryItemOverlayDescriptorCount,
		kScene8020Pickup6cFrameMap, ARRAYSIZE(kScene8020Pickup6cFrameMap), kScene8020FrameMillis)
		.hookAt(4, kScene8020OverlayHookPickup6c)
		.unskippable()
		.noFinalFrameDelay()
		.noRedrawAtEnd());
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	addInventoryItem(kScene8020InventoryItem6c);
	_soundBank0.playSample(1, 100);
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(4, 0);
}

void Scene8020::runPickupInventoryItem5dSequence() {
	runActorReplacement(ActionOverlaySpec(kScene8020InventoryItem5dOverlayChunk, kScene8020InventoryItem5dOverlayDescriptorCount,
		kScene8020Pickup5dFrameMap, ARRAYSIZE(kScene8020Pickup5dFrameMap), kScene8020FrameMillis)
		.hookAt(7, kScene8020OverlayHookPickup5d)
		.unskippable()
		.noFinalFrameDelay()
		.noRedrawAtEnd());
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	addInventoryItem(kScene8020InventoryItem5d);
	_soundBank0.playSample(1, 100);
	_vm->gameState().scene8020SecondaryObjectVisible = false;
	applySceneStateToHotspotsAndPatches(2);
	beginSecondarySpeechLine(0x14, (byte)_random.getRandomNumber(4));
}

void Scene8020::runRemoveInventoryItem6cSequence() {
	runActorReplacement(ActionOverlaySpec(kScene8020InventoryItem6cOverlayChunk, kScene8020InventoryItemOverlayDescriptorCount,
		kScene8020Reverse6cFrameMap, ARRAYSIZE(kScene8020Reverse6cFrameMap), kScene8020FrameMillis)
		.hookAt(5, kScene8020OverlayHookRemove6c)
		.unskippable()
		.noFinalFrameDelay()
		.noRedrawAtEnd());
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	removeInventoryItem(kScene8020InventoryItem6c);
	_soundBank0.playSample(1, 100);
	applySceneStateToHotspotsAndPatches(1);
	beginSecondarySpeechLine(1, 1);
}

void Scene8020::runForegroundTransformationSequence() {
	beginSecondarySpeechLine(8, 0);
	beginSecondarySpeechLine(8, 1);
	if (!walkActiveActorTo(0x118, 0x00eb, 5, 0, false))
		return;

	runActorReplacement(ActionOverlaySpec(kScene8020InventoryItem6cOverlayChunk, kScene8020InventoryItemOverlayDescriptorCount,
		kScene8020Reverse6cFrameMap, ARRAYSIZE(kScene8020Reverse6cFrameMap), kScene8020FrameMillis)
		.hookAt(5, kScene8020OverlayHookTransformationReverse)
		.unskippable()
		.noFinalFrameDelay()
		.noRedrawAtEnd());
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	_foregroundAnimationState = 2;
	_foregroundRepeatCount = 4;
	_soundBank0.playSample(0x1a, 100);
	removeInventoryItem(kScene8020InventoryItem6c);
	removeInventoryItem(kScene8020InventoryItem4f);
	_soundBank0.playSample(1, 100);

	if (!walkActiveActorTo(0x15e, 0x0104, 0xff, 0, false))
		return;
	if (!walkActiveActorTo(0x15e, 0x0104, 5, 0, false))
		return;
	if (!waitForForegroundAnimationIdle())
		return;

	_sceneLayers.setLayerFrame(kScene8020ForegroundLayer, 7);
	_soundBank0.playSample(0x1a, 100);
	_foregroundSequenceLocked = true;
	playAndPresentAnimationFrames(_sceneLayers, kScene8020ForegroundLayer,
		AnimationFrameRange(7, 0x17, kScene8020FrameMillis)
			.hookEveryFrame(kScene8020TransformationHook)
			.unskippable()
			.noFinalFrameDelay());
	_foregroundSequenceLocked = false;
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	GameplayState &state = _vm->gameState();
	state.scene8020ForegroundObjectState = 1;
	state.scene8020SecondaryObjectVisible = true;
	applySceneStateToHotspotsAndPatches(1);
	applySceneStateToHotspotsAndPatches(2);
	resetForegroundLayer();
	beginSecondarySpeechLine(8, 2);
}

} // End of namespace Hollywood
