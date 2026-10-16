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


#include "harvester/npc/mcknight_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

#include "common/util.h"

namespace Harvester {

Common::Error McknightDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	if (runtime.startupScript().getFlagValue("LOOK_SAFE_2ND"))
		return runtime.playDialogueLineWithVariant(0x15d0, "MCKNIGHT", 2);

	if (runtime.startupScript().getFlagValue("LOOK_SAFE_1ST") && !_state.lookSafeFirstLineGate) {
		_state.lookSafeFirstLineGate = true;
		return runtime.playDialogueLineWithVariant(0x15c9, "MCKNIGHT", 2);
	}

	if (usedItemName.empty()) {
		if (_state.talkStatePending) {
			_state.talkStatePending = false;
			const DialogueLineEntry lines[] = {
				{ 0x1587, "MCKNIGHT", 2 },
				{ 0x158b, "PC", 0 },
				{ 0x158f, "MCKNIGHT", 0 },
				{ 0x1594, "PC", 0 },
				{ 0x1598, "MCKNIGHT", 2 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.dialogueStateD2f04 = true;
			lineError = runtime.playDialogueLineWithVariant(0x159d, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x15a2, "MCKNIGHT", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x18a, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1)
				return runtime.playDialogueLineWithVariant(0x15af, "MCKNIGHT", 0);
			if (responseIndex == 2) {
				const DialogueLineEntry replyLines[] = {
					{ 0x15b5, "MCKNIGHT", 4 },
					{ 0x15bd, "PC", 0 },
					{ 0x15c1, "MCKNIGHT", 2 }
				};
				return runtime.playDialogueEntrySequence(replyLines, ARRAYSIZE(replyLines));
			}
		}
		return runtime.playDialogueLineWithVariant(0x15e4, "MCKNIGHT", 0);
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		sharedState.discussedWhaleyHerrillPhoto = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return runtime.playDialogueLineWithVariant(0x15ea, "MCKNIGHT", 1);
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		sharedState.discussedCasketPhotoEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x15f2, "MCKNIGHT", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (sharedState.dialogueStateD2ebc) {
			lineError = runtime.playDialogueLineWithVariant(0x15f8, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return runtime.playDialogueLineWithVariant(0x15fd, "MCKNIGHT", 0);
		}

		lineError = runtime.playDialogueLineWithVariant(0x1605, "PC", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runtime.playDialogueLineWithVariant(0x1609, "MCKNIGHT", 0);
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		sharedState.discussedLedgerEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		return runtime.playDialogueLineWithVariant(0x1611, "MCKNIGHT", 1);
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		sharedState.discussedNoteCheckbookEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		const DialogueLineEntry lines[] = {
			{ 0x161b, "MCKNIGHT", 1 },
			{ 0x1621, "PC", 0 },
			{ 0x1626, "MCKNIGHT", 3 },
			{ 0x162b, "PC", 4 },
			{ 0x1631, "MCKNIGHT", 0 }
		};
		return runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
	}

	return runtime.playDialogueLineWithVariant(0x15de, "MCKNIGHT", 0);
}

} // End of namespace Harvester
