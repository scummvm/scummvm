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

#include "hollywood/scenes/playable/scene1040.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene1040LeftEntryState = 0x0410;
const uint16 kScene1040CloakroomReturnState = 0x0411;
const uint16 kScene1040BanquetEntryState = 0x0412;
const uint16 kScene1040ExitState1050 = 0x041a;
const uint16 kScene1040ExitState1030LeftEntry = 0x0407;
const uint16 kScene1040ExitState1060 = 0x0424;
const uint16 kScene1040ViewportXOffset = 0x0130;
const uint16 kScene1040ViewportMinXOffset = 0x0130;
const uint16 kScene1040ViewportMaxXOffset = 0x0140;
const uint kScene1040ActorBankTableEntry = 0x0000;
const uint kScene1040ActorPaletteTableEntry = 0x00cc;
const uint kScene1040Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene1040SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene1040FrameMillis = 75;
const uint kScene1040GorillaDescriptorCount = 8;
const uint kScene1040DoorOverlayDescriptorCount = 4;
const uint kScene1040BalloonOverlayDescriptorCount = 0x0d;
const uint kScene1040CordOverlayDescriptorCount = 0x19;
const uint kScene1040GorillaCordOverlayChunk = 0x10;
const uint kScene1040GorillaCordOverlayDescriptorCount = 0x0b;
const int kScene1040ForegroundYThreshold = 0x15f;
const byte kScene1040FirstAmbientSoundCue = 0x25;
const byte kScene1040AmbientSoundCueCount = 7;
const byte kScene1040FirstAmbientMusicCue = 0x0b;
const byte kScene1040AmbientMusicCueCount = 5;
const byte kScene1040AmbientSoundProbabilityModulus = 25;
const byte kScene1040AmbientMusicProbabilityModulus = 50;

const byte kScene1040GorillaFrameMap[] = {
	0, 0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3,
	3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0, 4,
	5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5,
	4
};

const byte kScene1040DoorFrameMap[] = { 1, 1, 2, 3 };

const byte kScene1040BalloonFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12
};

const byte kScene1040CordFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22,
	23, 24
};

const byte kScene1040GorillaCordSetupFrameMap[] = {
	9, 9, 8, 7, 6, 5, 4, 3,
	2, 1, 0, 1, 2, 1, 0, 1,
	2, 1, 0, 1, 2, 1, 0, 1,
	2, 1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10
};

const uint kScene1040GorillaLayer = 0;
const SceneLayerSpec kScene1040LayerSpecs[] = {
	{kSceneAnimationBehindActors, 7, kScene1040GorillaDescriptorCount,
		kScene1040GorillaFrameMap, ARRAYSIZE(kScene1040GorillaFrameMap), true, 0}
};

static PlayableSceneConfig scene1040Config() {
	PlayableSceneConfig config(1040,
		SceneResourceLayout(17, 5, 16),
		SceneViewport(kScene1040ViewportXOffset, kScene1040ViewportMinXOffset, kScene1040ViewportMaxXOffset),
		SceneActorPose(0x163, 0x130, 2));
	config.setActorResources(kScene1040ActorBankTableEntry, kScene1040ActorPaletteTableEntry);
	config.setTextResources(kScene1040Resource003RowsOffsetIndex, kScene1040SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 6;
	return config;
}

Scene1040::Scene1040(HollywoodEngine *vm) :
		PlayableScene(vm, scene1040Config()),
		_gorillaChannel(),
		_gorillaLongSequence(false) {
	_sceneLayers.configure(kScene1040LayerSpecs);
}

void Scene1040::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	_gorillaChannel.reset(0, kScene1040FrameMillis);
	_gorillaLongSequence = false;

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId == kScene1040CloakroomReturnState) {
		_activeActorWorldX = 0x284;
		_activeActorWorldY = 0x13d;
		_activeActorFacing = 4;
	} else if (stateId == kScene1040BanquetEntryState) {
		_activeActorWorldX = 0x193;
		_activeActorWorldY = 0x170;
		_activeActorFacing = 5;
	} else {
		_activeActorWorldX = 0x163;
		_activeActorWorldY = 0x130;
		_activeActorFacing = 2;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene1040::drawCustomForegroundComposite(int activeWorldX, int activeWorldY) {
	(void)activeWorldX;
	drawForegroundBlocks(activeWorldY);
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene1040::runCustomEntrySequence() {
	const uint16 stateId = _vm->gameState().mainFlowStateId;
	if (stateId != kScene1040LeftEntryState &&
		stateId != kScene1040CloakroomReturnState &&
		stateId != kScene1040BanquetEntryState) {
		PlayableScene::runCustomEntrySequence();
		return;
	}

	if (stateId == kScene1040LeftEntryState) {
		runEntryPath(0x118, 0x10b, 2, 0x163, 0x130);
		return;
	}

	if (stateId == kScene1040CloakroomReturnState) {
		_soundBank0.playSample(4, 100);
		setActiveActorPose(0x284, 0x13d, 4);
		drawPlayableComposite();
		presentFrame();
		return;
	}

	runEntryPath(0x0d8, 0x1a1, 5, 0x193, 0x170);
	if (!_vm->gameState().scene1040EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().scene1040EntryLineSeen = true;
	}
}

void Scene1040::prepareCustomGameplayLoop() {
	_gorillaChannel.reset(_sceneLayers.layerFrame(kScene1040GorillaLayer), kScene1040FrameMillis);
}

void Scene1040::advanceCustomGameplayLoop(uint32 delta) {
	advanceGorillaAnimation(delta);
}

bool Scene1040::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar puerta (look at door): state-dependent line.
		beginSecondarySpeechLine(1, _vm->gameState().scene1040CloakroomDoorOpened ? 1 : 0);
		return true;
	case 302: // Usar/abrir puerta (use/open door): enter cloakroom.
		runDoorToCloakroomAction();
		return true;
	case 303: // Ir a salón (go to lounge): next room.
		_vm->gameState().mainFlowStateId = kScene1040ExitState1060;
		return true;
	case 304: // Mirar salón (look at lounge).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 305: // Coger cordón (take cord).
		handleCordPickup();
		return true;
	case 306: // Mirar cordón (look at cord).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Coger globo (take balloon).
		handleBalloonPickup();
		return true;
	case 308: // Mirar globo (look at balloon).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 309: // Mirar estatua (look at statue).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 310: // Ir a escalera (go to stairs).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 311: // Mirar escalera (look at stairs).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 312: // Mirar mesa (look at table).
		beginSecondarySpeechLine(9, 0);
		return true;
	case 313: // Mirar papelera (look at wastebasket).
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Hablar con enorme gorila intimidatorio (talk to huge intimidating gorilla).
		beginSecondarySpeechLine(11, 0);
		return true;
	case 315: // Mirar enorme gorila intimidatorio (look at huge intimidating gorilla).
		beginSecondarySpeechLine(12, 0);
		return true;
	case 316: // Usar bisturí/navaja con cordón (use scalpel/knife with cord).
		handleGorillaCordSetup();
		return true;
	case 317: // Ir a terraza / back to banquet room.
		runGorillaExitBackToBanquetRoom();
		return true;
	default:
		return false;
	}
}

bool Scene1040::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	GameplayState &state = _vm->gameState();
	if (_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize &&
			_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
		const byte cordState = MIN<byte>(state.scene1040GorillaCordState, 2);
		for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
			const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
			byte item = originalItem;
			if (cordState == 0) {
				if (originalItem == 0x0b)
					item = 5;
			} else if (cordState == 1) {
				if (originalItem == 0x0b)
					item = 3;
			} else {
				if (originalItem == 3)
					item = 0;
				else if (originalItem == 0x0b)
					item = 5;
			}

			if (state.scene1040BalloonTaken && item == 4)
				item = 0;
			_paletteMask[kSceneColorToItemMap + i] = item;
		}
	}

	if (state.scene1040GorillaCordState == 1 && _sceneChunkTable.isValidChunk(13))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[13], _baseFramebuffer);
	else if (state.scene1040GorillaCordState >= 2 && _sceneChunkTable.isValidChunk(14))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[14], _baseFramebuffer);

	if (state.scene1040BalloonTaken && _sceneChunkTable.isValidChunk(10))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	setCordActionTarget();
	return true;
}

AmbientAudioProfile Scene1040::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(kScene1040FirstAmbientSoundCue,
		kScene1040AmbientSoundCueCount, 15, kScene1040AmbientSoundProbabilityModulus,
		kScene1040FirstAmbientMusicCue, kScene1040AmbientMusicCueCount, 100,
		kScene1040AmbientMusicProbabilityModulus);
}

void Scene1040::runDoorToCloakroomAction() {
	BlockingSequence(*this)
		.actorReplacement(8, kScene1040DoorOverlayDescriptorCount,
			kScene1040DoorFrameMap, ARRAYSIZE(kScene1040DoorFrameMap), kScene1040FrameMillis)
		.sound(3)
		.commit(_vm->gameState().scene1040CloakroomDoorOpened, true)
		.commit(_vm->gameState().mainFlowStateId, kScene1040ExitState1050);
}

void Scene1040::handleCordPickup() {
	GameplayState &state = _vm->gameState();
	if (state.scene1040GorillaCordState == 0) {
		beginSecondarySpeechLine(3, 0);
		return;
	}

	if (hasInventoryItem(0x1b))
		return;

	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(12, kScene1040CordOverlayDescriptorCount,
		kScene1040CordFrameMap, ARRAYSIZE(kScene1040CordFrameMap), kScene1040FrameMillis)
		.resourcePatchAt(0, 14))
		.commit(state.scene1040GorillaCordState, (byte)2)
		.framebufferPatch(2);
	addInventoryItem(0x1b);
	sequence.sound(1);
	dispatchGenericSceneAction(21);
}

void Scene1040::handleBalloonPickup() {
	if (hasInventoryItem(0x1c))
		return;

	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(9, kScene1040BalloonOverlayDescriptorCount,
		kScene1040BalloonFrameMap, ARRAYSIZE(kScene1040BalloonFrameMap), kScene1040FrameMillis)
		.patchAt(7, 3))
		.commit(_vm->gameState().scene1040BalloonTaken, true)
		.framebufferPatch(3);
	addInventoryItem(0x1c);
	sequence.sound(1);
	dispatchGenericSceneAction(21);
}

void Scene1040::handleGorillaCordSetup() {
	GameplayState &state = _vm->gameState();
	if (state.scene1040GorillaCordState == 1) {
		beginSecondarySpeechLine(13, 1);
		return;
	}
	if (state.scene1040GorillaCordState >= 2)
		return;

	BlockingSequence(*this)
		.actorReplacement(ActionOverlaySpec(kScene1040GorillaCordOverlayChunk,
			kScene1040GorillaCordOverlayDescriptorCount,
			kScene1040GorillaCordSetupFrameMap, ARRAYSIZE(kScene1040GorillaCordSetupFrameMap),
			kScene1040FrameMillis)
			.loopingSoundAt(8, 0x23, 75)
			.stopSoundAt(28))
		.stopSound()
		.secondarySpeech(13, 0)
		.commit(state.scene1040GorillaCordState, (byte)1)
		.framebufferPatch(2);
}

void Scene1040::runGorillaExitBackToBanquetRoom() {
	_vm->gameState().mainFlowStateId = kScene1040ExitState1030LeftEntry;
}

void Scene1040::drawForegroundBlocks(int activeWorldY) {
	if (activeWorldY >= kScene1040ForegroundYThreshold)
		return;

	const uint chunkIndex = _vm->gameState().scene1040GorillaCordState < 2 ? 6 : 15;
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

void Scene1040::advanceGorillaAnimation(uint32 delta) {
	ResourceSpriteLayer &gorillaLayer = _sceneLayers.layer(kScene1040GorillaLayer);
	const uint frameCount = _gorillaChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		if (!_gorillaLongSequence) {
			if (gorillaLayer.frameIndex < 0x15) {
				if (gorillaLayer.frameIndex == 0x0d && _random.getRandomNumber(14) == 0) {
					_gorillaLongSequence = true;
					gorillaLayer.setFrame(0x16);
				} else {
					if (gorillaLayer.frameIndex == 0x0d)
						_soundBank0.playSample((byte)(0x12 + _random.getRandomNumber(2)), 50);
					gorillaLayer.setFrame(gorillaLayer.frameIndex + 1);
				}
			} else {
				gorillaLayer.setFrame(0);
			}
		} else if (gorillaLayer.frameIndex < 0x30) {
			gorillaLayer.setFrame(gorillaLayer.frameIndex + 1);
		} else {
			gorillaLayer.setFrame(6);
			_gorillaLongSequence = false;
		}
	}
}

void Scene1040::setCordActionTarget() {
	ScenePoint interactionPoint;
	ScenePoint approachPoint;
	if (_vm->gameState().scene1040GorillaCordState == 1) {
		interactionPoint.x = 0x23f;
		interactionPoint.y = 0x16f;
	} else {
		interactionPoint.x = 0x2c5;
		interactionPoint.y = 0x19a;
	}
	approachPoint = interactionPoint;
	_hotspots.setActionTarget(3, interactionPoint, approachPoint);
}

} // End of namespace Hollywood
