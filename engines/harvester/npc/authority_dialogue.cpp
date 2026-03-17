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

#include "harvester/npc/authority_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool AuthorityDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("AUTHORITY");
}

Common::Error AuthorityDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	if (runtime.startupScript().getFlagValue("STOP_AUTHOR_TALK"))
		return Common::kNoError;

	if (runtime.startupScript().getFlagValue("IF_DON_T_EAT_THE_FOOD"))
		return runtime.playDialogueLine(0x3992, "AUTHORITY");

	return runtime.playDialogueLine(
		runtime.startupScript().getFlagValue("IF_YOU_EAT_THE_FOOD_AND_DEFEAT_THE_ENEMIES")
			? 0x399b
			: 0x398a,
		"AUTHORITY");
}

} // End of namespace Harvester
