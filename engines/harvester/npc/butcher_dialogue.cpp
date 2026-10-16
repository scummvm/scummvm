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


#include "harvester/npc/butcher_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC060FstPath = "GRAPHIC/FST/C060.FST";
static const char *const kDialogueC060AFstPath = "GRAPHIC/FST/C060A.FST";
static const char *const kDialogueC119FstPath = "GRAPHIC/FST/C119.FST";
static const char *const kDialogueC119AFstPath = "GRAPHIC/FST/C119A.FST";
static const char *const kDialogueC119BFstPath = "GRAPHIC/FST/C119B.FST";
static const char *const kDialogueC119CFstPath = "GRAPHIC/FST/C119C.FST";
static const char *const kDialogueC123FstPath = "GRAPHIC/FST/C123.FST";
static const char *const kDialogueC123AFstPath = "GRAPHIC/FST/C123A.FST";
static const char *const kDialogueC123BFstPath = "GRAPHIC/FST/C123B.FST";

static const DialogueLineEntry kButcherWhaleyPhotoLines[] = {
	{ 0x1a5a, "BUTCHER", 0 },
	{ 0x1a5e, "PC", 0 },
	{ 0x1a62, "BUTCHER", 1 }
};

static const DialogueLineEntry kButcherCasketPhotoLines[] = {
	{ 0x1a69, "BUTCHER", 0 },
	{ 0x1a6d, "PC", 4 },
	{ 0x1a71, "BUTCHER", 1 },
	{ 0x1a76, "PC", 0 },
	{ 0x1a7a, "BUTCHER", 0 }
};

static const DialogueLineEntry kButcherLedgerLines[] = {
	{ 0x1a80, "BUTCHER", 0 },
	{ 0x1a85, "PC", 0 },
	{ 0x1a8a, "BUTCHER", 0 },
	{ 0x1a92, "BUTCHER", 0 },
	{ 0x1a96, "BUTCHER", 2 }
};

static const DialogueLineEntry kButcherOpeningQuestionLines[] = {
	{ 0x192a, "PC", 0 },
	{ 0x192f, "BUTCHER", 0 },
	{ 0x1933, "PC", 0 },
	{ 0x1938, "BUTCHER", 2 }
};

static const DialogueLineEntry kButcherLodgeTopicLines[] = {
	{ 0x1a07, "BUTCHER", 0 },
	{ 0x1a0d, "PC", 0 },
	{ 0x1a11, "BUTCHER", 1 },
	{ 0x1a18, "PC", 0 },
	{ 0x1a1c, "BUTCHER", 1 }
};

} // End of namespace

Common::Error ButcherDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	ButcherRoomDialogueState &state = _state;
	Common::String &butcherTopicBuffer = state.currentTopicBuffer;
	int &butcherTopicBufferLineIndex = state.currentTopicBufferLineIndex;
	state.currentTopicBuffer.clear();
	state.currentTopicBufferLineIndex = -1;

	DialogueTopicBuffer topics(runtime, butcherTopicBuffer, butcherTopicBufferLineIndex, "Butcher topic buffer");

	auto hasMeatPermission = [&]() {
		return runtime.startupScript().isObjectInInventory("MEAT_PERMISSION0") ||
			runtime.startupScript().isObjectInInventory("MEAT_PERMISSION");
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return runtime.playDialogueEntrySequence(kButcherWhaleyPhotoLines, ARRAYSIZE(kButcherWhaleyPhotoLines));
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfCorpse, true);
			return runtime.playDialogueEntrySequence(kButcherCasketPhotoLines, ARRAYSIZE(kButcherCasketPhotoLines));
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			sharedState.discussedLedgerEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownLedgersToAnyone, true);
			return runtime.playDialogueEntrySequence(kButcherLedgerLines, ARRAYSIZE(kButcherLedgerLines));
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			return runtime.playDialogueLineWithVariant(0x1a9e, "BUTCHER", 0);
		}
		if (usedItemName.equalsIgnoreCase("MEAT_PERMISSION")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x1ac4, "BUTCHER", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int ignoredResponseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x25f, ignoredResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			lineError = runtime.playDialogueLineWithVariant(0x1ace, "BUTCHER", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC060AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			(void)runtime.startupScript().addRuntimeObjectToInventory("MEAT");
			(void)runtime.startupScript().setRuntimeFlagValue("GOT_MEAT", true);
			sharedState.dadMeatPermissionState = 0;
			return Common::kNoError;
		}

		return runtime.playDialogueLineWithVariant(0x1a54, "BUTCHER", 0);
	}

	if (state.introPending) {
		state.introPending = false;

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x1902, "BUTCHER", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x1906, "PC", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x190a, "BUTCHER", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(sharedState.dialogueStateD2ed0 ? 0x1913 : 0x190f, "PC", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x1917, "BUTCHER", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x191d, "PC", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x1921, "BUTCHER", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		topics.assign(0x260);
	} else {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x1a47, "BUTCHER", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (!hasMeatPermission() && runtime.startupScript().getCurrentStoryDayIndex() <= 3)
			topics.assign(0x261);
		else
			topics.clear();
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x1aa7, "BUTCHER", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC123BFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DAY_5") && !state.dayFiveShown) {
		state.dayFiveShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x1ab6, "BUTCHER", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC119CFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			butcherTopicBuffer, butcherTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()) ||
				runtime.matchesResponseLine(selectedTopic, 0x262)) {
			return runtime.playDialogueLineWithVariant(0x1a41, "BUTCHER", 0);
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x263)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(kButcherOpeningQuestionLines,
				ARRAYSIZE(kButcherOpeningQuestionLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x264, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x1943, "BUTCHER", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kDialogueC119FstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x1950, "BUTCHER", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x1955, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x195a, "BUTCHER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				topics.assign(hasMeatPermission() ? 0x266 : 0x265);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x267) && !hasMeatPermission()) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x1964, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x1969, "BUTCHER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC060FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x1975, "BUTCHER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC119AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x1980, "BUTCHER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x268, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x198b, "BUTCHER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				sharedState.dadMeatPermissionState = 1;
				(void)runtime.startupScript().addRuntimeObjectToInventory("MEAT_PERMISSION0");
				topics.assign(0x269);
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x1992, "BUTCHER", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0x26a);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x26b)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x199c, "BUTCHER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC119BFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x19a7, "BUTCHER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x19ae, "BUTCHER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x19b3, "BUTCHER", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC123FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x19bf, "BUTCHER", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(hasMeatPermission() ? 0x26d : 0x26c);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x26e)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x19ca, "BUTCHER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (sharedState.momFatherTopicState == 0) {
				lineError = runtime.playDialogueLineWithVariant(0x19ce, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else {
				lineError = runtime.playDialogueLineWithVariant(0x19d2, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x19d6, "BUTCHER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			if (sharedState.dialogueStateD2ed0) {
				lineError = runtime.playDialogueLineWithVariant(0x19dc, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x19e1, "BUTCHER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x19e5, "PC", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x19e9, "BUTCHER", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				topics.assign(0x26f);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x270)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x19f6, "BUTCHER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC123AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(hasMeatPermission() ? 0x272 : 0x271);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x273) ||
				runtime.matchesResponseLine(selectedTopic, 0x274)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = runtime.playDialogueEntrySequence(kButcherLodgeTopicLines,
				ARRAYSIZE(kButcherLodgeTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(hasMeatPermission() ? 0x276 : 0x275);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x277)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x1a24, "PC", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x1a29, "BUTCHER", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (!sharedState.dialogueStateD2f04) {
				sharedState.dialogueStateD2f04 = true;
				lineError = runtime.playDialogueLineWithVariant(0x1a2d, "PC", 0);
			} else {
				lineError = runtime.playDialogueLineWithVariant(0x1a31, "PC", 0);
				if (lineError.getCode() == Common::kNoError)
					lineError = runtime.playDialogueLineWithVariant(0x1a36, "BUTCHER", 2);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x278);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x279))
			continue;

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x1a4e, "BUTCHER", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
