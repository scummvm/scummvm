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

#include "harvester/npc/moynahan_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool MoynahanDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MOYNAHAN");
}

Common::Error MoynahanDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	MoynahanRoomDialogueState &state = _state;

	if (runtime.startupScript().getFlagValue("IF_TRY_TO_TAKE_THE_GLUE"))
		return runtime.playDialogueLine(0x3e4d, "MOYNAHAN");

	if (usedItemName.empty()) {
		if (state.talkStatePending) {
			state.talkStatePending = false;
			return runtime.playDialogueLine(0x3d33, "MOYNAHAN");
		}
		return runtime.playDialogueLine(0x3fd1, "MOYNAHAN");
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return runtime.playDialogueLine(0x3e45, "MOYNAHAN");
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY"))
		return runtime.playDialogueLine(0x3e84, "MOYNAHAN");
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		if (runtime.startupScript().getFlagValue(DialogueFlags::kShownLedgersToAnyone) &&
				!state.ledgerEvidenceLinePlayed) {
			state.ledgerEvidenceLinePlayed = true;
			Common::Error lineError = runtime.playDialogueLine(0x3f87, "MOYNAHAN");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		return runtime.playDialogueLine(0x3ede, "MOYNAHAN");
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		return runtime.playDialogueLine(0x3f3e, "MOYNAHAN");
	}
	if (usedItemName.equalsIgnoreCase("WED_MATCHES"))
		return runtime.playDialogueLine(0x4049, "MOYNAHAN");

	return runtime.playDialogueLine(0x3e3f, "MOYNAHAN");
}

} // End of namespace Harvester
