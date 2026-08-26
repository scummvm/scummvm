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

#include "hollywood/scenes/playable/scene2110.h"

#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene2110ScriptedReturnState = 0x083f;
const uint16 kScene2100ReturnFromTreasureState = 0x0835;
const uint16 kScene2100LeftPassageState = 0x0836;
const uint16 kScene2110ViewportXOffset = 0x0000;
const uint16 kScene2110ViewportMaxXOffset = 0x0030;
const uint kScene2110ActorBankTableEntry = 0x0000;
const uint kScene2110ActorPaletteTableEntry = 0x00cc;
const uint kScene2110Resource003RowsOffsetIndex = 0x0000;
const uint32 kScene2110SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene2110FrameMillis = 75;
const byte kScene2110InvalidFacing = 0xff;
const byte kScene2110EntryLayerChunk = 5;
const byte kScene2110EntryLayerDescriptorCount = 0x1a;
const byte kScene2110AmbientChunk = 6;
const byte kScene2110AmbientDescriptorCount = 0x1a;
const byte kScene2110TreasureChunk = 7;
const byte kScene2110TreasureDescriptorCount = 0x13;

const byte kScene2110EntryLayerFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 20, 19, 18,
	13
};

const byte kScene2110AmbientFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25
};

const byte kScene2110TreasureFrameMap[] = {
	0, 1, 2, 3, 4, 8, 9, 10, 9, 8,
	4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 4, 8, 9, 10, 9,
	8, 4, 3, 2, 1, 0
};

const byte kScene2110TreasureGrantItems[] = {
	0x30, 0x42, 0x4c
};
const byte kScene2110TreasureSoundHook = 1;

static_assert(ARRAYSIZE(kScene2110EntryLayerFrameMap) == 0x1f,
	"Scene 2110 entry-layer frame map size changed");
static_assert(ARRAYSIZE(kScene2110AmbientFrameMap) == 0x1a,
	"Scene 2110 ambient frame map size changed");
static_assert(ARRAYSIZE(kScene2110TreasureFrameMap) == 0x24,
	"Scene 2110 treasure frame map size changed");

static PlayableSceneConfig scene2110Config() {
	PlayableSceneConfig config(2110,
		SceneResourceLayout(8, 5, 7),
		SceneViewport(kScene2110ViewportXOffset, kScene2110ViewportXOffset, kScene2110ViewportMaxXOffset),
		SceneActorPose(0x24e, 0x122, 4));
	config.setActorResources(kScene2110ActorBankTableEntry, kScene2110ActorPaletteTableEntry);
	config.setTextResources(kScene2110Resource003RowsOffsetIndex, kScene2110SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 1;
	config.useActorDepthTest = true;
	return config;
}

Scene2110::Scene2110(HollywoodEngine *vm) :
		PlayableScene(vm, scene2110Config()),
		_ambientChannel(),
		_entryLayer(),
		_ambientLayer(),
		_treasureLayer() {
	_entryLayer.configure(kScene2110EntryLayerChunk, kScene2110EntryLayerDescriptorCount,
		kScene2110EntryLayerFrameMap, ARRAYSIZE(kScene2110EntryLayerFrameMap));
	_ambientLayer.configure(kScene2110AmbientChunk, kScene2110AmbientDescriptorCount,
		kScene2110AmbientFrameMap, ARRAYSIZE(kScene2110AmbientFrameMap));
	_treasureLayer.configure(kScene2110TreasureChunk, kScene2110TreasureDescriptorCount,
		kScene2110TreasureFrameMap, ARRAYSIZE(kScene2110TreasureFrameMap));
}

void Scene2110::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();

	if (_vm->gameState().mainFlowStateId == kScene2110ScriptedReturnState) {
		_activeActorWorldX = 0x320;
		_activeActorWorldY = 0x118;
		_activeActorFacing = 4;
	} else {
		_activeActorWorldX = 0x320;
		_activeActorWorldY = 0x104;
		_activeActorFacing = 4;
	}

	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

void Scene2110::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawResourceSpriteLayer(_entryLayer);
	drawResourceSpriteLayer(_ambientLayer);
	drawActionOverlayLayer();

	if (_treasureLayer.visible) {
		drawResourceSpriteLayer(_treasureLayer);
	} else {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}
}

void Scene2110::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene2110ScriptedReturnState)
		runScriptedReturnToScene2100();
	else
		runEntryFromScene2100();
}

bool Scene2110::advanceCustomGameplayLoop(uint32 delta) {
	advanceAmbientLayer(delta);
	return false;
}

bool Scene2110::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a entrada (go to entrance): returns to the mummy office.
		runExitToScene2100();
		return true;
	case 302: // Mirar trofeo (look at trophy): impressive trophy.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 303: // Mirar base del trofeo (look at trophy base): academy award inscription.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 304: // Coger tesoros / abrir base del trofeo (take treasures/open trophy base): reward animation.
		runTreasureGrantAction();
		return true;
	case 305: // Mirar tesoros (look at treasures): Ron wants them but cannot take all.
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Mirar pasta gansa / dinero (look at big money): treasure description.
		beginSecondarySpeechLine(5, 0);
		return true;
	default:
		return false;
	}
}

bool Scene2110::adjustCustomWalkTargetToFloorMask(int &targetX, int &targetY) const {
	targetX = MAX<int>(targetX, 0x102);

	while (targetY < 0x1df && walkableMaskAt(targetX, targetY) == 0)
		++targetY;
	while (targetY > 0 && walkableMaskAt(targetX, targetY) == 0)
		--targetY;

	return walkableMaskAt(targetX, targetY) != 0;
}

AmbientAudioProfile Scene2110::ambientAudioProfile() const {
	AmbientAudioProfile profile;
	profile.checkMillis = 250;
	profile.musicMode = kAmbientMusicRandomRange;
	profile.musicFirstCueId = 0x0e;
	profile.musicCueCount = 3;
	profile.musicVolumePercent = 100;
	profile.musicProbabilityModulus = 50;
	return profile;
}

void Scene2110::resetAnimationLayers() {
	_ambientChannel.reset(0, kScene2110FrameMillis);
	_entryLayer.visible = false;
	_entryLayer.reset(0);
	_ambientLayer.visible = true;
	_ambientLayer.reset(0);
	_treasureLayer.visible = false;
	_treasureLayer.reset(0);
}

void Scene2110::advanceAmbientLayer(uint32 delta) {
	const uint frameCount = _ambientChannel.consumeFrames(delta);
	for (uint i = 0; i < frameCount; ++i) {
		byte nextFrame = (byte)(_ambientLayer.frameIndex + 1);
		if (nextFrame >= ARRAYSIZE(kScene2110AmbientFrameMap))
			nextFrame = 0;
		_ambientLayer.setFrame(nextFrame);
	}
}

void Scene2110::runEntryFromScene2100() {
	runEntryPathWithFinalFacing(0x320, 0x104, 4, 0x24e, 0x122, kScene2110InvalidFacing, 0);

	GameplayState &state = _vm->gameState();
	if (!state.scene2110EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene2110EntryLineSeen = true;
	}
}

void Scene2110::runScriptedReturnToScene2100() {
	setActiveActorPose(0x320, 0x118, 4);
	_entryLayer.visible = true;
	_entryLayer.reset(0);
	drawPlayableComposite();
	presentFrame();

	playAnimationFrames(_entryLayer, AnimationFrameRange(1, 0x0e, kScene2110FrameMillis));
	walkActiveActorTo(0x263, 0x135, kScene2110InvalidFacing, 0, false);
	beginSecondarySpeechLine(6, 0);
	playAnimationFrames(_entryLayer, AnimationFrameRange(0x0f, 0x16, kScene2110FrameMillis));
	runTreasurePrimarySpeechLine(6, 1);
	playAnimationFrames(_entryLayer, AnimationFrameRange(0x17, 0x1e, kScene2110FrameMillis));
	beginSecondarySpeechLine(6, 2);
	runTreasurePrimarySpeechLine(6, 3);
	_entryLayer.visible = false;

	GameplayState &state = _vm->gameState();
	state.mainFlowStateId = kScene2100ReturnFromTreasureState;
}

void Scene2110::runExitToScene2100() {
	GameplayState &state = _vm->gameState();
	state.mainFlowStateId = kScene2100LeftPassageState;
}

void Scene2110::runTreasureGrantAction() {
	GameplayState &state = _vm->gameState();
	_treasureLayer.visible = true;
	_treasureLayer.reset(0);
	playAnimationFrames(_treasureLayer, AnimationFrameRange(1, 10, kScene2110FrameMillis));

	if (!state.scene2110TreasureGranted &&
			state.scene2110TreasureGrantIndex < ARRAYSIZE(kScene2110TreasureGrantItems)) {
		const byte grantIndex = state.scene2110TreasureGrantIndex;
		runTreasurePrimarySpeechLine(0x16, (byte)(grantIndex * 2));
		playAnimationFrames(_treasureLayer,
			AnimationFrameRange(11, 0x23, kScene2110FrameMillis)
				.hookAt(0x18, kScene2110TreasureSoundHook));

		const byte itemId = kScene2110TreasureGrantItems[grantIndex];
		if (!hasInventoryItem(itemId))
			addInventoryItem(itemId);
		walkActiveActorTo(0x194, 0x155, 3, 0, false);
		beginSecondarySpeechLine(0x16, (byte)(grantIndex * 2 + 1));
		++state.scene2110TreasureGrantIndex;
		state.scene2110TreasureGranted = true;
	} else {
		runTreasurePrimarySpeechLine(0x66, 0);
		playAnimationFrames(_treasureLayer, AnimationFrameRange(11, 0x23, kScene2110FrameMillis));
	}

	_treasureLayer.visible = false;
	drawPlayableComposite();
	presentFrame();
}

void Scene2110::runEntryPathWithFinalFacing(int startX, int startY, byte startFacing,
		int targetX, int targetY, byte finalFacing, byte finalCel) {
	setActiveActorPose(startX, startY, startFacing);

	drawPlayableComposite();
	presentFrame();
	walkActiveActorTo(targetX, targetY, finalFacing, finalCel, false);
	if (finalFacing != kScene2110InvalidFacing)
		_activeActorFacing = finalFacing;
	_activeActorCel = finalCel;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();
}

void Scene2110::handleAnimationFrameHook(byte hookId, uint frame) {
	if (hookId == kScene2110TreasureSoundHook && frame == 0x18)
		_soundBank0.playSample(1, 100);
}

void Scene2110::runTreasurePrimarySpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLine(rowIndex, frameIndex, 0x1f8, 0x46, 0x32, 0x32, 0x3f);
}

} // End of namespace Hollywood
