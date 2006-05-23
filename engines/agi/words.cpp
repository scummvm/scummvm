/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2002 Sarien Team
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

/*
 * New find_word algorithm by Thomas Akesson <tapilot@home.se>
 */

#include "agi/agi.h"
#include "agi/keyboard.h"	/* for clean_input() */

namespace Agi {

static uint8 *words;		/* words in the game */
static uint32 words_flen;	/* length of word memory */

/*
 * Local implementation to avoid problems with strndup() used by
 * gcc 3.2 Cygwin (see #635984)
 */
static char *my_strndup(char *src, int n) {
	char *tmp = strncpy((char *)malloc(n + 1), src, n);
	tmp[n] = 0;
	return tmp;
}

int load_words(char *fname) {
	Common::File fp;
	uint32 flen;
	uint8 *mem = NULL;
	char *path = NULL;

	words = NULL;

	path = fname;

	if (!fp.open(path)) {
		report("Warning: can't open %s\n", path);
		return err_OK /*err_BadFileOpen */ ;
	}
	report("Loading dictionary: %s\n", path);

	fp.seek(0, SEEK_END);
	flen = fp.pos();
	words_flen = flen;
	fp.seek(0, SEEK_SET);

	if ((mem = (uint8 *)calloc(1, flen + 32)) == NULL) {
		fp.close();
		return err_NotEnoughMemory;
	}

	fp.read(mem, flen);
	fp.close();

	words = mem;

	return err_OK;
}

void unload_words() {
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
 * Thomas Åkesson, November 2001
 */
int find_word(char *word, int *flen) {
	int mchr = 0;		/* matched chars */
	int len, fchr, id = -1;
	uint8 *p = words;
	uint8 *q = words + words_flen;
	*flen = 0;

	debugC(2, kDebugLevelScripts, "find_word(%s)", word);
	if (word[0] >= 'a' && word[0] <= 'z')
		fchr = word[0] - 'a';
	else
		return -1;

	len = strlen(word);

	/* Get the offset to the first word beginning with the
	 * right character
	 */
	p += READ_BE_UINT16(p + 2 * fchr);

	while (p[0] >= mchr) {
		if (p[0] == mchr) {
			p++;
			/* Loop through all matching characters */
			while ((p[0] ^ word[mchr]) == 0x7F && mchr < len) {
				mchr++;
				p++;
			}
			/* Check if this is the last character of the word
			 * and if it matches
			 */
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

		/* Step to the next word */
		while (p[0] < 0x80)
			p++;
		p += 3;
	}

	return id;
}

void dictionary_words(char *msg) {
	char *p = NULL;
	char *q = NULL;
	int wid, wlen;

	debugC(2, kDebugLevelScripts, "msg = \"%s\"", msg);

	clean_input();

	for (p = msg; p && *p && getvar(V_word_not_found) == 0;) {
		if (*p == 0x20)
			p++;

		if (*p == 0)
			break;

		wid = find_word(p, &wlen);
		debugC(2, kDebugLevelScripts, "find_word(p) == %d", wid);

		switch (wid) {
		case -1:
			debugC(2, kDebugLevelScripts, "unknown word");
			game.ego_words[game.num_ego_words].word = strdup(p);
			q = game.ego_words[game.num_ego_words].word;
			game.ego_words[game.num_ego_words].id = 19999;
			setvar(V_word_not_found, 1 + game.num_ego_words);
			game.num_ego_words++;
			p += strlen(p);
			break;
		case 0:
			/* ignore this word */
			debugC(2, kDebugLevelScripts, "ignore word");
			p += wlen;
			q = NULL;
			break;
		default:
			/* an OK word */
			debugC(3, kDebugLevelScripts, "ok word (%d)", wid);
			game.ego_words[game.num_ego_words].id = wid;
			game.ego_words[game.num_ego_words].word = my_strndup(p, wlen);
			game.num_ego_words++;
			p += wlen;
			break;
		}

		if (p != NULL && *p) {
			debugC(2, kDebugLevelScripts, "p = %s", p);
			*p = 0;
			p++;
		}

		if (q != NULL) {
			for (; (*q != 0 && *q != 0x20); q++);
			if (*q) {
				*q = 0;
				q++;
			}
		}
	}

	debugC(4, kDebugLevelScripts, "num_ego_words = %d", game.num_ego_words);
	if (game.num_ego_words > 0) {
		setflag(F_entered_cli, true);
		setflag(F_said_accepted_input, false);
	}
}

}                             // End of namespace Agi
