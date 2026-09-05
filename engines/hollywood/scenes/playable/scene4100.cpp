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

#include "hollywood/hollywood.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/scenes/playable/scene4100.h"

namespace Hollywood {

const uint16 kScene4100DemoExteriorState = 0x0fab;
const uint16 kScene4060ReturnState = 0x0fdd;
const uint16 kScene4070ReturnState = 0x0fe6;
const uint16 kScene4080ReturnState = 0x0ff0;
const uint16 kScene4090ReturnState = 0x0ffa;
const uint16 kScene4110FirstState = 0x100e;
const uint16 kScene4100ViewportXOffset = 0x0040;
const uint16 kScene4100ViewportMinXOffset = 0x0040;
const uint16 kScene4100ViewportMaxXOffset = 0x0070;
const uint kScene4100ActorBankTableEntry = 0x0070;
const uint kScene4100ActorPaletteTableEntry = 0x00cc;
const uint kScene4100Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene4100SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene4100FrameMillis = 75;
const uint32 kScene4100PaletteCycleMillis = 300;
const byte kScene4100PaletteCycleFirstColor = 0xa8;
const byte kScene4100PaletteCycleLastColor = 0xaf;
const uint32 kScene4100AmbientCheckMillis = 250;
const byte kScene4100SpecialAmbientCue = 0x0e;
const byte kScene4100AmbientFirstCue = 0x0f;
const byte kScene4100AmbientCueCount = 8;
const uint kScene4100FirstDoorOverlayChunk = 5;
const uint kScene4100SecondDoorOverlayChunk = 6;
const uint kScene4100ThirdDoorOverlayChunk = 7;
const uint kScene4100DoorOverlayDescriptorCount = 3;
const int kScene4100MaximumWalkX = 0x0294;
const int kScene4100MaximumWalkY = 0x01bc;
const uint kScene4100SpecialStepDeltaSourceOffset = 0x3c;
const uint kScene4100SpecialStepDeltaCount = 0x0c;

PlayableSceneConfig scene4100Config() {
	PlayableSceneConfig config(4100,
		SceneResourceLayout(5, 5, 7),
		SceneViewport(kScene4100ViewportXOffset, kScene4100ViewportMinXOffset, kScene4100ViewportMaxXOffset),
		SceneActorPose(0x017e, 0x0138, 2));
	config.setActorResources(kScene4100ActorBankTableEntry, kScene4100ActorPaletteTableEntry);
	config.setTextResources(kScene4100Resource003RowsOffsetIndex, kScene4100SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSet5A);
	config.walkablePaletteMaxRegion = 20;
	config.useActorDepthTest = true;
	config.entrySequenceOwnsFirstPresentation = true;
	return config;
}

Scene4100::Scene4100(HollywoodEngine *vm) :
		PlayableScene(vm, scene4100Config()),
		_ambientSoundTimerAccumulator(0),
		_previousAmbientSoundCue(0) {
}

void Scene4100::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	applyD10PaletteDimming();
	resetPaletteCycle();
	resetAmbientSoundScheduler();
	applySceneStateToHotspotsAndPatches(0xff);

	const uint16 stateId = _vm->gameState().mainFlowStateId;
	switch (stateId) {
	case 0x1005:
		_activeActorWorldX = 0x0093;
		_activeActorWorldY = 0x00a4;
		_activeActorFacing = 2;
		break;
	case 0x1006:
		_activeActorWorldX = 0x0166;
		_activeActorWorldY = 0x0075;
		_activeActorFacing = 3;
		break;
	case 0x1007:
		_activeActorWorldX = 0x01f1;
		_activeActorWorldY = 0x0077;
		_activeActorFacing = 3;
		break;
	case 0x1008:
		_activeActorWorldX = 0x0294;
		_activeActorWorldY = 0x008c;
		_activeActorFacing = 3;
		break;
	default:
		_activeActorWorldX = 0x011e;
		_activeActorWorldY = 0x00f5;
		_activeActorFacing = 2;
		break;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene4100::runCustomEntrySequence() {
	applyD10PaletteDimming();
	resetPaletteCycle();
	resetAmbientSoundScheduler();
	applySceneStateToHotspotsAndPatches(0xff);

	GameplayState &state = _vm->gameState();
	switch (state.mainFlowStateId) {
	case 0x1004:
		if (revealEntryPose(0x011e, 0x00f5, 2))
			walkActiveActorTo(0x017e, 0x0138, 0xff, 0, false);
		break;
	case 0x1005:
		_soundBank0.playSample(5, 100);
		if (!revealEntryPose(0x0093, 0x00a4, 2))
			break;
		if (!state.scene4100EntryLineSeen) {
			beginSecondarySpeechLine(0, 0);
			state.scene4100EntryLineSeen = true;
		}
		break;
	case 0x1006:
		_soundBank0.playSample(5, 100);
		revealEntryPose(0x0166, 0x0075, 3);
		break;
	case 0x1007:
		_soundBank0.playSample(5, 100);
		revealEntryPose(0x01f1, 0x0077, 3);
		break;
	case 0x1008:
		if (revealEntryPose(0x0294, 0x008c, 3))
			walkActiveActorTo(0x0238, 0x0074, 0xff, 0, false);
		break;
	default:
		break;
	}
}

void Scene4100::runExitSideEffectsAfterLoop() {
	if (!didLeaveSceneAfterLoop())
		return;

	fadePaletteToBlack();
	stopAmbientSoundCues();
}

void Scene4100::prepareCustomGameplayLoop() {
	applyD10PaletteDimming();
	resetPaletteCycle();
	resetAmbientSoundScheduler();
	applySceneStateToHotspotsAndPatches(0xff);
}

void Scene4100::advanceCustomGameplayLoop(uint32 delta) {
	advanceAmbientSound(delta);
	advancePaletteCycle(delta);
}

bool Scene4100::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Mirar primera puerta (look at first door): choose a shared random description.
	case 303: // Mirar segunda puerta (look at second door): choose a shared random description.
	case 305: // Mirar tercera puerta (look at third door): choose a shared random description.
		beginSecondarySpeechLine(1, _random.getRandomBit());
		return true;
	case 302: // Usar/abrir primera puerta (use/open first door): return to Scene 4060.
		runFirstDoorAction();
		return true;
	case 304: // Usar/abrir segunda puerta (use/open second door): return to Scene 4080.
		runDoorTransition(kScene4100SecondDoorOverlayChunk, kScene4100DoorOverlayDescriptorCount, kScene4080ReturnState);
		return true;
	case 306: // Usar/abrir tercera puerta (use/open third door): return to Scene 4090.
		runDoorTransition(kScene4100ThirdDoorOverlayChunk, kScene4100DoorOverlayDescriptorCount, kScene4090ReturnState);
		return true;
	case 307: // Mirar armadura (look at armor).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 308: // Mirar patio (look at courtyard).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 309: // Ir al exterior (go outside).
		_vm->gameState().mainFlowStateId = _vm->isDemo() ?
			kScene4100DemoExteriorState : kScene4110FirstState;
		return true;
	case 310: // Mirar planta (look at plant).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 311: // Ir a sala (go to hall): return to Scene 4070.
		_vm->gameState().mainFlowStateId = kScene4070ReturnState;
		return true;
	case 312: // Mirar sala (look at hall).
		beginSecondarySpeechLine(5, 0);
		return true;
	case 313: // Usar semillas con planta (use seeds with plant).
		beginSecondarySpeechLine(6, 0);
		return true;
	default:
		return false;
	}
}

bool Scene4100::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = CLIP<int>(targetX, 0, kScene4100MaximumWalkX);
	targetY = CLIP<int>(targetY, 0, kScene4100MaximumWalkY);

	if (targetY < kScene4100MaximumWalkY)
		++targetY;
	while (targetY < kScene4100MaximumWalkY && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

bool Scene4100::customizeRouteSegment(byte currentRegion, byte nextRegion, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if (currentRegion == 6 && nextRegion == 5) {
		copySpecialStepDeltas(0x30);
		requestedFacing = 4;
		restoredStepDeltas = true;
		return true;
	}
	if ((currentRegion == 3 && nextRegion == 8) ||
			(currentRegion == 6 && nextRegion == 17)) {
		copySpecialStepDeltas(0x18);
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene4100::customizeRouteFinal(byte currentRegion, byte targetRegion, const ActorPathBuildState &state,
		int targetX, int targetY, int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)targetX;
	(void)targetY;

	if ((currentRegion == 3 && targetRegion == 8) ||
			(currentRegion == 6 && targetRegion == 17)) {
		copySpecialStepDeltas(0x18);
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene4100::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	(void)selector;
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());
	rebuildWalkablePaletteMask();

	for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
		if ((_walkablePaletteMask[i] != 0 && _walkablePaletteMask[i] < 7) ||
				_walkablePaletteMask[i] == 0x14)
			_walkablePaletteMask[i] = 0;
	}

	// The original demo reserves Dracula's salon for its closing showcase.
	if (_vm->isDemo() && !_vm->restoredContentEnabled() &&
			_paletteMask.size() >= kSceneColorToItemMap + kScenePaletteMapPageSize) {
		for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
			if (_paletteMask[kSceneColorToItemMap + color] == 7)
				_paletteMask[kSceneColorToItemMap + color] = 0;
		}
	}

	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene4100::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = kScene4100AmbientCheckMillis;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0b;
	profile.musicCueCount = 5;
	profile.musicProbabilityModulus = 50;
	profile.musicVolumePercent = 100;
	return profile;
}

bool Scene4100::revealEntryPose(int x, int y, byte facing) {
	setActiveActorPose(x, y, facing);
	drawPlayableComposite();
	return !fadePaletteFromBlack();
}

void Scene4100::applyD10PaletteDimming() {
	if (_paletteResource.size() < 0xa8 * 3)
		return;

	for (byte color = 1; color < 0xa8; ++color) {
		const uint offset = color * 3;
		const byte red = _paletteResource[offset] > 4 ? _paletteResource[offset] - 4 : 0;
		const byte green = _paletteResource[offset + 1] > 4 ? _paletteResource[offset + 1] - 4 : 0;
		const byte blue = _paletteResource[offset + 2] > 4 ? _paletteResource[offset + 2] - 4 : 0;
		setPaletteEntry6Bit(color, red, green, blue);
	}
}

void Scene4100::resetPaletteCycle() {
	_paletteCycleChannel.reset(0, kScene4100PaletteCycleMillis);
}

void Scene4100::advancePaletteCycle(uint32 delta) {
	const uint frameCount = _paletteCycleChannel.consumeFrames(delta);
	for (uint frame = 0; frame < frameCount; ++frame)
		rotatePaletteCycle();
}

void Scene4100::rotatePaletteCycle() {
	const uint lastOffset = kScene4100PaletteCycleLastColor * 3;
	if (_paletteCurrent.size() <= lastOffset + 2)
		return;

	byte saved[3];
	memcpy(saved, &_paletteCurrent[lastOffset], sizeof(saved));
	for (uint color = kScene4100PaletteCycleLastColor; color > kScene4100PaletteCycleFirstColor; --color)
		memcpy(&_paletteCurrent[color * 3], &_paletteCurrent[(color - 1) * 3], sizeof(saved));
	memcpy(&_paletteCurrent[kScene4100PaletteCycleFirstColor * 3], saved, sizeof(saved));
	invalidatePresentationPalette();
}

void Scene4100::resetAmbientSoundScheduler() {
	_ambientSoundTimerAccumulator = 0;
	_previousAmbientSoundCue = 0;
}

void Scene4100::advanceAmbientSound(uint32 delta) {
	_ambientSoundTimerAccumulator += delta;
	while (_ambientSoundTimerAccumulator >= kScene4100AmbientCheckMillis) {
		_ambientSoundTimerAccumulator -= kScene4100AmbientCheckMillis;
		SoundBank0Player &player = _additionalAmbientSoundBank0Slots[1];
		if (player.isPlaying() || _random.getRandomNumber(24) != 0)
			continue;

		if (_random.getRandomNumber(9) == 0) {
			player.playSample(kScene4100SpecialAmbientCue, 50);
			continue;
		}

		byte cue = 0;
		do {
			cue = (byte)(kScene4100AmbientFirstCue +
				_random.getRandomNumber(kScene4100AmbientCueCount - 1));
		} while (cue == _previousAmbientSoundCue);
		_previousAmbientSoundCue = cue;
		player.playSample(cue, 4);
	}
}

void Scene4100::copySpecialStepDeltas(uint destinationOffset) {
	for (uint i = 0; i < kScene4100SpecialStepDeltaCount &&
			destinationOffset + i < _actorPathStepDeltas.size() &&
			kScene4100SpecialStepDeltaSourceOffset + i < ARRAYSIZE(kActorPathStepDeltaTableSet5A); ++i) {
		_actorPathStepDeltas[destinationOffset + i] =
			kActorPathStepDeltaTableSet5A[kScene4100SpecialStepDeltaSourceOffset + i];
	}
}

void Scene4100::runFirstDoorAction() {
	const bool spanishDemo = _vm->isDemo() && _vm->getLanguage() == Common::ES_ESP;
	if (spanishDemo) {
		// In the demo Ron must reach Sherilyn through the tower window.
		beginSecondarySpeechLine(7, 0);
		if (animationPlaybackShouldStop())
			return;
		beginPrimarySpeechLine(7, 1, 0x86, 0x32, 0x3f, 0x32, 0);
		if (!animationPlaybackShouldStop())
			beginSecondarySpeechLine(7, 2);
		return;
	}

	runDoorTransition(kScene4100FirstDoorOverlayChunk,
		kScene4100DoorOverlayDescriptorCount, kScene4060ReturnState);
}

void Scene4100::runDoorTransition(uint chunkIndex, uint descriptorCount, uint16 targetState) {
	runActorReplacement(ActionOverlaySpec(chunkIndex, descriptorCount, kScene4100FrameMillis));
	_soundBank0.playSample(3, 100);
	_vm->gameState().mainFlowStateId = targetState;
}

} // End of namespace Hollywood
