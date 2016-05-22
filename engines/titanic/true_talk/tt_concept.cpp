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

namespace Titanic {

TTconcept::TTconcept() {
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

TTconcept::TTconcept(const TTconcept &src) :
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
	_field2C = 0;
	_field30 = 0;
	_field34 = 0;
	_field38 = 0;
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

void TTconcept::copyFrom(const TTconcept &src) {
	// TODO
}

} // End of namespace Titanic
