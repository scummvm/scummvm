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
	_field0 = 0;
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

void TTconcept::reset() {
	delete _wordP;
	_wordP = nullptr;
	_scriptP = nullptr;

	int oldStatus = _status;
	setScriptType(ST_UNKNOWN_SCRIPT);
	_status = oldStatus;
}

} // End of namespace Titanic
