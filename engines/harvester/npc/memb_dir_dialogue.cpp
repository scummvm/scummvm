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

static const int kMembDirResponseLineIndex = 0xfe;

} // End of namespace

Common::Error MembDirDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	if (!runtime.startupScript().getFlagValue("PC_KILLED_KEWPIE")) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x2b7, "MEMB_DIR", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(kMembDirResponseLineIndex, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1)
			return runtime.playDialogueLineWithVariant(0x2c2, "MEMB_DIR", 0);
		if (responseIndex == 2)
			return runtime.playDialogueLineWithVariant(0x2cb, "MEMB_DIR", 0);

		return Common::kNoError;
	}

	if (_state.kewpieKillLinePlayed)
		return Common::kNoError;

	_state.kewpieKillLinePlayed = true;
	Common::Error lineError = runtime.playDialogueLineWithVariant(0x2d5, "MEMB_DIR", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	if (runtime.startupScript().getFlagValue("TOOK_THE_DOLL"))
		return runtime.playDialogueLineWithVariant(0x2e3, "MEMB_DIR", 0);

	return runtime.playDialogueLineWithVariant(0x2ea, "MEMB_DIR", 0);
}

} // End of namespace Harvester
