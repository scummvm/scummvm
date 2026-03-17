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

#include "harvester/npc/stub_dialogue.h"

#include "common/debug.h"
namespace Harvester {

StubNpcDialogueHandler::StubNpcDialogueHandler(const char *npcName) {
	_npcNames.push_back(npcName);
}

StubNpcDialogueHandler::StubNpcDialogueHandler(const Common::Array<Common::String> &npcNames)
	: _npcNames(npcNames) {
}

bool StubNpcDialogueHandler::matchesNpc(const Common::String &npcName) const {
	for (uint i = 0; i < _npcNames.size(); ++i) {
		if (npcName.equalsIgnoreCase(_npcNames[i]))
			return true;
	}

	return false;
}

Common::Error StubNpcDialogueHandler::handleDialogue(DialogueRuntime &,
		const Common::String &usedItemName, DialogueSharedState &) {
	debug(1, "Harvester: stub NPC dialogue handler '%s' item='%s'",
		_npcNames.empty() ? "<unknown>" : _npcNames.front().c_str(),
		usedItemName.empty() ? "<none>" : usedItemName.c_str());
	return Common::kNoError;
}

} // End of namespace Harvester
