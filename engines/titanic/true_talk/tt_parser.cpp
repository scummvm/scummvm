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
#include "titanic/support/files_manager.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/true_talk/tt_action.h"
#include "titanic/true_talk/tt_concept.h"
#include "titanic/true_talk/tt_picture.h"
#include "titanic/true_talk/tt_sentence.h"
#include "titanic/true_talk/tt_word.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

TTparser::TTparser(CScriptHandler *owner) : _owner(owner), _sentenceConcept(nullptr),
		_sentence(nullptr), _fieldC(0), _field10(0), _field14(0),
		_currentWordP(nullptr), _nodesP(nullptr), _conceptP(nullptr) {
	loadArrays();
}

TTparser::~TTparser() {
	clear();
}

void TTparser::clear() {
	if (_nodesP) {
		_nodesP->deleteSiblings();
		delete _nodesP;
		_nodesP = nullptr;
	}

	if (_conceptP) {
		_conceptP->deleteSiblings();
		delete _conceptP;
		_conceptP = nullptr;
	}

	delete _currentWordP;
	_currentWordP = nullptr;
}

void TTparser::loadArray(StringArray &arr, const CString &name) {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource(name);
	while (r->pos() < r->size())
		arr.push_back(readStringFromStream(r));
	delete r;
}

void TTparser::loadArrays() {
	loadArray(_replacements1, "TEXT/REPLACEMENTS1");
	loadArray(_replacements2, "TEXT/REPLACEMENTS2");
	loadArray(_replacements3, "TEXT/REPLACEMENTS3");
	if (g_language == Common::DE_DEU)
		loadArray(_replacements4, "TEXT/REPLACEMENTS4");
	loadArray(_phrases, "TEXT/PHRASES");
	loadArray(_pronouns, "TEXT/PRONOUNS");

	Common::SeekableReadStream *r = g_vm->_filesManager->getResource("TEXT/NUMBERS");
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

	if (g_language == Common::DE_DEU) {
		preprocessGerman(sentence->_normalizedLine);
	} else {
		// Scan for and replace common slang and contractions with verbose versions
		searchAndReplace(sentence->_normalizedLine, _replacements1);
		searchAndReplace(sentence->_normalizedLine, _replacements2);
	}

	// Check entire normalized line against common phrases to replace
	for (uint idx = 0; idx < _phrases.size(); idx += 2) {
		if (!_phrases[idx].compareTo(sentence->_normalizedLine))
			sentence->_normalizedLine = _phrases[idx + 1];
	}

	if (g_language == Common::DE_DEU) {
		// Scan for and replace common slang and contractions with verbose versions
		searchAndReplace(sentence->_normalizedLine, _replacements1);
		searchAndReplace(sentence->_normalizedLine, _replacements2);
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
			(*destLine) += tolower(c);
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

bool TTparser::normalizeContraction(const TTstring &srcLine, int &srcIndex, TTstring &destLine) {
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
				lineSize = line.size();

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
	int total = 0, factor = 0, endIndex;

	do {
		if (!(numEntry->_flags & NF_1)) {
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

		endIndex = index;
	} while ((numEntry = replaceNumbers2(line, &index)) != nullptr);

	index = endIndex;
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
	line = CString::format("%s%s%s",
		CString(line.c_str(), line.c_str() + startIndex).c_str(),
		numStr.c_str(),
		(index == -1) ? "" : line.c_str() + index - 1
	);

	index = startIndex + numStr.size();
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
	_sentenceConcept = &sentence->_sentenceConcept;
	_sentence = sentence;

	TTstring *line = sentence->_normalizedLine.copy();
	TTstring wordString;
	int status = 0;
	for (int ctr = 1; status <= 1; ++ctr) {
		// Keep stripping words off the start of the passed input
		wordString = line->tokenize(" \n");
		if (wordString.empty())
			break;

		TTword *srcWord = nullptr;
		TTword *word = _owner->_vocab->getWord(wordString, &srcWord);
		sentence->storeVocabHit(srcWord);

		if (!word && ctr == 1) {
			word = new TTword(wordString, WC_UNKNOWN, 0);
		}

		for (TTword *currP = word; currP && status <= 1; currP = currP->_nextP)
			status = processRequests(currP);

		if (word) {
			word->deleteSiblings();
			delete word;
		}
	}

	if (status <= 1) {
		status = checkForAction();
		clear();
	}

	delete line;
	return status;
}

int TTparser::loadRequests(TTword *word) {
	int status = 0;

	if (word->_tag != MKTAG('Z', 'Z', 'Z', '['))
		addNode(word->_tag);

	switch (word->_wordClass) {
	case WC_UNKNOWN:
		break;

	case WC_ACTION:
		if (word->_id != 0x70 && word->_id != 0x71)
			addNode(CHECK_COMMAND_FORM);
		addNode(SET_ACTION);

		switch (word->_id) {
		case 101:
		case 110:
			addNode(SEEK_OBJECT);
			addNode(SEEK_ACTOR);
			break;

		case 102:
			addNode(SEEK_ACTOR);
			break;

		case 103:
		case 111:
			addNode(SEEK_FROM);
			addNode(SEEK_TO);
			addNode(SEEK_OBJECT);
			addNode(SEEK_ACTOR);
			break;

		case 104:
		case 107:
			addNode(SEEK_NEW_FRAME);
			addNode(SEEK_OBJECT);
			addNode(SEEK_ACTOR);
			break;

		case 106:
			addNode(SEEK_TO);
			addNode(SEEK_ACTOR);
			break;

		case 108:
			addNode(SEEK_OBJECT);
			addNode(SEEK_ACTOR);
			addNode(WORD_TYPE_IS_SENTENCE_TYPE);
			break;

		case 112:
		case 113:
			addNode(SEEK_STATE);
			addNode(SEEK_OBJECT);
			break;

		default:
			break;
		}

		if (_sentenceConcept) {
			if (_sentenceConcept->get18() == 0 || _sentenceConcept->get18() == 2) {
				TTaction *action = dynamic_cast<TTaction *>(word);
				_sentenceConcept->set18(action->getVal());
			}
		}
		break;

	case WC_THING:
		if (word->checkTag() && _sentence->_field58 > 0)
			_sentence->_field58--;
		addNode(SEEK_MODIFIERS);
		break;

	case WC_ABSTRACT:
		switch (word->_id) {
		case 300:
			addNode(SEEK_MODIFIERS);
			status = 1;
			break;

		case 306:
			addNode(WORD_TYPE_IS_SENTENCE_TYPE);
			addNode(SEEK_ACTOR);
			break;

		case 307:
		case 308:
			addNode(WORD_TYPE_IS_SENTENCE_TYPE);
			break;

		default:
			break;
		}

		if (status != 1) {
			addToConceptList(word);
			addNode(SEEK_STATE);
			addNode(SEEK_MODIFIERS);
		}
		break;

	case WC_ARTICLE:
		addNode(EXPECT_THING);
		status = 1;
		break;

	case WC_CONJUNCTION:
		if (_sentence->checkCategory()) {
			_sentenceConcept->_field1C = 1;
			_sentenceConcept = _sentenceConcept->addSibling();
			clear();
		} else {
			addNode(WORD_TYPE_IS_SENTENCE_TYPE);
		}
		break;

	case WC_PRONOUN:
		status = fn2(word);
		break;

	case WC_PREPOSITION:
		switch (word->_id) {
		case 700:
			addNode(SEEK_OBJECT_OVERRIDE);
			addNode(SEEK_OBJECT);
			break;
		case 701:
			addNode(SEEK_LOCATION);
			break;
		case 702:
			status = 1;
			break;
		case 703:
			addNode(SEEK_TO_OVERRIDE);
			break;
		case 704:
			addNode(SEEK_FROM_OVERRIDE);
			break;
		default:
			break;
		}
		break;

	case WC_ADJECTIVE:
		if (word->_id == 304) {
			// Nothing
		} else if (word->_id == 801) {
			addNode(STATE_IDENTITY);
		} else {
			if (word->proc16())
				_sentence->_field58++;
			if (word->proc17())
				_sentence->_field58++;
		}
		break;

	case WC_ADVERB:
		switch (word->_id) {
		case 900:
		case 901:
		case 902:
		case 904:
			if (_sentence->_category == 9) {
				_sentenceConcept->_field1C = 1;
				_sentenceConcept = _sentenceConcept->addSibling();
				addNode(CHECK_COMMAND_FORM);
			}
			else {
				addNode(WORD_TYPE_IS_SENTENCE_TYPE);
				addNode(SEEK_STATE);
				addNode(CHECK_COMMAND_FORM);
			}
			break;

		case 905:
		case 907:
		case 908:
		case 909:
			addNode(WORD_TYPE_IS_SENTENCE_TYPE);
			break;

		case 906:
			addNode(WORD_TYPE_IS_SENTENCE_TYPE);
			status = 1;
			break;

		case 910:
			addNode(SEEK_ACTOR);
			addNode(COMPLEX_VERB);
			addNode(WORD_TYPE_IS_SENTENCE_TYPE);
			addNode(SEEK_MODIFIERS);
			status = 1;
			break;

		default:
			break;
		}

		if (word->_id == 906) {
			addNode(SEEK_MODIFIERS);
			status = 1;
		}
		break;

	default:
		break;
	}

	return status;
}

int TTparser::considerRequests(TTword *word) {
	if (!_nodesP || !word)
		return 0;

	TTconcept *concept = nullptr;
	int status = 0;
	bool flag = false;
	bool modifierFlag = false;
	int seekVal = 0;

	for (TTparserNode *nodeP = _nodesP; nodeP; ) {
		switch (nodeP->_tag) {
		case CHECK_COMMAND_FORM:
			if (_sentenceConcept->_concept1P && _sentence->_category == 1 &&
					!_sentenceConcept->_concept0P) {
				TTconcept *newConcept = new TTconcept(_sentence->_npcScript, ST_NPC_SCRIPT);
				_sentenceConcept->_concept0P = newConcept;
				_sentenceConcept->_field18 = 3;
			}

			flag = true;
			break;

		case EXPECT_THING:
			if (!word->_wordClass) {
				word->_wordClass = WC_THING;
				addToConceptList(word);
				addNode(SEEK_MODIFIERS);
			}

			flag = true;
			break;

		case OBJECT_IS_TO:
			flag = resetConcept(&_sentenceConcept->_concept2P, 3);
			break;

		case SEEK_ACTOR:
		case MKTAG('S', 'A', 'C', 'T'):
			if (!_sentenceConcept->_concept0P) {
				flag = filterConcepts(5, 0);
			} else if (_sentenceConcept->_concept0P->compareTo("?") &&
						(_sentenceConcept->_concept1P && isWordId(_sentenceConcept->_concept1P, 113)) &&
						word->_wordClass == WC_THING) {
				TTconcept *oldConcept = _sentenceConcept->_concept0P;
				_sentenceConcept->_concept0P = nullptr;
				flag = filterConcepts(5, 2);
				if (flag)
					delete oldConcept;
			} else {
				flag = true;
			}
			break;

		case SEEK_OBJECT:
			if (_sentenceConcept->_concept2P && _sentenceConcept->_concept2P->compareTo(word)) {
				flag = true;
			} else if (!_sentenceConcept->_concept2P) {
				if (filterConcepts(5, 2) && _sentenceConcept->_concept2P->checkWordId1())
					addNode(SEEK_OBJECT);
			} else if (word->_wordClass == WC_THING && _sentence->fn2(2, TTstring("?"), _sentenceConcept)) {
				TTconcept *oldConcept = _sentenceConcept->_concept2P;
				flag = filterConcepts(5, 2);
				_sentenceConcept->_concept2P->_field20 = oldConcept->get20();
				if (flag)
					delete oldConcept;
			} else if (!_sentenceConcept->_concept3P &&
					(!_sentenceConcept->_concept1P || (getWordId(_sentenceConcept->_concept1P) != 113 &&
					getWordId(_sentenceConcept->_concept1P) != 112)) &&
					_sentenceConcept->_concept2P->checkWordId1() &&
					(word->_wordClass == WC_THING || word->_wordClass == WC_PRONOUN)) {
				_sentenceConcept->changeConcept(0, &_sentenceConcept->_concept2P, 3);

				if (_conceptP && isWordId(_conceptP, word->_id)) {
					status = _sentenceConcept->replaceConcept(0, 2, _conceptP);
					removeConcept(_conceptP);
				} else {
					status = _sentenceConcept->createConcept(0, 2, word);
				}

				if (!status && !_sentenceConcept->_concept4P && _sentenceConcept->_concept0P) {
					TTconcept *oldConcept = _sentenceConcept->_concept2P;
					flag = filterConcepts(5, 2);
					_sentenceConcept->_concept2P->_field20 = oldConcept->get20();
					if (flag)
						delete oldConcept;
				} else {
					flag = true;
				}
			}
			break;

		case SEEK_OBJECT_OVERRIDE:
			if ((word->_wordClass == WC_THING || word->_wordClass == WC_PRONOUN) &&
					_sentence->fn2(2, TTstring("thePlayer"), _sentenceConcept) &&
					!_sentenceConcept->_concept3P) {
				_sentenceConcept->_concept3P = _sentenceConcept->_concept2P;
				_sentenceConcept->_concept2P = nullptr;

				flag = filterConcepts(5, 2);
				if (!flag) {
					status = _sentenceConcept->createConcept(0, 2, word);
				}
			}
			break;

		case SEEK_TO:
			if (!_sentenceConcept->_concept3P) {
				if (!filterConcepts(8, 3))
					flag = filterConcepts(3, 3);
			} else {
				flag = true;
			}
			break;

		case SEEK_FROM:
			if (!_sentenceConcept->_concept4P) {
				if (!filterConcepts(8, 4))
					flag = filterConcepts(3, 3);
			} else {
				flag = true;
			}
			break;

		case SEEK_TO_OVERRIDE:
			if (word->_wordClass == WC_ACTION) {
				status = _sentenceConcept->createConcept(0, 1, word);
				if (!status) {
					seekVal = _sentenceConcept->_field18;
					_sentenceConcept->_field18 = 4;
					flag = true;
				}
			} else if (word->_id == 703) {
				if (_sentenceConcept->_concept2P) {
					delete _sentenceConcept->_concept2P;
					_sentenceConcept->_concept2P = nullptr;
				}

				if (_sentenceConcept->_concept4P || !_sentenceConcept->_concept0P) {
					addNode(SEEK_TO);
				} else {
					_sentenceConcept->changeConcept(1, &_sentenceConcept->_concept0P, 4);
					concept = nullptr;
					addNode(SEEK_TO);
				}
			} else {
				flag = true;
			}
			break;

		case SEEK_FROM_OVERRIDE:
			if (_sentenceConcept->_concept4P) {
				delete _sentenceConcept->_concept4P;
				_sentenceConcept->_concept4P = nullptr;
			}

			addNode(SEEK_FROM);
			flag = true;
			break;

		case SEEK_LOCATION:
			addNode(SEEK_OBJECT);
			_sentenceConcept->createConcept(0, 5, word);
			flag = true;
			break;

		case SEEK_OWNERSHIP:
			if (word->_id == 601) {
				if (_conceptP->findByWordClass(WC_THING))
					status = _conceptP->setOwner(word, false);

				flag = true;
			}
			break;

		case SEEK_STATE:
			if (_sentenceConcept->_concept5P) {
				if (_sentenceConcept->_concept5P->findByWordId(306) ||
						_sentenceConcept->_concept5P->findByWordId(904)) {
					TTconcept *oldConcept = _sentenceConcept->_concept5P;
					_sentenceConcept->_concept5P = nullptr;
					flag = filterConcepts(9, 5);
					if (flag)
						delete oldConcept;
				} else {
					flag = true;
				}
			} else {
				flag = filterConcepts(9, 5);
				if (!flag && word->_wordClass == WC_ADVERB) {
					status = _sentenceConcept->createConcept(1, 5, word);
					flag = true;
				}
			}
			break;

		case SEEK_MODIFIERS:
			if (!modifierFlag) {
				bool tempFlag = false;

				switch (word->_wordClass) {
				case WC_ACTION:
					status = processModifiers(1, word);
					break;
				case WC_THING:
					status = processModifiers(2, word);
					break;
				case WC_ABSTRACT:
					if (word->_id != 300) {
						status = processModifiers(3, word);
					} else if (!_conceptP || !_conceptP->findByWordClass(WC_THING)) {
						status = processModifiers(3, word);
					} else {
						word->_id = atoi(word->_text.c_str());
					}
					break;
				case WC_PRONOUN:
					if (word->_id != 602)
						addToConceptList(word);
					break;
				case WC_ADJECTIVE: {
					TTconcept *conceptP = _conceptP->findByWordClass(WC_THING);
					if (conceptP) {
						conceptP->_string2 += ' ';
						conceptP->_string2 += word->getText();
					} else {
						status = processModifiers(8, word);
					}
					break;
				}
				case WC_ADVERB:
					if (word->_id == 906) {
						for (TTconcept *currP = _conceptP; currP; currP = currP->_nextP) {
							if (_sentence->isFrameSlotClass(1, WC_ACTION) ||
									_sentence->isFrameSlotClass(1, WC_THING))
								currP->_field34 = 1;
						}
					} else {
						TTconcept *conceptP = _conceptP->findByWordClass(WC_ACTION);

						if (conceptP) {
							conceptP->_string2 += ' ';
							conceptP->_string2 += word->getText();
						} else {
							tempFlag = true;
						}
					}
					break;
				default:
					addToConceptList(word);
					status = 0;
					break;
				}

				if (tempFlag)
					status = _sentenceConcept->createConcept(1, 5, word);

				modifierFlag = true;
				flag = true;
			}
			break;

		case SEEK_NEW_FRAME:
			if (word->_wordClass == WC_ACTION && word->_id != 104 && word->_id != 107) {
				if (concept && (_sentenceConcept->_concept5P || _sentenceConcept->_concept2P)) {
					TTsentenceConcept *oldNode = _sentenceConcept;
					oldNode->_field1C = 2;
					_sentenceConcept = oldNode->addSibling();
					concept = nullptr;

					_sentenceConcept->_concept1P = oldNode->_concept1P;
					_sentenceConcept->_concept5P = oldNode->_concept5P;
					_sentenceConcept->_concept2P = oldNode->_concept2P;

					if (seekVal) {
						seekVal = 0;

						_sentenceConcept->_field18 = oldNode->_field18;
						oldNode->_field18 = seekVal;
					}
				}

				flag = true;
			}
			break;

		case SEEK_STATE_OBJECT:
			if (!_sentenceConcept->_concept5P) {
				addToConceptList(word);
			} else if (_sentenceConcept->concept5WordId() == 113 ||
					_sentenceConcept->concept5WordId() == 112) {
				_sentenceConcept->createConcept(1, 2, word);
			} else {
				addToConceptList(word);
			}

			flag = true;
			break;

		case SET_ACTION:
			if (_sentence->fn4(1, 104, _sentenceConcept) ||
					_sentence->fn4(1, 107, _sentenceConcept)) {
				concept = _sentenceConcept->_concept1P;
				_sentenceConcept->_concept1P = nullptr;
				addNode(SEEK_NEW_FRAME);
			}

			if (_sentence->checkCategory() && word->_id == 113)
				addNode(SEEK_ACTOR);

			if (word->_wordClass == WC_ACTION)
				_sentenceConcept->createConcept(0, 1, word);

			flag = true;
			break;

		case ACTOR_IS_TO:
			_sentenceConcept->changeConcept(1, &_sentenceConcept->_concept0P, 3);
			flag = true;
			break;

		case ACTOR_IS_FROM:
			_sentenceConcept->changeConcept(1, &_sentenceConcept->_concept0P, 4);
			break;

		case ACTOR_IS_OBJECT:
			flag = resetConcept(&_sentenceConcept->_concept0P, 2);
			break;

		case WORD_TYPE_IS_SENTENCE_TYPE:
			if (_sentence->_category == 1 || _sentence->_category == 10) {
				for (TTword *wordP = _currentWordP; wordP; wordP = wordP->_nextP) {
					if (wordP->_id == 906) {
						_sentence->_category = 12;
						flag = true;
						break;
					}
				}

				TTpicture *newPictP;
				TTconcept *newConceptP;
				switch (word->_id) {
				case 108:
					_sentence->_category = 8;
					break;
				case 113:
					if (!_sentenceConcept->_concept3P)
						_sentence->_category = 22;
					break;
				case 306:
					_sentence->_category = 7;
					break;
				case 307:
					_sentence->_category = 24;
					break;
				case 308:
					_sentence->_category = 25;
					break;
				case 501:
					_sentence->_category = 9;
					break;
				case 900:
					_sentence->_category = 5;
					break;
				case 901:
					_sentence->_category = 4;
					break;
				case 904:
					_sentence->_category = 6;
					break;
				case 905:
					_sentence->_category = 11;
					break;
				case 906:
					_sentence->_category = 12;
					break;
				case 907:
					_sentence->_category = 13;
					break;
				case 908:
					_sentence->_category = 2;
					if (!_sentenceConcept->_concept0P) {
						newPictP = new TTpicture(TTstring("?"), WC_THING, 0, 0, 0, 0, 0);
						newConceptP = new TTconcept(newPictP);

						_sentenceConcept->_concept0P = newConceptP;
						delete newPictP;
						addNode(SEEK_ACTOR);
					}
					break;
				case 909:
					_sentence->_category = 3;
					newPictP = new TTpicture(TTstring("?"), WC_THING, 0, 0, 0, 0, 0);
					newConceptP = new TTconcept(newPictP);

					_sentenceConcept->_concept2P = newConceptP;
					delete newPictP;
					addNode(SEEK_ACTOR);
					break;

				default:
					break;
				}
			}

			flag = true;
			break;

		case COMPLEX_VERB:
			if (word->_wordClass == WC_ACTION) {
				flag = true;
			} else if (!_sentenceConcept->_concept1P) {
				TTstring wordStr = word->getText();
				if (wordStr == "do" || wordStr == "doing" || wordStr == "does" || wordStr == "done") {
					TTaction *verbP = new TTaction(TTstring("do"), WC_ACTION, 112, 0,
						_sentenceConcept->get18());
					status = _sentenceConcept->createConcept(1, 1, verbP);
					delete verbP;
					flag = true;
				}
			}
			break;

		case MKTAG('C', 'O', 'M', 'E'):
			addNode(SEEK_TO);
			addNode(SEEK_OBJECT);
			addNode(ACTOR_IS_OBJECT);

			if (!_sentence->_category)
				_sentence->_category = 15;
			break;

		case MKTAG('C', 'U', 'R', 'S'):
		case MKTAG('S', 'E', 'X', 'X'):
			if (_sentence->_field58 > 1)
				_sentence->_field58--;
			flag = true;
			break;

		case MKTAG('E', 'X', 'I', 'T'):
			addNode(SEEK_FROM);
			addNode(SEEK_OBJECT);
			addNode(ACTOR_IS_OBJECT);

			if (!_sentence->_category)
				_sentence->_category = 14;
			break;

		case MKTAG('F', 'A', 'R', 'R'):
			if (_conceptP->findBy20(0))
				_conceptP->_field20 = 2;
			break;

		case MKTAG('F', 'U', 'T', 'R'):
			_sentenceConcept->_field18 = 3;
			break;

		case MKTAG('G', 'O', 'G', 'O'):
			addNode(SEEK_TO);
			addNode(SEEK_OBJECT);
			addNode(ACTOR_IS_OBJECT);

			if (_sentence->_category == 1)
				_sentence->_category = 14;

			flag = true;
			break;

		case MKTAG('H', 'E', 'L', 'P'):
			if (_sentence->_category == 1)
				_sentence->_category = 18;

			flag = true;
			break;

		case MKTAG('L', 'O', 'C', 'F'):
			status = _sentenceConcept->createConcept(1, 5, word);
			if (!status)
				_sentenceConcept->_concept5P->_field20 = 2;

			flag = true;
			break;

		case MKTAG('L', 'O', 'C', 'N'):
			status = _sentenceConcept->createConcept(1, 5, word);
			if (!status)
				_sentenceConcept->_concept5P->_field20 = 1;

			flag = true;
			break;

		case MKTAG('N', 'E', 'A', 'R'):
			if (_conceptP->findBy20(0)) {
				_conceptP->_field20 = 1;
			} else {
				TTpicture *newPictP = new TTpicture(TTstring("?"), WC_THING, 0, 0, 0, 0, 0);
				status = addToConceptList(newPictP);
				_conceptP->_field20 = 1;
				if (!status)
					delete newPictP;
			}

			flag = true;
			break;

		case MKTAG('P', 'A', 'S', 'T'):
			_sentenceConcept->_field18 = 1;
			flag = true;
			break;

		case MKTAG('P', 'L', 'E', 'Z'):
			if (_sentence->_field58 < 10)
				_sentence->_field58++;
			break;

		case MKTAG('P', 'R', 'E', 'Z'):
			_sentenceConcept->_field18 = 2;
			flag = true;
			break;

		case MKTAG('S', 'A', 'A', 'O'):
			addNode(SEEK_OBJECT);
			addNode(SEEK_ACTOR);
			flag = true;
			break;

		case MKTAG('S', 'S', 'T', 'A'):
			addNode(SEEK_STATE);
			addNode(SEEK_OBJECT);
			flag = true;
			break;

		case MKTAG('T', 'E', 'A', 'C'):
			if (_sentence->_category == 1)
				_sentence->_category = 10;

			flag = true;
			break;

		case MKTAG('V', 'O', 'B', 'J'):
			status = _sentenceConcept->createConcept(1, 2, word);
			flag = true;
			break;

		default:
			flag = true;
			break;
		}

		TTparserNode *nextP = dynamic_cast<TTparserNode *>(nodeP->_nextP);
		if (flag) {
			removeNode(nodeP);
			flag = false;
		}

		nodeP = nextP;
	}

	delete concept;
	return status;
}

int TTparser::processRequests(TTword *word) {
	int status = loadRequests(word);
	switch (status) {
	case 0:
		status = considerRequests(word);

		// Iterate through the words
		while (_currentWordP) {
			considerRequests(_currentWordP);
			TTword *nextP = _currentWordP->_nextP;

			delete _currentWordP;
			_currentWordP = nextP;
		}
		break;

	case 1: {
		TTword *newWord = new TTword(word);
		newWord->_nextP = nullptr;

		// Add word to word chain
		if (_currentWordP) {
			// Add at end of existing chain
			for (word = _currentWordP; word->_nextP; word = word->_nextP)
				;
			word->_nextP = newWord;
		} else {
			// First word, so set as head
			_currentWordP = newWord;
		}
		break;
	}

	default:
		warning("unexpected return from consider requests");
		break;
	}

	return status;
}

int TTparser::addToConceptList(TTword *word) {
	TTconcept *concept = new TTconcept(word, ST_UNKNOWN_SCRIPT);
	addConcept(concept);
	return 0;
}

void TTparser::addNode(uint tag) {
	TTparserNode *newNode = new TTparserNode(tag);
	if (_nodesP)
		_nodesP->addToHead(newNode);
	_nodesP = newNode;
}

int TTparser::addConcept(TTconcept *concept) {
	if (!concept)
		return SS_5;

	if (_conceptP)
		concept->_nextP = _conceptP;
	_conceptP = concept;

	return SS_VALID;
}

void TTparser::removeConcept(TTconcept *concept) {
	// If no concept passed, exit immediately
	if (!concept)
		return;

	if (_conceptP == concept) {
		// Concept specified is the ver ystart of the linked list, so reset head pointer
		_conceptP = _conceptP->_nextP;
	} else {
		// Scan through the linked list, looking for the specific concept
		for (TTconcept *currP = _conceptP; currP; currP = currP->_nextP) {
			if (currP->_nextP == concept) {
				// Found match, so unlink the next link from the chain
				currP->_nextP = currP->_nextP->_nextP;
				break;
			}
		}
	}

	// FInally, delete the concept
	concept->_nextP = nullptr;
	delete concept;
}

void TTparser::removeNode(TTparserNode *node) {
	if (!node->_priorP)
		// Node is the head of the chain, so reset parser's nodes pointer
		_nodesP = dynamic_cast<TTparserNode *>(node->_nextP);

	delete node;
}

int TTparser::checkForAction() {
	int status = SS_VALID;
	bool flag = false;
	bool actionFlag = false;

	if (_conceptP && _currentWordP) {
		// Firstly we need to get the next word to process, and remove it from
		// the list pointed to by _currentWordP
		TTword *word = _currentWordP;
		if (word->_nextP) {
			// Chain of words, so we need to find the last word of the chain,
			// and set the last-but-one's _nextP to nullptr to detach the last one
			TTword *prior = nullptr;
			for (; word->_nextP; word = word->_nextP) {
				prior = word;
			}

			if (prior)
				prior->_nextP = nullptr;
		} else {
			// No chain, so singular word can simply be removed
			_currentWordP = nullptr;
			if (word->_id == 906 && _sentence->_category == 1)
				_sentence->_category = 12;
		}

		if (word->_text == "do" || word->_text == "doing" || word->_text == "does" ||
				word->_text == "done") {
			TTstring doStr("do");
			TTaction *action = new TTaction(doStr, WC_ACTION, 112, 0, _sentenceConcept->get18());

			if (!action->isValid()) {
				status = SS_4;
			} else {
				// Have the new action replace the old word instance
				delete word;
				word = action;
				actionFlag = true;
			}
		}

		addToConceptList(word);
		delete word;
		flag = true;
	}

	// Handle any remaining words
	while (_currentWordP) {
		int result = considerRequests(_currentWordP);
		if (result > 1) {
			status = result;
		} else {
			// Delete the top of the word chain
			TTword *wordP = _currentWordP;
			_currentWordP = _currentWordP->_nextP;
			delete wordP;
		}
	}

	if (flag && _conceptP) {
		if (actionFlag && (!_sentenceConcept->_concept1P || isWordId(_sentenceConcept->_concept1P, 113))) {
			_sentenceConcept->replaceConcept(0, 1, _conceptP);
		} else if (!_sentenceConcept->_concept5P) {
			_sentenceConcept->replaceConcept(1, 5, _conceptP);
		} else if (isWordId(_sentenceConcept->_concept5P, 904)) {
			_sentenceConcept->replaceConcept(0, 5, _conceptP);
		}

		removeConcept(_conceptP);
	}

	if (_sentence->fn2(3, TTstring("thePlayer"), _sentenceConcept) && !flag) {
		if (_sentenceConcept->concept1WordId() == 101) {
			_sentence->_category = 16;
		} else if (_sentence->_category != 18 && _sentenceConcept->concept1WordId() == 102) {
			if (_sentence->fn2(0, TTstring("targetNpc"), _sentenceConcept))
				_sentence->_category = 15;
		}
	}

	if (_sentence->fn2(2, TTstring("thePlayer"), _sentenceConcept) &&
			_sentenceConcept->concept1WordId() == 101 && flag)
		_sentence->_category = 17;

	if (!_sentenceConcept->_concept0P && !_sentenceConcept->_concept1P &&
			!_sentenceConcept->_concept2P && !_sentenceConcept->_concept5P && !flag) {
		if (_conceptP)
			filterConcepts(5, 2);

		if (!_sentenceConcept->_concept2P && _sentence->_category == 1)
			_sentence->_category = 0;
	}

	if (_sentence->_field58 < 5 && _sentence->_category == 1 && !flag)
		_sentence->_category = 19;

	for (TTconceptNode *nodeP = &_sentence->_sentenceConcept; nodeP; nodeP = nodeP->_nextP) {
		if (nodeP->_field18 == 0 && nodeP->_concept1P) {
			nodeP->_field18 = _sentence->concept18(nodeP);
		} else if (nodeP->_field18 == 4 && !_sentenceConcept->_concept0P) {
			if (_sentenceConcept->_concept3P) {
				_sentenceConcept->_concept0P = _sentenceConcept->_concept3P;
				_sentenceConcept->_concept3P = nullptr;
			} else if (_sentenceConcept->_concept2P) {
				_sentenceConcept->_concept0P = _sentenceConcept->_concept2P;
				_sentenceConcept->_concept2P = nullptr;
			}
		}
	}

	if (_sentence->_category == 1 && _sentenceConcept->_concept5P &&
			_sentenceConcept->_concept2P) {
		if (_sentence->fn4(1, 113, nullptr)) {
			if (_sentence->fn2(2, TTstring("targetNpc"), nullptr)) {
				_sentence->_category = 20;
			} else if (_sentence->fn2(2, TTstring("thePlayer"), nullptr)) {
				_sentence->_category = 21;
			} else {
				_sentence->_category = 22;
			}
		}
	} else if (!_sentenceConcept->_concept0P && !_sentenceConcept->_concept1P &&
			!_sentenceConcept->_concept2P && !_sentenceConcept->_concept5P) {
		if (_conceptP)
			filterConcepts(5, 2);

		if (!_sentenceConcept->_concept2P && _sentence->_category == 1)
			_sentence->_category = 0;
	}

	return status;
}

int TTparser::fn2(TTword *word) {
	switch (word->_id) {
	case 600:
		addNode(SEEK_STATE);
		return 0;

	case 601:
		addNode(SEEK_OWNERSHIP);
		return 1;

	case 602:
	case 607:
		return checkReferent(dynamic_cast<TTpronoun *>(word));

	case 608:
		return 1;

	default:
		return 0;
	}
}

int TTparser::checkReferent(TTpronoun *pronoun) {
	TTconcept *concept;

	switch (pronoun->getVal()) {
	case 0:
		return 0;

	case 1:
		concept = new TTconcept(_owner->_script, ST_ROOM_SCRIPT);
		break;

	case 2:
		concept = new TTconcept(_sentence->_npcScript, ST_NPC_SCRIPT);
		break;

	default:
		concept = new TTconcept(pronoun, (ScriptType)pronoun->getVal());
		break;
	}

	addConcept(concept);
	return 0;
}

void TTparser::conceptChanged(TTconcept *newConcept, TTconcept *oldConcept) {
	if (!oldConcept && newConcept != _currentConceptP)
		_currentConceptP = nullptr;
	else if (oldConcept && oldConcept == _currentConceptP)
		_currentConceptP = newConcept;
}

bool TTparser::checkConcept2(TTconcept *concept, int conceptMode) {
	switch (conceptMode) {
	case 3:
		return concept->checkWordId2();

	case 5:
		return concept->checkWordClass();

	case 8:
		return concept->checkWordId1();

	case 9:
		if (concept->checkWordId3())
			return true;

		if (_sentenceConcept->_concept2P) {
			if (!_sentenceConcept->_concept2P->checkWordId2() || !concept->checkWordId2()) {
				return _sentenceConcept->_concept2P->checkWordClass() &&
					concept->checkWordClass();
			}
		}
		break;

	default:
		break;
	}

	return false;
}

int TTparser::filterConcepts(int conceptMode, int conceptIndex) {
	int result = 0;

	for (TTconcept *nextP, *currP = _conceptP; currP && !result; currP = nextP) {
		nextP = currP->_nextP;

		if (checkConcept2(currP, conceptMode)) {
			TTconcept **ptrPP = _sentenceConcept->setConcept(conceptIndex, currP);
			TTconcept *newConcept = new TTconcept(*currP);
			*ptrPP = newConcept;

			if (newConcept->isValid()) {
				removeConcept(currP);
				(*ptrPP)->_nextP = nullptr;
				result = 1;
			} else {
				result = -2;
			}
		}
	}

	return result;
}

bool TTparser::resetConcept(TTconcept **conceptPP, int conceptIndex) {
	TTconcept **ptrPP = _sentenceConcept->setConcept(conceptIndex, nullptr);

	if (!*ptrPP)
		return 0;

	int result = _sentenceConcept->changeConcept(1, conceptPP, conceptIndex);
	if (*conceptPP)
		_sentenceConcept->setConcept(conceptIndex, *conceptPP);

	return !result;
}

int TTparser::processModifiers(int modifier, TTword *word) {
	TTconcept *newConcept = new TTconcept(word, ST_UNKNOWN_SCRIPT);

	// Cycles through each word
	for (TTword *currP = _currentWordP; currP && currP != word; currP = _currentWordP) {
		if ((modifier == 2 && currP->_wordClass == WC_ADJECTIVE) ||
				(modifier == 1 && currP->_wordClass == WC_ADVERB)) {
			newConcept->_string2 += ' ';
			newConcept->_string2 += _currentWordP->getText();
		} else if (word->_id == 113 && currP->_wordClass == WC_ADJECTIVE) {
			addToConceptList(currP);
			addNode(SEEK_STATE);
		}

		if (modifier == 2 || modifier == 3) {
			switch (_currentWordP->_id) {
			case 94:
				_currentConceptP->setOwner(newConcept);
				if (_currentWordP) {
					_currentWordP->deleteSiblings();
					delete _currentWordP;
					_currentWordP = nullptr;
				}

				delete newConcept;
				newConcept = nullptr;
				break;

			case 204:
				newConcept->_field34 = 1;
				if (_sentence->_category == 1)
					_sentence->_category = 12;
				newConcept->_field14 = 1;
				break;

			case 300:
				newConcept->set1C(atoi(_currentWordP->_text.c_str()));
				break;

			case 400:
				newConcept->_field14 = 2;
				break;

			case 401:
				newConcept->_field14 = 1;
				break;

			case 601:
				newConcept->setOwner(_currentWordP, false);
				break;

			case 608:
				if (_currentWordP->comparePronounTo(10)) {
					newConcept->_field20 = 1;
				} else if (_currentWordP->comparePronounTo(11)) {
					newConcept->_field20 = 2;
				}

			default:
				break;
			}
		}

		if (_currentWordP) {
			// Detaches word and deletes it
			TTword *wordP = _currentWordP;
			_currentWordP = wordP->_nextP;

			wordP->_nextP = nullptr;
			delete wordP;
		}
	}

	if (newConcept) {
		newConcept->setFlag(true);
		_currentConceptP = newConcept;
		addConcept(newConcept);
	}

	return 0;
}

void TTparser::preprocessGerman(TTstring &line) {
	static const char *const SUFFIXES[12] = {
		" ", "est ", "em ", "en ", "er ", "es ",
		"et ", "st ", "s ", "e ", "n ", "t "
	};

	for (uint idx = 0; idx < _replacements4.size(); ++idx) {
		if (!line.hasSuffix(_replacements4[idx]))
			continue;

		const char *lineP = line.c_str();
		const char *p = strstr(lineP, _replacements4[idx].c_str());
		if (!p || p == lineP || *(p - 1) != ' ')
			continue;

		const char *wordEndP = p + _replacements4[idx].size();

		for (int sIdx = 0; sIdx < 12; ++sIdx) {
			const char *suffixP = SUFFIXES[sIdx];
			if (!strncmp(wordEndP, suffixP, strlen(suffixP))) {
				// Form a new line with the replacement word
				const char *nextWordP = wordEndP + strlen(suffixP);
				line = Common::String::format("%s %s %s",
					Common::String(lineP, p).c_str(),
					_replacements4[idx + 1].c_str(),
					nextWordP
					);
				return;
			}
		}
	}
}

} // End of namespace Titanic
