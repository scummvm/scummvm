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

#ifndef HARVESTER_NPC_MOM_DIALOGUE_H
#define HARVESTER_NPC_MOM_DIALOGUE_H

#include "harvester/npc/dialogue_handler.h"

namespace Harvester {

class MomDialogueHandler : public NpcDialogueHandler {
public:
	struct MomRoomDialogueState {
		bool introPending = true;
		bool sameDayIntroLineEnabled = false;
		bool postIntroDefaultLineEnabled = false;
		int introDayIndex = 0;
		bool stephMidgameShown = false;
		bool dinerBurnedKarinMissingOrDeadShown = false;
		bool burnedTvStationShown = false;
		bool scratchedTuckerShown = false;
		bool barberPoleStolenShown = false;
		bool boltOfClothTakenShown = false;
		bool dinerBurnedKarinAliveShown = false;
		bool escapedJailShown = false;
		bool karinKidnapedUnresolvedShown = false;
		bool karinFoundAliveShown = false;
		bool karinFoundDeadShown = false;
		bool butcherAbsentShown = false;
		bool moynahanAbsentShown = false;
		bool waspWomanAbsentShown = false;
		bool stephanieDeadPreMidgameShown = false;
		bool day5Shown = false;
		bool day6Shown = false;
	};

	bool matchesNpc(const Common::String &npcName) const override;
	void resetState() override { _state = MomRoomDialogueState(); }
	Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) override;

private:
	MomRoomDialogueState _state;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_MOM_DIALOGUE_H
