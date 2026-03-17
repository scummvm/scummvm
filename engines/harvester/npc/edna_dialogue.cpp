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

#include "harvester/npc/edna_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC043FstPath = "GRAPHIC/FST/C043.FST";
static const char *const kDialogueC043KFstPath = "GRAPHIC/FST/C043K.FST";

} // End of namespace

bool EdnaDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("EDNA");
}

Common::Error EdnaDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	EdnaRoomDialogueState &state = _state;
	auto playEdnaLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "EDNA", headVariant);
	};

	if (runtime.startupScript().getFlagValue("KILLED_KARIN1"))
		return playEdnaLine(0x3cdf, 2);

	if (runtime.startupScript().getFlagValue("DNA_S_SUICIDE_NOTE")) {
		(void)runtime.startupScript().setRuntimeFlagValue("DNA_S_SUICIDE_NOTE", false);
		Common::Error lineError = runtime.playDialogueFst(
			runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE")
				? kDialogueC043KFstPath
				: kDialogueC043FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		sharedState.dialogueStateD2ea4 = true;
		sharedState.dialogueStateD2ea8 = true;
		(void)runtime.startupScript().setRuntimeObjectVisible("DNAEXT", "SIGNOUT", true);
		return Common::kNoError;
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("K_PURSE")) {
			state.karinPurseLinePlayed = true;
			return playEdnaLine(0x3d05, 3);
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return playEdnaLine(0x3bd6);
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			return playEdnaLine(0x3bd6);
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playEdnaLine(0x3bde, 2);
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playEdnaLine(0x3be5, 2);
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);
			return playEdnaLine(0x3bee);
		}

		return playEdnaLine(0x3bcf);
	}

	if (runtime.startupScript().getFlagValue("BRING_KARIN_TO_SHERIFF")) {
		Common::Error lineError = playEdnaLine(0x3cce, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLine(0x3cd2, "PC");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playEdnaLine(0x3cd7, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().addRuntimeObjectToInventory("REWARD_MONEY");
		(void)runtime.startupScript().setRuntimeFlagValue("BRING_KARIN_TO_SHERIFF", false);
		return Common::kNoError;
	}

	if (state.introPending) {
		state.introPending = false;
		Common::Error lineError = playEdnaLine(0x3a95);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x9f, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playEdnaLine(0x3aa1, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLine(0x3aa6, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playEdnaLine(0x3aaa);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.dialogueStateD2f04 = true;
			lineError = runtime.playDialogueLine(0x3aae, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			const DialogueLineEntry responseLines[] = {
				{ 0x3ab2, "EDNA", 0 },
				{ 0x3ab7, "PC", 0 },
				{ 0x3abb, "EDNA", 0 },
				{ 0x3ac2, "EDNA", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = playEdnaLine(0x3ac6, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().getFlagValue("KARIN_KIDNAPED")) {
		Common::Error lineError = playEdnaLine(0x3bf8);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return playEdnaLine(0x3bff);
}

} // End of namespace Harvester
