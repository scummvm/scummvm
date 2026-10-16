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


#include "harvester/npc/karin_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {


static const DialogueLineEntry kKarinCem10AliveLines[] = {
	{ 0x10a5, "PC", 4 },
	{ 0x10a9, "KARIN", 3 },
	{ 0x10ae, "PC", 2 },
	{ 0x10b2, "KARIN", 3 },
	{ 0x10b6, "PC", 0 },
	{ 0x10ba, "KARIN", 3 },
	{ 0x10bf, "PC", 0 },
	{ 0x10c3, "KARIN", 3 }
};

static const DialogueLineEntry kKarinFirstNoItemLines[] = {
	{ 0x1076, "PC", 1 },
	{ 0x107a, "KARIN", 1 },
	{ 0x107e, "PC", 1 },
	{ 0x1082, "KARIN", 1 },
	{ 0x1087, "PC", 0 },
	{ 0x108b, "KARIN", 1 }
};

static const DialogueLineEntry kKarinFirstPhotoReplyLines[] = {
	{ 0x10e3, "KARIN", 4 },
	{ 0x10e7, "EDNA", 2 }
};

static const DialogueLineEntry kKarinInvMagLines[] = {
	{ 0x10ee, "KARIN", 0 },
	{ 0x10f2, "EDNA", 2 }
};

} // End of namespace

Common::Error KarinDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	KarinRoomDialogueState &state = _state;

	const bool karinFoundAlive = runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE");
	if (karinFoundAlive && runtime.startupScript().getFlagValue("IN_CEM10") &&
			!state.cem10AliveLinePlayed) {
		state.cem10AliveLinePlayed = true;
		return runtime.playDialogueEntrySequence(kKarinCem10AliveLines, ARRAYSIZE(kKarinCem10AliveLines));
	}

	if (usedItemName.empty()) {
		if (!karinFoundAlive && !state.notFoundAliveLinePlayed) {
			state.notFoundAliveLinePlayed = true;
			Common::Error lineError = runtime.playDialogueEntrySequence(kKarinFirstNoItemLines,
				ARRAYSIZE(kKarinFirstNoItemLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return runtime.playDialogueLineWithVariant(0x10dc, "KARIN", 0);
	}

	const bool photoEvidence = usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
		usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY") ||
		usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL");
	if (photoEvidence) {
		if (!state.photoReplyOverrideFlag) {
			if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL"))
				sharedState.discussedWhaleyHerrillPhoto = 1;
			else
				sharedState.discussedCasketPhotoEvidence = 1;

			Common::Error lineError = runtime.playDialogueEntrySequence(kKarinFirstPhotoReplyLines,
				ARRAYSIZE(kKarinFirstPhotoReplyLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			runtime.setActiveSpeakerPortrait("KARIN", 0);
			state.photoReplyOverrideFlag = true;
			return Common::kNoError;
		}

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x10e3, "KARIN", 4);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue("DNALFT_PERVERT", true);
		runtime.executeAndQueueAction("CALL_POLICE");
		return Common::kNoError;
	}

	if (usedItemName.equalsIgnoreCase("INV_MAG")) {
		Common::Error lineError = runtime.playDialogueEntrySequence(kKarinInvMagLines, ARRAYSIZE(kKarinInvMagLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue("DNALFT_PERVERT", true);
		runtime.executeAndQueueAction("CALL_POLICE");
		return Common::kNoError;
	}

	return runtime.playDialogueLineWithVariant(0x10d6, "KARIN", 1);
}

} // End of namespace Harvester
