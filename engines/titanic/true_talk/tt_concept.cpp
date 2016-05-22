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
#include "titanic/true_talk/tt_script_base.h"
#include "titanic/true_talk/tt_word.h"
#include "titanic/titanic.h"

namespace Titanic {

TTconcept::TTconcept() : _string1(" "), _string2(" "), 
		_scriptP(nullptr), _wordP(nullptr) {
	if (setStatus())
		setScriptType(ST_UNKNOWN_SCRIPT);
	else
		reset();
}

TTconcept::TTconcept(TTscriptBase *script, ScriptType scriptType) :
		_string1(" "), _string2(" "), _wordP(nullptr), _scriptP(nullptr) {
	if (!script->getStatus()) {
		setScriptType(scriptType);
		_scriptP = script;

		if (scriptType == ST_UNKNOWN_SCRIPT && script->_field8 == 1)
			_scriptType = ST_ROOM_SCRIPT;
	}

	if (_status)
		reset();
}

TTconcept::TTconcept(TTword *word, ScriptType scriptType) :
	_string1(" "), _string2(" "), _wordP(nullptr), _scriptP(nullptr) {
	
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
		_string1(src._string1), _string2(src._string2),
		_wordP(nullptr), _scriptP(nullptr) {

	if (src.getStatus()) {
		_status = SS_5;
	} else {
		if (setStatus()) {
			_status = SS_VALID;
			_scriptP = src._scriptP;
			
			if (src._wordP) {
				_status = initializeWordRef(src._wordP);
				copyFrom(src);
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
	_wordP = word;
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
	return this != nullptr && _wordP != nullptr &&
		_wordP->compareTo(str);
}

void TTconcept::copyFrom(TTconcept &src) {
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

} // End of namespace Titanic
