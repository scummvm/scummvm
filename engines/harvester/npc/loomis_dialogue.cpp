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


#include "harvester/npc/loomis_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const int kLoomisSheriffTopicLineIndices[] = { 0x5f, 0x60, 0x61 };
static const char *const kLoomisTopicAlias = "LOOMIS";
static const char *const kPhelpsTopicAlias = "Phelps";
static const char *const kMensNeedsTopicAlias = "Man's Needs";
static const char *const kPcFryDaddyActionTag = "PC_FRY_DADDY_1";
static const char *const kDialogueC048FstPath = "GRAPHIC/FST/C048.FST";

static const DialogueLineEntry kLoomisIntroOpeningLines[] = {
	{ 0x10fd, "LOOMIS", 0 },
	{ 0x1101, "PC", 0 },
	{ 0x1105, "LOOMIS", 0 },
	{ 0x110a, "PC", 0 },
	{ 0x110e, "LOOMIS", 0 }
};

static const DialogueLineEntry kLoomisReturnVisitFollowupLines[] = {
	{ 0x11a8, "LOOMIS", 1 },
	{ 0x11ac, "PC", 0 },
	{ 0x11b0, "LOOMIS", 3 }
};

static const DialogueLineEntry kLoomisMrsLoomisLines[] = {
	{ 0x114f, "LOOMIS", 1 },
	{ 0x1150, "LOOMIS", 2 },
	{ 0x1151, "LOOMIS", 2 },
	{ 0x1152, "LOOMIS", 2 },
	{ 0x1153, "LOOMIS", 3 }
};

static const DialogueLineEntry kLoomisMrsPhelpsLines[] = {
	{ 0x1161, "LOOMIS", 0 },
	{ 0x1162, "LOOMIS", 3 },
	{ 0x1163, "LOOMIS", 3 }
};

} // End of namespace

Common::Error LoomisDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	LoomisRoomDialogueState &state = _state;
	Common::String &loomisTopicBuffer = state.currentTopicBuffer;
	int &loomisTopicBufferLineIndex = state.currentTopicBufferLineIndex;

	DialogueTopicBuffer topics(runtime, loomisTopicBuffer, loomisTopicBufferLineIndex, "Loomis topic buffer");

	if (usedItemName.empty()) {
		Common::Error lineError = Common::kNoError;
		if (state.talkStatePending) {
			state.talkStatePending = false;
			state.returnVisitFollowupPending = true;

			if (!runtime.startupScript().getFlagValue("DWAYNE_INTRODUCED")) {
				lineError = runtime.playDialogueEntrySequence(
					kLoomisIntroOpeningLines, ARRAYSIZE(kLoomisIntroOpeningLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				sharedState.dialogueStateD2f04 = true;
				if (runtime.startupScript().getFlagValue("SHERIFF_IN_DINER")) {
					lineError = runtime.playDialogueLineWithVariant(0x1112, "PC", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = runtime.playDialogueLineWithVariant(0x1118, "LOOMIS", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			}

			lineError = runtime.playDialogueLineWithVariant(0x1121, "LOOMIS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int ignoredResponseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x53, ignoredResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			lineError = runtime.playDialogueLineWithVariant(0x1130, "LOOMIS", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (state.returnVisitFollowupPending) {
			state.returnVisitFollowupPending = false;
			lineError = runtime.playDialogueEntrySequence(
				kLoomisReturnVisitFollowupLines, ARRAYSIZE(kLoomisReturnVisitFollowupLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else {
			lineError = runtime.playDialogueLineWithVariant(0x11f6, "LOOMIS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
				!state.stephMidgameShown) {
			state.stephMidgameShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x1230, "LOOMIS", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getCurrentStoryDayIndex() == 5 &&
				!state.dayFiveShown) {
			state.dayFiveShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x123c, "LOOMIS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		topics.assign(0x54);
		for (;;) {
			Common::String selectedTopic;
			Common::Error menuError = runtime.runKeywordMenu(
				loomisTopicBuffer, loomisTopicBufferLineIndex, selectedTopic);
			if (menuError.getCode() != Common::kNoError)
				return menuError;
			if (selectedTopic.empty())
				return Common::kNoError;

			if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()) ||
					runtime.matchesResponseLine(selectedTopic, 0x56)) {
				return runtime.playDialogueLineWithVariant(0x11a1, "LOOMIS", 1);
			}

			if (runtime.matchesResponseLine(selectedTopic, 0x57)) {
				lineError = runtime.playDialogueLineWithVariant(0x1143, "LOOMIS", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0x58);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x59) ||
					selectedTopic.equalsIgnoreCase(kLoomisTopicAlias)) {
				lineError = runtime.playDialogueEntrySequence(
					kLoomisMrsLoomisLines, ARRAYSIZE(kLoomisMrsLoomisLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0x5a);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x5b) ||
					selectedTopic.equalsIgnoreCase(kPhelpsTopicAlias)) {
				lineError = runtime.playDialogueEntrySequence(
					kLoomisMrsPhelpsLines, ARRAYSIZE(kLoomisMrsPhelpsLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0x5c);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x5d) ||
					selectedTopic.equalsIgnoreCase(kMensNeedsTopicAlias)) {
				lineError = runtime.playDialogueLineWithVariant(0x1170, "LOOMIS", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0x5e);
				continue;
			}
			if (runtime.matchesAnyResponseLine(selectedTopic,
					kLoomisSheriffTopicLineIndices, ARRAYSIZE(kLoomisSheriffTopicLineIndices))) {
				lineError = runtime.playDialogueLineWithVariant(0x117e, "LOOMIS", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0x62);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x63)) {
				lineError = runtime.playDialogueLineWithVariant(0x118a, "LOOMIS", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				topics.assign(0x64);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x65)) {
				lineError = runtime.playDialogueLineWithVariant(0x1196, "LOOMIS", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x66))
				continue;

			lineError = runtime.playDialogueLineWithVariant(0x11fc, "LOOMIS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("TV_DEED") ||
			usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		sharedState.discussedNoteCheckbookEvidence = 1;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x1209, "LOOMIS", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x50, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x1214, "LOOMIS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x1219, "LOOMIS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = runtime.playDialogueLineWithVariant(0x121f, "LOOMIS", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		runtime.executeAndQueueAction(kPcFryDaddyActionTag);
		return Common::kNoError;
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		sharedState.discussedLedgerEvidence = 1;
		return runtime.playDialogueLineWithVariant(0x1229, "LOOMIS", 0);
	}
	if (usedItemName.equalsIgnoreCase("INV_MAG")) {
		if (!runtime.startupScript().getFlagValue("SHERIFF_IN_DINER")) {
			(void)runtime.startupScript().addRuntimeObjectToInventory("INV_MAG");
			return runtime.playDialogueLineWithVariant(0x11db, "LOOMIS", 1);
		}

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x11bb, "LOOMIS", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x51, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x11c6, "LOOMIS", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			bool changed = false;
			changed |= runtime.startupScript().setRuntimeFlagValue("GAVE_MAG_TO_LOOMIS_TODAY", true);
			changed |= runtime.startupScript().setRuntimeObjectVisible("INVENTORY", "INV_MAG", false);
			changed |= runtime.startupScript().setRuntimeObjectVisible("SHRFOFC", "SHERIF_DRAWR", true);
			changed |= runtime.startupScript().setRuntimeObjectVisible("SHRFOFC", "SHERIF_DRAWR2", false);
			changed |= runtime.startupScript().setRuntimeNpcState("LOOMIS", false, false);

			lineError = runtime.playDialogueFst(kDialogueC048FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			InteractionResult interaction;
			interaction.mutatedRuntimeState = changed;
			interaction.visualRuntimeStateChanged = changed;
			runtime.queueDialogueInteractionIfNeeded(interaction);
			return Common::kNoError;
		}

		(void)runtime.startupScript().addRuntimeObjectToInventory("INV_MAG");
		if (responseIndex == 2)
			return runtime.playDialogueLineWithVariant(0x11cc, "LOOMIS", 3);
		return Common::kNoError;
	}
	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL"))
		return runtime.playDialogueLineWithVariant(0x1143, "LOOMIS", 1);

	return runtime.playDialogueLineWithVariant(0x1202, "LOOMIS", 0);
}

} // End of namespace Harvester
