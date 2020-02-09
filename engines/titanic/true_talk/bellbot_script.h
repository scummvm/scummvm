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

#ifndef TITANIC_BELLBOT_SCRIPT_H
#define TITANIC_BELLBOT_SCRIPT_H

#include "titanic/true_talk/tt_npc_script.h"

namespace Titanic {

class BellbotScript : public TTnpcScript {
private:
	static uint _oldId;
	TTmapEntryArray _states;
	TTmapEntryArray _preResponses;
	TTsentenceEntries _sentences[20];
	TTcommonPhraseArray _phrases;
	int _array[150];
	bool _responseFlag;
	bool _room107First;
private:
	/**
	 * Setup sentence data
	 */
	void setupSentences();

	/**
	 * Add the current location to the response
	 */
	int addLocation();

	/**
	 * Get a dialogue Id based on the state
	 */
	int getStateDialogueId() const;

	/**
	 * Sets the state value 25 based on the passed Id
	 */
	void setValue23(uint id);

	/**
	 * Does preprocessing for the sentence
	 */
	int preprocess(const TTroomScript *roomScript, const TTsentence *sentence);

	/**
	 * Checks for good, better, or bad in the sentence
	 */
	bool better(const TTsentence *sentence, uint id1, uint id2);

	bool randomResponse0(const TTroomScript *roomScript, uint id);
	bool randomResponse1(const TTroomScript *roomScript, uint id);
	bool randomResponse2(const TTroomScript *roomScript, uint id);
	void randomResponse3(const TTroomScript *roomScript, uint id);
	void randomResponse4(const TTroomScript *roomScript, uint id);

	int checkCommonSentences(const TTroomScript *roomScript, const TTsentence *sentence);
	bool checkCommonWords(const TTroomScript *roomScript, const TTsentence *sentence);

	uint getRoomDialogueId(const TTroomScript *roomScript);

	/**
	 * Adds a description of the room to the conversation response
	 */
	bool addRoomDescription(const TTroomScript *roomScript);
public:
	BellbotScript(int val1, const char *charClass, int v2,
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
	 * Process a sentence fragment entry
	 */
	int doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) override;

	/**
	 * Handles a randomzied response
	 */
	bool randomResponse(uint index) override;
};

} // End of namespace Titanic

#endif /* TITANIC_BELLBOT_SCRIPT_H */
