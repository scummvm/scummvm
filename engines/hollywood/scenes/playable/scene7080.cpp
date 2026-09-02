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

#include "hollywood/scenes/playable/scene7080.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const uint16 kScene7080BackToG07State = 0x1ba0;
const uint16 kScene7080ViewportXOffset = 0x68;
const int kScene7080EntryX = 0x1db;
const int kScene7080EntryY = 0x102;
const byte kScene7080EntryFacing = 2;
const uint16 kScene7080Chunk6DescriptorCount = 4;
const uint16 kScene7080Chunk7DescriptorCount = 0x0b;
const uint32 kScene7080FrameMillis = 75;
const byte kScene7080TableItemColorId = 6;
const byte kScene7080PostPickupTableItemId = 4;
const byte kScene7080BackToG07FrameMap[] = {
	0, 1, 2, 3
};
const byte kScene7080PickupItem13FrameMap[] = {
	0, 6, 7, 8, 9, 10, 1, 2, 3, 3, 4, 5, 0
};

const SceneSpeechActionSpec kScene7080SpeechActions[] = {
	{ 302, 1, 0 }, // Mirar puerta (look at door).
	{ 303, 2, 0 }, // Mirar ventana (look at window).
	{ 304, 3, 0 }, // Mirar estantería (look at shelves).
	{ 305, 4, 0 }, // Mirar mesa (look at table).
	{ 306, 5, 0 }, // Mirar archivadores (look at filing cabinets).
	{ 308, 6, 0 }, // Mirar manivela (look at crank).
	{ 309, 7, 0 }, // Mirar papelera (look at wastebasket).
	{ 310, 8, 0 }, // Mirar armadura (look at armor).
	{ 311, 9, 0 }  // Usar armadura (use armor).
};

static PlayableSceneConfig scene7080Config() {
	PlayableSceneConfig config(7080,
		SceneResourceLayout(9, 5, 8),
		SceneViewport(kScene7080ViewportXOffset),
		SceneActorPose(kScene7080EntryX, kScene7080EntryY, kScene7080EntryFacing));
	config.setSecondarySpeechActions(kScene7080SpeechActions);
	return config;
}

Scene7080::Scene7080(HollywoodEngine *vm) :
		PlayableScene(vm, scene7080Config()) {
}

void Scene7080::initializeCustomPreviewState() {
	_actionOverlayPlayer.reset();
	_primaryLeftSpeechActive = false;
	_primaryDialogueSpeechActive = false;
	_primaryDialogueSpeechGroup = 0xff;
	_primaryLeftSpeechTimerAccumulator = 0;
	_primaryDialogueSpeechTimerAccumulator = 0;
	_previousAmbientMusicTrackId = 0;
	setActiveActorPose(kScene7080EntryX, kScene7080EntryY, kScene7080EntryFacing);
	_secondaryActorFrame = 0;
	memset(_sceneStateFlags, 0, sizeof(_sceneStateFlags));
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene7080::drawCustomForegroundComposite(int activeWorldX, int activeWorldY) {
	(void)activeWorldX;
	(void)activeWorldY;
	drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

void Scene7080::runCustomEntrySequence() {
	_soundBank0.playSample(4, 100);
	setActiveActorPose(kScene7080EntryX, kScene7080EntryY, kScene7080EntryFacing);
	drawPlayableComposite();
	presentFrame();

	GameplayState &state = _vm->gameState();
	if (!state.seenHannoverOfficeIntro) {
		beginSecondarySpeechLine(0, 0);
		state.seenHannoverOfficeIntro = true;
	}
}

bool Scene7080::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Usar/Abrir puerta (use/open door)
		handleBackToG07();
		return true;
	case 307: // Coger manivela (take crank)
		handlePickupItem13();
		return true;
	default:
		return false;
	}
}

bool Scene7080::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (selector == 1 || selector == 0xff) {
		memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
		memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
		restoreBaseFramebufferFromOriginal();

		GameplayState &state = _vm->gameState();
		if (!state.crankOnHannoverDesk)
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[8], _baseFramebuffer);

		if (_paletteMaskOriginal.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize &&
				_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
			for (uint i = 0; i < kScenePaletteMapPageSize; ++i) {
				const byte originalItem = _paletteMaskOriginal[kSceneColorToItemMap + i];
				if (!state.crankOnHannoverDesk && originalItem == kScene7080TableItemColorId)
					_paletteMask[kSceneColorToItemMap + i] = kScene7080PostPickupTableItemId;
			}
		}

		rebuildWalkableMask();
		_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	}
	return true;
}

void Scene7080::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] > 1)
			_walkablePaletteMask[i] = 0;
	}
}

AmbientAudioProfile Scene7080::ambientAudioProfile() const {
	return createLoopingAmbientAudioProfile(50);
}

void Scene7080::handleBackToG07() {
	BlockingSequence(*this)
		.actorReplacement(6, kScene7080Chunk6DescriptorCount,
			kScene7080BackToG07FrameMap, ARRAYSIZE(kScene7080BackToG07FrameMap),
			kScene7080FrameMillis)
		.sound(3)
		.commit(_vm->gameState().mainFlowStateId, kScene7080BackToG07State);
}

void Scene7080::handlePickupItem13() {
	dispatchGenericSceneAction(19);
	GameplayState &state = _vm->gameState();
	BlockingSequence sequence(*this);
	sequence.actorReplacement(ActionOverlaySpec(7, kScene7080Chunk7DescriptorCount,
		kScene7080PickupItem13FrameMap, ARRAYSIZE(kScene7080PickupItem13FrameMap),
		kScene7080FrameMillis)
		.commitAt(3, state.crankOnHannoverDesk, false)
		.patchAt(3, 1));
	addInventoryItem(0x13);
	sequence.sound(1);
}

} // End of namespace Hollywood
