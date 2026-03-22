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

#ifndef HARVESTER_NPC_MR_POTTS_DIALOGUE_H
#define HARVESTER_NPC_MR_POTTS_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class MrPottsDialogueHandler : public NpcDialogueHandler {
public:
	struct MrPottsRoomDialogueState {
		bool auxIntroPending = true;
		int followupDayIndex = 0;
		bool pottsdamDiggingLinePlayed = false;
		int topicBufferLineIndex = -1;
		bool stephanieAliveLaterDayShown = false;
		bool karinKidnapedShown = false;
		bool karinFoundDeadShown = false;
		bool karinFoundAliveShown = false;
		bool day6KarinShown = false;
		bool scratchedTuckerShown = false;
		bool barberPoleStolenShown = false;
		bool boltOfClothTakenShown = false;
		bool dinerBurnedShown = false;
		bool gotRemainsForLodgeShown = false;
		bool escapedJailShown = false;
		bool butcherDeathShown = false;
		bool moynahanDeathShown = false;
		bool ednaHungShown = false;
		bool day6KarinFoundAliveFollowupShown = false;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override {
		const int topicBufferLineIndex = _state.topicBufferLineIndex;
		_state = MrPottsRoomDialogueState();
		_state.topicBufferLineIndex = topicBufferLineIndex;
	}
	void syncState(Common::Serializer &s) override {
		syncDialogueBool(s, _state.auxIntroPending);
		syncDialogueInt(s, _state.followupDayIndex);
		syncDialogueBool(s, _state.pottsdamDiggingLinePlayed);
		syncDialogueInt(s, _state.topicBufferLineIndex);
		syncDialogueBool(s, _state.stephanieAliveLaterDayShown);
		syncDialogueBool(s, _state.karinKidnapedShown);
		syncDialogueBool(s, _state.karinFoundDeadShown);
		syncDialogueBool(s, _state.karinFoundAliveShown);
		syncDialogueBool(s, _state.day6KarinShown);
		syncDialogueBool(s, _state.scratchedTuckerShown);
		syncDialogueBool(s, _state.barberPoleStolenShown);
		syncDialogueBool(s, _state.boltOfClothTakenShown);
		syncDialogueBool(s, _state.dinerBurnedShown);
		syncDialogueBool(s, _state.gotRemainsForLodgeShown);
		syncDialogueBool(s, _state.escapedJailShown);
		syncDialogueBool(s, _state.butcherDeathShown);
		syncDialogueBool(s, _state.moynahanDeathShown);
		syncDialogueBool(s, _state.ednaHungShown);
		syncDialogueBool(s, _state.day6KarinFoundAliveFollowupShown);
	}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	MrPottsRoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_MR_POTTS_DIALOGUE_H
