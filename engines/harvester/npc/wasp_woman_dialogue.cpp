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

#include "harvester/npc/wasp_woman_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC027AFstPath = "GRAPHIC/FST/C027A.FST";

static const DialogueLineEntry kWaspWomanWhaleyLines[] = {
	{ 0x4ca6, "WASP_WOMAN", 0 },
	{ 0x4cae, "WASP_WOMAN", 1 },
	{ 0x4cb2, "PC", 0 },
	{ 0x4cb6, "WASP_WOMAN", 0 }
};

static const DialogueLineEntry kWaspWomanIntroTailLines[] = {
	{ 0x4bf6, "WASP_WOMAN", 0 },
	{ 0x4bfc, "PC", 0 }
};

static const DialogueLineEntry kWaspWomanTopic302PreludeLines[] = {
	{ 0x4c0a, "WASP_WOMAN", 0 },
	{ 0x4c13, "PC", 0 },
	{ 0x4c17, "WASP_WOMAN", 2 }
};

static const DialogueLineEntry kWaspWomanTopic302PostFstLines[] = {
	{ 0x4c25, "WASP_WOMAN", 1 },
	{ 0x4c29, "WASP_WOMAN", 1 }
};

static const DialogueLineEntry kWaspWomanTopic305Response1Lines[] = {
	{ 0x4c4d, "WASP_WOMAN", 1 },
	{ 0x4c53, "WASP_WOMAN", 1 },
	{ 0x4c57, "WASP_WOMAN", 2 }
};

static const DialogueLineEntry kWaspWomanTopic30aLines[] = {
	{ 0x4c74, "WASP_WOMAN", 1 },
	{ 0x4c7f, "WASP_WOMAN", 1 }
};

static const int kWaspWomanInitialTopicBufferResponseLine = 0x300;

} // End of namespace

bool WaspWomanDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("WASP_WOMAN");
}

Common::Error WaspWomanDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	WaspWomanRoomDialogueState &state = _state;
	Common::String waspWomanTopicBuffer;
	int waspWomanTopicBufferLineIndex = state.topicBufferLineIndex;
	auto assignWaspWomanTopicBuffer = [&](int responseLineIndex) {
		state.topicBufferLineIndex = responseLineIndex;
		runtime.assignTopicBuffer(waspWomanTopicBuffer, waspWomanTopicBufferLineIndex,
			responseLineIndex, "Wasp Woman topic buffer");
	};
	auto playWaspWomanLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "WASP_WOMAN", headVariant);
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return runtime.playDialogueEntrySequence(kWaspWomanWhaleyLines,
				ARRAYSIZE(kWaspWomanWhaleyLines));
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return playWaspWomanLine(0x4cbd, 0);
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playWaspWomanLine(0x4cc3, 0);
		}

		return playWaspWomanLine(0x4ca0, 0);
	}

	if (state.introPending) {
		state.introPending = false;

		Common::Error lineError = playWaspWomanLine(0x4bee, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.waspWomanDialogueState) {
			lineError = runtime.playDialogueLine(0x4bf2, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = runtime.playDialogueEntrySequence(kWaspWomanIntroTailLines,
			ARRAYSIZE(kWaspWomanIntroTailLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playWaspWomanLine(0x4c00, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignWaspWomanTopicBuffer(kWaspWomanInitialTopicBufferResponseLine);
	} else {
		Common::Error lineError = playWaspWomanLine(0x4bee, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignWaspWomanTopicBuffer(state.topicBufferLineIndex);
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			waspWomanTopicBuffer, waspWomanTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;
		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic())) {
			if (waspWomanTopicBufferLineIndex == kWaspWomanInitialTopicBufferResponseLine)
				return Common::kNoError;
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x302)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kWaspWomanTopic302PreludeLines, ARRAYSIZE(kWaspWomanTopic302PreludeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC027AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueEntrySequence(
				kWaspWomanTopic302PostFstLines, ARRAYSIZE(kWaspWomanTopic302PostFstLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignWaspWomanTopicBuffer(0x304);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x305)) {
			Common::Error lineError = playWaspWomanLine(0x4c31, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (!sharedState.waspWomanDialogueState)
				continue;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x307, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = runtime.playDialogueEntrySequence(kWaspWomanTopic305Response1Lines,
					ARRAYSIZE(kWaspWomanTopic305Response1Lines));
				break;
			case 2:
				lineError = playWaspWomanLine(0x4c5e, 2);
				break;
			default:
				lineError = Common::kNoError;
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x308)) {
			Common::Error lineError = playWaspWomanLine(0x4c67, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignWaspWomanTopicBuffer(0x309);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x30a)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kWaspWomanTopic30aLines, ARRAYSIZE(kWaspWomanTopic30aLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignWaspWomanTopicBuffer(0x30b);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x30c)) {
			Common::Error lineError = playWaspWomanLine(0x4c85, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		Common::Error lineError = playWaspWomanLine(0x4c9a, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return Common::kNoError;
	}
}

} // End of namespace Harvester
