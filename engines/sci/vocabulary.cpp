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
#include "sci/console.h"

namespace Sci {

#if 0

#define VOCAB_RESOURCE_CLASSES 996
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

int *vocab_get_classes(ResourceManager *resMan, int* count) {
	Resource* r;
	int *c;
	unsigned int i;

	if ((r = resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_CLASSES), 0)) == NULL)
		return 0;

	c = (int *)malloc(sizeof(int) * r->size / 2);
	for (i = 2; i < r->size; i += 4) {
		c[i/4] = READ_LE_UINT16(r->data + i);
	}
	*count = r->size / 4;

	return c;
}

int vocab_get_class_count(ResourceManager *resMan) {
	Resource* r;

	if ((r = resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_CLASSES), 0)) == 0)
		return 0;

	return r->size / 4;
}

#endif

Vocabulary::Vocabulary(ResourceManager *resMan) : _resMan(resMan) {
	_parserRules = NULL;
	_vocabVersion = kVocabularySCI0;

	debug(2, "Initializing vocabulary");

	if (_resMan->sciVersion() <= SCI_VERSION_1_EGA && loadParserWords()) {
		loadSuffixes();
		if (loadBranches())
			// Now build a GNF grammar out of this
			_parserRules = buildGNF();
	} else {
		debug(2, "Assuming that this game does not use a parser.");
		_parserRules = NULL;
	}
}

Vocabulary::~Vocabulary() {
	freeRuleList(_parserRules);
	_parserWords.clear();
	_parserBranches.clear();
	freeSuffixes();
}

bool Vocabulary::loadParserWords() {

	char currentword[256] = ""; // They're not going to use words longer than 255 ;-)
	int currentwordpos = 0;

	// First try to load the SCI0 vocab resource.
	Resource *resource = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_MAIN_VOCAB), 0);
 
	if (!resource) {
		warning("SCI0: Could not find a main vocabulary, trying SCI01");
		resource = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_MAIN_VOCAB), 0);
		_vocabVersion = kVocabularySCI1;
	}

	if (!resource) {
		warning("SCI1: Could not find a main vocabulary");
		return false; // NOT critical: SCI1 games and some demos don't have one!
	}

	unsigned int seeker;
	if (_vocabVersion == kVocabularySCI1)
		seeker = 255 * 2; // vocab.900 starts with 255 16-bit pointers which we don't use
	else
		seeker = 26 * 2; // vocab.000 starts with 26 16-bit pointers which we don't use

	if (resource->size < seeker) {
		warning("Invalid main vocabulary encountered: Too small");
		return false;
		// Now this ought to be critical, but it'll just cause parse() and said() not to work
	}

	_parserWords.clear();

	while (seeker < resource->size) {
		byte c;

		currentwordpos = resource->data[seeker++]; // Parts of previous words may be re-used

		if (_vocabVersion == kVocabularySCI1) {
			c = 1;
			while (seeker < resource->size && currentwordpos < 255 && c) {
				c = resource->data[seeker++];
				currentword[currentwordpos++] = c;
			}
			if (seeker == resource->size) {
				warning("SCI1: Vocabulary not usable, disabling");
				_parserWords.clear();
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
		_parserWords[currentword] = newWord;

		seeker += 3;
	}

	return true;
}

const char *Vocabulary::getAnyWordFromGroup(int group) {
	if (group == VOCAB_MAGIC_NUMBER_GROUP)
		return "{number}";

	for (WordMap::const_iterator i = _parserWords.begin(); i != _parserWords.end(); ++i)
		if (i->_value._group == group)
			return i->_key.c_str();

	return "{invalid}";
}

bool Vocabulary::loadSuffixes() {
	// Determine if we can find a SCI1 suffix vocabulary first
	Resource* resource = NULL;
	
	if (_vocabVersion == kVocabularySCI0)
		resource = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_SUFFIX_VOCAB), 1);
	else
		resource = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_SUFFIX_VOCAB), 1);

	if (!resource)
		return false; // No vocabulary found

	unsigned int seeker = 1;

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

		_parserSuffixes.push_back(suffix);
	}

	return true;
}

void Vocabulary::freeSuffixes() {
	Resource* resource = NULL;
	
	if (_vocabVersion == kVocabularySCI0)
		resource = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_SUFFIX_VOCAB), 0);
	else
		resource = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_SUFFIX_VOCAB), 0);
	
	if (resource)
		_resMan->unlockResource(resource);

	_parserSuffixes.clear();
}

bool Vocabulary::loadBranches() {
	Resource *resource = NULL;

	if (_vocabVersion == kVocabularySCI0)
		resource = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_PARSE_TREE_BRANCHES), 0);
	else
		resource = _resMan->findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_PARSE_TREE_BRANCHES), 0);

	_parserBranches.clear();

	if (!resource)
		return false;		// No parser tree data found

	int branches_nr = resource->size / 20;

	if (branches_nr == 0) {
		warning("Parser tree data is empty");
		return false;
	}

	_parserBranches.resize(branches_nr);

	for (int i = 0; i < branches_nr; i++) {
		byte *base = resource->data + i * 20;

		_parserBranches[i].id = (int16)READ_LE_UINT16(base);

		for (int k = 0; k < 9; k++)
			_parserBranches[i].data[k] = READ_LE_UINT16(base + 2 + 2 * k);

		_parserBranches[i].data[9] = 0; // Always terminate
	}

	if (!_parserBranches[branches_nr - 1].id) // branch lists may be terminated by empty rules
		_parserBranches.remove_at(branches_nr - 1);

	return true;
}


ResultWord Vocabulary::lookupWord(const char *word, int word_len) {
	Common::String tempword(word, word_len);

	// Remove all dashes from tempword
	for (uint i = 0; i < tempword.size(); ) {
		if (tempword[i] == '-')
			tempword.deleteChar(i);
		else
			++i;
	}

	// Look it up:
	WordMap::iterator dict_word = _parserWords.find(tempword);

	// Match found? Return it!
	if (dict_word != _parserWords.end()) {
		return dict_word->_value;
	}

	// Now try all suffixes
	for (SuffixList::const_iterator suffix = _parserSuffixes.begin(); suffix != _parserSuffixes.end(); ++suffix)
		if (suffix->alt_suffix_length <= word_len) {

			int suff_index = word_len - suffix->alt_suffix_length;
			// Offset of the start of the suffix

			if (scumm_strnicmp(suffix->alt_suffix, word + suff_index, suffix->alt_suffix_length) == 0) { // Suffix matched!
				// Terminate word at suffix start position...:
				Common::String tempword2(word, MIN(word_len, suff_index));

				// ...and append "correct" suffix
				tempword2 += Common::String(suffix->word_suffix, suffix->word_suffix_length);

				dict_word = _parserWords.find(tempword2);

				if ((dict_word != _parserWords.end()) && (dict_word->_value._class & suffix->class_mask)) { // Found it?
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

void Vocabulary::decipherSaidBlock(byte *addr) {
	byte nextitem;

	do {
		nextitem = *addr++;

		if (nextitem < 0xf0) {
			nextitem = nextitem << 8 | *addr++;
			printf(" %s[%03x]", getAnyWordFromGroup(nextitem), nextitem);

			nextitem = 42; // Make sure that group 0xff doesn't abort
		} else switch (nextitem) {
			case 0xf0:
				printf(" ,");
				break;
			case 0xf1:
				printf(" &");
				break;
			case 0xf2:
				printf(" /");
				break;
			case 0xf3:
				printf(" (");
				break;
			case 0xf4:
				printf(" )");
				break;
			case 0xf5:
				printf(" [");
				break;
			case 0xf6:
				printf(" ]");
				break;
			case 0xf7:
				printf(" #");
				break;
			case 0xf8:
				printf(" <");
				break;
			case 0xf9:
				printf(" >");
				break;
			case 0xff:
				break;
			}
	} while (nextitem != 0xff);

	printf("\n");
}

bool Vocabulary::tokenizeString(ResultWordList &retval, const char *sentence, char **error) {
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

				ResultWord lookup_result = lookupWord(lastword, wordlen);
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

void Vocabulary::printSuffixes() const {
	char word_buf[256], alt_buf[256];
	Console *con = ((SciEngine *)g_engine)->getSciDebugger();

	int i = 0;
	for (SuffixList::const_iterator suf = _parserSuffixes.begin(); suf != _parserSuffixes.end(); ++suf) {
		strncpy(word_buf, suf->word_suffix, suf->word_suffix_length);
		word_buf[suf->word_suffix_length] = 0;
		strncpy(alt_buf, suf->alt_suffix, suf->alt_suffix_length);
		alt_buf[suf->alt_suffix_length] = 0;

		con->DebugPrintf("%4d: (%03x) -%12s  =>  -%12s (%03x)\n", i, suf->class_mask, word_buf, alt_buf, suf->result_class);
		++i;
	}
}

void Vocabulary::printParserWords() const {
	Console *con = ((SciEngine *)g_engine)->getSciDebugger();

	int j = 0;
	for (WordMap::iterator i = _parserWords.begin(); i != _parserWords.end(); ++i) {
		con->DebugPrintf("%4d: %03x [%03x] %20s |", j, i->_value._class, i->_value._group, i->_key.c_str());
		if (j % 3 == 0)
			con->DebugPrintf("\n");
		j++;
	}

	con->DebugPrintf("\n");
}

void _vocab_recursive_ptree_dump_treelike(parse_tree_node_t *nodes, int nr, int prevnr) {
	if ((nr > VOCAB_TREE_NODES)/* || (nr < prevnr)*/) {
		printf("Error(%04x)", nr);
		return;
	}

	if (nodes[nr].type == kParseTreeLeafNode)
		//printf("[%03x]%04x", nr, nodes[nr].content.value);
		printf("%x", nodes[nr].content.value);
	else {
		int lbranch = nodes[nr].content.branches[0];
		int rbranch = nodes[nr].content.branches[1];
		//printf("<[%03x]", nr);
		printf("<");

		if (lbranch)
			_vocab_recursive_ptree_dump_treelike(nodes, lbranch, nr);
		else
			printf("NULL");

		printf(",");

		if (rbranch)
			_vocab_recursive_ptree_dump_treelike(nodes, rbranch, nr);
		else
			printf("NULL");

		printf(">");
	}
}

void _vocab_recursive_ptree_dump(parse_tree_node_t *nodes, int nr, int prevnr, int blanks) {
	int lbranch = nodes[nr].content.branches[0];
	int rbranch = nodes[nr].content.branches[1];
	int i;

	if (nodes[nr].type == kParseTreeLeafNode) {
		printf("vocab_dump_parse_tree: Error: consp is nil for element %03x\n", nr);
		return;
	}

	if ((nr > VOCAB_TREE_NODES)/* || (nr < prevnr)*/) {
		printf("Error(%04x))", nr);
		return;
	}

	if (lbranch) {
		if (nodes[lbranch].type == kParseTreeBranchNode) {
			printf("\n");
			for (i = 0; i < blanks; i++)
				printf("    ");
			printf("(");
			_vocab_recursive_ptree_dump(nodes, lbranch, nr, blanks + 1);
			printf(")\n");
			for (i = 0; i < blanks; i++)
				printf("    ");
		} else
			printf("%x", nodes[lbranch].content.value);
		printf(" ");
	}/* else printf ("nil");*/

	if (rbranch) {
		if (nodes[rbranch].type == kParseTreeBranchNode)
			_vocab_recursive_ptree_dump(nodes, rbranch, nr, blanks);
		else
			printf("%x", nodes[rbranch].content.value);
	}/* else printf("nil");*/
}

void vocab_dump_parse_tree(const char *tree_name, parse_tree_node_t *nodes) {
	//_vocab_recursive_ptree_dump_treelike(nodes, 0, 0);
	printf("(setq %s \n'(", tree_name);
	_vocab_recursive_ptree_dump(nodes, 0, 0, 1);
	printf("))\n");
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
