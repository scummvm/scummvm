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


#include "harvester/npc/ryder_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kPcSpeakerId = "PC";
static const char *const kRyderSpeakerId = "RYDER";
static const char *const kRyderTvOutSpeakerId = "RYDER_TV_OUT";
static const char *const kRyderIntroFstPath = "GRAPHIC/FST/C063A.FST";
static const char *const kRyderFollowupFstPath = "GRAPHIC/FST/C063B.FST";
static const char *const kRyderKarinIntroFstPath = "GRAPHIC/FST/C063D.FST";
static const char *const kRyderKarinExitFstPath = "GRAPHIC/FST/C063E.FST";
static const char *const kRyderTopicFstPath = "GRAPHIC/FST/C101.FST";
static const int kRyderInterviewTopicLineIndices[] = { 0x2a7, 0x2a8 };

static const DialogueLineEntry kRyderWhaleyPhotoLines[] = {
	{ 0x52d, kRyderSpeakerId, 0 },
	{ 0x533, kPcSpeakerId, 2 },
	{ 0x538, kRyderSpeakerId, 0 },
	{ 0x542, kRyderSpeakerId, 1 },
	{ 0x546, kRyderSpeakerId, 1 }
};

static const DialogueLineEntry kRyderBlackmailEvidenceLines[] = {
	{ 0x556, kRyderSpeakerId, 0 },
	{ 0x560, kRyderSpeakerId, 1 },
	{ 0x564, kRyderSpeakerId, 1 }
};

} // End of namespace

bool RyderDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kRyderSpeakerId) || npcName.equalsIgnoreCase(kRyderTvOutSpeakerId);
}

Common::Error RyderDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	const bool burnedTvStation = runtime.startupScript().getFlagValue("BURNED_TV_STATION");
	const char *speakerId = burnedTvStation ? kRyderTvOutSpeakerId : kRyderSpeakerId;
	auto playSpeakerLine = [&](int wavId, int headVariant, const char *lineSpeakerId) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, lineSpeakerId, headVariant);
	};
	auto playRyderLine = [&](int wavId, int headVariant) -> Common::Error {
		return playSpeakerLine(wavId, headVariant, speakerId);
	};

	if (burnedTvStation) {
		Common::Error lineError = playSpeakerLine(0x5a6, 3, kRyderTvOutSpeakerId);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (!runtime.startupScript().getFlagValue("BOYLE_BURNED_TV")) {
			lineError = playSpeakerLine(0x5ab, 4, kPcSpeakerId);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		return playSpeakerLine(0x5af, 3, kRyderTvOutSpeakerId);
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		sharedState.discussedWhaleyHerrillPhoto = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return runtime.playDialogueEntrySequence(kRyderWhaleyPhotoLines, ARRAYSIZE(kRyderWhaleyPhotoLines));
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		sharedState.discussedCasketPhotoEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		return playRyderLine(0x54f, 1);
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		sharedState.discussedNoteCheckbookEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		return runtime.playDialogueEntrySequence(
			kRyderBlackmailEvidenceLines, ARRAYSIZE(kRyderBlackmailEvidenceLines));
	}
	if (!usedItemName.empty())
		return playRyderLine(0x4db, 1);

	Common::Error lineError = Common::kNoError;
	if (_state.talkStatePending) {
		_state.talkStatePending = false;

		lineError = playRyderLine(0x477, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kRyderIntroFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playRyderLine(0x485, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playSpeakerLine(0x489, 0, kPcSpeakerId);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kRyderFollowupFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playRyderLine(0x49b, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2a1, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			(void)runtime.startupScript().addRuntimeObjectToInventory("AUTOGRAPH");
			lineError = playRyderLine(0x4a5, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playRyderLine(0x4a9, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playRyderLine(0x4ae, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	} else {
		lineError = playRyderLine(0x4cf, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!sharedState.karinKidnapedDialogueState) {
		sharedState.karinKidnapedDialogueState = true;

		lineError = playRyderLine(0x56d, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kRyderKarinIntroFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playRyderLine(0x57f, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2a3, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playRyderLine(0x589, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playRyderLine(0x592, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kRyderKarinExitFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playRyderLine(0x58e, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playRyderLine(0x4a5, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)runtime.startupScript().addRuntimeObjectToInventory("AUTOGRAPH");
		}
	}

	Common::String topicBuffer;
	int topicBufferLineIndex = -1;
	runtime.assignTopicBuffer(topicBuffer, topicBufferLineIndex, 0x2a5, "Ryder topic buffer");

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(topicBuffer, topicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()) ||
				runtime.matchesResponseLine(selectedTopic, 0x2a6))
			return playRyderLine(0x4e1, 1);

		if (runtime.matchesAnyResponseLine(selectedTopic, kRyderInterviewTopicLineIndices,
					ARRAYSIZE(kRyderInterviewTopicLineIndices))) {
			lineError = playRyderLine(0x4e7, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2a9, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playRyderLine(0x4f2, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playSpeakerLine(0x4f9, 0, kPcSpeakerId);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playRyderLine(0x4fe, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kRyderTopicFstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playRyderLine(0x4ff, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playSpeakerLine(0x50c, 0, kPcSpeakerId);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playRyderLine(0x511, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playRyderLine(0x517, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playSpeakerLine(0x51c, 0, kPcSpeakerId);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playRyderLine(0x521, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			return playRyderLine(0x527, 0);
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x2aa))
			continue;

		lineError = playRyderLine(0x4d5, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
