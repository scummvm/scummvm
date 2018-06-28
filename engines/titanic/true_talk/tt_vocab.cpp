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

#include "titanic/true_talk/tt_vocab.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/true_talk/tt_action.h"
#include "titanic/true_talk/tt_adj.h"
#include "titanic/true_talk/tt_major_word.h"
#include "titanic/true_talk/tt_picture.h"
#include "titanic/true_talk/tt_pronoun.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"
#include "common/file.h"

namespace Titanic {

TTvocab::TTvocab(VocabMode vocabMode): _headP(nullptr), _tailP(nullptr),
		_word(nullptr), _vocabMode(vocabMode) {
	load("STVOCAB");
}

TTvocab::~TTvocab() {
	if (_headP) {
		_headP->deleteSiblings();
		delete _headP;
		_headP = _tailP = nullptr;
	}
}

int TTvocab::load(const CString &name) {
	SimpleFile *file = g_vm->_exeResources._owner->openResource(name);
	int result = 0;
	bool skipFlag;

	while (!result && !file->eos()) {
		skipFlag = false;
		WordClass wordClass = (WordClass)file->readNumber();
		TTstring space(" ");

		switch (wordClass) {
		case WC_UNKNOWN: {
			if (_word)
				result = _word->readSyn(file);
			skipFlag = true;
			break;
		}

		case WC_ACTION: {
			TTaction *word = new TTaction(space, WC_UNKNOWN, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case WC_THING: {
			TTpicture *word = new TTpicture(space, WC_UNKNOWN, 0, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case WC_ABSTRACT:
		case WC_ADVERB: {
			TTmajorWord *word = new TTmajorWord(space, WC_UNKNOWN, 0, 0);
			result = word->load(file, wordClass);
			_word = word;
			break;
		}

		case WC_ARTICLE:
		case WC_CONJUNCTION:
		case WC_PREPOSITION: {
			TTword *word = new TTword(space, WC_UNKNOWN, 0);
			result = word->load(file, wordClass);
			_word = word;
			break;
		}

		case WC_ADJECTIVE: {
			TTadj *word = new TTadj(space, WC_UNKNOWN, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case WC_PRONOUN: {
			TTpronoun *word = new TTpronoun(space, WC_UNKNOWN, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		default:
			result = 4;
			break;
		}

		if (!skipFlag && _word) {
			if (result) {
				// Something wrong occurred, so delete word
				delete _word;
				_word = nullptr;
			} else {
				// Add the word to the master vocab list
				addWord(_word);
			}
		}
	}

	// Close resource and return result
	delete file;
	return result;
}

void TTvocab::addWord(TTword *word) {
	TTword *existingWord = g_language == Common::DE_DEU ? nullptr :
		findWord(word->_text);

	if (existingWord) {
		if (word->_synP) {
			// Move over the synonym
			existingWord->appendNode(word->_synP);
			word->_synP = nullptr;
		}

		_word = nullptr;
		if (word)
			delete word;
	} else if (_tailP) {
		_tailP->_nextP = word;
		_tailP = word;
	} else {
		if (!_headP)
			_headP = word;

		_tailP = word;
	}
}

TTword *TTvocab::findWord(const TTstring &str) {
	TTsynonym *tempNode = new TTsynonym();
	bool flag = false;
	TTword *word = _headP;

	while (word && !flag) {
		if (_vocabMode != VOCAB_MODE_EN || strcmp(word->c_str(), str)) {
			if (word->findSynByName(str, tempNode, _vocabMode))
				flag = true;
			else
				word = word->_nextP;
		} else {
			flag = true;
		}
	}

	delete tempNode;
	return word;
}

TTword *TTvocab::getWord(TTstring &str, TTword **srcWord) const {
	TTword *word = getPrimeWord(str, srcWord);

	if (!word) {
		TTstring tempStr(str);
		if (tempStr.size() > 2) {
			word = getSuffixedWord(tempStr, srcWord);

			if (!word)
				word = getPrefixedWord(tempStr, srcWord);
		}
	}

	return word;
}

TTword *TTvocab::getPrimeWord(TTstring &str, TTword **srcWord) const {
	TTsynonym tempSyn;
	char c = str.charAt(0);
	TTword *newWord = nullptr;
	TTword *vocabP;

	if (Common::isDigit(c)) {
		// Number
		vocabP = _headP;
		newWord = new TTword(str, WC_ABSTRACT, 300);
	} else {
		// Standard word
		for (vocabP = _headP; vocabP; vocabP = vocabP->_nextP) {
			if (_vocabMode == VOCAB_MODE_EN && !strcmp(str.c_str(), vocabP->c_str())) {
				newWord = vocabP->copy();
				newWord->_nextP = nullptr;
				newWord->setSyn(nullptr);
				break;
			} else if (vocabP->findSynByName(str, &tempSyn, _vocabMode)) {
				// Create a copy of the word and the found synonym
				TTsynonym *newSyn = new TTsynonym(tempSyn);
				newSyn->_nextP = newSyn->_priorP = nullptr;
				newWord = vocabP->copy();
				newWord->_nextP = nullptr;
				newWord->setSyn(newSyn);
				break;
			}
		}
	}

	if (srcWord)
		// Pass out the pointer to the original word
		*srcWord = vocabP;

	// Return the new copy of the word
	return newWord;
}

TTword *TTvocab::getSuffixedWord(TTstring &str, TTword **srcWord) const {
	TTstring tempStr(str);
	TTword *word = nullptr;

	if (g_language == Common::DE_DEU) {
		static const char *const SUFFIXES[11] = {
			"est", "em", "en", "er", "es", "et", "st",
			"s", "e", "n", "t"
		};

		for (int idx = 0; idx < 11; ++idx) {
			if (tempStr.hasSuffix(SUFFIXES[idx])) {
				tempStr.deleteSuffix(strlen(SUFFIXES[idx]));
				word = getPrimeWord(tempStr, srcWord);
				if (word)
					break;
				tempStr = str;
			}
		}

		if (word)
			word->setSynStr(str);
		return word;
	}

	if (tempStr.hasSuffix("s")) {
		tempStr.deleteSuffix(1);
		word = getPrimeWord(tempStr);

		if (!word) {
			if (!tempStr.hasSuffix("e")) {
				tempStr = str;
			} else {
				tempStr.deleteLastChar();
				word = getPrimeWord(tempStr);
			}
		}

	} else if (tempStr.hasSuffix("ing")) {
		tempStr.deleteSuffix(3);
		word = getPrimeWord(tempStr);

		if (word) {
			if (word->_wordClass == 1) {
				delete word;
				word = nullptr;
			} else {
				delete word;
				word = new TTadj(str, WC_ADJECTIVE, 0, 0, 0);
			}
		} else {
			tempStr += "e";
			word = getPrimeWord(tempStr);

			if (word) {
				if (word->_wordClass != 1) {
					delete word;
					word = new TTadj(str, WC_ADJECTIVE, 0, 0, 0);
				}
			} else {
				tempStr.deleteSuffix(2);
				word = getPrimeWord(tempStr);

				if (word) {
					if (word->_wordClass != 1) {
						delete word;
						word = new TTadj(str, WC_ADJECTIVE, 0, 0, 0);
					}
				} else {
					tempStr = str;
				}
			}
		}

	} else if (tempStr.hasSuffix("ed")) {
		tempStr.deleteSuffix(1);
		word = getPrimeWord(tempStr);

		if (!word) {
			tempStr.deleteSuffix(1);
			word = getPrimeWord(tempStr);
		}

		if (word) {
			if (word->_wordClass == WC_ACTION) {
				TTaction *action = dynamic_cast<TTaction *>(word);
				assert(action);
				action->setVal(1);
			}
		} else {
			tempStr = str;
		}

	} else if (tempStr.hasSuffix("ly")) {
		tempStr.deleteSuffix(2);
		word = getPrimeWord(tempStr);

		if (word) {
			delete word;
			word = new TTword(str, WC_ADVERB, 0);
		} else {
			tempStr = str;
		}

	} else if (tempStr.hasSuffix("er")) {
		tempStr.deleteSuffix(1);
		word = getPrimeWord(tempStr);

		if (word) {
			if (word->_wordClass == WC_ADJECTIVE) {
				TTadj *adj = static_cast<TTadj *>(word);
				int val1 = word->proc15();
				int val2 = word->proc15();

				if (val2 < 5) {
					if (--val1 > 0) {
						adj->adjFn1(val1);
					}
				} else {
					if (++val1 < 11) {
						adj->adjFn1(val1);
					}
				}
			}
		} else {
			tempStr.deleteSuffix(1);
			word = getPrimeWord(tempStr);

			if (word) {
				if (word->_wordClass == WC_ADJECTIVE) {
					TTadj *adj = dynamic_cast<TTadj *>(word);
					int val1 = word->proc15();
					int val2 = word->proc15();

					if (val2 < 5) {
						if (--val1 > 0) {
							adj->adjFn1(val1);
						}
					} else {
						if (++val1 < 11) {
							adj->adjFn1(val1);
						}
					}
				}
			} else {
				tempStr.deleteSuffix(1);
				word = getPrimeWord(tempStr);

				if (word && word->_wordClass == WC_ADJECTIVE) {
					TTadj *adj = dynamic_cast<TTadj *>(word);
					int val1 = word->proc15();
					int val2 = word->proc15();

					if (val2 < 5) {
						if (--val1 > 0) {
							adj->adjFn1(val1);
						}
					} else {
						if (++val1 < 11) {
							adj->adjFn1(val1);
						}
					}
				}
			}
		}

	} else if (tempStr.hasSuffix("est")) {
		tempStr.deleteSuffix(2);
		word = getPrimeWord(tempStr);

		if (word) {
			if (word->_wordClass == WC_ADJECTIVE) {
				TTadj *adj = static_cast<TTadj *>(word);
				int val1 = word->proc15();
				int val2 = word->proc15();

				if (val2 < 5) {
					if (--val1 > 0) {
						adj->adjFn1(val1);
					}
				} else {
					if (++val1 < 11) {
						adj->adjFn1(val1);
					}
				}
			}
		} else {
			tempStr.deleteSuffix(1);
			word = getPrimeWord(tempStr);

			if (word) {
				if (word->_wordClass == WC_ADJECTIVE) {
					TTadj *adj = dynamic_cast<TTadj *>(word);
					int val1 = word->proc15();
					int val2 = word->proc15();

					if (val2 < 5) {
						if (--val1 > 0) {
							adj->adjFn1(val1);
						}
					} else {
						if (++val1 < 11) {
							adj->adjFn1(val1);
						}
					}
				}
			} else {
				tempStr.deleteSuffix(1);
				word = getPrimeWord(tempStr);

				if (word) {
					TTadj *adj = dynamic_cast<TTadj *>(word);
					int val1 = word->proc15();
					int val2 = word->proc15();

					if (val2 < 5) {
						if (--val1 > 0) {
							adj->adjFn1(val1);
						}
					} else {
						if (++val1 < 11) {
							adj->adjFn1(val1);
						}
					}
				}
			}
		}

	} else if (tempStr.hasSuffix("s*")) {
		tempStr.deleteSuffix(2);
		word = getPrimeWord(tempStr);

		if (word) {
			if (word->_wordClass == WC_PRONOUN || word->_wordClass == WC_ADVERB) {
				delete word;
				TTstring isStr("is");
				word = getPrimeWord(isStr);
			} else {
				switch (word->_id) {
				case 200:
					if (word->proc10() == 2) {
						delete word;
						word = new TTpronoun(tempStr, WC_PRONOUN, 601, 0, 5);
					} else if (word->proc10() == 1) {
						delete word;
						word = new TTpronoun(tempStr, WC_PRONOUN, 601, 0, 4);
					}
					break;

				case 201:
					delete word;
					word = new TTpronoun(tempStr, WC_PRONOUN, 601, 0, 5);
					break;

				case 202:
				case 203:
					if (word->proc10() == 2) {
						delete word;
						word = new TTpronoun(tempStr, WC_PRONOUN, 601, 0, 5);
					} else {
						int val = word->proc10() == 1 ? 0 : 4;
						delete word;
						word = new TTpronoun(tempStr, WC_PRONOUN, 601, 0, val);
					}
					break;

				case 204:
					delete word;
					word = new TTpronoun(tempStr, WC_PRONOUN, 601, 0, 6);
					break;

				default:
					delete word;
					word = new TTpronoun(tempStr, WC_PRONOUN, 601, 0, 0);
					break;
				}
			}
		}
	}

	if (word)
		word->setSynStr(str);

	return word;
}

TTword *TTvocab::getPrefixedWord(TTstring &str, TTword **srcWord) const {
	TTstring tempStr(str);
	TTword *word = nullptr;
	int prefixLen = 0;

	if (tempStr.hasPrefix("pre")) {
		prefixLen = 3;
	} else if (tempStr.hasPrefix("re") || tempStr.hasPrefix("co")) {
		prefixLen = 2;
	} else if (tempStr.hasPrefix("inter") || tempStr.hasPrefix("multi")) {
		prefixLen = 5;
	} else if (tempStr.hasPrefix("over") || tempStr.hasPrefix("post") || tempStr.hasPrefix("self")) {
		prefixLen = 4;
	}

	if (prefixLen) {
		// Known prefix found, so scan for word without prefix
		tempStr.deletePrefix(prefixLen);
		word = getPrimeWord(tempStr);
		if (word)
			tempStr = str;

	} else if (tempStr.hasPrefix("anti") || tempStr.hasPrefix("counter")) {
		prefixLen = tempStr[0] == 'a' ? 4 : 7;

		tempStr.deletePrefix(prefixLen);
		word = getPrimeWord(tempStr);
		if (!word)
			tempStr = str;
		else if (word->_wordClass == 8) {
			delete word;
			word = nullptr;
		}

	} else if (tempStr.hasPrefix("hyper") || tempStr.hasPrefix("super") ||
			tempStr.hasPrefix("ultra")) {
		tempStr.deletePrefix(5);
		word = getPrimeWord(tempStr);

		if (!word)
			tempStr = str;
		else if (word->_wordClass == WC_ADJECTIVE) {
			TTadj *adj = static_cast<TTadj *>(word);
			int val1 = word->proc15();
			int val2 = word->proc15();

			if (val2 < 5) {
				if (--val1 > 0)
					adj->adjFn1(val1);
			} else if (++val1 < 11) {
				adj->adjFn1(val1);
			}
		}
	}

	if (word) {
		// Set the original word on either the found word or synonym
		if (word->hasSynonyms())
			word->setSynStr(str);
		else
			word->_text = str;
	}

	return word;
}

} // End of namespace Titanic
