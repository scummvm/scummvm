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


#include "harvester/npc/maint_man_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kMaintManNpc = "MAINT_MAN";
static const char *const kMaintManFirstConversationFlag = "MAINTENANCE_MAN_FIRST_CONVERSATION";
static const char *const kMaintManThirdConversationFlag = "MAINTENANCE_MAN_THIRD_CONVERSATION";
static const char *const kMaintManFourthConversationFlag = "MAINTENANCE_MAN_FOURTH_CONVERSATION";

} // End of namespace

bool MaintManDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kMaintManNpc);
}

Common::Error MaintManDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	int wavId = 0xdb6;
	int headVariant = 0;

	if (runtime.startupScript().getFlagValue(kMaintManFirstConversationFlag)) {
		wavId = 0xdaf;
		headVariant = 1;
	} else if (runtime.startupScript().getFlagValue(kMaintManThirdConversationFlag)) {
		return runtime.playDialogueLineWithVariant(0xdbc, kMaintManNpc, 2);
	} else if (runtime.startupScript().getFlagValue(kMaintManFourthConversationFlag)) {
		return runtime.playDialogueLineWithVariant(0xdc4, kMaintManNpc, 2);
	}

	if (headVariant != 0)
		return runtime.playDialogueLineWithVariant(wavId, kMaintManNpc, headVariant);

	return runtime.playDialogueLine(wavId, kMaintManNpc);
}

} // End of namespace Harvester
