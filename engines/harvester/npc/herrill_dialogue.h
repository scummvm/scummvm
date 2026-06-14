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

#ifndef HARVESTER_NPC_HERRILL_DIALOGUE_H
#define HARVESTER_NPC_HERRILL_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class HerrillDialogueHandler : public NpcDialogueHandler {
public:
	struct HerrillRoomDialogueState {
		bool introPending = true;
		bool gascanPresented = false;
		bool dialogueStateD2ce4 = false;
		bool dialogueSuppressed = false;
		bool requiresEvidenceForDialogue = false;
		bool stephMidgamePlayedShown = false;
		bool dayFiveShown = false;
		bool scratchedTuckerShown = false;
		bool barberPoleStolenShown = false;
		bool karinKidnapedShown = false;
		bool karinFoundAliveShown = false;
		bool karinFoundDeadShown = false;
		bool dinerBurnedShown = false;
		bool whaleyHerrillFollowupShown = false;
		Common::String currentTopicBuffer;
		int currentTopicBufferLineIndex = -1;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override { _state = HerrillRoomDialogueState(); }
	void syncState(Common::Serializer &s) override {
		syncDialogueBool(s, _state.introPending);
		syncDialogueBool(s, _state.gascanPresented);
		syncDialogueBool(s, _state.dialogueStateD2ce4);
		syncDialogueBool(s, _state.dialogueSuppressed);
		syncDialogueBool(s, _state.requiresEvidenceForDialogue);
		syncDialogueBool(s, _state.stephMidgamePlayedShown);
		syncDialogueBool(s, _state.dayFiveShown);
		syncDialogueBool(s, _state.scratchedTuckerShown);
		syncDialogueBool(s, _state.barberPoleStolenShown);
		syncDialogueBool(s, _state.karinKidnapedShown);
		syncDialogueBool(s, _state.karinFoundAliveShown);
		syncDialogueBool(s, _state.karinFoundDeadShown);
		syncDialogueBool(s, _state.dinerBurnedShown);
		syncDialogueBool(s, _state.whaleyHerrillFollowupShown);
		syncDialogueString(s, _state.currentTopicBuffer);
		syncDialogueInt(s, _state.currentTopicBufferLineIndex);
	}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	HerrillRoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_HERRILL_DIALOGUE_H
