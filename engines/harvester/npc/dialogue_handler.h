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

#ifndef HARVESTER_NPC_DIALOGUE_HANDLER_H
#define HARVESTER_NPC_DIALOGUE_HANDLER_H

#include "common/error.h"
#include "common/serializer.h"
#include "common/str.h"

namespace Harvester {

inline void syncDialogueBool(Common::Serializer &s, bool &value) {
	byte serialized = value ? 1 : 0;
	s.syncAsByte(serialized);
	if (s.isLoading())
		value = serialized != 0;
}

inline void syncDialogueInt(Common::Serializer &s, int &value) {
	s.syncAsSint32LE(value);
}

inline void syncDialogueString(Common::Serializer &s, Common::String &value) {
	s.syncString(value);
}

class DialogueRuntime;

struct DialogueSharedState {
	bool boyleGascanApplicationState = false;
	bool dialogueStateD2e98 = false;
	bool dialogueStateD2eb0 = false;
	bool dialogueStateD2eb8 = false;
	bool dialogueStateD2ebc = false;
	bool dialogueStateD2ec0 = false;
	bool dialogueStateD2ec8 = false;
	bool dialogueStateD2eec = false;
	bool dialogueStateD2ed0 = false;
	bool dialogueStateD2ef4 = false;
	bool dialogueStateD2f04 = false;
	bool dialogueStateD2ea4 = false;
	bool dialogueStateD2ea8 = false;
	bool dialogueStateD2f00 = false;
	bool karinKidnapedDialogueState = false;
	bool discussedLodgeTopic = false;
	bool dialogueStateD2f08 = false;
	bool waspWomanDialogueState = false;
	bool momGoodCauseDay5State = false;
	bool moynahanKarinKidnapedDiscussionState = false;
	bool dwayneWhaleyDisciplineFollowupState = false;
	bool dwayneDiscussedBoylesButton = false;
	bool dwayneCompletedKarinAliveFollowup = false;
	bool discussedMrPottsTuesdayNightAlibi = false;
	bool discussedMrsPottsTuesdayNightAlibi = false;
	bool confrontedMrPottsAboutSpyhole = false;
};

class NpcDialogueHandler {
public:
	virtual ~NpcDialogueHandler() {}

	virtual bool matchesNpc(const Common::String &npcName) const = 0;
	virtual void resetState() {}
	virtual void syncState(Common::Serializer &) {}
	virtual Common::Error handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) = 0;
};

} // End of namespace Harvester

#endif // HARVESTER_NPC_DIALOGUE_HANDLER_H
