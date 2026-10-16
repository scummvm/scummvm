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

static const char *const kRyderIntroFstPath = "GRAPHIC/FST/C063A.FST";
static const char *const kRyderFollowupFstPath = "GRAPHIC/FST/C063B.FST";
static const char *const kRyderKarinIntroFstPath = "GRAPHIC/FST/C063D.FST";
static const char *const kRyderKarinExitFstPath = "GRAPHIC/FST/C063E.FST";
static const char *const kRyderTopicFstPath = "GRAPHIC/FST/C101.FST";
static const int kRyderInterviewTopicLineIndices[] = { 0x2a7, 0x2a8 };

static const DialogueLineEntry kRyderWhaleyPhotoLines[] = {
	{ 0x52d, "RYDER", 0 },
	{ 0x533, "PC", 2 },
	{ 0x538, "RYDER", 0 },
	{ 0x542, "RYDER", 1 },
	{ 0x546, "RYDER", 1 }
};

static const DialogueLineEntry kRyderBlackmailEvidenceLines[] = {
	{ 0x556, "RYDER", 0 },
	{ 0x560, "RYDER", 1 },
	{ 0x564, "RYDER", 1 }
};

} // End of namespace

Common::Error RyderDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	const bool burnedTvStation = runtime.startupScript().getFlagValue("BURNED_TV_STATION");
	const char *speakerId = burnedTvStation ? "RYDER_TV_OUT" : "RYDER";

	if (burnedTvStation) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x5a6, "RYDER_TV_OUT", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (!runtime.startupScript().getFlagValue("BOYLE_BURNED_TV")) {
			lineError = runtime.playDialogueLineWithVariant(0x5ab, "PC", 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		return runtime.playDialogueLineWithVariant(0x5af, "RYDER_TV_OUT", 3);
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
		return runtime.playDialogueLineWithVariant(0x54f, speakerId, 1);
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
		return runtime.playDialogueLineWithVariant(0x4db, speakerId, 1);

	Common::Error lineError = Common::kNoError;
	if (_state.talkStatePending) {
		_state.talkStatePending = false;

		lineError = runtime.playDialogueLineWithVariant(0x477, speakerId, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kRyderIntroFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x485, speakerId, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x489, "PC", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kRyderFollowupFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x49b, speakerId, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2a1, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			(void)runtime.startupScript().addRuntimeObjectToInventory("AUTOGRAPH");
			lineError = runtime.playDialogueLineWithVariant(0x4a5, speakerId, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x4a9, speakerId, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x4ae, speakerId, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	} else {
		lineError = runtime.playDialogueLineWithVariant(0x4cf, speakerId, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!sharedState.karinKidnapedDialogueState) {
		sharedState.karinKidnapedDialogueState = true;

		lineError = runtime.playDialogueLineWithVariant(0x56d, speakerId, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kRyderKarinIntroFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x57f, speakerId, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2a3, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x589, speakerId, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x592, speakerId, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kRyderKarinExitFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x58e, speakerId, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x4a5, speakerId, 1);
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
			return runtime.playDialogueLineWithVariant(0x4e1, speakerId, 1);

		if (runtime.matchesAnyResponseLine(selectedTopic, kRyderInterviewTopicLineIndices,
					ARRAYSIZE(kRyderInterviewTopicLineIndices))) {
			lineError = runtime.playDialogueLineWithVariant(0x4e7, speakerId, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2a9, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x4f2, speakerId, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x4f9, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x4fe, speakerId, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kRyderTopicFstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x4ff, speakerId, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x50c, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x511, speakerId, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x517, speakerId, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x51c, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x521, speakerId, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			return runtime.playDialogueLineWithVariant(0x527, speakerId, 0);
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x2aa))
			continue;

		lineError = runtime.playDialogueLineWithVariant(0x4d5, speakerId, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
