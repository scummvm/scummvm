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
static const char *const kLoomisNpc = "LOOMIS";
static const char *const kPcSpeaker = "PC";
static const char *const kLoomisTopicAlias = "LOOMIS";
static const char *const kPhelpsTopicAlias = "Phelps";
static const char *const kMensNeedsTopicAlias = "Man's Needs";
static const char *const kPcFryDaddyActionTag = "PC_FRY_DADDY_1";
static const char *const kDialogueC048FstPath = "GRAPHIC/FST/C048.FST";

static const DialogueLineEntry kLoomisIntroOpeningLines[] = {
	{ 0x10fd, kLoomisNpc, 0 },
	{ 0x1101, kPcSpeaker, 0 },
	{ 0x1105, kLoomisNpc, 0 },
	{ 0x110a, kPcSpeaker, 0 },
	{ 0x110e, kLoomisNpc, 0 }
};

static const DialogueLineEntry kLoomisReturnVisitFollowupLines[] = {
	{ 0x11a8, kLoomisNpc, 1 },
	{ 0x11ac, kPcSpeaker, 0 },
	{ 0x11b0, kLoomisNpc, 3 }
};

static const DialogueLineEntry kLoomisMrsLoomisLines[] = {
	{ 0x114f, kLoomisNpc, 1 },
	{ 0x1150, kLoomisNpc, 2 },
	{ 0x1151, kLoomisNpc, 2 },
	{ 0x1152, kLoomisNpc, 2 },
	{ 0x1153, kLoomisNpc, 3 }
};

static const DialogueLineEntry kLoomisMrsPhelpsLines[] = {
	{ 0x1161, kLoomisNpc, 0 },
	{ 0x1162, kLoomisNpc, 3 },
	{ 0x1163, kLoomisNpc, 3 }
};

} // End of namespace

bool LoomisDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("LOOMIS");
}

Common::Error LoomisDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	LoomisRoomDialogueState &state = _state;
	Common::String &loomisTopicBuffer = state.currentTopicBuffer;
	int &loomisTopicBufferLineIndex = state.currentTopicBufferLineIndex;

	auto assignLoomisTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(loomisTopicBuffer, loomisTopicBufferLineIndex,
			responseLineIndex, "Loomis topic buffer");
	};
	auto playLoomisLine = [&](int wavId, const char *speakerId = kLoomisNpc,
			int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, speakerId, headVariant);
	};
	auto executeLoomisActionTag = [&](const char *tag) {
		InteractionResult interaction;
		if (!runtime.executeActionTag(tag, interaction))
			return;

		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};

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
					lineError = playLoomisLine(0x1112, kPcSpeaker);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = playLoomisLine(0x1118);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			}

			lineError = playLoomisLine(0x1121);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int ignoredResponseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x53, ignoredResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			lineError = playLoomisLine(0x1130, kLoomisNpc, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (state.returnVisitFollowupPending) {
			state.returnVisitFollowupPending = false;
			lineError = runtime.playDialogueEntrySequence(
				kLoomisReturnVisitFollowupLines, ARRAYSIZE(kLoomisReturnVisitFollowupLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else {
			lineError = playLoomisLine(0x11f6);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
				!state.stephMidgameShown) {
			state.stephMidgameShown = true;
			lineError = playLoomisLine(0x1230, kLoomisNpc, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getCurrentStoryDayIndex() == 5 &&
				!state.dayFiveShown) {
			state.dayFiveShown = true;
			lineError = playLoomisLine(0x123c);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		assignLoomisTopicBuffer(0x54);
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
				return playLoomisLine(0x11a1, kLoomisNpc, 1);
			}

			if (runtime.matchesResponseLine(selectedTopic, 0x57)) {
				lineError = playLoomisLine(0x1143, kLoomisNpc, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignLoomisTopicBuffer(0x58);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x59) ||
					selectedTopic.equalsIgnoreCase(kLoomisTopicAlias)) {
				lineError = runtime.playDialogueEntrySequence(
					kLoomisMrsLoomisLines, ARRAYSIZE(kLoomisMrsLoomisLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignLoomisTopicBuffer(0x5a);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x5b) ||
					selectedTopic.equalsIgnoreCase(kPhelpsTopicAlias)) {
				lineError = runtime.playDialogueEntrySequence(
					kLoomisMrsPhelpsLines, ARRAYSIZE(kLoomisMrsPhelpsLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignLoomisTopicBuffer(0x5c);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x5d) ||
					selectedTopic.equalsIgnoreCase(kMensNeedsTopicAlias)) {
				lineError = playLoomisLine(0x1170, kLoomisNpc, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignLoomisTopicBuffer(0x5e);
				continue;
			}
			if (runtime.matchesAnyResponseLine(selectedTopic,
					kLoomisSheriffTopicLineIndices, ARRAYSIZE(kLoomisSheriffTopicLineIndices))) {
				lineError = playLoomisLine(0x117e);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignLoomisTopicBuffer(0x62);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x63)) {
				lineError = playLoomisLine(0x118a);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignLoomisTopicBuffer(0x64);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x65)) {
				lineError = playLoomisLine(0x1196, kLoomisNpc, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x66))
				continue;

			lineError = playLoomisLine(0x11fc);
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
		Common::Error lineError = playLoomisLine(0x1209);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x50, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playLoomisLine(0x1214);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playLoomisLine(0x1219);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = playLoomisLine(0x121f, kLoomisNpc, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		executeLoomisActionTag(kPcFryDaddyActionTag);
		return Common::kNoError;
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		sharedState.discussedLedgerEvidence = 1;
		return playLoomisLine(0x1229);
	}
	if (usedItemName.equalsIgnoreCase("INV_MAG")) {
		if (!runtime.startupScript().getFlagValue("SHERIFF_IN_DINER")) {
			(void)runtime.startupScript().addRuntimeObjectToInventory("INV_MAG");
			return playLoomisLine(0x11db, kLoomisNpc, 1);
		}

		Common::Error lineError = playLoomisLine(0x11bb);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x51, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playLoomisLine(0x11c6, kLoomisNpc, 1);
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
			return playLoomisLine(0x11cc, kLoomisNpc, 3);
		return Common::kNoError;
	}
	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL"))
		return playLoomisLine(0x1143, kLoomisNpc, 1);

	return playLoomisLine(0x1202);
}

} // End of namespace Harvester
