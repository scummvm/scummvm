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

#include "common/system.h"

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/scenes/playable/scene5090.h"

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
const uint kScene5090DimmedPaletteColorCount = 0xb0;
const byte kScene5090PaletteDimming = 8;
const byte kScene5090WaterfallPaletteFirstColor = 0x70;
const byte kScene5090WaterfallPaletteLastColor = 0x7f;
const uint32 kScene5090WaterfallPaletteMillis = 300;
const byte kScene5090LagoonPaletteFirstColor = 0x80;
const byte kScene5090LagoonPaletteLastColor = 0x8f;
const uint32 kScene5090LagoonPaletteMillis = 500;
const uint kScene5090LagoonWrapDestinationBase = 0x32d;

const byte kScene5090WaterFillFrameMap[] = {
	8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8
};

PlayableSceneConfig scene5090Config() {
	PlayableSceneConfig config(5090,
		SceneResourceLayout(5, 5, 7),
		SceneViewport(kScene5090ViewportXOffset, kScene5090ViewportXOffset, kScene5090ViewportXOffset),
		SceneActorPose(0x0b5, 0x076, 2));
	config.setActorResources(kScene5090ActorBankTableEntrySet5A, kScene5090ActorPaletteTableEntry);
	config.setTextResources(0, kScene5090SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet5A);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene5090::Scene5090(HollywoodEngine *vm) :
		PlayableScene(vm, scene5090Config()),
		_waterfallPaletteChannel(),
		_lagoonPaletteChannel(),
		_lagoonPalettePhase(0),
		_lagoonPaletteReverse(false),
		_mineCartRumbleActive(false),
		_routeStartRegion(0) {
}

void Scene5090::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applyScenePaletteDimming();
	resetPaletteAnimations();
	_mineCartRumbleActive = false;
	_routeStartRegion = 0;
	setActiveActorPose(0x0b5, 0x076, 2);
}

void Scene5090::runCustomEntrySequence() {
	setActiveActorPose(0x054, 0x068, 2);
	runEntryClip();
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	walkActiveActorTo(0x0b5, 0x076, 2, 0, false);

	GameplayState &state = _vm->gameState();
	if (!state.scene5090EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5090EntryLineSeen = true;
	}
}

void Scene5090::advanceCustomGameplayLoop(uint32 delta) {
	ensureAmbientSoundCuePlaying(0, 0x1c, 10);
	if (_mineCartRumbleActive && !_soundBank0.isPlaying())
		_soundBank0.playSample(0x18, 100);
	advancePaletteAnimations(delta);
}

bool Scene5090::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Usar vagoneta (use mine cart): return to the switch room.
		runReturnToMineSwitches();
		return true;
	case 302: // Mirar laguna subterranea (look at underground lagoon).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar cascada (look at waterfall).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Usar recipiente vacio con cascada: fills item 0x1a and grants item 0x52.
		runFillWaterContainer();
		return true;
	case 305: // Usar recipiente lleno con cascada: Ron already has enough water.
		beginSecondarySpeechLine(4, 0);
		return true;
	default:
		return false;
	}
}

bool Scene5090::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0x0b5, 0x1b7);

	if (targetY < 0x1df)
		++targetY;
	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

void Scene5090::prepareCustomActorPathRoute(int startX, int startY) {
	_routeStartRegion = paletteRegionAt(startX, startY);
	if (_routeStartRegion == 0)
		_routeStartRegion = _activeActorDrawOrderMode;
}

bool Scene5090::customizeRouteFinal(byte currentRegion, byte targetRegion,
		const ActorPathBuildState &state, int targetX, int targetY,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)targetRegion;
	(void)state;
	(void)targetX;
	(void)targetY;
	(void)restoredStepDeltas;

	if (currentRegion == 1 && _routeStartRegion == 2) {
		requestedFacing = 4;
		return true;
	}

	return false;
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

void Scene5090::runExitSideEffectsAfterLoop() {
	fadePaletteToBlack();
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

void Scene5090::applyScenePaletteDimming() {
	const uint byteCount = MIN<uint>(kScene5090DimmedPaletteColorCount * 3,
		MIN<uint>(_paletteResource.size(), _paletteCurrent.size()));
	for (uint offset = 0; offset < byteCount; ++offset) {
		const byte component = _paletteResource[offset];
		const byte dimmed = component > kScene5090PaletteDimming ?
			component - kScene5090PaletteDimming : 0;
		_paletteResource[offset] = dimmed;
		_paletteCurrent[offset] = dimmed;
	}
	invalidatePresentationPalette();
}

void Scene5090::resetPaletteAnimations() {
	_waterfallPaletteChannel.reset(0, kScene5090WaterfallPaletteMillis);
	_lagoonPaletteChannel.reset(0, kScene5090LagoonPaletteMillis);
	_lagoonPalettePhase = 0;
	_lagoonPaletteReverse = false;
}

void Scene5090::advancePaletteAnimations(uint32 delta) {
	uint frameCount = _waterfallPaletteChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		rotateWaterfallPalette();

	frameCount = _lagoonPaletteChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		advanceLagoonPalette();
}

void Scene5090::rotateWaterfallPalette() {
	const uint firstOffset = kScene5090WaterfallPaletteFirstColor * 3;
	const uint lastOffset = kScene5090WaterfallPaletteLastColor * 3;
	if (_paletteCurrent.size() < lastOffset + 3)
		return;

	byte savedColor[3];
	memcpy(savedColor, _paletteCurrent.data() + lastOffset, sizeof(savedColor));
	for (uint color = kScene5090WaterfallPaletteLastColor;
			color > kScene5090WaterfallPaletteFirstColor; --color) {
		memcpy(_paletteCurrent.data() + color * 3,
			_paletteCurrent.data() + (color - 1) * 3, sizeof(savedColor));
	}
	memcpy(_paletteCurrent.data() + firstOffset, savedColor, sizeof(savedColor));
	invalidatePresentationPalette();
}

void Scene5090::advanceLagoonPalette() {
	const uint paletteEnd = (kScene5090LagoonPaletteLastColor + 1) * 3;
	if (_paletteResource.size() < paletteEnd || _paletteCurrent.size() < paletteEnd)
		return;

	if (_lagoonPalettePhase == 0)
		_lagoonPalettePhase = 0x81;
	if (_random.getRandomNumber(49) == 0)
		_lagoonPaletteReverse = !_lagoonPaletteReverse;

	if (_lagoonPaletteReverse) {
		--_lagoonPalettePhase;
		if (_lagoonPalettePhase < kScene5090LagoonPaletteFirstColor)
			_lagoonPalettePhase = kScene5090LagoonPaletteLastColor;
	} else {
		++_lagoonPalettePhase;
		if (_lagoonPalettePhase > kScene5090LagoonPaletteLastColor)
			_lagoonPalettePhase = kScene5090LagoonPaletteFirstColor;
	}

	const uint phaseOffset = _lagoonPalettePhase * 3;
	const uint firstByteCount = paletteEnd - phaseOffset;
	memcpy(_paletteCurrent.data() + kScene5090LagoonPaletteFirstColor * 3,
		_paletteResource.data() + phaseOffset, firstByteCount);

	const uint secondByteCount = phaseOffset - kScene5090LagoonPaletteFirstColor * 3;
	// The original wrap copy overlaps the first span's final color.
	const uint secondDestination = kScene5090LagoonWrapDestinationBase - phaseOffset;
	memcpy(_paletteCurrent.data() + secondDestination,
		_paletteResource.data() + kScene5090LagoonPaletteFirstColor * 3,
		secondByteCount);
	invalidatePresentationPalette();
}

void Scene5090::runEntryClip() {
	copyBaseFramebufferToSceneFramebuffer();
	if (fadePaletteFromBlack() || !_sceneChunkTable.isValidChunk(5))
		return;

	_mineCartRumbleActive = true;
	ensureAmbientSoundCuePlaying(0, 0x1c, 10);
	_soundBank0.playSample(0x18, 100);
	// The first clip tick replaces the initial cart rumble with cue 0x16.
	_soundBank0.playSample(0x16, 100);
	_mineCartRumbleActive = false;

	for (uint frame = 0; frame < kScene5090EntryDescriptorCount &&
			!Engine::shouldQuit() && !_vm->isSceneRestartRequested(); ++frame) {
		if (frame != 0 && waitEntryClipFrameMillis(kScene5090FrameMillis))
			break;
		drawClipFrameDelta(5, kScene5090EntryDescriptorCount, frame);
		presentFrame();
	}
}

bool Scene5090::waitEntryClipFrameMillis(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		if (pollEvents(false))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		g_system->delayMillis(slice);
		advanceGameplayLoop(slice);
		presentFrame();
		remaining -= slice;
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

void Scene5090::runReturnToMineSwitches() {
	walkActiveActorTo(0x054, 0x068, 0xff, 0, false);
	_mineCartRumbleActive = true;
	_soundBank0.playSample(0x15, 100);
	runActorReplacement(ActionOverlaySpec(5, kScene5090ReturnDescriptorCount, kScene5090FrameMillis)
		.noFinalFrameDelay());
	_mineCartRumbleActive = false;
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5090::runFillWaterContainer() {
	beginSecondarySpeechLine(3, 0);
	runActorReplacement(ActionOverlaySpec(7, kScene5090WaterFillDescriptorCount,
		kScene5090WaterFillFrameMap, ARRAYSIZE(kScene5090WaterFillFrameMap), kScene5090FrameMillis)
		.startAt(1)
		.noFinalFrameDelay());
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
