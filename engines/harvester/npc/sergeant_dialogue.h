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

#ifndef HARVESTER_NPC_SERGEANT_DIALOGUE_H
#define HARVESTER_NPC_SERGEANT_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class SergeantDialogueHandler : public NpcDialogueHandler {
public:
	struct SergeantRoomDialogueState {
		bool talkStateBlock = false;
		bool dialogueStateD2d50 = false;
		bool postApplicationBriefingCompleted = false;
		bool dialogueStateD2d58 = false;
		bool dialogueStateD2d5c = false;
		bool dialogueStateD2d60 = false;
		bool dialogueStateD2d64 = false;
		bool dialogueStateD2d68 = false;
		bool dialogueStateD2d6c = false;
		bool completedFirstTaskState = false;
		bool day5ExitLinePlayed = false;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override { _state = SergeantRoomDialogueState(); }
	void syncState(Common::Serializer &s) override {
		syncDialogueBool(s, _state.talkStateBlock);
		syncDialogueBool(s, _state.dialogueStateD2d50);
		syncDialogueBool(s, _state.postApplicationBriefingCompleted);
		syncDialogueBool(s, _state.dialogueStateD2d58);
		syncDialogueBool(s, _state.dialogueStateD2d5c);
		syncDialogueBool(s, _state.dialogueStateD2d60);
		syncDialogueBool(s, _state.dialogueStateD2d64);
		syncDialogueBool(s, _state.dialogueStateD2d68);
		syncDialogueBool(s, _state.dialogueStateD2d6c);
		syncDialogueBool(s, _state.completedFirstTaskState);
		syncDialogueBool(s, _state.day5ExitLinePlayed);
	}
	void migrateSharedState(DialogueSharedState &sharedState) override {
		if (sharedState.sergeantCompletedFirstTaskState == 0 && _state.completedFirstTaskState)
			sharedState.sergeantCompletedFirstTaskState = 1;
	}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	SergeantRoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_SERGEANT_DIALOGUE_H
