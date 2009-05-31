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

/** The string used to identify the "unknown" SCI0 function for each game */
#define SCRIPT_UNKNOWN_FUNCTION_STRING "[Unknown]"

// Default kernel name table
#define SCI0_KNAMES_WELL_DEFINED 0x6e
#define SCI0_KNAMES_DEFAULT_ENTRIES_NR 0x72
#define SCI1_KNAMES_DEFAULT_ENTRIES_NR 0x89

static const char *sci0_default_knames[SCI0_KNAMES_DEFAULT_ENTRIES_NR] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Clone",
	/*0x05*/ "DisposeClone",
	/*0x06*/ "IsObject",
	/*0x07*/ "RespondsTo",
	/*0x08*/ "DrawPic",
	/*0x09*/ "Show",
	/*0x0a*/ "PicNotValid",
	/*0x0b*/ "Animate",
	/*0x0c*/ "SetNowSeen",
	/*0x0d*/ "NumLoops",
	/*0x0e*/ "NumCels",
	/*0x0f*/ "CelWide",
	/*0x10*/ "CelHigh",
	/*0x11*/ "DrawCel",
	/*0x12*/ "AddToPic",
	/*0x13*/ "NewWindow",
	/*0x14*/ "GetPort",
	/*0x15*/ "SetPort",
	/*0x16*/ "DisposeWindow",
	/*0x17*/ "DrawControl",
	/*0x18*/ "HiliteControl",
	/*0x19*/ "EditControl",
	/*0x1a*/ "TextSize",
	/*0x1b*/ "Display",
	/*0x1c*/ "GetEvent",
	/*0x1d*/ "GlobalToLocal",
	/*0x1e*/ "LocalToGlobal",
	/*0x1f*/ "MapKeyToDir",
	/*0x20*/ "DrawMenuBar",
	/*0x21*/ "MenuSelect",
	/*0x22*/ "AddMenu",
	/*0x23*/ "DrawStatus",
	/*0x24*/ "Parse",
	/*0x25*/ "Said",
	/*0x26*/ "SetSynonyms",
	/*0x27*/ "HaveMouse",
	/*0x28*/ "SetCursor",
	/*0x29*/ "FOpen",
	/*0x2a*/ "FPuts",
	/*0x2b*/ "FGets",
	/*0x2c*/ "FClose",
	/*0x2d*/ "SaveGame",
	/*0x2e*/ "RestoreGame",
	/*0x2f*/ "RestartGame",
	/*0x30*/ "GameIsRestarting",
	/*0x31*/ "DoSound",
	/*0x32*/ "NewList",
	/*0x33*/ "DisposeList",
	/*0x34*/ "NewNode",
	/*0x35*/ "FirstNode",
	/*0x36*/ "LastNode",
	/*0x37*/ "EmptyList",
	/*0x38*/ "NextNode",
	/*0x39*/ "PrevNode",
	/*0x3a*/ "NodeValue",
	/*0x3b*/ "AddAfter",
	/*0x3c*/ "AddToFront",
	/*0x3d*/ "AddToEnd",
	/*0x3e*/ "FindKey",
	/*0x3f*/ "DeleteKey",
	/*0x40*/ "Random",
	/*0x41*/ "Abs",
	/*0x42*/ "Sqrt",
	/*0x43*/ "GetAngle",
	/*0x44*/ "GetDistance",
	/*0x45*/ "Wait",
	/*0x46*/ "GetTime",
	/*0x47*/ "StrEnd",
	/*0x48*/ "StrCat",
	/*0x49*/ "StrCmp",
	/*0x4a*/ "StrLen",
	/*0x4b*/ "StrCpy",
	/*0x4c*/ "Format",
	/*0x4d*/ "GetFarText",
	/*0x4e*/ "ReadNumber",
	/*0x4f*/ "BaseSetter",
	/*0x50*/ "DirLoop",
	/*0x51*/ "CanBeHere",
	/*0x52*/ "OnControl",
	/*0x53*/ "InitBresen",
	/*0x54*/ "DoBresen",
	/*0x55*/ "DoAvoider",
	/*0x56*/ "SetJump",
	/*0x57*/ "SetDebug",
	/*0x58*/ "InspectObj",
	/*0x59*/ "ShowSends",
	/*0x5a*/ "ShowObjs",
	/*0x5b*/ "ShowFree",
	/*0x5c*/ "MemoryInfo",
	/*0x5d*/ "StackUsage",
	/*0x5e*/ "Profiler",
	/*0x5f*/ "GetMenu",
	/*0x60*/ "SetMenu",
	/*0x61*/ "GetSaveFiles",
	/*0x62*/ "GetCWD",
	/*0x63*/ "CheckFreeSpace",
	/*0x64*/ "ValidPath",
	/*0x65*/ "CoordPri",
	/*0x66*/ "StrAt",
	/*0x67*/ "DeviceInfo",
	/*0x68*/ "GetSaveDir",
	/*0x69*/ "CheckSaveGame",
	/*0x6a*/ "ShakeScreen",
	/*0x6b*/ "FlushResources",
	/*0x6c*/ "SinMult",
	/*0x6d*/ "CosMult",
	/*0x6e*/ "SinDiv",
	/*0x6f*/ "CosDiv",
	/*0x70*/ "Graph",
	/*0x71*/ SCRIPT_UNKNOWN_FUNCTION_STRING
};

static const char *sci1_default_knames[SCI1_KNAMES_DEFAULT_ENTRIES_NR] = {
	/*0x00*/ "Load",
	/*0x01*/ "UnLoad",
	/*0x02*/ "ScriptID",
	/*0x03*/ "DisposeScript",
	/*0x04*/ "Clone",
	/*0x05*/ "DisposeClone",
	/*0x06*/ "IsObject",
	/*0x07*/ "RespondsTo",
	/*0x08*/ "DrawPic",
	/*0x09*/ "Show",
	/*0x0a*/ "PicNotValid",
	/*0x0b*/ "Animate",
	/*0x0c*/ "SetNowSeen",
	/*0x0d*/ "NumLoops",
	/*0x0e*/ "NumCels",
	/*0x0f*/ "CelWide",
	/*0x10*/ "CelHigh",
	/*0x11*/ "DrawCel",
	/*0x12*/ "AddToPic",
	/*0x13*/ "NewWindow",
	/*0x14*/ "GetPort",
	/*0x15*/ "SetPort",
	/*0x16*/ "DisposeWindow",
	/*0x17*/ "DrawControl",
	/*0x18*/ "HiliteControl",
	/*0x19*/ "EditControl",
	/*0x1a*/ "TextSize",
	/*0x1b*/ "Display",
	/*0x1c*/ "GetEvent",
	/*0x1d*/ "GlobalToLocal",
	/*0x1e*/ "LocalToGlobal",
	/*0x1f*/ "MapKeyToDir",
	/*0x20*/ "DrawMenuBar",
	/*0x21*/ "MenuSelect",
	/*0x22*/ "AddMenu",
	/*0x23*/ "DrawStatus",
	/*0x24*/ "Parse",
	/*0x25*/ "Said",
	/*0x26*/ "SetSynonyms",
	/*0x27*/ "HaveMouse",
	/*0x28*/ "SetCursor",
	/*0x29*/ "SaveGame",
	/*0x2a*/ "RestoreGame",
	/*0x2b*/ "RestartGame",
	/*0x2c*/ "GameIsRestarting",
	/*0x2d*/ "DoSound",
	/*0x2e*/ "NewList",
	/*0x2f*/ "DisposeList",
	/*0x30*/ "NewNode",
	/*0x31*/ "FirstNode",
	/*0x32*/ "LastNode",
	/*0x33*/ "EmptyList",
	/*0x34*/ "NextNode",
	/*0x35*/ "PrevNode",
	/*0x36*/ "NodeValue",
	/*0x37*/ "AddAfter",
	/*0x38*/ "AddToFront",
	/*0x39*/ "AddToEnd",
	/*0x3a*/ "FindKey",
	/*0x3b*/ "DeleteKey",
	/*0x3c*/ "Random",
	/*0x3d*/ "Abs",
	/*0x3e*/ "Sqrt",
	/*0x3f*/ "GetAngle",
	/*0x40*/ "GetDistance",
	/*0x41*/ "Wait",
	/*0x42*/ "GetTime",
	/*0x43*/ "StrEnd",
	/*0x44*/ "StrCat",
	/*0x45*/ "StrCmp",
	/*0x46*/ "StrLen",
	/*0x47*/ "StrCpy",
	/*0x48*/ "Format",
	/*0x49*/ "GetFarText",
	/*0x4a*/ "ReadNumber",
	/*0x4b*/ "BaseSetter",
	/*0x4c*/ "DirLoop",
	/*0x4d*/ "CanBeHere",
	/*0x4e*/ "OnControl",
	/*0x4f*/ "InitBresen",
	/*0x50*/ "DoBresen",
	/*0x51*/ "Platform",
	/*0x52*/ "SetJump",
	/*0x53*/ "SetDebug",
	/*0x54*/ "InspectObj",
	/*0x55*/ "ShowSends",
	/*0x56*/ "ShowObjs",
	/*0x57*/ "ShowFree",
	/*0x58*/ "MemoryInfo",
	/*0x59*/ "StackUsage",
	/*0x5a*/ "Profiler",
	/*0x5b*/ "GetMenu",
	/*0x5c*/ "SetMenu",
	/*0x5d*/ "GetSaveFiles",
	/*0x5e*/ "GetCWD",
	/*0x5f*/ "CheckFreeSpace",
	/*0x60*/ "ValidPath",
	/*0x61*/ "CoordPri",
	/*0x62*/ "StrAt",
	/*0x63*/ "DeviceInfo",
	/*0x64*/ "GetSaveDir",
	/*0x65*/ "CheckSaveGame",
	/*0x66*/ "ShakeScreen",
	/*0x67*/ "FlushResources",
	/*0x68*/ "SinMult",
	/*0x69*/ "CosMult",
	/*0x6a*/ "SinDiv",
	/*0x6b*/ "CosDiv",
	/*0x6c*/ "Graph",
	/*0x6d*/ "Joystick",
	/*0x6e*/ "ShiftScreen",
	/*0x6f*/ "Palette",
	/*0x70*/ "MemorySegment",
	/*0x71*/ "MoveCursor",
	/*0x72*/ "Memory",
	/*0x73*/ "ListOps",
	/*0x74*/ "FileIO",
	/*0x75*/ "DoAudio",
	/*0x76*/ "DoSync",
	/*0x77*/ "AvoidPath",
	/*0x78*/ "Sort",
	/*0x79*/ "ATan",
	/*0x7a*/ "Lock",
	/*0x7b*/ "StrSplit",
	/*0x7c*/ "Message",
	/*0x7d*/ "IsItSkip",
	/*0x7e*/ "MergePoly",
	/*0x7f*/ "ResCheck",
	/*0x80*/ "AssertPalette",
	/*0x81*/ "TextColors",
	/*0x82*/ "TextFonts",
	/*0x83*/ "Record",
	/*0x84*/ "PlayBack",
	/*0x85*/ "ShowMovie",
	/*0x86*/ "SetVideoMode",
	/*0x87*/ "SetQuitStr",
	/*0x88*/ "DbugStr"
};

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

Vocabulary::Vocabulary(EngineState *s) : _resmgr(s->resmgr), _isOldSci0(s->flags & GF_SCI0_OLD) {
	s->parser_rules = NULL;
	_vocabVersion = 0;
	memset(&_selectorMap, 0, sizeof(_selectorMap));	// FIXME: Remove this once/if we C++ify selector_map_t

	debug(2, "Initializing vocabulary");

	if (_resmgr->_sciVersion < SCI_VERSION_01_VGA && getParserWords()) {
		getSuffixes();
		if (getBranches())
			// Now build a GNF grammar out of this
			s->parser_rules = vocab_build_gnf(_parserBranches, 0);
	} else {
		debug(2, "Assuming that this game does not use a parser.");
		s->parser_rules = NULL;
	}

	getOpcodes();

	if (!getSelectorNames()) {
		error("Vocabulary: Could not retrieve selector names");
	}

	// Map a few special selectors for later use
	script_map_selectors(&_selectorNames, &_selectorMap);

	getKernelNames();
}

Vocabulary::~Vocabulary() {
	_parserWords.clear();
	_selectorNames.clear();
	_opcodes.clear();
	_kernelNames.clear();
	_parserBranches.clear();
	freeSuffixes();
}

bool Vocabulary::getSelectorNames() {
	int count;

	Resource *r = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SNAMES, 0);

	if (!r) // No such resource?
		return false;

	count = READ_LE_UINT16(r->data) + 1; // Counter is slightly off

	for (int i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset);

		Common::String tmp((const char *)r->data + offset + 2, len);
		_selectorNames.push_back(tmp);

		// Early SCI versions used the LSB in the selector ID as a read/write
		// toggle. To compensate for that, we add every selector name twice.
		if (_isOldSci0)
			_selectorNames.push_back(tmp);
	}

	return true;
}

bool Vocabulary::getOpcodes() {
	int count, i = 0;
	Resource* r = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_OPCODES, 0);

	_opcodes.clear();

	// if the resource couldn't be loaded, leave
	if (r == NULL) {
		warning("unable to load vocab.%03d", VOCAB_RESOURCE_OPCODES);
		return false;
	}

	count = READ_LE_UINT16(r->data);

	_opcodes.resize(count);
	for (i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset) - 2;
		_opcodes[i].type = READ_LE_UINT16(r->data + offset + 2);
		// QFG3 has empty opcodes
		_opcodes[i].name = len > 0 ? Common::String((char *)r->data + offset + 4, len) : "Dummy";
#if 1 //def VOCABULARY_DEBUG
		printf("Opcode %02X: %s, %d\n", i, _opcodes[i].name.c_str(), _opcodes[i].type);
#endif
	}

	return true;
}

bool Vocabulary::getParserWords() {

	char currentword[256] = ""; // They're not going to use words longer than 255 ;-)
	int currentwordpos = 0;

	// First try to load the SCI0 vocab resource.
	Resource *resource = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_MAIN_VOCAB, 0);
 
	if (!resource) {
		warning("SCI0: Could not find a main vocabulary, trying SCI01");
		resource = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_MAIN_VOCAB, 0);
		_vocabVersion = 1;
	}

	if (!resource) {
		warning("SCI1: Could not find a main vocabulary");
		return false; // NOT critical: SCI1 games and some demos don't have one!
	}

	unsigned int seeker;
	if (_vocabVersion == 1)
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

		if (_vocabVersion == 1) {
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

bool Vocabulary::getSuffixes() {
	// Determine if we can find a SCI1 suffix vocabulary first
	Resource* resource = NULL;
	
	if (_vocabVersion == 0)
		resource = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_SUFFIX_VOCAB, 1);
	else
		resource = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_SUFFIX_VOCAB, 1);

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
	
	if (_vocabVersion == 0)
		resource = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_SUFFIX_VOCAB, 0);
	else
		resource = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_SUFFIX_VOCAB, 0);
	
	if (resource && resource->status == kResStatusLocked)
		_resmgr->unlockResource(resource, resource->number, kResourceTypeVocab);

	_parserSuffixes.clear();
}

bool Vocabulary::getBranches() {
	Resource *resource = NULL;

	if (_vocabVersion == 0)
		resource = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_PARSE_TREE_BRANCHES, 0);
	else
		resource = _resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_PARSE_TREE_BRANCHES, 0);

	_parserBranches.clear();

	if (!resource)
		return false;		// No parser tree data found

	int branches_nr = resource->size / 20;

	if (branches_nr == 0) {
		warning("Parser tree data is empty!");
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

void vocab_decypher_said_block(EngineState *s, byte *addr) {
	int nextitem;

	do {
		nextitem = *addr++;

		if (nextitem < 0xf0) {
			nextitem = nextitem << 8 | *addr++;
			sciprintf(" %s[%03x]", s->_vocabulary->getAnyWordFromGroup(nextitem), nextitem);

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

// Alternative kernel func names retriever. Required for KQ1/SCI (at least).
static void _vocab_get_knames0alt(const Resource *r, Common::StringList &names) {
	uint idx = 0;

	while (idx < r->size) {
		Common::String tmp((const char *)r->data + idx);
		names.push_back(tmp);
		idx += tmp.size() + 1;
	}

	// The mystery kernel function- one in each SCI0 package
	names.push_back(SCRIPT_UNKNOWN_FUNCTION_STRING);
}

static void vocab_get_knames0(ResourceManager *resmgr, Common::StringList &names) {
	int count, i, index = 2, empty_to_add = 1;
	Resource *r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_KNAMES, 0);

	if (!r) { // No kernel name table found? Fall back to default table
		names.resize(SCI0_KNAMES_DEFAULT_ENTRIES_NR);
		for (i = 0; i < SCI0_KNAMES_DEFAULT_ENTRIES_NR; i++)
			names[i] = sci0_default_knames[i];
		return;
	}

	count = READ_LE_UINT16(r->data);

	if (count > 1023) {
		_vocab_get_knames0alt(r, names);
		return;
	}

	if (count < SCI0_KNAMES_WELL_DEFINED) {
		empty_to_add = SCI0_KNAMES_WELL_DEFINED - count;
		sciprintf("Less than %d kernel functions; adding %d\n", SCI0_KNAMES_WELL_DEFINED, empty_to_add);
	}

	names.resize(count + 1 + empty_to_add);

	for (i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + index);
		int len = READ_LE_UINT16(r->data + offset);
		//fprintf(stderr,"Getting name %d of %d...\n", i, count);
		index += 2;
		names[i] = Common::String((const char *)r->data + offset + 2, len);
	}

	for (i = 0; i < empty_to_add; i++) {
		names[count + i] = SCRIPT_UNKNOWN_FUNCTION_STRING;
	}
}

static void vocab_get_knames1(ResourceManager *resmgr, Common::StringList &names) {
	// vocab.999/999.voc is notoriously unreliable in SCI1 games, and should not be used
	// We hardcode the default SCI1 kernel names here (i.e. the ones inside the "special"
	// 999.voc file from FreeSCI). All SCI1 games seem to be working with this change, but
	// if any SCI1 game has different kernel vocabulary names, it might not work. It seems
	// that all SCI1 games use the same kernel vocabulary names though, so this seems to be
	// a safe change. If there's any SCI1 game with different kernel vocabulary names, we can
	// add special flags to it to our detector

	names.resize(SCI1_KNAMES_DEFAULT_ENTRIES_NR);
	for (int i = 0; i < SCI1_KNAMES_DEFAULT_ENTRIES_NR; i++)
		names[i] = sci1_default_knames[i];
}

#ifdef ENABLE_SCI32
static void vocab_get_knames11(ResourceManager *resmgr, Common::StringList &names) {
/*
 999.voc format for SCI1.1 games:
	[b] # of kernel functions
	[w] unknown
	[offset to function name info]
		...
    {[w name-len][function name]}
		...
*/
	//unsigned int size = 64, pos = 3;
	int len;
	Resource *r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_KNAMES, 0);
	if(r == NULL) // failed to open vocab.999 (happens with SCI1 demos)
		return; // FIXME: should return a default table for this engine
	const byte nCnt = *r->data;

	names.resize(nCnt);
	for (int i = 0; i < nCnt; i++) {
		int off = READ_LE_UINT16(r->data + 2 * i + 2);
		len = READ_LE_UINT16(r->data + off);
		names[i] = Common::String((char *)r->data + off + 2, len);
	}
}
#endif

bool Vocabulary::getKernelNames() {
	_kernelNames.clear();

	switch (_resmgr->_sciVersion) {
	case SCI_VERSION_0:
	case SCI_VERSION_01:
		vocab_get_knames0(_resmgr, _kernelNames);
		break;
	case SCI_VERSION_01_VGA:
	case SCI_VERSION_01_VGA_ODD:
		// HACK: KQ5 needs the SCI1 default vocabulary names to work correctly.
		// Having more vocabulary names (like in SCI1) doesn't seem to have any
		// ill effects, other than resulting in unmapped functions towards the
		// end, which are never used by the game interpreter anyway
		// return vocab_get_knames0(resmgr, count);
	case SCI_VERSION_1_EARLY:
	case SCI_VERSION_1_LATE:
		vocab_get_knames1(_resmgr, _kernelNames);
		break;
	case SCI_VERSION_1_1:
		vocab_get_knames1(_resmgr, _kernelNames);
		// KQ6CD calls unimplemented function 0x26
                _kernelNames[0x26] = "Dummy";
		break;
#ifdef ENABLE_SCI32
	case SCI_VERSION_32:
		vocab_get_knames11(_resmgr, _kernelNames);
#endif
		break;
	default:
		break;
	}

	return true;
}

} // End of namespace Sci
