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

#include "glk/alan3/dictionary.h"
#include "glk/alan3/word.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
DictionaryEntry *dictionary;    /* Dictionary pointer */
int dictionarySize;
int conjWord;           /* First conjunction in dictionary, for ',' */



/* Word class query methods, move to Word.c */
/* Word classes are numbers but in the dictionary they are generated as bits */
static bool isVerb(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & VERB_BIT) != 0;
}

bool isVerbWord(int wordIndex) {
	return isVerb(playerWords[wordIndex].code);
}

bool isConjunction(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & CONJUNCTION_BIT) != 0;
}

bool isConjunctionWord(int wordIndex) {
	return isConjunction(playerWords[wordIndex].code);
}

static bool isExcept(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & EXCEPT_BIT) != 0;
}

bool isExceptWord(int wordIndex) {
	return isExcept(playerWords[wordIndex].code);
}

static bool isThem(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & THEM_BIT) != 0;
}

bool isThemWord(int wordIndex) {
	return isThem(playerWords[wordIndex].code);
}

static bool isIt(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & IT_BIT) != 0;
}

bool isItWord(int wordIndex) {
	return isIt(playerWords[wordIndex].code);
}

static bool isNoun(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & NOUN_BIT) != 0;
}

bool isNounWord(int wordIndex) {
	return isNoun(playerWords[wordIndex].code);
}

static bool isAdjective(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & ADJECTIVE_BIT) != 0;
}

bool isAdjectiveWord(int wordIndex) {
	return isAdjective(playerWords[wordIndex].code);
}

static bool isPreposition(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & PREPOSITION_BIT) != 0;
}

bool isPrepositionWord(int wordIndex) {
	return isPreposition(playerWords[wordIndex].code);
}

bool isAll(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & ALL_BIT) != 0;
}

bool isAllWord(int wordIndex) {
	return isAll(playerWords[wordIndex].code);
}

static bool isDir(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & DIRECTION_BIT) != 0;
}

bool isDirectionWord(int wordIndex) {
	return isDir(playerWords[wordIndex].code);
}

bool isNoise(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & NOISE_BIT) != 0;
}

bool isPronoun(int wordCode) {
	return wordCode < dictionarySize && (dictionary[wordCode].classBits & PRONOUN_BIT) != 0;
}

bool isPronounWord(int wordIndex) {
	return isPronoun(playerWords[wordIndex].code);
}

bool isLiteralWord(int wordIndex) {
	return playerWords[wordIndex].code >= dictionarySize;
}

} // End of namespace Alan3
} // End of namespace Glk
