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

#include "hollywood/scenes/playable/scene5090.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene5090ArchiveName = "RESOURCE.E09";
const char *const kScene5090MusicArchiveName = "RESOURCE.M05";
const char *const kScene5090SoundArchiveName = "RESOURCE.S05";
const uint kScene5090InitialRequiredChunkCount = 5;
const uint kScene5090ArenaFirstChunk = 5;
const uint kScene5090ArenaLastChunk = 7;
const uint kScene5090StageIndex = 509;
const uint16 kScene5090FirstState = 0x13e2;
const uint16 kScene5090LastState = 0x13eb;
const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5090ViewportXOffset = 0x0068;
const uint kScene5090ActorBankTableEntrySet5A = 0x0070;
const uint kScene5090ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5090SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5090FrameMillis = 75;
const uint kScene5090EntryDescriptorCount = 0x0c;
const uint kScene5090ReturnDescriptorCount = 0x13;
const uint kScene5090WaterFillDescriptorCount = 9;
const byte kScene5090EmptyWaterContainerItem = 0x1a;
const byte kScene5090FilledWaterContainerItem = 0x52;

const byte kScene5090ActorPathStepDeltaTableSet5A[] = {
	4, 1, 1, 2, 2, 2, 5, 1, 0, 0, 3, 2,
	2, 1, 6, 4, 4, 5, 4, 3, 7, 2, 1, 6,
	6, 6, 5, 4, 7, 4, 7, 7, 3, 4, 3, 7,
	3, 2, 2, 3, 0, 3, 3, 1, 0, 3, 1, 4,
	4, 7, 7, 3, 4, 3, 7, 6, 6, 5, 4, 7,
	4, 3, 7, 2, 1, 6, 2, 1, 6, 4, 4, 5
};

const byte kScene5090EntryFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const byte kScene5090ReturnFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
};

const byte kScene5090WaterFillFrameMap[] = {
	8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8
};

static PlayableSceneConfig scene5090Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene5090ArchiveName;
	config.initialRequiredChunkCount = kScene5090InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene5090ArenaFirstChunk;
	config.arenaLastChunk = kScene5090ArenaLastChunk;
	config.stageIndex = kScene5090StageIndex;
	config.debugName = "Scene 5090";
	config.viewportXOffset = kScene5090ViewportXOffset;
	config.viewportMinXOffset = kScene5090ViewportXOffset;
	config.viewportMaxXOffset = kScene5090ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 5;
	config.actorBankTableEntry = kScene5090ActorBankTableEntrySet5A;
	config.actorPaletteTableEntry = kScene5090ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = 0;
	config.speechCueDescriptorTableOffset = kScene5090SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene5090ActorPathStepDeltaTableSet5A;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene5090ActorPathStepDeltaTableSet5A);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene5090MusicArchiveName;
	config.soundBank0ArchiveName = kScene5090SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene5090FirstState;
	config.mainFlowLastState = kScene5090LastState;
	return config;
}

Scene5090::Scene5090(HollywoodEngine *vm) :
		PlayableScene(vm, scene5090Config(), "scene5090", 0x0b5, 0x076, 2, 0xfd, 0xfb) {
}

bool Scene5090::hasCustomPreviewState() const {
	return true;
}

void Scene5090::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);

	_activeActorWorldX = 0x0b5;
	_activeActorWorldY = 0x076;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene5090::hasCustomComposite() const {
	return true;
}

void Scene5090::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActionOverlayLayer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
}

bool Scene5090::hasCustomEntrySequence() const {
	return true;
}

void Scene5090::runCustomEntrySequence() {
	_activeActorWorldX = 0x054;
	_activeActorWorldY = 0x068;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();

	runEntryClip();
	walkActiveActorTo(0x0b5, 0x076, 2, 0, false);

	GameplayState &state = _vm->gameState();
	if (!state.scene5090EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5090EntryLineSeen = true;
	}
}

bool Scene5090::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene5090::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar laguna subterranea (look at underground lagoon).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 302: // Mirar/coger cascada (look/take waterfall): water source description.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 303: // Ir a vagoneta/tunel (go to cart/tunnel): return to mine switches.
		runReturnToMineSwitches();
		return true;
	case 304: // Usar recipiente vacio con cascada: fills item 0x1a and grants item 0x52.
		runFillWaterContainer();
		return true;
	case 305: // Usar recipiente lleno con cascada: Ron already has enough water.
		beginSecondarySpeechLine(6, 0);
		return true;
	default:
		return false;
	}
}

bool Scene5090::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x0b5, 0x1b7);

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene5090::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	rebuildWalkableMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene5090::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.soundMode = kAmbientSoundLoop;
	profile.soundCueId = 0x1c;
	profile.soundVolumePercent = 10;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

void Scene5090::runEntryClip() {
	runConfiguredActionOverlay(5, kScene5090EntryDescriptorCount,
		kScene5090EntryFrameMap, ARRAYSIZE(kScene5090EntryFrameMap),
		kScene5090FrameMillis, kActionOverlayHideActiveActor, -1, 0, 1, 0x16);
}

void Scene5090::runReturnToMineSwitches() {
	walkActiveActorTo(0x054, 0x068, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	runConfiguredActionOverlay(5, kScene5090ReturnDescriptorCount,
		kScene5090ReturnFrameMap, ARRAYSIZE(kScene5090ReturnFrameMap),
		kScene5090FrameMillis, kActionOverlayHideActiveActor);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5090::runFillWaterContainer() {
	if (hasInventoryItem(kScene5090FilledWaterContainerItem)) {
		beginSecondarySpeechLine(6, 0);
		return;
	}
	if (!hasInventoryItem(kScene5090EmptyWaterContainerItem)) {
		beginSecondarySpeechLine(5, 0);
		return;
	}

	beginSecondarySpeechLine(3, 0);
	runConfiguredActionOverlay(7, kScene5090WaterFillDescriptorCount,
		kScene5090WaterFillFrameMap, ARRAYSIZE(kScene5090WaterFillFrameMap),
		kScene5090FrameMillis, kActionOverlayHideActiveActor);
	removeInventoryItem(kScene5090EmptyWaterContainerItem);
	addInventoryItem(kScene5090FilledWaterContainerItem);
	_soundBank0.playSample(1, 100);
}

void Scene5090::rebuildWalkableMask() {
	memcpy(_walkablePaletteMask.data(), _fullPaletteRegionMask.data(), _walkablePaletteMask.size());
	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if (_walkablePaletteMask[i] == 1)
			_walkablePaletteMask[i] = 0;
	}
}

} // End of namespace Hollywood
