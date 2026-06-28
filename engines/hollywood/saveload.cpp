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
	s.syncAsByte(state.activeChapterAudioArchiveIndex);
	s.syncAsByte(state.currentInventoryOwnerIndex);
	s.syncAsByte(state.currentRandomAmbientMusicTrackId);
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
	syncStateBool(s, state.inventoryOwner1ItemsInitialized);
	s.syncAsByte(state.sceneActionStateSelector);
	syncStateBool(s, state.g01Item0BSequenceCompleted);
	s.syncAsByte(state.g01DialogueOverlayMode);
	s.syncAsByte(state.g01DialogueBranchState);
	syncStateBool(s, state.g01DialogueBranchFollowUpSeen);
	syncStateBool(s, state.g04EntryConversationPlayed);
	s.syncAsByte(state.g04MajorActionProgress);
	s.syncAsByte(state.g04PatchState);
	syncStateBool(s, state.g04ExitActionDone);
	syncStateBool(s, state.g05DialogueIntroSeen);
	s.syncAsByte(state.g05PatchState);
	syncStateBool(s, state.g06DialogueIntroSeen);
	syncStateBool(s, state.g06ExitMachineTriggered);
	s.syncAsByte(state.g06MachineSpeed);
	syncStateBool(s, state.g07IntroSeen);
	s.syncAsByte(state.g07ExitDoorState);
	s.syncAsByte(state.g07ObjectPatchState);
	syncStateBool(s, state.g08IntroSeen);
	syncStateBool(s, state.g08Item13OnTable);
	syncStateBool(s, state.g09IntroSeen);
	syncStateBool(s, state.g09PatchState);
	syncStateBool(s, state.g09ActionGate);
	syncStateBool(s, state.g10EnvironmentActive);
	s.syncAsByte(state.g10ObjectPatchState);
	syncStateBool(s, state.g10Item15OnScene);
	syncStateBool(s, state.g10Item14PatchState);
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

	state.actorSpriteBankSet00Loaded = false;
	state.inventoryOwner1ResourceTablesLoaded = false;
	state.sceneActionCallbackTableInstalled = false;
	state.inventoryPanelRedrawn = true;

	if (state.currentInventoryOwnerIndex >= GameplayState::kInventoryOwnerCount)
		state.currentInventoryOwnerIndex = 1;

	for (uint owner = 0; owner < GameplayState::kInventoryOwnerCount; ++owner) {
		if (state.inventoryItemCountByOwner[owner] > GameplayState::kInventoryLastSlot)
			state.inventoryItemCountByOwner[owner] = GameplayState::kInventoryLastSlot;
		if (state.inventoryItemCountByOwner[owner] != 0 &&
				state.inventoryFirstVisibleSlotByOwner[owner] == 0)
			state.inventoryFirstVisibleSlotByOwner[owner] = GameplayState::kInventoryFirstSlot;
	}
}

} // End of namespace Hollywood
