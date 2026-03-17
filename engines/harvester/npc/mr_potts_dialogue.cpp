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

#include "harvester/npc/mr_potts_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool MrPottsDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MR_POTTS") ||
		npcName.equalsIgnoreCase("MR_POTTS_CEM10") ||
		npcName.equalsIgnoreCase("MR_POTTS_HALL");
}

Common::Error MrPottsDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	MrPottsRoomDialogueState &state = _state;
	auto playMrPottsLine = [&](int wavId, const char *speakerId = "MR_POTTS") -> Common::Error {
		return runtime.playDialogueLine(wavId, speakerId);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<StartupObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const StartupObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}

		return false;
	};

	if (hasInventoryItem("HARVEST_BLADE")) {
		(void)runtime.startupScript().setRuntimeFlagValue("PC_TALKED_TO_POTTS", true);
		return playMrPottsLine(0x2f1d, "POTTS_FLESH");
	}

	if (runtime.startupScript().getFlagValue("STEPHANIE_DEAD_IN_HALL_JUST_OUTSIDE")) {
		(void)runtime.startupScript().setRuntimeFlagValue(
			"STEPHANIE_DEAD_IN_HALL_JUST_OUTSIDE", false);
		return playMrPottsLine(0x2afc);
	}

	if (runtime.startupScript().getFlagValue("POTTSDAM_DIGGING")) {
		if (!state.pottsdamDiggingLinePlayed) {
			state.pottsdamDiggingLinePlayed = true;
			Common::Error lineError = playMrPottsLine(0x2c63);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return playMrPottsLine(0x2cd9);
	}

	if (runtime.startupScript().getFlagValue("POTTSDAM_DIGGING_KARIN"))
		return playMrPottsLine(0x2c33);

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("MEAT"))
			return playMrPottsLine(0x2c7f);
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return playMrPottsLine(0x2c97);
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playMrPottsLine(0x2cb3);
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playMrPottsLine(0x2ce0);
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceSheriffOwns, true);
			return playMrPottsLine(0x2ce0);
		}

		return playMrPottsLine(0x2c79);
	}

	if (runtime.startupScript().getFlagValue("IF_TRY_TO_SEE_STEPHANIE"))
		return playMrPottsLine(0x2b18);

	if (state.auxIntroPending &&
			!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
		state.auxIntroPending = false;
		state.followupDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
		return playMrPottsLine(0x2988);
	}

	if (state.followupDayIndex == runtime.startupScript().getCurrentStoryDayIndex()) {
		if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD"))
			return playMrPottsLine(0x2ac7);
		return playMrPottsLine(0x2aed);
	}

	if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD"))
		return playMrPottsLine(0x2ace);

	return playMrPottsLine(0x2bdf);
}

} // End of namespace Harvester
