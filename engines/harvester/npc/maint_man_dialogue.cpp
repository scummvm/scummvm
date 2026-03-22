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

#include "harvester/npc/maint_man_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool MaintManDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MAINT_MAN");
}

Common::Error MaintManDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	if (runtime.startupScript().getFlagValue("MAINTENANCE_MAN_FIRST_CONVERSATION"))
		return runtime.playDialogueLineWithVariant(0xdaf, "MAINT_MAN", 1);

	if (runtime.startupScript().getFlagValue("MAINTENANCE_MAN_THIRD_CONVERSATION"))
		return runtime.playDialogueLineWithVariant(0xdbc, "MAINT_MAN", 2);

	if (runtime.startupScript().getFlagValue("MAINTENANCE_MAN_FOURTH_CONVERSATION"))
		return runtime.playDialogueLineWithVariant(0xdc4, "MAINT_MAN", 2);

	return runtime.playDialogueLine(0xdb6, "MAINT_MAN");
}

} // End of namespace Harvester
