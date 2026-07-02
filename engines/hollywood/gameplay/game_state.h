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
		sueInventoryInitialized = false;
		sceneActionCallbacksInstalled = false;
		inventoryPanelDirty = false;
		multiToolKnifeState = 0;
		ronTapeRecorderState = 0;
		ronWalletOpened = false;
		ronPendingMabusePillsInMagnetPillbox = false;
		ronLampFueled = false;
		initializeTravelScreenSlots();
		ronTravelScreenUnlocked = false;
		travelScreenCurrentChapterId = 0;
		seenScene3010EntryLine = false;
		windmillBladesMoving = false;
		scene3020Item31Taken = false;
		scene3030MachineActivated = false;
		seenScene3040EntryLine = false;
		scene3040ConditionalObjectVisible = false;
		seenScene3050EntryLine = false;
		scene3050Row3CaptionUpdated = false;
		scene3060SecretDoorState = 0;
		scene3060GlobeFrame = 0;
		scene3060GlobePuzzleSlot = 3;
		scene3060LastGlobeButton = 0;
		for (uint i = 0; i < sizeof(scene3060GlobePuzzleHistory); ++i)
			scene3060GlobePuzzleHistory[i] = 0;
		scene3060TitleFlags = 0;
		scene3060GlobeButtonsKnown = false;
		seenScene3070EntryLine = false;
		scene3070PatchDoorOpen = false;
		scene3070Item9PatchState = 0;
		scene3070Item10Visible = false;
		scene3070Item32Taken = false;
		scene3070Row3Alternate = false;
		scene3070Row12Alternate = false;
		scene3070BackLayerAlternateFrames = false;
		scene3070ForegroundAlternate = false;
		scene3070WindowPatchActive = false;
		seenScene3070InterludeCutscene = false;
		scene3070InterludeUnlocked = false;
		scene3070LateCutscenePlayed = false;
		seenScene3080EntryLine = false;
		scene3080DoorSeen = false;
		scene3080FrankensteinDiaryRevealed = false;
		scene3080DiaryTaken = false;
		scene3080WindowPatchActive = false;
		scene3080StickTaken = false;
		scene3080SmokeAlternateFrames = false;
		seenScene3090EntryLine = false;
		scene3090BlindManPuzzleStage = 0;
		scene3090WindowSequenceState = 0;
		scene3090BlindManPlayingSaxophone = false;
		scene3090SaltShakerTaken = false;
		scene3090DowsingRodTaken = false;
		scene3090TalkedToBlindMan = false;
		scene3090PuzzleProgress = 0;
		scene3090DialogueMentionedBlindManLaxative = false;
		seenScene3100EntrySequence = false;
		scene3100CabinState = 0;
		scene3100ObjectVisible = false;
		scene3100DialogueCounter = 0;
		scene3100Item38Taken = false;
		scene3100Item39Taken = false;
		seenScene1010EntryLine = false;
		seenScene1020EntryLine = false;
		scene1020ResourceBlockChoiceState = 0;
		scene1020ResourceBlockVariantState = 0;
		scene1020AlternateResourceBlockActive = false;
		scene1020EventFlag0 = false;
		scene1020EventFlag1 = false;
		scene1020EventFlag2 = false;
		scene1020EventFlag3 = false;
		seenScene1030EntryConversation = false;
		scene1030PatchState = 0;
		scene1030EventFlag0 = false;
		scene1030ShrinkingManNamed = false;
		seenScene1040EntryLine = false;
		scene1040DoorOpened = false;
		scene1040CordState = 0;
		scene1040BalloonTaken = false;
		scene1050SuitcaseTaken = false;
		scene1050TalkedToCloakroomAttendant = false;
		scene1050JackLookedAt = false;
		scene1050CloakroomSecretMentioned = false;
		seenScene1060EntryLine = false;
		scene1060FlyDoctorState = 0;
		seenScene1060DoctorConversation = false;
		scene1060PocketPaperTaken = false;
		seenScene1060InvisibleManConversation = false;
		scene1060PartyRemainsState = 0;
		scene1060FlySlimeHotspotActive = false;
		scene1070DoorOpened = false;
		scene1070ChainRemoved = false;
		scene1070SpiritBlockingHotspot = false;
		seenScene1070QuasimodoConversation = false;
		seenScene1070SpencerConversation = false;
		scene1070SpencerDialogueState = 0;
		scene1070MicrophoneStandTaken = false;
		scene1070MicrophoneTaken = false;
		scene1070SpencerExtraFlag = false;
		seenScene1080EntryLine = false;
		scene1080FrancoisState = 0;
		seenScene1090EntryLine = false;
		scene1090LightsOn = false;
		scene1090WrappedBrainState = 0;
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
		scene6020TaffyKnown = false;
		scene6020TaffyLeft = false;
		scene6030HannoverInterviewCompleted = false;
		scene6030CoffeeState = 0;
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
		scene4010Item3BPickupState = 0;
		scene4010DestinationUnlocked = false;
		seenScene4020FallReactionLine = false;
		scene4020GateUnlocked = false;
		seenScene4030EntryLine = false;
		scene4030RopeTaken = false;
		scene4030BoneState = 0;
		scene4030LeverInstalled = false;
		seenScene4040EntryLine = false;
		scene4040CandilTaken = false;
		seenScene4050EntryLine = false;
		scene4050PatchState = 0;
		scene4060CardStage = 0;
		scene4060SecondCardStage = 0;
		scene4060ForegroundState = 0;
		seenScene4060EntryLine = false;
		scene4060SherilynDialogueIntroSeen = false;
		scene4060DialogueProgressCounter = 0;
		scene4070DraculaStage = 0;
		seenScene4070EntryLine = false;
		scene4070TrophyBaseOpened = false;
		scene4070FrankiePartIndex = 0;
		scene4070SlimmingTreatmentApplied = false;
		scene4080PaletteMapState = 1;
		scene4080PendingPaletteMapPromotion = 0;
		scene4080SidePatchState = 0;
		scene4080PassagePatchState = 1;
		scene4080TextVariantState = 0;
		scene4080Resource13PatchState = 0;
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
		ronTravelScreenUnlocked = true;
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
		inventoryItemSlotByOwnerAndItemId[owner][9] = 2;
		inventoryItemCountByOwner[owner] = 4;
		inventoryFirstVisibleSlotByOwner[owner] = firstVisibleInventorySlotForCount(4);
		sueInventoryInitialized = true;
		inventoryPanelDirty = true;
	}

	bool hasInventoryItem(byte owner, byte itemId) const {
		if (owner >= kInventoryOwnerCount || itemId >= kInventoryOwnerSlotStride)
			return false;

		return inventoryItemSlotByOwnerAndItemId[owner][itemId] != 0;
	}

	void addInventoryItem(byte owner, byte itemId) {
		if (owner >= kInventoryOwnerCount || itemId >= kInventoryOwnerSlotStride ||
				hasInventoryItem(owner, itemId) ||
				inventoryItemCountByOwner[owner] >= kInventoryLastSlot)
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

		const byte slot = inventoryItemSlotByOwnerAndItemId[owner][itemId];
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

		byte writeSlot = kInventoryFirstSlot;
		for (byte readSlot = kInventoryFirstSlot; readSlot <= inventoryItemCountByOwner[owner]; ++readSlot) {
			const byte itemId = inventorySlotItemIdByOwner[owner][readSlot];
			if (itemId == 0)
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
	bool sueInventoryInitialized;
	bool sceneActionCallbacksInstalled;
	bool inventoryPanelDirty;
	byte multiToolKnifeState;
	byte ronTapeRecorderState;
	bool ronWalletOpened;
	bool ronPendingMabusePillsInMagnetPillbox;
	bool ronLampFueled;
	byte travelScreenSlotIds[kTravelScreenSlotCount];
	bool ronTravelScreenUnlocked;
	byte travelScreenCurrentChapterId;
	bool seenScene3010EntryLine;
	bool windmillBladesMoving;
	bool scene3020Item31Taken;
	bool scene3030MachineActivated;
	bool seenScene3040EntryLine;
	bool scene3040ConditionalObjectVisible;
	bool seenScene3050EntryLine;
	bool scene3050Row3CaptionUpdated;
	byte scene3060SecretDoorState;
	byte scene3060GlobeFrame;
	byte scene3060GlobePuzzleSlot;
	byte scene3060LastGlobeButton;
	byte scene3060GlobePuzzleHistory[8];
	byte scene3060TitleFlags;
	bool scene3060GlobeButtonsKnown;
	bool seenScene3070EntryLine;
	bool scene3070PatchDoorOpen;
	byte scene3070Item9PatchState;
	bool scene3070Item10Visible;
	bool scene3070Item32Taken;
	bool scene3070Row3Alternate;
	bool scene3070Row12Alternate;
	bool scene3070BackLayerAlternateFrames;
	bool scene3070ForegroundAlternate;
	bool scene3070WindowPatchActive;
	bool seenScene3070InterludeCutscene;
	bool scene3070InterludeUnlocked;
	bool scene3070LateCutscenePlayed;
	bool seenScene3080EntryLine;
	bool scene3080DoorSeen;
	bool scene3080FrankensteinDiaryRevealed;
	bool scene3080DiaryTaken;
	bool scene3080WindowPatchActive;
	bool scene3080StickTaken;
	bool scene3080SmokeAlternateFrames;
	bool seenScene3090EntryLine;
	byte scene3090BlindManPuzzleStage;
	byte scene3090WindowSequenceState;
	bool scene3090BlindManPlayingSaxophone;
	bool scene3090SaltShakerTaken;
	bool scene3090DowsingRodTaken;
	bool scene3090TalkedToBlindMan;
	byte scene3090PuzzleProgress;
	bool scene3090DialogueMentionedBlindManLaxative;
	bool seenScene3100EntrySequence;
	byte scene3100CabinState;
	bool scene3100ObjectVisible;
	byte scene3100DialogueCounter;
	bool scene3100Item38Taken;
	bool scene3100Item39Taken;
	bool seenScene1010EntryLine;
	bool seenScene1020EntryLine;
	byte scene1020ResourceBlockChoiceState;
	byte scene1020ResourceBlockVariantState;
	bool scene1020AlternateResourceBlockActive;
	bool scene1020EventFlag0;
	bool scene1020EventFlag1;
	bool scene1020EventFlag2;
	bool scene1020EventFlag3;
	bool seenScene1030EntryConversation;
	byte scene1030PatchState;
	bool scene1030EventFlag0;
	bool scene1030ShrinkingManNamed;
	bool seenScene1040EntryLine;
	bool scene1040DoorOpened;
	byte scene1040CordState;
	bool scene1040BalloonTaken;
	bool scene1050SuitcaseTaken;
	bool scene1050TalkedToCloakroomAttendant;
	bool scene1050JackLookedAt;
	bool scene1050CloakroomSecretMentioned;
	bool seenScene1060EntryLine;
	byte scene1060FlyDoctorState;
	bool seenScene1060DoctorConversation;
	bool scene1060PocketPaperTaken;
	bool seenScene1060InvisibleManConversation;
	byte scene1060PartyRemainsState;
	bool scene1060FlySlimeHotspotActive;
	bool scene1070DoorOpened;
	bool scene1070ChainRemoved;
	bool scene1070SpiritBlockingHotspot;
	bool seenScene1070QuasimodoConversation;
	bool seenScene1070SpencerConversation;
	byte scene1070SpencerDialogueState;
	bool scene1070MicrophoneStandTaken;
	bool scene1070MicrophoneTaken;
	bool scene1070SpencerExtraFlag;
	bool seenScene1080EntryLine;
	byte scene1080FrancoisState;
	bool seenScene1090EntryLine;
	bool scene1090LightsOn;
	byte scene1090WrappedBrainState;
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
	bool scene6020TaffyKnown;
	bool scene6020TaffyLeft;
	bool scene6030HannoverInterviewCompleted;
	byte scene6030CoffeeState;
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
	byte scene4010Item3BPickupState;
	bool scene4010DestinationUnlocked;
	bool seenScene4020FallReactionLine;
	bool scene4020GateUnlocked;
	bool seenScene4030EntryLine;
	bool scene4030RopeTaken;
	byte scene4030BoneState;
	bool scene4030LeverInstalled;
	bool seenScene4040EntryLine;
	bool scene4040CandilTaken;
	bool seenScene4050EntryLine;
	byte scene4050PatchState;
	byte scene4060CardStage;
	byte scene4060SecondCardStage;
	byte scene4060ForegroundState;
	bool seenScene4060EntryLine;
	bool scene4060SherilynDialogueIntroSeen;
	byte scene4060DialogueProgressCounter;
	byte scene4070DraculaStage;
	bool seenScene4070EntryLine;
	bool scene4070TrophyBaseOpened;
	byte scene4070FrankiePartIndex;
	bool scene4070SlimmingTreatmentApplied;
	byte scene4080PaletteMapState;
	byte scene4080PendingPaletteMapPromotion;
	byte scene4080SidePatchState;
	byte scene4080PassagePatchState;
	byte scene4080TextVariantState;
	byte scene4080Resource13PatchState;
	bool seenScene5010EntryLine;
	byte scene5010MineTransportState;
	bool scene5010MineTransportReady;
	bool scene5010SwitchPanelSeen;
	byte scene5010SwitchRow;
	byte scene5010SwitchColumn;
	bool scene5010MineCartDeparted;
	bool scene5010DestinationTableInitialized;
	uint16 scene5010DestinationStateBySwitchSlot[9];
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
