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

#include "hollywood/scenes/playable/scene3010.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene3010ArchiveName = "RESOURCE.C01";
const char *const kScene3010MusicArchiveName = "RESOURCE.M03";
const char *const kScene3010SoundArchiveName = "RESOURCE.S03";
const uint kScene3010InitialRequiredChunkCount = 14;
const uint kScene3010ArenaFirstChunk = 5;
const uint kScene3010ArenaLastChunk = 13;
const uint kScene3010StageIndex = 301;
const uint16 kScene3010FirstState = 0x0bc2;
const uint16 kScene3010LastState = 0x0bcb;
const uint16 kScene3010EntryFromScene3020State = 0x0bc3;
const uint16 kScene3010EntryFromPathState = 0x0bc4;
const uint16 kScene3020State = 0x0bcc;
const uint16 kScene3050State = 0x0bea;
const uint16 kScene3010ViewportXOffset = 0x0078;
const uint kScene3010ActorBankTableEntry = 0x0000;
const uint kScene3010ActorPaletteTableEntry = 0x00cc;
const uint kScene3010Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene3010SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene3010WindmillFrameMillis = 125;
const uint32 kScene3010ForestIdleFrameMillis = 75;
const uint kScene3010WindmillDescriptorCount = 0x1e;
const uint kScene3010ForestIdleDescriptorCount = 0x0f;
const uint kScene3010ExitDescriptorCount = 6;
const byte kScene3010InitialWindmillFrame = 4;

const byte kScene3010ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene3010WindmillFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29
};

const byte kScene3010ForestIdleFrameMap[] = {
	0, 1, 1, 1, 1, 1, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9, 10, 11, 12, 13, 14
};

const byte kScene3010ExitFrameMap[] = { 0, 1, 2, 3, 4, 5 };

static PlayableSceneConfig scene3010Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene3010ArchiveName;
	config.initialRequiredChunkCount = kScene3010InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene3010ArenaFirstChunk;
	config.arenaLastChunk = kScene3010ArenaLastChunk;
	config.stageIndex = kScene3010StageIndex;
	config.debugName = "Scene 3010";
	config.viewportXOffset = kScene3010ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 3;
	config.actorBankTableEntry = kScene3010ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene3010ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = kScene3010Resource003RowsOffsetIndex;
	config.speechCueDescriptorTableOffset = kScene3010SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene3010ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene3010ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene3010MusicArchiveName;
	config.soundBank0ArchiveName = kScene3010SoundArchiveName;
	config.mainFlowFirstState = kScene3010FirstState;
	config.mainFlowLastState = kScene3010LastState;
	return config;
}

Scene3010::Scene3010(HollywoodEngine *vm) :
		PlayableScene(vm, scene3010Config(), "scene3010", 0x160, 0x1ca, 1, 0xfd, 0xfb),
		_windmillChannel(),
		_forestIdleChannel(),
		_windmillLayer(),
		_forestIdleLayer(),
		_forestIdleState(0) {
	_windmillLayer.configure(7, kScene3010WindmillDescriptorCount,
		kScene3010WindmillFrameMap, ARRAYSIZE(kScene3010WindmillFrameMap));
	_forestIdleLayer.configure(9, kScene3010ForestIdleDescriptorCount,
		kScene3010ForestIdleFrameMap, ARRAYSIZE(kScene3010ForestIdleFrameMap));
}

bool Scene3010::shouldLoadArenaChunk(uint index) const {
	return index < 10 || index == 13;
}

bool Scene3010::hasCustomPreviewState() const {
	return true;
}

void Scene3010::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	switch (_vm->gameState().mainFlowStateId) {
	case kScene3010EntryFromScene3020State:
		_activeActorWorldX = 0x278;
		_activeActorWorldY = 0x15d;
		_activeActorFacing = 4;
		break;
	case kScene3010EntryFromPathState:
		_activeActorWorldX = 0x129;
		_activeActorWorldY = 0x13f;
		_activeActorFacing = 2;
		break;
	default:
		_activeActorWorldX = 0x160;
		_activeActorWorldY = 0x1ca;
		_activeActorFacing = 1;
		break;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene3010::hasCustomComposite() const {
	return true;
}

void Scene3010::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;
	(void)activeWorldX;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_windmillLayer);
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawForegroundBlocks(activeWorldY);
	drawResourceSpriteLayer(_forestIdleLayer);
}

bool Scene3010::hasCustomEntrySequence() const {
	return _vm->gameState().mainFlowStateId >= kScene3010FirstState &&
		_vm->gameState().mainFlowStateId <= kScene3010EntryFromPathState;
}

void Scene3010::runCustomEntrySequence() {
	switch (_vm->gameState().mainFlowStateId) {
	case kScene3010FirstState:
		runEntryFromChapterStart();
		break;
	case kScene3010EntryFromScene3020State:
		runEntryFromScene3020();
		break;
	case kScene3010EntryFromPathState:
		runEntryFromPath();
		break;
	default:
		drawPlayableComposite();
		presentFrame();
		break;
	}
}

bool Scene3010::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene3010::advanceCustomGameplayLoop(uint32 delta) {
	if (_vm->gameState().windmillBladesMoving)
		advanceWindmillLayer(delta);
	advanceForestIdleLayer(delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene3010::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Original slot 00: scene row 1 speech.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 302: // Original slot 01: short overlay, then transition toward scene 3050.
		runExitToScene3050();
		return true;
	case 303: // Original slot 02: scene row 2, frame depends on local C01 state.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Original slot 03: scene row 3 speech.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Original slot 04: transition toward scene 3020.
		beginSecondarySpeechLine(4, 0);
		_vm->gameState().mainFlowStateId = kScene3020State;
		return true;
	case 306: // Original slot 05: scene row 5 speech.
		beginSecondarySpeechLine(5, 0);
		return true;
	case 307: // Original slot 06: scene row 6, frame depends on local C01 state.
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Original slot 07: scene row 7 speech.
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Original slot 08: scene row 8 speech.
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Original slot 09: scene row 9 speech.
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Original slot 10: gated transition toward scene 3040.
		beginSecondarySpeechLine(10, 2);
		return true;
	default:
		return false;
	}
}

void Scene3010::resetAnimationLayers() {
	_windmillChannel.reset(kScene3010InitialWindmillFrame, kScene3010WindmillFrameMillis);
	_forestIdleChannel.reset(0, kScene3010ForestIdleFrameMillis);
	_forestIdleState = 0;
	_windmillLayer.visible = true;
	_forestIdleLayer.visible = true;
	_windmillLayer.reset(kScene3010InitialWindmillFrame);
	_forestIdleLayer.reset(0);
}

void Scene3010::advanceWindmillLayer(uint32 delta) {
	const uint frameCount = _windmillChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		_windmillChannel.frameIndex = _windmillChannel.frameIndex + 1 < ARRAYSIZE(kScene3010WindmillFrameMap) ?
			_windmillChannel.frameIndex + 1 : 0;
		_windmillLayer.setFrame(_windmillChannel.frameIndex);
	}
}

void Scene3010::advanceForestIdleLayer(uint32 delta) {
	const uint frameCount = _forestIdleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame) {
		if (_forestIdleState == 0) {
			if (_forestIdleChannel.frameIndex == 0 && _random.getRandomNumber(999) == 0) {
				_forestIdleState = 1;
				_forestIdleChannel.frameIndex = 3;
			} else if (_random.getRandomNumber(24) == 0) {
				_forestIdleState = 2;
				_forestIdleChannel.frameIndex = 1;
			}
		} else if (_forestIdleState == 1) {
			if (_forestIdleChannel.frameIndex < 20)
				++_forestIdleChannel.frameIndex;
			else
				_forestIdleState = 3;
		} else if (_forestIdleState == 2) {
			if (_forestIdleChannel.frameIndex < 6) {
				++_forestIdleChannel.frameIndex;
			} else {
				_forestIdleState = 0;
				_forestIdleChannel.frameIndex = 0;
			}
		}
		_forestIdleLayer.setFrame(_forestIdleChannel.frameIndex);
	}
}

void Scene3010::drawForegroundBlocks(int activeWorldY) {
	const uint chunkIndex = activeWorldY < 0x14c ? 6 : 5;
	if (_sceneChunkTable.isValidChunk(chunkIndex))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunkIndex], _sceneFramebuffer);
}

void Scene3010::runEntryFromChapterStart() {
	runEntryPath(0x13d, 0x1df, 1, 0x160, 0x1ca);
	if (!_vm->gameState().seenScene3010EntryLine) {
		beginSecondarySpeechLine(0, 0);
		_vm->gameState().seenScene3010EntryLine = true;
	}
}

void Scene3010::runEntryFromScene3020() {
	runEntryPath(0x278, 0x15d, 4, 0x278, 0x15d);
}

void Scene3010::runEntryFromPath() {
	_soundBank0.playSample(4, 100);
	runEntryPath(0x0ef, 0x139, 2, 0x129, 0x13f);
}

void Scene3010::runExitToScene3050() {
	runActionOverlay(8, kScene3010ExitDescriptorCount, kScene3010ExitFrameMap,
		ARRAYSIZE(kScene3010ExitFrameMap), kScene3010ForestIdleFrameMillis);
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene3050State;
}

} // End of namespace Hollywood
