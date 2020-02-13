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

#include "common/array.h"
#include "titanic/true_talk/tt_concept_node.h"
#include "titanic/true_talk/tt_npc_script.h"
#include "titanic/true_talk/tt_room_script.h"
#include "titanic/true_talk/tt_sentence_node.h"
#include "titanic/true_talk/tt_string.h"

namespace Titanic {

class CScriptHandler;
class TTword;

class TTsentenceConcept : public TTconceptNode {
public:
	TTsentenceConcept() : TTconceptNode() {}
	TTsentenceConcept(const TTsentenceConcept &src) : TTconceptNode(src) {}
	~TTsentenceConcept() override;

	/**
	 * Adds a new sibling instance
	 */
	TTsentenceConcept *addSibling();
};

class TTsentence {
private:
	CScriptHandler *_owner;
	int _inputCtr;
	int _field34;
	TTsentenceNode *_nodesP;
	int _field5C;
	int _status;
private:
	/**
	 * Copy sentence data from a given source
	 */
	void copyFrom(const TTsentence &src);
public:
	TTsentenceConcept _sentenceConcept;
	TTstring _initialLine;
	TTstring _normalizedLine;
	int _field38;
	int _field58;
	TTroomScript *_roomScript;
	TTnpcScript *_npcScript;
	int _category;
public:
	TTsentence(int inputCtr, const TTstring &line, CScriptHandler *owner,
		TTroomScript *roomScript, TTnpcScript *npcScript);
	TTsentence(const TTsentence *src);
	~TTsentence();

	void setState(int v) { _field34 = v; }
	void set38(int v) { _field38 = v; }
	bool checkCategory() const { return _category > 1 && _category <= 10; }
	int concept18(TTconceptNode *conceptNode) {
		return conceptNode ? conceptNode->get18() : 0;
	}
	int get58() const { return _field58; }
	int is18(int val, const TTconceptNode *node) const;
	int is1C(int val, const TTconceptNode *node) const;

	int getStatus() const { return _status; }

	/**
	 * Gets a concept slot
	 */
	TTconcept *getFrameEntry(int slotIndex, const TTconceptNode *conceptNode = nullptr) const;

	/**
	 * Gets a conecpt slot and returns a duplicate of it
	 */
	TTconcept *getFrameSlot(int slotIndex, const TTconceptNode *conceptNode = nullptr) const;

	/**
	 * Returns true if the specified slot has an attached word with a given class
	 */
	bool isFrameSlotClass(int slotIndex, WordClass wordClass, const TTconceptNode *conceptNode = nullptr) const;

	/**
	 * Adds a found vocab word to the list of words representing
	 * the player's input
	 * @param word		Word to node
	 */
	int storeVocabHit(TTword *word);

	bool fn1(const CString &str, int wordId1, const CString &str1, const CString &str2,
		const CString &str3, int wordId2, int val1, int val2, const TTconceptNode *node) const;
	bool fn3(const CString &str1, const CString &str2, const CString &str3,
		const CString &str4, const CString &str5, const CString &str6,
		int val1, int val2, const TTconceptNode *node) const;
	bool fn2(int slotIndex, const TTstring &str, const TTconceptNode *node = nullptr) const;
	bool fn4(int mode, int wordId, const TTconceptNode *node = nullptr) const;

	bool isConcept34(int slotIndex, const TTconceptNode *node = nullptr) const;

	/**
	 * Returns true if the sentence contains the specified word,
	 * allowing for common synonyms of the desired word
	 */
	bool localWord(const char *str) const;

	/**
	 * Returns true if the sentence (either the original or normalized lines)
	 * contain the specified substring
	 */
	bool contains(const char *str) const;
};

} // End of namespace Titanic

#endif /* TITANIC_TT_SENTENCE_H */
