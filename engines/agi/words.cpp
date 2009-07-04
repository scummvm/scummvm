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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

//
// New find_word algorithm by Thomas Akesson <tapilot@home.se>
//

#include "agi/agi.h"
#include "agi/keyboard.h"	// for clean_input()

namespace Agi {

static uint8 *words;		// words in the game
static uint32 wordsFlen;	// length of word memory

//
// Local implementation to avoid problems with strndup() used by
// gcc 3.2 Cygwin (see #635984)
//
static char *myStrndup(char *src, int n) {
	char *tmp = strncpy((char *)malloc(n + 1), src, n);
	tmp[n] = 0;
	return tmp;
}

int AgiEngine::loadWords(const char *fname) {
	Common::File fp;
	uint32 flen;
	uint8 *mem = NULL;

	words = NULL;

	if (!fp.open(fname)) {
		report("Warning: can't open %s\n", fname);
		return errOK; // err_BadFileOpen
	}
	report("Loading dictionary: %s\n", fname);

	fp.seek(0, SEEK_END);
	flen = fp.pos();
	wordsFlen = flen;
	fp.seek(0, SEEK_SET);

	if ((mem = (uint8 *)calloc(1, flen + 32)) == NULL) {
		fp.close();
		return errNotEnoughMemory;
	}

	fp.read(mem, flen);
	fp.close();

	words = mem;

	return errOK;
}

void AgiEngine::unloadWords() {
	if (words != NULL) {
		free(words);
		words = NULL;
	}
}

/**
 * Find a word in the dictionary
 * Uses an algorithm hopefully like the one Sierra used. Returns the ID
 * of the word and the length in flen. Returns -1 if not found.
 *
 * Thomas Akesson, November 2001
 */
int AgiEngine::findWord(char *word, int *flen) {
	int mchr = 0;		// matched chars
	int len, fchr, id = -1;
	uint8 *p = words;
	uint8 *q = words + wordsFlen;
	*flen = 0;

	debugC(2, kDebugLevelScripts, "find_word(%s)", word);

	if (word[0] >= 'a' && word[0] <= 'z')
		fchr = word[0] - 'a';
	else
		return -1;

	len = strlen(word);

	// Get the offset to the first word beginning with the
	// right character
	p += READ_BE_UINT16(p + 2 * fchr);

	while (p[0] >= mchr) {
		if (p[0] == mchr) {
			p++;
			// Loop through all matching characters
			while ((p[0] ^ word[mchr]) == 0x7F && mchr < len) {
				mchr++;
				p++;
			}
			// Check if this is the last character of the word
			// and if it matches
			if ((p[0] ^ word[mchr]) == 0xFF && mchr < len) {
				mchr++;
				if (word[mchr] == 0 || word[mchr] == 0x20) {
					id = READ_BE_UINT16(p + 1);
					*flen = mchr;
				}
			}
		}
		if (p >= q)
			return -1;

		// Step to the next word
		while (p[0] < 0x80)
			p++;

		p += 3;
	}

	return id;
}

void AgiEngine::dictionaryWords(char *msg) {
	char *p = NULL;
	char *q = NULL;
	int wid, wlen;

	debugC(2, kDebugLevelScripts, "msg = \"%s\"", msg);

	cleanInput();

	for (p = msg; p && *p && getvar(vWordNotFound) == 0;) {
		if (*p == 0x20)
			p++;

		if (*p == 0)
			break;

		wid = findWord(p, &wlen);
		debugC(2, kDebugLevelScripts, "find_word(p) == %d", wid);

		switch (wid) {
		case -1:
			debugC(2, kDebugLevelScripts, "unknown word");
			_game.egoWords[_game.numEgoWords].word = strdup(p);

			q = _game.egoWords[_game.numEgoWords].word;

			_game.egoWords[_game.numEgoWords].id = 19999;
			setvar(vWordNotFound, 1 + _game.numEgoWords);

			_game.numEgoWords++;

			p += strlen(p);
			break;
		case 0:
			// ignore this word
			debugC(2, kDebugLevelScripts, "ignore word");
			p += wlen;
			q = NULL;
			break;
		default:
			// an OK word
			debugC(3, kDebugLevelScripts, "ok word (%d)", wid);
			_game.egoWords[_game.numEgoWords].id = wid;
			_game.egoWords[_game.numEgoWords].word = myStrndup(p, wlen);
			_game.numEgoWords++;
			p += wlen;
			break;
		}

		if (p != NULL && *p) {
			debugC(2, kDebugLevelScripts, "p = %s", p);
			*p = 0;
			p++;
		}

		if (q != NULL) {
			for (; (*q != 0 && *q != 0x20); q++)
				;
			if (*q) {
				*q = 0;
				q++;
			}
		}
	}

	debugC(4, kDebugLevelScripts, "num_ego_words = %d", _game.numEgoWords);
	if (_game.numEgoWords > 0) {
		setflag(fEnteredCli, true);
		setflag(fSaidAcceptedInput, false);
	}
}

} // End of namespace Agi
