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
#include "hollywood/scenes/scene_registry.h"

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

bool isBytePermutationValid(const byte *values, uint count) {
	bool seen[GameplayState::kScene2050MuralTilePermutationSize];
	memset(seen, 0, sizeof(seen));

	for (uint i = 1; i < count; ++i) {
		const byte value = values[i];
		if (value == 0 || value >= count || seen[value])
			return false;
		seen[value] = true;
	}

	return true;
}

void setIdentityBytePermutation(byte *values, uint count) {
	values[0] = 0;
	for (uint i = 1; i < count; ++i)
		values[i] = (byte)i;
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
	s.syncAsUint16LE(state.scene9140ReturnStateId);
	s.syncAsByte(state.scene9140VariantIndex);
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
	syncStateBool(s, state.ronInventoryInitialized);
	syncStateBool(s, state.sueInventoryInitialized);
	s.syncAsByte(state.multiToolKnifeState);
	s.syncAsByte(state.ronTapeRecorderState);
	s.syncAsByte(state.ronTravelQuipIndex);
	syncStateBool(s, state.ronWalletOpened);
	s.syncAsUint16LE(state.ronEgyptianMoneyAmount);
	syncStateBool(s, state.ronPendingMabusePillsInMagnetPillbox);
	syncStateBool(s, state.ronLampFueled);
	syncStateBool(s, state.scene3010EntryLineSeen);
	syncStateBool(s, state.scene3010ForestExitSeen);
	syncStateBool(s, state.windmillBladesMoving);
	syncStateBool(s, state.scene3020MaceTaken);
	syncStateBool(s, state.scene3030MachineActivated);
	syncStateBool(s, state.scene3040EntryLineSeen);
	syncStateBool(s, state.scene3040HiddenObjectVisible);
	syncStateBool(s, state.scene3050EntryLineSeen);
	syncStateBool(s, state.scene3050LibraryCaptionRevealed);
	s.syncAsByte(state.scene3060SecretDoorRevealState);
	s.syncAsByte(state.scene3060GlobeFrame);
	s.syncAsByte(state.scene3060GlobePuzzleRunIndex);
	s.syncAsByte(state.scene3060LastGlobePuzzleButton);
	s.syncBytes(state.scene3060GlobePuzzleRuns, sizeof(state.scene3060GlobePuzzleRuns));
	s.syncAsByte(state.scene3060InspectedTitleFlags);
	syncStateBool(s, state.scene3060GlobeButtonsDiscovered);
	syncStateBool(s, state.scene3070EntryLineSeen);
	syncStateBool(s, state.scene3070DrawerOpen);
	s.syncAsByte(state.scene3070SurgicalNeedleThreadState);
	s.syncAsByte(state.scene3070FrankensteinBodyState);
	syncStateBool(s, state.scene3070SurgicalNeedleThreadTaken);
	syncStateBool(s, state.scene3070OperatingTableAlternateDescription);
	s.syncAsByte(state.scene3070StoryPhase);
	s.syncAsByte(state.scene3070SerumIngredientCount);
	syncStateBool(s, state.scene3070OperatingTableForegroundAlternate);
	syncStateBool(s, state.scene3070WindowForegroundPatchActive);
	syncStateBool(s, state.scene3070InterludeCutsceneSeen);
	syncStateBool(s, state.scene3070InterludeUnlocked);
	syncStateBool(s, state.scene3070LateCutscenePlayed);
	syncStateBool(s, state.scene3080EntryLineSeen);
	syncStateBool(s, state.scene3080CabinDoorVisited);
	syncStateBool(s, state.scene3080FrankensteinDiaryRevealed);
	syncStateBool(s, state.scene3080FrankensteinDiaryTaken);
	syncStateBool(s, state.scene3080WindowOpened);
	syncStateBool(s, state.scene3080BranchTaken);
	syncStateBool(s, state.scene3080ChimneySmokeAnimationChanged);
	syncStateBool(s, state.scene3090EntryLineSeen);
	s.syncAsByte(state.scene3090SecretDiaryPuzzleStage);
	s.syncAsByte(state.scene3090WindowOpenSequenceState);
	syncStateBool(s, state.scene3090BlindManPlayingSaxophone);
	syncStateBool(s, state.scene3090SaltShakerTaken);
	syncStateBool(s, state.scene3090DowsingRodTaken);
	syncStateBool(s, state.scene3090BlindManConversationSeen);
	s.syncAsByte(state.scene3090SecretDiaryPuzzleProgress);
	syncStateBool(s, state.scene3090DialogueMentionedBlindManLaxative);
	syncStateBool(s, state.scene3100CabinVisited);
	s.syncAsByte(state.scene3100GirlConversationState);
	syncStateBool(s, state.scene3100DaisyVisible);
	s.syncAsByte(state.scene3100GirlDialogueRepeatCounter);
	syncStateBool(s, state.scene3100SapSyringeTaken);
	syncStateBool(s, state.scene3100DaisyTaken);
	syncStateBool(s, state.scene1010EntryLineSeen);
	syncStateBool(s, state.scene1020EntryLineSeen);
	s.syncAsByte(state.scene1020HookPositionState);
	s.syncAsByte(state.scene1020ChainAttachedToGrate);
	syncStateBool(s, state.scene1020GrateRaised);
	syncStateBool(s, state.scene1020SueTapeVisible);
	syncStateBool(s, state.scene1020BrokenRecorderIdentified);
	syncStateBool(s, state.scene1020RustyRailGreased);
	syncStateBool(s, state.scene1020SueTapeNoticed);
	syncStateBool(s, state.scene1030EntryConversationSeen);
	s.syncAsByte(state.scene1030TablePickupState);
	syncStateBool(s, state.scene1030SleepingDrunkInspected);
	syncStateBool(s, state.scene1030ShrinkingManNamed);
	syncStateBool(s, state.scene1040EntryLineSeen);
	syncStateBool(s, state.scene1040CloakroomDoorOpened);
	s.syncAsByte(state.scene1040GorillaCordState);
	syncStateBool(s, state.scene1040BalloonTaken);
	syncStateBool(s, state.scene1050SuitcaseTaken);
	syncStateBool(s, state.scene1050CloakroomAttendantConversationSeen);
	syncStateBool(s, state.scene1050JackLookedAt);
	s.syncBytes(state.travelScreenSlotIds, sizeof(state.travelScreenSlotIds));
	syncStateBool(s, state.ronTravelScreenUnlocked);
	s.syncAsByte(state.travelScreenCurrentChapterId);
	syncStateBool(s, state.scene1050CharlieBogWerewolfClueHeard);
	syncStateBool(s, state.scene1060EntryLineSeen);
	s.syncAsByte(state.scene1060DrFlyState);
	syncStateBool(s, state.scene1060DrFlyConversationSeen);
	syncStateBool(s, state.scene1060PocketPaperTaken);
	syncStateBool(s, state.scene1060InvisibleManConversationSeen);
	s.syncAsByte(state.scene1060PartyRemainsState);
	syncStateBool(s, state.scene1060FlySlimeHotspotActive);
	syncStateBool(s, state.scene1070DoorOpened);
	syncStateBool(s, state.scene1070ChainRemoved);
	syncStateBool(s, state.scene1070SpiritBlockingHotspot);
	syncStateBool(s, state.scene1070QuasimodoConversationSeen);
	syncStateBool(s, state.scene1070SpencerConversationSeen);
	s.syncAsByte(state.scene1070SpencerTravelClueProgress);
	syncStateBool(s, state.scene1070SpencerCocktailRecipeLearned);
	syncStateBool(s, state.scene1070MicrophoneStandTaken);
	syncStateBool(s, state.scene1070MicrophoneTaken);
	syncStateBool(s, state.scene1080EntryLineSeen);
	s.syncAsByte(state.scene1080FrancoisProgressState);
	syncStateBool(s, state.scene1090EntryLineSeen);
	syncStateBool(s, state.scene1090LightsOn);
	s.syncAsByte(state.scene1090WrappedBrainState);
	syncStateBool(s, state.scene2010EntryLineSeen);
	syncStateBool(s, state.scene2010LongSequenceFirstSpeechSeen);
	syncStateBool(s, state.scene2010B02EntranceUsed);
	s.syncAsByte(state.scene2010TravelReturnSpeechState);
	syncStateBool(s, state.scene2020EntryLineSeen);
	syncStateBool(s, state.scene2020PrincessGone);
	s.syncAsByte(state.scene2020TigerToothState);
	syncStateBool(s, state.scene2020HatPresent);
	syncStateBool(s, state.scene2020SunglassesPresent);
	syncStateBool(s, state.scene2020PrincessConversationSeen);
	syncStateBool(s, state.scene2030RightMerchantConversationSeen);
	s.syncAsByte(state.scene2030SeedOfferState);
	s.syncAsByte(state.scene2030MerchantItem2AOfferState);
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
	s.syncAsByte(state.scene2050EntrySpeechState);
	s.syncAsByte(state.scene2050MuralPuzzleState);
	s.syncBytes(state.scene2050MuralTilePermutation, sizeof(state.scene2050MuralTilePermutation));
	syncStateBool(s, state.scene2050SealRevealed);
	syncStateBool(s, state.scene2050LabyrinthLampReady);
	s.syncAsByte(state.egyptSealPuzzleProgress);
	s.syncAsByte(state.egyptLabyrinthPositionIndex);
	s.syncAsByte(state.scene2070EntryProgress);
	s.syncAsByte(state.scene2070SealExitPatchState);
	s.syncAsByte(state.scene2070InnerPassagePatchState);
	s.syncAsByte(state.scene2070HiddenItemPatchState);
	syncStateBool(s, state.scene2080EntryLineSeen);
	s.syncAsByte(state.scene2080ForegroundState);
	syncStateBool(s, state.scene2080ExitBackSequenceSeen);
	s.syncAsByte(state.scene2080PrincessHairSearchState);
	syncStateBool(s, state.scene2080FirstRow09LookSeen);
	syncStateBool(s, state.scene2080DialogueBranchBUnlocked);
	s.syncAsByte(state.scene2080DialogueBranchAIndex);
	s.syncAsByte(state.scene2080DialogueBranchBIndex);
	s.syncAsByte(state.scene2080DialogueTerminalIndex);
	syncStateBool(s, state.scene2090EntryLineSeen);
	syncStateBool(s, state.scene2100RaStaffTaken);
	syncStateBool(s, state.scene2100PassageOpen);
	s.syncAsByte(state.scene2100MummyBranchState);
	syncStateBool(s, state.scene2100MummyGreetingSeen);
	syncStateBool(s, state.scene2100AfterlifeBranchUnlocked);
	s.syncAsUint16LE(state.scene2100MummyDialogueUsedChoiceMask);
	syncStateBool(s, state.scene2110EntryLineSeen);
	s.syncAsByte(state.scene2110TreasureGrantIndex);
	syncStateBool(s, state.scene2110TreasureGranted);
	syncStateBool(s, state.scene6020TaffyKnown);
	syncStateBool(s, state.scene6020TaffyLeft);
	syncStateBool(s, state.scene6030HannoverInterviewCompleted);
	syncStateBool(s, state.scene6030SaxophoneTaken);
	syncStateBool(s, state.scene6030HannoverAbsent);
	s.syncAsByte(state.scene6030CoffeeState);
	syncStateBool(s, state.scene6040EntryLineSeen);
	syncStateBool(s, state.scene6040PaintCanTaken);
	s.syncAsByte(state.scene6040WireState);
	syncStateBool(s, state.scene6050MuseumInteriorUnlocked);
	syncStateBool(s, state.scene6050GuardPresent);
	syncStateBool(s, state.scene6050GuardAllowsEntry);
	syncStateBool(s, state.scene6050DisplayCaseOpened);
	syncStateBool(s, state.scene6070SuePresent);
	syncStateBool(s, state.scene6070CellDoorOpen);
	syncStateBool(s, state.scene6080Visited);
	syncStateBool(s, state.scene6100EntryLineSeen);
	s.syncAsByte(state.scene6100CharlieState);
	syncStateBool(s, state.scene6100BriefcasePresent);
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
	s.syncAsByte(state.scene4010PillboxPickupState);
	syncStateBool(s, state.scene4010DestinationUnlocked);
	syncStateBool(s, state.scene4020FallReactionLineSeen);
	syncStateBool(s, state.scene4020GateUnlocked);
	syncStateBool(s, state.scene4030InitialEntryLineSeen);
	syncStateBool(s, state.scene4030RopeTaken);
	s.syncAsByte(state.scene4030LooseBoneState);
	syncStateBool(s, state.scene4030ImprovisedLeverInstalled);
	syncStateBool(s, state.scene4040EntryLineSeen);
	syncStateBool(s, state.scene4040CandilTaken);
	syncStateBool(s, state.scene4050EntryLineSeen);
	s.syncAsByte(state.scene4050RopeSwingState);
	s.syncAsByte(state.scene4060PictureCardStage);
	s.syncAsByte(state.scene4060PerfumeBottleCardStage);
	s.syncAsByte(state.scene4060SherilynSheetWon);
	syncStateBool(s, state.scene4060EntryLineSeen);
	syncStateBool(s, state.scene4060SherilynDialogueIntroSeen);
	s.syncAsByte(state.scene4060SherilynPokerProgressCounter);
	s.syncAsByte(state.scene4070DraculaStage);
	syncStateBool(s, state.scene4070EntryLineSeen);
	syncStateBool(s, state.scene4070TrophyBaseOpened);
	s.syncAsByte(state.scene4070FrankiePartGranted);
	syncStateBool(s, state.scene4070SlimmingTreatmentApplied);
	s.syncAsByte(state.scene4080GwendolynState);
	s.syncAsByte(state.scene4080GwendolynStateTransition);
	s.syncAsByte(state.scene4080CoffinShiftedState);
	s.syncAsByte(state.scene4080OilBottleState);
	s.syncAsByte(state.scene4080GwendolynNameState);
	s.syncAsByte(state.scene4080GominolaVisibleState);
	syncStateBool(s, state.scene4090InitialGreetingSeen);
	s.syncAsByte(state.scene4090WideCoffinVariant);
	syncStateBool(s, state.scene4090OrganRevealDialogueSeen);
	s.syncAsByte(state.scene4090FinalCutsceneCompleted);
	syncStateBool(s, state.scene4090FinalCutsceneDialogueSeen);
	syncStateBool(s, state.scene4100EntryLineSeen);
	syncStateBool(s, state.scene4110LetterTaken);
	syncStateBool(s, state.scene4110BridgeOpened);
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
	syncStateBool(s, state.scene5020ExplosivesCrateIdentified);
	syncStateBool(s, state.scene5020WoodenPlankTaken);
	syncStateBool(s, state.scene5030EntryLineSeen);
	s.syncAsByte(state.scene5030DeckOfCardsState);
	syncStateBool(s, state.scene5030MusiciansNamed);
	syncStateBool(s, state.scene5040EntryLineSeen);
	s.syncAsByte(state.scene5040MineGalleryState);
	syncStateBool(s, state.scene5040LooseObjectTaken);
	syncStateBool(s, state.scene5040OldSockTaken);
	s.syncAsByte(state.scene5040DialState);
	syncStateBool(s, state.scene5040KarlDialogueIntroSeen);
	s.syncAsByte(state.scene5040SpecialTransitionState);
	syncStateBool(s, state.scene5050EntryLineSeen);
	syncStateBool(s, state.scene5050TrophyBoxTaken);
	s.syncAsByte(state.scene5050PickupIndex);
	syncStateBool(s, state.scene5060EntryLineSeen);
	syncStateBool(s, state.scene5060GasSmelled);
	syncStateBool(s, state.scene5060RockTaken);
	syncStateBool(s, state.scene5070ShovelTaken);
	s.syncAsByte(state.scene5070AviatorCapState);
	syncStateBool(s, state.scene5080EntryLineSeen);
	syncStateBool(s, state.scene5080PassageUnlocked);
	syncStateBool(s, state.scene5080AlternatePassageSide);
	syncStateBool(s, state.scene5080BookTaken);
	syncStateBool(s, state.scene5080StairDoorBarrierSeen);
	syncStateBool(s, state.scene5080StairDoorConstructionSeen);
	syncStateBool(s, state.scene5090EntryLineSeen);
	syncStateBool(s, state.scene5100EntryLineSeen);
	syncStateBool(s, state.scene5100ButtonsUnlocked);
	syncStateBool(s, state.scene5110IntroSeen);
	syncStateBool(s, state.scene5110UnderwearTaken);
	s.syncAsByte(state.scene5110BottleState);
	s.syncAsByte(state.scene5110SalonTransformState);
	syncStateBool(s, state.scene5110ElevatorTransitionSeen);
	syncStateBool(s, state.scene5110MirrorTaken);
	s.syncAsByte(state.scene5110JacuzziInspectionState);
	syncStateBool(s, state.scene5110WerewolfDialogueChoiceUnlocked);
	syncStateBool(s, state.scene5120TongsTaken);
	s.syncAsByte(state.scene5120CocktailState);
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

	syncStateBool(s, state.frankensteinDiaryRead);
	syncStateBool(s, state.ronPosterPhotoRemoved);

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
	if (state.scene1020HookPositionState > 2)
		state.scene1020HookPositionState = 0;
	if (state.scene1020ChainAttachedToGrate > 1)
		state.scene1020ChainAttachedToGrate = 0;
	if (state.scene1030TablePickupState > 3)
		state.scene1030TablePickupState = 0;
	if (state.scene1040GorillaCordState > 2)
		state.scene1040GorillaCordState = 0;
	if (state.scene6040WireState > 2)
		state.scene6040WireState = 0;
	if (state.scene6100CharlieState > 2)
		state.scene6100CharlieState = 1;
	if (state.scene3060SecretDoorRevealState > 2)
		state.scene3060SecretDoorRevealState = 0;
	if (state.scene3060GlobeFrame >= 0x1e)
		state.scene3060GlobeFrame = 0;
	if (state.scene3060GlobePuzzleRunIndex > 3)
		state.scene3060GlobePuzzleRunIndex = 3;
	if (state.scene3060LastGlobePuzzleButton > 2)
		state.scene3060LastGlobePuzzleButton = 0;
	if (state.scene3070SurgicalNeedleThreadState > 2)
		state.scene3070SurgicalNeedleThreadState = 0;
	if (state.scene3070FrankensteinBodyState > 2)
		state.scene3070FrankensteinBodyState = 0;
	if (state.scene3070StoryPhase > 2)
		state.scene3070StoryPhase = 0;
	if (state.scene3070SerumIngredientCount > 5)
		state.scene3070SerumIngredientCount = 0;
	if (state.scene3090SecretDiaryPuzzleStage > 2)
		state.scene3090SecretDiaryPuzzleStage = 0;
	if (state.scene3090WindowOpenSequenceState > 2)
		state.scene3090WindowOpenSequenceState = 0;
	if (state.scene3090SecretDiaryPuzzleProgress > 9)
		state.scene3090SecretDiaryPuzzleProgress = 9;
	if (state.scene3100GirlConversationState > 2)
		state.scene3100GirlConversationState = 0;
	if (state.scene3100GirlDialogueRepeatCounter > 9)
		state.scene3100GirlDialogueRepeatCounter = 9;
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
	if (state.scene1060DrFlyState > 2)
		state.scene1060DrFlyState = 0;
	if (state.scene1060PartyRemainsState > 1)
		state.scene1060PartyRemainsState = 0;
	if (state.scene1070SpencerTravelClueProgress > 3)
		state.scene1070SpencerTravelClueProgress = 0;
	if (state.scene1080FrancoisProgressState > 2)
		state.scene1080FrancoisProgressState = 0;
	if (state.scene1090WrappedBrainState > 2)
		state.scene1090WrappedBrainState = 0;
	if (state.scene2010TravelReturnSpeechState > 2)
		state.scene2010TravelReturnSpeechState = 0;
	if (state.scene2020TigerToothState > 2)
		state.scene2020TigerToothState = 0;
	if (state.scene2030SeedOfferState > 2)
		state.scene2030SeedOfferState = 0;
	if (state.scene2030MerchantItem2AOfferState > 2)
		state.scene2030MerchantItem2AOfferState = 0;
	if (state.scene2040SphinxBasePatchState > 1)
		state.scene2040SphinxBasePatchState = 0;
	if (state.scene2040SphinxFaceState > 3)
		state.scene2040SphinxFaceState = 0;
	if (state.scene2040SphinxItemRevealed > 1)
		state.scene2040SphinxItemRevealed = 0;
	if (state.scene2040SphinxExitInterviewState > 3)
		state.scene2040SphinxExitInterviewState = 0;
	if (state.scene2050EntrySpeechState > 2)
		state.scene2050EntrySpeechState = 0;
	if (state.scene2050MuralPuzzleState > 3)
		state.scene2050MuralPuzzleState = 0;
	if (state.egyptSealPuzzleProgress > 2)
		state.egyptSealPuzzleProgress = 0;
	if (!isBytePermutationValid(state.scene2050MuralTilePermutation,
			GameplayState::kScene2050MuralTilePermutationSize)) {
		if (state.scene2050MuralPuzzleState >= 2 || state.egyptSealPuzzleProgress != 0)
			setIdentityBytePermutation(state.scene2050MuralTilePermutation,
				GameplayState::kScene2050MuralTilePermutationSize);
		else
			memset(state.scene2050MuralTilePermutation, 0, sizeof(state.scene2050MuralTilePermutation));
	}
	if (state.egyptLabyrinthPositionIndex >= 0x48 && state.egyptLabyrinthPositionIndex != 0xff)
		state.egyptLabyrinthPositionIndex = 0x2a;
	if (state.scene2070EntryProgress > 2)
		state.scene2070EntryProgress = 0;
	if (state.scene2070SealExitPatchState > 1)
		state.scene2070SealExitPatchState = 0;
	if (state.scene2070InnerPassagePatchState > 1)
		state.scene2070InnerPassagePatchState = 0;
	if (state.scene2070HiddenItemPatchState > 1)
		state.scene2070HiddenItemPatchState = 0;
	if (state.scene2080ForegroundState > 2)
		state.scene2080ForegroundState = 2;
	if (state.scene2080PrincessHairSearchState > 2)
		state.scene2080PrincessHairSearchState = 0;
	if (state.scene2080DialogueBranchAIndex > 2)
		state.scene2080DialogueBranchAIndex = 0;
	if (state.scene2080DialogueBranchBIndex > 2)
		state.scene2080DialogueBranchBIndex = 0;
	if (state.scene2080DialogueTerminalIndex > 2)
		state.scene2080DialogueTerminalIndex = 0;
	if (state.scene2100MummyBranchState > 2)
		state.scene2100MummyBranchState = 0;
	const byte frankensteinPartRewardIndex = MIN<byte>(
		MAX<byte>(state.frankensteinPartRewardIndex(), state.scene4070FrankiePartGranted),
		GameplayState::kFrankensteinPartRewardCount);
	state.scene4070FrankiePartGranted = state.scene4070FrankiePartGranted != 0 ? 1 : 0;
	state.setFrankensteinPartRewardIndex(frankensteinPartRewardIndex);
	if (state.scene4010AlternateBackgroundState > 1)
		state.scene4010AlternateBackgroundState = 0;
	if (state.scene4010EntryPathSpeechState > 2)
		state.scene4010EntryPathSpeechState = 0;
	if (state.scene4010ProgressiveExitSpeechState > 3)
		state.scene4010ProgressiveExitSpeechState = 0;
	if (state.scene4010Item3APickupState > 3)
		state.scene4010Item3APickupState = 0;
	if (state.scene4010PillboxPickupState > 2)
		state.scene4010PillboxPickupState = 0;
	if (state.scene4050RopeSwingState > 2)
		state.scene4050RopeSwingState = 0;
	if (state.scene5010MineTransportState > 4)
		state.scene5010MineTransportState = 0;
	if (state.scene5040MineGalleryState > 2)
		state.scene5040MineGalleryState = 0;
	if (state.scene5040DialState > 4)
		state.scene5040DialState = 0;
	if (state.scene5040SpecialTransitionState > 2)
		state.scene5040SpecialTransitionState = 0;
	if (state.scene5070AviatorCapState > 2)
		state.scene5070AviatorCapState = 1;
	if (state.scene5110BottleState > 2)
		state.scene5110BottleState = 0;
	if (state.scene5110SalonTransformState > 4)
		state.scene5110SalonTransformState = 0;
	if (state.scene5110JacuzziInspectionState > 2)
		state.scene5110JacuzziInspectionState = 0;
	if (state.scene5120CocktailState > 3)
		state.scene5120CocktailState = 0;
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
	if (state.scene5030DeckOfCardsState > 2)
		state.scene5030DeckOfCardsState = 0;
	if (state.scene6010DoorActionState > 3)
		state.scene6010DoorActionState = 0;
	if (state.scene8010FishermanConversationState > 2)
		state.scene8010FishermanConversationState = 0;
	if (state.scene8020ForegroundObjectState > 2)
		state.scene8020ForegroundObjectState = 0;
	if (state.scene4060PictureCardStage > 2)
		state.scene4060PictureCardStage = 0;
	if (state.scene4060PerfumeBottleCardStage > 2)
		state.scene4060PerfumeBottleCardStage = 0;
	if (state.scene4060SherilynSheetWon > 1)
		state.scene4060SherilynSheetWon = 0;
	if (state.scene4070DraculaStage > 4)
		state.scene4070DraculaStage = 0;
	if (state.scene4080GwendolynState > 2)
		state.scene4080GwendolynState = 1;
	if (state.scene4080GwendolynStateTransition > 2)
		state.scene4080GwendolynStateTransition = 0;
	if (state.scene4080CoffinShiftedState > 1)
		state.scene4080CoffinShiftedState = 0;
	if (state.scene4080OilBottleState > 2)
		state.scene4080OilBottleState = 1;
	if (state.scene4080GwendolynNameState > 2)
		state.scene4080GwendolynNameState = 0;
	if (state.scene4080GominolaVisibleState > 1)
		state.scene4080GominolaVisibleState = 0;
	if (state.scene4090WideCoffinVariant > 1)
		state.scene4090WideCoffinVariant = 0;
	if (state.scene4090FinalCutsceneCompleted > 1)
		state.scene4090FinalCutsceneCompleted = 0;
}

} // End of namespace Hollywood
