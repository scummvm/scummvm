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

#ifndef TITANIC_DESKBOT_SCRIPT_H
#define TITANIC_DESKBOT_SCRIPT_H

#include "common/array.h"
#include "titanic/true_talk/tt_npc_script.h"

namespace Titanic {

class DeskbotScript : public TTnpcScript {
private:
	/**
	 * Setup sentence data
	 */
	void setupSentences();

	/**
	 * Adds dialogue for the player's assigned room
	 */
	uint addAssignedRoomDialogue2();

	/**
	 * Adds dialogue for the player's assigned room
	 */
	void addAssignedRoomDialogue3();

	/**
	 * Gets a dialogue Id based on the NPC's state
	 */
	uint getStateDialogueId() const;

	/**
	 * Sets the current state for what the Deskbot is doing/asking
	 */
	void setCurrentState(uint newId, uint index);

	/**
	 * Scans the quotes tree
	 */
	int searchQuotes(const TTroomScript *roomScript, const TTsentence *sentence);

	/**
	 * Checks for common words
	 */
	int checkCommonWords(const TTsentence *sentence);

	/**
	 * Adds a dialogue for asking the player what kind of breakfast they'd like
	 */
	void addAskBreakfast();

	/**
	 * Adds a dialogue description for the player's assigned room
	 */
	void addAssignedRoom();
protected:
	static int _oldId;
	TTupdateStateArray _states;
	TTsentenceEntries _entries2;
	TTsentenceEntries _entries3;
protected:
	/**
	 * Does preprocessing for the sentence
	 */
	int preprocess(const TTroomScript *roomScript, const TTsentence *sentence);

	/**
	 * Adds dialogue for the player's assigned room
	 */
	uint addAssignedRoomDialogue();
public:
	DeskbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2);

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
	 * Handles updating NPC state based on specified dialogue Ids and dial positions
	 */
	int updateState(uint oldId, uint newId, int index) override;

	/**
	 * Handles getting a pre-response
	 */
	int preResponse(uint id) override;

	/**
	 * Returns a bitset of the first three dialgs being on or not
	 */
	uint getDialsBitset() const override;

	/**
	 * Process a sentence fragment entry
	 */
	int doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) override;

	/**
	 * Handles a randomzied response
	 */
	bool randomResponse(uint index) override;

	/**
	 * Returns true if dial 1 is the medium (1) region
	 */
	virtual bool isDial0Medium() const;

	/**
	 * Returns true if dial 0 is the low end region
	 */
	virtual bool isDial0Low() const;

	/**
	 * Returns true if dial 1 is the medium (1) region
	 */
	bool isDial1Medium() const;

	/**
	 * Returns true if dial 1 is the low end region
	 */
	virtual bool isDial1Low() const;
};

} // End of namespace Titanic

#endif /* TITANIC_DESKBOT_SCRIPT_H */
