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

#ifndef HARVESTER_NPC_MRS_POTTS_DIALOGUE_H
#define HARVESTER_NPC_MRS_POTTS_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class MrsPottsDialogueHandler : public NpcDialogueHandler {
public:
	struct MrsPottsRoomDialogueState {
		bool auxIntroPending = true;
		bool returnVisitFollowupPending = false;
		bool gotRemainsForLodgeLinePlayed = false;
		bool takenInviteToSergeantLinePlayed = false;
		bool day5LinePlayed = false;
		bool burnedTvStationLinePlayed = false;
		bool karinKidnappedLinePlayed = false;
		bool dinerBurnedLinePlayed = false;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override { _state = MrsPottsRoomDialogueState(); }
	void syncState(Common::Serializer &s) override {
		syncDialogueBool(s, _state.auxIntroPending);
		syncDialogueBool(s, _state.returnVisitFollowupPending);
		syncDialogueBool(s, _state.gotRemainsForLodgeLinePlayed);
		syncDialogueBool(s, _state.takenInviteToSergeantLinePlayed);
		syncDialogueBool(s, _state.day5LinePlayed);
		syncDialogueBool(s, _state.burnedTvStationLinePlayed);
		syncDialogueBool(s, _state.karinKidnappedLinePlayed);
		syncDialogueBool(s, _state.dinerBurnedLinePlayed);
	}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	MrsPottsRoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_MRS_POTTS_DIALOGUE_H
