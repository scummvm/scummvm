/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_DOORBOT_SCRIPT_H
#define TITANIC_DOORBOT_SCRIPT_H

#include "common/hashmap.h"
#include "titanic/true_talk/tt_npc_script.h"

namespace Titanic {

class DoorbotScript : public TTnpcScript {
	typedef Common::HashMap<uint, TTsentenceEntries> SentenceEntriesMap;
private:
	TTupdateStateArray _states;
	SentenceEntriesMap _sentences;
	int _stateIndex;
	int _doorbotState;
private:
	/**
	 * Setup sentence data
	 */
	void setupSentences();

	/**
	 * Sets a response
	 */
	int setResponse(int dialogueId, int v34 = -1);

	/**
	 * Gets the dialogue Id for a given room
	 */
	int getRoomDialogueId1(const TTroomScript *roomScript);

	/**
	 * Gets the dialogue Id for a given room
	 */
	int getRoomDialogueId2(const TTroomScript *roomScript);
public:
	DoorbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7);
	~DoorbotScript() override;

	/**
	 * Chooses and adds a conversation response based on a specified tag Id.
	 */
	int chooseResponse(const TTroomScript *roomScript, const TTsentence *sentence, uint tag) override;

	/**
	 * Does NPC specific processing of the parsed sentence
	 */
	int process(const TTroomScript *roomScript, const TTsentence *sentence) override;

	/**
	 * Called when the script/id changes
	 */
	ScriptChangedResult scriptChanged(const TTroomScript *roomScript, uint id) override;

	int handleQuote(const TTroomScript *roomScript, const TTsentence *sentence,
		uint tag1, uint tag2, uint remainder) override;

	/**
	 * Returns true if the NPC's dial region affects quote responses
	 */
	bool isQuoteDialled() const override { return true; }

	/**
	 * Handles updating NPC state based on specified dialogue Ids and dial positions
	 */
	int updateState(uint oldId, uint newId, int index) override;

	/**
	 * Handles getting a pre-response
	 */
	int preResponse(uint id) override;

	/**
	 * Returns a bitset of the dials being off or not
	 */
	uint getDialsBitset() const override;

	/**
	 * Process a sentence fragment entry
	 */
	int doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) override;

	/**
	 * Sets a given dial to be pointing in a specified region (0 to 2)
	 */
	void setDialRegion(int dialNum, int region) override;

	/**
	 * Handles a randomzied response
	 */
	bool randomResponse(uint index) override;
};

} // End of namespace Titanic

#endif /* TITANIC_DOORBOT_SCRIPT_H */
