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

#include "harvester/npc/boyle_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool BoyleDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("BOYLE");
}

Common::Error BoyleDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	BoyleRoomDialogueState &state = _state;
	auto playBoyleLine = [&](int wavId) -> Common::Error {
		return runtime.playDialogueLine(wavId, "BOYLE");
	};

	if (usedItemName.empty()) {
		if (state.introPending) {
			state.introPending = false;
			return playBoyleLine(0x7);
		}
		return playBoyleLine(0x23c);
	}

	if (usedItemName.equalsIgnoreCase("BOYLES_BUTTON"))
		return playBoyleLine(0x155);

	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(
			DialogueFlags::kShownEvidenceOfBlackmail, true);
		return playBoyleLine(runtime.startupScript().getFlagValue("HAVE_LODGE_APP") ? 0x13d : 0x197);
	}

	if (usedItemName.equalsIgnoreCase("GASCAN") && sharedState.boyleGascanApplicationState) {
		const DialogueLineEntry gascanLines[] = {
			{ 0x1f2, "BOYLE", 0 },
			{ 0x1f6, "BOYLE", 0 },
			{ 0x1fa, "BOYLE", 0 },
			{ 0x206, "BOYLE", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(gascanLines, ARRAYSIZE(gascanLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		sharedState.boyleGascanApplicationState = false;
		(void)runtime.startupScript().resetRuntimeObjectToInitialState("GASCAN");
		(void)runtime.startupScript().addRuntimeObjectToInventory("LODGE_APPLICATION");
		(void)runtime.startupScript().setRuntimeFlagValue("HAVE_LODGE_APP", true);
		return Common::kNoError;
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		(void)runtime.startupScript().setRuntimeFlagValue(
			DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return playBoyleLine(0x25f);
	}

	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		return playBoyleLine(0x24f);
	}

	return playBoyleLine(0x249);
}

} // End of namespace Harvester
