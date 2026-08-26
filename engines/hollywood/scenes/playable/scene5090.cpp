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
	PlayableSceneConfig config(5090,
		SceneResourceLayout(5, 5, 7),
		SceneViewport(kScene5090ViewportXOffset, kScene5090ViewportXOffset, kScene5090ViewportXOffset),
		SceneActorPose(0x0b5, 0x076, 2));
	config.setActorResources(kScene5090ActorBankTableEntrySet5A, kScene5090ActorPaletteTableEntry);
	config.setTextResources(0, kScene5090SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet5A);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	return config;
}

Scene5090::Scene5090(HollywoodEngine *vm) :
		PlayableScene(vm, scene5090Config()) {
}

void Scene5090::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applySceneStateToHotspotsAndPatches(0xff);

	setActiveActorPose(0x0b5, 0x076, 2);
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

void Scene5090::runCustomEntrySequence() {
	setActiveActorPose(0x054, 0x068, 2);
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
	runActorReplacement(ActionOverlaySpec(5, kScene5090EntryDescriptorCount,
		kScene5090EntryFrameMap, ARRAYSIZE(kScene5090EntryFrameMap), kScene5090FrameMillis)
		.soundAt(1, 0x16));
}

void Scene5090::runReturnToMineSwitches() {
	walkActiveActorTo(0x054, 0x068, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	runActorReplacement(ActionOverlaySpec(5, kScene5090ReturnDescriptorCount,
		kScene5090ReturnFrameMap, ARRAYSIZE(kScene5090ReturnFrameMap), kScene5090FrameMillis));
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
	runActorReplacement(ActionOverlaySpec(7, kScene5090WaterFillDescriptorCount,
		kScene5090WaterFillFrameMap, ARRAYSIZE(kScene5090WaterFillFrameMap), kScene5090FrameMillis));
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
