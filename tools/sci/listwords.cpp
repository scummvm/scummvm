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

#include "sciunpack.h"
#include <engine.h>
#include <kernel.h>

int
_vocab_cmp_group(const void *word1, const void *word2) {
#define fw (* ((word_t **) word1))
#define sw (* ((word_t **) word2))
	if (fw->group < sw->group)
		return -1;
	else if (fw->group == sw->group)
		return 0;
	else
		return 1;
}

int vocab_sort = DEFAULT_SORTING;

int
vocab_print(void) {
	int b, words_nr, counter;
	word_t **words, **tracker;

	tracker = words = vocab_get_words(resmgr, &words_nr);

	counter = words_nr;

	if (vocab_sort == SORT_METHOD_GROUP)
		qsort(words, words_nr, sizeof(word_t *), _vocab_cmp_group); /* Sort entries */

	while (counter--) {
		printf("%s (class %03x, group %03x) ", &tracker[0]->word,
		       tracker[0]->w_class, tracker[0]->group);

		if ((tracker[0]->w_class >= 0xf00) ||
		        (tracker[0]->w_class == 0))
			printf("anyword\n");
		else
			while (tracker[0]->w_class) {
				b = sci_ffs(tracker[0]->w_class) - 1;
				tracker[0]->w_class &= ~(1 << b);
				printf("%s", class_names[b]);
				if (tracker[0]->w_class)
					printf("|");
				else
					printf("\n");
			}
		tracker++;
	}

	vocab_free_words(words, words_nr);

	return 0;
}






