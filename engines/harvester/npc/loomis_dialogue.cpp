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

#include "harvester/npc/loomis_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const int kLoomisSheriffTopicLineIndices[] = { 0x5f, 0x60, 0x61 };
static const char *const kDialogueC048FstPath = "GRAPHIC/FST/C048.FST";

} // End of namespace

bool LoomisDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("LOOMIS");
}

Common::Error LoomisDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	LoomisRoomDialogueState &state = _state;
	Common::String &loomisTopicBuffer = state.currentTopicBuffer;
	int &loomisTopicBufferLineIndex = state.currentTopicBufferLineIndex;

	auto assignLoomisTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(loomisTopicBuffer, loomisTopicBufferLineIndex,
			responseLineIndex, "Loomis topic buffer");
	};
	auto playLoomisLine = [&](int wavId, const char *speakerId = "LOOMIS") -> Common::Error {
		return runtime.playDialogueLine(wavId, speakerId);
	};
	auto clearLoomisTopicBuffer = [&]() {
		loomisTopicBuffer.clear();
		loomisTopicBufferLineIndex = -1;
	};

	if (usedItemName.empty()) {
		if (!state.talkStatePending) {
			if (state.returnVisitFollowupPending) {
				state.returnVisitFollowupPending = false;
				const DialogueLineEntry followupLines[] = {
					{ 0x11a8, "LOOMIS", 0 },
					{ 0x11ac, "PC", 0 },
					{ 0x11b0, "LOOMIS", 0 }
				};
				return runtime.playDialogueEntrySequence(followupLines, ARRAYSIZE(followupLines));
			}
			return runtime.playDialogueLine(0x11f6, "LOOMIS");
		}

		state.talkStatePending = false;
		if (!runtime.startupScript().getFlagValue("DWAYNE_INTRODUCED")) {
			const DialogueLineEntry introLines[] = {
				{ 0x10fd, "LOOMIS", 0 },
				{ 0x1101, "PC", 0 },
				{ 0x1105, "LOOMIS", 0 },
				{ 0x110a, "PC", 0 },
				{ 0x110e, "LOOMIS", 0 },
				{ 0x1112, "PC", 0 },
				{ runtime.startupScript().getFlagValue("SHERIFF_IN_DINER") ? 0x1118 : 0x111e, "LOOMIS", 0 },
				{ 0x1121, "LOOMIS", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else {
			Common::Error lineError = runtime.playDialogueLine(0x1121, "LOOMIS");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		Common::Error lineError = playLoomisLine(0x1130);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

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
					runtime.matchesResponseLine(selectedTopic, 0x56) ||
					runtime.matchesResponseLine(selectedTopic, 0x66)) {
				clearLoomisTopicBuffer();
				state.returnVisitFollowupPending = true;
				return playLoomisLine(0x113a);
			}

			if (runtime.matchesResponseLine(selectedTopic, 0x57)) {
				lineError = playLoomisLine(0x1143);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignLoomisTopicBuffer(0x58);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x59)) {
				const DialogueLineEntry mrsLoomisLines[] = {
					{ 0x114f, "LOOMIS", 0 },
					{ 0x1150, "LOOMIS", 0 },
					{ 0x1151, "LOOMIS", 0 },
					{ 0x1152, "LOOMIS", 0 },
					{ 0x1153, "LOOMIS", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(mrsLoomisLines, ARRAYSIZE(mrsLoomisLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignLoomisTopicBuffer(0x5a);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x5b)) {
				const DialogueLineEntry mrsPhelpsLines[] = {
					{ 0x1161, "LOOMIS", 0 },
					{ 0x1162, "LOOMIS", 0 },
					{ 0x1163, "LOOMIS", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(mrsPhelpsLines, ARRAYSIZE(mrsPhelpsLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignLoomisTopicBuffer(0x5c);
				continue;
			}
			if (runtime.matchesResponseLine(selectedTopic, 0x5d)) {
				lineError = playLoomisLine(0x1170);
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
				lineError = playLoomisLine(0x1196);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
		}
	}

	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("TV_DEED") ||
			usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		return runtime.playDialogueLine(0x1209, "LOOMIS");
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		return runtime.playDialogueLine(0x1229, "LOOMIS");
	}
	if (usedItemName.equalsIgnoreCase("INV_MAG")) {
		if (!runtime.startupScript().getFlagValue("SHERIFF_IN_DINER")) {
			(void)runtime.startupScript().addRuntimeObjectToInventory("INV_MAG");
			return runtime.playDialogueLine(0x11db, "LOOMIS");
		}

		Common::Error lineError = runtime.playDialogueLine(0x11bb, "LOOMIS");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x51, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			bool changed = runtime.startupScript().setRuntimeObjectVisible("INVENTORY", "INV_MAG", false);
			if (runtime.currentRoomName().equalsIgnoreCase("SHRFOFC")) {
				changed |= runtime.startupScript().setRuntimeFlagValue("GAVE_MAG_TO_LOOMIS_TODAY", true);
				changed |= runtime.startupScript().setRuntimeNpcState("LOOMIS", false, false);
				changed |= runtime.startupScript().setRuntimeObjectVisible("SHRFOFC", "SHERIF_DRAWR", true);
				changed |= runtime.startupScript().setRuntimeObjectVisible("SHRFOFC", "SHERIF_DRAWR2", false);
			}
			lineError = runtime.playDialogueLine(0x11c6, "LOOMIS");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC048FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			StartupInteractionResult interaction;
			interaction.mutatedRuntimeState = changed;
			runtime.queueDialogueInteractionIfNeeded(interaction);
			return Common::kNoError;
		}

		(void)runtime.startupScript().addRuntimeObjectToInventory("INV_MAG");
		return runtime.playDialogueLine(0x11cc, "LOOMIS");
	}
	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL"))
		return runtime.playDialogueLine(0x1143, "LOOMIS");

	return runtime.playDialogueLine(0x1202, "LOOMIS");
}

} // End of namespace Harvester
