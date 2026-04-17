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

#ifndef HARVESTER_NPC_FIREMAN2_DIALOGUE_H
#define HARVESTER_NPC_FIREMAN2_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class Fireman2DialogueHandler : public NpcDialogueHandler {
public:
	struct Fireman2RoomDialogueState {
		bool talkStateBlock = true;
		int dialogueStateD2de0 = 0;
		int dialogueStateD2de4 = 0;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override { _state = Fireman2RoomDialogueState(); }
	void syncState(Common::Serializer &s) override {
		syncDialogueBool(s, _state.talkStateBlock);
		syncDialogueInt(s, _state.dialogueStateD2de0);
		syncDialogueInt(s, _state.dialogueStateD2de4);
	}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	Fireman2RoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_FIREMAN2_DIALOGUE_H
