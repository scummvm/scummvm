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

static const char *const kJimmyNpc = "JIMMY";
static const char *const kPcSpeaker = "PC";
static const char *const kRahRoomName = "RAH";
static const char *const kJimmyFirstMeetingFlag = "PAPER_CHK_1";
static const char *const kGivenPaperTodayFlag = "GIVEN_PAPER_TODAY";
static const int kJimmyFirstResponseLineIndex = 0xf5;
static const int kJimmySecondResponseLineIndex = 0xf6;
static const int kJimmyThirdResponseLineIndex = 0xf7;
static const int kJimmySneakersResponseLineIndex = 0xf8;

static const DialogueLineEntry kJimmyPaperHandoffLines[] = {
	{ 0x4a4c, kJimmyNpc, 1 },
	{ 0x4a52, kPcSpeaker, 0 }
};

static const DialogueLineEntry kJimmyFirstMeetingFollowupLines[] = {
	{ 0x4a6b, kJimmyNpc, 0 },
	{ 0x4a70, kPcSpeaker, 0 },
	{ 0x4a74, kJimmyNpc, 0 },
	{ 0x4a7a, kJimmyNpc, 0 }
};

static const DialogueLineEntry kJimmySecondResponsePreludeLines[] = {
	{ 0x4a8a, kPcSpeaker, 0 },
	{ 0x4a8e, kJimmyNpc, 1 }
};

static const DialogueLineEntry kJimmyNoSneakersLines[] = {
	{ 0x4aa6, kJimmyNpc, 0 },
	{ 0x4aaa, kPcSpeaker, 0 },
	{ 0x4aae, kJimmyNpc, 2 }
};

static const DialogueLineEntry kJimmyLedgersAndCorpseEvidenceLines[] = {
	{ 0x4b00, kJimmyNpc, 0 },
	{ 0x4b04, kPcSpeaker, 0 },
	{ 0x4b08, kJimmyNpc, 0 },
	{ 0x4b0c, kPcSpeaker, 0 },
	{ 0x4b10, kJimmyNpc, 0 },
	{ 0x4b15, kPcSpeaker, 0 },
	{ 0x4b19, kJimmyNpc, 0 }
};

static const DialogueLineEntry kJimmyBlackmailEvidenceLines[] = {
	{ 0x4b21, kJimmyNpc, 0 },
	{ 0x4b25, kPcSpeaker, 0 },
	{ 0x4b29, kJimmyNpc, 0 },
	{ 0x4b2d, kPcSpeaker, 0 },
	{ 0x4b31, kJimmyNpc, 1 }
};

} // End of namespace

bool JimmyDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("JIMMY");
}

Common::Error JimmyDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	JimmyRoomDialogueState &state = _state;
	Common::Error lineError = Common::kNoError;

	auto playJimmyLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kJimmyNpc, headVariant);
	};
	auto playJimmySequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		return runtime.startupScript().isObjectInInventory(objectName);
	};
	auto executeActionTagIfSet = [&](const char *tag) {
		InteractionResult interaction;
		if (!runtime.executeActionTag(tag, interaction))
			return;

		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};
	auto restoreItemToRah = [&](const char *objectName) {
		(void)runtime.startupScript().resetRuntimeObjectToInitialState(objectName);
		(void)runtime.startupScript().setRuntimeObjectVisible(kRahRoomName, objectName, true);
	};
	auto completeSneakersTrade = [&]() {
		restoreItemToRah("SNEAKERS");
		(void)runtime.startupScript().addRuntimeObjectToInventory("BROOMKEY");
	};
	auto playPaperHandoffPreludeIfNeeded = [&]() -> Common::Error {
		if (state.paperHandoffStateSet)
			return Common::kNoError;

		Common::Error preludeError = playJimmySequence(
			kJimmyPaperHandoffLines, ARRAYSIZE(kJimmyPaperHandoffLines));
		if (preludeError.getCode() != Common::kNoError)
			return preludeError;

		state.paperHandoffStateSet = true;
		return Common::kNoError;
	};
	auto runSneakersOfferMenu = [&]() -> Common::Error {
		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(kJimmyThirdResponseLineIndex, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			if (hasInventoryItem("SNEAKERS")) {
				Common::Error lineError = playJimmyLine(0x4a9e, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				completeSneakersTrade();
				return Common::kNoError;
			}

			return playJimmySequence(kJimmyNoSneakersLines, ARRAYSIZE(kJimmyNoSneakersLines));
		}
		if (responseIndex == 2)
			return playJimmyLine(0x4ab4, 1);

		return Common::kNoError;
	};
	auto runJimmySneakersPrompt = [&](int responseLineIndex) -> Common::Error {
		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(responseLineIndex, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			Common::Error lineError = playJimmyLine(0x4a86, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = playJimmySequence(kJimmySecondResponsePreludeLines,
				ARRAYSIZE(kJimmySecondResponsePreludeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			return runSneakersOfferMenu();
		}
		if (responseIndex == 2)
			return playJimmyLine(0x4abc, 0);

		return Common::kNoError;
	};
	auto playJimmyNoItemFallback = [&]() -> Common::Error {
		if (hasInventoryItem("SNEAKERS") && !hasInventoryItem("BROOMKEY")) {
			Common::Error sneakersLineError = playJimmyLine(0x4ac3, 0);
			if (sneakersLineError.getCode() != Common::kNoError)
				return sneakersLineError;

			Common::Error sneakersPromptError = runJimmySneakersPrompt(kJimmySneakersResponseLineIndex);
			if (sneakersPromptError.getCode() != Common::kNoError)
				return sneakersPromptError;
		}
		if (runtime.startupScript().getFlagValue("PAPER_CHK_4"))
			return playJimmyLine(0x4ae2, 2);
		if (runtime.startupScript().getFlagValue("PAPER_CHK_3"))
			return playJimmyLine(0x4adb, 2);
		if (runtime.startupScript().getFlagValue("PAPER_CHK_2"))
			return playJimmyLine(0x4ad4, 2);

		return playJimmyLine(0x4b38, 0);
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("NEWSPAPER")) {
			(void)runtime.startupScript().setRuntimeFlagValue(kGivenPaperTodayFlag, true);
			restoreItemToRah("NEWSPAPER");
			executeActionTagIfSet("ACTV_HOUSE_EXIT");

			lineError = playPaperHandoffPreludeIfNeeded();
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			const bool playJimmyRevisitBark = runtime.getRandomNumber(1) == 0;
			lineError = playJimmyLine(
				playJimmyRevisitBark ? 0x4acc : 0x4a4b,
				playJimmyRevisitBark ? 0 : 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			state.firstNoItemLinePending = false;
			(void)runtime.startupScript().setRuntimeFlagValue(kJimmyFirstMeetingFlag, true);
			(void)runtime.startupScript().setRuntimeFlagValue(kGivenPaperTodayFlag, true);
			return Common::kNoError;
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			return playJimmyLine(0x4af8, 1);
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
			return playJimmySequence(kJimmyLedgersAndCorpseEvidenceLines,
				ARRAYSIZE(kJimmyLedgersAndCorpseEvidenceLines));
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			return playJimmySequence(kJimmyBlackmailEvidenceLines,
				ARRAYSIZE(kJimmyBlackmailEvidenceLines));
		}
		if (usedItemName.equalsIgnoreCase("SNEAKERS")) {
			lineError = playJimmyLine(0x4a9e, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			completeSneakersTrade();
			return Common::kNoError;
		}

		return playJimmyLine(0x4af2, 0);
	}

	if (state.firstNoItemLinePending) {
		state.firstNoItemLinePending = false;
		lineError = playPaperHandoffPreludeIfNeeded();
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = playJimmyLine(0x4a58, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue(kJimmyFirstMeetingFlag, true);
		(void)runtime.startupScript().setRuntimeFlagValue(kGivenPaperTodayFlag, true);

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(kJimmyFirstResponseLineIndex, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playJimmyLine(0x4a63, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playJimmyLine(0x4a67, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = playJimmySequence(kJimmyFirstMeetingFollowupLines, ARRAYSIZE(kJimmyFirstMeetingFollowupLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runJimmySneakersPrompt(kJimmySecondResponseLineIndex);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return playJimmyNoItemFallback();
}

} // End of namespace Harvester
