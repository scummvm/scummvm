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
 * along with this source distribution.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "hollywood/hollywood.h"

#include "common/debug.h"
#include "common/serializer.h"

namespace Hollywood {

enum {
	kSaveVersion = 1
};

void syncStateBool(Common::Serializer &s, bool &value) {
	byte rawValue = value ? 1 : 0;
	s.syncAsByte(rawValue);
	if (s.isLoading())
		value = rawValue != 0;
}

void syncUint16Table(Common::Serializer &s, uint16 *values, uint count) {
	for (uint i = 0; i < count; ++i)
		s.syncAsUint16LE(values[i]);
}

bool HollywoodEngine::canLoadGameStateCurrently(Common::U32String *) {
	return _gameState.mainFlowStateId != 0;
}

bool HollywoodEngine::canSaveGameStateCurrently(Common::U32String *) {
	return isImplementedGameplayState(_gameState.mainFlowStateId);
}

Common::Error HollywoodEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	if (!s.syncVersion(kSaveVersion) || s.err())
		return Common::Error(Common::kReadingFailed, "Unsupported Hollywood Monsters save version");

	GameplayState previousState = _gameState;
	_gameState.reset();
	Common::Error result = syncGameStream(s);
	if (result.getCode() != Common::kNoError || s.err()) {
		_gameState = previousState;
		return result.getCode() == Common::kNoError ? Common::kReadingFailed : result;
	}

	if (!isImplementedGameplayState(_gameState.mainFlowStateId)) {
		_gameState = previousState;
		return Common::Error(Common::kReadingFailed, "Save points to an unsupported gameplay state");
	}

	normalizeLoadedGameState();
	syncSoundSettingsFromGameState();
	_sceneRestartRequested = true;
	debugC(1, kDebugGeneral, "Loaded gameplay state 0x%04x", _gameState.mainFlowStateId);
	return Common::kNoError;
}

Common::Error HollywoodEngine::saveGameStream(Common::WriteStream *stream, bool) {
	Common::Serializer s(nullptr, stream);
	s.syncVersion(kSaveVersion);
	Common::Error result = syncGameStream(s);
	if (result.getCode() != Common::kNoError || s.err())
		return Common::kWritingFailed;

	return Common::kNoError;
}

Common::Error HollywoodEngine::syncGameStream(Common::Serializer &s) {
	GameplayState &state = _gameState;

	s.syncAsUint16LE(state.mainFlowStateId);
	syncStateBool(s, state.activeActorPoseValid);
	s.syncAsUint16LE(state.activeActorPoseStateId);
	s.syncAsUint16LE(state.activeActorWorldX);
	s.syncAsUint16LE(state.activeActorWorldY);
	s.syncAsByte(state.activeActorFacing);
	s.syncAsByte(state.activeActorCel);
	s.syncAsUint16LE(state.activeViewportXOffset);
	s.syncAsByte(state.activeAudioChapterIndex);
	s.syncAsByte(state.currentInventoryOwnerIndex);
	s.syncAsByte(state.currentAmbientMusicCueId);
	s.syncBytes(state.inventoryItemCountByOwner, sizeof(state.inventoryItemCountByOwner));
	s.syncBytes(state.inventoryFirstVisibleSlotByOwner, sizeof(state.inventoryFirstVisibleSlotByOwner));
	s.syncBytes(&state.inventorySlotItemIdByOwner[0][0], sizeof(state.inventorySlotItemIdByOwner));
	s.syncBytes(&state.inventoryItemSlotByOwnerAndItemId[0][0], sizeof(state.inventoryItemSlotByOwnerAndItemId));
	s.syncBytes(&state.inventoryItemResourcePageByOwnerAndItemId[0][0], sizeof(state.inventoryItemResourcePageByOwnerAndItemId));
	syncUint16Table(s, state.fixedInventoryVerbHandlerIdsByItemAndStrip,
		GameplayState::kFixedInventoryActionTableEntryCount);
	syncUint16Table(s, state.dialogueRelationMode1HandlerIdsByItemPair,
		GameplayState::kInventoryItemRelationTableEntryCount);
	syncUint16Table(s, state.dialogueRelationMode2HandlerIdsByItemPair,
		GameplayState::kInventoryItemRelationTableEntryCount);
	syncStateBool(s, state.sueInventoryInitialized);
	s.syncAsByte(state.multiToolKnifeState);
	s.syncAsByte(state.ronTapeRecorderState);
	syncStateBool(s, state.ronWalletOpened);
	syncStateBool(s, state.reviewedFrankensteinNote);
	s.syncAsByte(state.frankensteinNoteOverlayMode);
	s.syncAsByte(state.hannoverCourtyardDialogueState);
	syncStateBool(s, state.hannoverCourtyardFollowUpSeen);
	syncStateBool(s, state.seenJosephGuestListGreeting);
	s.syncAsByte(state.officeStatueActionProgress);
	s.syncAsByte(state.officeNotePickupState);
	syncStateBool(s, state.openedOfficeClosetDoor);
	syncStateBool(s, state.spokenToCloakroomAttendant);
	s.syncAsByte(state.cloakroomRagVisible);
	syncStateBool(s, state.spokenToBruno);
	s.syncAsByte(state.humeroBarrierState);
	s.syncAsByte(state.humeroBonePickupState);
	s.syncAsByte(state.punchBowlGlassPatchState);
	syncStateBool(s, state.activatedLabExitMachine);
	s.syncAsByte(state.labMachineSpeed);
	syncStateBool(s, state.seenGramophoneRoomIntro);
	s.syncAsByte(state.gramophoneRoomDoorState);
	s.syncAsByte(state.gramophoneCrankState);
	syncStateBool(s, state.seenHannoverOfficeIntro);
	syncStateBool(s, state.crankOnHannoverDesk);
	syncStateBool(s, state.seenHannoverBedroomIntro);
	syncStateBool(s, state.movedBedroomArmor);
	syncStateBool(s, state.cellPipesActive);
	s.syncAsByte(state.cellPlateRatProgress);
	syncStateBool(s, state.posterOnCellWall);
	syncStateBool(s, state.cellPlateRemoved);
	syncStateBool(s, state.musicEnabled);
	syncStateBool(s, state.soundEffectsEnabled);
	syncStateBool(s, state.optionsTestAudioEnabled);
	s.syncAsByte(state.musicVolumeLevel);
	s.syncAsByte(state.soundEffectsVolumeLevel);
	s.syncAsByte(state.voiceVolumeLevel);
	s.syncAsByte(state.speechTextSpeedLevel);
	s.syncAsByte(state.actorSpeechTextMode);

	return s.err() ? Common::kReadingFailed : Common::kNoError;
}

void HollywoodEngine::normalizeLoadedGameState() {
	GameplayState &state = _gameState;

	state.sharedActorSpriteBankLoaded = false;
	state.sueInventoryResourceTablesLoaded = false;
	state.sceneActionCallbacksInstalled = false;
	state.inventoryPanelDirty = true;

	if (state.currentInventoryOwnerIndex >= GameplayState::kInventoryOwnerCount)
		state.currentInventoryOwnerIndex = 1;

	if (state.activeActorPoseValid) {
		if (state.activeActorPoseStateId != state.mainFlowStateId ||
				!isImplementedGameplayState(state.activeActorPoseStateId) ||
				state.activeActorWorldX >= kSceneBufferWidth ||
				state.activeActorWorldY >= kSceneBufferHeight) {
			state.activeActorPoseValid = false;
		} else {
			if (state.activeActorFacing == 0xff || state.activeActorFacing > 4)
				state.activeActorFacing = 1;
			if (state.activeViewportXOffset > kSceneBufferWidth - kScreenWidth)
				state.activeViewportXOffset = 0;
		}
	}

	for (uint owner = 0; owner < GameplayState::kInventoryOwnerCount; ++owner) {
		if (state.inventoryItemCountByOwner[owner] > GameplayState::kInventoryLastSlot)
			state.inventoryItemCountByOwner[owner] = GameplayState::kInventoryLastSlot;
		if (state.inventoryItemCountByOwner[owner] != 0 &&
				state.inventoryFirstVisibleSlotByOwner[owner] == 0)
			state.inventoryFirstVisibleSlotByOwner[owner] = GameplayState::kInventoryFirstSlot;
	}

	if (state.humeroBarrierState == 0)
		state.humeroBarrierState = 1;
	if (state.punchBowlGlassPatchState > 2)
		state.punchBowlGlassPatchState = 1;
}

} // End of namespace Hollywood
