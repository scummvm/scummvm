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

#include "harvester/npc/dark_woman_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kHandMirrorItemName = "HANDMIRROR";

} // End of namespace

bool DarkWomanDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("DARK_WOMAN");
}

Common::Error DarkWomanDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	if (usedItemName.equalsIgnoreCase(kHandMirrorItemName))
		return runtime.playDialogueLine(0x4d03, "DARK_WOMAN");

	if (!_state.talkStatePending)
		return Common::kNoError;

	_state.talkStatePending = false;
	return runtime.playDialogueLine(0x4cd5, "DARK_WOMAN");
}

} // End of namespace Harvester
