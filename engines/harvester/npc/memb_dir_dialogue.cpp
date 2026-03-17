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

#include "harvester/npc/memb_dir_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool MembDirDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MEMB_DIR");
}

Common::Error MembDirDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	if (!runtime.startupScript().getFlagValue("PC_KILLED_KEWPIE"))
		return runtime.playDialogueLine(0x2b7, "MEMB_DIR");

	if (_state.kewpieKillLinePlayed)
		return Common::kNoError;

	_state.kewpieKillLinePlayed = true;
	return runtime.playDialogueLine(0x2d5, "MEMB_DIR");
}

} // End of namespace Harvester
