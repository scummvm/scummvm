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

#include "titanic/true_talk/tt_concept.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/true_talk/tt_script_base.h"
#include "titanic/true_talk/tt_word.h"
#include "titanic/titanic.h"

namespace Titanic {

TTconcept::TTconcept() : _string1(" "), _string2(" "),
		_nextP(nullptr), _scriptP(nullptr), _wordP(nullptr), _word2P(nullptr), _status(SS_VALID),
		_scriptType(ST_UNKNOWN_SCRIPT), _field14(0), _field1C(0), _field20(0), _field30(0), _field34(0) {
	if (setStatus())
		setScriptType(ST_UNKNOWN_SCRIPT);
	else
		reset();
}

TTconcept::TTconcept(TTscriptBase *script, ScriptType scriptType) :
		_string1(" "), _string2(" "), _nextP(nullptr), _wordP(nullptr), _word2P(nullptr), _scriptP(nullptr),
		_status(SS_VALID), _scriptType(ST_UNKNOWN_SCRIPT), _field14(0), _field1C(0), _field20(0), _field30(0),
		_field34(0) {
	if (!script->getStatus()) {
		setScriptType(scriptType);
		_scriptP = script;

		if (scriptType == ST_UNKNOWN_SCRIPT && script->_id == 1)
			_scriptType = ST_ROOM_SCRIPT;
	}

	if (_status)
		reset();
}

TTconcept::TTconcept(TTword *word, ScriptType scriptType) :
		_string1(" "), _string2(" "), _nextP(nullptr), _wordP(nullptr), _word2P(nullptr), _scriptP(nullptr),
		_status(SS_VALID), _scriptType(ST_UNKNOWN_SCRIPT), _field14(0), _field1C(0), _field20(0),
		_field30(0), _field34(0), _flag(false) {
	if (!word || !setStatus() || word->getStatus()) {
		_status = SS_5;
	} else {
		_status = initializeWordRef(word);
		if (!_status)
			setScriptType(scriptType);
	}

	if (_status)
		reset();
}

TTconcept::TTconcept(TTconcept &src) :
		_string1(src._string1), _string2(src._string2), _nextP(nullptr),
		_wordP(nullptr), _word2P(nullptr), _scriptP(nullptr), _status(SS_VALID),
		_scriptType(ST_UNKNOWN_SCRIPT), _field14(0), _field1C(0), _field20(0),
		_field30(0), _field34(0), _flag(false) {
	if (src.getStatus()) {
		_status = SS_5;
	} else {
		if (setStatus()) {
			_status = SS_VALID;
			_scriptP = src._scriptP;

			if (src._wordP) {
				_status = initializeWordRef(src._wordP);
				initialize(src);
			}
		}
	}

	if (_status)
		reset();
}

TTconcept::~TTconcept() {
	if (_word2P) {
		_word2P->deleteSiblings();
		delete _word2P;
	}
	delete _wordP;

	if (_flag)
		g_vm->_exeResources._owner->setParserConcept(this, nullptr);
}

void TTconcept::deleteSiblings() {
	for (TTconcept *currP = _nextP, *nextP; currP; currP = nextP) {
		nextP = currP->_nextP;
		delete currP;
	}

	_nextP = nullptr;
}

bool TTconcept::setStatus() {
	if (_string1.isValid() && _string2.isValid()) {
		_status = SS_VALID;
		return true;
	} else {
		_status = SS_11;
		return false;
	}
}

void TTconcept::setScriptType(ScriptType scriptType) {
	_nextP = nullptr;
	_field14 = 0;
	_scriptType = scriptType;
	_field1C = -1;
	_field20 = 0;
	_word2P = nullptr;
	_field30 = 0;
	_field34 = 0;
	_flag = false;
	_status = 0;
}

int TTconcept::initializeWordRef(TTword *word) {
	delete _wordP;
	_wordP = word->copy();
	return 0;
}

void TTconcept::reset() {
	delete _wordP;
	_wordP = nullptr;
	_scriptP = nullptr;

	int oldStatus = _status;
	setScriptType(ST_UNKNOWN_SCRIPT);
	_status = oldStatus;
}

bool TTconcept::compareTo(const char *str) const {
	return _wordP != nullptr &&
		_wordP->compareTo(str);
}

bool TTconcept::compareTo(TTword *word) const {
	if (_wordP && _wordP->compareTo(word->_text))
		return true;

	if (_scriptP && _scriptP->getId() == 1 && word->comparePronounTo(1))
		return true;

	return false;
}

void TTconcept::initialize(TTconcept &src) {
	_nextP = src._nextP;
	_field14 = src._field14;
	_scriptType = src._scriptType;
	_field1C = src._field1C;
	_field20 = src._field20;

	if (src._word2P) {
		_word2P = src._word2P->copyWords();
		if (src._word2P->getChainStatus())
			_status = 11;
	} else {
		_word2P = nullptr;
	}

	_field30 = src._field30;
	_field34 = src._field34;

	if (src._flag) {
		g_vm->_exeResources._owner->setParserConcept(this, &src);
		src.setFlag(true);
		_flag = true;
	}

	_status = src._status;
}

void TTconcept::copyFrom(TTconcept *src) {
	if (this != src) {
		if (src->getStatus()) {
			_status = SS_5;
		} else {
			_string1 = src->_string1;
			_string2 = src->_string2;

			if (setStatus()) {
				_scriptP = src->_scriptP;
				if (src->_wordP) {
					_status = initializeWordRef(src->_wordP);
					initialize(*src);
				} else {
					_wordP = nullptr;
					initialize(*src);
				}
			}
		}
	}

	if (_status)
		reset();
}

int TTconcept::setOwner(TTconcept *src) {
	if (src->_wordP) {
		TTword *newWord = src->_wordP->copy();
		return setOwner(newWord, 1);
	}

	return 0;
}

int TTconcept::setOwner(TTword *src, bool dontDup) {
	TTword *word = dontDup ? src : src->copy();

	if (word) {
		if (!_word2P) {
			_word2P = word;
		} else {
			// Add word to end of word list
			TTword *tailP = _word2P;
			while (tailP->_nextP)
				tailP = tailP->_nextP;

			tailP->_nextP = word;
		}
	}

	return 0;
}

bool TTconcept::checkWordId1() const {
	return (_wordP && (_wordP->_id == 200 || _wordP->_id == 201 ||
		_wordP->_id == 602 || _wordP->_id == 607)) ||
		(_scriptP && _scriptP->_id <= 2);
}

bool TTconcept::checkWordId2() const {
	return (_wordP && _wordP->_id == 204) || (_scriptP && _scriptP->getId() == 3);
}

bool TTconcept::checkWordId3() const {
	return isWordClass(WC_ABSTRACT) || isWordClass(WC_ADJECTIVE) ||
		(isWordClass(WC_ADVERB) && getTheWordId() != 910);
}

bool TTconcept::checkWordClass() const {
	return _scriptP || (_wordP && (_wordP->_wordClass == WC_THING || _wordP->_wordClass == WC_PRONOUN));
}

const TTstring TTconcept::getText() {
	if (_scriptP)
		return _scriptP->getText();
	else if (_wordP)
		return _wordP->getText();
	else
		return TTstring();
}

TTconcept *TTconcept::findByWordId(int id) {
	for (TTconcept *conceptP = this; conceptP; conceptP = conceptP->_nextP) {
		if (conceptP->_wordP && conceptP->_wordP->_id == id)
			return conceptP;
	}

	return nullptr;
}

TTconcept *TTconcept::findByWordClass(WordClass wordClass) {
	for (TTconcept *conceptP = this; conceptP; conceptP = conceptP->_nextP) {
		if (conceptP->_wordP && conceptP->_wordP->_wordClass == wordClass)
			return conceptP;
	}

	return nullptr;
}

TTconcept *TTconcept::findBy20(int val) {
	for (TTconcept *conceptP = this; conceptP; conceptP = conceptP->_nextP) {
		if (conceptP->_field20 == val)
			return conceptP;
	}

	return nullptr;
}

bool TTconcept::isTheWordId(int id) const {
	return _wordP && _wordP->_id == id;
}

int TTconcept::getTheWordId() const {
	return _wordP ? _wordP->_id : 0;
}

bool isWordId(const TTconcept *concept, int id) {
	return concept ? concept->isTheWordId(id) : 0;
}

int getWordId(const TTconcept *concept) {
	return concept ? concept->getTheWordId() : 0;
}

} // End of namespace Titanic
