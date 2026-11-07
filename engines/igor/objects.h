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

#ifndef IGOR_OBJECTS_H
#define IGOR_OBJECTS_H

namespace Igor {

enum ObjectType {
	kObjectTypeInventory = 1,
	kObjectTypeRoom = 2
};

// One-based inventory object numbers, separate from object state and inventory lookup indices.
enum InventoryObjectNumber {
	kInventoryObjectBoomerang = 1,
	kInventoryObjectWallet = 2,
	kInventoryObjectParachute = 3,
	kInventoryObjectTplTickets = 4,
	kInventoryObjectRecords = 5,
	kInventoryObjectAlarmClock = 6,
	kInventoryObjectButterflyNet = 7,
	kInventoryObjectMatches = 8,
	kInventoryObjectHammer = 9,
	kInventoryObjectPickaxe = 10,
	kInventoryObjectDynamite = 11,
	kInventoryObjectHairpin = 12,
	kInventoryObjectDeansBook = 13,
	kInventoryObjectMonkeySuit = 14,
	kInventoryObjectNewspaper = 15,
	kInventoryObjectLabKeys = 16,
	kInventoryObjectBurger = 17,
	kInventoryObjectSlug = 18,
	kInventoryObjectPhoto = 19,
	kInventoryObjectFlask = 20,
	kInventoryObjectLoveNote = 21,
	kInventoryObjectFolder = 22,
	kInventoryObjectBottle = 23,
	kInventoryObjectMoney = 24,
	kInventoryObjectLizard = 25,
	kInventoryObjectRocket = 26,
	kInventoryObjectResin = 27,
	kInventoryObjectPapyrus = 28,
	kInventoryObjectPassword = 29,
	kInventoryObjectPen = 30,
	kInventoryObjectAstronomyBook = 31,
	kInventoryObjectLabNote = 32,
	kInventoryObjectFlower = 33,
	kInventoryObjectVase = 34,
	kInventoryObjectBiologyProject = 35,
	kInventoryObjectCamera = 36
};

/**
 * Indices for global game objects state, for better readability
 */
enum ObjectStateIndex {
	kBottleStateIndex = 0,
	kLizardFedStateIndex = 1,
	kPapyrusFiguresVisibleStateIndex = 2,
	kFolderSwappedStateIndex = 3,
	kAstronomyBookStateIndex = 4,
	kPhillipLockerCombinationLearnedStateIndex = 5,
	kVaseStateIndex = 6,
	kStatueDeliveredStateIndex = 7,
	kBedroomNightstandOpenStateIndex = 8,
	kAlarmClockTakenStateIndex = 9,
	kBedroomClosetOpenStateIndex = 10,
	kBedroomHoleOpenStateIndex = 11,
	kRecordTakenStateIndex = 12,
	kBedroomWindowStateIndex = 13,
	kRatSkinHatSeenStateIndex = 14,
	kBedroomWindowExitUsedStateIndex = 15,
	kAtticTrunkOpenStateIndex = 16,
	kButterflyNetTakenStateIndex = 17,
	kAtticWallHolePresentStateIndex = 18,
	kAtticLargeCrackPresentStateIndex = 19,
	kAtticSmallCrackPresentStateIndex = 20,
	kAtticNailHotspotActiveStateIndex = 21,
	kAtticNailExaminedStateIndex = 22,
	kDynamitePlacedStateIndex = 23,
	kAtticWindowExitTriedStateIndex = 24,
	kMatchesTakenStateIndex = 25,
	kDeanDoorOpenStateIndex = 26,
	kSecretaryDoorOpenStateIndex = 27,
	kDeanBookTakenStateIndex = 28,
	kDeanPassedOutStateIndex = 29,
	kPendingDeanDrinkStateIndex = 30,
	kNewspaperTakenStateIndex = 31,
	kIntercomUsedStateIndex = 32,
	kSecretaryClosetOpenStateIndex = 34,
	kSecretaryDismissedStateIndex = 35,
	kSecretaryClosetStateIndex = 36,
	kPersonalFileChangedStateIndex = 37,
	kHamburgerTakenStateIndex = 38,
	kSlugTakenStateIndex = 40,
	kGlassShardsPresentStateIndex = 41,
	kButterflyNetStateIndex = 42,
	kLizardTakenStateIndex = 44,
	kChurchDoorOpenStateIndex = 45,
	kProjectHandedToTobiasStateIndex = 46,
	kMoneyHandedToTobiasStateIndex = 47,
	kCuckooClockTriggeredStateIndex = 48,
	kCuckooAnimationControlStateIndex = 49,
	kSecretPassageEntranceOpenStateIndex = 50,
	kFlaskTakenStateIndex = 52,
	kPhotoCutsceneSeenStateIndex = 53,
	kPhillipAndJimmyPresentStateIndex = 54,
	kCollegeEntranceFoldersSwappedStateIndex = 55,
	kPhillipAndJimmyHaveLeftStateIndex = 56,
	kHairpinTakenStateIndex = 58,
	kToiletGirlsHaveLeftStateIndex = 59,
	kSpringBridgePaperStateIndex = 60,
	kPhotographerGoneStateIndex = 61,
	kTripodStateIndex = 62,
	kCameraStateIndex = 63,
	kPhysicsClassDoorStateIndex = 64,
	kMargaretStateIndex = 65,
	kLadiesRoomDoorStateIndex = 66,
	kChemistryDoorStateIndex = 67,
	kMensRoomDoorStateIndex = 68,
	kLaboratoryDoorOpenStateIndex = 69,
	kMazeBeatenStateIndex = 70,
	kMazeEntryDirectionStateIndex = 71,
	kAstronomyLabDoorStateIndex = 72,
	kLauraMetAtCollegeStateIndex = 73,
	kAstronomyBookTakenStateIndex = 74,
	kHarrisonPhotoStateIndex = 75,
	kHarrisonNoteShownStateIndex = 76,
	kBellTowerConversationSeenStateIndex = 78,
	kSeenChickThroughTelescopeStateIndex = 79,
	kLizardGivenStateIndex = 80,
	kOldLadyConversationSeenStateIndex = 81,
	kTreeResinDiscoveredStateIndex = 82,
	kPhillipLockerStateIndex = 84,
	kLockersExaminedStateIndex = 85,
	kCarolineFolderStateIndex = 87,
	kPenTakenStateIndex = 88,
	kPhysicsClassPaperTakenStateIndex = 89,
	kCemeteryEntrySideStateIndex = 90,
	kVaseTakenStateIndex = 91,
	kUndertakerLeftStateIndex = 92,
	kRecordGivenToUndertakerStateIndex = 93,
	kBorisIntroducedStateIndex = 94,
	kPart69FireflyCapturedStateIndex = 96,
	kPart69LeaderFireflyIdentifiedStateIndex = 97,
	kPart69SwarmAnimationPhaseStateIndex = 98,
	kPart69LeaderFireflyPathStateIndex = 99,
	kStatuetteTakenStateIndex = 100,
	kStatuetteExaminedStateIndex = 101,
	kFlowerTakenStateIndex = 102,
	kLighthouseMessageSentStateIndex = 104,
	kChurchBellMessageSeenStateIndex = 105,
	kStatuetteTaken2StateIndex = 106,
	kFlaskLeftInPhillipLockerStateIndex = 107,
	kPlaneDoorOpenStateIndex = 108,
	kParachuteTakenStateIndex = 109,
	kMargaretSceneStateIndex = 110,
	kExamCompletedStateIndex = 111
};

enum MargaretState {
	kMargaretStateInitial = 0,
	kMargaretStateNoteAltered,
	kMargaretStateReadNote,
	kMargaretStatePhotoTaken,
	kMargaretStatePhotoSequenceComplete
};

// Special Entries of _inventoryInfo after the objects: the number of the first
// visible object of the inventory and the number of objects held.
enum {
	kInventoryFirstVisible = 72,
	kInventoryCount = 73,
	kInventoryInfoSize = 74
};

enum Verb {
	kVerbWalk = 1,
	kVerbTalk,
	kVerbTake,
	kVerbLook,
	kVerbUse,
	kVerbOpen,
	kVerbClose,
	kVerbGive
};

} // End of namespace Igor

#endif // IGOR_OBJECTS_H
