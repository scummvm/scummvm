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
#include "titanic/true_talk/st_vocab.h"
#include "titanic/titanic.h"

namespace Titanic {

STVocab::STVocab(int val): _pHead(nullptr), _pTail(nullptr), _word(nullptr),
		_fieldC(0), _field10(0), _field18(val) {
	_field14 = load("STVOCAB.TXT");
}

int STVocab::load(const CString &name) {
	SimpleFile *file = g_vm->_fileReader._owner->openResource(name);
	int result = 0;
	bool skipFlag;

	while (!result && !file->eos()) {
		skipFlag = false;
		int mode = file->readNumber();
		TTString space(" ");

		switch (mode) {
		case 0: {
			if (_word)
				result = _word->readSyn(file);
			skipFlag = true;
			break;
		}

		case 1: {
			TTword2 *word = new TTword2(space, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case 2: {
			TTword3 *word = new TTword3(space, 0, 0, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case 3:
		case 9: {
			TTword1 *word = new TTword1(space, 0, 0, 0);
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
			TTword4 *word = new TTword4(space, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case 6: {
			TTword5 *word = new TTword5(space, 0, 0, 0, 0);
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

void STVocab::addWord(TTword *word) {
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
	} else if (_pTail) {
		_pTail->_pNext = word;
		_pTail = word;
	} else {
		if (!_pHead)
			_pHead = word;

		_pTail = word;
	}
}

TTword *STVocab::findWord(const TTString &str) {
	TTsynonymNode *tempNode = new TTsynonymNode();
	bool flag = false;
	TTword *word = _pHead;

	while (!flag) {
		if (_field18 != 3 || strcmp(word->c_str(), str)) {
			if (word->scanCopy(str, tempNode, _field18))
				word = word->_pNext;
			else
				flag = true;
		} else {
			flag = true;
		}
	}

	delete tempNode;
	return word;
}

} // End of namespace Titanic
