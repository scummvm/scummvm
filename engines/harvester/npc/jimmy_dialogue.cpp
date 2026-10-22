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


#include "harvester/npc/jimmy_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kRahRoomName = "RAH";
static const char *const kJimmyFirstMeetingFlag = "PAPER_CHK_1";
static const char *const kGivenPaperTodayFlag = "GIVEN_PAPER_TODAY";
static const int kJimmyFirstResponseLineIndex = 0xf5;
static const int kJimmySecondResponseLineIndex = 0xf6;
static const int kJimmyThirdResponseLineIndex = 0xf7;
static const int kJimmySneakersResponseLineIndex = 0xf8;

static const DialogueLineEntry kJimmyPaperHandoffLines[] = {
	{ 0x4a4c, "JIMMY", 1 },
	{ 0x4a52, "PC", 0 }
};

static const DialogueLineEntry kJimmyFirstMeetingFollowupLines[] = {
	{ 0x4a6b, "JIMMY", 0 },
	{ 0x4a70, "PC", 0 },
	{ 0x4a74, "JIMMY", 0 },
	{ 0x4a7a, "JIMMY", 0 }
};

static const DialogueLineEntry kJimmySecondResponsePreludeLines[] = {
	{ 0x4a8a, "PC", 0 },
	{ 0x4a8e, "JIMMY", 1 }
};

static const DialogueLineEntry kJimmyNoSneakersLines[] = {
	{ 0x4aa6, "JIMMY", 0 },
	{ 0x4aaa, "PC", 0 },
	{ 0x4aae, "JIMMY", 2 }
};

static const DialogueLineEntry kJimmyLedgersAndCorpseEvidenceLines[] = {
	{ 0x4b00, "JIMMY", 0 },
	{ 0x4b04, "PC", 0 },
	{ 0x4b08, "JIMMY", 0 },
	{ 0x4b0c, "PC", 0 },
	{ 0x4b10, "JIMMY", 0 },
	{ 0x4b15, "PC", 0 },
	{ 0x4b19, "JIMMY", 0 }
};

static const DialogueLineEntry kJimmyBlackmailEvidenceLines[] = {
	{ 0x4b21, "JIMMY", 0 },
	{ 0x4b25, "PC", 0 },
	{ 0x4b29, "JIMMY", 0 },
	{ 0x4b2d, "PC", 0 },
	{ 0x4b31, "JIMMY", 1 }
};

static void restoreItemToRah(DialogueRuntime &runtime, const char *objectName) {
	(void)runtime.startupScript().resetRuntimeObjectToInitialState(objectName);
	(void)runtime.startupScript().setRuntimeObjectVisible(kRahRoomName, objectName, true);
}

static void completeSneakersTrade(DialogueRuntime &runtime) {
	restoreItemToRah(runtime, "SNEAKERS");
	(void)runtime.startupScript().addRuntimeObjectToInventory("BROOMKEY");
}

} // End of namespace

Common::Error JimmyDialogueHandler::playPaperHandoffPreludeIfNeeded(DialogueRuntime &runtime) {
	if (_state.paperHandoffStateSet)
		return Common::kNoError;

	Common::Error preludeError = runtime.playDialogueEntrySequence(kJimmyPaperHandoffLines,
		ARRAYSIZE(kJimmyPaperHandoffLines));
	if (preludeError.getCode() != Common::kNoError)
		return preludeError;

	_state.paperHandoffStateSet = true;
	return Common::kNoError;
}

Common::Error JimmyDialogueHandler::runSneakersOfferMenu(DialogueRuntime &runtime) {
	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(kJimmyThirdResponseLineIndex, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		if (runtime.startupScript().isObjectInInventory("SNEAKERS")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x4a9e, "JIMMY", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			completeSneakersTrade(runtime);
			return Common::kNoError;
		}

		return runtime.playDialogueEntrySequence(kJimmyNoSneakersLines, ARRAYSIZE(kJimmyNoSneakersLines));
	}
	if (responseIndex == 2)
		return runtime.playDialogueLineWithVariant(0x4ab4, "JIMMY", 1);

	return Common::kNoError;
}

Common::Error JimmyDialogueHandler::runJimmySneakersPrompt(DialogueRuntime &runtime, int responseLineIndex) {
	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(responseLineIndex, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x4a86, "JIMMY", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueEntrySequence(kJimmySecondResponsePreludeLines,
			ARRAYSIZE(kJimmySecondResponsePreludeLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		return runSneakersOfferMenu(runtime);
	}
	if (responseIndex == 2)
		return runtime.playDialogueLineWithVariant(0x4abc, "JIMMY", 0);

	return Common::kNoError;
}

Common::Error JimmyDialogueHandler::playJimmyNoItemFallback(DialogueRuntime &runtime) {
	if (runtime.startupScript().isObjectInInventory("SNEAKERS") &&
			!runtime.startupScript().isObjectInInventory("BROOMKEY")) {
		Common::Error sneakersLineError = runtime.playDialogueLineWithVariant(0x4ac3, "JIMMY", 0);
		if (sneakersLineError.getCode() != Common::kNoError)
			return sneakersLineError;

		Common::Error sneakersPromptError = runJimmySneakersPrompt(runtime, kJimmySneakersResponseLineIndex);
		if (sneakersPromptError.getCode() != Common::kNoError)
			return sneakersPromptError;
	}
	if (runtime.startupScript().getFlagValue("PAPER_CHK_4"))
		return runtime.playDialogueLineWithVariant(0x4ae2, "JIMMY", 2);
	if (runtime.startupScript().getFlagValue("PAPER_CHK_3"))
		return runtime.playDialogueLineWithVariant(0x4adb, "JIMMY", 2);
	if (runtime.startupScript().getFlagValue("PAPER_CHK_2"))
		return runtime.playDialogueLineWithVariant(0x4ad4, "JIMMY", 2);

	return runtime.playDialogueLineWithVariant(0x4b38, "JIMMY", 0);
}

Common::Error JimmyDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	JimmyRoomDialogueState &state = _state;
	Common::Error lineError = Common::kNoError;

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("NEWSPAPER")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kGivenPaperTodayFlag, true);
			restoreItemToRah(runtime, "NEWSPAPER");
			runtime.executeAndQueueAction("ACTV_HOUSE_EXIT");

			lineError = playPaperHandoffPreludeIfNeeded(runtime);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			const bool playJimmyRevisitBark = runtime.getRandomNumber(1) == 0;
			lineError = runtime.playDialogueLineWithVariant(playJimmyRevisitBark ? 0x4acc : 0x4a4b,
				"JIMMY", playJimmyRevisitBark ? 0 : 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			state.firstNoItemLinePending = false;
			(void)runtime.startupScript().setRuntimeFlagValue(kJimmyFirstMeetingFlag, true);
			(void)runtime.startupScript().setRuntimeFlagValue(kGivenPaperTodayFlag, true);
			return Common::kNoError;
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			return runtime.playDialogueLineWithVariant(0x4af8, "JIMMY", 1);
		}
		if (((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
					usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
				sharedState.discussedCasketPhotoEvidence = 1;
			} else {
				sharedState.discussedLedgerEvidence = 1;
			}
			return runtime.playDialogueEntrySequence(kJimmyLedgersAndCorpseEvidenceLines,
				ARRAYSIZE(kJimmyLedgersAndCorpseEvidenceLines));
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			return runtime.playDialogueEntrySequence(kJimmyBlackmailEvidenceLines, ARRAYSIZE(kJimmyBlackmailEvidenceLines));
		}
		if (usedItemName.equalsIgnoreCase("SNEAKERS")) {
			lineError = runtime.playDialogueLineWithVariant(0x4a9e, "JIMMY", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			completeSneakersTrade(runtime);
			return Common::kNoError;
		}

		return runtime.playDialogueLineWithVariant(0x4af2, "JIMMY", 0);
	}

	if (state.firstNoItemLinePending) {
		state.firstNoItemLinePending = false;
		lineError = playPaperHandoffPreludeIfNeeded(runtime);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLineWithVariant(0x4a58, "JIMMY", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue(kJimmyFirstMeetingFlag, true);
		(void)runtime.startupScript().setRuntimeFlagValue(kGivenPaperTodayFlag, true);

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(kJimmyFirstResponseLineIndex, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x4a63, "JIMMY", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x4a67, "JIMMY", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = runtime.playDialogueEntrySequence(kJimmyFirstMeetingFollowupLines,
			ARRAYSIZE(kJimmyFirstMeetingFollowupLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runJimmySneakersPrompt(runtime, kJimmySecondResponseLineIndex);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return playJimmyNoItemFallback(runtime);
}

} // End of namespace Harvester
