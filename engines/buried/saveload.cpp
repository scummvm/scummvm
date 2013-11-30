/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/error.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/translation.h"

#include "buried/buried.h"
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/global_flags.h"
#include "buried/inventory_window.h"
#include "buried/navdata.h"
#include "buried/scene_view.h"

namespace Buried {

Common::StringArray BuriedEngine::listSaveFiles() {
	Common::StringArray fileNames = g_system->getSavefileManager()->listSavefiles("buried-*.sav");
	Common::sort(fileNames.begin(), fileNames.end());
	return fileNames;
}

bool BuriedEngine::canLoadGameStateCurrently() {
	// TODO: This probably needs to be more strict
	return !isDemo() && _mainWindow;
}

bool BuriedEngine::canSaveGameStateCurrently() {
	// TODO: This should be OK, except possibly synchronous video saving may give
	// weird results. Need to investigate.
	return !isDemo() && _mainWindow && ((FrameWindow *)_mainWindow)->isGameInProgress();
}

Common::Error BuriedEngine::loadGameState(int slot) {
	Common::StringArray fileNames = listSaveFiles();
	Common::InSaveFile *loadFile = _saveFileMan->openForLoading(fileNames[slot]);
	if (!loadFile)
		return Common::kUnknownError;	

	Location location;
	GlobalFlags flags;
	Common::Array<int> inventoryItems;
	if (!loadState(loadFile, location, flags, inventoryItems)) {
		delete loadFile;
		return Common::kUnknownError;
	}

	((FrameWindow *)_mainWindow)->loadFromState(location, flags, inventoryItems);
	delete loadFile;
	return Common::kNoError;
}

static bool isValidSaveFileChar(char c) {
	// Limit it to letters, digits, and a few other characters that should be safe
	return Common::isAlnum(c) || c == ' ' || c == '_' || c == '+' || c == '-' || c == '.';
}

static bool isValidSaveFileName(const Common::String &desc) {
	for (uint32 i = 0; i < desc.size(); i++)
		if (!isValidSaveFileChar(desc[i]))
			return false;

	return true;
}

Common::Error BuriedEngine::saveGameState(int slot, const Common::String &desc) {
	if (!isValidSaveFileName(desc))
		return Common::Error(Common::kCreatingFileFailed, _("Invalid save file name"));

	Common::String output = Common::String::format("buried-%s.sav", desc.c_str());
	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(output, false);
	if (!saveFile)
		return Common::kUnknownError;

	GameUIWindow *gameUI = (GameUIWindow *)((FrameWindow *)_mainWindow)->getMainChildWindow();

	Location location;
	gameUI->_sceneViewWindow->getCurrentSceneLocation(location);
	GlobalFlags &flags = gameUI->_sceneViewWindow->getGlobalFlags();
	Common::Array<int> &inventoryItems = gameUI->_inventoryWindow->getItemArray();

	if (!saveState(saveFile, location, flags, inventoryItems)) {
		delete saveFile;
		return Common::kUnknownError;
	}

	delete saveFile;
	return Common::kNoError;
}

enum {
	kSavedGameHeaderSize = 9,
	kSavedGameHeaderSizeAlt = 7
};

static const byte s_savedGameHeader[kSavedGameHeaderSize] = { 'B', 'I', 'T', 'M', 'P', 'C', 0, 5, 0 };

bool BuriedEngine::loadState(Common::SeekableReadStream *saveFile, Location &location, GlobalFlags &flags, Common::Array<int> &inventoryItems) {
	byte header[9];
	saveFile->read(header, kSavedGameHeaderSize);

	// Only compare the first 6 bytes
	// Win95 version of the game output garbage as the last two bytes
	if (saveFile->eos() || memcmp(header, s_savedGameHeader, kSavedGameHeaderSizeAlt) != 0)
		return false;

	Common::Serializer s(saveFile, 0);

	if (!syncLocation(s, location))
		return false;

	if (saveFile->eos())
		return false;

	if (!syncGlobalFlags(s, flags))
		return false;

	if (saveFile->eos())
		return false;

	uint16 itemCount = saveFile->readUint16LE();

	if (saveFile->eos())
		return false;

	inventoryItems.clear();
	for (uint16 i = 0; i < itemCount; i++)
		inventoryItems.push_back(saveFile->readUint16LE());

	return !saveFile->eos();
}

bool BuriedEngine::saveState(Common::WriteStream *saveFile, Location &location, GlobalFlags &flags, Common::Array<int> &inventoryItems) {
	saveFile->write(s_savedGameHeader, kSavedGameHeaderSize);

	Common::Serializer s(0, saveFile);

	if (!syncLocation(s, location))
		return false;

	if (!syncGlobalFlags(s, flags))
		return false;

	saveFile->writeUint16LE(inventoryItems.size());

	for (uint16 i = 0; i < inventoryItems.size(); i++)
		saveFile->writeUint16LE(inventoryItems[i]);

	// Fill in remaining items with all zeroes
	uint16 fillItems = 50 - inventoryItems.size();
	while (fillItems--)
		saveFile->writeUint16LE(0);

	return true;
}

// Since we can't take the address of a uint16 or uint32 from
// the packed GlobalFlags struct, we need to work around
// it this way.

#define SYNC_FLAG_UINT16(x) \
	do { \
		if (s.isSaving()) { \
			uint16 value = flags.x; \
			s.syncAsUint16LE(value); \
		} else { \
			uint16 value; \
			s.syncAsUint16LE(value); \
			flags.x = value; \
		} \
	} while (0)

#define SYNC_FLAG_UINT32(x) \
	do { \
		if (s.isSaving()) { \
			uint32 value = flags.x; \
			s.syncAsUint32LE(value); \
		} else { \
			uint32 value; \
			s.syncAsUint32LE(value); \
			flags.x = value; \
		} \
	} while (0)

bool BuriedEngine::syncLocation(Common::Serializer &s, Location &location) {
	s.syncAsSint16LE(location.timeZone);
	s.syncAsSint16LE(location.environment);
	s.syncAsSint16LE(location.node);
	s.syncAsSint16LE(location.facing);
	s.syncAsSint16LE(location.orientation);
	s.syncAsSint16LE(location.depth);
	return s.bytesSynced() == 12;
} 

bool BuriedEngine::syncGlobalFlags(Common::Serializer &s, GlobalFlags &flags) {
	uint32 startBytes = s.bytesSynced();

	s.syncAsByte(flags.cgWallExploded);
	s.syncAsByte(flags.cgHookPresent);
	s.syncAsByte(flags.cgArrowPresent);
	s.syncAsByte(flags.cgHammerPresent);
	s.syncAsByte(flags.cgSmithyStatus);
	s.syncAsByte(flags.cgSmithyGuard);
	s.syncAsByte(flags.cgBaileyOneWayGuard);
	s.syncAsByte(flags.cgBaileyTwoWayGuards);
	s.syncAsByte(flags.cgTapestryFlag);
	s.syncAsByte(flags.cgBurnedLetterPresent);
	s.syncAsByte(flags.cgGoldCoinsPresent);
	s.syncAsByte(flags.cgStorageRoomVisit);
	s.syncAsByte(flags.bcTranslateEnabled);
	s.syncAsByte(flags.bcCloakingEnabled);
	s.syncAsByte(flags.bcLocateEnabled);
	s.syncAsByte(flags.myPickedUpCeramicBowl);
	s.syncAsByte(flags.myTPCodeWheelStatus);
	s.syncAsByte(flags.myTPCodeWheelLeftIndex);
	s.syncAsByte(flags.myTPCodeWheelRightIndex);
	s.syncAsByte(flags.myMCPickedUpSkull);
	s.syncAsByte(flags.myMCDeathGodOfferings);
	s.syncAsByte(flags.myWGPlacedRope);
	s.syncAsByte(flags.myWGRetrievedJadeBlock);
	s.syncAsByte(flags.myWTRetrievedLimestoneBlock);
	s.syncAsByte(flags.myWTCurrentBridgeStatus);
	s.syncAsByte(flags.myAGRetrievedEntrySkull);
	s.syncAsByte(flags.myAGRetrievedSpearSkull);
	s.syncAsByte(flags.myAGRetrievedCopperMedal);
	s.syncAsByte(flags.myAGRetrievedObsidianBlock);
	s.syncAsByte(flags.myAGHeadAStatus);
	s.syncAsByte(flags.myAGHeadBStatus);
	s.syncAsByte(flags.myAGHeadCStatus);
	s.syncAsByte(flags.myAGHeadDStatus);
	s.syncAsByte(flags.myAGHeadAStatusSkullID);
	s.syncAsByte(flags.myAGHeadBStatusSkullID);
	s.syncAsByte(flags.myAGHeadCStatusSkullID);
	s.syncAsByte(flags.myAGHeadDStatusSkullID);
	s.syncAsByte(flags.myAGTimerHeadID);
	SYNC_FLAG_UINT32(myAGTimerStartTime);
	s.syncAsByte(flags.myDGOfferedHeart);
	s.syncAsByte(flags.takenEnvironCart);
	s.syncAsByte(flags.alRDTakenLiveCore);
	s.syncAsByte(flags.alRDTakenDeadCore);
	s.syncAsByte(flags.alNMWrongAlienPrefixCode);
	s.syncAsByte(flags.faKIOvenStatus);
	s.syncAsByte(flags.faKIPostBoxSlotA);
	s.syncAsByte(flags.faKIPostBoxSlotB);
	s.syncAsByte(flags.faKIPostBoxSlotC);
	s.syncAsByte(flags.faERCurrentCartridge);
	s.syncAsByte(flags.faERTakenRemoteControl);
	s.syncAsByte(flags.myMCStingerID);
	s.syncAsByte(flags.myMCStingerChannelID);
	s.syncAsByte(flags.faStingerID);
	s.syncAsByte(flags.faStingerChannelID);
	s.syncBytes(flags.unused0, sizeof(flags.unused0));
	SYNC_FLAG_UINT32(cgMWCatapultData);
	SYNC_FLAG_UINT32(cgMWCatapultOffset);
	s.syncAsByte(flags.cgTSTriedDoor);
	s.syncAsByte(flags.cgMBCrossedMoat);
	s.syncAsByte(flags.cgKSSmithyEntryRead);
	s.syncAsByte(flags.cgKSSmithyEntryTranslated);
	s.syncAsByte(flags.cgBSFoundMold);
	s.syncAsByte(flags.readBurnedLetter);
	s.syncAsByte(flags.evcapNumCaptured);
	s.syncBytes(flags.evcapBaseID, sizeof(flags.evcapBaseID));
	s.syncBytes(flags.unused1, sizeof(flags.unused1));
	s.syncAsByte(flags.faMNEnvironDoor);
	s.syncAsByte(flags.faMNClockClicked);
	s.syncAsByte(flags.faMNBooksClicked);
	s.syncAsByte(flags.faMNTazClicked);
	s.syncAsByte(flags.faMNPongClicked);
	s.syncAsByte(flags.faKIBirdsBobbed);
	s.syncAsByte(flags.faKICoffeeSpilled);
	s.syncAsByte(flags.cgViewedKeepPlans);
	s.syncAsByte(flags.cgFoundChestPanel);
	s.syncAsByte(flags.cgTRFoundSword);
	s.syncAsByte(flags.faHeardAgentFigure);
	s.syncAsByte(flags.jumpBCNoInfoMessageCycle);
	s.syncAsByte(flags.myTPCalendarTopTranslated);
	s.syncAsByte(flags.myTPCalendarListTranslated);
	s.syncAsByte(flags.myTPTextTranslated);
	s.syncAsByte(flags.myMCTransDoor);
	s.syncAsByte(flags.myMCTransAGOffering);
	s.syncAsByte(flags.myMCTransWGOffering);
	s.syncAsByte(flags.myMCTransWTOffering);
	s.syncAsByte(flags.myMCTransDGOffering);
	s.syncAsByte(flags.myMCTransMadeAnOffering);
	s.syncAsByte(flags.myWGTransDoorTop);
	s.syncAsByte(flags.myWGSeenLowerPassage);
	s.syncAsByte(flags.myWGCrossedRopeBridge);
	s.syncAsByte(flags.myWMCViewedDeathGodDoor);
	s.syncAsByte(flags.myTPTransBreathOfItzamna);
	SYNC_FLAG_UINT32(myAGHeadAOpenedTime);
	SYNC_FLAG_UINT32(myAGHeadBOpenedTime);
	SYNC_FLAG_UINT32(myAGHeadCOpenedTime);
	SYNC_FLAG_UINT32(myAGHeadDOpenedTime);
	s.syncAsByte(flags.myAGHeadATouched);
	s.syncAsByte(flags.myAGHeadBTouched);
	s.syncAsByte(flags.myAGHeadCTouched);
	s.syncAsByte(flags.myAGHeadDTouched);
	s.syncAsByte(flags.lensFilterActivated);
	s.syncAsByte(flags.dsPTElevatorPresent);
	s.syncAsByte(flags.dsPTElevatorLeverA);
	s.syncAsByte(flags.dsPTElevatorLeverB);
	s.syncAsByte(flags.dsPTDoorLocked);
	s.syncAsByte(flags.dsWSPickedUpWheelAssembly);
	s.syncAsByte(flags.dsWSPickedUpGearAssembly);
	s.syncAsByte(flags.dsWSPickedUpPegs);
	s.syncAsByte(flags.dsWSSiegeCycleStatus);
	s.syncAsByte(flags.dsWSGrabbedSiegeCycle);
	s.syncAsByte(flags.dsPTUseElevatorControls);
	s.syncAsByte(flags.dsPTTransElevatorControls);
	s.syncAsByte(flags.dsGDTakenCoilOfRope);
	s.syncAsByte(flags.dsCTUnlockedDoor);
	s.syncAsByte(flags.dsCTViewedAgent3);
	s.syncAsByte(flags.dsPTViewedAgent3);
	s.syncAsByte(flags.dsCTRetrievedLens);
	s.syncAsByte(flags.dsCTTakenHeart);
	s.syncAsByte(flags.dsCYFiredCannon);
	s.syncAsByte(flags.dsCYBallistaStatus);
	s.syncAsByte(flags.dsCYPlacedSiegeCycle);
	s.syncAsByte(flags.dsCYBallistaXPos);
	s.syncAsByte(flags.dsCYBallistaYPos);
	s.syncAsByte(flags.aiHWStingerID);
	s.syncAsByte(flags.aiHWStingerChannelID);
	s.syncAsByte(flags.aiCRStingerID);
	s.syncAsByte(flags.aiCRStingerChannelID);
	s.syncAsByte(flags.aiDBStingerID);
	s.syncAsByte(flags.aiDBStingerChannelID);
	s.syncAsByte(flags.aiCRGrabbedMetalBar);
	s.syncAsByte(flags.aiICGrabbedWaterCanister);
	s.syncAsByte(flags.aiOxygenTimer);
	s.syncAsByte(flags.aiCRPressurized);
	s.syncAsByte(flags.aiCRPressurizedAttempted);
	s.syncAsByte(flags.aiMRPressurized);
	s.syncAsByte(flags.aiIceMined);
	s.syncAsByte(flags.aiOxygenReserves);
	s.syncAsByte(flags.aiSCHeardInitialSpeech);
	s.syncAsByte(flags.aiSCInitialAudioChannel);
	s.syncAsByte(flags.aiSCDBDoorWarning);
	s.syncAsByte(flags.aiSCMoveCenterWarning);
	s.syncAsByte(flags.aiSCConversationStatus);
	s.syncAsByte(flags.aiHWIceDoorUnlocked);
	s.syncAsByte(flags.aiICWaterInFillHandle);
	s.syncAsByte(flags.aiICTakenWaterCanister);
	s.syncAsByte(flags.aiSWStingerID);
	s.syncAsByte(flags.aiSWStingerChannelID);
	s.syncAsByte(flags.aiMRCorrectFreqSet);
	s.syncAsByte(flags.aiSCHeardNexusDoorComment);
	s.syncAsByte(flags.aiSCHeardNexusDoorCode);
	s.syncAsByte(flags.asInitialGuardsPass);
	s.syncAsByte(flags.asRBPodAStatus);
	s.syncAsByte(flags.asRBPodBStatus);
	s.syncAsByte(flags.asRBPodCStatus);
	s.syncAsByte(flags.asRBPodDStatus);
	s.syncAsByte(flags.asRBPodEStatus);
	s.syncAsByte(flags.asRBPodFStatus);
	s.syncAsByte(flags.asRBPodATakenEnvironCart);
	s.syncAsByte(flags.asRBPodBTakenPuzzleBox);
	s.syncAsByte(flags.asRBPodCTakenCodex);
	s.syncAsByte(flags.asRBPodDTakenSculpture);
	s.syncAsByte(flags.asRBPodETakenSword);
	s.syncAsByte(flags.asTakenEvidenceThisTrip);
	s.syncAsByte(flags.asDangerDoorASealed);
	s.syncAsByte(flags.asDoorBGuardsSeen);
	s.syncAsByte(flags.asAmbassadorEncounter);
	s.syncAsByte(flags.dsCTTriedLockedDoor);
	s.syncAsByte(flags.dsCTCodexTranslateAttempted);
	s.syncAsByte(flags.dsCTCodexFormulaeFound);
	s.syncAsByte(flags.dsCTCodexAtlanticusPage2);
	s.syncAsByte(flags.dsCTTriedElevatorControls);
	s.syncAsByte(flags.aiDBPlayedMomComment);
	s.syncAsByte(flags.aiDBPlayedFirstArthur);
	s.syncAsByte(flags.aiDBPlayedSecondArthur);
	s.syncAsByte(flags.aiDBPlayedThirdArthur);
	s.syncAsByte(flags.aiDBPlayedFourthArthur);
	s.syncAsByte(flags.aiSCPlayedNoStinger);
	s.syncAsByte(flags.faKITakenPostboxItem);
	s.syncAsByte(flags.cgMBVisited);
	s.syncAsByte(flags.cgKCVisited);
	s.syncAsByte(flags.cgTRVisited);
	s.syncAsByte(flags.cgKSReadJournal);
	s.syncAsByte(flags.cgSRClickedOnLockedChest);
	s.syncAsByte(flags.cgSROpenedChest);
	s.syncAsByte(flags.dsVisitedCodexTower);
	s.syncAsByte(flags.dsPTRaisedPlatform);
	s.syncAsByte(flags.dsPTWalkedDownElevator);
	s.syncAsByte(flags.dsPTBeenOnBalcony);
	s.syncAsByte(flags.dsGDClickedOnCodexDoor);
	s.syncAsByte(flags.dsWSSeenCycleSketch);
	s.syncAsByte(flags.dsWSSeenBallistaSketch);
	s.syncAsByte(flags.genHadSiegeCycle);
	s.syncAsByte(flags.genHadDriveAssembly);
	s.syncAsByte(flags.genHadWheelAssembly);
	s.syncAsByte(flags.dsCYNeverConnectedHook);
	s.syncAsByte(flags.dsCYNeverShotBallista);
	s.syncAsByte(flags.dsCYNeverUsedCrank);
	s.syncAsByte(flags.dsCYNeverOpenedBalconyDoor);
	s.syncAsByte(flags.dsCYTranslatedCodex);
	s.syncAsByte(flags.dsCYTriedOpeningDoor);
	s.syncAsByte(flags.dsCYTriedElevator);
	s.syncAsByte(flags.dsCYFoundCodexes);
	s.syncAsByte(flags.myVisitedMainCavern);
	s.syncAsByte(flags.myVisitedArrowGod);
	s.syncAsByte(flags.myVisitedWaterGod);
	s.syncAsByte(flags.myVisitedWealthGod);
	s.syncAsByte(flags.myVisitedDeathGod);
	s.syncAsByte(flags.myVisitedSpecRooms);
	s.syncAsByte(flags.myWTSteppedOnSwings);
	s.syncAsByte(flags.myWTSteppedOnFarLedge);
	s.syncAsByte(flags.myDGOpenedPuzzleBox);
	s.syncAsByte(flags.myAGVisitedAltar);
	s.syncAsByte(flags.dsCTPlayedBallistaFalling);
	s.syncAsByte(flags.cgTSTriedDoorA);
	s.syncAsByte(flags.cgTSTriedDoorB);
	s.syncAsByte(flags.aiHWLastCommentPlayed);
	s.syncAsByte(flags.aiNXPlayedBrainComment);
	s.syncAsByte(flags.asRBLastStingerID);
	s.syncAsByte(flags.asRBStingerID);
	s.syncAsByte(flags.aiICProcessedOxygen);
	s.syncAsByte(flags.dsCYWeeblieClicked);
	s.syncAsByte(flags.aiICUsedMiningControls);
	s.syncAsByte(flags.aiSWAttemptedPresMR);
	s.syncAsByte(flags.aiICRefilledOxygen);
	s.syncAsByte(flags.aiMRUsedHarmonicsInterface);
	s.syncAsByte(flags.alRestoreSkipAgent3Initial);
	s.syncBytes(flags.unused2, sizeof(flags.unused2));
	s.syncAsByte(flags.scoreGotTranslateBioChip);
	s.syncAsByte(flags.scoreEnteredSpaceStation);
	s.syncAsByte(flags.scoreDownloadedArthur);
	s.syncAsByte(flags.scoreFoundSculptureDiagram);
	s.syncAsByte(flags.scoreEnteredKeep);
	s.syncAsByte(flags.scoreGotKeyFromSmithy);
	s.syncAsByte(flags.scoreEnteredTreasureRoom);
	s.syncAsByte(flags.scoreFoundSwordDiamond);
	s.syncAsByte(flags.scoreMadeSiegeCycle);
	s.syncAsByte(flags.scoreEnteredCodexTower);
	s.syncAsByte(flags.scoreLoggedCodexEvidence);
	s.syncAsByte(flags.scoreEnteredMainCavern);
	s.syncAsByte(flags.scoreGotWealthGodPiece);
	s.syncAsByte(flags.scoreGotRainGodPiece);
	s.syncAsByte(flags.scoreGotWarGodPiece);
	s.syncAsByte(flags.scoreCompletedDeathGod);
	s.syncAsByte(flags.scoreEliminatedAgent3);
	s.syncAsByte(flags.scoreTransportToKrynn);
	s.syncAsByte(flags.scoreGotKrynnArtifacts);
	s.syncAsByte(flags.scoreDefeatedIcarus);
	s.syncAsByte(flags.scoreResearchINNLouvreReport);
	s.syncAsByte(flags.scoreResearchINNHighBidder);
	s.syncAsByte(flags.scoreResearchINNAppeal);
	s.syncAsByte(flags.scoreResearchINNUpdate);
	s.syncAsByte(flags.scoreResearchINNJumpsuit);
	s.syncAsByte(flags.scoreResearchBCJumpsuit);
	s.syncAsByte(flags.scoreResearchMichelle);
	s.syncAsByte(flags.scoreResearchMichelleBkg);
	s.syncAsByte(flags.scoreResearchLensFilter);
	s.syncAsByte(flags.scoreResearchCastleFootprint);
	s.syncAsByte(flags.scoreResearchDaVinciFootprint);
	s.syncAsByte(flags.scoreResearchMorphSculpture);
	s.syncAsByte(flags.scoreResearchEnvironCart);
	s.syncAsByte(flags.scoreResearchAgent3Note);
	s.syncAsByte(flags.scoreResearchAgent3DaVinci);
	SYNC_FLAG_UINT16(scoreHintsTotal);
	s.syncBytes(flags.unused3, sizeof(flags.unused3));
	s.syncAsByte(flags.genJumpCastleBriefing);
	s.syncAsByte(flags.genJumpMayanBriefing);
	s.syncAsByte(flags.genJumpDaVinciBriefing);
	s.syncAsByte(flags.genJumpStationBriefing);
	s.syncBytes(flags.unused4, sizeof(flags.unused4));
	s.syncAsByte(flags.generalWalkthroughMode);
	s.syncBytes(flags.unused5, sizeof(flags.unused5));
	s.syncBytes(flags.aiData, sizeof(flags.aiData));

	return s.bytesSynced() - startBytes == 1024;
}

} // End of namespace Buried
