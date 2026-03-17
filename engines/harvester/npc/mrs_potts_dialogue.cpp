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

#include "harvester/npc/mrs_potts_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool MrsPottsDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MRS_POTTS") ||
		npcName.equalsIgnoreCase("MRS_POTTS_ST_BEDRM");
}

Common::Error MrsPottsDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	MrsPottsRoomDialogueState &state = _state;
	auto playMrsPottsLine = [&](int wavId, const char *speakerId = "MRS_POTTS") -> Common::Error {
		return runtime.playDialogueLine(wavId, speakerId);
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playMrsPottsLine(0x293c);
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return playMrsPottsLine(0x294d);
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			return playMrsPottsLine(0x294d);
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playMrsPottsLine(0x2954);
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceSheriffOwns, true);
			return playMrsPottsLine(0x296e);
		}
		if (usedItemName.equalsIgnoreCase("REMAINS"))
			return playMrsPottsLine(0x28b7);

		return playMrsPottsLine(0x2936);
	}

	if (runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
		Common::Error lineError = Common::kNoError;
		if (!sharedState.dialogueStateD2ea4) {
			sharedState.dialogueStateD2ea4 = true;
			lineError = playMrsPottsLine(0x283b);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!runtime.currentRoomName().equalsIgnoreCase("STKITCHN")) {
			lineError = playMrsPottsLine(0x2854);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE") &&
				!state.gotRemainsForLodgeLinePlayed) {
			state.gotRemainsForLodgeLinePlayed = true;
			lineError = playMrsPottsLine(0x289e);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("TAKEN_INVITE_TO_SERGEANT") &&
				!state.takenInviteToSergeantLinePlayed) {
			state.takenInviteToSergeantLinePlayed = true;
			lineError = playMrsPottsLine(0x286b);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return playMrsPottsLine(0x2835);
	}

	if (state.auxIntroPending) {
		state.auxIntroPending = false;
		state.returnVisitFollowupPending = true;
		return playMrsPottsLine(0x2726);
	}

	if (state.returnVisitFollowupPending) {
		state.returnVisitFollowupPending = false;
		return playMrsPottsLine(0x2800);
	}

	const int storyDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
	if (storyDayIndex > 1 && storyDayIndex < 6) {
		state.returnVisitFollowupPending = true;
		return playMrsPottsLine(0x281b);
	}

	if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
		state.returnVisitFollowupPending = true;
		return playMrsPottsLine(0x282d);
	}

	if (storyDayIndex == 5 && !state.day5LinePlayed) {
		state.day5LinePlayed = true;
		return playMrsPottsLine(0x2889);
	}
	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationLinePlayed) {
		state.burnedTvStationLinePlayed = true;
		return playMrsPottsLine(0x2891);
	}
	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnappedLinePlayed) {
		state.karinKidnappedLinePlayed = true;
		return playMrsPottsLine(0x28dc);
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			!state.dinerBurnedLinePlayed) {
		state.dinerBurnedLinePlayed = true;
		return playMrsPottsLine(0x290e);
	}

	return playMrsPottsLine(0x2930);
}

} // End of namespace Harvester
