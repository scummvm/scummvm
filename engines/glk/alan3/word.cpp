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

#include "glk/alan3/word.h"
#include "glk/alan3/types.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/lists.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */

/* List of parsed words, index into dictionary */
Word *playerWords = NULL;
int currentWordIndex; /* An index into the list of playerWords */
int firstWord, lastWord;  /* Index for the first and last words for this command */

/* Some variable for dynamically allocating the playerWords, which will happen in scan() */
static int playerWordsLength = 0;
#define PLAYER_WORDS_EXTENT 20

/* What did the user say? */
int verbWord; /* The word he used as a verb, dictionary index */
int verbWordCode; /* The code for that verb */


/* PRIVATE TYPES & DATA */


/*+++++++++++++++++++++++++++++++++++++++++++++++++++*/

void ensureSpaceForPlayerWords(int size) {
	int newLength = playerWordsLength + PLAYER_WORDS_EXTENT;

	if (playerWordsLength < size + 1) {
		playerWords = (Word *)realloc(playerWords, newLength * sizeof(Word));
		if (playerWords == NULL)
			syserr("Out of memory in 'ensureSpaceForPlayerWords()'");
		playerWordsLength = newLength;
	}
}


/*======================================================================*/
char *playerWordsAsCommandString(void) {
	char *commandString;
	int size = playerWords[lastWord].end - playerWords[firstWord].start;
	commandString = (char *)allocate(size + 1);
	strncpy(commandString, playerWords[firstWord].start, size);
	commandString[size] = '\0';
	return commandString;
}


/*======================================================================*/
void clearWordList(Word list[]) {
	implementationOfSetEndOfArray((Aword *)list);
}

} // End of namespace Alan3
} // End of namespace Glk
