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

#include "common/file.h"
#include "titanic/true_talk/tt_vocab.h"
#include "titanic/true_talk/tt_adj.h"
#include "titanic/true_talk/tt_action.h"
#include "titanic/true_talk/tt_adj.h"
#include "titanic/true_talk/tt_major_word.h"
#include "titanic/true_talk/tt_picture.h"
#include "titanic/true_talk/tt_pronoun.h"
#include "titanic/titanic.h"

namespace Titanic {

TTvocab::TTvocab(int val): _headP(nullptr), _tailP(nullptr), _word(nullptr),
		_fieldC(0), _field10(0), _vocabMode(val) {
	_field14 = load("STVOCAB.TXT");
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
		int mode = file->readNumber();
		TTstring space(" ");

		switch (mode) {
		case 0: {
			if (_word)
				result = _word->readSyn(file);
			skipFlag = true;
			break;
		}

		case 1: {
			TTaction *word = new TTaction(space, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case 2: {
			TTpicture *word = new TTpicture(space, 0, 0, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case 3:
		case 9: {
			TTmajorWord *word = new TTmajorWord(space, 0, 0, 0);
			result = word->load(file, mode);
			_word = word;
			break;
		}

		case 4:
		case 5:
		case 7: {
			TTword *word = new TTword(space, 0, 0);
			result = word->load(file, mode);
			_word = word;
			break;
		}

		case 8: {
			TTadj *word = new TTadj(space, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case 6: {
			TTpronoun *word = new TTpronoun(space, 0, 0, 0, 0);
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
	TTword *existingWord = findWord(word->_string);

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
		if (_vocabMode != 3 || strcmp(word->c_str(), str)) {
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

TTword *TTvocab::getPrimeWord(TTstring &str, TTword **words) {
	TTsynonym *synonym = new TTsynonym();
	char c = str.charAt(0);
	TTword *returnWord = nullptr;

	if (!Common::isDigit(c)) {
		returnWord = new TTword(str, 3, 300);
	} else {
		TTword *foundWord = nullptr;
		for (TTword *vocabP = _headP; vocabP && !foundWord; vocabP = vocabP->_nextP) {
			if (_vocabMode == 3 && !strcmp(str.c_str(), vocabP->c_str())) {
				foundWord = vocabP->copy();
				foundWord->_nextP = nullptr;
				foundWord->setSyn(nullptr);
			} else {
				vocabP->findSynByName(str, synonym, _vocabMode);
				// TODO
			}
		}

		// TODO

	}

//	if (words)
//		*words = vocabList;
	delete synonym;
	
	return returnWord;
}

} // End of namespace Titanic
