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


#include "harvester/npc/boyle_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kBoylesButtonObject = "BOYLES_BUTTON";
static const char *const kGascanObject = "GASCAN";
static const char *const kLodgeApplicationObject = "LODGE_APPLICATION";
static const char *const kHaveLodgeAppFlag = "HAVE_LODGE_APP";
static const char *const kSheriffInDinerFlag = "SHERIFF_IN_DINER";
static const char *const kBoyleArsonistTopic = "Arsonist";
static const char *const kDialogueC019BFstPath = "GRAPHIC/FST/C019B.FST";

static const DialogueLineEntry kBoyleHaveLodgeAppEvidenceLines[] = {
	{ 0x13d, "BOYLE", 2 },
	{ 0x141, "PC", 0 },
	{ 0x145, "BOYLE", 2 },
	{ 0x14a, "PC", 0 },
	{ 0x14e, "BOYLE", 3 }
};

static const DialogueLineEntry kBoyleButtonIntroLines[] = {
	{ 0x155, "BOYLE", 1 },
	{ 0x15a, "PC", 0 },
	{ 0x15f, "BOYLE", 2 }
};

static const DialogueLineEntry kBoyleButtonResponse2Lines[] = {
	{ 0x174, "BOYLE", 0 },
	{ 0x179, "PC", 0 },
	{ 0x17d, "BOYLE", 0 }
};

static const DialogueLineEntry kBoyleNoteResponse1IntroLines[] = {
	{ 0x1a1, "BOYLE", 2 },
	{ 0x1a9, "PC", 0 },
	{ 0x1ad, "BOYLE", 0 }
};

static const DialogueLineEntry kBoyleGascanRewardLines[] = {
	{ 0x1f2, "BOYLE", 3 },
	{ 0x1f6, "PC", 2 },
	{ 0x1fa, "BOYLE", 3 },
	{ 0x202, "PC", 2 },
	{ 0x206, "BOYLE", 2 }
};

static const DialogueLineEntry kBoyleIntroOpeningLines[] = {
	{ 0x7, "BOYLE", 0 },
	{ 0xb, "PC", 0 },
	{ 0xf, "BOYLE", 1 }
};

static const DialogueLineEntry kBoyleIntroResponse1Lines[] = {
	{ 0x19, "BOYLE", 0 },
	{ 0x1d, "PC", 2 },
	{ 0x21, "BOYLE", 1 }
};

static const DialogueLineEntry kBoyleIntroResponse1TailLines[] = {
	{ 0x3a, "BOYLE", 0 },
	{ 0x41, "PC", 2 },
	{ 0x45, "BOYLE", 0 }
};

static const DialogueLineEntry kBoyleIntroResponse2Choice2Lines[] = {
	{ 0x76, "BOYLE", 0 },
	{ 0x77, "BOYLE", 2 }
};

static const DialogueLineEntry kBoyleKeywordTopic0eLines[] = {
	{ 0x8c, "PC", 0 },
	{ 0x90, "BOYLE", 0 },
	{ 0x94, "PC", 0 },
	{ 0x98, "BOYLE", 0 },
	{ 0x9d, "PC", 0 },
	{ 0xa1, "BOYLE", 1 }
};

static const DialogueLineEntry kBoyleKeywordTopic17Lines[] = {
	{ 0xee, "PC", 0 },
	{ 0xf2, "BOYLE", 2 },
	{ 0xf6, "PC", 4 },
	{ 0xfa, "BOYLE", 0 }
};

static const DialogueLineEntry kBoyleKeywordArsonResponse1Lines[] = {
	{ 0x126, "BOYLE", 0 },
	{ 0x12c, "PC", 0 },
	{ 0x130, "BOYLE", 2 }
};

static const DialogueLineEntry kBoyleFollowupBurnedTvLines[] = {
	{ 0x28a, "BOYLE", 1 },
	{ 0x28e, "PC", 0 },
	{ 0x292, "BOYLE", 1 }
};

static const int kBoyleKeywordTopic0fTo12Lines[] = { 0xf, 0x10, 0x11, 0x12 };
static const int kBoyleKeywordTopic13To14Lines[] = { 0x13, 0x14 };
static const int kBoyleKeywordTopic15To16Lines[] = { 0x15, 0x16 };
static const int kBoyleKeywordTopic18To1bLines[] = { 0x18, 0x19, 0x1a, 0x1b };

} // End of namespace

Common::Error BoyleDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	BoyleRoomDialogueState &state = _state;
	Common::String boyleTopicBuffer;
	int boyleTopicBufferLineIndex = -1;

	DialogueTopicBuffer topics(runtime, boyleTopicBuffer, boyleTopicBufferLineIndex, "Boyle topic buffer");

	auto hideInventoryItem = [&](const char *objectName) {
		return runtime.startupScript().setRuntimeObjectVisible(
			kInventoryOwnerName, objectName, false);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<ObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const ObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}
		return false;
	};
	auto isBlackmailEvidence = [&](const Common::String &itemName) {
		return itemName.equalsIgnoreCase("NOTE") ||
			itemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			itemName.equalsIgnoreCase("CHECKBOOK") ||
			itemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY");
	};

	Common::Error lineError = Common::kNoError;
	const bool hasBoylesButton = hasInventoryItem(kBoylesButtonObject);

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase(kBoylesButtonObject)) {
			lineError = runtime.playDialogueEntrySequence(kBoyleButtonIntroLines, ARRAYSIZE(kBoyleButtonIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x16d, "BOYLE", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				return runtime.playDialogueLineWithVariant(0x100, "BOYLE", 1);
			}
			if (responseIndex == 2) {
				lineError = runtime.playDialogueEntrySequence(kBoyleButtonResponse2Lines, ARRAYSIZE(kBoyleButtonResponse2Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				if (state.extraEvidenceReplyEnabled) {
					lineError = runtime.playDialogueLineWithVariant(0x186, "PC", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = runtime.playDialogueLineWithVariant(0x18b, "BOYLE", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				} else {
					lineError = runtime.playDialogueLineWithVariant(0x1d2, "BOYLE", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}

				(void)hideInventoryItem(kBoylesButtonObject);
				sharedState.boyleGascanApplicationState = true;
				sharedState.dialogueStateD2e98 = true;
				sharedState.dialogueStateD2eb0 = true;
				(void)runtime.startupScript().setRuntimeFlagValue(kSheriffInDinerFlag, true);
			}
			return Common::kNoError;
		}

		if (isBlackmailEvidence(usedItemName)) {
			if (runtime.startupScript().getFlagValue(kHaveLodgeAppFlag)) {
				return runtime.playDialogueEntrySequence(kBoyleHaveLodgeAppEvidenceLines,
					ARRAYSIZE(kBoyleHaveLodgeAppEvidenceLines));
			}

			lineError = runtime.playDialogueLineWithVariant(0x197, "BOYLE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x3, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				sharedState.boyleGascanApplicationState = true;
				lineError = runtime.playDialogueEntrySequence(kBoyleNoteResponse1IntroLines,
					ARRAYSIZE(kBoyleNoteResponse1IntroLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				if (state.extraEvidenceReplyEnabled) {
					lineError = runtime.playDialogueLineWithVariant(0x186, "PC", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = runtime.playDialogueLineWithVariant(0x1b8, "BOYLE", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
				return runtime.playDialogueLineWithVariant(0x1d2, "BOYLE", 0);
			}
			if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x1d9, "BOYLE", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int followupResponseIndex = 0;
				responseError = runtime.runResponseMenu(0x4, followupResponseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (followupResponseIndex == 1)
					return runtime.playDialogueLineWithVariant(0x1e5, "BOYLE", 1);
				if (followupResponseIndex == 2)
					return runtime.playDialogueLineWithVariant(0x1ea, "BOYLE", 2);
			}
			return Common::kNoError;
		}

		if (usedItemName.equalsIgnoreCase(kGascanObject) &&
				sharedState.boyleGascanApplicationState) {
			lineError = runtime.playDialogueEntrySequence(kBoyleGascanRewardLines, ARRAYSIZE(kBoyleGascanRewardLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			(void)hideInventoryItem(kGascanObject);
			(void)runtime.startupScript().addRuntimeObjectToInventory(kLodgeApplicationObject);
			(void)runtime.startupScript().setRuntimeFlagValue(kHaveLodgeAppFlag, true);
			return Common::kNoError;
		}

		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			return runtime.playDialogueLineWithVariant(0x25f, "BOYLE", 2);
		}

		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			lineError = runtime.playDialogueLineWithVariant(0x24f, "BOYLE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dialogueStateD2ebc) {
				lineError = runtime.playDialogueLineWithVariant(0x253, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			return runtime.playDialogueLineWithVariant(0x258, "BOYLE", 2);
		}

		return runtime.playDialogueLineWithVariant(0x249, "BOYLE", 0);
	}

	if (state.introPending) {
		state.introPending = false;

		lineError = runtime.playDialogueEntrySequence(kBoyleIntroOpeningLines, ARRAYSIZE(kBoyleIntroOpeningLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x5, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		bool seedButtonTopicBuffer = false;
		if (responseIndex == 1) {
			lineError = runtime.playDialogueEntrySequence(kBoyleIntroResponse1Lines, ARRAYSIZE(kBoyleIntroResponse1Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int followupResponseIndex = 0;
			responseError = runtime.runResponseMenu(0x6, followupResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (followupResponseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x2f, "BOYLE", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (followupResponseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x34, "BOYLE", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = runtime.playDialogueEntrySequence(kBoyleIntroResponse1TailLines,
				ARRAYSIZE(kBoyleIntroResponse1TailLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			seedButtonTopicBuffer = true;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x4d, "BOYLE", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int followupResponseIndex = 0;
			responseError = runtime.runResponseMenu(0x7, followupResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (followupResponseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x59, "BOYLE", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int nestedResponseIndex = 0;
				responseError = runtime.runResponseMenu(0x8, nestedResponseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (nestedResponseIndex == 1) {
					lineError = runtime.playDialogueLineWithVariant(0x68, "BOYLE", 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				} else if (nestedResponseIndex == 2) {
					lineError = runtime.playDialogueLineWithVariant(0x6c, "BOYLE", 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			} else if (followupResponseIndex == 2) {
				state.extraEvidenceReplyEnabled = true;
				lineError = runtime.playDialogueEntrySequence(kBoyleIntroResponse2Choice2Lines,
					ARRAYSIZE(kBoyleIntroResponse2Choice2Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = runtime.playDialogueLineWithVariant(0x80, "BOYLE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x9);
			seedButtonTopicBuffer = true;
		}

		if (seedButtonTopicBuffer && hasBoylesButton)
			topics.assign(0xa);
	} else {
		lineError = runtime.playDialogueLineWithVariant(0x23c, "BOYLE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (hasBoylesButton)
			topics.assign(0xb);
	}

	if ((sharedState.discussedNoteCheckbookEvidence != 0 ||
				sharedState.dwayneDiscussedBoylesButton) &&
			!state.evidenceFollowupShown) {
		state.evidenceFollowupShown = true;
		lineError = runtime.playDialogueLineWithVariant(0x210, "BOYLE", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.discussedNoteCheckbookEvidence != 0) {
			lineError = runtime.playDialogueLineWithVariant(0x21a, "BOYLE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (sharedState.dwayneDiscussedBoylesButton) {
			lineError = runtime.playDialogueLineWithVariant(0x215, "BOYLE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (!runtime.startupScript().getFlagValue(kHaveLodgeAppFlag)) {
			lineError = runtime.playDialogueLineWithVariant(0x21f, "BOYLE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0xc, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x22a, "BOYLE", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x22e, "BOYLE", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		} else {
			lineError = runtime.playDialogueLineWithVariant(0x233, "BOYLE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		topics.clear();
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;
		lineError = runtime.playDialogueLineWithVariant(0x267, "BOYLE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("DAY_5") && !state.dayFiveShown) {
		state.dayFiveShown = true;
		lineError = runtime.playDialogueLineWithVariant(0x26e, "BOYLE", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		lineError = runtime.playDialogueEntrySequence(kBoyleFollowupBurnedTvLines, ARRAYSIZE(kBoyleFollowupBurnedTvLines));
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
			return runtime.playDialogueLineWithVariant(0x100, "BOYLE", 1);
		}

		if (runtime.matchesResponseLine(selectedTopic, 0xe)) {
			lineError = runtime.playDialogueEntrySequence(kBoyleKeywordTopic0eLines, ARRAYSIZE(kBoyleKeywordTopic0eLines));
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic0fTo12Lines,
				ARRAYSIZE(kBoyleKeywordTopic0fTo12Lines))) {
			lineError = runtime.playDialogueLineWithVariant(0xab, "BOYLE", 0);
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic13To14Lines,
				ARRAYSIZE(kBoyleKeywordTopic13To14Lines))) {
			lineError = runtime.playDialogueLineWithVariant(0xb6, "BOYLE", 3);
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic15To16Lines,
				ARRAYSIZE(kBoyleKeywordTopic15To16Lines))) {
			lineError = runtime.startupScript().getCurrentStoryDayIndex() >= 3
				? runtime.playDialogueLineWithVariant(0xc6, "BOYLE", 0)
				: runtime.playDialogueLineWithVariant(0x243, "BOYLE", 0);
		} else if (runtime.matchesResponseLine(selectedTopic, 0x17)) {
			lineError = runtime.playDialogueEntrySequence(kBoyleKeywordTopic17Lines, ARRAYSIZE(kBoyleKeywordTopic17Lines));
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic18To1bLines,
				ARRAYSIZE(kBoyleKeywordTopic18To1bLines))) {
			if (!sharedState.dialogueStateD2eb0) {
				lineError = runtime.playDialogueFst(kDialogueC019BFstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x10e, "BOYLE", 0);
			} else {
				lineError = runtime.playDialogueLineWithVariant(0x115, "BOYLE", 1);
			}
		} else if (runtime.matchesResponseLine(selectedTopic, 0x1d) ||
				selectedTopic.equalsIgnoreCase(kBoyleArsonistTopic)) {
			lineError = runtime.playDialogueLineWithVariant(0x11c, "BOYLE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x1e, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueEntrySequence(kBoyleKeywordArsonResponse1Lines,
					ARRAYSIZE(kBoyleKeywordArsonResponse1Lines));
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x135, "BOYLE", 0);
			} else {
				lineError = Common::kNoError;
			}
		} else if (runtime.matchesResponseLine(selectedTopic, 0x1f)) {
			lineError = runtime.playDialogueEntrySequence(kBoyleHaveLodgeAppEvidenceLines,
				ARRAYSIZE(kBoyleHaveLodgeAppEvidenceLines));
		} else {
			lineError = runtime.playDialogueLineWithVariant(0x243, "BOYLE", 0);
		}

		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
