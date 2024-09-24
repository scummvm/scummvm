/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/parser.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/ac/words_dictionary.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_compat.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;




int Parser_FindWordID(const char *wordToFind) {
	return find_word_in_dictionary(wordToFind);
}

const char *Parser_SaidUnknownWord() {
	if (_GP(play).bad_parsed_word[0] == 0)
		return nullptr;
	return CreateNewScriptString(_GP(play).bad_parsed_word);
}

void ParseText(const char *text) {
	parse_sentence(text, &_GP(play).num_parsed_words, _GP(play).parsed_words, nullptr, 0);
}

// Said: call with argument for example "get apple"; we then check
// word by word if it matches (using dictonary ID equivalence to match
// synonyms). Returns 1 if it does, 0 if not.
int Said(const char *checkwords) {
	int numword = 0;
	short words[MAX_PARSED_WORDS];
	return parse_sentence(checkwords, &numword, &words[0], _GP(play).parsed_words, _GP(play).num_parsed_words);
}

//=============================================================================

int find_word_in_dictionary(const char *lookfor) {
	int j;
	if (_GP(game).dict == nullptr)
		return -1;

	for (j = 0; j < _GP(game).dict->num_words; j++) {
		if (ags_stricmp(lookfor, _GP(game).dict->word[j]) == 0) {
			return _GP(game).dict->wordnum[j];
		}
	}
	if (lookfor[0] != 0) {
		// If the word wasn't found, but it ends in 'S', see if there's
		// a non-plural version
		const char *ptat = &lookfor[strlen(lookfor) - 1];
		char lastletter = *ptat;
		if ((lastletter == 's') || (lastletter == 'S') || (lastletter == '\'')) {
			String singular = lookfor;
			singular.ClipRight(1);
			return find_word_in_dictionary(singular.GetCStr());
		}
	}
	return -1;
}

int is_valid_word_char(char theChar) {
	if ((Common::isAlnum((unsigned char)theChar)) || (theChar == '\'') || (theChar == '-')) {
		return 1;
	}
	return 0;
}

int FindMatchingMultiWordWord(char *thisword, const char **text) {
	// see if there are any multi-word words
	// that match -- if so, use them
	const char *tempptr = *text;
	char tempword[150] = "";
	if (thisword != nullptr)
		Common::strcpy_s(tempword, thisword);

	int bestMatchFound = -1, word;
	const char *tempptrAtBestMatch = tempptr;

	do {
		// extract and concat the next word
		Common::strcat_s(tempword, " ");
		while (tempptr[0] == ' ') tempptr++;
		char chbuffer[2];
		while (is_valid_word_char(tempptr[0])) {
			snprintf(chbuffer, sizeof(chbuffer), "%c", tempptr[0]);
			Common::strcat_s(tempword, chbuffer);
			tempptr++;
		}
		// is this it?
		word = find_word_in_dictionary(tempword);
		// take the longest match we find
		if (word >= 0) {
			bestMatchFound = word;
			tempptrAtBestMatch = tempptr;
		}

	} while (tempptr[0] == ' ');

	word = bestMatchFound;

	if (word >= 0) {
		// yes, a word like "pick up" was found
		*text = tempptrAtBestMatch;
		if (thisword != nullptr)
			Common::strcpy_s(thisword, 150, tempword);
	}

	return word;
}

// parse_sentence: pass compareto as NULL to parse the sentence, or
// compareto as non-null to check if it matches the passed sentence
int parse_sentence(const char *src_text, int *numwords, short *wordarray, short *compareto, int comparetonum) {
	char thisword[150] = "\0";
	int  i = 0, comparing = 0;
	int8 in_optional = 0, do_word_now = 0;
	int  optional_start = 0;

	numwords[0] = 0;
	if (compareto == nullptr)
		_GP(play).bad_parsed_word[0] = 0;

	String uniform_text = src_text;
	uniform_text.MakeLower();
	const char *text = uniform_text.GetCStr();
	while (1) {
		if ((compareto != nullptr) && (compareto[comparing] == RESTOFLINE))
			return 1;

		if ((text[0] == ']') && (compareto != nullptr)) {
			if (!in_optional)
				quitprintf("!Said: unexpected ']'\nText: %s", src_text);
			do_word_now = 1;
		}

		if (is_valid_word_char(text[0])) {
			// Part of a word, add it on
			thisword[i] = text[0];
			i++;
		} else if ((text[0] == '[') && (compareto != nullptr)) {
			if (in_optional)
				quitprintf("!Said: nested optional words\nText: %s", src_text);

			in_optional = 1;
			optional_start = comparing;
		} else if ((thisword[0] != 0) || ((text[0] == 0) && (i > 0)) || (do_word_now == 1)) {
			// End of word, so process it
			thisword[i] = 0;
			i = 0;
			int word = -1;

			if (text[0] == ' ') {
				word = FindMatchingMultiWordWord(thisword, &text);
			}

			if (word < 0) {
				// just a normal word
				word = find_word_in_dictionary(thisword);
			}

			// "look rol"
			if (word == RESTOFLINE)
				return 1;
			if (compareto) {
				// check string is longer than user input
				if (comparing >= comparetonum) {
					if (in_optional) {
						// eg. "exit [door]" - there's no more user input
						// but the optional word is still there
						if (do_word_now) {
							in_optional = 0;
							do_word_now = 0;
						}
						thisword[0] = 0;
						text++;
						continue;
					}
					return 0;
				}
				if (word <= 0)
					quitprintf("!Said: supplied word '%s' is not in dictionary or is an ignored word\nText: %s", thisword, src_text);
				if (word == ANYWORD) {
				} else if (word != compareto[comparing]) {
					// words don't match - if a comma then a list of possibles,
					// so allow retry
					if (text[0] == ',')
						comparing--;
					else {
						// words don't match
						if (in_optional) {
							// inside an optional clause, so skip it
							while (text[0] != ']') {
								if (text[0] == 0)
									quitprintf("!Said: unterminated [optional]\nText: %s", src_text);
								text++;
							}
							// -1 because it's about to be ++'d again
							comparing = optional_start - 1;
						}
						// words don't match outside an optional clause, abort
						else
							return 0;
					}
				} else if (text[0] == ',') {
					// this alternative matched, but there are more
					// so skip the other alternatives
					int continueSearching = 1;
					while (continueSearching) {

						const char *textStart = ++text; // begin with next char

						// find where the next word ends
						while ((text[0] == ',') || is_valid_word_char(text[0])) {
							// shift beginning of potential multi-word each time we see a comma
							if (text[0] == ',')
								textStart = ++text;
							else
								text++;
						}

						continueSearching = 0;

						if (text[0] == 0 || text[0] == ' ') {
							Common::strcpy_s(thisword, textStart);
							thisword[text - textStart] = 0;
							// forward past any multi-word alternatives
							if (FindMatchingMultiWordWord(thisword, &text) >= 0) {
								if (text[0] == 0)
									break;
								continueSearching = 1;
							}
						}
					}

					if ((text[0] == ']') && (in_optional)) {
						// [go,move]  we just matched "go", so skip over "move"
						in_optional = 0;
						text++;
					}

					// go back cos it'll be ++'d in a minute
					text--;
				}
				comparing++;
			} else if (word != 0) {
				// it's not an ignore word (it's a known word, or an unknown
				// word, so save its index)
				wordarray[numwords[0]] = word;
				numwords[0]++;
				if (numwords[0] >= MAX_PARSED_WORDS)
					return 0;
				// if it's an unknown word, store it for use in messages like
				// "you can't use the word 'xxx' in this game"
				if ((word < 0) && (_GP(play).bad_parsed_word[0] == 0))
					Common::strcpy_s(_GP(play).bad_parsed_word, 100, thisword);
			}

			if (do_word_now) {
				in_optional = 0;
				do_word_now = 0;
			}

			thisword[0] = 0;
		}
		if (text[0] == 0)
			break;
		text++;
	}
	// If the user input is longer than the Said string, it's wrong
	// eg Said("look door") and they type "look door jibble"
	// rol should be used instead to enable this
	if (comparing < comparetonum)
		return 0;
	return 1;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// int (const char *wordToFind)
RuntimeScriptValue Sc_Parser_FindWordID(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(Parser_FindWordID, const char);
}

// void  (char*text)
RuntimeScriptValue Sc_ParseText(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_POBJ(ParseText, /*const*/ char);
}

// const char* ()
RuntimeScriptValue Sc_Parser_SaidUnknownWord(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ(const char, _GP(myScriptStringImpl), Parser_SaidUnknownWord);
}

// int  (char*checkwords)
RuntimeScriptValue Sc_Said(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(Said, /*const*/ char);
}


void RegisterParserAPI() {
	ScFnRegister parser_api[] = {
		{"Parser::FindWordID^1", API_FN_PAIR(Parser_FindWordID)},
		{"Parser::ParseText^1", API_FN_PAIR(ParseText)},
		{"Parser::SaidUnknownWord^0", API_FN_PAIR(Parser_SaidUnknownWord)},
		{"Parser::Said^1", API_FN_PAIR(Said)},
	};

	ccAddExternalFunctions361(parser_api);
}

} // namespace AGS3
