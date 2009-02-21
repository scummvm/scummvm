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

/* Main vocabulary support functions and word lookup */


#include "sci/include/sciresource.h"
#include "sci/include/engine.h"
#include "sci/include/kernel.h"

namespace Sci {

int vocab_version;

#define VOCAB_RESOURCE_PARSE_TREE_BRANCHES vocab_version==1 ? \
					   VOCAB_RESOURCE_SCI1_PARSE_TREE_BRANCHES : \
					   VOCAB_RESOURCE_SCI0_PARSE_TREE_BRANCHES

#define VOCAB_RESOURCE_SUFFIX_VOCAB vocab_version==1 ? \
				    VOCAB_RESOURCE_SCI1_SUFFIX_VOCAB : \
				    VOCAB_RESOURCE_SCI0_SUFFIX_VOCAB

const char *class_names[] = {"",              /* These strange names were taken from an SCI01 interpreter */
                             "",
                             "conj",		/* conjunction */
                             "ass",		/* ? */
                             "pos",		/* preposition ? */
                             "art",		/* article */
                             "adj",		/* adjective */
                             "pron",		/* pronoun */
                             "noun",		/* noun */
                             "auxv",		/* auxillary verb */
                             "adv",		/* adverb */
                             "verb",		/* verb */
                             "",
                             "",
                             "",
                             ""
                            };


int
_vocab_cmp_words(const void *word1, const void *word2) {
	return scumm_stricmp((*((word_t **) word1))->word, (*((word_t **) word2))->word);
}


word_t **
vocab_get_words(ResourceManager *resmgr, int *word_counter) {
	int counter = 0;
	unsigned int seeker;
	word_t **words;

	char currentword[256] = ""; /* They're not going to use words longer than 255 ;-) */
	int currentwordpos = 0;

	resource_t *resource;

	/* First try to load the SCI0 vocab resource. */
	resource = scir_find_resource(resmgr, sci_vocab,
	                              VOCAB_RESOURCE_SCI0_MAIN_VOCAB, 0);
	vocab_version = 0;

	if (!resource) {
		warning("SCI0: Could not find a main vocabulary, trying SCI01");
		resource = scir_find_resource(resmgr, sci_vocab,
		                              VOCAB_RESOURCE_SCI1_MAIN_VOCAB, 0);
		vocab_version = 1;
	}

	if (!resource) {
		warning("SCI1: Could not find a main vocabulary");
		return NULL; /* NOT critical: SCI1 games and some demos don't have one! */
	}

	if (vocab_version == 1)
		seeker = 255 * 2; /* vocab.900 starts with 255 16-bit pointers which we don't use */
	else
		seeker = 26 * 2; /* vocab.000 starts with 26 16-bit pointers which we don't use */

	if (resource->size < seeker) {
		fprintf(stderr, "Invalid main vocabulary encountered: Too small\n");
		return NULL;
		/* Now this ought to be critical, but it'll just cause parse() and said() not to work */
	}

	words = (word_t**)sci_malloc(sizeof(word_t *));

	while (seeker < resource->size) {
		byte c;

		words = (word_t**)sci_realloc(words, (counter + 1) * sizeof(word_t *));

		currentwordpos = resource->data[seeker++]; /* Parts of previous words may be re-used */

		if (vocab_version == 1) {
			c = 1;
			while (seeker < resource->size
			        && currentwordpos < 255
			        && c) {
				c = resource->data[seeker++];
				currentword[currentwordpos++] = c;
			}
			if (seeker == resource->size) {
				warning("SCI1: Vocabulary not usable, disabling");
				vocab_free_words(words, counter);
				return NULL;
			}
		} else {
			do {
				c = resource->data[seeker++];
				currentword[currentwordpos++] = c & 0x7f; /* 0x80 is used to terminate the string */
			} while (c < 0x80);
		}

		currentword[currentwordpos] = 0;

		words[counter] = (word_t*)sci_malloc(sizeof(word_t) + currentwordpos);
		/* Allocate more memory, so that the word fits into the structure */

		strcpy(&(words[counter]->word[0]), &(currentword[0])); /* Copy the word */

		/* Now decode class and group: */
		c = resource->data[seeker + 1];
		words[counter]->w_class = ((resource->data[seeker]) << 4) | ((c & 0xf0) >> 4);
		words[counter]->group = (resource->data[seeker + 2]) | ((c & 0x0f) << 8);

		seeker += 3;
		++counter;

	}

	*word_counter = counter;

	qsort(words, counter, sizeof(word_t *), _vocab_cmp_words); /* Sort entries */

	return words;
}


void
vocab_free_words(word_t **words, int words_nr) {
	int i;

	for (i = 0; i < words_nr; i++)
		free(words[i]);

	free(words);
}


const char *
vocab_get_any_group_word(int group, word_t **words, int words_nr) {
	int i;

	if (group == VOCAB_MAGIC_NUMBER_GROUP)
		return "{number}";

	for (i = 0; i < words_nr; i++)
		if (words[i]->group == group)
			return words[i]->word;

	return "{invalid}";
}


static inline unsigned int
inverse_16(unsigned int foo) {
	return (((foo & 0xff) << 8) | ((foo & 0xff00) >> 8));
}

suffix_t **
vocab_get_suffices(ResourceManager *resmgr, int *suffices_nr) {
	int counter = 0;
	suffix_t **suffices;
	resource_t *resource = scir_find_resource(resmgr, sci_vocab, VOCAB_RESOURCE_SUFFIX_VOCAB, 1);
	unsigned int seeker = 1;

	if (!resource) {
		fprintf(stderr, "Could not find suffix vocabulary!\n");
		return NULL; /* Not critical */
	}

	suffices = (suffix_t**)sci_malloc(sizeof(suffix_t *));

	while ((seeker < resource->size - 1) && (resource->data[seeker + 1] != 0xff)) {

		char *alt_suffix = (char *) resource->data + seeker;
		int alt_len = strlen(alt_suffix);
		char *word_suffix;
		int word_len;

		suffices = (suffix_t**)sci_realloc(suffices, sizeof(suffix_t *) * (counter + 1));

		seeker += alt_len + 1; /* Hit end of string */
		word_suffix = (char *) resource->data + seeker + 3; /* Beginning of next string +1 (ignore '*') */
		word_len = strlen(word_suffix);

		suffices[counter] = (suffix_t*)sci_malloc(sizeof(suffix_t));
		/* allocate enough memory to store the strings */

		suffices[counter]->word_suffix = word_suffix;
		suffices[counter]->alt_suffix = alt_suffix;

		suffices[counter]->alt_suffix_length = alt_len;
		suffices[counter]->word_suffix_length = word_len;
		suffices[counter]->class_mask = inverse_16(getInt16(resource->data + seeker)); /* Inverse endianness */

		seeker += word_len + 4;
		suffices[counter]->result_class = inverse_16(getInt16(resource->data + seeker));
		seeker += 3; /* Next entry */

		++counter;

	}

	*suffices_nr = counter;
	return suffices;
}



void
vocab_free_suffices(ResourceManager *resmgr, suffix_t **suffices, int suffices_nr) {
	int i;

	scir_unlock_resource(resmgr, scir_find_resource(resmgr, sci_vocab, VOCAB_RESOURCE_SUFFIX_VOCAB, 0),
	                     VOCAB_RESOURCE_SUFFIX_VOCAB, sci_vocab);

	for (i = 0; i < suffices_nr; i++)
		free(suffices[i]);

	free(suffices);
}


void
vocab_free_branches(parse_tree_branch_t *parser_branches) {
	if (parser_branches)
		free(parser_branches);
}


parse_tree_branch_t *
vocab_get_branches(ResourceManager * resmgr, int *branches_nr) {
	resource_t *resource = scir_find_resource(resmgr, sci_vocab,
	                       VOCAB_RESOURCE_PARSE_TREE_BRANCHES, 0);
	parse_tree_branch_t *retval;
	int i;

	if (!resource) {
		fprintf(stderr, "No parser tree data found!\n");
		return NULL;
	}

	*branches_nr = resource->size / 20;

	if (*branches_nr == 0) {
		fprintf(stderr, "Parser tree data is empty!\n");
		return NULL;
	}

	retval = (parse_tree_branch_t*)sci_malloc(sizeof(parse_tree_branch_t) * *branches_nr);

	for (i = 0; i < *branches_nr; i++) {
		int k;

		byte *base = resource->data + i * 20;

		retval[i].id = getInt16(base);

		for (k = 0; k < 9; k++)
			retval[i].data[k] = getUInt16(base + 2 + 2 * k);

		retval[i].data[9] = 0; /* Always terminate */
	}

	if (!retval[*branches_nr - 1].id) /* branch lists may be terminated by empty rules */
		--(*branches_nr);

	return retval;
}


result_word_t *
vocab_lookup_word(char *word, int word_len,
                  word_t **words, int words_nr,
                  suffix_t **suffices, int suffices_nr) {
	word_t *tempword = (word_t*)sci_malloc(sizeof(word_t) + word_len + 256);
	/* 256: For suffices. Should suffice. */
	word_t **dict_word;
	result_word_t *retval;
	char *tester;
	int i, word_len_tmp;

	strncpy(&(tempword->word[0]), word, word_len);
	tempword->word[word_len] = 0;

	word_len_tmp = word_len;
	while ((tester = strchr(tempword->word, '-')))
		memmove(tester, tester + 1, (tempword->word + word_len_tmp--) - tester);

	retval = (result_word_t*)sci_malloc(sizeof(result_word_t));

	dict_word = (word_t**)bsearch(&tempword, words, words_nr, sizeof(word_t *), _vocab_cmp_words);

	if (dict_word) {
		free(tempword);

		retval->w_class = (*dict_word)->w_class;
		retval->group = (*dict_word)->group;

		return retval;
	}

	/* Now try all suffices */
	for (i = 0; i < suffices_nr; i++)
		if (suffices[i]->alt_suffix_length <= word_len) {

			int suff_index = word_len - suffices[i]->alt_suffix_length;
			/* Offset of the start of the suffix */

			if (scumm_strnicmp(suffices[i]->alt_suffix, word + suff_index,
			                suffices[i]->alt_suffix_length) == 0) { /* Suffix matched! */

				strncpy(&(tempword->word[0]), word, word_len);
				tempword->word[suff_index] = 0; /* Terminate word at suffix start position... */
				strncat(&(tempword->word[0]), suffices[i]->word_suffix, suffices[i]->word_suffix_length); /* ...and append "correct" suffix */

				dict_word = (word_t**)bsearch(&tempword, words, words_nr, sizeof(word_t *), _vocab_cmp_words);

				if ((dict_word) && ((*dict_word)->w_class & suffices[i]->class_mask)) { /* Found it? */
					free(tempword);

					retval->w_class = suffices[i]->result_class; /* Use suffix class */
					retval->group = (*dict_word)->group;

					return retval;
				}
			}
		}

	/* No match so far? Check if it's a number. */

	strncpy(&(tempword->word[0]), word, word_len);
	tempword->word[word_len] = 0;

	word_len_tmp = word_len;
	while ((tester = strchr(tempword->word, '-')))
		memmove(tester, tester + 1, (tempword->word + word_len--) - tester);

	if ((strtol(&(tempword->word[0]), &tester, 10) >= 0)
	        && (*tester == '\0')) { /* Do we have a complete number here? */
		free(tempword);

		retval->group = VOCAB_MAGIC_NUMBER_GROUP;
		retval->w_class = VOCAB_CLASS_NUMBER;

		return(retval);
	}

	free(tempword);
	free(retval);
	return NULL;
}

int
vocab_get_said_spec_length(byte *addr) {
	int result = 0;

	while (*addr != 0xff) {
		if (*addr < 0xf0) {
			result += 2;
			addr += 2;
		} else {
			result += 1;
			addr += 1;
		}
	}

	return result + 1;
}

void
vocab_decypher_said_block(EngineState *s, byte *addr) {
	int nextitem;

	do {
		nextitem = *addr++;

		if (nextitem < 0xf0) {
			nextitem = nextitem << 8 | *addr++;
			sciprintf(" %s[%03x]", vocab_get_any_group_word(nextitem, s->parser_words, s->parser_words_nr),
			          nextitem);

			nextitem = 42; /* Make sure that group 0xff doesn't abort */
		} else switch (nextitem) {
			case 0xf0:
				sciprintf(" ,");
				break;
			case 0xf1:
				sciprintf(" &");
				break;
			case 0xf2:
				sciprintf(" /");
				break;
			case 0xf3:
				sciprintf(" (");
				break;
			case 0xf4:
				sciprintf(" )");
				break;
			case 0xf5:
				sciprintf(" [");
				break;
			case 0xf6:
				sciprintf(" ]");
				break;
			case 0xf7:
				sciprintf(" #");
				break;
			case 0xf8:
				sciprintf(" <");
				break;
			case 0xf9:
				sciprintf(" >");
				break;
			case 0xff:
				break;
			}
	} while (nextitem != 0xff);

	sciprintf("\n");
}


#ifdef SCI_SIMPLE_SAID_CODE

static short _related_words[][2] = { /* 0 is backwards, 1 is forward */
	{0x800, 0x180}, /* preposition */
	{0x000, 0x180}, /* article */
	{0x000, 0x180}, /* adjective */
	{0x800, 0x000}, /* pronoun */
	{0x800, 0x180}, /* noun */
	{0x000, 0x800}, /* auxiliary verb */
	{0x800, 0x800}, /* adverb */
	{0x000, 0x180}, /* verb */
	{0x000, 0x180} /* number */
};

int
vocab_build_simple_parse_tree(parse_tree_node_t *nodes, result_word_t *words, int words_nr) {
	int i, length, pos = 0;

	for (i = 0; i < words_nr; ++i) {
		if (words[i].classID != VOCAB_CLASS_ANYWORD) {
			nodes[pos].type = words[i].classID;
			nodes[pos].content.value = words[i].group;
			pos += 2; /* Link information is filled in below */
		}
	}
	nodes[pos].type = -1; /* terminate */
	length = pos >> 1;

	/* now find all referenced words */
#ifdef SCI_SIMPLE_SAID_DEBUG
	sciprintf("Semantic references:\n");
#endif

	for (i = 0; i < length; i++) {
		int j;
		int searchmask;
		int type;

		pos = (i << 1);
		type = sci_ffs(nodes[pos].type);

		if (type) {
			int found = -1;

			type -= 5; /* 1 because ffs starts counting at 1, 4 because nodes[pos].type is a nibble off */
			if (type < 0)
				type = 0;
#ifdef SCI_SIMPLE_SAID_DEBUG
			sciprintf("#%d: Word %04x: type %04x\n", i, nodes[pos].content.value, type);
#endif

			/* search backwards */
			searchmask = _related_words[type][0];
			if (searchmask) {
				for (j = i - 1; j >= 0; j--)
					if (nodes[j << 1].type & searchmask) {
						found = j << 1;
						break;
					}
			}
			nodes[pos+1].content.branches[0] = found;
#ifdef SCI_SIMPLE_SAID_DEBUG
			if (found > -1)
				sciprintf("  %d <\n", found >> 1);
#endif

			/* search forward */
			found = -1;
			searchmask = _related_words[type][1];
			if (searchmask) {
				for (j = i + 1; j < length; j++)
					if (nodes[j << 1].type & searchmask) {
						found = j << 1;
						break;
					}
			}
#ifdef SCI_SIMPLE_SAID_DEBUG
			if (found > -1)
				sciprintf("  > %d\n", found >> 1);
#endif

		} else {
#ifdef SCI_SIMPLE_SAID_DEBUG
			sciprintf("#%d: Untypified word\n", i); /* Weird, but not fatal */
#endif
			nodes[pos+1].content.branches[0] = -1;
			nodes[pos+1].content.branches[1] = -1;
		}
	}
#ifdef SCI_SIMPLE_SAID_DEBUG
	sciprintf("/Semantic references.\n");
#endif

	return 0;
}
#endif

result_word_t *
vocab_tokenize_string(char *sentence, int *result_nr,
                      word_t **words, int words_nr,
                      suffix_t **suffices, int suffices_nr,
                      char **error) {
	char *lastword = sentence;
	int pos_in_sentence = 0;
	char c;
	int wordlen = 0;
	result_word_t *retval = (result_word_t*)sci_malloc(sizeof(result_word_t));
	/* malloc'd size is always one result_word_t too big */

	result_word_t *lookup_result;


	*result_nr = 0;
	*error = NULL;

	do {

		c = sentence[pos_in_sentence++];

		if (isalnum(c) || (c == '-' && wordlen))
			++wordlen; /* Continue on this word */
		/* Words may contain a '-', but may not
		** start with one.  */

		else {

			if (wordlen) { /* Finished a word? */

				lookup_result =
				    vocab_lookup_word(lastword, wordlen,
				                      words, words_nr,
				                      suffices, suffices_nr);
				/* Look it up */

				if (!lookup_result) { /* Not found? */
					*error = (char*)sci_calloc(wordlen + 1, 1);
					strncpy(*error, lastword, wordlen); /* Set the offending word */
					free(retval);
					return NULL; /* And return with error */
				}

				memcpy(retval + *result_nr, lookup_result, sizeof(result_word_t));
				/* Copy into list */
				++(*result_nr); /* Increase number of resulting words */
				free(lookup_result);

				retval = (result_word_t*)sci_realloc(retval, sizeof(result_word_t) * (*result_nr + 1));

			}

			lastword = sentence + pos_in_sentence;
			wordlen = 0;
		}

	} while (c); /* Until terminator is hit */

	if (*result_nr == 0) {
		free(retval);
		return NULL;
	}

	return retval;
}


void
_vocab_recursive_ptree_dump_treelike(parse_tree_node_t *nodes, int nr, int prevnr) {
	if ((nr > VOCAB_TREE_NODES)/* || (nr < prevnr)*/) {
		sciprintf("Error(%04x)", nr);
		return;
	}

	if (nodes[nr].type == PARSE_TREE_NODE_LEAF)
		/*    sciprintf("[%03x]%04x", nr, nodes[nr].content.value); */
		sciprintf("%x", nodes[nr].content.value);
	else {
		int lbranch = nodes[nr].content.branches[0];
		int rbranch = nodes[nr].content.branches[1];
		/*    sciprintf("<[%03x]",nr); */
		sciprintf("<");

		if (lbranch)
			_vocab_recursive_ptree_dump_treelike(nodes, lbranch, nr);
		else sciprintf("NULL");

		sciprintf(",");

		if (rbranch)
			_vocab_recursive_ptree_dump_treelike(nodes, rbranch, nr);
		else sciprintf("NULL");

		sciprintf(">");
	}
}

void
_vocab_recursive_ptree_dump(parse_tree_node_t *nodes, int nr, int prevnr, int blanks) {
	int lbranch = nodes[nr].content.branches[0];
	int rbranch = nodes[nr].content.branches[1];
	int i;

	if (nodes[nr].type == PARSE_TREE_NODE_LEAF) {
		sciprintf("vocab_dump_parse_tree: Error: consp is nil for element %03x\n", nr);
		return;
	}

	if ((nr > VOCAB_TREE_NODES)/* || (nr < prevnr)*/) {
		sciprintf("Error(%04x))", nr);
		return;
	}

	if (lbranch) {
		if (nodes[lbranch].type == PARSE_TREE_NODE_BRANCH) {
			sciprintf("\n");
			for (i = 0; i < blanks; i++)
				sciprintf("    ");
			sciprintf("(");
			_vocab_recursive_ptree_dump(nodes, lbranch, nr, blanks + 1);
			sciprintf(")\n");
			for (i = 0; i < blanks; i++)
				sciprintf("    ");
		} else
			sciprintf("%x", nodes[lbranch].content.value);
		sciprintf(" ");
	}/* else sciprintf ("nil"); */

	if (rbranch) {
		if (nodes[rbranch].type == PARSE_TREE_NODE_BRANCH)
			_vocab_recursive_ptree_dump(nodes, rbranch, nr, blanks);
		else
			sciprintf("%x", nodes[rbranch].content.value);
	}/* else sciprintf("nil");*/
}

void
vocab_dump_parse_tree(const char *tree_name, parse_tree_node_t *nodes) {
	/*  _vocab_recursive_ptree_dump_treelike(nodes, 0, 0); */
	sciprintf("(setq %s \n'(", tree_name);
	_vocab_recursive_ptree_dump(nodes, 0, 0, 1);
	sciprintf("))\n");
}

void
vocab_synonymize_tokens(result_word_t *words, int words_nr, synonym_t *synonyms, int synonyms_nr) {
	int i, sync;

	if (!synonyms || !synonyms_nr)
		return; /* No synonyms: Nothing to check */

	for (i = 0; i < words_nr; i++)
		for (sync = 0; sync < synonyms_nr; sync++)
			if (words[i].group == synonyms[sync].replaceant)
				words[i].group = synonyms[sync].replacement;
}

} // End of namespace Sci
