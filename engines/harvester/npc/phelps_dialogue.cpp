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


#include "harvester/npc/phelps_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kGetPrnMagActionTag = "GET_PRN_MAG";
static const char *const kDeleteDirtyMagazineActionTag = "DEL_DRT_MAG";
static const char *const kUseRewardAtStoreActionTag = "USE_REWARD_AT_STORE";
static const char *const kSetGenCuStuffActionTag = "SET_GEN_CU_STUFF";

static const DialogueLineEntry kPhelpsWhaleyLines[] = {
	{ 0x1c0c, "PHELPS", 4 },
	{ 0x1c13, "PC", 0 },
	{ 0x1c17, "PHELPS", 4 }
};

static const DialogueLineEntry kPhelpsCasketLines[] = {
	{ 0x1c1e, "PHELPS", 2 },
	{ 0x1c22, "PC", 0 },
	{ 0x1c27, "PHELPS", 0 },
	{ 0x1c2c, "PC", 0 },
	{ 0x1c30, "PHELPS", 2 }
};

static const DialogueLineEntry kPhelpsQuarterLines[] = {
	{ 0x1ba4, "PHELPS", 4 },
	{ 0x1bac, "PC", 0 },
	{ 0x1bb0, "PHELPS", 1 }
};

static const DialogueLineEntry kPhelpsIntroPreludeLines[] = {
	{ 0x1b44, "PHELPS", 1 },
	{ 0x1b48, "PC", 0 },
	{ 0x1b4c, "PHELPS", 0 },
	{ 0x1b52, "PC", 0 },
	{ 0x1b56, "PHELPS", 1 }
};

static const DialogueLineEntry kPhelpsKarinFoundDeadLines[] = {
	{ 0x1d2a, "PHELPS", 3 },
	{ 0x1d33, "PHELPS", 0 },
	{ 0x1d3a, "PHELPS", 0 }
};

static const DialogueLineEntry kPhelpsKarinKidnapedLines[] = {
	{ 0x1d40, "PHELPS", 4 },
	{ 0x1d46, "PC", 0 },
	{ 0x1d4a, "PHELPS", 0 },
	{ 0x1d4f, "PC", 0 },
	{ 0x1d53, "PHELPS", 0 }
};

} // End of namespace

Common::Error PhelpsDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	PhelpsRoomDialogueState &state = _state;

	if (usedItemName.empty()) {
		if (state.talkStatePending) {
			state.talkStatePending = false;
			Common::Error lineError = runtime.playDialogueEntrySequence(kPhelpsIntroPreludeLines,
				ARRAYSIZE(kPhelpsIntroPreludeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			sharedState.dialogueStateD2f04 = true;
			lineError = runtime.playDialogueLineWithVariant(0x1b5a, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x1b5e, "PHELPS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dialogueStateD2ed0) {
				lineError = runtime.playDialogueLineWithVariant(0x1b70, "PC", 3);
				if (lineError.getCode() == Common::kNoError)
					lineError = runtime.playDialogueLineWithVariant(0x1b75, "PHELPS", 1);
			} else {
				lineError = runtime.playDialogueLineWithVariant(runtime.startupScript().getFlagValue("HEARD_DAD_MOAN") ?
					0x1b6b : 0x1b67, "PC", 0);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x1bf4, "PHELPS", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		Common::Error lineError;

		if (runtime.startupScript().getCurrentStoryDayIndex() < 5) {
			lineError = runtime.playDialogueLineWithVariant(0x1b7b, "PHELPS", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x22a, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x1b85, "PHELPS", 0);
			} else {
				lineError = runtime.playDialogueLineWithVariant(0x1bbc, "PHELPS", 1);
				if (lineError.getCode() == Common::kNoError)
					lineError = runtime.playDialogueLineWithVariant(0x1c44, "PHELPS", 1);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") && !state.stephMidgameShown) {
			state.stephMidgameShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1c4a, "PHELPS", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getCurrentStoryDayIndex() == 5 && !state.dayFiveShown) {
			state.dayFiveShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1c56, "PHELPS", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") && !state.scratchedTuckerShown) {
			state.scratchedTuckerShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1c65, "PHELPS", 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
				!state.barberPoleStolenShown) {
			state.barberPoleStolenShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1c6e, "PHELPS", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x1c73, "PHELPS", 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
				!state.boltOfClothTakenShown) {
			state.boltOfClothTakenShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1c80, "PHELPS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (runtime.startupScript().isNamedNpcDeathTypeClear("FIREMAN2") &&
					runtime.startupScript().isNamedNpcDeathTypeClear("SPARKY")) {
				lineError = runtime.playDialogueLineWithVariant(0x1c87, "PHELPS", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		}
		if (runtime.startupScript().getFlagValue("DINER_BURNED") && !state.dinerBurnedShown) {
			state.dinerBurnedShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1c8f, "PHELPS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") && !state.escapedJailShown) {
			state.escapedJailShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1c98, "PHELPS", 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE") &&
				runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") &&
				!state.gotRemainsForLodgeShown) {
			state.gotRemainsForLodgeShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1ca0, "PHELPS", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
				!state.burnedTvStationShown) {
			state.burnedTvStationShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1cb4, "PHELPS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!runtime.startupScript().isNamedNpcDeathTypeClear("BUTCHER") &&
				!state.butcherDeadShown) {
			state.butcherDeadShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1cfe, "PHELPS", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") &&
				!state.moynahanDeadShown) {
			state.moynahanDeadShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1d06, "PHELPS", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY") &&
				!state.jimmyDeadShown) {
			state.jimmyDeadShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1d0f, "PHELPS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x22c, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x1d1c, "PHELPS", 1);
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x1d21, "PHELPS", 0);
			} else {
				lineError = Common::kNoError;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
				!state.karinFoundDeadShown) {
			state.karinFoundDeadShown = true;
			return runtime.playDialogueEntrySequence(kPhelpsKarinFoundDeadLines, ARRAYSIZE(kPhelpsKarinFoundDeadLines));
		}
		if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
				!state.karinKidnapedShown) {
			state.karinKidnapedShown = true;
			sharedState.karinKidnapedDialogueState = true;
			return runtime.playDialogueEntrySequence(kPhelpsKarinKidnapedLines, ARRAYSIZE(kPhelpsKarinKidnapedLines));
		}

		return Common::kNoError;
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		sharedState.discussedWhaleyHerrillPhoto = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return runtime.playDialogueEntrySequence(kPhelpsWhaleyLines, ARRAYSIZE(kPhelpsWhaleyLines));
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		sharedState.discussedCasketPhotoEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		return runtime.playDialogueEntrySequence(kPhelpsCasketLines, ARRAYSIZE(kPhelpsCasketLines));
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		sharedState.discussedLedgerEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		return runtime.playDialogueLineWithVariant(0x1c36, "PHELPS", 0);
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		sharedState.discussedNoteCheckbookEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		return runtime.playDialogueLineWithVariant(0x1c3d, "PHELPS", 1);
	}
	if (usedItemName.equalsIgnoreCase("QUARTER")) {
		runtime.executeAndQueueAction(kGetPrnMagActionTag);
		Common::Error lineError = runtime.playDialogueEntrySequence(kPhelpsQuarterLines, ARRAYSIZE(kPhelpsQuarterLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		runtime.executeAndQueueAction(kDeleteDirtyMagazineActionTag);
		return Common::kNoError;
	}
	if (usedItemName.equalsIgnoreCase("REWARD_MONEY")) {
		runtime.executeAndQueueAction(kUseRewardAtStoreActionTag);
		runtime.executeAndQueueAction(kSetGenCuStuffActionTag);
		return Common::kNoError;
	}

	return runtime.playDialogueLineWithVariant(0x1c06, "PHELPS", 1);
}

} // End of namespace Harvester
