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

#include "harvester/npc/jimmy_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kJimmyNpc = "JIMMY";
static const char *const kPcSpeaker = "PC";
static const char *const kRahRoomName = "RAH";
static const char *const kJimmyFirstMeetingFlag = "PAPER_CHK_1";
static const int kJimmyFirstResponseLineIndex = 0xf5;
static const int kJimmySecondResponseLineIndex = 0xf6;
static const int kJimmyThirdResponseLineIndex = 0xf7;

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

} // End of namespace

bool JimmyDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("JIMMY");
}

Common::Error JimmyDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	JimmyRoomDialogueState &state = _state;
	Common::Error lineError = Common::kNoError;

	auto playJimmyLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kJimmyNpc, headVariant);
	};
	auto playJimmySequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
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
	auto playJimmyNoItemFallback = [&]() -> Common::Error {
		if (hasInventoryItem("SNEAKERS") && !hasInventoryItem("BROOMKEY"))
			return playJimmyLine(0x4ac3, 0);
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
			StartupInteractionResult jimmyInteraction;
			const bool changedGivenPaperToday =
				runtime.startupScript().setRuntimeFlagValue("GIVEN_PAPER_TODAY", true);
			const bool changedNewspaperState =
				runtime.startupScript().resetRuntimeObjectToInitialState("NEWSPAPER");
			jimmyInteraction.mutatedRuntimeState = changedGivenPaperToday || changedNewspaperState;

			StartupInteractionResult actionInteraction;
			if (runtime.startupScript().executeNestedActionTag("ACTV_HOUSE_EXIT", actionInteraction)) {
				jimmyInteraction.abortRemainingCommandChain =
					jimmyInteraction.abortRemainingCommandChain ||
					actionInteraction.abortRemainingCommandChain;
				jimmyInteraction.mutatedRuntimeState =
					jimmyInteraction.mutatedRuntimeState || actionInteraction.mutatedRuntimeState;
				if (!actionInteraction.musicPath.empty())
					jimmyInteraction.musicPath = actionInteraction.musicPath;
				if (!actionInteraction.nextRoomName.empty())
					jimmyInteraction.nextRoomName = actionInteraction.nextRoomName;
				if (actionInteraction.roomTransition != kStartupRoomTransitionNone)
					jimmyInteraction.roomTransition = actionInteraction.roomTransition;
				if (!actionInteraction.deathFlicPath.empty())
					jimmyInteraction.deathFlicPath = actionInteraction.deathFlicPath;
				if (!actionInteraction.dialogueNpcName.empty())
					jimmyInteraction.dialogueNpcName = actionInteraction.dialogueNpcName;
				if (!actionInteraction.dialogueContinuationTag.empty())
					jimmyInteraction.dialogueContinuationTag = actionInteraction.dialogueContinuationTag;
				jimmyInteraction.requestMainMenu =
					jimmyInteraction.requestMainMenu || actionInteraction.requestMainMenu;
				for (const StartupAudioCommand &command : actionInteraction.audioCommands)
					jimmyInteraction.audioCommands.push_back(command);
			}
			runtime.queueDialogueInteractionIfNeeded(jimmyInteraction);

			if (!state.paperHandoffStateSet) {
				state.paperHandoffStateSet = true;
				lineError = playJimmyLine(0x4a4c, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			return runtime.getRandomNumber(1) == 0
				? playJimmyLine(0x4acc, 0)
				: playJimmyLine(0x4a4b, 1);
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playJimmyLine(0x4af8, 1);
		}
		if (((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
					usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
				(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			} else {
				(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			}
			return playJimmyLine(0x4b00, 0);
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playJimmyLine(0x4b21, 0);
		}
		if (usedItemName.equalsIgnoreCase("SNEAKERS")) {
			lineError = playJimmyLine(0x4a9e, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return playJimmyLine(0x4af2, 0);
	}

	if (state.firstNoItemLinePending) {
		state.firstNoItemLinePending = false;
		if (!state.paperHandoffStateSet) {
			lineError = playJimmyLine(0x4a4c, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = playJimmyLine(0x4a58, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue(kJimmyFirstMeetingFlag, true);
		(void)runtime.startupScript().setRuntimeFlagValue("GIVEN_PAPER_TODAY", true);

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

		responseIndex = 0;
		responseError = runtime.runResponseMenu(kJimmySecondResponseLineIndex, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playJimmyLine(0x4a86, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = playJimmySequence(kJimmySecondResponsePreludeLines,
				ARRAYSIZE(kJimmySecondResponsePreludeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int thirdResponseIndex = 0;
			responseError = runtime.runResponseMenu(kJimmyThirdResponseLineIndex, thirdResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (thirdResponseIndex == 1) {
				if (hasInventoryItem("SNEAKERS")) {
					lineError = playJimmyLine(0x4a9e, 1);
					if (lineError.getCode() != Common::kNoError)
						return lineError;

					(void)runtime.startupScript().resetRuntimeObjectToInitialState("SNEAKERS");
					(void)runtime.startupScript().setRuntimeObjectVisible(kRahRoomName, "SNEAKERS", true);
					(void)runtime.startupScript().addRuntimeObjectToInventory("BROOMKEY");
				} else {
					lineError = playJimmySequence(kJimmyNoSneakersLines, ARRAYSIZE(kJimmyNoSneakersLines));
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			} else if (thirdResponseIndex == 2) {
				lineError = playJimmyLine(0x4ab4, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		} else if (responseIndex == 2) {
			lineError = playJimmyLine(0x4abc, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	return playJimmyNoItemFallback();
}

} // End of namespace Harvester
