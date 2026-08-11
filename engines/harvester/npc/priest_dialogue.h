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

#ifndef HARVESTER_NPC_PRIEST_DIALOGUE_H
#define HARVESTER_NPC_PRIEST_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class PriestDialogueHandler : public NpcDialogueHandler {
public:
	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override {}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_PRIEST_DIALOGUE_H
