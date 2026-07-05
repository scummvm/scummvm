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

const char *const kScene5050ArchiveName = "RESOURCE.E05";
const char *const kScene5050MusicArchiveName = "RESOURCE.M05";
const char *const kScene5050SoundArchiveName = "RESOURCE.S05";
const uint kScene5050InitialRequiredChunkCount = 5;
const uint kScene5050ArenaFirstChunk = 5;
const uint kScene5050ArenaLastChunk = 8;
const uint kScene5050StageIndex = 505;
const uint16 kScene5050FirstState = 0x13ba;
const uint16 kScene5050LastState = 0x13c3;
const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5050ViewportXOffset = 0x00c8;
const uint kScene5050ActorBankTableEntry = 0x0000;
const uint kScene5050ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5050SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5050FrameMillis = 75;
const uint32 kScene5050SpecialFrameMillis = 40;
const uint kScene5050SpecialTransitionDescriptorCount = 0x1f;
const uint kScene5050PickupOverlayDescriptorCount = 0x13;

const byte kScene5050ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene5050PickupFrameMap[] = {
	0, 1, 2, 3, 4, 8, 9, 10, 9, 8, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 4, 8, 9, 10, 9, 8, 4, 3, 2, 1, 0
};

const byte kScene5050PickupItems[] = {
	0x30, 0x42, 0x4c
};

PlayableSceneConfig scene5050Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene5050ArchiveName;
	config.initialRequiredChunkCount = kScene5050InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene5050ArenaFirstChunk;
	config.arenaLastChunk = kScene5050ArenaLastChunk;
	config.stageIndex = kScene5050StageIndex;
	config.debugName = "Scene 5050";
	config.viewportXOffset = kScene5050ViewportXOffset;
	config.viewportMinXOffset = kScene5050ViewportXOffset;
	config.viewportMaxXOffset = kScene5050ViewportXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 5;
	config.actorBankTableEntry = kScene5050ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene5050ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = 0;
	config.speechCueDescriptorTableOffset = kScene5050SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene5050ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene5050ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene5050MusicArchiveName;
	config.soundBank0ArchiveName = kScene5050SoundArchiveName;
	config.mainFlowFirstState = kScene5050FirstState;
	config.mainFlowLastState = kScene5050LastState;
	return config;
}

Scene5050::Scene5050(HollywoodEngine *vm) :
		PlayableScene(vm, scene5050Config(), "scene5050", 0x134, 0x192, 2, 0xfd, 0xfb) {
}

bool Scene5050::hasCustomPreviewState() const {
	return true;
}

void Scene5050::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	_activeActorWorldX = 0x134;
	_activeActorWorldY = 0x192;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene5050::hasCustomComposite() const {
	return true;
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

bool Scene5050::hasCustomEntrySequence() const {
	return true;
}

void Scene5050::runCustomEntrySequence() {
	if (_vm->gameState().mainFlowStateId == kScene5050FirstState) {
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
	_activeActorWorldX = 0x069;
	_activeActorWorldY = 0x157;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();

	Common::Array<byte> frameMap;
	frameMap.resize(kScene5050SpecialTransitionDescriptorCount);
	for (uint i = 0; i < frameMap.size(); ++i)
		frameMap[i] = (byte)i;

	runConfiguredActionOverlay(7, kScene5050SpecialTransitionDescriptorCount,
		frameMap.data(), frameMap.size(), kScene5050SpecialFrameMillis,
		kActionOverlayHideActiveActor);

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
	runConfiguredActionOverlay(8, kScene5050PickupOverlayDescriptorCount,
		kScene5050PickupFrameMap, ARRAYSIZE(kScene5050PickupFrameMap),
		kScene5050FrameMillis, kActionOverlayHideActiveActor,
		-1, 0, 0x18, 1);

	if (state.scene5050TrophyBoxTaken) {
		beginSecondarySpeechLine(5, 0);
		return;
	}

	const byte pickupIndex = MIN<byte>(state.scene5050PickupIndex, ARRAYSIZE(kScene5050PickupItems) - 1);
	const byte itemId = kScene5050PickupItems[pickupIndex];
	addInventoryItem(itemId);
	_soundBank0.playSample(1, 100);
	walkActiveActorTo(0x2b3, 0x1ba, 3, 0, false);
	beginSecondarySpeechLine(0x16, (byte)(pickupIndex * 2 + 1));
	state.scene5050PickupIndex = MIN<byte>((byte)(pickupIndex + 1), ARRAYSIZE(kScene5050PickupItems));
	state.scene5050TrophyBoxTaken = true;
}

} // End of namespace Hollywood
