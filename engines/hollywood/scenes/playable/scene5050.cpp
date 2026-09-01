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

#include "hollywood/scenes/playable/scene5050.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene5050FirstState = 0x13ba;
const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5040ReturnState = 0x13b1;
const uint16 kScene5050ViewportXOffset = 0x00c8;
const uint kScene5050ActorBankTableEntry = 0x0000;
const uint kScene5050ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5050SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5050SpecialFrameMillis = 40;
const uint32 kScene5050SpecialHoldMillis = 60 * kScene5050SpecialFrameMillis;
const uint32 kScene5050PickupFrameMillis = 125;
const uint32 kScene5050PickupSpeechFrameMillis = 75;
const uint kScene5050SpecialTransitionDescriptorCount = 0x1f;
const uint kScene5050PickupOverlayDescriptorCount = 0x13;
const byte kScene5050PickupSpeechBaseFrame = 0x0a;
const byte kScene5050PickupSpeechFrameCount = 4;

enum Scene5050AnimationHookId {
	kScene5050SpecialHoldHook = 1
};

const byte kScene5050SpecialTransitionFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
};

const byte kScene5050PickupFrameMap[] = {
	0, 1, 2, 3, 4, 8, 9, 10, 9, 8, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 4, 8, 9, 10, 9, 8, 4, 3, 2, 1, 0
};

const byte kScene5050PickupItems[] = {
	0x30, 0x42, 0x4c
};

const byte kScene5050AmbientSoundVolumes[] = {
	10, 10, 10, 2, 10, 10, 10, 100
};

PlayableSceneConfig scene5050Config() {
	PlayableSceneConfig config(5050,
		SceneResourceLayout(5, 5, 8),
		SceneViewport(kScene5050ViewportXOffset, kScene5050ViewportXOffset, kScene5050ViewportXOffset),
		SceneActorPose(0x134, 0x192, 2));
	config.setActorResources(kScene5050ActorBankTableEntry, kScene5050ActorPaletteTableEntry);
	config.setTextResources(0, kScene5050SpeechCueDescriptorTableOffset);
	config.setActorPathStepDeltas(kActorPathStepDeltaTableSetB4);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene5050::Scene5050(HollywoodEngine *vm) :
		PlayableScene(vm, scene5050Config()),
		_specialTransitionActive(false),
		_specialExitAlreadyFaded(false) {
}

void Scene5050::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	for (uint color = 0; color < _walkablePaletteMask.size(); ++color) {
		if (_fullPaletteRegionMask[color] < 6)
			_walkablePaletteMask[color] = 0;
	}
}

void Scene5050::drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) {
	(void)activeWorldX;
	(void)activeWorldY;
	(void)actorDrawOrderMode;
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
}

bool Scene5050::shouldPresentPreviewBeforeEntrySequence() const {
	return false;
}

void Scene5050::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene5050FirstState) {
		runSpecialTransitionToMineSwitches();
		return;
	}

	setActiveActorPose(0x069, 0x157, 2);
	drawPlayableComposite();
	fadePaletteFromBlack();
	walkActiveActorTo(0x134, 0x192, 2, 0, false);

	GameplayState &state = _vm->gameState();
	if (!state.scene5050EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5050EntryLineSeen = true;
	}
}

void Scene5050::advanceCustomGameplayLoop(uint32 delta) {
	(void)delta;
	ensureAmbientSoundCuePlaying(1, 0x0c, 10);
	if (_specialTransitionActive)
		ensureAmbientSoundCuePlaying(2, 0x18, 100);
}

bool Scene5050::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a boquete/salida (go to hole/exit): return to Karl's gallery.
		runExitToKarlGallery();
		return true;
	case 302: // Mirar boquete (look at hole).
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar vagoneta volcada (look at overturned mine cart).
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Mirar trofeo (look at trophy).
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Coger/abrir base del trofeo/restos de caja: grants the next trophy-box item.
		runTrophyBoxPickup();
		return true;
	case 306: // Mirar base del trofeo (look at trophy base).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 307: // Mirar restos de caja (look at box remains).
		beginSecondarySpeechLine(5, 0);
		return true;
	default:
		return false;
	}
}

bool Scene5050::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = MIN<int>(targetX, 0x30d);
	targetY = CLIP<int>(targetY, 0, 0x1df);
	if (targetY < 0x1df)
		++targetY;

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;
	return true;
}

bool Scene5050::customizeRouteSegment(byte currentRegion, byte nextRegion,
		const ActorPathBuildState &state, const ScenePoint &boundary,
		int &requestedFacing, bool &restoredStepDeltas) {
	(void)state;
	(void)boundary;

	if ((currentRegion == 2 && nextRegion == 3) ||
			(currentRegion == 3 && nextRegion == 4) ||
			(currentRegion == 4 && nextRegion == 5)) {
		for (uint offset = 0x18; offset <= 0x23; ++offset)
			_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
		requestedFacing = 2;
		restoredStepDeltas = true;
		return true;
	}

	if ((currentRegion == 5 && nextRegion == 4) ||
			(currentRegion == 4 && nextRegion == 3) ||
			(currentRegion == 3 && nextRegion == 2) ||
			(currentRegion == 2 && nextRegion == 1)) {
		for (uint offset = 0x3c; offset <= 0x47; ++offset)
			_actorPathStepDeltas[offset] = kActorPathStepDeltaTableSet87[offset];
		requestedFacing = 5;
		restoredStepDeltas = true;
		return true;
	}

	return false;
}

bool Scene5050::shouldRunExitSideEffectsAfterLoop() const {
	return true;
}

void Scene5050::runExitSideEffectsAfterLoop() {
	if (!_specialExitAlreadyFaded)
		fadePaletteToBlack();
}

AmbientAudioProfile Scene5050::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 10, 25, 0x0b, 5, 100, 50);
}

byte Scene5050::ambientSoundCueVolume(byte cueId, byte defaultVolumePercent) const {
	if (cueId >= 0x0d && cueId <= 0x14)
		return kScene5050AmbientSoundVolumes[cueId - 0x0d];
	return defaultVolumePercent;
}

void Scene5050::handleAnimationFrameHook(byte hookId, uint frame) {
	switch (hookId) {
	case kScene5050SpecialHoldHook:
		waitSceneMillis(kScene5050SpecialHoldMillis, false);
		return;
	default:
		PlayableScene::handleAnimationFrameHook(hookId, frame);
		return;
	}
}

byte Scene5050::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	return kScene5050PickupSpeechBaseFrame;
}

byte Scene5050::primarySpeechAnimationFrameCount(byte animationGroup) const {
	(void)animationGroup;
	return kScene5050PickupSpeechFrameCount;
}

uint32 Scene5050::primarySpeechAnimationFrameMillis(byte animationGroup) const {
	(void)animationGroup;
	return kScene5050PickupSpeechFrameMillis;
}

void Scene5050::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	_actionOverlayPlayer.setFrame(frameIndex);
}

void Scene5050::runSpecialTransitionToMineSwitches() {
	setActiveActorPose(0x069, 0x157, 2);
	drawPlayableComposite();
	fadePaletteFromBlack();

	_specialTransitionActive = true;
	runActorReplacement(ActionOverlaySpec(7, kScene5050SpecialTransitionDescriptorCount,
		kScene5050SpecialTransitionFrameMap, ARRAYSIZE(kScene5050SpecialTransitionFrameMap),
		kScene5050SpecialFrameMillis)
		.hookAt(14, kScene5050SpecialHoldHook)
		.noFinalFrameDelay()
		.noRedrawAtEnd());
	_specialTransitionActive = false;
	fadePaletteToBlack();
	_specialExitAlreadyFaded = true;

	GameplayState &state = _vm->gameState();
	state.scene5010MineTransportState = 3;
	state.mainFlowStateId = kScene5010ReturnState;
}

void Scene5050::runExitToKarlGallery() {
	_vm->gameState().mainFlowStateId = kScene5040ReturnState;
}

void Scene5050::runTrophyBoxPickup() {
	GameplayState &state = _vm->gameState();
	const byte pickupIndex = state.frankensteinPartRewardIndex();
	const bool grantItem = !state.scene5050TrophyBoxTaken &&
		pickupIndex < ARRAYSIZE(kScene5050PickupItems);
	runActorReplacement(ActionOverlaySpec(8, kScene5050PickupOverlayDescriptorCount,
		kScene5050PickupFrameMap, ARRAYSIZE(kScene5050PickupFrameMap),
		kScene5050PickupFrameMillis)
		.endAt(11)
		.primarySpeechAt(10, grantItem ? 0x16 : 0x66,
			grantItem ? (byte)(pickupIndex * 2) : 0,
			0x29d, 0x128, 0x3f, 0x3f, 0x3f)
		.noFinalFrameDelay()
		.noRedrawAtEnd());

	ActionOverlaySpec closing(8, kScene5050PickupOverlayDescriptorCount,
		kScene5050PickupFrameMap, ARRAYSIZE(kScene5050PickupFrameMap),
		kScene5050PickupFrameMillis);
	closing.frameRange(grantItem ? 14 : 26, ARRAYSIZE(kScene5050PickupFrameMap))
		.noFinalFrameDelay();
	if (grantItem)
		closing.soundAt(24, 1);
	runActorReplacement(closing);

	if (!grantItem || Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	const byte itemId = kScene5050PickupItems[pickupIndex];
	addInventoryItem(itemId);
	walkActiveActorTo(0x2b3, 0x1ba, 3, 0, false);
	beginSecondarySpeechLine(0x16, (byte)(pickupIndex * 2 + 1));
	state.setFrankensteinPartRewardIndex(pickupIndex + 1);
	state.scene5050TrophyBoxTaken = true;
}

} // End of namespace Hollywood
