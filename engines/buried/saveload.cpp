/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/translation.h"
#include "gui/message.h"
#include "gui/saveload.h"

#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/global_flags.h"
#include "buried/graphics.h"
#include "buried/inventory_window.h"
#include "buried/navdata.h"
#include "buried/scene_view.h"

namespace Buried {

#define SAVEGAME_CURRENT_VERSION 1

bool BuriedEngine::canLoadGameStateCurrently() {
	return !isDemo() && _mainWindow && !_yielding;
}

bool BuriedEngine::canSaveGameStateCurrently() {
	return !isDemo() && _mainWindow && !_yielding && ((FrameWindow *)_mainWindow)->isGameInProgress();
}

void BuriedEngine::checkForOriginalSavedGames() {
	Common::StringArray fileNames = _saveFileMan->listSavefiles("buried-*.sav");
	Common::StringArray newFileNames = _saveFileMan->listSavefiles("buried.###");
	Common::sort(newFileNames.begin(), newFileNames.end());
	if (fileNames.size() == 0)
		return;

	GUI::MessageDialog dialog(
		_("ScummVM found that you have saved games that should be converted from the original saved game format.\n"
		  "The original saved game format is no longer supported directly, so you will not be able to load your games if you don't convert them.\n\n"
		  "Press OK to convert them now, otherwise you will be asked again the next time you start the game.\n"),
		_("OK"), _("Cancel"));

	int choice = dialog.runModal();
	if (choice != GUI::kMessageOK)
		return;

	// Convert every save slot we find with the original naming scheme
	for (Common::StringArray::const_iterator file = fileNames.begin(); file != fileNames.end(); ++file) {
		int slotNum = 1;
		if (newFileNames.size() > 0) {
			Common::String lastFile = newFileNames.back();
			const char *slotStr = lastFile.c_str() + lastFile.size() - 3;
			slotNum = atoi(slotStr) + 1;
		}

		Common::String newFile = getMetaEngine()->getSavegameFile(slotNum);
		convertSavedGame(*file, newFile);
		newFileNames.push_back(newFile);
	}
}

enum {
	kSavedGameHeaderSize = 9,
	kSavedGameHeaderSizeAlt = 7
};

void BuriedEngine::convertSavedGame(Common::String oldFile, Common::String newFile) {
	static const byte s_savedGameHeader[kSavedGameHeaderSize] = {'B', 'I', 'T', 'M', 'P', 'C', 0, 5, 0};
	Location location;
	GlobalFlags flags;
	Common::Array<int> inventoryItems;
	byte header[9];

	debug("Converting %s to %s", oldFile.c_str(), newFile.c_str());

	// Read original/old saved game
	// Isolate the description from the file name
	Common::String desc = oldFile.c_str() + 7;
	for (int j = 0; j < 4; j++)
		desc.deleteLastChar();

	Common::InSaveFile *inFile = _saveFileMan->openForLoading(oldFile);
	inFile->read(header, kSavedGameHeaderSize);

	// Only compare the first 6 bytes
	// Win95 version of the game output garbage as the last two bytes
	if (inFile->eos() || memcmp(header, s_savedGameHeader, kSavedGameHeaderSizeAlt) != 0) {
		delete inFile;
		warning("Saved game %s is using an unsupported format, skipping", oldFile.c_str());
		return;
	}
	
	// Set necessary properties from the old save
	Common::Serializer inS(inFile, nullptr);
	Common::Error res = syncSaveData(inS, location, flags, inventoryItems);
	delete inFile;
	if (res.getCode() != Common::kNoError) {
		warning("Error reading data from saved game %s, skipping", oldFile.c_str());
		return;
	}

	flags.curItem = 0;	// did not persist in the original format, so set it here

	// Write the new saved format
	Common::OutSaveFile *outFile = _saveFileMan->openForSaving(newFile);
	if (!outFile) {
		warning("Error creating new save file %s", newFile.c_str());
		return;
	}

	const byte version = SAVEGAME_CURRENT_VERSION;
	Common::Serializer outS(nullptr, outFile);
	outS.setVersion(version);
	outFile->writeByte(version);

	if (syncSaveData(outS, location, flags, inventoryItems).getCode() == Common::kNoError) {
		getMetaEngine()->appendExtendedSave(outFile, getTotalPlayTime() / 1000, desc, false);

		outFile->finalize();
		delete outFile;

		// Delete the old saved game
		_saveFileMan->removeSavefile(oldFile);
	} else {
		delete outFile;

		warning("Error writing data to saved game %s, skipping", newFile.c_str());

		// The newly created saved game is corrupted, delete it
		_saveFileMan->removeSavefile(newFile);
	}
}

Common::Error BuriedEngine::loadGameStream(Common::SeekableReadStream *stream) {
	const byte version = stream->readByte();
	if (version > SAVEGAME_CURRENT_VERSION) {
		GUI::MessageDialog dialog(_s("Saved game was created with a newer version of ScummVM. Unable to load."));
		dialog.runModal();
		return Common::kUnknownError;
	}

	Common::Serializer ser(stream, nullptr);
	ser.setVersion(version);

	return syncSaveData(ser);
}

Common::Error BuriedEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	const byte version = SAVEGAME_CURRENT_VERSION;
	Common::Serializer ser(nullptr, stream);
	ser.setVersion(version);
	stream->writeByte(version);

	GameUIWindow *gameUI = (GameUIWindow *)((FrameWindow *)_mainWindow)->getMainChildWindow();
	gameUI->_bioChipRightWindow->destroyBioChipViewWindow();	// to capture a game screenshot
	_gfx->updateScreen();

	return syncSaveData(ser);
}

Common::Error BuriedEngine::syncSaveData(Common::Serializer &ser) {
	Common::Error result;

	if (ser.isLoading()) {
		Location location;
		GlobalFlags flags;
		Common::Array<int> inventoryItems;

		result = syncSaveData(ser, location, flags, inventoryItems);

		if (isTrial() && location.timeZone != 4) {
			// Display a message preventing the user from loading a non-apartment
			// saved game in the trial version
			GUI::MessageDialog dialog("ERROR: The location in this saved game is not included in this version of Buried in Time");
			dialog.runModal();
		} else {
			((FrameWindow *)_mainWindow)->loadFromState(location, flags, inventoryItems);
		}
	} else {
		Location location;
		GameUIWindow *gameUI = (GameUIWindow *)((FrameWindow *)_mainWindow)->getMainChildWindow();
		gameUI->_sceneViewWindow->getCurrentSceneLocation(location);
		GlobalFlags &flags = gameUI->_sceneViewWindow->getGlobalFlags();
		Common::Array<int> &inventoryItems = gameUI->_inventoryWindow->getItemArray();

		result = syncSaveData(ser, location, flags, inventoryItems);
	}

	return result;
}

Common::Error BuriedEngine::syncSaveData(Common::Serializer &ser, Location &location, GlobalFlags &flags, Common::Array<int> &inventoryItems) {
	if (!syncLocation(ser, location)) {
		warning("Error while synchronizing location data");
		return Common::kUnknownError;
	}

	if (!syncGlobalFlags(ser, flags)) {
		warning("Error while synchronizing global flag data");
		return Common::kUnknownError;
	}

	if (ser.err()) {
		warning("Error while synchronizing data");
		return Common::kUnknownError;
	}

	uint16 itemCount = inventoryItems.size();
	ser.syncAsUint16LE(itemCount);

	if (ser.isLoading()) {
		inventoryItems.clear();
		inventoryItems.reserve(itemCount);
	}

	for (uint16 i = 0; i < itemCount; i++) {
		uint16 itemId = 0;

		if (ser.isLoading()) {
			ser.syncAsUint16LE(itemId);
			inventoryItems.push_back(itemId);
		} else {
			itemId = inventoryItems[i];
			ser.syncAsUint16LE(itemId);
		}
	}

	if (ser.isSaving()) {
		// Fill in remaining items with all zeroes
		uint16 fillItems = 50 - itemCount;
		uint16 filler = 0;
		while (fillItems--)
			ser.syncAsUint16LE(filler);
	}

	if (ser.err()) {
		warning("Error while synchronizing inventory data");
		return Common::kUnknownError;
	}

	return Common::kNoError;
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
	SYNC_FLAG_UINT16(curItem);
	s.syncAsByte(flags.unused0);
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
	s.syncAsByte(flags.myMCViewedDeathGodDoor);
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
	s.syncAsByte(flags.dsCYWeebleClicked);
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
