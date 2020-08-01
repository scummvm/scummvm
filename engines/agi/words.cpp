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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "agi/agi.h"
#include "agi/words.h"

#include "common/textconsole.h"

namespace Agi {

Words::Words(AgiEngine *vm) {
	_vm = vm;

	clearEgoWords();
}

Words::~Words() {
	clearEgoWords();
}

int Words::loadDictionary_v1(Common::File &f) {
	char str[64];
	int k;

	debug(0, "Loading dictionary");

	// Loop through alphabet, as words in the dictionary file are sorted by
	// first character
	f.seek(f.pos() + 26 * 2, SEEK_SET);
	do {
		// Read next word
		for (k = 0; k < (int)sizeof(str) - 1; k++) {
			str[k] = f.readByte();
			if (str[k] == 0 || (uint8)str[k] == 0xFF)
				break;
		}

		// And store it in our internal dictionary
		if (k > 0) {
			WordEntry *newWord = new WordEntry;
			byte firstCharNr = str[0] - 'a';

			newWord->word = Common::String(str, k + 1); // myStrndup(str, k + 1);
			newWord->id = f.readUint16LE();

			_dictionaryWords[firstCharNr].push_back(newWord);
			debug(3, "'%s' (%d)", newWord->word.c_str(), newWord->id);
		}
	} while ((uint8)str[0] != 0xFF);

	return errOK;
}

int Words::loadDictionary(const char *fname) {
	Common::File fp;

	if (!fp.open(fname)) {
		warning("loadWords: can't open %s", fname);
		return errOK; // err_BadFileOpen
	}
	debug(0, "Loading dictionary: %s", fname);

	// Loop through alphabet, as words in the dictionary file are sorted by
	// first character
	for (int i = 0; i < 26; i++) {
		fp.seek(i * 2, SEEK_SET);
		int offset = fp.readUint16BE();
		if (offset == 0)
			continue;
		fp.seek(offset, SEEK_SET);
		int k = fp.readByte();
		while (!fp.eos() && !fp.err()) {
			// Read next word
			char c, str[64];
			do {
				c = fp.readByte();
				str[k++] = (c ^ 0x7F) & 0x7F;
			} while (!(c & 0x80) && k < (int)sizeof(str) - 1);
			str[k] = 0;

			// WORKAROUND:
			// The SQ0 fan game stores words starting with numbers (like '7up')
			// in its dictionary under the 'a' entry. We skip these.
			// See bug #3615061
			if (str[0] == 'a' + i) {
				// And store it in our internal dictionary
				WordEntry *newWord = new WordEntry;
				newWord->word = Common::String(str, k);
				newWord->id = fp.readUint16BE();
				_dictionaryWords[i].push_back(newWord);
			}

			k = fp.readByte();

			// Are there more words with an already known prefix?
			// WORKAROUND: We only break after already seeing words with the
			// right prefix, for the SQ0 words starting with digits filed under
			// 'a'. See above comment and bug #3615061.
			if (k == 0 && str[0] >= 'a' + i)
				break;
		}
	}

	return errOK;
}

void Words::unloadDictionary() {
	for (int16 firstCharNr = 0; firstCharNr < 26; firstCharNr++) {
		Common::Array<WordEntry *> &dictionary = _dictionaryWords[firstCharNr];
		int16 dictionarySize = dictionary.size();

		for (int16 dictionaryWordNr = 0; dictionaryWordNr < dictionarySize; dictionaryWordNr++) {
			delete dictionary[dictionaryWordNr];
		}

		_dictionaryWords[firstCharNr].clear();
	}
}

void Words::clearEgoWords() {
	for (int16 wordNr = 0; wordNr < MAX_WORDS; wordNr++) {
		_egoWords[wordNr].id = 0;
		_egoWords[wordNr].word.clear();
	}
	_egoWordCount = 0;
}


static bool isCharSeparator(const char curChar) {
	switch (curChar) {
	case ' ':
	case ',':
	case '.':
	case '?':
	case '!':
	case '(':
	case ')':
	case ';':
	case ':':
	case '[':
	case ']':
	case '{':
	case '}':
		return true;
		break;
	default:
		break;
	}
	return false;
}

static bool isCharInvalid(const char curChar) {
	switch (curChar) {
	case 0x27: // '
	case 0x60: // `
	case '-':
	case '\\':
	case '"':
		return true;
		break;
	default:
		break;
	}
	return false;
}

void Words::cleanUpInput(const char *rawUserInput, Common::String &cleanInput) {
	byte curChar = 0;

	cleanInput.clear();

	curChar = *rawUserInput;
	while (curChar) {
		// skip separators / invalid characters
		if (isCharSeparator(curChar) || isCharInvalid(curChar)) {
			rawUserInput++;
			curChar = *rawUserInput;
		} else {
			do {
				if (!isCharInvalid(curChar)) {
					// not invalid char, add it to the cleaned up input
					cleanInput += curChar;
				}

				rawUserInput++;
				curChar = *rawUserInput;

				if (isCharSeparator(curChar)) {
					cleanInput += ' ';
					break;
				}
			} while (curChar);
		}
	}
	if (cleanInput.hasSuffix(" ")) {
		// ends with a space? remove it
		cleanInput.deleteLastChar();
	}
}

int16 Words::findWordInDictionary(const Common::String &userInputLowcased, uint16 userInputLen, uint16 userInputPos, uint16 &foundWordLen) {
	uint16 userInputLeft = userInputLen - userInputPos;
	uint16 wordStartPos = userInputPos;
	int16 wordId = DICTIONARY_RESULT_UNKNOWN;
	byte  firstChar = userInputLowcased[userInputPos];
	byte  curUserInputChar = 0;

	foundWordLen = 0;

	if ((firstChar >= 'a') && (firstChar <= 'z')) {
		// word has to start with a letter
		if (((userInputPos + 1) < userInputLen) && (userInputLowcased[userInputPos + 1] == ' ')) {
			// current word is 1 char only?
			if ((firstChar == 'a') || (firstChar == 'i')) {
				// and it's "a" or "i"? -> then set current type to ignore
				wordId = DICTIONARY_RESULT_IGNORE;
			}
		}

		Common::Array<WordEntry *> &dictionary = _dictionaryWords[firstChar - 'a'];
		int16 dictionarySize = dictionary.size();

		for (int16 dictionaryWordNr = 0; dictionaryWordNr < dictionarySize; dictionaryWordNr++) {
			WordEntry *dictionaryEntry = dictionary[dictionaryWordNr];
			uint16 dictionaryWordLen = dictionaryEntry->word.size();

			if (dictionaryWordLen <= userInputLeft) {
				// dictionary word is longer or same length as the remaining user input
				uint16 curCompareLeft = dictionaryWordLen;
				uint16 dictionaryWordPos = 0;
				byte   curDictionaryChar = 0;

				userInputPos = wordStartPos;
				while (curCompareLeft) {
					curUserInputChar = userInputLowcased[userInputPos];
					curDictionaryChar = dictionaryEntry->word[dictionaryWordPos];

					if (curUserInputChar != curDictionaryChar)
						break;

					userInputPos++;
					dictionaryWordPos++;
					curCompareLeft--;
				}

				if (!curCompareLeft) {
					// check, if there is also nothing more of user input left or if a space the follow-up char?
					if ((userInputPos >= userInputLen) || (userInputLowcased[userInputPos] == ' ')) {
						// so fully matched, remember match
						wordId = dictionaryEntry->id;
						foundWordLen = dictionaryWordLen;

						// perfect match? -> exit loop
						if (userInputLeft == foundWordLen) {
							// perfect match -> break
							break;
						}
					}
				}
			}
		}
	}

	if (foundWordLen == 0) {
		userInputPos = wordStartPos;
		while (userInputPos < userInputLen) {
			if (userInputLowcased[userInputPos] == ' ') {
				break;
			}
			userInputPos++;
		}
		foundWordLen = userInputPos - wordStartPos;
	}
	return wordId;
}

void Words::parseUsingDictionary(const char *rawUserInput) {
	Common::String userInput;
	Common::String userInputLowcased;
	const char *userInputPtr = nullptr;
	uint16 userInputLen;
	uint16 userInputPos = 0;
	uint16 foundWordPos;
	int16  foundWordId;
	uint16 foundWordLen = 0;
	uint16 wordCount = 0;

	assert(rawUserInput);
	debugC(2, kDebugLevelScripts, "parse: userinput = \"%s\"", rawUserInput);

	// Reset result
	clearEgoWords();

	// clean up user input
	cleanUpInput(rawUserInput, userInput);

	// Sierra compared independent of upper case and lower case
	userInputLowcased = userInput;
	userInputLowcased.toLowercase();

	userInputLen = userInput.size();
	userInputPtr = userInput.c_str();

	// WORKAROUND: For Apple II support speed changes
	// some of the games hadn't this feature
	// some (like PQ1) had it, but we override the speed that the game request
	// with `timeDelayOverwrite`
	// this mechanism works for all the games, and therefore, doesn't bother to search in the dictionary
	if (_vm->getPlatform() == Common::kPlatformApple2GS) {
		if (userInput.equals("fastest")) {
			_vm->_game.setAppleIIgsSpeedLevel(0);
			return;
		} else if (userInput.equals("fast")) {
			_vm->_game.setAppleIIgsSpeedLevel(1);
			return;
		} else if (userInput.equals("normal")) {
			_vm->_game.setAppleIIgsSpeedLevel(2);
			return;
		} else if (userInput.equals("slow")) {
			_vm->_game.setAppleIIgsSpeedLevel(3);
			return;
		}
	}

	while (userInputPos < userInputLen) {
		// Skip trailing space
		if (userInput[userInputPos] == ' ')
			userInputPos++;

		foundWordPos = userInputPos;
		foundWordId = findWordInDictionary(userInputLowcased, userInputLen, userInputPos, foundWordLen);

		if (foundWordId != DICTIONARY_RESULT_IGNORE) {
			// word not supposed to get ignored
			// add it now
			if (foundWordId != DICTIONARY_RESULT_UNKNOWN) {
				// known word
				_egoWords[wordCount].id = foundWordId;
			}

			_egoWords[wordCount].word = Common::String(userInputPtr + foundWordPos, foundWordLen);
			debugC(2, kDebugLevelScripts, "found word %s (id %d)", _egoWords[wordCount].word.c_str(), _egoWords[wordCount].id);
			wordCount++;

			if (foundWordId == DICTIONARY_RESULT_UNKNOWN) {
				// unknown word
				_vm->setVar(VM_VAR_WORD_NOT_FOUND, wordCount);
				break; // and exit now
			}
		}

		userInputPos += foundWordLen;
	}

	_egoWordCount = wordCount;

	debugC(4, kDebugLevelScripts, "ego word count = %d", _egoWordCount);
	if (_egoWordCount > 0) {
		_vm->setFlag(VM_FLAG_ENTERED_CLI, true);
	} else {
		_vm->setFlag(VM_FLAG_ENTERED_CLI, false);
	}
	_vm->setFlag(VM_FLAG_SAID_ACCEPTED_INPUT, false);
}

uint16 Words::getEgoWordCount() {
	return _egoWordCount;
}
const char *Words::getEgoWord(int16 wordNr) {
	assert(wordNr >= 0 && wordNr < MAX_WORDS);
	return _egoWords[wordNr].word.c_str();
}
uint16 Words::getEgoWordId(int16 wordNr) {
	assert(wordNr >= 0 && wordNr < MAX_WORDS);
	return _egoWords[wordNr].id;
}

} // End of namespace Agi
