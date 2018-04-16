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

#ifndef TITANIC_ST_VOCAB_H
#define TITANIC_ST_VOCAB_H

#include "titanic/support/exe_resources.h"
#include "titanic/support/string.h"
#include "titanic/true_talk/tt_string.h"
#include "titanic/true_talk/tt_word.h"

namespace Titanic {

class TTvocab {
private:
	TTword *_headP;
	TTword *_tailP;
	TTword *_word;
	VocabMode _vocabMode;
private:
	/**
	 * Load the vocab data
	 */
	int load(const CString &name);

	/**
	 * Adds a specified word to the vocab list
	 */
	void addWord(TTword *word);

	/**
	 * Scans the vocab list for an existing word match
	 */
	TTword *findWord(const TTstring &str);

	/**
	 * Scans the vocab list for a word with a synonym matching the passed string.
	 * If found, creates a new word instance that only has the matching synonym
	 * linked to it.
	 * @param str		Word text to scan for
	 * @param srcWord	Optional pointer to store the original word match was found on
	 * @returns			A new word instance if a match if found, or null if not
	 */
	TTword *getPrimeWord(TTstring &str, TTword **srcWord = nullptr) const;

	/**
	 * Checks the passed word for common suffixes, like 's', 'ing', etc. and, if found,
	 * checks for a word match for the base word without the suffix.
	 * @param str		Word to check
	 * @returns			New word instance for found match, or nullptr otherwise
	 */
	TTword *getSuffixedWord(TTstring &str, TTword **srcWord = nullptr) const;

	/**
	 * Checks the passed word for common prefixes, and checks for a word
	 * match for the word without the given prefix
	 * @param str		Word to check
	 * @returns			New word instance for found match, or nullptr otherwise
	 */
	TTword *getPrefixedWord(TTstring &str, TTword **srcWord = nullptr) const;
public:
	TTvocab(VocabMode vocabMode);
	~TTvocab();

	/**
	 * Gets a matching word from the vocab list given a passed string
	 * @param str		Word text to scan for
	 * @param srcWord	Optional pointer to store the original word match was found on
	 * @returns			A new word instance if a match if found, or null if not
	 */
	TTword *getWord(TTstring &str, TTword **srcWord = nullptr) const;
};

} // End of namespace Titanic

#endif /* TITANIC_ST_VOCAB_H */
