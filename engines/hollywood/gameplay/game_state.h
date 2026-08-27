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

#ifndef HOLLYWOOD_GAMEPLAY_GAME_STATE_H
#define HOLLYWOOD_GAMEPLAY_GAME_STATE_H

#include "common/scummsys.h"

namespace Hollywood {

/**
 * Holds gameplay state shared across scene instances.
 *
 * Serialized fields cover the current flow and actor pose, inventory, puzzle
 * progression, travel unlocks, and user settings. reset() establishes new-game
 * defaults. Runtime-only cache flags are rebuilt or dirtied after loading.
 * Derived rendering and per-frame animation state remains owned by scene objects.
 */
struct GameplayState {
	enum {
		kInventoryOwnerCount = 2,
		kInventoryOwnerSlotStride = 0x79,
		kInventoryVerbCount = 8,
		kInventoryFirstSlot = 1,
		kInventoryLastSlot = kInventoryOwnerSlotStride - 1,
		kInventoryVisibleSlotCount = 16,
		kTravelScreenSlotCount = 8,
		kTravelScreenDisabledSlot = 0xff,
		kFrankensteinPartRewardCount = 3,
		kScene2050MuralTilePermutationSize = 49,
		kFixedInventoryActionTableEntryCount = kInventoryOwnerSlotStride * kInventoryVerbCount + 1,
		kInventoryItemRelationTableEntryCount = kInventoryOwnerSlotStride * kInventoryOwnerSlotStride
	};

	GameplayState() {
		reset();
	}

	void reset() {
		mainFlowStateId = 0;
		activeActorPoseValid = false;
		activeActorPoseStateId = 0;
		activeActorWorldX = 0;
		activeActorWorldY = 0;
		activeActorFacing = 1;
		activeActorCel = 0;
		activeViewportXOffset = 0;
		activeAudioChapterIndex = 0;
		currentInventoryOwnerIndex = 0;
		currentAmbientMusicCueId = 0;
		scene9140ReturnStateId = 1000;
		scene9140VariantIndex = 0;
		for (uint owner = 0; owner < kInventoryOwnerCount; ++owner) {
			inventoryItemCountByOwner[owner] = 0;
			inventoryFirstVisibleSlotByOwner[owner] = 0;
			for (uint slot = 0; slot < kInventoryOwnerSlotStride; ++slot) {
				inventorySlotItemIdByOwner[owner][slot] = 0;
				inventoryItemSlotByOwnerAndItemId[owner][slot] = 0;
				inventoryItemResourcePageByOwnerAndItemId[owner][slot] = 0;
			}
		}
		clearInventoryActionTables();
		sharedActorSpriteBankLoaded = false;
		sueInventoryResourceTablesLoaded = false;
		ronInventoryInitialized = false;
		sueInventoryInitialized = false;
		sceneActionCallbacksInstalled = false;
		inventoryPanelDirty = false;
		multiToolKnifeState = 0;
		ronTapeRecorderState = 0;
		ronTravelQuipIndex = 0;
		ronWalletOpened = false;
		ronEgyptianMoneyAmount = 0;
		ronPendingMabusePillsInMagnetPillbox = false;
		ronLampFueled = false;
		frankensteinDiaryRead = false;
		ronPosterPhotoRemoved = false;
		initializeTravelScreenSlots();
		ronTravelScreenUnlocked = false;
		travelScreenCurrentChapterId = 0;
		scene3010EntryLineSeen = false;
		scene3010ForestExitSeen = false;
		windmillBladesMoving = false;
		scene3020MaceTaken = false;
		scene3030MachineActivated = false;
		scene3040EntryLineSeen = false;
		scene3040HiddenObjectVisible = false;
		scene3050EntryLineSeen = false;
		scene3050LibraryCaptionRevealed = false;
		scene3060SecretDoorRevealState = 0;
		scene3060GlobeFrame = 0;
		scene3060GlobePuzzleRunIndex = 3;
		scene3060LastGlobePuzzleButton = 0;
		for (uint i = 0; i < sizeof(scene3060GlobePuzzleRuns); ++i)
			scene3060GlobePuzzleRuns[i] = 0;
		scene3060InspectedTitleFlags = 0;
		scene3060GlobeButtonsDiscovered = false;
		scene3070EntryLineSeen = false;
		scene3070DrawerOpen = false;
		scene3070SurgicalNeedleThreadState = 0;
		scene3070FrankensteinBodyState = 0;
		scene3070SurgicalNeedleThreadTaken = false;
		scene3070OperatingTableAlternateDescription = false;
		scene3070StoryPhase = 0;
		scene3070SerumIngredientCount = 0;
		scene3070OperatingTableForegroundAlternate = false;
		scene3070WindowForegroundPatchActive = false;
		scene3070InterludeCutsceneSeen = false;
		scene3070InterludeUnlocked = false;
		scene3070LateCutscenePlayed = false;
		scene3080EntryLineSeen = false;
		scene3080CabinDoorVisited = false;
		scene3080FrankensteinDiaryRevealed = false;
		scene3080FrankensteinDiaryTaken = false;
		scene3080WindowOpened = false;
		scene3080BranchTaken = false;
		scene3080ChimneySmokeAnimationChanged = false;
		scene3090EntryLineSeen = false;
		scene3090SecretDiaryPuzzleStage = 0;
		scene3090WindowOpenSequenceState = 0;
		scene3090BlindManPlayingSaxophone = false;
		scene3090SaltShakerTaken = false;
		scene3090DowsingRodTaken = false;
		scene3090BlindManConversationSeen = false;
		scene3090SecretDiaryPuzzleProgress = 0;
		scene3090DialogueMentionedBlindManLaxative = false;
		scene3100CabinVisited = false;
		scene3100GirlConversationState = 0;
		scene3100DaisyVisible = false;
		scene3100GirlDialogueRepeatCounter = 0;
		scene3100SapSyringeTaken = false;
		scene3100DaisyTaken = false;
		scene1010EntryLineSeen = false;
		scene1020EntryLineSeen = false;
		scene1020HookPositionState = 0;
		scene1020ChainAttachedToGrate = 0;
		scene1020GrateRaised = false;
		scene1020SueTapeVisible = false;
		scene1020BrokenRecorderIdentified = false;
		scene1020RustyRailGreased = false;
		scene1020SueTapeNoticed = false;
		scene1030EntryConversationSeen = false;
		scene1030TablePickupState = 0;
		scene1030SleepingDrunkInspected = false;
		scene1030ShrinkingManNamed = false;
		scene1040EntryLineSeen = false;
		scene1040CloakroomDoorOpened = false;
		scene1040GorillaCordState = 0;
		scene1040BalloonTaken = false;
		scene1050SuitcaseTaken = false;
		scene1050CloakroomAttendantConversationSeen = false;
		scene1050JackLookedAt = false;
		scene1050CharlieBogWerewolfClueHeard = false;
		scene1060EntryLineSeen = false;
		scene1060DrFlyState = 0;
		scene1060DrFlyConversationSeen = false;
		scene1060PocketPaperTaken = false;
		scene1060InvisibleManConversationSeen = false;
		scene1060PartyRemainsState = 0;
		scene1060FlySlimeHotspotActive = false;
		scene1070DoorOpened = false;
		scene1070ChainRemoved = false;
		scene1070SpiritBlockingHotspot = false;
		scene1070QuasimodoConversationSeen = false;
		scene1070SpencerConversationSeen = false;
		scene1070SpencerTravelClueProgress = 0;
		scene1070SpencerCocktailRecipeLearned = false;
		scene1070MicrophoneStandTaken = false;
		scene1070MicrophoneTaken = false;
		scene1080EntryLineSeen = false;
		scene1080FrancoisProgressState = 0;
		scene1090EntryLineSeen = false;
		scene1090LightsOn = false;
		scene1090WrappedBrainState = 0;
		scene2010EntryLineSeen = false;
		scene2010LongSequenceFirstSpeechSeen = false;
		scene2010B02EntranceUsed = false;
		scene2010TravelReturnSpeechState = 0;
		scene2020EntryLineSeen = false;
		scene2020PrincessGone = false;
		scene2020TigerToothState = 0;
		scene2020HatPresent = true;
		scene2020SunglassesPresent = true;
		scene2020PrincessConversationSeen = false;
		scene2030RightMerchantConversationSeen = false;
		scene2030SeedOfferState = 0;
		scene2030MerchantItem2AOfferState = 0;
		scene6010StudioEntryUnlocked = true;
		scene6010Item59Visible = false;
		scene6010DoorActionState = 0;
		scene6010ExitOverlayPlayed = false;
		scene6010EndgameTravelExitBlocked = false;
		scene6011PendingItem69Visible = false;
		scene6010Item58Taken = false;
		scene2040SphinxBasePatchState = 0;
		scene2040SphinxFaceState = 0;
		scene2040SphinxItemRevealed = 0;
		scene2040SphinxExitInterviewState = 0;
		scene2050EntrySpeechState = 0;
		scene2050MuralPuzzleState = 0;
		memset(scene2050MuralTilePermutation, 0, sizeof(scene2050MuralTilePermutation));
		scene2050SealRevealed = false;
		scene2050LabyrinthLampReady = false;
		egyptSealPuzzleProgress = 0;
		egyptLabyrinthPositionIndex = 0x2a;
		scene2070EntryProgress = 0;
		scene2070SealExitPatchState = 0;
		scene2070InnerPassagePatchState = 0;
		scene2070HiddenItemPatchState = 0;
		scene2080EntryLineSeen = false;
		scene2080ForegroundState = 2;
		scene2080ExitBackSequenceSeen = false;
		scene2080PrincessHairSearchState = 0;
		scene2080FirstRow09LookSeen = false;
		scene2080DialogueBranchBUnlocked = false;
		scene2080DialogueBranchAIndex = 0;
		scene2080DialogueBranchBIndex = 0;
		scene2080DialogueTerminalIndex = 0;
		scene2090EntryLineSeen = false;
		scene2100RaStaffTaken = false;
		scene2100PassageOpen = false;
		scene2100MummyBranchState = 0;
		scene2100MummyGreetingSeen = false;
		scene2100AfterlifeBranchUnlocked = false;
		scene2100MummyDialogueUsedChoiceMask = 0;
		scene2110EntryLineSeen = false;
		scene2110TreasureGrantIndex = 0;
		scene2110TreasureGranted = false;
		scene6020TaffyKnown = false;
		scene6020TaffyLeft = false;
		scene6030HannoverInterviewCompleted = false;
		scene6030SaxophoneTaken = false;
		scene6030HannoverAbsent = false;
		scene6030CoffeeState = 0;
		scene6040EntryLineSeen = false;
		scene6040PaintCanTaken = false;
		scene6040WireState = 0;
		scene6050MuseumInteriorUnlocked = false;
		scene6050GuardPresent = true;
		scene6050GuardAllowsEntry = false;
		scene6050DisplayCaseOpened = false;
		scene6070SuePresent = false;
		scene6070CellDoorOpen = false;
		scene6080Visited = false;
		scene6100EntryLineSeen = false;
		scene6100CharlieState = 1;
		scene6100BriefcasePresent = true;
		seenScene8010EntryLine = false;
		scene8010FishermanConversationState = 0;
		seenScene8020EntryLine = false;
		scene8020ForegroundObjectState = 0;
		scene8020SecondaryObjectVisible = false;
		scene4010AlternateBackgroundState = 0;
		scene4010FirstEntryConversationSeen = false;
		scene4010EntryPathSpeechState = 0;
		scene4010ProgressiveExitSpeechState = 0;
		scene4010Item3APickupState = 0;
		scene4010PillboxPickupState = 0;
		scene4010DestinationUnlocked = false;
		scene4020FallReactionLineSeen = false;
		scene4020GateUnlocked = false;
		scene4030InitialEntryLineSeen = false;
		scene4030RopeTaken = false;
		scene4030LooseBoneState = 0;
		scene4030ImprovisedLeverInstalled = false;
		scene4040EntryLineSeen = false;
		scene4040CandilTaken = false;
		scene4050EntryLineSeen = false;
		scene4050RopeSwingState = 0;
		scene4060PictureCardStage = 0;
		scene4060PerfumeBottleCardStage = 0;
		scene4060SherilynSheetWon = 0;
		scene4060EntryLineSeen = false;
		scene4060SherilynDialogueIntroSeen = false;
		scene4060SherilynPokerProgressCounter = 0;
		scene4070DraculaStage = 0;
		scene4070EntryLineSeen = false;
		scene4070TrophyBaseOpened = false;
		scene4070FrankiePartGranted = 0;
		scene4070SlimmingTreatmentApplied = false;
		scene4080GwendolynState = 1;
		scene4080GwendolynStateTransition = 0;
		scene4080CoffinShiftedState = 0;
		scene4080OilBottleState = 1;
		scene4080GwendolynNameState = 0;
		scene4080GominolaVisibleState = 0;
		scene4090InitialGreetingSeen = false;
		scene4090WideCoffinVariant = 0;
		scene4090OrganRevealDialogueSeen = false;
		scene4090FinalCutsceneCompleted = 0;
		scene4090FinalCutsceneDialogueSeen = false;
		scene4100EntryLineSeen = false;
		scene4110LetterTaken = false;
		scene4110BridgeOpened = false;
		seenScene5010EntryLine = false;
		scene5010MineTransportState = 0;
		scene5010MineTransportReady = false;
		scene5010SwitchPanelSeen = false;
		scene5010SwitchRow = 0;
		scene5010SwitchColumn = 0;
		scene5010MineCartDeparted = false;
		scene5010DestinationTableInitialized = false;
		for (uint i = 0; i < sizeof(scene5010DestinationStateBySwitchSlot) / sizeof(scene5010DestinationStateBySwitchSlot[0]); ++i)
			scene5010DestinationStateBySwitchSlot[i] = 0;
		scene5020ExplosivesCrateIdentified = false;
		scene5020WoodenPlankTaken = false;
		scene5030EntryLineSeen = false;
		scene5030DeckOfCardsState = 0;
		scene5030MusiciansNamed = false;
		scene5040EntryLineSeen = false;
		scene5040MineGalleryState = 0;
		scene5040LooseObjectTaken = false;
		scene5040OldSockTaken = false;
		scene5040DialState = 0;
		scene5040KarlDialogueIntroSeen = false;
		scene5040SpecialTransitionState = 0;
		scene5050EntryLineSeen = false;
		scene5050TrophyBoxTaken = false;
		scene5050PickupIndex = 0;
		scene5060EntryLineSeen = false;
		scene5060GasSmelled = false;
		scene5060RockTaken = false;
		scene5070ShovelTaken = false;
		scene5070AviatorCapState = 1;
		scene5080EntryLineSeen = false;
		scene5080PassageUnlocked = false;
		scene5080AlternatePassageSide = false;
		scene5080BookTaken = false;
		scene5080StairDoorBarrierSeen = false;
		scene5080StairDoorConstructionSeen = false;
		scene5090EntryLineSeen = false;
		scene5100EntryLineSeen = false;
		scene5100ButtonsUnlocked = false;
		scene5110IntroSeen = false;
		scene5110UnderwearTaken = false;
		scene5110BottleState = 0;
		scene5110SalonTransformState = 0;
		scene5110ElevatorTransitionSeen = false;
		scene5110MirrorTaken = false;
		scene5110JacuzziInspectionState = 0;
		scene5110WerewolfDialogueChoiceUnlocked = false;
		scene5120TongsTaken = false;
		scene5120CocktailState = 0;
		reviewedFrankensteinNote = false;
		frankensteinNoteOverlayMode = 0;
		hannoverCourtyardDialogueState = 0;
		hannoverCourtyardFollowUpSeen = false;
		seenJosephGuestListGreeting = false;
		officeStatueActionProgress = 0;
		officeNotePickupState = 0;
		openedOfficeClosetDoor = false;
		spokenToCloakroomAttendant = false;
		cloakroomRagVisible = 1;
		spokenToBruno = false;
		humeroBarrierState = 1;
		humeroBonePickupState = 0;
		punchBowlGlassPatchState = 1;
		activatedLabExitMachine = false;
		labMachineSpeed = 4;
		seenGramophoneRoomIntro = false;
		gramophoneRoomDoorState = 0;
		gramophoneCrankState = 0;
		seenHannoverOfficeIntro = false;
		crankOnHannoverDesk = true;
		seenHannoverBedroomIntro = false;
		movedBedroomArmor = false;
		cellPipesActive = true;
		cellPlateRatProgress = 0;
		posterOnCellWall = true;
		cellPlateRemoved = false;
		musicEnabled = true;
		soundEffectsEnabled = true;
		optionsTestAudioEnabled = false;
		musicVolumeLevel = 200;
		soundEffectsVolumeLevel = 200;
		voiceVolumeLevel = 200;
		speechTextSpeedLevel = 200;
		actorSpeechTextMode = 1;
	}

	// The original shares one reward index across all three trophy caches. These
	// accessors keep the existing serialized fields synchronized.
	byte frankensteinPartRewardIndex() const {
		return MAX<byte>(scene2110TreasureGrantIndex, scene5050PickupIndex);
	}

	void setFrankensteinPartRewardIndex(byte index) {
		scene2110TreasureGrantIndex = index;
		scene5050PickupIndex = index;
	}

	void initializeForState7000() {
		sharedActorSpriteBankLoaded = true;
		sueInventoryResourceTablesLoaded = false;
		sceneActionCallbacksInstalled = true;
		initializeSueItemResourcePages();
		initializeSueInventoryItems(false);
		currentInventoryOwnerIndex = 1;
		activeAudioChapterIndex = 7;
		currentAmbientMusicCueId = 0x0c;
		inventoryPanelDirty = true;
		mainFlowStateId = 0x1b62;
	}

	void initializeRonItemResourcePages() {
		if (kInventoryOwnerCount == 0)
			return;

		for (uint itemId = 0; itemId < kInventoryOwnerSlotStride; ++itemId)
			inventoryItemResourcePageByOwnerAndItemId[0][itemId] = 0;

		inventoryItemResourcePageByOwnerAndItemId[0][0x01] = 0x09;
		inventoryItemResourcePageByOwnerAndItemId[0][0x02] = 0x5c;
		inventoryItemResourcePageByOwnerAndItemId[0][0x03] = 0x03;
		inventoryItemResourcePageByOwnerAndItemId[0][0x04] = 0x04;
		inventoryItemResourcePageByOwnerAndItemId[0][0x05] = 0x0a;
		inventoryItemResourcePageByOwnerAndItemId[0][0x06] = 0x0d;
		inventoryItemResourcePageByOwnerAndItemId[0][0x07] = 0x47;
		inventoryItemResourcePageByOwnerAndItemId[0][0x08] = 0x48;
		inventoryItemResourcePageByOwnerAndItemId[0][0x09] = 0x06;
		inventoryItemResourcePageByOwnerAndItemId[0][0x0a] = 0x07;
		inventoryItemResourcePageByOwnerAndItemId[0][0x0b] = 0x4f;
		inventoryItemResourcePageByOwnerAndItemId[0][0x0c] = 0x50;
		inventoryItemResourcePageByOwnerAndItemId[0][0x0d] = 0x4f;
		inventoryItemResourcePageByOwnerAndItemId[0][0x0e] = 0x51;
		inventoryItemResourcePageByOwnerAndItemId[0][0x0f] = 0x50;
		inventoryItemResourcePageByOwnerAndItemId[0][0x10] = 0x61;
		inventoryItemResourcePageByOwnerAndItemId[0][0x11] = 0x17;
		inventoryItemResourcePageByOwnerAndItemId[0][0x12] = 0x6a;
		inventoryItemResourcePageByOwnerAndItemId[0][0x13] = 0x01;
		inventoryItemResourcePageByOwnerAndItemId[0][0x14] = 0x59;
		inventoryItemResourcePageByOwnerAndItemId[0][0x15] = 0x69;
		inventoryItemResourcePageByOwnerAndItemId[0][0x16] = 0x16;
		inventoryItemResourcePageByOwnerAndItemId[0][0x17] = 0x5e;
		inventoryItemResourcePageByOwnerAndItemId[0][0x18] = 0x67;
		inventoryItemResourcePageByOwnerAndItemId[0][0x19] = 0x02;
		inventoryItemResourcePageByOwnerAndItemId[0][0x1a] = 0x53;
		inventoryItemResourcePageByOwnerAndItemId[0][0x1b] = 0x5d;
		inventoryItemResourcePageByOwnerAndItemId[0][0x1c] = 0x5f;
		inventoryItemResourcePageByOwnerAndItemId[0][0x1d] = 0x05;
		inventoryItemResourcePageByOwnerAndItemId[0][0x1e] = 0x0b;
		inventoryItemResourcePageByOwnerAndItemId[0][0x1f] = 0x33;
		inventoryItemResourcePageByOwnerAndItemId[0][0x20] = 0x2e;
		inventoryItemResourcePageByOwnerAndItemId[0][0x21] = 0x04;
		inventoryItemResourcePageByOwnerAndItemId[0][0x23] = 0x4c;
		inventoryItemResourcePageByOwnerAndItemId[0][0x24] = 0x60;
		inventoryItemResourcePageByOwnerAndItemId[0][0x25] = 0x15;
		inventoryItemResourcePageByOwnerAndItemId[0][0x26] = 0x1a;
		inventoryItemResourcePageByOwnerAndItemId[0][0x27] = 0x1c;
		inventoryItemResourcePageByOwnerAndItemId[0][0x28] = 0x26;
		inventoryItemResourcePageByOwnerAndItemId[0][0x29] = 0x23;
		inventoryItemResourcePageByOwnerAndItemId[0][0x2a] = 0x28;
		inventoryItemResourcePageByOwnerAndItemId[0][0x2b] = 0x2c;
		inventoryItemResourcePageByOwnerAndItemId[0][0x2c] = 0x0c;
		inventoryItemResourcePageByOwnerAndItemId[0][0x2d] = 0x68;
		inventoryItemResourcePageByOwnerAndItemId[0][0x2e] = 0x27;
		inventoryItemResourcePageByOwnerAndItemId[0][0x2f] = 0x0f;
		inventoryItemResourcePageByOwnerAndItemId[0][0x30] = 0x5a;
		inventoryItemResourcePageByOwnerAndItemId[0][0x31] = 0x22;
		inventoryItemResourcePageByOwnerAndItemId[0][0x32] = 0x31;
		inventoryItemResourcePageByOwnerAndItemId[0][0x33] = 0x19;
		inventoryItemResourcePageByOwnerAndItemId[0][0x34] = 0x56;
		inventoryItemResourcePageByOwnerAndItemId[0][0x35] = 0x54;
		inventoryItemResourcePageByOwnerAndItemId[0][0x36] = 0x2b;
		inventoryItemResourcePageByOwnerAndItemId[0][0x37] = 0x2f;
		inventoryItemResourcePageByOwnerAndItemId[0][0x38] = 0x58;
		inventoryItemResourcePageByOwnerAndItemId[0][0x39] = 0x21;
		inventoryItemResourcePageByOwnerAndItemId[0][0x3a] = 0x39;
		inventoryItemResourcePageByOwnerAndItemId[0][0x3b] = 0x50;
		inventoryItemResourcePageByOwnerAndItemId[0][0x3c] = 0x3a;
		inventoryItemResourcePageByOwnerAndItemId[0][0x3d] = 0x5b;
		inventoryItemResourcePageByOwnerAndItemId[0][0x3e] = 0x44;
		inventoryItemResourcePageByOwnerAndItemId[0][0x3f] = 0x12;
		inventoryItemResourcePageByOwnerAndItemId[0][0x40] = 0x62;
		inventoryItemResourcePageByOwnerAndItemId[0][0x41] = 0x3f;
		inventoryItemResourcePageByOwnerAndItemId[0][0x42] = 0x37;
		inventoryItemResourcePageByOwnerAndItemId[0][0x43] = 0x30;
		inventoryItemResourcePageByOwnerAndItemId[0][0x44] = 0x42;
		inventoryItemResourcePageByOwnerAndItemId[0][0x45] = 0x10;
		inventoryItemResourcePageByOwnerAndItemId[0][0x46] = 0x24;
		inventoryItemResourcePageByOwnerAndItemId[0][0x47] = 0x1b;
		inventoryItemResourcePageByOwnerAndItemId[0][0x48] = 0x14;
		inventoryItemResourcePageByOwnerAndItemId[0][0x49] = 0x38;
		inventoryItemResourcePageByOwnerAndItemId[0][0x4a] = 0x3b;
		inventoryItemResourcePageByOwnerAndItemId[0][0x4b] = 0x1f;
		inventoryItemResourcePageByOwnerAndItemId[0][0x4c] = 0x66;
		inventoryItemResourcePageByOwnerAndItemId[0][0x4d] = 0x41;
		inventoryItemResourcePageByOwnerAndItemId[0][0x4e] = 0x57;
		inventoryItemResourcePageByOwnerAndItemId[0][0x4f] = 0x43;
		inventoryItemResourcePageByOwnerAndItemId[0][0x50] = 0x25;
		inventoryItemResourcePageByOwnerAndItemId[0][0x51] = 0x1d;
		inventoryItemResourcePageByOwnerAndItemId[0][0x52] = 0x4d;
		inventoryItemResourcePageByOwnerAndItemId[0][0x53] = 0x11;
		inventoryItemResourcePageByOwnerAndItemId[0][0x54] = 0x20;
		inventoryItemResourcePageByOwnerAndItemId[0][0x55] = 0x3d;
		inventoryItemResourcePageByOwnerAndItemId[0][0x56] = 0x29;
		inventoryItemResourcePageByOwnerAndItemId[0][0x57] = 0x49;
		inventoryItemResourcePageByOwnerAndItemId[0][0x58] = 0x3e;
		inventoryItemResourcePageByOwnerAndItemId[0][0x59] = 0x63;
		inventoryItemResourcePageByOwnerAndItemId[0][0x5a] = 0x4a;
		inventoryItemResourcePageByOwnerAndItemId[0][0x5b] = 0x4e;
		inventoryItemResourcePageByOwnerAndItemId[0][0x5c] = 0x64;
		inventoryItemResourcePageByOwnerAndItemId[0][0x5d] = 0x45;
		inventoryItemResourcePageByOwnerAndItemId[0][0x5e] = 0x35;
		inventoryItemResourcePageByOwnerAndItemId[0][0x5f] = 0x32;
		inventoryItemResourcePageByOwnerAndItemId[0][0x60] = 0x52;
		inventoryItemResourcePageByOwnerAndItemId[0][0x61] = 0x65;
		inventoryItemResourcePageByOwnerAndItemId[0][0x62] = 0x34;
		inventoryItemResourcePageByOwnerAndItemId[0][0x63] = 0x6b;
		inventoryItemResourcePageByOwnerAndItemId[0][0x64] = 0x55;
		inventoryItemResourcePageByOwnerAndItemId[0][0x65] = 0x4b;
		inventoryItemResourcePageByOwnerAndItemId[0][0x66] = 0x6e;
		inventoryItemResourcePageByOwnerAndItemId[0][0x67] = 0x6d;
		inventoryItemResourcePageByOwnerAndItemId[0][0x68] = 0x6c;
		inventoryItemResourcePageByOwnerAndItemId[0][0x69] = 0x3c;
		inventoryItemResourcePageByOwnerAndItemId[0][0x6a] = 0x08;
		inventoryItemResourcePageByOwnerAndItemId[0][0x6b] = 0x36;
		inventoryItemResourcePageByOwnerAndItemId[0][0x6c] = 0x46;
	}

	void initializeRonInventoryItems() {
		if (kInventoryOwnerCount == 0)
			return;

		const byte owner = 0;
		for (uint slot = 0; slot < kInventoryOwnerSlotStride; ++slot) {
			inventorySlotItemIdByOwner[owner][slot] = 0;
			inventoryItemSlotByOwnerAndItemId[owner][slot] = 0;
		}

		inventorySlotItemIdByOwner[owner][1] = 0x6a;
		inventorySlotItemIdByOwner[owner][2] = 0x09;
		inventorySlotItemIdByOwner[owner][3] = 0x01;
		inventoryItemSlotByOwnerAndItemId[owner][0x6a] = 1;
		inventoryItemSlotByOwnerAndItemId[owner][0x09] = 2;
		inventoryItemSlotByOwnerAndItemId[owner][0x01] = 3;
		inventoryItemCountByOwner[owner] = 3;
		inventoryFirstVisibleSlotByOwner[owner] = firstVisibleInventorySlotForCount(3);
		ronInventoryInitialized = true;
		inventoryPanelDirty = true;
	}

	void initializeTravelScreenSlots() {
		for (uint slot = 0; slot < kTravelScreenSlotCount; ++slot)
			travelScreenSlotIds[slot] = kTravelScreenDisabledSlot;

		// Verified in LoadResource000StartupTablesAndRuntimeGlobals: Ron starts
		// with destination 0 and destination 5 visible in the notebook.
		travelScreenSlotIds[0] = 0;
		travelScreenSlotIds[1] = 5;
	}

	bool hasTravelScreenDestination(byte destinationId) const {
		for (uint slot = 0; slot < kTravelScreenSlotCount; ++slot) {
			if (travelScreenSlotIds[slot] == destinationId)
				return true;
		}

		return false;
	}

	bool unlockTravelScreenDestination(byte destinationId) {
		if (destinationId >= 7 || hasTravelScreenDestination(destinationId))
			return false;

		for (uint slot = 2; slot < 7; ++slot) {
			if (travelScreenSlotIds[slot] == kTravelScreenDisabledSlot) {
				travelScreenSlotIds[slot] = destinationId;
				return true;
			}
		}

		return false;
	}

	void requestTravelScreenSelection(byte currentChapterId) {
		travelScreenCurrentChapterId = currentChapterId;
		mainFlowStateId = 0xffff;
		activeActorPoseValid = false;
	}

	void initializeSueItemResourcePages() {
		if (kInventoryOwnerCount <= 1)
			return;

		for (uint itemId = 0; itemId < kInventoryOwnerSlotStride; ++itemId)
			inventoryItemResourcePageByOwnerAndItemId[1][itemId] = 0;

		inventoryItemResourcePageByOwnerAndItemId[1][0x01] = 0x6f;
		inventoryItemResourcePageByOwnerAndItemId[1][0x02] = 0x7c;
		inventoryItemResourcePageByOwnerAndItemId[1][0x05] = 0x7a;
		inventoryItemResourcePageByOwnerAndItemId[1][0x06] = 0x78;
		inventoryItemResourcePageByOwnerAndItemId[1][0x07] = 0x06;
		inventoryItemResourcePageByOwnerAndItemId[1][0x08] = 0x76;
		inventoryItemResourcePageByOwnerAndItemId[1][0x09] = 0x77;
		inventoryItemResourcePageByOwnerAndItemId[1][0x0b] = 0x74;
		inventoryItemResourcePageByOwnerAndItemId[1][0x0c] = 0x71;
		inventoryItemResourcePageByOwnerAndItemId[1][0x0d] = 0x72;
		inventoryItemResourcePageByOwnerAndItemId[1][0x0f] = 0x70;
		inventoryItemResourcePageByOwnerAndItemId[1][0x10] = 0x75;
		inventoryItemResourcePageByOwnerAndItemId[1][0x11] = 0x73;
		inventoryItemResourcePageByOwnerAndItemId[1][0x13] = 0x79;
		inventoryItemResourcePageByOwnerAndItemId[1][0x14] = 0x65;
		inventoryItemResourcePageByOwnerAndItemId[1][0x15] = 0x6b;
		inventoryItemResourcePageByOwnerAndItemId[1][0x16] = 0x55;
		inventoryItemResourcePageByOwnerAndItemId[1][0x17] = 0x03;
		inventoryItemResourcePageByOwnerAndItemId[1][0x18] = 0x61;
		inventoryItemResourcePageByOwnerAndItemId[1][0x19] = 0x26;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1a] = 0x68;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1b] = 0x2f;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1c] = 0x3f;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1d] = 0x14;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1e] = 0x29;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1f] = 0x4a;
		inventoryItemResourcePageByOwnerAndItemId[1][0x20] = 0x52;
		inventoryItemResourcePageByOwnerAndItemId[1][0x21] = 0x4b;
		inventoryItemResourcePageByOwnerAndItemId[1][0x22] = 0x7b;
	}

	void initializeSueInventoryItems(bool giveAllItems) {
		if (kInventoryOwnerCount <= 1)
			return;

		const byte owner = 1;
		for (uint slot = 0; slot < kInventoryOwnerSlotStride; ++slot) {
			inventorySlotItemIdByOwner[owner][slot] = 0;
			inventoryItemSlotByOwnerAndItemId[owner][slot] = 0;
		}

		if (giveAllItems) {
			giveInventoryItemsWithResourcePages(owner);
			sueInventoryInitialized = true;
			return;
		}

		inventorySlotItemIdByOwner[owner][1] = 1;
		inventorySlotItemIdByOwner[owner][2] = 7;
		inventorySlotItemIdByOwner[owner][3] = 2;
		inventorySlotItemIdByOwner[owner][4] = 5;
		inventoryItemSlotByOwnerAndItemId[owner][1] = 1;
		inventoryItemSlotByOwnerAndItemId[owner][2] = 3;
		inventoryItemSlotByOwnerAndItemId[owner][5] = 4;
		inventoryItemSlotByOwnerAndItemId[owner][7] = 2;
		inventoryItemCountByOwner[owner] = 4;
		inventoryFirstVisibleSlotByOwner[owner] = firstVisibleInventorySlotForCount(4);
		sueInventoryInitialized = true;
		inventoryPanelDirty = true;
	}

	bool hasInventoryItem(byte owner, byte itemId) const {
		return inventorySlotForItem(owner, itemId) != 0;
	}

	byte inventorySlotForItem(byte owner, byte itemId) const {
		if (owner >= kInventoryOwnerCount || itemId == 0 || itemId >= kInventoryOwnerSlotStride)
			return 0;

		const byte itemCount = MIN<byte>(inventoryItemCountByOwner[owner], kInventoryLastSlot);
		for (byte slot = kInventoryFirstSlot; slot <= itemCount; ++slot) {
			if (inventorySlotItemIdByOwner[owner][slot] == itemId)
				return slot;
		}

		return 0;
	}

	void addInventoryItem(byte owner, byte itemId) {
		if (owner >= kInventoryOwnerCount || itemId == 0 || itemId >= kInventoryOwnerSlotStride)
			return;

		const byte existingSlot = inventorySlotForItem(owner, itemId);
		if (existingSlot != 0) {
			inventoryItemSlotByOwnerAndItemId[owner][itemId] = existingSlot;
			return;
		}

		if (inventoryItemCountByOwner[owner] >= kInventoryLastSlot)
			return;

		const byte slot = (byte)(inventoryItemCountByOwner[owner] + 1);
		inventoryItemCountByOwner[owner] = slot;
		inventorySlotItemIdByOwner[owner][slot] = itemId;
		inventoryItemSlotByOwnerAndItemId[owner][itemId] = slot;
		inventoryFirstVisibleSlotByOwner[owner] = firstVisibleInventorySlotForCount(slot);
		inventoryPanelDirty = true;
	}

	byte giveInventoryItemsWithResourcePages(byte owner) {
		if (owner >= kInventoryOwnerCount)
			return 0;

		for (uint slot = 0; slot < kInventoryOwnerSlotStride; ++slot) {
			inventorySlotItemIdByOwner[owner][slot] = 0;
			inventoryItemSlotByOwnerAndItemId[owner][slot] = 0;
		}

		byte writeSlot = kInventoryFirstSlot;
		for (byte itemId = kInventoryFirstSlot; itemId < kInventoryOwnerSlotStride; ++itemId) {
			if (inventoryItemResourcePageByOwnerAndItemId[owner][itemId] == 0)
				continue;

			inventorySlotItemIdByOwner[owner][writeSlot] = itemId;
			inventoryItemSlotByOwnerAndItemId[owner][itemId] = writeSlot;
			++writeSlot;
		}

		inventoryItemCountByOwner[owner] = (byte)(writeSlot - 1);
		inventoryFirstVisibleSlotByOwner[owner] = kInventoryFirstSlot;
		inventoryPanelDirty = true;
		return inventoryItemCountByOwner[owner];
	}

	void removeInventoryItem(byte owner, byte itemId) {
		if (owner >= kInventoryOwnerCount || itemId >= kInventoryOwnerSlotStride)
			return;

		byte slot = inventoryItemSlotByOwnerAndItemId[owner][itemId];
		if (slot >= kInventoryOwnerSlotStride || inventorySlotItemIdByOwner[owner][slot] != itemId)
			slot = inventorySlotForItem(owner, itemId);
		if (slot == 0)
			return;

		inventoryItemSlotByOwnerAndItemId[owner][itemId] = 0;
		inventorySlotItemIdByOwner[owner][slot] = 0;
		compactInventory(owner);
		inventoryPanelDirty = true;
	}

	void compactInventory(byte owner) {
		if (owner >= kInventoryOwnerCount)
			return;

		for (uint itemId = 0; itemId < kInventoryOwnerSlotStride; ++itemId)
			inventoryItemSlotByOwnerAndItemId[owner][itemId] = 0;

		byte writeSlot = kInventoryFirstSlot;
		for (byte readSlot = kInventoryFirstSlot; readSlot <= inventoryItemCountByOwner[owner]; ++readSlot) {
			const byte itemId = inventorySlotItemIdByOwner[owner][readSlot];
			if (itemId == 0 || itemId >= kInventoryOwnerSlotStride)
				continue;

			inventorySlotItemIdByOwner[owner][writeSlot] = itemId;
			inventoryItemSlotByOwnerAndItemId[owner][itemId] = writeSlot;
			++writeSlot;
		}

		for (byte slot = writeSlot; slot <= inventoryItemCountByOwner[owner]; ++slot)
			inventorySlotItemIdByOwner[owner][slot] = 0;
		inventoryItemCountByOwner[owner] = (byte)(writeSlot - 1);
		inventoryFirstVisibleSlotByOwner[owner] =
			firstVisibleInventorySlotForCount(inventoryItemCountByOwner[owner]);
	}

	byte firstVisibleInventorySlotForCount(byte itemCount) const {
		if (itemCount <= kInventoryVisibleSlotCount)
			return kInventoryFirstSlot;

		return (byte)(((itemCount - 9) & ~7) + 1);
	}

	uint16 fixedInventoryVerbHandler(byte owner, byte itemId, byte stripIndex) const {
		if (owner >= kInventoryOwnerCount || itemId >= kInventoryOwnerSlotStride || stripIndex == 0 ||
				stripIndex > kInventoryVerbCount)
			return 0;

		const uint index = (uint)itemId * kInventoryVerbCount + stripIndex;
		if (index >= kFixedInventoryActionTableEntryCount)
			return 0;

		return fixedInventoryVerbHandlerIdsByItemAndStrip[index];
	}

	uint16 dialogueInventoryRelationHandler(byte primaryItemId, byte secondaryItemId, byte relationMode) const {
		if (primaryItemId >= kInventoryOwnerSlotStride || secondaryItemId >= kInventoryOwnerSlotStride)
			return 0;

		const uint index = (uint)primaryItemId * kInventoryOwnerSlotStride + secondaryItemId;
		if (index >= kInventoryItemRelationTableEntryCount)
			return 0;

		if (relationMode == 1)
			return dialogueRelationMode1HandlerIdsByItemPair[index];
		if (relationMode == 2)
			return dialogueRelationMode2HandlerIdsByItemPair[index];

		return 0;
	}

	void clearInventoryActionTables() {
		for (uint i = 0; i < kFixedInventoryActionTableEntryCount; ++i)
			fixedInventoryVerbHandlerIdsByItemAndStrip[i] = 0;
		for (uint i = 0; i < kInventoryItemRelationTableEntryCount; ++i) {
			dialogueRelationMode1HandlerIdsByItemPair[i] = 0;
			dialogueRelationMode2HandlerIdsByItemPair[i] = 0;
		}
	}

	uint16 mainFlowStateId;
	bool activeActorPoseValid;
	uint16 activeActorPoseStateId;
	uint16 activeActorWorldX;
	uint16 activeActorWorldY;
	byte activeActorFacing;
	byte activeActorCel;
	uint16 activeViewportXOffset;
	byte activeAudioChapterIndex;
	byte currentInventoryOwnerIndex;
	byte currentAmbientMusicCueId;
	uint16 scene9140ReturnStateId;
	byte scene9140VariantIndex;
	byte inventoryItemCountByOwner[kInventoryOwnerCount];
	byte inventoryFirstVisibleSlotByOwner[kInventoryOwnerCount];
	byte inventorySlotItemIdByOwner[kInventoryOwnerCount][kInventoryOwnerSlotStride];
	byte inventoryItemSlotByOwnerAndItemId[kInventoryOwnerCount][kInventoryOwnerSlotStride];
	byte inventoryItemResourcePageByOwnerAndItemId[kInventoryOwnerCount][kInventoryOwnerSlotStride];
	uint16 fixedInventoryVerbHandlerIdsByItemAndStrip[kFixedInventoryActionTableEntryCount];
	uint16 dialogueRelationMode1HandlerIdsByItemPair[kInventoryItemRelationTableEntryCount];
	uint16 dialogueRelationMode2HandlerIdsByItemPair[kInventoryItemRelationTableEntryCount];
	bool sharedActorSpriteBankLoaded;
	bool sueInventoryResourceTablesLoaded;
	bool ronInventoryInitialized;
	bool sueInventoryInitialized;
	bool sceneActionCallbacksInstalled;
	bool inventoryPanelDirty;
	byte multiToolKnifeState;
	byte ronTapeRecorderState;
	byte ronTravelQuipIndex;
	bool ronWalletOpened;
	uint16 ronEgyptianMoneyAmount;
	bool ronPendingMabusePillsInMagnetPillbox;
	bool ronLampFueled;
	bool frankensteinDiaryRead;
	bool ronPosterPhotoRemoved;
	byte travelScreenSlotIds[kTravelScreenSlotCount];
	bool ronTravelScreenUnlocked;
	byte travelScreenCurrentChapterId;
	bool scene3010EntryLineSeen;
	bool scene3010ForestExitSeen;
	bool windmillBladesMoving;
	bool scene3020MaceTaken;
	bool scene3030MachineActivated;
	bool scene3040EntryLineSeen;
	bool scene3040HiddenObjectVisible;
	bool scene3050EntryLineSeen;
	bool scene3050LibraryCaptionRevealed;
	byte scene3060SecretDoorRevealState;
	byte scene3060GlobeFrame;
	byte scene3060GlobePuzzleRunIndex;
	byte scene3060LastGlobePuzzleButton;
	byte scene3060GlobePuzzleRuns[8];
	byte scene3060InspectedTitleFlags;
	bool scene3060GlobeButtonsDiscovered;
	bool scene3070EntryLineSeen;
	bool scene3070DrawerOpen;
	byte scene3070SurgicalNeedleThreadState;
	byte scene3070FrankensteinBodyState;
	bool scene3070SurgicalNeedleThreadTaken;
	bool scene3070OperatingTableAlternateDescription;
	byte scene3070StoryPhase;
	byte scene3070SerumIngredientCount;
	bool scene3070OperatingTableForegroundAlternate;
	bool scene3070WindowForegroundPatchActive;
	bool scene3070InterludeCutsceneSeen;
	bool scene3070InterludeUnlocked;
	bool scene3070LateCutscenePlayed;
	bool scene3080EntryLineSeen;
	bool scene3080CabinDoorVisited;
	bool scene3080FrankensteinDiaryRevealed;
	bool scene3080FrankensteinDiaryTaken;
	bool scene3080WindowOpened;
	bool scene3080BranchTaken;
	bool scene3080ChimneySmokeAnimationChanged;
	bool scene3090EntryLineSeen;
	byte scene3090SecretDiaryPuzzleStage;
	byte scene3090WindowOpenSequenceState;
	bool scene3090BlindManPlayingSaxophone;
	bool scene3090SaltShakerTaken;
	bool scene3090DowsingRodTaken;
	bool scene3090BlindManConversationSeen;
	byte scene3090SecretDiaryPuzzleProgress;
	bool scene3090DialogueMentionedBlindManLaxative;
	bool scene3100CabinVisited;
	byte scene3100GirlConversationState;
	bool scene3100DaisyVisible;
	byte scene3100GirlDialogueRepeatCounter;
	bool scene3100SapSyringeTaken;
	bool scene3100DaisyTaken;
	bool scene1010EntryLineSeen;
	bool scene1020EntryLineSeen;
	byte scene1020HookPositionState;
	byte scene1020ChainAttachedToGrate;
	bool scene1020GrateRaised;
	bool scene1020SueTapeVisible;
	bool scene1020BrokenRecorderIdentified;
	bool scene1020RustyRailGreased;
	bool scene1020SueTapeNoticed;
	bool scene1030EntryConversationSeen;
	byte scene1030TablePickupState;
	bool scene1030SleepingDrunkInspected;
	bool scene1030ShrinkingManNamed;
	bool scene1040EntryLineSeen;
	bool scene1040CloakroomDoorOpened;
	byte scene1040GorillaCordState;
	bool scene1040BalloonTaken;
	bool scene1050SuitcaseTaken;
	bool scene1050CloakroomAttendantConversationSeen;
	bool scene1050JackLookedAt;
	bool scene1050CharlieBogWerewolfClueHeard;
	bool scene1060EntryLineSeen;
	byte scene1060DrFlyState;
	bool scene1060DrFlyConversationSeen;
	bool scene1060PocketPaperTaken;
	bool scene1060InvisibleManConversationSeen;
	byte scene1060PartyRemainsState;
	bool scene1060FlySlimeHotspotActive;
	bool scene1070DoorOpened;
	bool scene1070ChainRemoved;
	bool scene1070SpiritBlockingHotspot;
	bool scene1070QuasimodoConversationSeen;
	bool scene1070SpencerConversationSeen;
	byte scene1070SpencerTravelClueProgress;
	bool scene1070SpencerCocktailRecipeLearned;
	bool scene1070MicrophoneStandTaken;
	bool scene1070MicrophoneTaken;
	bool scene1080EntryLineSeen;
	byte scene1080FrancoisProgressState;
	bool scene1090EntryLineSeen;
	bool scene1090LightsOn;
	byte scene1090WrappedBrainState;
	bool scene2010EntryLineSeen;
	bool scene2010LongSequenceFirstSpeechSeen;
	bool scene2010B02EntranceUsed;
	byte scene2010TravelReturnSpeechState;
	bool scene2020EntryLineSeen;
	bool scene2020PrincessGone;
	byte scene2020TigerToothState;
	bool scene2020HatPresent;
	bool scene2020SunglassesPresent;
	bool scene2020PrincessConversationSeen;
	bool scene2030RightMerchantConversationSeen;
	byte scene2030SeedOfferState;
	byte scene2030MerchantItem2AOfferState;
	bool scene6010StudioEntryUnlocked;
	bool scene6010Item59Visible;
	byte scene6010DoorActionState;
	bool scene6010ExitOverlayPlayed;
	bool scene6010EndgameTravelExitBlocked;
	bool scene6011PendingItem69Visible;
	bool scene6010Item58Taken;
	byte scene2040SphinxBasePatchState;
	byte scene2040SphinxFaceState;
	byte scene2040SphinxItemRevealed;
	byte scene2040SphinxExitInterviewState;
	byte scene2050EntrySpeechState;
	byte scene2050MuralPuzzleState;
	byte scene2050MuralTilePermutation[kScene2050MuralTilePermutationSize];
	bool scene2050SealRevealed;
	bool scene2050LabyrinthLampReady;
	byte egyptSealPuzzleProgress;
	byte egyptLabyrinthPositionIndex;
	byte scene2070EntryProgress;
	byte scene2070SealExitPatchState;
	byte scene2070InnerPassagePatchState;
	byte scene2070HiddenItemPatchState;
	bool scene2080EntryLineSeen;
	byte scene2080ForegroundState;
	bool scene2080ExitBackSequenceSeen;
	byte scene2080PrincessHairSearchState;
	bool scene2080FirstRow09LookSeen;
	bool scene2080DialogueBranchBUnlocked;
	byte scene2080DialogueBranchAIndex;
	byte scene2080DialogueBranchBIndex;
	byte scene2080DialogueTerminalIndex;
	bool scene2090EntryLineSeen;
	bool scene2100RaStaffTaken;
	bool scene2100PassageOpen;
	byte scene2100MummyBranchState;
	bool scene2100MummyGreetingSeen;
	bool scene2100AfterlifeBranchUnlocked;
	uint16 scene2100MummyDialogueUsedChoiceMask;
	bool scene2110EntryLineSeen;
	byte scene2110TreasureGrantIndex;
	bool scene2110TreasureGranted;
	bool scene6020TaffyKnown;
	bool scene6020TaffyLeft;
	bool scene6030HannoverInterviewCompleted;
	bool scene6030SaxophoneTaken;
	bool scene6030HannoverAbsent;
	byte scene6030CoffeeState;
	bool scene6040EntryLineSeen;
	bool scene6040PaintCanTaken;
	byte scene6040WireState;
	bool scene6050MuseumInteriorUnlocked;
	bool scene6050GuardPresent;
	bool scene6050GuardAllowsEntry;
	bool scene6050DisplayCaseOpened;
	bool scene6070SuePresent;
	bool scene6070CellDoorOpen;
	bool scene6080Visited;
	bool scene6100EntryLineSeen;
	byte scene6100CharlieState;
	bool scene6100BriefcasePresent;
	bool seenScene8010EntryLine;
	byte scene8010FishermanConversationState;
	bool seenScene8020EntryLine;
	byte scene8020ForegroundObjectState;
	bool scene8020SecondaryObjectVisible;
	byte scene4010AlternateBackgroundState;
	bool scene4010FirstEntryConversationSeen;
	byte scene4010EntryPathSpeechState;
	byte scene4010ProgressiveExitSpeechState;
	byte scene4010Item3APickupState;
	byte scene4010PillboxPickupState;
	bool scene4010DestinationUnlocked;
	bool scene4020FallReactionLineSeen;
	bool scene4020GateUnlocked;
	bool scene4030InitialEntryLineSeen;
	bool scene4030RopeTaken;
	byte scene4030LooseBoneState;
	bool scene4030ImprovisedLeverInstalled;
	bool scene4040EntryLineSeen;
	bool scene4040CandilTaken;
	bool scene4050EntryLineSeen;
	byte scene4050RopeSwingState;
	byte scene4060PictureCardStage;
	byte scene4060PerfumeBottleCardStage;
	byte scene4060SherilynSheetWon;
	bool scene4060EntryLineSeen;
	bool scene4060SherilynDialogueIntroSeen;
	byte scene4060SherilynPokerProgressCounter;
	byte scene4070DraculaStage;
	bool scene4070EntryLineSeen;
	bool scene4070TrophyBaseOpened;
	byte scene4070FrankiePartGranted;
	bool scene4070SlimmingTreatmentApplied;
	byte scene4080GwendolynState;
	byte scene4080GwendolynStateTransition;
	byte scene4080CoffinShiftedState;
	byte scene4080OilBottleState;
	byte scene4080GwendolynNameState;
	byte scene4080GominolaVisibleState;
	bool scene4090InitialGreetingSeen;
	byte scene4090WideCoffinVariant;
	bool scene4090OrganRevealDialogueSeen;
	byte scene4090FinalCutsceneCompleted;
	bool scene4090FinalCutsceneDialogueSeen;
	bool scene4100EntryLineSeen;
	bool scene4110LetterTaken;
	bool scene4110BridgeOpened;
	bool seenScene5010EntryLine;
	byte scene5010MineTransportState;
	bool scene5010MineTransportReady;
	bool scene5010SwitchPanelSeen;
	byte scene5010SwitchRow;
	byte scene5010SwitchColumn;
	bool scene5010MineCartDeparted;
	bool scene5010DestinationTableInitialized;
	uint16 scene5010DestinationStateBySwitchSlot[9];
	bool scene5020ExplosivesCrateIdentified;
	bool scene5020WoodenPlankTaken;
	bool scene5030EntryLineSeen;
	byte scene5030DeckOfCardsState;
	bool scene5030MusiciansNamed;
	bool scene5040EntryLineSeen;
	byte scene5040MineGalleryState;
	bool scene5040LooseObjectTaken;
	bool scene5040OldSockTaken;
	byte scene5040DialState;
	bool scene5040KarlDialogueIntroSeen;
	byte scene5040SpecialTransitionState;
	bool scene5050EntryLineSeen;
	bool scene5050TrophyBoxTaken;
	byte scene5050PickupIndex;
	bool scene5060EntryLineSeen;
	bool scene5060GasSmelled;
	bool scene5060RockTaken;
	bool scene5070ShovelTaken;
	byte scene5070AviatorCapState;
	bool scene5080EntryLineSeen;
	bool scene5080PassageUnlocked;
	bool scene5080AlternatePassageSide;
	bool scene5080BookTaken;
	bool scene5080StairDoorBarrierSeen;
	bool scene5080StairDoorConstructionSeen;
	bool scene5090EntryLineSeen;
	bool scene5100EntryLineSeen;
	bool scene5100ButtonsUnlocked;
	bool scene5110IntroSeen;
	bool scene5110UnderwearTaken;
	byte scene5110BottleState;
	byte scene5110SalonTransformState;
	bool scene5110ElevatorTransitionSeen;
	bool scene5110MirrorTaken;
	byte scene5110JacuzziInspectionState;
	bool scene5110WerewolfDialogueChoiceUnlocked;
	bool scene5120TongsTaken;
	byte scene5120CocktailState;
	bool reviewedFrankensteinNote;
	byte frankensteinNoteOverlayMode;
	byte hannoverCourtyardDialogueState;
	bool hannoverCourtyardFollowUpSeen;
	bool seenJosephGuestListGreeting;
	byte officeStatueActionProgress;
	byte officeNotePickupState;
	bool openedOfficeClosetDoor;
	bool spokenToCloakroomAttendant;
	byte cloakroomRagVisible;
	bool spokenToBruno;
	byte humeroBarrierState;
	byte humeroBonePickupState;
	byte punchBowlGlassPatchState;
	bool activatedLabExitMachine;
	byte labMachineSpeed;
	bool seenGramophoneRoomIntro;
	byte gramophoneRoomDoorState;
	byte gramophoneCrankState;
	bool seenHannoverOfficeIntro;
	bool crankOnHannoverDesk;
	bool seenHannoverBedroomIntro;
	bool movedBedroomArmor;
	bool cellPipesActive;
	byte cellPlateRatProgress;
	bool posterOnCellWall;
	bool cellPlateRemoved;
	bool musicEnabled;
	bool soundEffectsEnabled;
	bool optionsTestAudioEnabled;
	byte musicVolumeLevel;
	byte soundEffectsVolumeLevel;
	byte voiceVolumeLevel;
	byte speechTextSpeedLevel;
	byte actorSpeechTextMode;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_GAME_STATE_H
