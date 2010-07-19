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

#include "sci/parser/vocabulary.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/console.h"

namespace Sci {

Vocabulary::Vocabulary(ResourceManager *resMan, bool foreign) : _resMan(resMan), _foreign(foreign) {
	_parserRules = NULL;

	memset(_parserNodes, 0, sizeof(_parserNodes));
	// Mark parse tree as unused
	_parserNodes[0].type = kParseTreeLeafNode;
	_parserNodes[0].content.value = 0;

	_synonyms.clear(); // No synonyms

	debug(2, "Initializing vocabulary");
	if (_resMan->testResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_MAIN_VOCAB))) {
		_vocabVersion = kVocabularySCI0;
		_resourceIdWords = VOCAB_RESOURCE_SCI0_MAIN_VOCAB;
		_resourceIdSuffixes = VOCAB_RESOURCE_SCI0_SUFFIX_VOCAB;
		_resourceIdBranches = VOCAB_RESOURCE_SCI0_PARSE_TREE_BRANCHES;
	} else {
		warning("Could not find a main vocabulary, trying SCI01");
		_vocabVersion = kVocabularySCI1;
		_resourceIdWords = VOCAB_RESOURCE_SCI1_MAIN_VOCAB;
		_resourceIdSuffixes = VOCAB_RESOURCE_SCI1_SUFFIX_VOCAB;
		_resourceIdBranches = VOCAB_RESOURCE_SCI1_PARSE_TREE_BRANCHES;
	}
	if (_foreign) {
		_resourceIdWords += 10;
		_resourceIdSuffixes += 10;
		_resourceIdBranches += 10;
	}

	if (getSciVersion() <= SCI_VERSION_1_EGA && loadParserWords()) {
		loadSuffixes();
		if (loadBranches())
			// Now build a GNF grammar out of this
			_parserRules = buildGNF();
	} else {
		debug(2, "Assuming that this game does not use a parser.");
		_parserRules = NULL;
	}

	parser_base = NULL_REG;
	parser_event = NULL_REG;
	parserIsValid = false;
}

Vocabulary::~Vocabulary() {
	freeRuleList(_parserRules);
	freeSuffixes();
}

void Vocabulary::reset() {
	parserIsValid = false; // Invalidate parser
	parser_event = NULL_REG; // Invalidate parser event
	parser_base = make_reg(g_sci->getEngineState()->_segMan->getSysStringsSegment(), SYS_STRING_PARSER_BASE);
}

bool Vocabulary::loadParserWords() {
	char currentWord[VOCAB_MAX_WORDLENGTH] = "";
	int currentWordPos = 0;

	// First try to load the SCI0 vocab resource.
	Resource *resource = _resMan->findResource(ResourceId(kResourceTypeVocab, _resourceIdWords), 0);

	if (!resource) {
		warning("Could not find a main vocabulary");
		return false; // NOT critical: SCI1 games and some demos don't have one!
	}

	VocabularyVersions resourceType = _vocabVersion;

	if (resourceType == kVocabularySCI0) {
		if (resource->size < 26 * 2) {
			warning("Invalid main vocabulary encountered: Much too small");
			return false;
		}
		// Check the alphabet-offset table for any content
		int alphabetNr;
		for (alphabetNr = 0; alphabetNr < 26; alphabetNr++) {
			if (READ_LE_UINT16(resource->data + alphabetNr * 2))
				break;
		}
		// If all of them were empty, we are definitely seeing SCI01 vocab in disguise (e.g. pq2 japanese)
		if (alphabetNr == 26) {
			warning("SCI0: Found SCI01 vocabulary in disguise");
			resourceType = kVocabularySCI1;
		}
	}

	unsigned int seeker;
	if (resourceType == kVocabularySCI1)
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

		currentWordPos = resource->data[seeker++]; // Parts of previous words may be re-used

		if (resourceType == kVocabularySCI1) {
			c = 1;
			while (seeker < resource->size && currentWordPos < 255 && c) {
				c = resource->data[seeker++];
				currentWord[currentWordPos++] = c;
			}
			if (seeker == resource->size) {
				warning("SCI1: Vocabulary not usable, disabling");
				_parserWords.clear();
				return false;
			}
		} else {
			do {
				c = resource->data[seeker++];
				currentWord[currentWordPos++] = c & 0x7f; // 0x80 is used to terminate the string
			} while (c < 0x80);
		}

		currentWord[currentWordPos] = 0;

		// Now decode class and group:
		c = resource->data[seeker + 1];
		ResultWord newWord;
		newWord._class = ((resource->data[seeker]) << 4) | ((c & 0xf0) >> 4);
		newWord._group = (resource->data[seeker + 2]) | ((c & 0x0f) << 8);

		// Add the word to the list
		_parserWords[currentWord] = newWord;

		seeker += 3;
	}

	return true;
}

const char *Vocabulary::getAnyWordFromGroup(int group) {
	if (group == VOCAB_MAGIC_NUMBER_GROUP)
		return "{number}";
	if (group == VOCAB_MAGIC_NOTHING_GROUP)
		return "{nothing}";

	for (WordMap::const_iterator i = _parserWords.begin(); i != _parserWords.end(); ++i) {
		if (i->_value._group == group)
			return i->_key.c_str();
	}

	return "{invalid}";
}

bool Vocabulary::loadSuffixes() {
	// Determine if we can find a SCI1 suffix vocabulary first
	Resource* resource = _resMan->findResource(ResourceId(kResourceTypeVocab, _resourceIdSuffixes), 1);
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
	Resource* resource = _resMan->findResource(ResourceId(kResourceTypeVocab, _resourceIdSuffixes), 0);
	if (resource)
		_resMan->unlockResource(resource);

	_parserSuffixes.clear();
}

bool Vocabulary::loadBranches() {
	Resource *resource = _resMan->findResource(ResourceId(kResourceTypeVocab, _resourceIdBranches), 0);

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

// we assume that *word points to an already lowercased word
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

			if (strncmp(suffix->alt_suffix, word + suff_index, suffix->alt_suffix_length) == 0) { // Suffix matched!
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
	uint16 nextitem;

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

static const byte lowerCaseMap[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // 0x00
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, // 0x10
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, // 0x20
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, // 0x30
	0x40,  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o', // 0x40
	 'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z', 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, // 0x50
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, // 0x60
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, // 0x70
	0x87, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x84, 0x86, // 0x80
	//^^                                                                                ^^^^  ^^^^
	0x82, 0x91, 0x91, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x94, 0x81, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, // 0x90
	//^^        ^^^^                                      ^^^^  ^^^^
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa4, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, // 0xa0
	//                            ^^^^
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, // 0xb0
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, // 0xc0
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, // 0xd0
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, // 0xe0
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff  // 0xf0
};

bool Vocabulary::tokenizeString(ResultWordList &retval, const char *sentence, char **error) {
	char currentWord[VOCAB_MAX_WORDLENGTH] = "";
	int pos_in_sentence = 0;
	unsigned char c;
	int wordLen = 0;

	*error = NULL;

	do {

		c = sentence[pos_in_sentence++];
		if (isalnum(c) || (c == '-' && wordLen) || (c >= 0x80)) {
			currentWord[wordLen] = lowerCaseMap[c];
			++wordLen;
		}
		// Continue on this word */
		// Words may contain a '-', but may not
		// start with one.
		else {
			if (wordLen) { // Finished a word?

				ResultWord lookup_result = lookupWord(currentWord, wordLen);
				// Look it up

				if (lookup_result._class == -1) { // Not found?
					*error = (char *)calloc(wordLen + 1, 1);
					strncpy(*error, currentWord, wordLen); // Set the offending word
					retval.clear();
					return false; // And return with error
				}

				// Copy into list
				retval.push_back(lookup_result);
			}

			wordLen = 0;
		}

	} while (c); // Until terminator is hit

	return true;
}

void Vocabulary::printSuffixes() const {
	char word_buf[VOCAB_MAX_WORDLENGTH], alt_buf[VOCAB_MAX_WORDLENGTH];
	Console *con = g_sci->getSciDebugger();

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
	Console *con = g_sci->getSciDebugger();

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

void Vocabulary::dumpParseTree() {
	//_vocab_recursive_ptree_dump_treelike(nodes, 0, 0);
	printf("(setq parse-tree \n'(");
	_vocab_recursive_ptree_dump(_parserNodes, 0, 0, 1);
	printf("))\n");
}

void Vocabulary::synonymizeTokens(ResultWordList &words) {
	if (_synonyms.empty())
		return; // No synonyms: Nothing to check

	for (ResultWordList::iterator i = words.begin(); i != words.end(); ++i)
		for (SynonymList::const_iterator sync = _synonyms.begin(); sync != _synonyms.end(); ++sync)
			if (i->_group == sync->replaceant)
				i->_group = sync->replacement;
}

void Vocabulary::printParserNodes(int num) {
	Console *con = g_sci->getSciDebugger();

	for (int i = 0; i < num; i++) {
		con->DebugPrintf(" Node %03x: ", i);
		if (_parserNodes[i].type == kParseTreeLeafNode)
			con->DebugPrintf("Leaf: %04x\n", _parserNodes[i].content.value);
		else
			con->DebugPrintf("Branch: ->%04x, ->%04x\n", _parserNodes[i].content.branches[0],
			          _parserNodes[i].content.branches[1]);
	}
}

int Vocabulary::parseNodes(int *i, int *pos, int type, int nr, int argc, const char **argv) {
	int nextToken = 0, nextValue = 0, newPos = 0, oldPos = 0;
	Console *con = g_sci->getSciDebugger();

	if (type == kParseNil)
		return 0;

	if (type == kParseNumber) {
		_parserNodes[*pos += 1].type = kParseTreeLeafNode;
		_parserNodes[*pos].content.value = nr;
		return *pos;
	}
	if (type == kParseEndOfInput) {
		con->DebugPrintf("Unbalanced parentheses\n");
		return -1;
	}
	if (type == kParseClosingParenthesis) {
		con->DebugPrintf("Syntax error at token %d\n", *i);
		return -1;
	}

	_parserNodes[oldPos = ++(*pos)].type = kParseTreeBranchNode;

	for (int j = 0; j <= 1; j++) {
		if (*i == argc) {
			nextToken = kParseEndOfInput;
		} else {
			const char *token = argv[(*i)++];

			if (!strcmp(token, "(")) {
				nextToken = kParseOpeningParenthesis;
			} else if (!strcmp(token, ")")) {
				nextToken = kParseClosingParenthesis;
			} else if (!strcmp(token, "nil")) {
				nextToken = kParseNil;
			} else {
				nextValue = strtol(token, NULL, 0);
				nextToken = kParseNumber;
			}
		}

		if ((newPos = _parserNodes[oldPos].content.branches[j] = parseNodes(i, pos, nextToken, nextValue, argc, argv)) == -1)
			return -1;
	}

	const char *token = argv[(*i)++];
	if (strcmp(token, ")"))
		con->DebugPrintf("Expected ')' at token %d\n", *i);

	return oldPos;
}

} // End of namespace Sci
