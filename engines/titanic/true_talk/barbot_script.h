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

#ifndef TITANIC_BARBOT_SCRIPT_H
#define TITANIC_BARBOT_SCRIPT_H

#include "titanic/true_talk/tt_npc_script.h"

namespace Titanic {

class BarbotScript : public TTnpcScript {
private:
	int _state;
	int _arrIndex;
	TTsentenceEntries _entries2;
private:
	/**
	 * Adjust a given dial number by a given delta amount
	 */
	void adjustDial(int dialNum, int amount);

	/**
	 * Setup sentence data
	 */
	void setupSentences();

	bool isState9() const;

	int applySentenceIds(int dialogueId, int v34 = -1);
public:
	BarbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7);

	/**
	 * Chooses and adds a conversation response based on a specified tag Id.
	 */
	virtual int chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag);

	/**
	 * Does NPC specific processing of the parsed sentence
	 */
	virtual int process(TTroomScript *roomScript, TTsentence *sentence);

	/**
	 * Called when the script/id changes
	 */
	virtual ScriptChangedResult scriptChanged(TTscriptBase *roomScript, uint id);

	virtual bool handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		int val, uint tagId, uint remainder) const;
	
	virtual int proc21(int v1, int v2, int v3);
	virtual int proc22(int id) const;
	virtual int proc23() const;
	virtual int proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const;
	virtual void proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence);
	virtual void proc32();
	virtual int proc36(int val) const;
};

} // End of namespace Titanic

#endif /* TITANIC_BARBOT_SCRIPT_H */
