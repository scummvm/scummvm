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

namespace {

static const char *const kMcknightNpc = "MCKNIGHT";
static const char *const kPcSpeaker = "PC";

} // End of namespace

bool McknightDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kMcknightNpc);
}

Common::Error McknightDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	auto playMcknightLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kMcknightNpc, headVariant);
	};
	auto playPcLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kPcSpeaker, headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};

	if (runtime.startupScript().getFlagValue("LOOK_SAFE_2ND"))
		return playMcknightLine(0x15d0, 2);

	if (runtime.startupScript().getFlagValue("LOOK_SAFE_1ST") && !_state.lookSafeFirstLineGate) {
		_state.lookSafeFirstLineGate = true;
		return playMcknightLine(0x15c9, 2);
	}

	if (usedItemName.empty()) {
		if (_state.talkStatePending) {
			_state.talkStatePending = false;
			const DialogueLineEntry lines[] = {
				{ 0x1587, kMcknightNpc, 2 },
				{ 0x158b, kPcSpeaker, 0 },
				{ 0x158f, kMcknightNpc, 0 },
				{ 0x1594, kPcSpeaker, 0 },
				{ 0x1598, kMcknightNpc, 2 }
			};
			Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.dialogueStateD2f04 = true;
			lineError = playPcLine(0x159d);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playMcknightLine(0x15a2, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x18a, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1)
				return playMcknightLine(0x15af);
			if (responseIndex == 2) {
				const DialogueLineEntry replyLines[] = {
					{ 0x15b5, kMcknightNpc, 4 },
					{ 0x15bd, kPcSpeaker, 0 },
					{ 0x15c1, kMcknightNpc, 2 }
				};
				return playSequence(replyLines, ARRAYSIZE(replyLines));
			}
		}
		return playMcknightLine(0x15e4);
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		sharedState.discussedWhaleyHerrillPhoto = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return playMcknightLine(0x15ea, 1);
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		sharedState.discussedCasketPhotoEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		Common::Error lineError = playMcknightLine(0x15f2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (sharedState.dialogueStateD2ebc) {
			lineError = playPcLine(0x15f8);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return playMcknightLine(0x15fd);
		}

		lineError = playPcLine(0x1605);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return playMcknightLine(0x1609);
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		sharedState.discussedLedgerEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		return playMcknightLine(0x1611, 1);
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		sharedState.discussedNoteCheckbookEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		const DialogueLineEntry lines[] = {
			{ 0x161b, kMcknightNpc, 1 },
			{ 0x1621, kPcSpeaker, 0 },
			{ 0x1626, kMcknightNpc, 3 },
			{ 0x162b, kPcSpeaker, 4 },
			{ 0x1631, kMcknightNpc, 0 }
		};
		return playSequence(lines, ARRAYSIZE(lines));
	}

	return playMcknightLine(0x15de);
}

} // End of namespace Harvester
