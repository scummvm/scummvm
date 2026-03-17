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

#ifndef HARVESTER_NPC_DIALOGUE_HANDLER_H
#define HARVESTER_NPC_DIALOGUE_HANDLER_H

#include "common/error.h"
#include "common/str.h"

namespace Harvester {

class DialogueRuntime;

struct DialogueSharedState {
	bool boyleGascanApplicationState = false;
	bool dialogueStateD2f04 = false;
	bool karinKidnapedDialogueState = false;
	bool discussedLodgeTopic = false;
	bool waspWomanDialogueState = false;
};

class NpcDialogueHandler {
public:
	virtual ~NpcDialogueHandler() {}

	virtual bool matchesNpc(const Common::String &npcName) const = 0;
	virtual void resetState() {}
	virtual Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) = 0;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_DIALOGUE_HANDLER_H
