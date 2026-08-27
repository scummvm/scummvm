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
const uint16 kScene5050ViewportXOffset = 0x00c8;
const uint kScene5050ActorBankTableEntry = 0x0000;
const uint kScene5050ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5050SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5050FrameMillis = 75;
const uint32 kScene5050SpecialFrameMillis = 40;
const uint kScene5050SpecialTransitionDescriptorCount = 0x1f;
const uint kScene5050PickupOverlayDescriptorCount = 0x13;

const byte kScene5050PickupFrameMap[] = {
	0, 1, 2, 3, 4, 8, 9, 10, 9, 8, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 4, 8, 9, 10, 9, 8, 4, 3, 2, 1, 0
};

const byte kScene5050PickupItems[] = {
	0x30, 0x42, 0x4c
};

PlayableSceneConfig scene5050Config() {
	PlayableSceneConfig config(5050,
		SceneResourceLayout(5, 5, 8),
		SceneViewport(kScene5050ViewportXOffset, kScene5050ViewportXOffset, kScene5050ViewportXOffset),
		SceneActorPose(0x134, 0x192, 2));
	config.setActorResources(kScene5050ActorBankTableEntry, kScene5050ActorPaletteTableEntry);
	config.setTextResources(0, kScene5050SpeechCueDescriptorTableOffset);
	config.walkablePaletteMaxRegion = 20;
	return config;
}

Scene5050::Scene5050(HollywoodEngine *vm) :
		PlayableScene(vm, scene5050Config()) {
}

void Scene5050::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	if (_sceneChunkTable.isValidChunk(5))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	drawActionOverlayLayer();
}

void Scene5050::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene5050FirstState) {
		// State 0x13ba is the mine-switch bounce-back path. The trophy room is
		// intentionally only playable from state 0x13bb, reached through Karl's hole.
		runSpecialTransitionToMineSwitches();
		return;
	}

	runEntryPath(0x069, 0x157, 2, 0x134, 0x192);
	_activeActorFacing = 2;
	_activeActorCel = 0;

	GameplayState &state = _vm->gameState();
	if (!state.scene5050EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5050EntryLineSeen = true;
	}
}

bool Scene5050::advanceCustomGameplayLoop(uint32 delta) {
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene5050::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a boquete/salida (go to hole/exit): return to mine switches.
		runExitToMineSwitches();
		return true;
	case 302: // Mirar boquete (look at hole): leads to Karl's gallery.
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

AmbientAudioProfile Scene5050::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 75, 25, 0x0b, 5, 100, 50);
}

void Scene5050::runSpecialTransitionToMineSwitches() {
	setActiveActorPose(0x069, 0x157, 2);
	drawPlayableComposite();
	presentFrame();

	Common::Array<byte> frameMap;
	frameMap.resize(kScene5050SpecialTransitionDescriptorCount);
	for (uint i = 0; i < frameMap.size(); ++i)
		frameMap[i] = (byte)i;

	runActorReplacement(ActionOverlaySpec(7, kScene5050SpecialTransitionDescriptorCount,
		frameMap.data(), frameMap.size(), kScene5050SpecialFrameMillis));

	GameplayState &state = _vm->gameState();
	state.scene5010MineTransportState = 3;
	state.mainFlowStateId = kScene5010ReturnState;
}

void Scene5050::runExitToMineSwitches() {
	walkActiveActorTo(0x069, 0x157, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5050::runTrophyBoxPickup() {
	GameplayState &state = _vm->gameState();
	runActorReplacement(ActionOverlaySpec(8, kScene5050PickupOverlayDescriptorCount,
		kScene5050PickupFrameMap, ARRAYSIZE(kScene5050PickupFrameMap), kScene5050FrameMillis)
		.soundAt(0x18, 1));

	if (state.scene5050TrophyBoxTaken) {
		beginSecondarySpeechLine(5, 0);
		return;
	}

	const byte pickupIndex = state.frankensteinPartRewardIndex();
	if (pickupIndex >= ARRAYSIZE(kScene5050PickupItems)) {
		beginSecondarySpeechLine(5, 0);
		return;
	}
	const byte itemId = kScene5050PickupItems[pickupIndex];
	addInventoryItem(itemId);
	_soundBank0.playSample(1, 100);
	walkActiveActorTo(0x2b3, 0x1ba, 3, 0, false);
	beginSecondarySpeechLine(0x16, (byte)(pickupIndex * 2 + 1));
	state.setFrankensteinPartRewardIndex(pickupIndex + 1);
	state.scene5050TrophyBoxTaken = true;
}

} // End of namespace Hollywood
