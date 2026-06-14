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


#include "harvester/npc/fireman1_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kFireman1Npc = "FIREMAN1";

} // End of namespace

bool Fireman1DialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kFireman1Npc);
}

Common::Error Fireman1DialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	Fireman1RoomDialogueState &state = _state;

	Common::Error lineError = Common::kNoError;
	if (state.talkStatePending) {
		state.talkStatePending = false;
		lineError = runtime.playDialogueLine(0x453, kFireman1Npc);
	} else {
		lineError = runtime.playDialogueLine(0x463, kFireman1Npc);
	}
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgamePlayedShown) {
		state.stephMidgamePlayedShown = true;
		lineError = runtime.playDialogueLineWithVariant(0x45a, kFireman1Npc, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
			!state.boltOfClothTakenShown) {
		state.boltOfClothTakenShown = true;
		lineError = runtime.playDialogueLineWithVariant(0x46a, kFireman1Npc, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return Common::kNoError;
}

} // End of namespace Harvester
