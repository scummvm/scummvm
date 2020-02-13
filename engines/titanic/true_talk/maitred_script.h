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

#ifndef TITANIC_MAITRED_SCRIPT_H
#define TITANIC_MAITRED_SCRIPT_H

#include "titanic/true_talk/tt_npc_script.h"

namespace Titanic {

class MaitreDScript : public TTnpcScript {
private:
	TTmapEntryArray _states;
	TTsentenceEntries _sentences1;
	int _answerCtr;
private:
	/**
	 * Setup sentence data
	 */
	void setupSentences();

	/**
	 * Alter dialogue Id based on current NPC state
	 */
	uint getStateDialogueId(uint oldId, uint newId);

	/**
	 * Starts the MaitreD fighting, if he isn't already
	 */
	void startFighting();

	/**
	 * Stops the MaitreD fighting
	 */
	void stopFighting(bool flag);

	/**
	 * Sets flags 10 to different values based on the passed
	 * dialogue Id
	 */
	void setFlags10(uint newId, uint index);

	/**
	 * Does preprocessing for the sentence
	 */
	int preprocess(const TTroomScript *roomScript, const TTsentence *sentence);
public:
	MaitreDScript(int val1, const char *charClass, int v2,
			const char *charName, int v3, int val2);

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
	 * Handles updating NPC state based on specified dialogue Ids and dial positions
	 */
	int updateState(uint oldId, uint newId, int index) override;

	/**
	 * Handles getting a pre-response
	 */
	int preResponse(uint id) override;
};

} // End of namespace Titanic

#endif /* TITANIC_MAITRED_SCRIPT_H */
