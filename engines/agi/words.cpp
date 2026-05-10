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

#include "agi/agi.h"
#include "agi/words.h"

#include "common/textconsole.h"

namespace Agi {

Words::Words(AgiEngine *vm) : 
	_vm(vm),
	_hasExtendedCharacters(false) {

	clearEgoWords();
}

Words::~Words() {
	clearEgoWords();
}

int Words::loadDictionary_v1(Common::SeekableReadStream &stream) {
	char str[64];
	int k;

	debug(0, "Loading dictionary");

	// Loop through alphabet, as words in the dictionary file are sorted by
	// first character
	stream.seek(26 * 2, SEEK_CUR);
	do {
		// Read next word
		for (k = 0; k < ARRAYSIZE(str) - 1; k++) {
			str[k] = stream.readByte();
			if (str[k] == 0 || (uint8)str[k] == 0xFF)
				break;
		}

		// Store word in dictionary
		if (k > 0) {
			WordEntry newWord;
			newWord.word = Common::String(str, k + 1);
			newWord.id = stream.readUint16LE();
			_dictionary[str[0]].push_back(newWord);
		}
	} while ((uint8)str[0] != 0xFF);

	return errOK;
}

int Words::loadDictionary(const char *fname) {
	Common::File fp;
	if (!fp.open(fname)) {
		warning("loadDictionary: can't open %s", fname);
		// FIXME
		return errOK; // err_BadFileOpen
	}

	debug(0, "Loading dictionary: %s", fname);
	return loadDictionary(fp);
}

/**
 * Load all words from WORDS.TOK into the dictionary.
 *
 * Note that this parser handles words that start with a digit. These appear in
 * fan games because AGI Studio allowed them and placed them at the start of the
 * 'A' section. These words had no effect because the interpreter only matched
 * user input that began with A-Z, and the matching logic happened to skip words
 * until it reached one with the expected first letter. In the past, these words 
 * caused problems for our parser. See bugs #6415, #15000
 */
int Words::loadDictionary(Common::SeekableReadStream &stream) {
	// Read words for each letter (A-Z)
	const uint32 start = stream.pos();
	for (int i = 0; i < 26; i++) {
		// Read letter index and seek to first word
		stream.seek(start + i * 2);
		int offset = stream.readUint16BE();
		if (offset == 0) {
			continue; // no words
		}
		stream.seek(start + offset);

		// Read all words in this letter's section
		char str[64] = { 0 };
		int prevWordLength = 0;
		int k = stream.readByte(); // copy-count of first word
		while (!stream.eos() && !stream.err() && k <= prevWordLength) {
			// Read word
			char c = 0;
			while (!(c & 0x80) && k < ARRAYSIZE(str) - 1) {
				c = stream.readByte();
				str[k++] = (c ^ 0x7F) & 0x7F;
			}
			str[k] = 0;

			// Read word id
			uint16 wordId = stream.readUint16BE();
			if (stream.eos() || stream.err()) {
				break;
			}

			// Store word in dictionary
			WordEntry newWord;
			newWord.word = Common::String(str, k);
			newWord.id = wordId;
			_dictionary[str[0]].push_back(newWord);

			// Read next word's copy count, or this letter's zero terminator.
			// Stop on zero if the word we read begins with the expected letter,
			// otherwise this is a fan game and we just read a word that starts
			// with a digit at the start of the 'A' section. Bugs #6413, #15000
			k = stream.readByte();
			if (k == 0 && str[0] == 'a' + i) {
				break;
			}
			prevWordLength = k;
		}
	}

	return errOK;
}

int Words::loadExtendedDictionary(const char *fname) {
	Common::File fp;
	if (!fp.open(fname)) {
		warning("loadWords: can't open %s", fname);
		// FIXME
		return errOK; // err_BadFileOpen
	}
	debug(0, "Loading extended dictionary: %s", fname);

	// skip the header
	fp.readString('\n');

	while (!fp.eos() && !fp.err()) {
		WordEntry newWord;
		newWord.word = fp.readString();
		newWord.id = atoi(fp.readString('\n').c_str());
		if (!newWord.word.empty()) {
			_dictionary[(byte)newWord.word[0]].push_back(newWord);
		}
	}

	_hasExtendedCharacters = true;
	return errOK;
}

void Words::unloadDictionary() {
	_dictionary.clear();
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
	default:
		return false;
	}
}

static bool isCharInvalid(const char curChar) {
	switch (curChar) {
	case 0x27: // '
	case 0x60: // `
	case '-':
	case '\\':
	case '"':
		return true;
	default:
		return false;
	}
}

void Words::cleanUpInput(const char *rawUserInput, Common::String &cleanInput) {
	cleanInput.clear();

	byte curChar = *rawUserInput;
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

int16 Words::findWordInDictionary(const Common::String &userInputLowercase, uint16 userInputLen, uint16 userInputPos, uint16 &foundWordLen) {
	uint16 userInputLeft = userInputLen - userInputPos;
	uint16 wordStartPos = userInputPos;
	int16 wordId = DICTIONARY_RESULT_UNKNOWN;
	byte  firstChar = userInputLowercase[userInputPos];

	foundWordLen = 0;

	const byte lastCharInAbc = _hasExtendedCharacters ? 0xff : 'z';

	// Words normally have to start with a letter.
	// ENHANCEMENT: Fan games and translations include words that start with a
	// digit, even though the original interpreter ignored them. We allow input
	// words to start with a digit if the dictionary contains such a word.
	if (('a' <= firstChar && firstChar <= lastCharInAbc) ||
		('0' <= firstChar && firstChar <= '9' && _dictionary.contains(firstChar))) {
		if (((userInputPos + 1) < userInputLen) && (userInputLowercase[userInputPos + 1] == ' ')) {
			// current word is 1 char only?
			if ((firstChar == 'a') || (firstChar == 'i')) {
				// and it's "a" or "i"? -> then set current type to ignore
				wordId = DICTIONARY_RESULT_IGNORE;
			}
		}

		const Common::Array<WordEntry> &words = _dictionary.getValOrDefault(firstChar);
		for (const WordEntry &wordEntry : words) {
			uint16 dictionaryWordLen = wordEntry.word.size();

			if (dictionaryWordLen <= userInputLeft) {
				// dictionary word is longer or same length as the remaining user input
				uint16 curCompareLeft = dictionaryWordLen;
				uint16 dictionaryWordPos = 0;

				userInputPos = wordStartPos;
				while (curCompareLeft) {
					byte curUserInputChar = userInputLowercase[userInputPos];
					byte curDictionaryChar = wordEntry.word[dictionaryWordPos];

					if (curUserInputChar != curDictionaryChar)
						break;

					userInputPos++;
					dictionaryWordPos++;
					curCompareLeft--;
				}

				if (!curCompareLeft) {
					// check, if there is also nothing more of user input left or if a space the follow-up char?
					if ((userInputPos >= userInputLen) || (userInputLowercase[userInputPos] == ' ')) {
						// so fully matched, remember match
						wordId = wordEntry.id;
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
			if (userInputLowercase[userInputPos] == ' ') {
				break;
			}
			userInputPos++;
		}
		foundWordLen = userInputPos - wordStartPos;
	}
	return wordId;
}

void Words::parseUsingDictionary(const char *rawUserInput) {
	assert(rawUserInput);
	debugC(2, kDebugLevelScripts, "parse: userinput = \"%s\"", rawUserInput);

	// Reset result
	clearEgoWords();

	// clean up user input
	Common::String userInput;
	cleanUpInput(rawUserInput, userInput);

	// Sierra compared independent of upper case and lower case
	Common::String userInputLowercase = userInput;
	userInputLowercase.toLowercase();

	if (_vm->getLanguage() == Common::RU_RUS) {
		convertRussianUserInput(userInputLowercase);
	}

	if (handleSpeedCommands(userInputLowercase)) {
		return;
	}

	uint16 wordCount = 0;
	uint16 userInputPos = 0;
	uint16 userInputLen = userInput.size();
	const char *userInputPtr = userInput.c_str();
	while (userInputPos < userInputLen) {
		// Skip trailing space
		if (userInput[userInputPos] == ' ')
			userInputPos++;

		uint16 foundWordPos = userInputPos;
		uint16 foundWordLen = 0;
		int16 foundWordId = findWordInDictionary(userInputLowercase, userInputLen, userInputPos, foundWordLen);

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

uint16 Words::getEgoWordCount() const {
	return _egoWordCount;
}

const char *Words::getEgoWord(int16 wordNr) const {
	assert(wordNr >= 0 && wordNr < MAX_WORDS);
	return _egoWords[wordNr].word.c_str();
}

uint16 Words::getEgoWordId(int16 wordNr) const {
	assert(wordNr >= 0 && wordNr < MAX_WORDS);
	return _egoWords[wordNr].id;
}

bool Words::handleSpeedCommands(const Common::String &userInputLowercase) {
	// We add speed controls to games that didn't originally have them.
	// Apple II games had no speed controls, the interpreter ran as fast as it could.
	// Some Apple IIgs games had speed controls, others didn't. We override the
	// the speed that the game requests with `timeDelayOverwrite`.
	switch (_vm->getPlatform()) {
	case Common::kPlatformApple2:
	case Common::kPlatformApple2GS:
		if (userInputLowercase == "fastest") {
			_vm->_game.setSpeedLevel(0);
			return true;
		} else if (userInputLowercase == "fast") {
			_vm->_game.setSpeedLevel(1);
			return true;
		} else if (userInputLowercase == "normal") {
			_vm->_game.setSpeedLevel(2);
			return true;
		} else if (userInputLowercase == "slow") {
			_vm->_game.setSpeedLevel(3);
			return true;
		}
		break;
	default:
		break;
	}
	return false;
}

void Words::convertRussianUserInput(Common::String &userInputLowercase) {
	const char *conv =
		// АБВГДЕЖЗИЙКЛМНОП
		  "abvgdewziiklmnop" // 80
		// РСТУФХЦЧШЩЪЫЬЭЮЯ
		  "rstufxcyhhjijeuq" // 90
		// абвгдежзийклмноп
		  "abvgdewziiklmnop" // a0
		  "                " // b0
		  "                " // c0
		  "                " // d0
		// рстуфхцчшщъыьэюя
		  "rstufxcyhhjijeuq" // e0
		// Ее
		  "ee              ";// f0

	Common::String tr;
	for (uint i = 0; i < userInputLowercase.size(); i++) {
		if ((byte)userInputLowercase[i] >= 0x80) {
			tr += conv[(byte)userInputLowercase[i] - 0x80];
		} else {
			tr += (byte)userInputLowercase[i];
		}
	}
	userInputLowercase = tr;
}

} // End of namespace Agi
