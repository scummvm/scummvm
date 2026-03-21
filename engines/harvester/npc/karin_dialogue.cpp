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

#include "harvester/npc/karin_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool KarinDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("KARIN");
}

Common::Error KarinDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	KarinRoomDialogueState &state = _state;
	auto playKarinLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "KARIN", headVariant);
	};
	auto playPcLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "PC", headVariant);
	};

	const bool karinFoundAlive = runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE");
	if (karinFoundAlive && runtime.startupScript().getFlagValue("IN_CEM10") &&
			!state.cem10AliveLinePlayed) {
		state.cem10AliveLinePlayed = true;
		Common::Error lineError = playPcLine(0x10a5, 4);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			if (!state.photoReplyOverrideFlag) {
				if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
					(void)runtime.startupScript().setRuntimeFlagValue(
						DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
					} else {
						(void)runtime.startupScript().setRuntimeFlagValue(
							DialogueFlags::kShownPhotoOfCorpse, true);
					}
				}
			return playKarinLine(0x10e3, 4);
		}
		if (usedItemName.equalsIgnoreCase("INV_MAG"))
			return playKarinLine(0x10ee);
		return playKarinLine(0x10d6, 1);
	}

	if (!karinFoundAlive && !state.notFoundAliveLinePlayed) {
		state.notFoundAliveLinePlayed = true;
		Common::Error lineError = playPcLine(0x1076, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return playKarinLine(0x10dc);
}

} // End of namespace Harvester
