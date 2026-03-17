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

#include "harvester/npc/phelps_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kGetPrnMagActionTag = "GET_PRN_MAG";
static const char *const kUseRewardAtStoreActionTag = "USE_REWARD_AT_STORE";
static const char *const kSetGenCuStuffActionTag = "SET_GEN_CU_STUFF";

} // End of namespace

bool PhelpsDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("PHELPS");
}

Common::Error PhelpsDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	auto executeDialogueActionTag = [&](const char *tag) {
		StartupInteractionResult interaction;
		if (runtime.startupScript().executeActionTag(tag, interaction)) {
			runtime.applyImmediateDialogueInteractionEffects(interaction);
			runtime.queueDialogueInteractionIfNeeded(interaction);
		}
	};

	if (usedItemName.empty()) {
		if (_state.talkStatePending) {
			_state.talkStatePending = false;
			return runtime.playDialogueLine(0x1b44, "PHELPS");
		}
		return runtime.playDialogueLine(0x1bf4, "PHELPS");
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return runtime.playDialogueLine(0x1c0c, "PHELPS");
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		return runtime.playDialogueLine(0x1c1e, "PHELPS");
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		return runtime.playDialogueLine(0x1c36, "PHELPS");
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		return runtime.playDialogueLine(0x1c3d, "PHELPS");
	}
	if (usedItemName.equalsIgnoreCase("QUARTER")) {
		executeDialogueActionTag(kGetPrnMagActionTag);
		return runtime.playDialogueLine(0x1ba4, "PHELPS");
	}
	if (usedItemName.equalsIgnoreCase("REWARD_MONEY")) {
		executeDialogueActionTag(kUseRewardAtStoreActionTag);
		executeDialogueActionTag(kSetGenCuStuffActionTag);
		return Common::kNoError;
	}

	return runtime.playDialogueLine(0x1c06, "PHELPS");
}

} // End of namespace Harvester
