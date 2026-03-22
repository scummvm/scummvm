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

#include "harvester/npc/butcher_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool ButcherDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("BUTCHER");
}

Common::Error ButcherDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	if (usedItemName.empty()) {
		if (_state.talkStatePending) {
			_state.talkStatePending = false;
			return runtime.playDialogueLine(0x1902, "BUTCHER");
		}
		return runtime.playDialogueLine(0x1a47, "BUTCHER");
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return runtime.playDialogueLine(0x1a5a, "BUTCHER");
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		return runtime.playDialogueLine(0x1a69, "BUTCHER");
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		return runtime.playDialogueLine(0x1a80, "BUTCHER");
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		return runtime.playDialogueLine(0x1a9e, "BUTCHER");
	}
	if (usedItemName.equalsIgnoreCase("MEAT_PERMISSION"))
		return runtime.playDialogueLineWithVariant(0x1ac4, "BUTCHER", 1);

	return runtime.playDialogueLine(0x1a54, "BUTCHER");
}

} // End of namespace Harvester
