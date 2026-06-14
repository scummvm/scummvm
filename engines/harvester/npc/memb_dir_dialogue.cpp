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


#include "harvester/npc/memb_dir_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kMembDirNpc = "MEMB_DIR";
static const int kMembDirResponseLineIndex = 0xfe;

} // End of namespace

bool MembDirDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kMembDirNpc);
}

Common::Error MembDirDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	auto playMembDirLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kMembDirNpc, headVariant);
	};

	if (!runtime.startupScript().getFlagValue("PC_KILLED_KEWPIE")) {
		Common::Error lineError = playMembDirLine(0x2b7, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(kMembDirResponseLineIndex, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1)
			return playMembDirLine(0x2c2);
		if (responseIndex == 2)
			return playMembDirLine(0x2cb);

		return Common::kNoError;
	}

	if (_state.kewpieKillLinePlayed)
		return Common::kNoError;

	_state.kewpieKillLinePlayed = true;
	Common::Error lineError = playMembDirLine(0x2d5, 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	if (runtime.startupScript().getFlagValue("TOOK_THE_DOLL"))
		return playMembDirLine(0x2e3);

	return playMembDirLine(0x2ea);
}

} // End of namespace Harvester
