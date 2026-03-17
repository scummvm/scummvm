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

#include "harvester/npc/hank_dialogue.h"

#include "common/array.h"
#include "common/debug.h"
#include "harvester/detection.h"
#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"
#include "harvester/startup_text.h"

namespace Harvester {

namespace {

static const char *const kDialogueRangshotFstPath = "GRAPHIC/FST/RANGSHOT.FST";

struct HankDialogueTopicLine {
	int responseLineIndex;
	int wavId;
	const char *speakerId;
	bool setDiscussedLodgeTopic;
};

static const HankDialogueTopicLine kHankDialogueTopicLines[] = {
	{ 0xd2, 0x725, "HANK", false },
	{ 0xd6, 0x74f, "PC", false },
	{ 0xd8, 0x76d, "PC", false },
	{ 0xd9, 0x76d, "PC", false },
	{ 0xdb, 0x77b, "PC", false },
	{ 0xdc, 0x77b, "PC", false },
	{ 0xe0, 0x7ad, "HANK", false },
	{ 0xe1, 0x7b7, "PC", false },
	{ 0xe3, 0x7d6, "PC", false },
	{ 0xe5, 0x7f4, "HANK", false },
	{ 0xe6, 0x7fc, "PC", false },
	{ 0xe9, 0x82d, "PC", false },
	{ 0xeb, 0x863, "PC", false },
	{ 0xed, 0x881, "PC", false },
	{ 0xee, 0x8a0, "HANK", false },
	{ 0xef, 0x8a0, "HANK", false },
	{ 0xf0, 0x8a0, "HANK", false },
	{ 0xf1, 0x8bf, "HANK", true }
};

static void splitDialogueMenuLine(const Common::String &line, Common::Array<Common::String> &parts) {
	parts.clear();
	if (line.empty())
		return;

	Common::String token;
	for (uint i = 0; i < line.size(); ++i) {
		if (line[i] == '/') {
			parts.push_back(token);
			token.clear();
			continue;
		}

		token += line[i];
	}

	parts.push_back(token);
}

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

	debugC(1, kDebugDialogue,
		"Harvester: Hank dialogue start day=%d item='%s' initial=%d trackedDayValid=%d trackedDay=%d sameDayPending=%d rangshotPending=%d topicLine=%d topicBuffer='%s'",
		currentStoryDayIndex, usedItemName.empty() ? "<none>" : usedItemName.c_str(),
		(int)state.pendingInitialConversation,
		(int)state.hasTrackedDayState, state.trackedDayIndex,
		(int)state.pendingSameDayFollowup,
		(int)state.pendingRangshotSequence,
		hankTopicBufferLineIndex, hankTopicBuffer.c_str());

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return runtime.playDialogueLine(0xa3e, "HANK");
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return runtime.playDialogueLine(0xa53, "HANK");
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
			state.pendingSameDayFollowup = false;
			state.pendingRangshotSequence = true;

			Common::Error lineError = runtime.playDialogueLine(0x8f5, "HANK");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (state.pendingRangshotSequence) {
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
		Common::Error lineError = runtime.playDialogueLine(0x987, "PC");
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
				{ 0x9bc, "HANK", 0 },
				{ 0x9c9, "HANK", 0 }
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
			lineError = runtime.playDialogueLine(0xa26, "HANK");
			break;
		default:
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLine(0xa2b, "HANK");
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
		debugC(1, kDebugDialogue, "Harvester: Hank selected topic='%s'", selectedTopic.c_str());
		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()))
			return runtime.playDialogueLine(0x8dc, "HANK");

		const Common::String momTopic = runtime.startupText().getDialogueResponseLine(0xd2);
		Common::Array<Common::String> currentTopics;
		splitDialogueMenuLine(hankTopicBuffer, currentTopics);
		const bool matchesMomTopicByLine = !momTopic.empty() && selectedTopic.equalsIgnoreCase(momTopic);
		const bool matchesMomTopicFromIntroBuffer = hankTopicBufferLineIndex == 0xc8 &&
			!currentTopics.empty() && selectedTopic.equalsIgnoreCase(currentTopics[0]);
		if (matchesMomTopicByLine || matchesMomTopicFromIntroBuffer) {
			debugC(1, kDebugDialogue,
				"Harvester: Hank matched special Mom branch '%s' (lineMatch=%d bufferMatch=%d)",
				selectedTopic.c_str(), (int)matchesMomTopicByLine, (int)matchesMomTopicFromIntroBuffer);
			const DialogueLineEntry lines[] = {
				{ 0x725, "HANK", 0 },
				{ 0x729, "PC", 0 },
				{ 0x72d, "HANK", 0 },
				{ 0x733, "PC", 0 },
				{ 0x737, "HANK", 0 },
				{ 0x73b, "PC", 0 },
				{ 0x741, "HANK", 0 },
				{ 0x747, "MOM", 2 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.setActiveSpeakerPortrait("HANK", 0);
			assignHankTopicBuffer(0xd5);
			continue;
		}

		bool handledTopic = false;
		for (const HankDialogueTopicLine &topic : kHankDialogueTopicLines) {
			if (topic.responseLineIndex == 0xd2)
				continue;

			const Common::String topicText =
				runtime.startupText().getDialogueResponseLine(topic.responseLineIndex);
			if (topicText.empty() || !selectedTopic.equalsIgnoreCase(topicText))
				continue;

			if (topic.setDiscussedLodgeTopic)
				sharedState.discussedLodgeTopic = true;

			debugC(1, kDebugDialogue,
				"Harvester: Hank matched topic '%s' to response line 0x%x (%d) -> wav=0x%x speaker='%s'",
				selectedTopic.c_str(), topic.responseLineIndex, topic.responseLineIndex + 1,
				topic.wavId, topic.speakerId);
			Common::Error lineError = runtime.playDialogueLine(topic.wavId, topic.speakerId);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			handledTopic = true;
			break;
		}

		Common::Error lineError = runtime.playDialogueLine(0xa32, "HANK");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (handledTopic) {
			debugC(1, kDebugDialogue,
				"Harvester: Hank handled topic '%s' and now falls through to generic response/exit",
				selectedTopic.c_str());
		} else {
			debugC(1, kDebugDialogue,
				"Harvester: Hank topic '%s' fell back to generic response and exits dialogue",
				selectedTopic.c_str());
		}

		return runtime.playDialogueLine(0x8dc, "HANK");
	}
}

} // End of namespace Harvester
