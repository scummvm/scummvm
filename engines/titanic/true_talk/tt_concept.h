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

#ifndef TITANIC_TT_CONCEPT_H
#define TITANIC_TT_CONCEPT_H

#include "titanic/true_talk/tt_string.h"
#include "titanic/true_talk/tt_word.h"

namespace Titanic {

enum ScriptType { ST_UNKNOWN_SCRIPT = 0, ST_ROOM_SCRIPT = 1, ST_NPC_SCRIPT = 2 };

class TTscriptBase;
class TTword;

class TTconcept {
private:
	TTstring _string1;
	int _field1C;
	TTword *_word2P;
	int _field30;
	bool _flag;
	int _status;
private:
	/**
	 * Sets the status of the concept
	 */
	bool setStatus();

	/**
	 * Sets the script type and resets other fields
	 */
	void setScriptType(ScriptType scriptType);

	/**
	 * Sets up the concept for a word reference
	 */
	int initializeWordRef(TTword *word);

	/**
	 * Resets the concept
	 */
	void reset();

	/**
	 * Initialize inner data for the concept from a given source concept
	 */
	void initialize(TTconcept &src);
public:
	TTconcept *_nextP;
	TTscriptBase *_scriptP;
	TTword *_wordP;
	int _scriptType;
	int _field14;
	int _field20;
	int _field34;
	TTstring _string2;
public:
	TTconcept();
	TTconcept(TTscriptBase *script, ScriptType scriptType);
	TTconcept(TTword *word, ScriptType scriptType = ST_UNKNOWN_SCRIPT);
	TTconcept(TTconcept &src);
	~TTconcept();

	/**
	 * Destroys any attached sibling concepts to the given concept
	 */
	void deleteSiblings();

	/**
	 * Copies data from a source concept
	 */
	void copyFrom(TTconcept *src);

	/**
	 * Compares the name of the associated word, if any, to the passed string
	 */
	bool compareTo(const char *str) const;

	/**
	 * Compares the concept to the specified word
	 */
	bool compareTo(TTword *word) const;

	/**
	 * Set an owner for the concept
	 */
	int setOwner(TTconcept *src);

	/**
	 * Set an owner for the concept
	 */
	int setOwner(TTword *src, bool dontDup);

	/**
	 * Return the status of the concept
	 */
	int getStatus() const { return _status; }

	/**
	 * True true if the concept is valid
	 */
	bool isValid() const { return _status == SS_VALID; }

	/**
	 * Returns true if the word is of the specified class
	 */
	bool isWordClass(WordClass wordClass) const {
		return _wordP && _wordP->isClass(wordClass);
	}

	void setFlag(bool val) { _flag = val; }
	void set1C(int val) { _field1C = val; }
	int get20() const { return _field20; }
	int getState() const { return _field34; }

	bool checkWordId1() const;
	bool checkWordId2() const;
	bool checkWordId3() const;
	bool checkWordClass() const;

	/**
	 * Return text assocaited with the concept's word or script
	 */
	const TTstring getText();

	/**
	 * Find a word by Id
	 */
	TTconcept *findByWordId(int id);

	/**
	 * Find a word by it's class
	 */
	TTconcept *findByWordClass(WordClass wordClass);

	TTconcept *findBy20(int val);

	/**
	 * Returns true if the concept has a word with a given Id
	 */
	bool isTheWordId(int id) const;

	/**
	 * If a word is associated, return it's Id
	 */
	int getTheWordId() const;
};

extern bool isWordId(const TTconcept *concept, int id);

extern int getWordId(const TTconcept *concept);

} // End of namespace Titanic

#endif /* TITANIC_TT_CONCEPT_H */
