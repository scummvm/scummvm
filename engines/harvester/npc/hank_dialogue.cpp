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


#include "harvester/npc/hank_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC021FstPath = "GRAPHIC/FST/C021.FST";
static const char *const kDialogueRangshotFstPath = "GRAPHIC/FST/RANGSHOT.FST";
static const char *const kInventoryOwnerName = "INVENTORY";

static const DialogueLineEntry kHankMomTopicLines[] = {
	{ 0x725, "HANK", 0 },
	{ 0x729, "PC", 0 },
	{ 0x72d, "HANK", 0 },
	{ 0x733, "PC", 0 },
	{ 0x737, "HANK", 0 },
	{ 0x73b, "PC", 0 },
	{ 0x741, "HANK", 0 },
	{ 0x747, "MOM", 2 }
};

static const DialogueLineEntry kHankSteveTopicLines[] = {
	{ 0x74f, "PC", 0 },
	{ 0x753, "HANK", 0 },
	{ 0x758, "PC", 2 },
	{ 0x75d, "HANK", 0 },
	{ 0x761, "PC", 2 },
	{ 0x765, "HANK", 0 }
};

static const DialogueLineEntry kHankLousyRatTopicLines[] = {
	{ 0x76d, "PC", 0 },
	{ 0x771, "HANK", 0 }
};

static const DialogueLineEntry kHankLightingOutTopicLines[] = {
	{ 0x77b, "PC", 0 },
	{ 0x77f, "HANK", 0 },
	{ 0x784, "PC", 2 },
	{ 0x788, "HANK", 0 },
	{ 0x78e, "MOM", 2 },
	{ 0x793, "HANK", 0 }
};

static const DialogueLineEntry kHankDaddyTopicLines[] = {
	{ 0x7b7, "PC", 0 },
	{ 0x7bb, "HANK", 0 },
	{ 0x7c0, "PC", 0 },
	{ 0x7c4, "HANK", 0 },
	{ 0x7ca, "PC", 0 },
	{ 0x7ce, "HANK", 0 }
};

static const DialogueLineEntry kHankGirlTopicLines[] = {
	{ 0x7d6, "PC", 0 },
	{ 0x7da, "HANK", 0 },
	{ 0x7df, "PC", 0 },
	{ 0x7e3, "HANK", 0 },
	{ 0x7e8, "PC", 4 },
	{ 0x7ec, "HANK", 0 }
};

static const DialogueLineEntry kHankTvTopicIntroLines[] = {
	{ 0x7fc, "PC", 0 },
	{ 0x801, "HANK", 0 },
	{ 0x805, "PC", 0 },
	{ 0x809, "HANK", 0 }
};

static const DialogueLineEntry kHankTvTopicOutroLines[] = {
	{ 0x812, "HANK", 0 },
	{ 0x816, "PC", 0 },
	{ 0x81a, "HANK", 0 },
	{ 0x81e, "PC", 0 },
	{ 0x824, "HANK", 0 }
};

static const DialogueLineEntry kHankCowboyShowTopicLines[] = {
	{ 0x82d, "PC", 0 },
	{ 0x831, "HANK", 0 },
	{ 0x835, "PC", 0 },
	{ 0x839, "HANK", 0 },
	{ 0x83d, "PC", 4 },
	{ 0x842, "HANK", 0 },
	{ 0x848, "HANK", 0 },
	{ 0x84c, "HANK", 0 }
};

static const DialogueLineEntry kHankSickTopicLines[] = {
	{ 0x863, "PC", 0 },
	{ 0x867, "HANK", 0 },
	{ 0x86b, "PC", 0 },
	{ 0x86f, "HANK", 0 },
	{ 0x873, "PC", 0 },
	{ 0x877, "HANK", 0 }
};

static const DialogueLineEntry kHankNewspaperFireSentinelTopicLines[] = {
	{ 0x8a0, "HANK", 0 },
	{ 0x8a5, "PC", 0 },
	{ 0x8a9, "HANK", 0 },
	{ 0x8af, "PC", 0 },
	{ 0x8b3, "HANK", 0 }
};

static const DialogueLineEntry kHankLodgeTopicLines[] = {
	{ 0x8bf, "HANK", 0 },
	{ 0x8c5, "PC", 0 },
	{ 0x8c9, "HANK", 0 },
	{ 0x8cf, "PC", 0 },
	{ 0x8d3, "HANK", 0 }
};

static const DialogueLineEntry kHankRangshotResponse1Lines[] = {
	{ 0x8ff, "HANK", 0 },
	{ 0x903, "PC", 0 },
	{ 0x908, "HANK", 0 },
	{ 0x90d, "PC", 4 },
	{ 0x911, "HANK", 0 }
};

static const DialogueLineEntry kHankRangshotResponse2Lines[] = {
	{ 0x918, "HANK", 0 },
	{ 0x91f, "PC", 0 },
	{ 0x924, "HANK", 0 }
};

static const DialogueLineEntry kHankBurnedTvStationLines[] = {
	{ 0x987, "PC", 0 },
	{ 0x98b, "HANK", 0 },
	{ 0x98f, "PC", 0 },
	{ 0x993, "HANK", 0 },
	{ 0x99a, "MOM", 2 },
	{ 0x99f, "HANK", 0 },
	{ 0x9a6, "HANK", 0 },
	{ 0x9aa, "HANK", 0 }
};

static const DialogueLineEntry kHankKarinFoundDeadResponse2Lines[] = {
	{ 0xa26, "HANK", 0 },
	{ 0xa2b, "HANK", 0 }
};

static const DialogueLineEntry kHankResponse0EdTopicLines[] = {
	{ 0x881, "PC", 0 },
	{ 0x885, "HANK", 0 },
	{ 0x88a, "PC", 0 },
	{ 0x88f, "HANK", 0 },
	{ 0x894, "PC", 0 },
	{ 0x898, "HANK", 0 }
};

} // End of namespace

bool HankDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("HANK");
}

Common::Error HankDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	const int currentStoryDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
	HankRoomDialogueState &state = _state;
	Common::String &hankTopicBuffer = state.currentTopicBuffer;
	int &hankTopicBufferLineIndex = state.currentTopicBufferLineIndex;
	auto assignHankTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(hankTopicBuffer, hankTopicBufferLineIndex,
			responseLineIndex, "Hank topic buffer");
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto playLine = [&](int wavId, const char *speakerId = "HANK",
			int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, speakerId, headVariant);
	};
	auto hidePresentedInventoryItem = [&]() {
		(void)runtime.startupScript().setRuntimeObjectVisible(
			kInventoryOwnerName, usedItemName, false);
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			Common::Error lineError = runtime.playDialogueLine(0xa3e, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0xc5, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				(void)runtime.startupScript().addRuntimeObjectToInventory("GILLMAN");
				hidePresentedInventoryItem();
				return runtime.playDialogueLine(0xa5e, "HANK");
			}
			if (responseIndex == 2)
				return runtime.playDialogueLine(0xa62, "HANK");
			return Common::kNoError;
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			Common::Error lineError = runtime.playDialogueLine(0xa53, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0xc6, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				(void)runtime.startupScript().addRuntimeObjectToInventory("ROBOT");
				hidePresentedInventoryItem();
				return runtime.playDialogueLine(0xa5e, "HANK");
			}
			if (responseIndex == 2)
				return runtime.playDialogueLine(0xa62, "HANK");
			return Common::kNoError;
		}
		return runtime.playDialogueLine(0xa38, "HANK");
	}

	bool skipHankFollowupBranches = false;
	if (state.pendingInitialConversation) {
		state.trackedDayIndex = currentStoryDayIndex;
		state.hasTrackedDayState = true;
		state.pendingSameDayFollowup = true;
		state.pendingInitialConversation = false;
		hankTopicBuffer.clear();
		hankTopicBufferLineIndex = -1;
		skipHankFollowupBranches = true;

		Common::Error lineError = runtime.playDialogueLine(0x703, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xc7, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1:
			lineError = runtime.playDialogueLine(0x70f, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xc8);
			break;
		case 2:
			lineError = runtime.playDialogueLine(0x715, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xc9);
			break;
		case 3:
			lineError = runtime.playDialogueLine(0x71c, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xca);
			break;
		default:
			break;
		}
	}

	if (!skipHankFollowupBranches && (!state.hasTrackedDayState ||
			currentStoryDayIndex != state.trackedDayIndex)) {
		if (state.pendingSameDayFollowup) {
			state.trackedDayIndex = currentStoryDayIndex;
			state.hasTrackedDayState = true;
			state.pendingSameDayFollowup = false;
			state.pendingRangshotSequence = true;

			Common::Error lineError = runtime.playDialogueLine(0x8f5, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (currentStoryDayIndex < 4) {
				int responseIndex = 0;
				Common::Error responseError = runtime.runResponseMenu(0xcb, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;
				if (responseIndex == 1) {
					lineError = playSequence(
						kHankRangshotResponse1Lines, ARRAYSIZE(kHankRangshotResponse1Lines));
				} else if (responseIndex == 2) {
					lineError = playSequence(
						kHankRangshotResponse2Lines, ARRAYSIZE(kHankRangshotResponse2Lines));
				}
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		} else if (state.pendingRangshotSequence) {
			Common::Error lineError = currentStoryDayIndex > 5
				? runtime.playDialogueLine(0x8e2, "HANK")
				: runtime.playDialogueFst(kDialogueRangshotFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	} else if (!skipHankFollowupBranches) {
		state.pendingSameDayFollowup = true;
		Common::Error lineError = currentStoryDayIndex > 5
			? runtime.playDialogueLine(0x8e2, "HANK")
			: runtime.playDialogueFst(kDialogueRangshotFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgamePlayedShown) {
		state.stephMidgamePlayedShown = true;

		Common::Error lineError = runtime.playDialogueLine(0x92c, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xcc, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineEntry lines[] = {
				{ 0x939, "HANK", 0 },
				{ 0x93f, "MOM", 2 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		case 2:
			lineError = runtime.playDialogueLine(0x944, "HANK");
			break;
		case 3:
			lineError = runtime.playDialogueLine(0x948, "HANK");
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLine(0x94f, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0xcd, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineEntry lines[] = {
				{ 0x95a, "HANK", 0 },
				{ 0x961, "PC", 2 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		case 2: {
			const DialogueLineEntry lines[] = {
				{ 0x965, "HANK", 0 },
				{ 0x96b, "PC", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLine(0x971, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		Common::Error lineError = playSequence(
			kHankBurnedTvStationLines, ARRAYSIZE(kHankBurnedTvStationLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("BUSTED_ONCE") &&
			!state.bustedOnceShown) {
		state.bustedOnceShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x9b1, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xce, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineEntry lines[] = {
				{ 0x9bc, "HANK", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		case 2: {
			const DialogueLineEntry lines[] = {
				{ 0x9c1, "HANK", 0 },
				{ 0x9c5, "PC", 0 },
				{ 0x9c9, "HANK", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			break;
		}
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnapedShown) {
		state.karinKidnapedShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x9d5, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (sharedState.karinKidnapedDialogueState) {
			lineError = runtime.playDialogueLineWithVariant(0x9da, "PC", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		const DialogueLineEntry lines[] = {
			{ 0x9de, "HANK", 0 },
			{ 0x9df, "HANK", 0 },
			{ 0x9e0, "HANK", 0 },
			{ 0x9e1, "HANK", 0 },
			{ 0x9e2, "HANK", 0 }
		};
		lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xcf, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1: {
			const DialogueLineEntry responseLines[] = {
				{ 0x9f4, "HANK", 0 },
				{ 0x9fc, "HANK", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			break;
		}
		case 2:
			lineError = runtime.playDialogueLine(0xa01, "HANK");
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinFoundAliveShown) {
		state.karinFoundAliveShown = true;
		Common::Error lineError = runtime.playDialogueLine(0xa0b, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!skipHankFollowupBranches && runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;
		Common::Error lineError = runtime.playDialogueLine(0xa15, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0xd0, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1:
			lineError = runtime.playDialogueLine(0xa20, "HANK");
			break;
		case 2:
			lineError = playSequence(
				kHankKarinFoundDeadResponse2Lines,
				ARRAYSIZE(kHankKarinFoundDeadResponse2Lines));
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			hankTopicBuffer, hankTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;

		if (selectedTopic.empty())
			return Common::kNoError;
		if (runtime.matchesResponseLine(selectedTopic, 0xd2)) {
			Common::Error lineError = playSequence(
				kHankMomTopicLines, ARRAYSIZE(kHankMomTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.setActiveSpeakerPortrait("HANK", 0);
			assignHankTopicBuffer(0xd5);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xd6)) {
			Common::Error lineError = playSequence(
				kHankSteveTopicLines, ARRAYSIZE(kHankSteveTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xd7);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xd8) ||
				runtime.matchesResponseLine(selectedTopic, 0xd9)) {
			Common::Error lineError = playSequence(
				kHankLousyRatTopicLines, ARRAYSIZE(kHankLousyRatTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xda);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xdb) ||
				runtime.matchesResponseLine(selectedTopic, 0xdc)) {
			Common::Error lineError = playSequence(
				kHankLightingOutTopicLines, ARRAYSIZE(kHankLightingOutTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0xdd, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playLine(0x79d);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignHankTopicBuffer(0xde);
				continue;
			}
			if (responseIndex == 2) {
				lineError = playLine(0x7a4);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignHankTopicBuffer(0xdf);
				continue;
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xe0)) {
			Common::Error lineError = playLine(0x7ad);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xe1)) {
			Common::Error lineError = playSequence(
				kHankDaddyTopicLines, ARRAYSIZE(kHankDaddyTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xe2);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xe3)) {
			Common::Error lineError = playSequence(
				kHankGirlTopicLines, ARRAYSIZE(kHankGirlTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xe4);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xe5)) {
			Common::Error lineError = playLine(0x7f4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xe6)) {
			Common::Error lineError = playSequence(
				kHankTvTopicIntroLines, ARRAYSIZE(kHankTvTopicIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC021FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSequence(
				kHankTvTopicOutroLines, ARRAYSIZE(kHankTvTopicOutroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xe8);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xe9)) {
			Common::Error lineError = playSequence(
				kHankCowboyShowTopicLines, ARRAYSIZE(kHankCowboyShowTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0xea, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playLine(0x856);
			} else if (responseIndex == 2) {
				lineError = playLine(0x85a);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xeb)) {
			Common::Error lineError = playSequence(
				kHankSickTopicLines, ARRAYSIZE(kHankSickTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignHankTopicBuffer(0xec);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xed)) {
			Common::Error lineError = playSequence(
				kHankResponse0EdTopicLines, ARRAYSIZE(kHankResponse0EdTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xee) ||
				runtime.matchesResponseLine(selectedTopic, 0xef) ||
				runtime.matchesResponseLine(selectedTopic, 0xf0)) {
			Common::Error lineError = playSequence(
				kHankNewspaperFireSentinelTopicLines,
				ARRAYSIZE(kHankNewspaperFireSentinelTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xf1)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playSequence(
				kHankLodgeTopicLines, ARRAYSIZE(kHankLodgeTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0xf2))
			return playLine(0x8dc);

		Common::Error lineError = playLine(0xa32);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		continue;
	}
}

} // End of namespace Harvester
