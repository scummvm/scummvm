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

// Main vocabulary support functions and word lookup

#include "sci/vocabulary.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"

namespace Sci {

static int vocab_version;	// FIXME: Avoid static vars

#define VOCAB_RESOURCE_PARSE_TREE_BRANCHES vocab_version == 1 ? \
					   VOCAB_RESOURCE_SCI1_PARSE_TREE_BRANCHES : \
					   VOCAB_RESOURCE_SCI0_PARSE_TREE_BRANCHES

#define VOCAB_RESOURCE_SUFFIX_VOCAB vocab_version==1 ? \
				    VOCAB_RESOURCE_SCI1_SUFFIX_VOCAB : \
				    VOCAB_RESOURCE_SCI0_SUFFIX_VOCAB



#if 0

/**
 * Vocabulary class names.
 * These strange names were taken from an SCI01 interpreter.
 */
const char *class_names[] = {"",
                             "",
                             "conj",   // conjunction
                             "ass",    // ?
                             "pos",    // preposition ?
                             "art",    // article
                             "adj",    // adjective
                             "pron",   // pronoun
                             "noun",   // noun
                             "auxv",   // auxillary verb
                             "adv",    // adverb
                             "verb",   // verb
                             "",
                             "",
                             "",
                             ""
                            };

int *vocab_get_classes(ResourceManager *resmgr, int* count) {
	Resource* r;
	int *c;
	unsigned int i;

	if ((r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_CLASSES, 0)) == NULL)
		return 0;

	c = (int *)malloc(sizeof(int) * r->size / 2);
	for (i = 2; i < r->size; i += 4) {
		c[i/4] = READ_LE_UINT16(r->data + i);
	}
	*count = r->size / 4;

	return c;
}

int vocab_get_class_count(ResourceManager *resmgr) {
	Resource* r;

	if ((r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_CLASSES, 0)) == 0)
		return 0;

	return r->size / 4;
}

#endif

bool vocab_get_snames(ResourceManager *resmgr, bool isOldSci0, Common::StringList &selectorNames) {
	int count;

	Resource *r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SNAMES, 0);

	if (!r) // No such resource?
		return false;

	count = READ_LE_UINT16(r->data) + 1; // Counter is slightly off

	for (int i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset);

		Common::String tmp((const char *)r->data + offset + 2, len);
		selectorNames.push_back(tmp);

		// Early SCI versions used the LSB in the selector ID as a read/write
		// toggle. To compensate for that, we add every selector name twice.
		if (isOldSci0)
			selectorNames.push_back(tmp);
	}

	return true;
}

bool vocab_get_opcodes(ResourceManager *resmgr, Common::Array<opcode> &o) {
	int count, i = 0;
	Resource* r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_OPCODES, 0);

	o.clear();

	// if the resource couldn't be loaded, leave
	if (r == NULL) {
		warning("unable to load vocab.%03d", VOCAB_RESOURCE_OPCODES);
		return false;
	}

	count = READ_LE_UINT16(r->data);

	o.resize(count);
	for (i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset) - 2;
		o[i].type = READ_LE_UINT16(r->data + offset + 2);
		// QFG3 has empty opcodes
		o[i].name = len > 0 ? Common::String((char *)r->data + offset + 4, len) : "Dummy";
#if 1 //def VOCABULARY_DEBUG
		printf("Opcode %02X: %s, %d\n", i, o[i].name.c_str(), o[i].type);
#endif
	}

	return true;
}

bool vocab_get_words(ResourceManager *resmgr, WordMap &words) {

	char currentword[256] = ""; // They're not going to use words longer than 255 ;-)
	int currentwordpos = 0;

	Resource *resource;

	// First try to load the SCI0 vocab resource.
	resource = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_MAIN_VOCAB, 0);
	vocab_version = 0;

	if (!resource) {
		warning("SCI0: Could not find a main vocabulary, trying SCI01");
		resource = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_MAIN_VOCAB, 0);
		vocab_version = 1;
	}

	if (!resource) {
		warning("SCI1: Could not find a main vocabulary");
		return false; // NOT critical: SCI1 games and some demos don't have one!
	}

	unsigned int seeker;
	if (vocab_version == 1)
		seeker = 255 * 2; // vocab.900 starts with 255 16-bit pointers which we don't use
	else
		seeker = 26 * 2; // vocab.000 starts with 26 16-bit pointers which we don't use

	if (resource->size < seeker) {
		fprintf(stderr, "Invalid main vocabulary encountered: Too small\n");
		return false;
		// Now this ought to be critical, but it'll just cause parse() and said() not to work
	}

	words.clear();

	while (seeker < resource->size) {
		byte c;

		currentwordpos = resource->data[seeker++]; // Parts of previous words may be re-used

		if (vocab_version == 1) {
			c = 1;
			while (seeker < resource->size && currentwordpos < 255 && c) {
				c = resource->data[seeker++];
				currentword[currentwordpos++] = c;
			}
			if (seeker == resource->size) {
				warning("SCI1: Vocabulary not usable, disabling");
				words.clear();
				return false;
			}
		} else {
			do {
				c = resource->data[seeker++];
				currentword[currentwordpos++] = c & 0x7f; // 0x80 is used to terminate the string
			} while (c < 0x80);
		}

		currentword[currentwordpos] = 0;

		// Now decode class and group:
		c = resource->data[seeker + 1];
		ResultWord newWord;
		newWord._class = ((resource->data[seeker]) << 4) | ((c & 0xf0) >> 4);
		newWord._group = (resource->data[seeker + 2]) | ((c & 0x0f) << 8);

		// Add the word to the list
		words[currentword] = newWord;

		seeker += 3;
	}

	return true;
}

const char *vocab_get_any_group_word(int group, const WordMap &words) {
	if (group == VOCAB_MAGIC_NUMBER_GROUP)
		return "{number}";

	for (WordMap::const_iterator i = words.begin(); i != words.end(); ++i)
		if (i->_value._group == group)
			return i->_key.c_str();

	return "{invalid}";
}

bool vocab_get_suffixes(ResourceManager *resmgr, SuffixList &suffixes) {
	// FIXME: This call relies on vocab_version being set, which is done by vocab_get_words.
	// So vocab_get_words *must* be called before vocab_get_branches gets called
	Resource *resource = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SUFFIX_VOCAB, 1);
	unsigned int seeker = 1;

	if (!resource) {
		warning("Could not find suffix vocabulary");
		return false; // Not critical
	}

	while ((seeker < resource->size - 1) && (resource->data[seeker + 1] != 0xff)) {
		suffix_t suffix;

		suffix.alt_suffix = (const char *)resource->data + seeker;
		suffix.alt_suffix_length = strlen(suffix.alt_suffix);
		seeker += suffix.alt_suffix_length + 1; // Hit end of string

		suffix.class_mask = (int16)READ_BE_UINT16(resource->data + seeker);
		seeker += 2;

		// Beginning of next string - skip leading '*'
		seeker++;

		suffix.word_suffix = (const char *)resource->data + seeker;
		suffix.word_suffix_length = strlen(suffix.word_suffix);
		seeker += suffix.word_suffix_length + 1;

		suffix.result_class = (int16)READ_BE_UINT16(resource->data + seeker);
		seeker += 3; // Next entry

		suffixes.push_back(suffix);
	}

	return true;
}

void vocab_free_suffixes(ResourceManager *resmgr, SuffixList &suffixes) {
	// FIXME: This call relies on vocab_version being set, which is done by vocab_get_words.
	// So vocab_get_words *must* be called before vocab_get_branches gets called
	resmgr->unlockResource(resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SUFFIX_VOCAB, 0),
	                     VOCAB_RESOURCE_SUFFIX_VOCAB, kResourceTypeVocab);

	suffixes.clear();
}

bool vocab_get_branches(ResourceManager * resmgr, Common::Array<parse_tree_branch_t> &branches) {
	// FIXME: This call relies on vocab_version being set, which is done by vocab_get_words.
	// So vocab_get_words *must* be called before vocab_get_branches gets called
	Resource *resource = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_PARSE_TREE_BRANCHES, 0);

	branches.clear();

	if (!resource) {
		fprintf(stderr, "No parser tree data found!\n");
		return false;
	}

	int branches_nr = resource->size / 20;

	if (branches_nr == 0) {
		fprintf(stderr, "Parser tree data is empty!\n");
		return false;
	}

	branches.resize(branches_nr);

	for (int i = 0; i < branches_nr; i++) {
		byte *base = resource->data + i * 20;

		branches[i].id = (int16)READ_LE_UINT16(base);

		for (int k = 0; k < 9; k++)
			branches[i].data[k] = READ_LE_UINT16(base + 2 + 2 * k);

		branches[i].data[9] = 0; // Always terminate
	}

	if (!branches[branches_nr - 1].id) // branch lists may be terminated by empty rules
		branches.remove_at(branches_nr - 1);

	return true;
}


ResultWord vocab_lookup_word(const char *word, int word_len, const WordMap &words, const SuffixList &suffixes) {
	Common::String tempword(word, word_len);

	// Remove all dashes from tempword
	for (uint i = 0; i < tempword.size(); ) {
		if (tempword[i] == '-')
			tempword.deleteChar(i);
		else
			++i;
	}

	// Look it up:
	WordMap::iterator dict_word = words.find(tempword);

	// Match found? Return it!
	if (dict_word != words.end()) {
		return dict_word->_value;
	}

	// Now try all suffixes
	for (SuffixList::const_iterator suffix = suffixes.begin(); suffix != suffixes.end(); ++suffix)
		if (suffix->alt_suffix_length <= word_len) {

			int suff_index = word_len - suffix->alt_suffix_length;
			// Offset of the start of the suffix

			if (scumm_strnicmp(suffix->alt_suffix, word + suff_index, suffix->alt_suffix_length) == 0) { // Suffix matched!
				// Terminate word at suffix start position...:
				Common::String tempword2(word, MIN(word_len, suff_index));

				// ...and append "correct" suffix
				tempword2 += Common::String(suffix->word_suffix, suffix->word_suffix_length);

				dict_word = words.find(tempword2);

				if ((dict_word != words.end()) && (dict_word->_value._class & suffix->class_mask)) { // Found it?
					// Use suffix class
					ResultWord tmp = dict_word->_value;
					tmp._class = suffix->result_class;
					return tmp;
				}
			}
		}

	// No match so far? Check if it's a number.

	ResultWord retval = { -1, -1 };
	char *tester;
	if ((strtol(tempword.c_str(), &tester, 10) >= 0) && (*tester == '\0')) { // Do we have a complete number here?
		ResultWord tmp = { VOCAB_CLASS_NUMBER, VOCAB_MAGIC_NUMBER_GROUP };
		retval = tmp;
	}

	return retval;
}

void vocab_decypher_said_block(EngineState *s, byte *addr) {
	int nextitem;

	do {
		nextitem = *addr++;

		if (nextitem < 0xf0) {
			nextitem = nextitem << 8 | *addr++;
			sciprintf(" %s[%03x]", vocab_get_any_group_word(nextitem, s->_parserWords), nextitem);

			nextitem = 42; // Make sure that group 0xff doesn't abort
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

bool vocab_tokenize_string(ResultWordList &retval, const char *sentence, const WordMap &words,
	const SuffixList &suffixes, char **error) {
	const char *lastword = sentence;
	int pos_in_sentence = 0;
	char c;
	int wordlen = 0;

	*error = NULL;

	do {

		c = sentence[pos_in_sentence++];

		if (isalnum(c) || (c == '-' && wordlen))
			++wordlen;
		// Continue on this word */
		// Words may contain a '-', but may not
		// start with one.
		else {
			if (wordlen) { // Finished a word?

				ResultWord lookup_result =
				    vocab_lookup_word(lastword, wordlen, words, suffixes);
				// Look it up

				if (lookup_result._class == -1) { // Not found?
					*error = (char *)calloc(wordlen + 1, 1);
					strncpy(*error, lastword, wordlen); // Set the offending word
					retval.clear();
					return false; // And return with error
				}

				// Copy into list
				retval.push_back(lookup_result);
			}

			lastword = sentence + pos_in_sentence;
			wordlen = 0;
		}

	} while (c); // Until terminator is hit

	return true;
}

void _vocab_recursive_ptree_dump_treelike(parse_tree_node_t *nodes, int nr, int prevnr) {
	if ((nr > VOCAB_TREE_NODES)/* || (nr < prevnr)*/) {
		sciprintf("Error(%04x)", nr);
		return;
	}

	if (nodes[nr].type == PARSE_TREE_NODE_LEAF)
		//sciprintf("[%03x]%04x", nr, nodes[nr].content.value);
		sciprintf("%x", nodes[nr].content.value);
	else {
		int lbranch = nodes[nr].content.branches[0];
		int rbranch = nodes[nr].content.branches[1];
		//sciprintf("<[%03x]", nr);
		sciprintf("<");

		if (lbranch)
			_vocab_recursive_ptree_dump_treelike(nodes, lbranch, nr);
		else
			sciprintf("NULL");

		sciprintf(",");

		if (rbranch)
			_vocab_recursive_ptree_dump_treelike(nodes, rbranch, nr);
		else
			sciprintf("NULL");

		sciprintf(">");
	}
}

void _vocab_recursive_ptree_dump(parse_tree_node_t *nodes, int nr, int prevnr, int blanks) {
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
	}/* else sciprintf ("nil");*/

	if (rbranch) {
		if (nodes[rbranch].type == PARSE_TREE_NODE_BRANCH)
			_vocab_recursive_ptree_dump(nodes, rbranch, nr, blanks);
		else
			sciprintf("%x", nodes[rbranch].content.value);
	}/* else sciprintf("nil");*/
}

void vocab_dump_parse_tree(const char *tree_name, parse_tree_node_t *nodes) {
	//_vocab_recursive_ptree_dump_treelike(nodes, 0, 0);
	sciprintf("(setq %s \n'(", tree_name);
	_vocab_recursive_ptree_dump(nodes, 0, 0, 1);
	sciprintf("))\n");
}

void vocab_synonymize_tokens(ResultWordList &words, const SynonymList &synonyms) {
	if (synonyms.empty())
		return; // No synonyms: Nothing to check

	for (ResultWordList::iterator i = words.begin(); i != words.end(); ++i)
		for (SynonymList::const_iterator sync = synonyms.begin(); sync != synonyms.end(); ++sync)
			if (i->_group == sync->replaceant)
				i->_group = sync->replacement;
}

} // End of namespace Sci
