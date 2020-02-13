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

#ifndef GLK_ALAN3_DICTIONARY
#define GLK_ALAN3_DICTIONARY

#include "glk/alan3/acode.h"
#include "glk/alan3/types.h"

namespace Glk {
namespace Alan3 {

/* DATA */
extern DictionaryEntry *dictionary;
extern int dictionarySize;
extern int conjWord;        /* First conjunction in dictionary */


/* FUNCTIONS */
extern bool isVerbWord(int wordIndex);
extern bool isConjunctionWord(int wordIndex);
extern bool isExceptWord(int wordIndex);
extern bool isThemWord(int wordIndex);
extern bool isItWord(int wordIndex);
extern bool isNounWord(int wordIndex);
extern bool isAdjectiveWord(int wordIndex);
extern bool isPrepositionWord(int wordIndex);
extern bool isAllWord(int wordIndex);
extern bool isDirectionWord(int wordIndex);
extern bool isPronounWord(int wordIndex);
extern bool isLiteralWord(int wordIndex);

extern bool isConjunction(int wordCode);
extern bool isAll(int wordCode);
extern bool isNoise(int wordCode);
extern bool isPronoun(int wordCode);

} // End of namespace Alan3
} // End of namespace Glk

#endif
