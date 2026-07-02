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
	return isImplementedGameplayState(_gameState.mainFlowStateId) && canSave();
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
	if (!canSaveGameStateCurrently())
		return Common::Error(Common::kWritingFailed, "Cannot save while a Hollywood Monsters scene action is running");

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
	syncStateBool(s, state.ronPendingMabusePillsInMagnetPillbox);
	syncStateBool(s, state.ronLampFueled);
	syncStateBool(s, state.seenScene3010EntryLine);
	syncStateBool(s, state.windmillBladesMoving);
	syncStateBool(s, state.scene3020Item31Taken);
	syncStateBool(s, state.scene3030MachineActivated);
	syncStateBool(s, state.seenScene3040EntryLine);
	syncStateBool(s, state.scene3040ConditionalObjectVisible);
	syncStateBool(s, state.seenScene3050EntryLine);
	syncStateBool(s, state.scene3050Row3CaptionUpdated);
	s.syncAsByte(state.scene3060SecretDoorState);
	s.syncAsByte(state.scene3060GlobeFrame);
	s.syncAsByte(state.scene3060GlobePuzzleSlot);
	s.syncAsByte(state.scene3060LastGlobeButton);
	s.syncBytes(state.scene3060GlobePuzzleHistory, sizeof(state.scene3060GlobePuzzleHistory));
	s.syncAsByte(state.scene3060TitleFlags);
	syncStateBool(s, state.scene3060GlobeButtonsKnown);
	syncStateBool(s, state.seenScene3070EntryLine);
	syncStateBool(s, state.scene3070PatchDoorOpen);
	s.syncAsByte(state.scene3070Item9PatchState);
	syncStateBool(s, state.scene3070Item10Visible);
	syncStateBool(s, state.scene3070Item32Taken);
	syncStateBool(s, state.scene3070Row3Alternate);
	syncStateBool(s, state.scene3070Row12Alternate);
	syncStateBool(s, state.scene3070BackLayerAlternateFrames);
	syncStateBool(s, state.scene3070ForegroundAlternate);
	syncStateBool(s, state.scene3070WindowPatchActive);
	syncStateBool(s, state.seenScene3070InterludeCutscene);
	syncStateBool(s, state.scene3070InterludeUnlocked);
	syncStateBool(s, state.scene3070LateCutscenePlayed);
	syncStateBool(s, state.seenScene3080EntryLine);
	syncStateBool(s, state.scene3080DoorSeen);
	syncStateBool(s, state.scene3080FrankensteinDiaryRevealed);
	syncStateBool(s, state.scene3080DiaryTaken);
	syncStateBool(s, state.scene3080WindowPatchActive);
	syncStateBool(s, state.scene3080StickTaken);
	syncStateBool(s, state.scene3080SmokeAlternateFrames);
	syncStateBool(s, state.seenScene3090EntryLine);
	s.syncAsByte(state.scene3090BlindManPuzzleStage);
	s.syncAsByte(state.scene3090WindowSequenceState);
	syncStateBool(s, state.scene3090BlindManPlayingSaxophone);
	syncStateBool(s, state.scene3090SaltShakerTaken);
	syncStateBool(s, state.scene3090DowsingRodTaken);
	syncStateBool(s, state.scene3090TalkedToBlindMan);
	s.syncAsByte(state.scene3090PuzzleProgress);
	syncStateBool(s, state.scene3090DialogueMentionedBlindManLaxative);
	syncStateBool(s, state.seenScene3100EntrySequence);
	s.syncAsByte(state.scene3100CabinState);
	syncStateBool(s, state.scene3100ObjectVisible);
	s.syncAsByte(state.scene3100DialogueCounter);
	syncStateBool(s, state.scene3100Item38Taken);
	syncStateBool(s, state.scene3100Item39Taken);
	syncStateBool(s, state.seenScene1010EntryLine);
	syncStateBool(s, state.seenScene1020EntryLine);
	s.syncAsByte(state.scene1020ResourceBlockChoiceState);
	s.syncAsByte(state.scene1020ResourceBlockVariantState);
	syncStateBool(s, state.scene1020AlternateResourceBlockActive);
	syncStateBool(s, state.scene1020EventFlag0);
	syncStateBool(s, state.scene1020EventFlag1);
	syncStateBool(s, state.scene1020EventFlag2);
	syncStateBool(s, state.scene1020EventFlag3);
	syncStateBool(s, state.seenScene1030EntryConversation);
	s.syncAsByte(state.scene1030PatchState);
	syncStateBool(s, state.scene1030EventFlag0);
	syncStateBool(s, state.scene1030ShrinkingManNamed);
	syncStateBool(s, state.seenScene1040EntryLine);
	syncStateBool(s, state.scene1040DoorOpened);
	s.syncAsByte(state.scene1040CordState);
	syncStateBool(s, state.scene1040BalloonTaken);
	syncStateBool(s, state.scene1050SuitcaseTaken);
	syncStateBool(s, state.scene1050TalkedToCloakroomAttendant);
	syncStateBool(s, state.scene1050JackLookedAt);
	s.syncBytes(state.travelScreenSlotIds, sizeof(state.travelScreenSlotIds));
	syncStateBool(s, state.ronTravelScreenUnlocked);
	s.syncAsByte(state.travelScreenCurrentChapterId);
	syncStateBool(s, state.scene1050CloakroomSecretMentioned);
	syncStateBool(s, state.seenScene1060EntryLine);
	s.syncAsByte(state.scene1060FlyDoctorState);
	syncStateBool(s, state.seenScene1060DoctorConversation);
	syncStateBool(s, state.scene1060PocketPaperTaken);
	syncStateBool(s, state.seenScene1060InvisibleManConversation);
	s.syncAsByte(state.scene1060PartyRemainsState);
	syncStateBool(s, state.scene1060FlySlimeHotspotActive);
	syncStateBool(s, state.scene1070DoorOpened);
	syncStateBool(s, state.scene1070ChainRemoved);
	syncStateBool(s, state.scene1070SpiritBlockingHotspot);
	syncStateBool(s, state.seenScene1070QuasimodoConversation);
	syncStateBool(s, state.seenScene1070SpencerConversation);
	s.syncAsByte(state.scene1070SpencerDialogueState);
	syncStateBool(s, state.scene1070MicrophoneStandTaken);
	syncStateBool(s, state.scene1070MicrophoneTaken);
	syncStateBool(s, state.scene1070SpencerExtraFlag);
	syncStateBool(s, state.seenScene1080EntryLine);
	s.syncAsByte(state.scene1080FrancoisState);
	syncStateBool(s, state.seenScene1090EntryLine);
	syncStateBool(s, state.scene1090LightsOn);
	s.syncAsByte(state.scene1090WrappedBrainState);
	syncStateBool(s, state.scene6010StudioEntryUnlocked);
	syncStateBool(s, state.scene6010Item59Visible);
	s.syncAsByte(state.scene6010DoorActionState);
	syncStateBool(s, state.scene6010ExitOverlayPlayed);
	syncStateBool(s, state.scene6010EndgameTravelExitBlocked);
	syncStateBool(s, state.scene6011PendingItem69Visible);
	syncStateBool(s, state.scene6010Item58Taken);
	s.syncAsByte(state.scene2040SphinxBasePatchState);
	s.syncAsByte(state.scene2040SphinxFaceState);
	s.syncAsByte(state.scene2040SphinxItemRevealed);
	s.syncAsByte(state.scene2040SphinxExitInterviewState);
	syncStateBool(s, state.scene6020TaffyKnown);
	syncStateBool(s, state.scene6020TaffyLeft);
	syncStateBool(s, state.scene6030HannoverInterviewCompleted);
	s.syncAsByte(state.scene6030CoffeeState);
	syncStateBool(s, state.seenScene8010EntryLine);
	s.syncAsByte(state.scene8010FishermanConversationState);
	syncStateBool(s, state.seenScene8020EntryLine);
	s.syncAsByte(state.scene8020ForegroundObjectState);
	syncStateBool(s, state.scene8020SecondaryObjectVisible);
	s.syncAsByte(state.scene4010AlternateBackgroundState);
	syncStateBool(s, state.scene4010FirstEntryConversationSeen);
	s.syncAsByte(state.scene4010EntryPathSpeechState);
	s.syncAsByte(state.scene4010ProgressiveExitSpeechState);
	s.syncAsByte(state.scene4010Item3APickupState);
	s.syncAsByte(state.scene4010Item3BPickupState);
	syncStateBool(s, state.scene4010DestinationUnlocked);
	syncStateBool(s, state.seenScene4020FallReactionLine);
	syncStateBool(s, state.scene4020GateUnlocked);
	syncStateBool(s, state.seenScene4030EntryLine);
	syncStateBool(s, state.scene4030RopeTaken);
	s.syncAsByte(state.scene4030BoneState);
	syncStateBool(s, state.scene4030LeverInstalled);
	syncStateBool(s, state.seenScene4040EntryLine);
	syncStateBool(s, state.scene4040CandilTaken);
	syncStateBool(s, state.seenScene4050EntryLine);
	s.syncAsByte(state.scene4050PatchState);
	s.syncAsByte(state.scene4060CardStage);
	s.syncAsByte(state.scene4060SecondCardStage);
	s.syncAsByte(state.scene4060ForegroundState);
	syncStateBool(s, state.seenScene4060EntryLine);
	syncStateBool(s, state.scene4060SherilynDialogueIntroSeen);
	s.syncAsByte(state.scene4060DialogueProgressCounter);
	s.syncAsByte(state.scene4070DraculaStage);
	syncStateBool(s, state.seenScene4070EntryLine);
	syncStateBool(s, state.scene4070TrophyBaseOpened);
	s.syncAsByte(state.scene4070FrankiePartIndex);
	syncStateBool(s, state.scene4070SlimmingTreatmentApplied);
	syncStateBool(s, state.seenScene5010EntryLine);
	s.syncAsByte(state.scene5010MineTransportState);
	syncStateBool(s, state.scene5010MineTransportReady);
	syncStateBool(s, state.scene5010SwitchPanelSeen);
	s.syncAsByte(state.scene5010SwitchRow);
	s.syncAsByte(state.scene5010SwitchColumn);
	syncStateBool(s, state.scene5010MineCartDeparted);
	syncStateBool(s, state.scene5010DestinationTableInitialized);
	syncUint16Table(s, state.scene5010DestinationStateBySwitchSlot,
		ARRAYSIZE(state.scene5010DestinationStateBySwitchSlot));
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
	if (state.scene1020ResourceBlockChoiceState > 2)
		state.scene1020ResourceBlockChoiceState = 0;
	if (state.scene1020ResourceBlockVariantState > 1)
		state.scene1020ResourceBlockVariantState = 0;
	if (state.scene1030PatchState > 3)
		state.scene1030PatchState = 0;
	if (state.scene1040CordState > 2)
		state.scene1040CordState = 0;
	if (state.scene3060SecretDoorState > 1)
		state.scene3060SecretDoorState = 0;
	if (state.scene3060GlobeFrame >= 0x1e)
		state.scene3060GlobeFrame = 0;
	if (state.scene3060GlobePuzzleSlot > 3)
		state.scene3060GlobePuzzleSlot = 3;
	if (state.scene3060LastGlobeButton > 2)
		state.scene3060LastGlobeButton = 0;
	if (state.scene3070Item9PatchState > 2)
		state.scene3070Item9PatchState = 0;
	if (state.scene3090BlindManPuzzleStage > 2)
		state.scene3090BlindManPuzzleStage = 0;
	if (state.scene3090WindowSequenceState > 2)
		state.scene3090WindowSequenceState = 0;
	if (state.scene3090PuzzleProgress > 9)
		state.scene3090PuzzleProgress = 9;
	if (state.scene3100CabinState > 2)
		state.scene3100CabinState = 0;
	if (state.scene3100DialogueCounter > 9)
		state.scene3100DialogueCounter = 9;
	if (state.travelScreenSlotIds[0] > 6)
		state.travelScreenSlotIds[0] = 0;
	if (state.travelScreenSlotIds[1] > 6)
		state.travelScreenSlotIds[1] = 5;
	for (uint slot = 2; slot < GameplayState::kTravelScreenSlotCount; ++slot) {
		if (state.travelScreenSlotIds[slot] == GameplayState::kTravelScreenDisabledSlot)
			continue;
		if (state.travelScreenSlotIds[slot] > 6) {
			state.travelScreenSlotIds[slot] = GameplayState::kTravelScreenDisabledSlot;
			continue;
		}
		for (uint previous = 0; previous < slot; ++previous) {
			if (state.travelScreenSlotIds[previous] == state.travelScreenSlotIds[slot]) {
				state.travelScreenSlotIds[slot] = GameplayState::kTravelScreenDisabledSlot;
				break;
			}
		}
	}
	if (state.travelScreenCurrentChapterId > 9)
		state.travelScreenCurrentChapterId = 0;
	if (state.scene1060FlyDoctorState > 2)
		state.scene1060FlyDoctorState = 0;
	if (state.scene1060PartyRemainsState > 1)
		state.scene1060PartyRemainsState = 0;
	if (state.scene1070SpencerDialogueState > 3)
		state.scene1070SpencerDialogueState = 0;
	if (state.scene1080FrancoisState > 2)
		state.scene1080FrancoisState = 0;
	if (state.scene1090WrappedBrainState > 2)
		state.scene1090WrappedBrainState = 0;
	if (state.scene2040SphinxBasePatchState > 1)
		state.scene2040SphinxBasePatchState = 0;
	if (state.scene2040SphinxFaceState > 3)
		state.scene2040SphinxFaceState = 0;
	if (state.scene2040SphinxItemRevealed > 1)
		state.scene2040SphinxItemRevealed = 0;
	if (state.scene2040SphinxExitInterviewState > 3)
		state.scene2040SphinxExitInterviewState = 0;
	if (state.scene4010AlternateBackgroundState > 1)
		state.scene4010AlternateBackgroundState = 0;
	if (state.scene4010EntryPathSpeechState > 2)
		state.scene4010EntryPathSpeechState = 0;
	if (state.scene4010ProgressiveExitSpeechState > 3)
		state.scene4010ProgressiveExitSpeechState = 0;
	if (state.scene4010Item3APickupState > 3)
		state.scene4010Item3APickupState = 0;
	if (state.scene4010Item3BPickupState > 2)
		state.scene4010Item3BPickupState = 0;
	if (state.scene4050PatchState > 2)
		state.scene4050PatchState = 0;
	if (state.scene5010MineTransportState > 4)
		state.scene5010MineTransportState = 0;
	if (state.scene5010SwitchRow > 2)
		state.scene5010SwitchRow = 0;
	if (state.scene5010SwitchColumn > 2)
		state.scene5010SwitchColumn = 0;
	for (uint slot = 0; slot < ARRAYSIZE(state.scene5010DestinationStateBySwitchSlot); ++slot) {
		const uint16 destination = state.scene5010DestinationStateBySwitchSlot[slot];
		if (destination == 0)
			continue;
		if (destination < 5020 || destination > 5100 || destination % 10 != 0) {
			state.scene5010DestinationStateBySwitchSlot[slot] = 0;
			state.scene5010DestinationTableInitialized = false;
		}
	}
	if (state.scene6010DoorActionState > 3)
		state.scene6010DoorActionState = 0;
	if (state.scene8010FishermanConversationState > 2)
		state.scene8010FishermanConversationState = 0;
	if (state.scene8020ForegroundObjectState > 2)
		state.scene8020ForegroundObjectState = 0;
	if (state.scene4060CardStage > 2)
		state.scene4060CardStage = 0;
	if (state.scene4060SecondCardStage > 2)
		state.scene4060SecondCardStage = 0;
	if (state.scene4060ForegroundState > 1)
		state.scene4060ForegroundState = 0;
	if (state.scene4070DraculaStage > 4)
		state.scene4070DraculaStage = 0;
	if (state.scene4070FrankiePartIndex > 3)
		state.scene4070FrankiePartIndex = 3;
}

} // End of namespace Hollywood
