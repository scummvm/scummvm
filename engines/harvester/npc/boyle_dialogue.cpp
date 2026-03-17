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

#include <functional>

#include "harvester/npc/boyle_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kBoyleArsonistTopic = "Arsonist";
static const char *const kDialogueC019BFstPath = "GRAPHIC/FST/C019B.FST";

static const DialogueLineEntry kBoyleHaveLodgeAppEvidenceLines[] = {
	{ 0x13d, "BOYLE", 2 },
	{ 0x141, "BOYLE", 0 },
	{ 0x145, "BOYLE", 2 },
	{ 0x14a, "BOYLE", 0 },
	{ 0x14e, "BOYLE", 3 }
};

static const DialogueLineEntry kBoyleButtonResponse2Lines[] = {
	{ 0x174, "BOYLE", 0 },
	{ 0x179, "BOYLE", 0 },
	{ 0x17d, "BOYLE", 0 }
};

static const DialogueLineEntry kBoyleNoteResponse1Lines[] = {
	{ 0x1a1, "BOYLE", 2 },
	{ 0x1a9, "BOYLE", 0 },
	{ 0x1ad, "BOYLE", 0 },
	{ 0x186, "BOYLE", 0 },
	{ 0x1b8, "BOYLE", 0 },
	{ 0x1d2, "BOYLE", 0 }
};

static const DialogueLineEntry kBoyleGascanRewardLines[] = {
	{ 0x1f2, "BOYLE", 3 },
	{ 0x1f6, "BOYLE", 2 },
	{ 0x1fa, "BOYLE", 3 },
	{ 0x202, "BOYLE", 2 },
	{ 0x206, "BOYLE", 2 }
};

static const DialogueLineEntry kBoyleKeywordTopic0eLines[] = {
	{ 0x8c, "BOYLE", 0 },
	{ 0x90, "BOYLE", 0 },
	{ 0x94, "BOYLE", 0 },
	{ 0x98, "BOYLE", 0 },
	{ 0x9d, "BOYLE", 0 },
	{ 0xa1, "BOYLE", 1 }
};

static const DialogueLineEntry kBoyleKeywordTopic17Lines[] = {
	{ 0xee, "BOYLE", 0 },
	{ 0xf2, "BOYLE", 2 },
	{ 0xf6, "BOYLE", 4 },
	{ 0xfa, "BOYLE", 0 }
};

static const DialogueLineEntry kBoyleKeywordArsonResponse1Lines[] = {
	{ 0x126, "BOYLE", 0 },
	{ 0x12c, "BOYLE", 0 },
	{ 0x130, "BOYLE", 2 }
};

static const DialogueLineEntry kBoyleFollowupBurnedTvLines[] = {
	{ 0x28a, "BOYLE", 1 },
	{ 0x28e, "BOYLE", 0 },
	{ 0x292, "BOYLE", 1 }
};

static const int kBoyleKeywordTopic0fTo12Lines[] = { 0xf, 0x10, 0x11, 0x12 };
static const int kBoyleKeywordTopic13To14Lines[] = { 0x13, 0x14 };
static const int kBoyleKeywordTopic15To16Lines[] = { 0x15, 0x16 };
static const int kBoyleKeywordTopic18To1bLines[] = { 0x18, 0x19, 0x1a, 0x1b };

} // End of namespace

bool BoyleDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("BOYLE");
}

Common::Error BoyleDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	BoyleRoomDialogueState &state = _state;
	Common::String boyleTopicBuffer;
	int boyleTopicBufferLineIndex = -1;

	auto assignBoyleTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(boyleTopicBuffer, boyleTopicBufferLineIndex,
			responseLineIndex, "Boyle topic buffer");
	};
	auto clearBoyleTopicBuffer = [&]() {
		boyleTopicBuffer.clear();
		boyleTopicBufferLineIndex = -1;
	};
	auto playBoyleLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "BOYLE", headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<StartupObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const StartupObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}
		return false;
	};

	Common::Error lineError = Common::kNoError;
	const bool hasBoylesButton = hasInventoryItem("BOYLES_BUTTON");

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("BOYLES_BUTTON")) {
			lineError = playBoyleLine(0x155, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playBoyleLine(0x15a, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playBoyleLine(0x15f, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playBoyleLine(0x16d, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				return playBoyleLine(0x100, 1);
			}
			if (responseIndex == 2) {
				lineError = playSequence(kBoyleButtonResponse2Lines,
					ARRAYSIZE(kBoyleButtonResponse2Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				lineError = sharedState.boyleGascanApplicationState
					? playBoyleLine(0x186, 0)
					: playBoyleLine(0x1d2, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				if (sharedState.boyleGascanApplicationState) {
					lineError = playBoyleLine(0x18b, 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}

				(void)runtime.startupScript().resetRuntimeObjectToInitialState("BOYLES_BUTTON");
				sharedState.boyleGascanApplicationState = true;
				sharedState.dialogueStateD2e98 = true;
				sharedState.dialogueStateD2eb0 = true;
				(void)runtime.startupScript().setRuntimeFlagValue("SHERIFF_IN_DINER", true);
			}
			return Common::kNoError;
		}

		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			if (runtime.startupScript().getFlagValue("HAVE_LODGE_APP"))
				return playSequence(kBoyleHaveLodgeAppEvidenceLines,
					ARRAYSIZE(kBoyleHaveLodgeAppEvidenceLines));

			lineError = playBoyleLine(0x197, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x3, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				sharedState.boyleGascanApplicationState = true;
				return playSequence(kBoyleNoteResponse1Lines,
					ARRAYSIZE(kBoyleNoteResponse1Lines));
			}
			if (responseIndex == 2) {
				lineError = playBoyleLine(0x1d9, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int followupResponseIndex = 0;
				responseError = runtime.runResponseMenu(0x4, followupResponseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (followupResponseIndex == 1)
					return playBoyleLine(0x1e5, 1);
				if (followupResponseIndex == 2)
					return playBoyleLine(0x1ea, 2);
			}
			return Common::kNoError;
		}

		if (usedItemName.equalsIgnoreCase("GASCAN") &&
				sharedState.boyleGascanApplicationState) {
			lineError = playSequence(kBoyleGascanRewardLines,
				ARRAYSIZE(kBoyleGascanRewardLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			sharedState.boyleGascanApplicationState = false;
			(void)runtime.startupScript().resetRuntimeObjectToInitialState("GASCAN");
			(void)runtime.startupScript().setRuntimeFlagValue("SHERIFF_IN_DINER", true);
			(void)runtime.startupScript().addRuntimeObjectToInventory("LODGE_APPLICATION");
			(void)runtime.startupScript().setRuntimeFlagValue("HAVE_LODGE_APP", true);
			return Common::kNoError;
		}

		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playBoyleLine(0x25f, 2);
		}

		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfCorpse, true);
			lineError = playBoyleLine(0x24f, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dialogueStateD2ebc) {
				lineError = playBoyleLine(0x253, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			return playBoyleLine(0x258, 2);
		}

		return playBoyleLine(0x249, 0);
	}

	if (state.introPending) {
		state.introPending = false;

		lineError = playBoyleLine(0x7, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playBoyleLine(0xb, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playBoyleLine(0xf, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x5, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playBoyleLine(0x19, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playBoyleLine(0x1d, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playBoyleLine(0x21, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int followupResponseIndex = 0;
			responseError = runtime.runResponseMenu(0x6, followupResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (followupResponseIndex == 1) {
				lineError = playBoyleLine(0x2f, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (followupResponseIndex == 2) {
				lineError = playBoyleLine(0x34, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = playBoyleLine(0x3a, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playBoyleLine(0x41, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playBoyleLine(0x45, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playBoyleLine(0x4d, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int followupResponseIndex = 0;
			responseError = runtime.runResponseMenu(0x7, followupResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (followupResponseIndex == 1) {
				lineError = playBoyleLine(0x59, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int nestedResponseIndex = 0;
				responseError = runtime.runResponseMenu(0x8, nestedResponseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (nestedResponseIndex == 1) {
					lineError = playBoyleLine(0x68, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				} else if (nestedResponseIndex == 2) {
					lineError = playBoyleLine(0x6c, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			} else if (followupResponseIndex == 2) {
				sharedState.dialogueStateD2ebc = true;
				lineError = playBoyleLine(0x76, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playBoyleLine(0x77, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = playBoyleLine(0x80, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignBoyleTopicBuffer(0x9);
		}

		if (hasBoylesButton)
			assignBoyleTopicBuffer(0xa);
	} else {
		lineError = playBoyleLine(0x23c, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (hasBoylesButton)
			assignBoyleTopicBuffer(0xb);
	}

	if ((runtime.startupScript().getFlagValue(DialogueFlags::kShownEvidenceOfBlackmail) ||
				sharedState.dwayneDiscussedBoylesButton) &&
			!state.evidenceFollowupShown) {
		state.evidenceFollowupShown = true;
		lineError = playBoyleLine(0x210, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.startupScript().getFlagValue(DialogueFlags::kShownEvidenceOfBlackmail)) {
			lineError = playBoyleLine(0x21a, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (sharedState.dwayneDiscussedBoylesButton) {
			lineError = playBoyleLine(0x215, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (!runtime.startupScript().getFlagValue("HAVE_LODGE_APP")) {
			lineError = playBoyleLine(0x21f, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0xc, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playBoyleLine(0x22a, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playBoyleLine(0x22e, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		} else {
			lineError = playBoyleLine(0x233, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		clearBoyleTopicBuffer();
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;
		lineError = playBoyleLine(0x267, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("DAY_5") && !state.dayFiveShown) {
		state.dayFiveShown = true;
		lineError = playBoyleLine(0x26e, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		lineError = playSequence(kBoyleFollowupBurnedTvLines,
			ARRAYSIZE(kBoyleFollowupBurnedTvLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	Common::String selectedTopic;
	for (;;) {
		Common::Error menuError = runtime.runKeywordMenu(
			boyleTopicBuffer, boyleTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;

		if (runtime.matchesResponseLine(selectedTopic, 0xd) ||
				runtime.matchesResponseLine(selectedTopic, 0x20)) {
			return playBoyleLine(0x100, 1);
		}

		if (runtime.matchesResponseLine(selectedTopic, 0xe)) {
			lineError = playSequence(kBoyleKeywordTopic0eLines,
				ARRAYSIZE(kBoyleKeywordTopic0eLines));
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic0fTo12Lines,
				ARRAYSIZE(kBoyleKeywordTopic0fTo12Lines))) {
			lineError = playBoyleLine(0xab, 0);
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic13To14Lines,
				ARRAYSIZE(kBoyleKeywordTopic13To14Lines))) {
			lineError = playBoyleLine(0xb6, 3);
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic15To16Lines,
				ARRAYSIZE(kBoyleKeywordTopic15To16Lines))) {
			lineError = runtime.startupScript().getCurrentStoryDayIndex() >= 3
				? playBoyleLine(0xc6, 0)
				: playBoyleLine(0x243, 0);
		} else if (runtime.matchesResponseLine(selectedTopic, 0x17)) {
			lineError = playSequence(kBoyleKeywordTopic17Lines,
				ARRAYSIZE(kBoyleKeywordTopic17Lines));
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic18To1bLines,
				ARRAYSIZE(kBoyleKeywordTopic18To1bLines))) {
			if (!sharedState.dialogueStateD2eb0) {
				lineError = runtime.playDialogueFst(kDialogueC019BFstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playBoyleLine(0x10e, 0);
			} else {
				lineError = playBoyleLine(0x115, 1);
			}
		} else if (runtime.matchesResponseLine(selectedTopic, 0x1d) ||
				selectedTopic.equalsIgnoreCase(kBoyleArsonistTopic)) {
			lineError = playBoyleLine(0x11c, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x1e, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playSequence(kBoyleKeywordArsonResponse1Lines,
					ARRAYSIZE(kBoyleKeywordArsonResponse1Lines));
			} else if (responseIndex == 2) {
				lineError = playBoyleLine(0x135, 0);
			} else {
				lineError = Common::kNoError;
			}
		} else if (runtime.matchesResponseLine(selectedTopic, 0x1f)) {
			lineError = playSequence(kBoyleHaveLodgeAppEvidenceLines,
				ARRAYSIZE(kBoyleHaveLodgeAppEvidenceLines));
		} else {
			lineError = playBoyleLine(0x243, 0);
		}

		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
