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

#include "harvester/npc/mcknight_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool McknightDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MCKNIGHT");
}

Common::Error McknightDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	auto playMcknightLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "MCKNIGHT", headVariant);
	};

	if (runtime.startupScript().getFlagValue("LOOK_SAFE_2ND"))
		return playMcknightLine(0x15d0, 2);

	if (runtime.startupScript().getFlagValue("LOOK_SAFE_1ST") && !_state.lookSafeFirstLineGate)
		return playMcknightLine(0x15c9, 2);

	if (usedItemName.empty()) {
		if (_state.talkStatePending) {
			_state.talkStatePending = false;
			return playMcknightLine(0x1587, 2);
		}
		return playMcknightLine(0x15e4);
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return playMcknightLine(0x15ea, 1);
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		return playMcknightLine(0x15f2);
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		return playMcknightLine(0x1611, 1);
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		return playMcknightLine(0x161b, 1);
	}

	return playMcknightLine(0x15de);
}

} // End of namespace Harvester
