/***************************************************************************
 listwords.c Copyright (C) 2000 Christoph Reichenbach, TU Darmstadt


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CJR) [creichen@rbg.informatik.tu-darmstadt.de]

 History:

   000413 - created (LS) from vocabdump.c

***************************************************************************/

#include "sciunpack.h"
#include <engine.h>
#include <kernel.h>

int
_vocab_cmp_group(const void *word1, const void *word2)
{
#define fw (* ((word_t **) word1))
#define sw (* ((word_t **) word2))
	if (fw->group<sw->group)
		return -1;
	else if (fw->group==sw->group)
		return 0;
	else
		return 1;
}

int vocab_sort = DEFAULT_SORTING;

int
vocab_print(void)
{
	int b, words_nr, counter;
	word_t **words, **tracker;

	tracker = words = vocab_get_words(resmgr, &words_nr);

	counter=words_nr;

	if (vocab_sort==SORT_METHOD_GROUP)
		qsort(words, words_nr, sizeof(word_t *), _vocab_cmp_group); /* Sort entries */

	while (counter--) {
		printf("%s (class %03x, group %03x) ", &tracker[0]->word,
		       tracker[0]->w_class, tracker[0]->group);
      
		if ((tracker[0]->w_class>=0xf00)||
		    (tracker[0]->w_class==0)) 
			printf("anyword\n"); else
				while (tracker[0]->w_class) {
					b=sci_ffs(tracker[0]->w_class)-1;
					tracker[0]->w_class&=~(1<<b);
					printf("%s", class_names[b]);
					if (tracker[0]->w_class)
						printf("|"); else
							printf("\n");
				}    
		tracker++;
	}

	vocab_free_words(words, words_nr);

	return 0;
}






