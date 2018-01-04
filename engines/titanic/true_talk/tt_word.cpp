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

#include "titanic/true_talk/tt_word.h"
#include "titanic/true_talk/tt_string_node.h"
#include "titanic/titanic.h"

namespace Titanic {

TTword::TTword(const TTstring &str, WordClass wordClass, int id) : _text(str),
		_wordClass(wordClass), _id(id), _tag(0), _field24(0),
		_field28(0), _synP(nullptr), _nextP(nullptr) {
	_status = str.getStatus() == SS_VALID ? SS_VALID : SS_5;
}

TTword::TTword(const TTword *src) {
	if (src->getStatus() != SS_VALID) {
		_status = SS_5;
		return;
	}

	_text = src->_text;
	_wordClass = src->_wordClass;
	_id = src->_id;
	_tag = src->_tag;
	_synP = nullptr;
	_status = SS_VALID;

	TTsynonym *priorSyn = nullptr;
	for (TTsynonym *synP = _synP; synP && !_status;) {
		TTsynonym *newSyn = new TTsynonym(synP);
		if (!newSyn) {
			_status = SS_7;
		} else {
			newSyn->_priorP = priorSyn;
			newSyn->_nextP = nullptr;

			if (priorSyn) {
				priorSyn->_nextP = newSyn;
			} else {
				_synP = newSyn;
			}

			priorSyn = newSyn;
		}
	}

	_nextP = src->_nextP;
	_field24 = src->_field24;
	_field28 = src->_field28;
}

TTword::~TTword() {
	if (_synP) {
		_synP->deleteSiblings();
		delete _synP;
	}
}

void TTword::deleteSiblings() {
	while (_nextP) {
		TTword *next = _nextP;
		_nextP = next->_nextP;
		delete next;
	}
}

int TTword::readSyn(SimpleFile *file) {
	CString str;
	int mode, val1;

	if (!file->scanf("%s %d %d", &str, &mode, &val1))
		return 8;
	if (!testFileHandle(file))
		return 5;

	// Create new synanym node
	TTsynonym *synNode = new TTsynonym(mode, str.c_str(), (FileHandle)val1);

	if (_synP) {
		// A synonym already exists, so add new one as a tail
		// at the end of the linked list of synonyms
		_synP->addToTail(synNode);
	} else {
		// Very first synonym, so set it
		_synP = synNode;
	}

	return 0;
}

void TTword::setSyn(TTsynonym *synP) {
	if (_synP) {
		_synP->deleteSiblings();
		delete _synP;
	}

	_synP = synP;
}

int TTword::setSynStr(TTstring &str) {
	if (str.empty())
		return 4;

	TTstring *newStr = new TTstring(str);
	TTsynonym *newSyn = new TTsynonym(4, newStr);
	setSyn(newSyn);
	return 0;
}

void TTword::appendNode(TTsynonym *node) {
	if (_synP)
		_synP->addToTail(node);
	else
		_synP = node;
}

int TTword::load(SimpleFile *file, WordClass wordClass) {
	CString str1, str2;
	int id;

	if (file->scanf("%d %s %s", &id, &str1, &str2)) {
		_text = str1;
		_id = id;
		_tag = readNumber(str2.c_str());
		_wordClass = wordClass;
		return 0;
	} else {
		return 3;
	}
}

uint TTword::readNumber(const char *str) {
	uint numValue = *str;
	if (*str == '0') {
		numValue = MKTAG('Z', 'Z', 'Z', '[');
	} else {
		++str;
		for (int idx = 0; idx < 3; ++idx, ++str)
			numValue = (numValue << 8) + *str;
	}

	return numValue;
}

bool TTword::testFileHandle(FileHandle file) const {
	if (g_vm->_exeResources.isVocabMode(VOCAB_MODE_EN))
		return true;

	// TODO: Figure out why original compares passed file handle against specific values
	return true;
}

bool TTword::findSynByName(const TTstring &str, TTsynonym *dest, VocabMode mode) const {
	if (!_synP)
		return false;

	const TTsynonym *synP = dynamic_cast<const TTsynonym *>(_synP->findByName(str, mode));
	if (synP) {
		dest->copyFrom(synP);
		dest->_priorP = nullptr;
		dest->_nextP = nullptr;

		return true;
	} else {
		return false;
	}
}

bool TTword::compareTo(const char *str) const {
	return _text == str;
}

TTword *TTword::copy() const {
	return new TTword(this);
}

FileHandle TTword::getSynFile() const {
	return _synP ? _synP->_file : HANDLE_STDIN;
}

bool TTword::checkSynFile(FileHandle file) const {
	return _synP && _synP->_file == file;
}

void TTword::setSynFile(FileHandle file) {
	if (_synP && testFileHandle(file))
		_synP->_file = file;
}

TTstringStatus TTword::getChainStatus() const {
	for (const TTword *word = this; word; word = word->_nextP) {
		if (word->getStatus())
			return word->getStatus();
	}

	return SS_VALID;
}

TTword *TTword::copyWords() {
	// Replicate the word and all following words it's linked to
	TTword *result = copy();
	for (TTword *word = result; word->_nextP; word = word->_nextP)
		word->_nextP = word->_nextP->copy();

	return result;
}

} // End of namespace Titanic
