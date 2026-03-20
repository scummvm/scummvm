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

bool LoomisDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("LOOMIS");
}

Common::Error LoomisDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	LoomisRoomDialogueState &state = _state;

	if (usedItemName.empty()) {
		if (!state.talkStatePending) {
			if (state.returnVisitFollowupPending) {
				state.returnVisitFollowupPending = false;
				return runtime.playDialogueLine(0x11a8, "LOOMIS");
			}
			return runtime.playDialogueLine(0x11f6, "LOOMIS");
		}

		state.talkStatePending = false;
		state.returnVisitFollowupPending = true;
		if (!runtime.startupScript().getFlagValue("DWAYNE_INTRODUCED")) {
			Common::Error lineError = runtime.playDialogueLine(0x10fd, "LOOMIS");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		return runtime.playDialogueLine(0x1121, "LOOMIS");
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
		if (runtime.currentRoomName().equalsIgnoreCase("SHRFOFC")) {
			StartupInteractionResult interaction;
			const bool changedFlag = runtime.startupScript().setRuntimeFlagValue(
				"GAVE_MAG_TO_LOOMIS_TODAY", true);
			const bool changedNpc = runtime.startupScript().setRuntimeNpcState("LOOMIS", false, false);
			interaction.mutatedRuntimeState = changedFlag || changedNpc;
			runtime.queueDialogueInteractionIfNeeded(interaction);
		}

		if (runtime.startupScript().getFlagValue("SHERIFF_IN_DINER"))
			return runtime.playDialogueLine(0x11bb, "LOOMIS");

		(void)runtime.startupScript().addRuntimeObjectToInventory("INV_MAG");
		return runtime.playDialogueLine(0x11db, "LOOMIS");
	}
	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL"))
		return runtime.playDialogueLine(0x1143, "LOOMIS");

	return runtime.playDialogueLine(0x1202, "LOOMIS");
}

} // End of namespace Harvester
