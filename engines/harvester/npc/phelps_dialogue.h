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

#ifndef HARVESTER_NPC_PHELPS_DIALOGUE_H
#define HARVESTER_NPC_PHELPS_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

	class PhelpsDialogueHandler : public NpcDialogueHandler {
public:
	struct PhelpsRoomDialogueState {
		bool talkStatePending = true;
		bool stephMidgameShown = false;
		bool dayFiveShown = false;
		bool scratchedTuckerShown = false;
		bool barberPoleStolenShown = false;
		bool boltOfClothTakenShown = false;
		bool dinerBurnedShown = false;
		bool escapedJailShown = false;
		bool gotRemainsForLodgeShown = false;
		bool burnedTvStationShown = false;
		bool butcherDeadShown = false;
		bool moynahanDeadShown = false;
		bool jimmyDeadShown = false;
		bool karinFoundDeadShown = false;
		bool karinKidnapedShown = false;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override { _state = PhelpsRoomDialogueState(); }
	void syncState(Common::Serializer &s) override {
		syncDialogueBool(s, _state.talkStatePending);
		syncDialogueBool(s, _state.stephMidgameShown);
		syncDialogueBool(s, _state.dayFiveShown);
		syncDialogueBool(s, _state.scratchedTuckerShown);
		syncDialogueBool(s, _state.barberPoleStolenShown);
		syncDialogueBool(s, _state.boltOfClothTakenShown);
		syncDialogueBool(s, _state.dinerBurnedShown);
		syncDialogueBool(s, _state.escapedJailShown);
		syncDialogueBool(s, _state.gotRemainsForLodgeShown);
		syncDialogueBool(s, _state.burnedTvStationShown);
		syncDialogueBool(s, _state.butcherDeadShown);
		syncDialogueBool(s, _state.moynahanDeadShown);
		syncDialogueBool(s, _state.jimmyDeadShown);
		syncDialogueBool(s, _state.karinFoundDeadShown);
		syncDialogueBool(s, _state.karinKidnapedShown);
	}
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	PhelpsRoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_PHELPS_DIALOGUE_H
