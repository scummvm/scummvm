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

#include "titanic/true_talk/tt_parser.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/true_talk/tt_sentence.h"
#include "titanic/titanic.h"

namespace Titanic {

TTparser::TTparser(CScriptHandler *owner) : _owner(owner), _sentenceSub(nullptr),
		_sentence(nullptr), _fieldC(0), _field10(0), _field14(0), _field18(0) {
	loadArrays();
}

void TTparser::loadArrays() {
	Common::SeekableReadStream *r;
	r = g_vm->_filesManager->getResource("TEXT/REPLACEMENTS1");
	while (r->pos() < r->size())
		_replacements1.push_back(readStringFromStream(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/REPLACEMENTS2");
	while (r->pos() < r->size())
		_replacements2.push_back(readStringFromStream(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/REPLACEMENTS3");
	while (r->pos() < r->size())
		_replacements3.push_back(readStringFromStream(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/PHRASES");
	while (r->pos() < r->size())
		_phrases.push_back(readStringFromStream(r));
	delete r;

	r = g_vm->_filesManager->getResource("TEXT/NUMBERS");
	while (r->pos() < r->size()) {
		NumberEntry ne;
		ne._text = readStringFromStream(r);
		ne._value = r->readSint32LE();
		ne._flags = r->readUint32LE();
		_numbers.push_back(ne);
	}
	delete r;

}

int TTparser::preprocess(TTsentence *sentence) {
	_sentence = sentence;
	if (normalize(sentence))
		return 0;

	// Scan for and replace common slang and contractions with verbose versions
	searchAndReplace(sentence->_normalizedLine, _replacements1);
	searchAndReplace(sentence->_normalizedLine, _replacements2);

	// Check entire normalized line against common phrases to replace
	for (uint idx = 0; idx < _phrases.size(); idx += 2) {
		if (!_phrases[idx].compareTo(sentence->_normalizedLine))
			sentence->_normalizedLine = _phrases[idx + 1];
	}

	// Do a further search and replace of roman numerals to decimal
	searchAndReplace(sentence->_normalizedLine, _replacements3);

	// Replace any roman numerals, spelled out words, etc. with decimal numbers
	CTrueTalkManager::_v1 = -1000;
	int idx = 0;
	do {
		idx = replaceNumbers(sentence->_normalizedLine, idx);
	} while (idx >= 0);

	if (CTrueTalkManager::_v1 == -1000 && !sentence->_normalizedLine.empty()) {
		// Scan the text for any numeric digits
		for (const char *strP = sentence->_normalizedLine.c_str(); *strP; ++strP) {
			if (Common::isDigit(*strP)) {
				// Found digit, so convert it and any following ones
				CTrueTalkManager::_v1 = atoi(strP);
				break;
			}
		}
	}

	return 0;
}

int TTparser::normalize(TTsentence *sentence) {
	TTstring *destLine = new TTstring();
	const TTstring &srcLine = sentence->_initialLine;
	int srcSize = srcLine.size();
	int savedIndex = 0;
	int counter1 = 0;
	int commandVal;

	for (int index = 0; index < srcSize; ++index) {
		char c = srcLine[index];
		if (Common::isLower(c)) {
			(*destLine) += c;
		} else if (Common::isSpace(c)) {
			if (!destLine->empty() && destLine->lastChar() != ' ')
				(*destLine) += ' ';
		} else if (Common::isUpper(c)) {
			(*destLine) += toupper(c);
		} else if (Common::isDigit(c)) {
			if (c == '0' && isEmoticon(srcLine, index)) {
				sentence->set38(10);
			} else {
				// Iterate through all the digits of the number
				(*destLine) += c;
				while (Common::isDigit(srcLine[index + 1]))
					(*destLine) += srcLine[++index];
			}
		} else if (Common::isPunct(c)) {
			bool flag = false;
			switch (c) {
			case '!':
				sentence->set38(3);
				break;
			
			case '\'':
				if (!normalizeContraction(srcLine, index, *destLine))
					flag = true;
				break;
			
			case '.':
				sentence->set38(1);
				break;
			
			case ':':
				commandVal = isEmoticon(srcLine, index);
				if (commandVal) {
					sentence->set38(commandVal);
					index += 2;
				} else {
					flag = true;
				}
				break;
			
			case ';':
				commandVal = isEmoticon(srcLine, index);
				if (commandVal == 6) {
					sentence->set38(7);
					index += 2;
				} else if (commandVal != 0) {
					sentence->set38(commandVal);
					index += 2;
				}
				break;
			
			case '<':
				++index;
				commandVal = isEmoticon(srcLine, index);
				if (commandVal == 6) {
					sentence->set38(12);
				} else {
					--index;
					flag = true;
				}
				break;

			case '>':
				++index;
				commandVal = isEmoticon(srcLine, index);
				if (commandVal == 6 || commandVal == 9) {
					sentence->set38(11);
				} else {
					--index;
					flag = true;
				}
				break;

			case '?':
				sentence->set38(2);
				break;

			default:
				flag = true;
				break;
			}

			if (flag && (!savedIndex || (index - savedIndex) == 1))
				++counter1;

			savedIndex = index;
		}
	}

	if (counter1 >= 4)
		sentence->set38(4);

	// Remove any trailing spaces
	while (destLine->hasSuffix(" "))
		destLine->deleteLastChar();

	// Copy out the normalized line
	sentence->_normalizedLine = *destLine;
	delete destLine;

	return 0;
}

int TTparser::isEmoticon(const TTstring &str, int &index) {
	if (str[index] != ':' && str[index] != ';')
		return 0;

	if (str[index + 1] != '-')
		return 0;

	index += 2;
	switch (str[index]) {
	case '(':
	case '<':
		return 8;

	case ')':
	case '>':
		return 6;

	case 'P':
	case 'p':
		return 9;

	default:
		return 5;
	}
}

bool TTparser::normalizeContraction(const TTstring &srcLine, int srcIndex, TTstring &destLine) {
	int startIndex = srcIndex + 1;
	switch (srcLine[startIndex]) {
	case 'd':
		srcIndex += 2;
		if (srcLine.compareAt(srcIndex, " a ") || srcLine.compareAt(srcIndex, " the ")) {
			destLine += " had";
		} else {
			destLine += " would";
		}

		srcIndex = startIndex;
		break;

	case 'l':
		if (srcLine[srcIndex + 2] == 'l') {
			// 'll ending
			destLine += " will";
			srcIndex = startIndex;
		}
		break;

	case 'm':
		// 'm ending
		destLine += " am";
		srcIndex = startIndex;
		break;

	case 'r':
		// 're ending
		if (srcLine[srcIndex + 2] == 'e') {
			destLine += " are";
			srcIndex = startIndex;
		}
		break;

	case 's':
		destLine += "s*";
		srcIndex = startIndex;
		break;

	case 't':
		if (srcLine[srcIndex - 1] == 'n' && srcIndex >= 3) {
			if (srcLine[srcIndex - 3] == 'c' && srcLine[srcIndex - 2] == 'a' &&
				(srcIndex == 3 || srcLine[srcIndex - 4])) {
				// can't -> can not
				destLine += 'n';
			} else if (srcLine[srcIndex - 3] == 'w' && srcLine[srcIndex - 2] == 'o' &&
				(srcIndex == 3 || srcLine[srcIndex - 4])) {
				// won't -> will not
				destLine.deleteLastChar();
				destLine.deleteLastChar();
				destLine += "ill";
			} else if (srcLine[srcIndex - 3] == 'a' && srcLine[srcIndex - 2] == 'i' &&
				(srcIndex == 3 || srcLine[srcIndex - 4])) {
				// ain't -> am not
				destLine.deleteLastChar();
				destLine.deleteLastChar();
				destLine += "m";
			} else if (srcLine.hasSuffix(" sha") || 
					(srcIndex == 4 && srcLine.hasSuffix("sha"))) {
				// shan't -> shall not
				destLine.deleteLastChar();
				destLine += "ll";
			}

			destLine += " not";
		}
		break;

	case 'v':
		// 've ending
		if (srcLine[startIndex + 2] == 'e') {
			destLine += " have";
			srcIndex = startIndex;
		}
		break;

	default:
		break;
	}

	return false;
}

void TTparser::searchAndReplace(TTstring &line, const StringArray &strings) {
	int charIndex = 0;
	while (charIndex >= 0)
		charIndex = searchAndReplace(line, charIndex, strings);
}

int TTparser::searchAndReplace(TTstring &line, int startIndex, const StringArray &strings) {
	int lineSize = line.size();
	if (startIndex >= lineSize)
		return -1;

	for (uint idx = 0; idx < strings.size(); idx += 2) {
		const CString &origStr = strings[idx];
		const CString &replacementStr = strings[idx + 1];

		if (!strncmp(line.c_str() + startIndex, origStr.c_str(), strings[idx].size())) {
			// Ensure that that a space follows the match, or the end of string,
			// so the end of the string doesn't match on parts of larger words
			char c = line[startIndex + strings[idx].size()];
			if (c == ' ' || c == '\0') {
				// Replace the text in the line with it's replacement
				line = CString(line.c_str(), line.c_str() + startIndex) + replacementStr +
					CString(line.c_str() + startIndex + origStr.size());

				startIndex += replacementStr.size();
				break;
			}
		}
	}

	// Skip to the end of the current word
	while (startIndex < lineSize && line[startIndex] != ' ')
		++startIndex;
	if (startIndex == lineSize)
		return -1;

	// ..and all spaces following it until the start of the next word
	while (startIndex < lineSize && line[startIndex] == ' ')
		++startIndex;
	if (startIndex == lineSize)
		return -1;

	// Return index of the start of the next word
	return startIndex;
}

int TTparser::replaceNumbers(TTstring &line, int startIndex) {
	int index = startIndex;
	const NumberEntry *numEntry = replaceNumbers2(line, &index);
	if (!numEntry || !(numEntry->_flags & NF_2))
		return index;

	bool flag1 = false, flag2 = false, flag3 = false;
	int total = 0, factor = 0;

	do {
		if (numEntry->_flags & NF_1) {
			flag2 = true;
			if (numEntry->_flags & NF_8)
				flag1 = true;

			if (numEntry->_flags & NF_4) {
				flag3 = true;
				factor *= numEntry->_value;
			}

			if (numEntry->_flags & NF_2) {
				if (flag3) {
					total += factor;
					factor = 0;
				}

				factor += numEntry->_value;
			}
		}
	} while (replaceNumbers2(line, &index));

	if (!flag2)
		return index;

	if (index >= 0) {
		if (line[index - 1] != ' ')
			return index;
	}

	total += factor;
	CTrueTalkManager::_v1 = total;
	if (flag1)
		total = -total;

	CString numStr = CString::format("%d", total);
	line = CString(line.c_str(), line.c_str() + startIndex) + numStr +
		CString(line.c_str() + index);
	return index;
}

const NumberEntry *TTparser::replaceNumbers2(TTstring &line, int *startIndex) {
	int lineSize = line.size();
	int index = *startIndex;
	if (index < 0 || index >= lineSize) {
		*startIndex = -1;
		return nullptr;
	}

	NumberEntry *numEntry = nullptr;

	for (uint idx = 0; idx < _numbers.size(); ++idx) {
		NumberEntry &ne = _numbers[idx];
		if (!strncmp(line.c_str() + index, ne._text.c_str(), ne._text.size())) {
			if ((ne._flags & NF_10) || (index + (int)ne._text.size()) >= lineSize ||
					line[index + ne._text.size()] == ' ') {
				*startIndex += ne._text.size();
				numEntry = &ne;
				break;
			}
		}
	}

	if (!numEntry || !(numEntry->_flags & NF_10)) {
		// Skip to end of current word
		while (*startIndex < lineSize && !Common::isSpace(line[*startIndex]))
			++*startIndex;
	}

	// Skip over following spaces until start of following word is reached
	while (*startIndex < lineSize && Common::isSpace(line[*startIndex]))
		++*startIndex;

	if (*startIndex >= lineSize)
		*startIndex = -1;

	return numEntry;
}

int TTparser::findFrames(TTsentence *sentence) {
	static bool flag;
	_sentenceSub = &sentence->_sub;
	_sentence = sentence;

	TTstring *line = sentence->_normalizedLine.copy();
	TTstring wordString;
	for (;;) {
		// Keep stripping words off the start of the passed input
		wordString = line->tokenize(" \n");
		if (wordString.empty())
			break;

		TTword *srcWord = nullptr;
		TTword *word = _owner->_vocab->getWord(wordString, &word);
		sentence->storeVocabHit(srcWord);

		if (word) {
			// TODO
		} else {

		}
	}


	// TODO
	delete line;
	return 0;
}

} // End of namespace Titanic
