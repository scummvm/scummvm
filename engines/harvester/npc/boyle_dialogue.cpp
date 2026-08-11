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

static const char *const kBoyleNpc = "BOYLE";
static const char *const kPcSpeaker = "PC";
static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kBoylesButtonObject = "BOYLES_BUTTON";
static const char *const kGascanObject = "GASCAN";
static const char *const kLodgeApplicationObject = "LODGE_APPLICATION";
static const char *const kHaveLodgeAppFlag = "HAVE_LODGE_APP";
static const char *const kSheriffInDinerFlag = "SHERIFF_IN_DINER";
static const char *const kBoyleArsonistTopic = "Arsonist";
static const char *const kDialogueC019BFstPath = "GRAPHIC/FST/C019B.FST";

static const DialogueLineEntry kBoyleHaveLodgeAppEvidenceLines[] = {
	{ 0x13d, kBoyleNpc, 2 },
	{ 0x141, kPcSpeaker, 0 },
	{ 0x145, kBoyleNpc, 2 },
	{ 0x14a, kPcSpeaker, 0 },
	{ 0x14e, kBoyleNpc, 3 }
};

static const DialogueLineEntry kBoyleButtonIntroLines[] = {
	{ 0x155, kBoyleNpc, 1 },
	{ 0x15a, kPcSpeaker, 0 },
	{ 0x15f, kBoyleNpc, 2 }
};

static const DialogueLineEntry kBoyleButtonResponse2Lines[] = {
	{ 0x174, kBoyleNpc, 0 },
	{ 0x179, kPcSpeaker, 0 },
	{ 0x17d, kBoyleNpc, 0 }
};

static const DialogueLineEntry kBoyleNoteResponse1IntroLines[] = {
	{ 0x1a1, kBoyleNpc, 2 },
	{ 0x1a9, kPcSpeaker, 0 },
	{ 0x1ad, kBoyleNpc, 0 }
};

static const DialogueLineEntry kBoyleGascanRewardLines[] = {
	{ 0x1f2, kBoyleNpc, 3 },
	{ 0x1f6, kPcSpeaker, 2 },
	{ 0x1fa, kBoyleNpc, 3 },
	{ 0x202, kPcSpeaker, 2 },
	{ 0x206, kBoyleNpc, 2 }
};

static const DialogueLineEntry kBoyleIntroOpeningLines[] = {
	{ 0x7, kBoyleNpc, 0 },
	{ 0xb, kPcSpeaker, 0 },
	{ 0xf, kBoyleNpc, 1 }
};

static const DialogueLineEntry kBoyleIntroResponse1Lines[] = {
	{ 0x19, kBoyleNpc, 0 },
	{ 0x1d, kPcSpeaker, 2 },
	{ 0x21, kBoyleNpc, 1 }
};

static const DialogueLineEntry kBoyleIntroResponse1TailLines[] = {
	{ 0x3a, kBoyleNpc, 0 },
	{ 0x41, kPcSpeaker, 2 },
	{ 0x45, kBoyleNpc, 0 }
};

static const DialogueLineEntry kBoyleIntroResponse2Choice2Lines[] = {
	{ 0x76, kBoyleNpc, 0 },
	{ 0x77, kBoyleNpc, 2 }
};

static const DialogueLineEntry kBoyleKeywordTopic0eLines[] = {
	{ 0x8c, kPcSpeaker, 0 },
	{ 0x90, kBoyleNpc, 0 },
	{ 0x94, kPcSpeaker, 0 },
	{ 0x98, kBoyleNpc, 0 },
	{ 0x9d, kPcSpeaker, 0 },
	{ 0xa1, kBoyleNpc, 1 }
};

static const DialogueLineEntry kBoyleKeywordTopic17Lines[] = {
	{ 0xee, kPcSpeaker, 0 },
	{ 0xf2, kBoyleNpc, 2 },
	{ 0xf6, kPcSpeaker, 4 },
	{ 0xfa, kBoyleNpc, 0 }
};

static const DialogueLineEntry kBoyleKeywordArsonResponse1Lines[] = {
	{ 0x126, kBoyleNpc, 0 },
	{ 0x12c, kPcSpeaker, 0 },
	{ 0x130, kBoyleNpc, 2 }
};

static const DialogueLineEntry kBoyleFollowupBurnedTvLines[] = {
	{ 0x28a, kBoyleNpc, 1 },
	{ 0x28e, kPcSpeaker, 0 },
	{ 0x292, kBoyleNpc, 1 }
};

static const int kBoyleKeywordTopic0fTo12Lines[] = { 0xf, 0x10, 0x11, 0x12 };
static const int kBoyleKeywordTopic13To14Lines[] = { 0x13, 0x14 };
static const int kBoyleKeywordTopic15To16Lines[] = { 0x15, 0x16 };
static const int kBoyleKeywordTopic18To1bLines[] = { 0x18, 0x19, 0x1a, 0x1b };

} // End of namespace

bool BoyleDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kBoyleNpc);
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
	auto playLine = [&](int wavId, const char *speakerId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, speakerId, headVariant);
	};
	auto playBoyleLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return playLine(wavId, kBoyleNpc, headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
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
			lineError = playSequence(kBoyleButtonIntroLines, ARRAYSIZE(kBoyleButtonIntroLines));
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

				if (state.extraEvidenceReplyEnabled) {
					lineError = playLine(0x186, kPcSpeaker, 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = playBoyleLine(0x18b);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				} else {
					lineError = playBoyleLine(0x1d2);
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
				return playSequence(kBoyleHaveLodgeAppEvidenceLines,
					ARRAYSIZE(kBoyleHaveLodgeAppEvidenceLines));
			}

			lineError = playBoyleLine(0x197);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x3, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				sharedState.boyleGascanApplicationState = true;
				lineError = playSequence(kBoyleNoteResponse1IntroLines,
					ARRAYSIZE(kBoyleNoteResponse1IntroLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				if (state.extraEvidenceReplyEnabled) {
					lineError = playLine(0x186, kPcSpeaker, 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = playBoyleLine(0x1b8);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
				return playBoyleLine(0x1d2);
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

		if (usedItemName.equalsIgnoreCase(kGascanObject) &&
				sharedState.boyleGascanApplicationState) {
			lineError = playSequence(kBoyleGascanRewardLines,
				ARRAYSIZE(kBoyleGascanRewardLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			(void)hideInventoryItem(kGascanObject);
			(void)runtime.startupScript().addRuntimeObjectToInventory(kLodgeApplicationObject);
			(void)runtime.startupScript().setRuntimeFlagValue(kHaveLodgeAppFlag, true);
			return Common::kNoError;
		}

		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			return playBoyleLine(0x25f, 2);
		}

		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			lineError = playBoyleLine(0x24f, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dialogueStateD2ebc) {
				lineError = playLine(0x253, kPcSpeaker, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			return playBoyleLine(0x258, 2);
		}

		return playBoyleLine(0x249);
	}

	if (state.introPending) {
		state.introPending = false;

		lineError = playSequence(kBoyleIntroOpeningLines, ARRAYSIZE(kBoyleIntroOpeningLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x5, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		bool seedButtonTopicBuffer = false;
		if (responseIndex == 1) {
			lineError = playSequence(kBoyleIntroResponse1Lines,
				ARRAYSIZE(kBoyleIntroResponse1Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int followupResponseIndex = 0;
			responseError = runtime.runResponseMenu(0x6, followupResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (followupResponseIndex == 1) {
				lineError = playBoyleLine(0x2f);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (followupResponseIndex == 2) {
				lineError = playBoyleLine(0x34, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = playSequence(kBoyleIntroResponse1TailLines,
				ARRAYSIZE(kBoyleIntroResponse1TailLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			seedButtonTopicBuffer = true;
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
				state.extraEvidenceReplyEnabled = true;
				lineError = playSequence(kBoyleIntroResponse2Choice2Lines,
					ARRAYSIZE(kBoyleIntroResponse2Choice2Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = playBoyleLine(0x80, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignBoyleTopicBuffer(0x9);
			seedButtonTopicBuffer = true;
		}

		if (seedButtonTopicBuffer && hasBoylesButton)
			assignBoyleTopicBuffer(0xa);
	} else {
		lineError = playBoyleLine(0x23c);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (hasBoylesButton)
			assignBoyleTopicBuffer(0xb);
	}

	if ((sharedState.discussedNoteCheckbookEvidence != 0 ||
				sharedState.dwayneDiscussedBoylesButton) &&
			!state.evidenceFollowupShown) {
		state.evidenceFollowupShown = true;
		lineError = playBoyleLine(0x210, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.discussedNoteCheckbookEvidence != 0) {
			lineError = playBoyleLine(0x21a, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (sharedState.dwayneDiscussedBoylesButton) {
			lineError = playBoyleLine(0x215, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (!runtime.startupScript().getFlagValue(kHaveLodgeAppFlag)) {
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
		lineError = playBoyleLine(0x267);
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
			lineError = playBoyleLine(0xab);
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic13To14Lines,
				ARRAYSIZE(kBoyleKeywordTopic13To14Lines))) {
			lineError = playBoyleLine(0xb6, 3);
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic15To16Lines,
				ARRAYSIZE(kBoyleKeywordTopic15To16Lines))) {
			lineError = runtime.startupScript().getCurrentStoryDayIndex() >= 3
				? playBoyleLine(0xc6)
				: playBoyleLine(0x243);
		} else if (runtime.matchesResponseLine(selectedTopic, 0x17)) {
			lineError = playSequence(kBoyleKeywordTopic17Lines,
				ARRAYSIZE(kBoyleKeywordTopic17Lines));
		} else if (runtime.matchesAnyResponseLine(selectedTopic, kBoyleKeywordTopic18To1bLines,
				ARRAYSIZE(kBoyleKeywordTopic18To1bLines))) {
			if (!sharedState.dialogueStateD2eb0) {
				lineError = runtime.playDialogueFst(kDialogueC019BFstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playBoyleLine(0x10e);
			} else {
				lineError = playBoyleLine(0x115, 1);
			}
		} else if (runtime.matchesResponseLine(selectedTopic, 0x1d) ||
				selectedTopic.equalsIgnoreCase(kBoyleArsonistTopic)) {
			lineError = playBoyleLine(0x11c);
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
				lineError = playBoyleLine(0x135);
			} else {
				lineError = Common::kNoError;
			}
		} else if (runtime.matchesResponseLine(selectedTopic, 0x1f)) {
			lineError = playSequence(kBoyleHaveLodgeAppEvidenceLines,
				ARRAYSIZE(kBoyleHaveLodgeAppEvidenceLines));
		} else {
			lineError = playBoyleLine(0x243);
		}

		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
