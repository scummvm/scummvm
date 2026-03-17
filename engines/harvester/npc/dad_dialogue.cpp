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

#include "harvester/npc/dad_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool DadDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("DAD");
}

Common::Error DadDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	DadRoomDialogueState &state = _state;
	auto playDadLine = [&](int wavId, const char *speakerId = "DAD") -> Common::Error {
		return runtime.playDialogueLine(wavId, speakerId);
	};

	if (runtime.startupScript().getFlagValue("TAKING_BONDAGE"))
		return playDadLine(0x3a61);

	if (usedItemName.empty()) {
		if (state.introPending) {
			state.introPending = false;
			return playDadLine(0x39a5);
		}
		return playDadLine(0x3a17);
	}

	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		return playDadLine(0x3a39);
	}
	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		(void)runtime.startupScript().setRuntimeFlagValue(
			DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return playDadLine(0x3a41);
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(
			DialogueFlags::kShownEvidenceOfBlackmail, true);
		return playDadLine(0x3a51);
	}
	if (usedItemName.equalsIgnoreCase("MEAT_PERMISSION0")) {
		if (state.meatPermissionState)
			return playDadLine(0x39f2, "PC");
		return playDadLine(0x3a32);
	}

	return playDadLine(0x3a32);
}

} // End of namespace Harvester
