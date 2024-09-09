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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/str.h"
#include "glk/scott/scott.h"
#include "glk/scott/globals.h"
#include "glk/scott/command_parser.h"

namespace Glk {
namespace Scott {

#define MAX_WORDLENGTH 128
#define MAX_WORDS 128
#define MAX_BUFFER 128

void freeStrings() {
	if (_G(_firstErrorMessage) != nullptr) {
		delete[] _G(_firstErrorMessage);
		_G(_firstErrorMessage) = nullptr;
	}
	if (_G(_wordsInInput) == 0) {
		if (_G(_unicodeWords) != nullptr || _G(_charWords) != nullptr) {
			g_scott->fatal("ERROR! Wordcount 0 but word arrays not empty!\n");
		}
		return;
	}
	for (int i = 0; i < _G(_wordsInInput); i++) {
		if (_G(_unicodeWords)[i] != nullptr)
			delete _G(_unicodeWords)[i];
		if (_G(_charWords)[i] != nullptr)
			delete _G(_charWords)[i];
	}

	delete _G(_unicodeWords);
	_G(_unicodeWords) = nullptr;
	delete _G(_charWords);
	_G(_charWords) = nullptr;
	_G(_wordsInInput) = 0;
}

void createErrorMessage(const char *fchar, glui32 *second, const char *tchar) {
	if (_G(_firstErrorMessage) != nullptr)
		return;
	glui32 *first = toUnicode(fchar);
	glui32 *third = toUnicode(tchar);
	glui32 buffer[MAX_BUFFER];
	int i, j = 0, k = 0;
	for (i = 0; first[i] != 0 && i < MAX_BUFFER; i++)
		buffer[i] = first[i];
	if (second != nullptr) {
		for (j = 0; second[j] != 0 && i + j < MAX_BUFFER; j++)
			buffer[i + j] = second[j];
	}
	if (third != nullptr) {
		for (k = 0; third[k] != 0 && i + j + k < MAX_BUFFER; k++)
			buffer[i + j + k] = third[k];
		delete[] third;
	}
	int length = i + j + k;
	_G(_firstErrorMessage) = new glui32[(length + 1) * 4];
	memcpy(_G(_firstErrorMessage), buffer, length * 4);
	_G(_firstErrorMessage)[length] = 0;
	delete[] first;
}

void printPendingError(void) {
	if (_G(_firstErrorMessage)) {
		g_scott->glk_put_string_stream_uni(g_scott->glk_window_get_stream(_G(_bottomWindow)), _G(_firstErrorMessage));
		delete[] _G(_firstErrorMessage);
		_G(_firstErrorMessage) = nullptr;
		_G(_stopTime) = 1;
	}
}

char **LineInput(void) {
	event_t ev;
	glui32 unibuf[512];

	do {
		g_scott->display(_G(_bottomWindow), "\n%s", _G(_sys)[WHAT_NOW].c_str());
		g_scott->glk_request_line_event_uni(_G(_bottomWindow), unibuf, (glui32)511, 0);

		while (ev.type != evtype_Quit) {
			g_scott->glk_select(&ev);
			if (ev.type == evtype_Quit)
				return nullptr;

			if (ev.type == evtype_LineInput)
				break;
			else
				g_scott->updates(ev);
		}

		unibuf[ev.val1] = 0;

		if (_G(_transcript)) {
			g_scott->glk_put_string_stream_uni(_G(_transcript), unibuf);
			g_scott->glk_put_char_stream_uni(_G(_transcript), 10);
		}

		_G(_charWords) = splitIntoWords(unibuf, ev.val1);

		if (_G(_wordsInInput) == 0 || _G(_charWords) == nullptr)
			g_scott->output(_G(_sys)[HUH]);
		else {
			return _G(_charWords);
		}

	} while (_G(_wordsInInput) == 0 || _G(_charWords) == nullptr);
	return nullptr;
}

int matchYMCA(glui32 *string, int length, int index) {
	const char *ymca = "y.m.c.a.";
	int i;
	for (i = 0; i < 8; i++) {
		if (i + index > length || string[index + i] != static_cast<unsigned int>(ymca[i]))
			return i;
	}
	return i;
}

char **splitIntoWords(glui32 *string, int length) {
	if (length < 1) {
		return nullptr;
	}

	g_scott->glk_buffer_to_lower_case_uni(string, 256, length);
	g_scott->glk_buffer_canon_normalize_uni(string, 256, length);

	int startpos[MAX_WORDS];
	int wordlength[MAX_WORDS];

	int words_found = 0;
	int word_index = 0;
	int foundspace = 0;
	int foundcomma = 0;
	startpos[0] = 0;
	wordlength[0] = 0;
	int lastwasspace = 1;
	for (int i = 0; string[i] != 0 && i < length && word_index < MAX_WORDS; i++) {
		foundspace = 0;
		switch (string[i]) {
		case 'y': {
			int ymca = matchYMCA(string, length, i);
			if (ymca > 3) {
				/* Start a new word */
				startpos[words_found] = i;
				wordlength[words_found] = ymca;
				words_found++;
				wordlength[words_found] = 0;
				i += ymca;
				if (i < length)
					foundspace = 1;
				lastwasspace = 0;
			}
		} break;
			/* Unicode space and tab variants */
		case ' ':
		case '\t':
		case '!':
		case '?':
		case '\"':
		case 0x83:   // ¿
		case 0x80:   // ¡
		case 0xa0:   // non-breaking space
		case 0x2000: // en quad
		case 0x2001: // em quad
		case 0x2003: // em
		case 0x2004: // three-per-em
		case 0x2005: // four-per-em
		case 0x2006: // six-per-em
		case 0x2007: // figure space
		case 0x2009: // thin space
		case 0x200A: // hair space
		case 0x202f: // narrow no-break space
		case 0x205f: // medium mathematical space
		case 0x3000: // ideographic space
			foundspace = 1;
			break;
		case '.':
		case ',':
			foundcomma = 1;
			break;
		default:
			break;
		}
		if (!foundspace) {
			if (lastwasspace || foundcomma) {
				/* Start a new word */
				startpos[words_found] = i;
				words_found++;
				wordlength[words_found] = 0;
			}
			wordlength[words_found - 1]++;
			lastwasspace = 0;
		} else {
			/* Check if the last character of previous word was a period or comma */
			lastwasspace = 1;
			foundcomma = 0;
		}
	}

	if (words_found == 0) {
		return nullptr;
	}

	wordlength[words_found]--; /* Don't count final newline character */

	/* Now we've created two arrays, one for starting positions
	 and one for word length. Now we convert these into an array of strings */
	glui32 **words = new glui32 *[words_found];
	char **words8 = new char *[words_found];

	for (int i = 0; i < words_found; i++) {
		words[i] = new glui32[(wordlength[i] + 1) * 4];
		memcpy(words[i], string + startpos[i], wordlength[i] * 4);
		words[i][wordlength[i]] = 0;
		words8[i] = fromUnicode(words[i], wordlength[i]);
	}
	_G(_unicodeWords) = words;
	_G(_wordsInInput) = words_found;

	return words8;
}

int findVerb(const char *string, Common::StringArray *list) {
	*list = _G(_verbs);
	int verb = whichWord(string, *list, _G(_gameHeader)->_wordLength);
	if (verb) {
		return verb;
	}
	*list = _G(_directions);
	verb = whichWord(string, *list, _G(_gameHeader)->_wordLength);
	if (verb) {
		if (verb == 13)
			verb = 4;
		if (verb > 6)
			verb -= 6;
		return verb;
	}
	*list = _G(_abbreviations);
	verb = whichWord(string, *list, _G(_gameHeader)->_wordLength);
	if (verb) {
		verb = whichWord(_G(_abbreviationsKey)[verb].c_str(), _G(_verbs), _G(_gameHeader)->_wordLength);
		if (verb) {
			list = &_G(_verbs);
			return verb;
		}
	}

	int stringlength = strlen(string);

	*list = _G(_skipList);
	verb = whichWord(string, *list, stringlength);
	if (verb) {
		return 0;
	}
	*list = _G(_nouns);
	verb = whichWord(string, *list, _G(_gameHeader)->_wordLength);
	if (verb) {
		return verb;
	}

	*list = _G(_extraCommands);
	verb = whichWord(string, *list, stringlength);
	if (verb) {
		verb = _G(_extraCommandsKey)[verb];
		return verb + _G(_gameHeader)->_numWords;
	}

	*list = _G(_extraNouns);
	verb = whichWord(string, *list, stringlength);
	if (verb) {
		verb = _G(_extraNounsKey)[verb];
		return verb + _G(_gameHeader)->_numWords;
	}

	*list = _G(_delimiterList);
	verb = whichWord(string, *list, stringlength);
	if (!verb)
		*list = Common::StringArray();
	return verb;
}

int findExtraneousWords(int *index, int noun) {
	/* Looking for extraneous words that should invalidate the command */
	int originalIndex = *index;
	if (*index >= _G(_wordsInInput)) {
		return 0;
	}
	Common::StringArray list;
	int verb = 0;
	int stringlength = strlen(_G(_charWords)[*index]);

	list = _G(_skipList);
	do {
		verb = whichWord(_G(_charWords)[*index], _G(_skipList), stringlength);
		if (verb)
			*index = *index + 1;
	} while (verb && *index < _G(_wordsInInput));

	if (*index >= _G(_wordsInInput))
		return 0;

	verb = findVerb(_G(_charWords)[*index], &list);

	if (list == _G(_delimiterList)) {
		if (*index > originalIndex)
			*index = *index - 1;
		return 0;
	}

	if (list == _G(_nouns) && noun) {
		if (g_scott->mapSynonym(noun) == g_scott->mapSynonym(verb)) {
			*index = *index + 1;
			return 0;
		}
	}

	if (list.empty()) {
		if (*index >= _G(_wordsInInput))
			*index = _G(_wordsInInput) - 1;
		createErrorMessage(_G(_sys)[I_DONT_KNOW_WHAT_A].c_str(), _G(_unicodeWords)[*index], _G(_sys)[IS].c_str());
	} else {
		createErrorMessage(_G(_sys)[I_DONT_UNDERSTAND].c_str(), nullptr, nullptr);
	}

	return 1;
}

Command *commandFromStrings(int index, Command *previous);

Command *createCommandStruct(int verb, int noun, int verbIndex, int nounIndex, Command *previous) {
	Command *command = new Command;
	command->_verb = verb;
	command->_noun = noun;
	command->_allFlag = 0;
	command->_item = 0;
	command->_previous = previous;
	command->_verbWordIndex = verbIndex;
	if (noun && nounIndex > 0) {
		command->_nounWordIndex = nounIndex - 1;
	} else {
		command->_nounWordIndex = 0;
	}
	command->_next = commandFromStrings(nounIndex, command);
	return command;
}

int findNoun(const char *string, Common::StringArray *list) {
	*list = _G(_nouns);
	int noun = whichWord(string, *list, _G(_gameHeader)->_wordLength);
	if (noun) {
		return noun;
	}

	*list = _G(_directions);
	noun = whichWord(string, *list, _G(_gameHeader)->_wordLength);
	if (noun) {
		if (noun > 6)
			noun -= 6;
		*list = _G(_nouns);
		return noun;
	}

	int stringLength = strlen(string);

	*list = _G(_extraNouns);

	noun = whichWord(string, *list, stringLength);
	if (noun) {
		noun = _G(_extraNounsKey)[noun];
		return noun + _G(_gameHeader)->_numWords;
	}

	*list = _G(_skipList);
	noun = whichWord(string, *list, stringLength);
	if (noun) {
		return 0;
	}

	*list = _G(_verbs);
	noun = whichWord(string, *list, _G(_gameHeader)->_wordLength);
	if (noun) {
		return noun;
	}

	*list = _G(_delimiterList);
	noun = whichWord(string, *list, stringLength);

	if (!noun)
		*list = Common::StringArray();
	return 0;
}

Command *commandFromStrings(int index, Command *previous) {
	if (index < 0 || index >= _G(_wordsInInput)) {
		return nullptr;
	}
	Common::StringArray list;
	int verb = 0;
	int i = index;

	do {
		/* Checking if it is a verb */
		verb = findVerb(_G(_charWords)[i++], &list);
	} while ((list == _G(_skipList) || list == _G(_delimiterList)) && i < _G(_wordsInInput));

	int verbindex = i - 1;

	if (list == _G(_directions)) {
		/* It is a direction */
		if (verb == 0 || findExtraneousWords(&i, 0) != 0)
			return nullptr;
		return createCommandStruct(GO, verb, 0, i, previous);
	}

	int found_noun_at_verb_position = 0;
	int lastverb = 0;

	if (list == _G(_nouns) || list == _G(_extraNouns)) {
		/* It is a noun */
		/* If we find no verb, we try copying the verb from the previous command */
		if (previous) {
			lastverb = previous->_verb;
		}
		/* Unless the game is German, where we allow the noun to come before the
		 * verb */
		if (CURRENT_GAME != GREMLINS_GERMAN && CURRENT_GAME != GREMLINS_GERMAN_C64) {
			if (!previous) {
				createErrorMessage(_G(_sys)[I_DONT_KNOW_HOW_TO].c_str(), _G(_unicodeWords)[i - 1], _G(_sys)[SOMETHING].c_str());
				return nullptr;
			} else {
				verbindex = previous->_verbWordIndex;
			}
			if (findExtraneousWords(&i, verb) != 0)
				return nullptr;

			return createCommandStruct(lastverb, verb, verbindex, i, previous);
		} else {
			found_noun_at_verb_position = 1;
		}
	}

	if (list.empty() || list == _G(_skipList)) {
		createErrorMessage(_G(_sys)[I_DONT_KNOW_HOW_TO].c_str(), _G(_unicodeWords)[i - 1], _G(_sys)[SOMETHING].c_str());
		return nullptr;
	}

	if (i == _G(_wordsInInput)) {
		if (lastverb) {
			return createCommandStruct(lastverb, verb, previous->_verbWordIndex, i, previous);
		} else if (found_noun_at_verb_position) {
			createErrorMessage(_G(_sys)[I_DONT_KNOW_HOW_TO].c_str(), _G(_unicodeWords)[i - 1], _G(_sys)[SOMETHING].c_str());
			return nullptr;
		} else {
			return createCommandStruct(verb, 0, i - 1, i, previous);
		}
	}

	int noun = 0;

	do {
		/* Check if it is a noun */
		noun = findNoun(_G(_charWords)[i++], &list);
	} while (list == _G(_skipList) && i < _G(_wordsInInput));

	if (list == _G(_nouns) || list == _G(_extraNouns)) {
		/* It is a noun */

		/* Check if it is an ALL followed by EXCEPT */
		int except = 0;
		if (list == _G(_extraNouns) && i < _G(_wordsInInput) && noun - _G(_gameHeader)->_numWords == ALL) {
			int stringlength = strlen(_G(_charWords)[i]);
			except = whichWord(_G(_charWords)[i], _G(_extraCommands), stringlength);
		}
		if (_G(_extraCommandsKey)[except] != EXCEPT && findExtraneousWords(&i, noun) != 0)
			return nullptr;
		if (found_noun_at_verb_position) {
			int realverb = whichWord(_G(_charWords)[i - 1], _G(_verbs), _G(_gameHeader)->_wordLength);
			if (realverb) {
				noun = verb;
				verb = realverb;
			} else if (lastverb) {
				noun = verb;
				verb = lastverb;
			}
		}
		return createCommandStruct(verb, noun, verbindex, i, previous);
	}

	if (list == _G(_delimiterList)) {
		/* It is a delimiter */
		return createCommandStruct(verb, 0, verbindex, i, previous);
	}

	if (list == _G(_verbs) && found_noun_at_verb_position) {
		/* It is a verb */
		/* Check if it is an ALL followed by EXCEPT */
		int except = 0;
		if (i < _G(_wordsInInput) && verb - _G(_gameHeader)->_numWords == ALL) {
			int stringlength = strlen(_G(_charWords)[i]);
			except = whichWord(_G(_charWords)[i], _G(_extraCommands), stringlength);
		}
		if (_G(_extraCommandsKey)[except] != EXCEPT && findExtraneousWords(&i, 0) != 0)
			return nullptr;
		return createCommandStruct(noun, verb, i - 1, i, previous);
	}

	createErrorMessage(_G(_sys)[I_DONT_KNOW_WHAT_A].c_str(), _G(_unicodeWords)[i - 1], _G(_sys)[IS].c_str());
	return nullptr;
}

int createAllCommands(Command *command) {

	Common::Array<int> exceptions(_G(_gameHeader)->_numItems);
	int exceptioncount = 0;

	int location = CARRIED;
	if (command->_verb == TAKE)
		location = MY_LOC;

	Command *next = command->_next;
	/* Check if the ALL command is followed by EXCEPT */
	/* and if it is, build an array of items to be excepted */
	while (next && next->_verb == _G(_gameHeader)->_numWords + EXCEPT) {
		for (int i = 0; i <= _G(_gameHeader)->_numItems; i++) {
			if (!_G(_items)[i]._autoGet.empty() && scumm_strnicmp(_G(_items)[i]._autoGet.c_str(), _G(_charWords)[next->_nounWordIndex], _G(_gameHeader)->_wordLength) == 0) {
				exceptions[exceptioncount++] = i;
			}
		}
		/* Remove the EXCEPT command from the linked list of commands */
		next = next->_next;
		delete command->_next;
		command->_next = next;
	}

	Command *c = command;
	int found = 0;
	for (int i = 0; i < _G(_gameHeader)->_numItems; i++) {
		if (!_G(_items)[i]._autoGet.empty() && _G(_items)[i]._autoGet[0] != '*' && _G(_items)[i]._location == location) {
			int exception = 0;
			for (int j = 0; j < exceptioncount; j++) {
				if (exceptions[j] == i) {
					exception = 1;
					break;
				}
			}
			if (!exception) {
				if (found) {
					c->_next = new Command;
					c->_next->_previous = c;
					c = c->_next;
				}
				found = 1;
				c->_verb = command->_verb;
				c->_noun = whichWord(_G(_items)[i]._autoGet.c_str(), _G(_nouns), _G(_gameHeader)->_wordLength);
				c->_item = i;
				c->_next = nullptr;
				c->_nounWordIndex = 0;
				c->_allFlag = 1;
			}
		}
	}
	if (found == 0) {
		if (command->_verb == TAKE)
			createErrorMessage(_G(_sys)[NOTHING_HERE_TO_TAKE].c_str(), nullptr, nullptr);
		else
			createErrorMessage(_G(_sys)[YOU_HAVE_NOTHING].c_str(), nullptr, nullptr);
		return 0;
	} else {
		c->_next = next;
		c->_allFlag = 1 | LASTALL;
	}
	return 1;
}

int getInput(int *vb, int *no) {
	if (_G(_currentCommand) && _G(_currentCommand)->_next) {
		_G(_currentCommand) = _G(_currentCommand)->_next;
	} else {
		printPendingError();
		if (_G(_currentCommand))
			freeCommands();
		_G(_charWords) = LineInput();

		if (_G(_wordsInInput) == 0 || _G(_charWords) == nullptr)
			return 0;

		_G(_currentCommand) = commandFromStrings(0, nullptr);
	}

	if (_G(_currentCommand) == nullptr) {
		printPendingError();
		return 1;
	}

	/* We use NumWords + verb for our extra commands */
	/* such as UNDO and TRANSCRIPT */
	if (_G(_currentCommand)->_verb > _G(_gameHeader)->_numWords) {
		if (!g_scott->performExtraCommand(0)) {
			createErrorMessage(_G(_sys)[I_DONT_UNDERSTAND].c_str(), nullptr, nullptr);
		}
		return 1;
		/* And NumWords + noun for our extra nouns */
		/* such as ALL */
	} else if (_G(_currentCommand)->_noun > _G(_gameHeader->_numWords)) {
		_G(_currentCommand)->_noun -= _G(_gameHeader)->_numWords;
		if (_G(_currentCommand)->_noun == ALL) {
			if (_G(_currentCommand)->_verb != TAKE && _G(_currentCommand)->_verb != DROP) {
				createErrorMessage(_G(_sys)[CANT_USE_ALL].c_str(), nullptr, nullptr);
				return 1;
			}
			if (!createAllCommands(_G(_currentCommand)))
				return 1;
		} else if (_G(_currentCommand)->_noun == IT) {
			_G(_currentCommand)->_noun = _G(_lastNoun);
		}
	}

	*vb = _G(_currentCommand)->_verb;
	*no = _G(_currentCommand)->_noun;

	if (*no > 6) {
		_G(_lastNoun) = *no;
	}

	return 0;
}

void freeCommands() {
	while (_G(_currentCommand) && _G(_currentCommand)->_previous)
		_G(_currentCommand) = _G(_currentCommand)->_previous;
	while (_G(_currentCommand)) {
		Command *temp = _G(_currentCommand);
		_G(_currentCommand) = _G(_currentCommand)->_next;
		delete temp;
	}
	_G(_currentCommand) = nullptr;
	freeStrings();
	if (_G(_firstErrorMessage))
		delete[] _G(_firstErrorMessage);
	_G(_firstErrorMessage) = nullptr;
}

glui32 *toUnicode(const char *string) {
	if (string == nullptr)
		return nullptr;
	glui32 unicode[2048];
	int i;
	int dest = 0;
	for (i = 0; string[i] != 0 && i < 2047; i++) {
		char c = string[i];
		if (c == '\n')
			c = 10;
		glui32 unichar = (glui32)c;
		if (_G(_game) && (CURRENT_GAME == GREMLINS_GERMAN || CURRENT_GAME == GREMLINS_GERMAN_C64)) {
			const char d = string[i + 1];
			if (c == 'u' && d == 'e') { // ü
				if (!(i > 2 && string[i - 1] == 'e')) {
					unichar = 0xfc;
					i++;
				}
			} else if (c == 'o' && d == 'e') {
				unichar = 0xf6; // ö
				i++;
			} else if (c == 'a' && d == 'e') {
				unichar = 0xe4; // ä
				i++;
			} else if (c == 's' && d == 's') {
				if (string[i + 2] != 'c' && string[i - 2] != 'W' && !(string[i - 1] == 'a' && string[i - 2] == 'l') && string[i + 2] != '-' && string[i - 2] != 'b') {
					unichar = 0xdf; // ß
					i++;
				}
			} else if (c == 'U' && d == 'E') {
				unichar = 0xdc; // Ü
				i++;
			}
			if (c == '\"') {
				unichar = 0x2019; // ’
			}
		} else if (_G(_game) && CURRENT_GAME == GREMLINS_SPANISH) {
			switch (c) {
			case '\x83':
				unichar = 0xbf; // ¿
				break;
			case '\x80':
				unichar = 0xa1; // ¡
				break;
			case '\x82':
				unichar = 0xfc; // ü
				break;
			case '{':
				unichar = 0xe1; // á
				break;
			case '}':
				unichar = 0xed; // í
				break;
			case '|':
				unichar = 0xf3; // ó
				break;
			case '~':
				unichar = 0xf1; // ñ
				break;
			case '\x84':
				unichar = 0xe9; // é
				break;
			case '\x85':
				unichar = 0xfa; // ú
				break;
			}
		} else if (_G(_game) && CURRENT_GAME == TI994A) {
			switch (c) {
			case '@':
				unicode[dest++] = 0xa9;
				unichar = ' ';
				break;
			case '}':
				unichar = 0xfc;
				break;
			case 12:
				unichar = 0xf6;
				break;
			case '{':
				unichar = 0xe4;
				break;
			}
		}
		unicode[dest++] = unichar;
	}
	unicode[dest] = 0;
	glui32 *result = new glui32[(dest + 1) * 4];
	memcpy(result, unicode, (dest + 1) * 4);
	return result;
}

char *fromUnicode(glui32 *unicodeString, int origLength) {
	int sourcepos = 0;
	int destpos = 0;

	char dest[MAX_WORDLENGTH];
	glui32 unichar = unicodeString[sourcepos];
	while (unichar != 0 && destpos < MAX_WORDLENGTH && sourcepos < origLength) {
		switch (unichar) {
		case '.':
			if (origLength == 1) {
				dest[destpos++] = 'a';
				dest[destpos++] = 'n';
				dest[destpos++] = 'd';
			} else {
				dest[destpos] = (char)unichar;
			}
			break;
		case 0xf6: // ö
			dest[destpos++] = 'o';
			dest[destpos] = 'e';
			break;
		case 0xe4: // ä
			dest[destpos++] = 'a';
			dest[destpos] = 'e';
			break;
		case 0xfc: // ü
			dest[destpos] = 'u';
			if (CURRENT_GAME == GREMLINS_GERMAN || CURRENT_GAME == GREMLINS_GERMAN_C64) {
				destpos++;
				dest[destpos] = 'e';
			}
			break;
		case 0xdf: // ß
			dest[destpos++] = 's';
			dest[destpos] = 's';
			break;
		case 0xed: // í
			dest[destpos] = 'i';
			break;
		case 0xe1: // á
			dest[destpos] = 'a';
			break;
		case 0xf3: // ó
			dest[destpos] = 'o';
			break;
		case 0xf1: // ñ
			dest[destpos] = 'n';
			break;
		case 0xe9: // é
			dest[destpos] = 'e';
			break;
		default:
			dest[destpos] = (char)unichar;
			break;
		}
		sourcepos++;
		destpos++;
		unichar = unicodeString[sourcepos];
	}
	if (destpos == 0)
		return nullptr;
	char *result = new char[destpos + 1];
	memcpy(result, dest, destpos);

	result[destpos] = 0;
	return result;
}

int recheckForExtraCommand() {
	const char *verbWord = _G(_charWords)[_G(_currentCommand)->_verbWordIndex];

	int extraVerb = whichWord(verbWord, _G(_extraCommands), _G(_gameHeader)->_wordLength);
	if (!extraVerb) {
		return 0;
	}
	int ExtraNoun = 0;
	if (_G(_currentCommand)->_noun) {
		const char *nounWord = _G(_charWords)[_G(_currentCommand)->_nounWordIndex];
		ExtraNoun = whichWord(nounWord, _G(_extraNouns), strlen(nounWord));
	}
	_G(_currentCommand)->_verb = _G(_extraCommandsKey)[extraVerb];
	if (ExtraNoun)
		_G(_currentCommand)->_noun = _G(_extraNounsKey)[ExtraNoun];

	return g_scott->performExtraCommand(1);
}

int whichWord(const char *word, Common::StringArray list, int wordLength) {
	int n = 1;
	unsigned int ne = 1;
	const char *tp;
	while (ne < list.size()) {
		tp = list[ne].c_str();
		if (*tp == '*')
			tp++;
		else
			n = ne;
		if (scumm_strnicmp(word, tp, wordLength) == 0)
			return (n);
		ne++;
	}
	return (0);
}

} // End of namespace Scott
} // End of namespace Glk
