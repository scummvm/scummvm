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

#include "harvester/npc/buster_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool BusterDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("BUSTER");
}

Common::Error BusterDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return runtime.playDialogueLine(0x187b, "BUSTER");
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return runtime.playDialogueLine(0x1884, "BUSTER");
		}
		if (usedItemName.equalsIgnoreCase("NOTE_CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			return runtime.playDialogueLine(0x189b, "BUSTER");
		}
		return runtime.playDialogueLine(0x1875, "BUSTER");
	}

	if (_state.introPending) {
		_state.introPending = false;
		_state.secondIntroPending = true;
		return runtime.playDialogueLineWithVariant(0x163c, "BUSTER", 2);
	}
	if (_state.secondIntroPending) {
		_state.secondIntroPending = false;
		_state.thirdIntroPending = true;
		return runtime.playDialogueLineWithVariant(0x1828, "BUSTER", 2);
	}
	if (_state.thirdIntroPending) {
		_state.thirdIntroPending = false;
		return runtime.playDialogueLineWithVariant(0x18c5, "BUSTER", 2);
	}

	return runtime.playDialogueLine(0x1868, "BUSTER");
}

} // End of namespace Harvester
