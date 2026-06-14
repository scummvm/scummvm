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

static const char *const kButcherNpc = "BUTCHER";
static const char *const kPcSpeaker = "PC";
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
	{ 0x1a5a, kButcherNpc, 0 },
	{ 0x1a5e, kPcSpeaker, 0 },
	{ 0x1a62, kButcherNpc, 1 }
};

static const DialogueLineEntry kButcherCasketPhotoLines[] = {
	{ 0x1a69, kButcherNpc, 0 },
	{ 0x1a6d, kPcSpeaker, 4 },
	{ 0x1a71, kButcherNpc, 1 },
	{ 0x1a76, kPcSpeaker, 0 },
	{ 0x1a7a, kButcherNpc, 0 }
};

static const DialogueLineEntry kButcherLedgerLines[] = {
	{ 0x1a80, kButcherNpc, 0 },
	{ 0x1a85, kPcSpeaker, 0 },
	{ 0x1a8a, kButcherNpc, 0 },
	{ 0x1a92, kButcherNpc, 0 },
	{ 0x1a96, kButcherNpc, 2 }
};

static const DialogueLineEntry kButcherOpeningQuestionLines[] = {
	{ 0x192a, kPcSpeaker, 0 },
	{ 0x192f, kButcherNpc, 0 },
	{ 0x1933, kPcSpeaker, 0 },
	{ 0x1938, kButcherNpc, 2 }
};

static const DialogueLineEntry kButcherLodgeTopicLines[] = {
	{ 0x1a07, kButcherNpc, 0 },
	{ 0x1a0d, kPcSpeaker, 0 },
	{ 0x1a11, kButcherNpc, 1 },
	{ 0x1a18, kPcSpeaker, 0 },
	{ 0x1a1c, kButcherNpc, 1 }
};

} // End of namespace

bool ButcherDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kButcherNpc);
}

Common::Error ButcherDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	ButcherRoomDialogueState &state = _state;
	Common::String &butcherTopicBuffer = state.currentTopicBuffer;
	int &butcherTopicBufferLineIndex = state.currentTopicBufferLineIndex;

	auto assignButcherTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(butcherTopicBuffer, butcherTopicBufferLineIndex,
			responseLineIndex, "Butcher topic buffer");
	};
	auto clearButcherTopicBuffer = [&]() {
		butcherTopicBuffer.clear();
		butcherTopicBufferLineIndex = -1;
	};
	auto playButcherLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kButcherNpc, headVariant);
	};
	auto playPcLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kPcSpeaker, headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto hasMeatPermission = [&]() {
		return runtime.startupScript().isObjectInInventory("MEAT_PERMISSION0") ||
			runtime.startupScript().isObjectInInventory("MEAT_PERMISSION");
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playSequence(kButcherWhaleyPhotoLines, ARRAYSIZE(kButcherWhaleyPhotoLines));
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfCorpse, true);
			return playSequence(kButcherCasketPhotoLines, ARRAYSIZE(kButcherCasketPhotoLines));
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownLedgersToAnyone, true);
			return playSequence(kButcherLedgerLines, ARRAYSIZE(kButcherLedgerLines));
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playButcherLine(0x1a9e);
		}
		if (usedItemName.equalsIgnoreCase("MEAT_PERMISSION")) {
			Common::Error lineError = playButcherLine(0x1ac4, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int ignoredResponseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x25f, ignoredResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			lineError = playButcherLine(0x1ace, 1);
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

		return playButcherLine(0x1a54);
	}

	if (state.introPending) {
		state.introPending = false;

		Common::Error lineError = playButcherLine(0x1902);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playPcLine(0x1906);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playButcherLine(0x190a);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playPcLine(sharedState.dialogueStateD2ed0 ? 0x1913 : 0x190f);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playButcherLine(0x1917, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playPcLine(0x191d);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playButcherLine(0x1921, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		assignButcherTopicBuffer(0x260);
	} else {
		Common::Error lineError = playButcherLine(0x1a47);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (!hasMeatPermission() && runtime.startupScript().getCurrentStoryDayIndex() <= 3)
			assignButcherTopicBuffer(0x261);
		else
			clearButcherTopicBuffer();
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;
		Common::Error lineError = playButcherLine(0x1aa7, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC123BFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DAY_5") && !state.dayFiveShown) {
		state.dayFiveShown = true;
		Common::Error lineError = playButcherLine(0x1ab6, 2);
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
			return playButcherLine(0x1a41);
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x263)) {
			Common::Error lineError = playSequence(
				kButcherOpeningQuestionLines, ARRAYSIZE(kButcherOpeningQuestionLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x264, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playButcherLine(0x1943, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kDialogueC119FstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playButcherLine(0x1950, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playPcLine(0x1955);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playButcherLine(0x195a);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				assignButcherTopicBuffer(hasMeatPermission() ? 0x266 : 0x265);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x267) && !hasMeatPermission()) {
			Common::Error lineError = playPcLine(0x1964);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playButcherLine(0x1969);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC060FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playButcherLine(0x1975);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC119AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playButcherLine(0x1980);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x268, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playButcherLine(0x198b);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				sharedState.dadMeatPermissionState = 1;
				(void)runtime.startupScript().addRuntimeObjectToInventory("MEAT_PERMISSION0");
				assignButcherTopicBuffer(0x269);
			} else if (responseIndex == 2) {
				lineError = playButcherLine(0x1992, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignButcherTopicBuffer(0x26a);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x26b)) {
			Common::Error lineError = playButcherLine(0x199c);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC119BFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playButcherLine(0x19a7);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playButcherLine(0x19ae);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playButcherLine(0x19b3, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC123FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playButcherLine(0x19bf, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			assignButcherTopicBuffer(hasMeatPermission() ? 0x26d : 0x26c);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x26e)) {
			Common::Error lineError = playButcherLine(0x19ca);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (sharedState.momFatherTopicState == 0) {
				lineError = playPcLine(0x19ce);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else {
				lineError = playPcLine(0x19d2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playButcherLine(0x19d6);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			if (sharedState.dialogueStateD2ed0) {
				lineError = playPcLine(0x19dc);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playButcherLine(0x19e1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playPcLine(0x19e5, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playButcherLine(0x19e9, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				assignButcherTopicBuffer(0x26f);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x270)) {
			Common::Error lineError = playButcherLine(0x19f6);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC123AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			assignButcherTopicBuffer(hasMeatPermission() ? 0x272 : 0x271);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x273) ||
				runtime.matchesResponseLine(selectedTopic, 0x274)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playSequence(
				kButcherLodgeTopicLines, ARRAYSIZE(kButcherLodgeTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			assignButcherTopicBuffer(hasMeatPermission() ? 0x276 : 0x275);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x277)) {
			Common::Error lineError = playPcLine(0x1a24, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return playButcherLine(0x1a29, 1);
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x279))
			continue;

		Common::Error lineError = playButcherLine(0x1a4e);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
