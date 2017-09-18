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

#ifndef TITANIC_TT_WORD_H
#define TITANIC_TT_WORD_H

#include "titanic/support/exe_resources.h"
#include "titanic/support/simple_file.h"
#include "titanic/true_talk/tt_string.h"
#include "titanic/true_talk/tt_synonym.h"

namespace Titanic {

/**
 * Types of words
 */
enum WordClass {
	WC_UNKNOWN = 0, WC_ACTION = 1, WC_THING = 2, WC_ABSTRACT = 3,
	WC_ARTICLE = 4, WC_CONJUNCTION = 5, WC_PRONOUN = 6,
	WC_PREPOSITION = 7, WC_ADJECTIVE = 8, WC_ADVERB = 9
};

class TTword {
protected:
	TTstringStatus _status;
	int _field24;
	int _field28;
protected:
	/**
	 * Read in a number
	 */
	uint readNumber(const char *str);

	bool testFileHandle(SimpleFile *file) const { return true; }
	bool testFileHandle(FileHandle resHandle) const;
public:
	TTword *_nextP;
	TTsynonym *_synP;
	TTstring _text;
	WordClass _wordClass;
	int _id;
	uint _tag;
public:
	TTword(const TTstring &str, WordClass wordClass, int val2);
	TTword(const TTword *src);
	virtual ~TTword();

	/**
	 * Delete any following words chained to the word
	 */
	void deleteSiblings();

	/**
	 * Read in a synonym for the given word
	 */
	int readSyn(SimpleFile *file);

	/**
	 * Set a new synonym for the word
	 */
	void setSyn(TTsynonym *synP);

	/**
	 * Set a new synonym string
	 */
	int setSynStr(TTstring &str);

	/**
	 * Returns true if synonyms have been set for the word
	 */
	bool hasSynonyms() const { return _synP != nullptr; }

	/**
	 * Either sets the first synonym for a word, or adds it to an existing one
	 */
	void appendNode(TTsynonym *node);

	/**
	 * Load the word
	 */
	int load(SimpleFile *file, WordClass wordClass);

	/**
	 * Finds a synonym in the word by name, if one exists
	 * @param str		Name to search for
	 * @param dest		Destination synonym instance to copy match into
	 * @param mode		Specifies English or German vocab mode
	 * @returns			Returns true if a match was found
	 */
	bool findSynByName(const TTstring &str, TTsynonym *dest, VocabMode mode) const;

	const char *c_str() const { return _text.c_str(); }
	operator const char *() const { return c_str(); }

	/**
	 * Return the text of the word
	 */
	const TTstring getText() { return _text; }

	/**
	 * Compares the word's text to a passed string
	 */
	bool compareTo(const char *str) const;

	/**
	 * Compares the word's text to a passed string
	 */
	bool compareTo(TTstring *str) const {
		return compareTo(str->c_str());
	}

	/**
	 * Return the status of the word
	 */
	TTstringStatus getStatus() const { return _status; }

	/**
	 * Returns true if the word is in a valid state
	 */
	bool isValid() const { return _status == SS_VALID; }

	/**
	 * Return the status of the entire word chain
	 */
	TTstringStatus getChainStatus() const;

	/**
	 * Returns true if the word is of the specified class
	 */
	bool isClass(WordClass wordClass) const { return _wordClass == wordClass; }

	/**
	 * Copy the word and any attached to it
	 */
	TTword *copyWords();

	/**
	 * Creates a copy of the word
	 */
	virtual TTword *copy() const;

	virtual bool proc2(int val) const { return false; }
	virtual int proc3() const { return -1; }
	virtual void proc4() {}
	virtual void proc5() {}

	/**
	 * Checks whether the word's tag is a known type
	 */
	virtual bool checkTag() const { return false; }

	/**
	 * Compare the word's tag to a given tag value
	 */
	virtual bool compareTagTo(uint tag) const { return false; }

	/**
	 * Return the tag associated with the word
	 */
	virtual uint getTag() const { return 0; }

	virtual bool proc9(int val) const { return false; }
	virtual int proc10() const { return 0; }
	virtual void proc11() {}
	virtual bool proc12(int val) const { return false; }
	virtual int proc13() const { return 0; }
	virtual bool proc14(int val) const { return false; }
	virtual int proc15() const { return -1; }
	virtual bool proc16() const { return false; }
	virtual bool proc17() const { return false; }
	virtual bool proc18() const { return false; }
	virtual bool comparePronounTo(int val) const { return false; }
	virtual int proc20() const { return 0; }

	/**
	 * Returns the file associated with the word's first synonym
	 */
	virtual FileHandle getSynFile() const;

	/**
	 * Checks whether the file associated with the word's first
	 * synonym matches the specified file
	 */
	virtual bool checkSynFile(FileHandle file) const;

	/**
	 * Sets the file associated with a synonym
	 */
	virtual void setSynFile(FileHandle file);

	/**
	 * Dumps data associated with the word to file
	 */
	virtual int save(SimpleFile *file) const { return 0; }
};

} // End of namespace Titanic

#endif /* TITANIC_TT_WORD_H */
