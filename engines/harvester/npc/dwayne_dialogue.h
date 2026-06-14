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

#ifndef HARVESTER_NPC_DWAYNE_DIALOGUE_H
#define HARVESTER_NPC_DWAYNE_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class DwayneDialogueHandler : public NpcDialogueHandler {
public:
	struct DwayneRoomDialogueState {
		bool pendingInitialConversation = true;
		bool sheriffInDinerIntroPending = true;
		bool sheriffInDinerIntroPlayed = false;
		bool eventFollowupGate = false;
		bool tvDeedReplyOverride = false;
		bool presentedEvidenceReplyOverride = false;
		bool discussedBoylesButton = false;
		bool discussedKarinPurse = false;
		bool pendingKarinAliveFollowup = false;
		bool completedKarinAliveFollowup = false;
		int pendingKarinAliveFollowupDayIndex = 0;
		bool bringKarinToSheriffLinePlayed = false;
		bool whaleyDisciplineFollowupShown = false;
		bool noteCheckbookFollowupShown = false;
		bool scratchedTuckerShown = false;
		bool boltOfClothTakenShown = false;
		bool barberPoleStolenShown = false;
		bool dinerBurnedShown = false;
		bool escapedJailShown = false;
		bool gotRemainsForLodgeShown = false;
		bool burnedTvStationShown = false;
		bool karinKidnapedShown = false;
		bool pendingKarinAliveFollowupLinePlayed = false;
		bool karinFoundDeadWithoutPurseShown = false;
		bool jimmyAbsentShown = false;
		bool moynahanAbsentShown = false;
		bool ednaHungShown = false;
		bool mcknightAbsentShown = false;
		Common::String currentTopicBuffer;
		int currentTopicBufferLineIndex = -1;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override { _state = DwayneRoomDialogueState(); }
	void syncState(Common::Serializer &s) override {
		syncDialogueBool(s, _state.pendingInitialConversation);
		syncDialogueBool(s, _state.sheriffInDinerIntroPending);
		syncDialogueBool(s, _state.sheriffInDinerIntroPlayed);
		syncDialogueBool(s, _state.eventFollowupGate);
		syncDialogueBool(s, _state.tvDeedReplyOverride);
		syncDialogueBool(s, _state.presentedEvidenceReplyOverride);
		syncDialogueBool(s, _state.discussedBoylesButton);
		syncDialogueBool(s, _state.discussedKarinPurse);
		syncDialogueBool(s, _state.pendingKarinAliveFollowup);
		syncDialogueBool(s, _state.completedKarinAliveFollowup);
		syncDialogueInt(s, _state.pendingKarinAliveFollowupDayIndex);
		syncDialogueBool(s, _state.bringKarinToSheriffLinePlayed);
		syncDialogueBool(s, _state.whaleyDisciplineFollowupShown);
		syncDialogueBool(s, _state.noteCheckbookFollowupShown);
		syncDialogueBool(s, _state.scratchedTuckerShown);
		syncDialogueBool(s, _state.boltOfClothTakenShown);
		syncDialogueBool(s, _state.barberPoleStolenShown);
		syncDialogueBool(s, _state.dinerBurnedShown);
		syncDialogueBool(s, _state.escapedJailShown);
		syncDialogueBool(s, _state.gotRemainsForLodgeShown);
		syncDialogueBool(s, _state.burnedTvStationShown);
		syncDialogueBool(s, _state.karinKidnapedShown);
		syncDialogueBool(s, _state.pendingKarinAliveFollowupLinePlayed);
		syncDialogueBool(s, _state.karinFoundDeadWithoutPurseShown);
		syncDialogueBool(s, _state.jimmyAbsentShown);
		syncDialogueBool(s, _state.moynahanAbsentShown);
		syncDialogueBool(s, _state.ednaHungShown);
		syncDialogueBool(s, _state.mcknightAbsentShown);
		syncDialogueString(s, _state.currentTopicBuffer);
		syncDialogueInt(s, _state.currentTopicBufferLineIndex);
	}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	DwayneRoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_DWAYNE_DIALOGUE_H
