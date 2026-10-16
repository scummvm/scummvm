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


#include "harvester/npc/edna_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kRahRoomName = "RAH";
static const char *const kDialogueC043FstPath = "GRAPHIC/FST/C043.FST";
static const char *const kDialogueC043KFstPath = "GRAPHIC/FST/C043K.FST";
static const char *const kDialogueC070FstPath = "GRAPHIC/FST/C070.FST";
static const char *const kDialogueC091FstPath = "GRAPHIC/FST/C091.FST";

static const int kEdnaIntroResponseMenuLine = 0x9f;
static const int kEdnaInitialTopicBufferLine = 0xa0;
static const int kEdnaDayFiveAliveResponseMenuLine = 0xa1;
static const int kEdnaKarinFoundDeadResponseMenuLine = 0xa2;
static const int kEdnaKarinPurseAvailableResponseMenuLine = 0xa3;
static const int kEdnaKarinPurseUnavailableResponseMenuLine = 0xa4;
static const int kEdnaExitTopicLine = 0xa5;
static const int kEdnaAcAdTopicLineIndices[] = { 0xac, 0xad };
static const int kEdnaLodgeTopicLineIndices[] = { 0xb6, 0xb7 };
static const int kEdnaBcBdBeTopicLineIndices[] = { 0xbc, 0xbd, 0xbe };

static const DialogueLineEntry kEdnaBringKarinToSheriffLines[] = {
	{ 0x3cce, "EDNA", 2 },
	{ 0x3cd2, "PC", 0 },
	{ 0x3cd7, "EDNA", 1 }
};

static const DialogueLineEntry kEdnaIntroResponseTwoLines[] = {
	{ 0x3ab2, "EDNA", 0 },
	{ 0x3ab7, "PC", 0 },
	{ 0x3abb, "EDNA", 0 },
	{ 0x3ac2, "PC", 0 }
};

static const DialogueLineEntry kEdnaDayFiveAliveResponseOneLines[] = {
	{ 0x3c23, "EDNA", 0 },
	{ 0x3c29, "PC", 0 },
	{ 0x3c2e, "EDNA", 3 }
};

static const DialogueLineEntry kEdnaBarberPoleStolenLines[] = {
	{ 0x3c4e, "EDNA", 3 },
	{ 0x3c54, "PC", 0 },
	{ 0x3c5a, "EDNA", 2 }
};

static const DialogueLineEntry kEdnaKarinFoundDeadIntroLines[] = {
	{ 0x3cf0, "EDNA", 2 },
	{ 0x3cf5, "PC", 0 },
	{ 0x3cf9, "EDNA", 2 }
};

static const DialogueLineEntry kEdnaTopicB2Lines[] = {
	{ 0x3b15, "EDNA", 1 },
	{ 0x3b1a, "PC", 0 },
	{ 0x3b1e, "EDNA", 0 }
};

static const DialogueLineEntry kEdnaLodgeTopicLines[] = {
	{ 0x3b57, "EDNA", 0 },
	{ 0x3b5d, "PC", 0 },
	{ 0x3b61, "EDNA", 0 },
	{ 0x3b62, "EDNA", 0 },
	{ 0x3b63, "EDNA", 0 },
	{ 0x3b6c, "PC", 0 },
	{ 0x3b70, "EDNA", 0 },
	{ 0x3b75, "PC", 0 },
	{ 0x3b79, "EDNA", 0 }
};

static const DialogueLineEntry kEdnaTopicBcBdBeLines[] = {
	{ 0x3b8e, "EDNA", 0 },
	{ 0x3b8f, "EDNA", 0 }
};

} // End of namespace

Common::Error EdnaDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	EdnaRoomDialogueState &state = _state;
	Script &startupScript = runtime.startupScript();
	Common::String topicBuffer;
	int topicBufferLineIndex = -1;

	DialogueTopicBuffer topics(runtime, topicBuffer, topicBufferLineIndex, "Edna topic buffer");

	auto completeKarinPurseExchange = [&]() -> Common::Error {
		state.karinPurseLinePlayed = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3d05, "EDNA", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)startupScript.addRuntimeObjectToInventory("REWARD_MONEY");
		(void)startupScript.setRuntimeObjectVisible(kRahRoomName, "K_PURSE", false);
		return Common::kNoError;
	};

	if (startupScript.getFlagValue("KILLED_KARIN1")) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3cdf, "EDNA", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		(void)startupScript.setRuntimeFlagValue("KILLED_KARIN1", false);
		return Common::kNoError;
	}

	if (startupScript.getFlagValue("DNA_S_SUICIDE_NOTE")) {
		(void)startupScript.setRuntimeFlagValue("DNA_S_SUICIDE_NOTE", false);
		Common::Error lineError = runtime.playDialogueFst(
			startupScript.getFlagValue("KARIN_FOUND_ALIVE")
				? kDialogueC043KFstPath
				: kDialogueC043FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		sharedState.dialogueStateD2ea4 = true;
		sharedState.dialogueStateD2ea8 = true;
		(void)runtime.startupScript().setRuntimeObjectVisible("DNAEXT", "SIGNOUT", true);
		return Common::kNoError;
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("K_PURSE"))
			return completeKarinPurseExchange();
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			sharedState.discussedCasketPhotoEvidence = 1;
			return runtime.playDialogueLineWithVariant(0x3bd6, "EDNA", 0);
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
				startupScript.getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			sharedState.discussedLedgerEvidence = 1;
			return runtime.playDialogueLineWithVariant(0x3bd6, "EDNA", 0);
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			sharedState.discussedWhaleyHerrillPhoto = 1;
			return runtime.playDialogueLineWithVariant(0x3bde, "EDNA", 2);
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			sharedState.discussedNoteCheckbookEvidence = 1;
			return runtime.playDialogueLineWithVariant(0x3be5, "EDNA", 2);
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);
			sharedState.discussedTvDeedEvidence = 1;
			return runtime.playDialogueLineWithVariant(0x3bee, "EDNA", 0);
		}

		return runtime.playDialogueLineWithVariant(0x3bcf, "EDNA", 0);
	}

	if (startupScript.getFlagValue("BRING_KARIN_TO_SHERIFF")) {
		Common::Error lineError = runtime.playDialogueEntrySequence(kEdnaBringKarinToSheriffLines,
			ARRAYSIZE(kEdnaBringKarinToSheriffLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)startupScript.addRuntimeObjectToInventory("REWARD_MONEY");
		(void)startupScript.setRuntimeFlagValue("BRING_KARIN_TO_SHERIFF", false);
		return Common::kNoError;
	}

	if (state.introPending) {
		state.introPending = false;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3a95, "EDNA", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(kEdnaIntroResponseMenuLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x3aa1, "EDNA", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLine(0x3aa6, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x3aaa, "EDNA", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.dialogueStateD2f04 = true;
			lineError = runtime.playDialogueLine(0x3aae, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueEntrySequence(kEdnaIntroResponseTwoLines, ARRAYSIZE(kEdnaIntroResponseTwoLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = runtime.playDialogueLineWithVariant(0x3ac6, "EDNA", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x3aca, "EDNA", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x3acf, "EDNA", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		topics.assign(kEdnaInitialTopicBufferLine);
	}

	if (!startupScript.getFlagValue("KARIN_KIDNAPED")) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3bf8, "EDNA", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3bff, "EDNA", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		sharedState.karinKidnapedDialogueState = true;
	}

	if (startupScript.getFlagValue("DAY_5") && !state.dayFiveShown) {
		state.dayFiveShown = true;
		if (startupScript.getFlagValue("KARIN_FOUND_ALIVE")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x3c17, "EDNA", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kEdnaDayFiveAliveResponseMenuLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueEntrySequence(kEdnaDayFiveAliveResponseOneLines,
					ARRAYSIZE(kEdnaDayFiveAliveResponseOneLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x3c33, "EDNA", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		} else if (startupScript.getFlagValue("KARIN_FOUND_DEAD")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x3c3a, "EDNA", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (startupScript.getFlagValue("SCRATCHED_TUCKER") && !state.scratchedTuckerShown) {
		state.scratchedTuckerShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3c45, "EDNA", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (startupScript.getFlagValue("BARBER_POLE_STOLEN") &&
			!state.barberPoleStolenShown) {
		state.barberPoleStolenShown = true;
		Common::Error lineError = runtime.playDialogueEntrySequence(kEdnaBarberPoleStolenLines,
			ARRAYSIZE(kEdnaBarberPoleStolenLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (startupScript.getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
			!state.boltOfClothTakenShown) {
		state.boltOfClothTakenShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3c62, "EDNA", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (startupScript.getFlagValue("PC_ESCAPED_JAIL") &&
			!state.escapedJailShown) {
		state.escapedJailShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3c92, "EDNA", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (startupScript.getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3ca2, "EDNA", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (startupScript.getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3ce5, "EDNA", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kEdnaKarinFoundDeadResponseMenuLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = runtime.playDialogueEntrySequence(kEdnaKarinFoundDeadIntroLines,
				ARRAYSIZE(kEdnaKarinFoundDeadIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			const bool canDiscussKarinPurse = state.karinPurseLinePlayed ||
				startupScript.isObjectInInventory("K_PURSE");
			responseIndex = 0;
			responseError = runtime.runResponseMenu(
				canDiscussKarinPurse
					? kEdnaKarinPurseAvailableResponseMenuLine
					: kEdnaKarinPurseUnavailableResponseMenuLine,
				responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				if (canDiscussKarinPurse) {
					lineError = completeKarinPurseExchange();
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x3d0a, "EDNA", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x3d0e, "EDNA", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			topicBuffer, topicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()) ||
				runtime.matchesResponseLine(selectedTopic, kEdnaExitTopicLine)) {
			if (!startupScript.getFlagValue("KARIN_KIDNAPED") &&
					!startupScript.getFlagValue("KARIN_FOUND_DEAD") &&
					!startupScript.getFlagValue("KARIN_FOUND_ALIVE")) {
				Common::Error lineError = runtime.playDialogueLineWithVariant(0x3bc3, "EDNA", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			return Common::kNoError;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0xa6)) {
			if (!startupScript.getFlagValue("KARIN_KIDNAPED") &&
					!startupScript.getFlagValue("KARIN_FOUND_DEAD")) {
				Common::Error lineError = runtime.playDialogueLineWithVariant(0x3ad9, "EDNA", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kDialogueC091FstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else {
				Common::Error lineError = runtime.playDialogueLineWithVariant(0x3ae3, "EDNA", 3);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				if (!startupScript.getFlagValue("KARIN_FOUND_DEAD")) {
					lineError = runtime.playDialogueLineWithVariant(0x3ae9, "EDNA", 3);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					topics.assign(startupScript.getFlagValue("SHERIFF_IN_DINER") ? 0xab : 0xaa);
				}
			}
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kEdnaAcAdTopicLineIndices,
					ARRAYSIZE(kEdnaAcAdTopicLineIndices))) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x3af6, "EDNA", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x3af8, "EDNA", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(startupScript.getFlagValue("SHERIFF_IN_DINER") ? 0xaf : 0xae);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0xb0)) {
			if (!startupScript.getFlagValue("SHERIFF_IN_DINER")) {
				Common::Error lineError = runtime.playDialogueLineWithVariant(0x3b03, "EDNA", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x3b07, "EDNA", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0xb1);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0xb2)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(kEdnaTopicB2Lines, ARRAYSIZE(kEdnaTopicB2Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kEdnaLodgeTopicLineIndices,
					ARRAYSIZE(kEdnaLodgeTopicLineIndices))) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = runtime.playDialogueEntrySequence(kEdnaLodgeTopicLines, ARRAYSIZE(kEdnaLodgeTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(startupScript.getFlagValue("SHERIFF_IN_DINER") ? 0xb9 : 0xb8);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0xba)) {
			if (!startupScript.getFlagValue("SHERIFF_IN_DINER")) {
				Common::Error lineError = runtime.playDialogueLineWithVariant(0x3b82, "EDNA", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0xbb);
			}
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kEdnaBcBdBeTopicLineIndices,
					ARRAYSIZE(kEdnaBcBdBeTopicLineIndices))) {
			if (!startupScript.getFlagValue("SHERIFF_IN_DINER")) {
				Common::Error lineError = runtime.playDialogueEntrySequence(kEdnaTopicBcBdBeLines,
					ARRAYSIZE(kEdnaTopicBcBdBeLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0xbf);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0xc0)) {
			if (startupScript.getFlagValue("KARIN_KIDNAPED")) {
				Common::Error lineError = runtime.playDialogueLineWithVariant(0x3b9e, "EDNA", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				sharedState.karinKidnapedDialogueState = true;

				int responseIndex = 0;
				Common::Error responseError = runtime.runResponseMenu(0xc1, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				lineError = runtime.playDialogueLineWithVariant(responseIndex == 1 ? 0x3bad : 0x3bb3, "EDNA", 3);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kDialogueC070FstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				return Common::kNoError;
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0xc2))
			continue;

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3bc9, "EDNA", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
