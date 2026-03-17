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

#include "harvester/npc/fireman1_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

bool Fireman1DialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("FIREMAN1");
}

Common::Error Fireman1DialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	if (_state.talkStatePending) {
		_state.talkStatePending = false;
		return runtime.playDialogueLine(0x453, "FIREMAN1");
	}

	return runtime.playDialogueLine(0x463, "FIREMAN1");
}

} // End of namespace Harvester
