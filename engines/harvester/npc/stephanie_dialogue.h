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

#ifndef HARVESTER_NPC_STEPHANIE_DIALOGUE_H
#define HARVESTER_NPC_STEPHANIE_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class StephanieDialogueHandler : public NpcDialogueHandler {
public:
	struct StephanieRoomDialogueState {
		bool introPending = true;
		int introDayIndex = 0;
		bool firstFollowupPending = true;
		int firstFollowupDayIndex = 0;
		bool boltOfClothEscalationTriggered = false;
		bool playedSpyholeBranch = false;
		bool secondFollowupPending = true;
		int secondFollowupDayIndex = 0;
		bool scratchedTuckerShown = false;
		bool boltOfClothTakenShown = false;
		bool barberPoleStolenShown = false;
		bool burnedTvStationShown = false;
		bool dinerBurnedShown = false;
		bool dinerBurnedKarinAliveFollowupShown = false;
		bool dayFiveLinePlayed = false;
		bool pcEscapedJailShown = false;
		bool karinKidnapedLinePlayed = false;
		bool karinOutcomeLinePlayed = false;
		Common::String currentTopicBuffer;
		int currentTopicBufferLineIndex = -1;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override { _state = StephanieRoomDialogueState(); }
	void syncState(Common::Serializer &s) override {
		syncDialogueBool(s, _state.introPending);
		syncDialogueInt(s, _state.introDayIndex);
		syncDialogueBool(s, _state.firstFollowupPending);
		syncDialogueInt(s, _state.firstFollowupDayIndex);
		syncDialogueBool(s, _state.boltOfClothEscalationTriggered);
		syncDialogueBool(s, _state.playedSpyholeBranch);
		syncDialogueBool(s, _state.secondFollowupPending);
		syncDialogueInt(s, _state.secondFollowupDayIndex);
		syncDialogueBool(s, _state.scratchedTuckerShown);
		syncDialogueBool(s, _state.boltOfClothTakenShown);
		syncDialogueBool(s, _state.barberPoleStolenShown);
		syncDialogueBool(s, _state.burnedTvStationShown);
		syncDialogueBool(s, _state.dinerBurnedShown);
		syncDialogueBool(s, _state.dinerBurnedKarinAliveFollowupShown);
		syncDialogueBool(s, _state.dayFiveLinePlayed);
		syncDialogueBool(s, _state.pcEscapedJailShown);
		syncDialogueBool(s, _state.karinKidnapedLinePlayed);
		syncDialogueBool(s, _state.karinOutcomeLinePlayed);
		syncDialogueString(s, _state.currentTopicBuffer);
		syncDialogueInt(s, _state.currentTopicBufferLineIndex);
	}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	StephanieRoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_STEPHANIE_DIALOGUE_H
