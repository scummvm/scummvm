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

#include "hollywood/scenes/playable/scene7070.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene7070ShortEntryState = 0x1b9f;
const uint16 kScene7070ReturnState = 0x1ba0;
const uint16 kScene7070BackToG06State = 0x1b95;
const uint16 kScene7070ExitToG08State = 0x1ba8;
const uint16 kScene7070ExitToG09State = 0x1bb2;
const uint16 kScene7070ViewportXOffset = 0x68;
const int kScene7070EntryFromG06StartX = 0x37a;
const int kScene7070EntryFromG06StartY = 0x1a1;
const byte kScene7070EntryFromG06Facing = 5;
const int kScene7070EntryFromG06TargetX = 0x2b2;
const int kScene7070EntryFromG06TargetY = 0x177;
const int kScene7070ReturnEntryStartX = 0x174;
const int kScene7070ReturnEntryStartY = 0x114;
const byte kScene7070ReturnEntryFacing = 4;
const int kScene7070ReturnEntryTargetX = 0x14a;
const int kScene7070ReturnEntryTargetY = 0x139;
const uint16 kScene7070Chunk7DescriptorCount = 10;
const uint16 kScene7070Chunk8DescriptorCount = 4;
const uint16 kScene7070Chunk12DescriptorCount = 0x24;
const uint32 kScene7070OverlayFrameMillis = 75;
const byte kScene7070ExitDoorFrameMap[] = {
	0, 0, 1, 2, 3
};
const byte kScene7070Chunk12ItemFrameMap[] = {
	0, 0x21, 0x20, 0x1f, 0x23, 0x22, 0x16, 0x17, 0x18, 0x19,
	0x1a, 0x1b, 0x1c, 0x1d, 0x1f, 0x20, 0x21, 0
};
const byte kScene7070TradeItemFrameMap[] = {
	0, 1, 2, 3, 3, 4, 5, 6, 9, 7, 8, 4, 3, 3, 2, 1, 0
};
const byte kScene7070PrimeExitDoorFrameMap[] = {
	0, 6, 5, 4, 3, 2, 1, 5, 6, 7, 8, 9, 8, 7, 8, 9,
	8, 7, 8, 9, 8, 7, 8, 9, 8, 7, 6, 5, 1, 2, 3, 4,
	5, 6, 0
};
const byte kScene7070UseItem13FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 0x1f, 0x20, 0x21, 0
};

static PlayableSceneConfig scene7070Config() {
	PlayableSceneConfig config(7070,
		SceneResourceLayout(13, 5, 12),
		SceneViewport(kScene7070ViewportXOffset),
		SceneActorPose(kScene7070EntryFromG06TargetX, kScene7070EntryFromG06TargetY, kScene7070EntryFromG06Facing));
	return config;
}

Scene7070::Scene7070(HollywoodEngine *vm) :
		PlayableScene(vm, scene7070Config()) {
}

void Scene7070::initializeCustomPreviewState() {
	_actionOverlayPlayer.reset();
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;

	if (_vm->gameState().mainFlowStateId == kScene7070ReturnState) {
		_activeActorWorldX = kScene7070ReturnEntryTargetX;
		_activeActorWorldY = kScene7070ReturnEntryTargetY;
		_activeActorFacing = kScene7070ReturnEntryFacing;
	} else {
		_activeActorWorldX = kScene7070EntryFromG06TargetX;
		_activeActorWorldY = kScene7070EntryFromG06TargetY;
		_activeActorFacing = kScene7070EntryFromG06Facing;
	}
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	_secondaryActorFrame = 0;
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene7070::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);

	drawActionOverlayLayer();

	if (activeWorldX < 0x0fa) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	} else if (activeWorldY < 0x134) {
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	}
}

void Scene7070::runCustomEntrySequence() {
	GameplayState &state = _vm->gameState();
	if (state.mainFlowStateId == kScene7070ReturnState) {
		_soundBank0.playSample(4, 100);
		runEntryPath(kScene7070ReturnEntryStartX, kScene7070ReturnEntryStartY,
			kScene7070ReturnEntryFacing, kScene7070ReturnEntryTargetX, kScene7070ReturnEntryTargetY);
		return;
	}

	if (state.mainFlowStateId == kScene7070ShortEntryState)
		_soundBank0.playSample(4, 100);

	runEntryPath(kScene7070EntryFromG06StartX, kScene7070EntryFromG06StartY,
		kScene7070EntryFromG06Facing, kScene7070EntryFromG06TargetX, kScene7070EntryFromG06TargetY);

	if (!state.seenGramophoneRoomIntro) {
		beginSecondarySpeechLine(0, 0);
		state.seenGramophoneRoomIntro = true;
	}
}

bool Scene7070::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene7070::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a pasillo (go to hallway)
		handleBackToG06();
		return true;
	case 302: // Mirar chimenea (look at fireplace)
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar gramófono (look at gramophone)
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Usar gramófono/disco (use gramophone/record)
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar mueble con discos (look at record cabinet)
		beginSecondarySpeechLine(0x17, 0);
		return true;
	case 306: // Mirar puerta (look at door)
		beginSecondarySpeechLine(5, _vm->gameState().gramophoneRoomDoorState < 2 ? 0 : 1);
		return true;
	case 307: // Usar/Abrir puerta (use/open door)
		handleExitDoorAction();
		return true;
	case 308: // Mirar mesa (look at table)
		beginSecondarySpeechLine(7, 0);
		return true;
	case 309: // Mirar trofeos (look at trophies)
		beginSecondarySpeechLine(8, 0);
		return true;
	case 310: // Coger/Usar carbón (take/use coal)
		beginSecondarySpeechLine(9, 0);
		return true;
	case 311: // Mirar carbón (look at coal)
		beginSecondarySpeechLine(10, 0);
		return true;
	case 314: // Cerrar cajón/Coger manivela (close drawer/take crank)
		beginSecondarySpeechLine(0x16, 0);
		return true;
	case 315: // Mirar manivela (look at crank)
		beginSecondarySpeechLine(0x0d, _vm->gameState().gramophoneRoomDoorState < 2 ? 0 : 1);
		return true;
	case 316: // Usar manivela (use crank)
		handleChunk12ItemAction();
		return true;
	case 317: // Usar trapo con carbón (use rag with coal)
		handleTradeItem10ForItem08();
		return true;
	case 318: // Usar trapo con carbón (use sooty rag with coal)
		beginSecondarySpeechLine(0x0f, 1);
		return true;
	case 319: // Usar tarjeta con puerta (use card with door)
		handlePrimeExitDoorAction();
		return true;
	case 320: // Usar manivela con gramófono/cajón (use crank with gramophone/drawer)
		handleUseItem13OnSceneObject();
		return true;
	case 321: // Usar llave con puerta (use key with door)
		beginSecondarySpeechLine(0x12, 0);
		return true;
	default:
		return false;
	}
}

bool Scene7070::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 0 || selector == 2 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		restoreBaseFramebufferFromOriginal();

		if (_vm->gameState().gramophoneCrankState != 0) {
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[10], _baseFramebuffer);
		}

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize &&
				_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
			for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
				if (_paletteMaskOriginal[kSceneColorToItemMap + i] == 10) {
					_paletteMask[kSceneColorToItemMap + i] =
						_vm->gameState().gramophoneCrankState != 0 ? 10 : 3;
				}
			}
		}

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}
	return true;
}

void Scene7070::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 1)
			_walkablePaletteMask[i] = 0;
	}
}

AmbientAudioProfile Scene7070::ambientAudioProfile() const {
	return createLoopingAmbientAudioProfile(50);
}

void Scene7070::runOverlaySequence(uint chunkIndex, uint descriptorCount, const byte *frameMap, uint frameMapSize,
		uint32 frameMillis, int statePatchFrame, int soundFrame, byte soundId) {
	runActorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount,
		frameMap, frameMapSize, frameMillis)
		.patchAt(statePatchFrame, 2)
		.soundAt(soundFrame, soundId));
}

void Scene7070::handleBackToG06() {
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = kScene7070BackToG06State;
}

void Scene7070::handleExitDoorAction() {
	GameplayState &state = _vm->gameState();
	if (state.gramophoneRoomDoorState == 0) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	runOverlaySequence(8, kScene7070Chunk8DescriptorCount,
		kScene7070ExitDoorFrameMap, ARRAYSIZE(kScene7070ExitDoorFrameMap),
		kScene7070OverlayFrameMillis);
	_soundBank0.playSample(3, 100);
	state.gramophoneRoomDoorState = 2;
	state.mainFlowStateId = state.gramophoneCrankState < 3 ? kScene7070ExitToG08State : kScene7070ExitToG09State;
}

void Scene7070::handleChunk12ItemAction() {
	GameplayState &state = _vm->gameState();
	runOverlaySequence(12, kScene7070Chunk12DescriptorCount,
		kScene7070Chunk12ItemFrameMap, ARRAYSIZE(kScene7070Chunk12ItemFrameMap),
		kScene7070OverlayFrameMillis, -1, 6, 0x19);

	if (state.gramophoneCrankState == 1) {
		beginSecondarySpeechLine(0x0e, 0);
		state.gramophoneCrankState = 3;
		state.hannoverCourtyardDialogueState = 1;
	} else if (state.gramophoneCrankState == 2) {
		state.gramophoneCrankState = 3;
		state.hannoverCourtyardDialogueState = 1;
	} else {
		state.gramophoneCrankState = 2;
		state.hannoverCourtyardDialogueState = 0;
	}
	applySceneStateToHotspotsAndPatches(2);
}

void Scene7070::handleTradeItem10ForItem08() {
	beginSecondarySpeechLine(0x0f, 0);
	runOverlaySequence(7, kScene7070Chunk7DescriptorCount,
		kScene7070TradeItemFrameMap, ARRAYSIZE(kScene7070TradeItemFrameMap),
		kScene7070OverlayFrameMillis);
	removeInventoryItem(0x10);
	addInventoryItem(0x08);
	_soundBank0.playSample(1, 100);
}

void Scene7070::handlePrimeExitDoorAction() {
	GameplayState &state = _vm->gameState();
	if (state.gramophoneRoomDoorState != 0) {
		handleExitDoorAction();
		return;
	}

	beginSecondarySpeechLine(0x10, 0);
	runOverlaySequence(9, kScene7070Chunk7DescriptorCount,
		kScene7070PrimeExitDoorFrameMap, ARRAYSIZE(kScene7070PrimeExitDoorFrameMap),
		kScene7070OverlayFrameMillis);
	state.gramophoneRoomDoorState = 1;
	beginSecondarySpeechLine(0x10, 1);
}

void Scene7070::handleUseItem13OnSceneObject() {
	GameplayState &state = _vm->gameState();
	state.gramophoneCrankState = 1;
	runOverlaySequence(12, kScene7070Chunk12DescriptorCount,
		kScene7070UseItem13FrameMap, ARRAYSIZE(kScene7070UseItem13FrameMap),
		kScene7070OverlayFrameMillis, 0x16);
	removeInventoryItem(0x13);
	_soundBank0.playSample(1, 100);
	beginSecondarySpeechLine(0x15, 0);
}

} // End of namespace Hollywood
