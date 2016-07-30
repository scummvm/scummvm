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
	int _array[150];
	int _field2D0;
	int _field2D4;
	int _field2D8;
	int _field2DC;
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
public:
	BellbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2);

	/**
	 * Does NPC specific processing of the parsed sentence
	 */
	virtual int process(TTroomScript *roomScript, TTsentence *sentence);

	/**
	 * Called when the script/id changes
	 */
	virtual ScriptChangedResult scriptChanged(TTscriptBase *roomScript, uint id);

	virtual int proc15() const;

	virtual int handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		uint val, uint tagId, uint remainder);

	/**
	 * Handles updating NPC state based on specified dialogue Ids and dial positions
	 */
	virtual int updateState(uint oldId, uint newId, int index);

	/**
	 * Handles getting a pre-response
	 */
	virtual int preResponse(uint id);

	/**
	 * Process a sentence fragment entry
	 */
	virtual int doSentenceEntry(int val1, const int *srcIdP, TTroomScript *roomScript, TTsentence *sentence);

	virtual int proc36(int val) const;
};

} // End of namespace Titanic

#endif /* TITANIC_BELLBOT_SCRIPT_H */
