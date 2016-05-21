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

#ifndef TITANIC_TT_SENTENCE_H
#define TITANIC_TT_SENTENCE_H

#include "titanic/true_talk/tt_npc_script.h"
#include "titanic/true_talk/tt_room_script.h"
#include "titanic/true_talk/tt_sentence_node.h"
#include "titanic/true_talk/tt_string.h"

namespace Titanic {

class CScriptHandler;
class TTword;

class TTsentenceSubBase {
public:
	int _field0;
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
	int _field14;
	int _field18;
	int _field1C;
	TTsentenceSubBase *_nextP;
	int _field24;
public:
	TTsentenceSubBase();

	/**
	 * Delete any sibling chain attached to this node
	 */
	void deleteSiblings();

	void set18(int val) { _field18 = val; }
	int get18() const { return _field18; }
	bool is24() const { return _field24 == 0; }
};

class TTsentenceSub : public TTsentenceSubBase {
public:
	TTsentenceSub() : TTsentenceSubBase() {}

	/**
	 * Adds a new sibling instance
	 */
	TTsentenceSub *addSibling();
};

class TTsentence {
private:
	CScriptHandler *_owner;
	int _field2C;
	int _inputCtr;
	int _field34;
	int _field38;
	TTsentenceNode *_nodesP;
	int _field5C;
	int _status;
private:
	/**
	 * Copy sentence data from a given source
	 */
	void copyFrom(const TTsentence &src);
public:
	TTsentenceSub _sub;
	TTstring _initialLine;
	TTstring _normalizedLine;
	int _field58;
	TTroomScript *_roomScript;
	TTnpcScript *_npcScript;
public:
	TTsentence(int inputCtr, const TTstring &line, CScriptHandler *owner,
		TTroomScript *roomScript, TTnpcScript *npcScript);
	TTsentence(const TTsentence *src);
	~TTsentence();

	void set34(int v) { _field34 = v; }
	void set38(int v) { _field38 = v; }
	bool check2C() const { return _field2C > 1 && _field2C <= 10; }

	int getStatus() const { return _status; }

	/**
	 * Adds a found vocab word to the list of words representing
	 * the player's input
	 * @param word		Word to node
	 */
	int storeVocabHit(TTword *word);
};

} // End of namespace Titanic

#endif /* TITANIC_TT_SENTENCE_H */
