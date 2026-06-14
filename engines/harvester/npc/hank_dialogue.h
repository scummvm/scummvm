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

#ifndef HARVESTER_NPC_HANK_DIALOGUE_H
#define HARVESTER_NPC_HANK_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class HankDialogueHandler : public NpcDialogueHandler {
public:
	struct HankRoomDialogueState {
		bool pendingInitialConversation = true;
		bool hasTrackedDayState = false;
		bool pendingRangshotSequence = false;
		bool pendingSameDayFollowup = false;
		int trackedDayIndex = 0;
		bool stephMidgamePlayedShown = false;
		bool burnedTvStationShown = false;
		bool bustedOnceShown = false;
		bool karinKidnapedShown = false;
		bool karinFoundAliveShown = false;
		bool karinFoundDeadShown = false;
		Common::String currentTopicBuffer;
		int currentTopicBufferLineIndex = -1;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override { _state = HankRoomDialogueState(); }
	void syncState(Common::Serializer &s) override {
		syncDialogueBool(s, _state.pendingInitialConversation);
		syncDialogueBool(s, _state.hasTrackedDayState);
		syncDialogueBool(s, _state.pendingRangshotSequence);
		syncDialogueBool(s, _state.pendingSameDayFollowup);
		syncDialogueInt(s, _state.trackedDayIndex);
		syncDialogueBool(s, _state.stephMidgamePlayedShown);
		syncDialogueBool(s, _state.burnedTvStationShown);
		syncDialogueBool(s, _state.bustedOnceShown);
		syncDialogueBool(s, _state.karinKidnapedShown);
		syncDialogueBool(s, _state.karinFoundAliveShown);
		syncDialogueBool(s, _state.karinFoundDeadShown);
		syncDialogueString(s, _state.currentTopicBuffer);
		syncDialogueInt(s, _state.currentTopicBufferLineIndex);
	}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	HankRoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_HANK_DIALOGUE_H
