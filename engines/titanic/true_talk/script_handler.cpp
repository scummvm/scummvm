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

#include "titanic/true_talk/script_handler.h"
#include "titanic/titanic.h"

namespace Titanic {

/*------------------------------------------------------------------------*/

CScriptHandler::CScriptHandler(CTitleEngine *owner, int val1, int val2) :
		_owner(owner), _script(owner->_script), _resources(g_vm->_exeResources),
		_sub1(), _sub2(this), _field10(0), _inputCtr(0), 
		_field20(0), _field24(0), _field28(0), _field2C(0), _field30(0) {
	g_vm->_scriptHandler = this;
	g_vm->_script = _script;
	g_vm->_exeResources.reset(this, val1, val2);
	_vocab = new TTvocab(val2);
}

CScriptHandler::~CScriptHandler() {
	delete _vocab;
}

ScriptChangedResult CScriptHandler::scriptChanged(TTRoomScript *roomScript, TTNpcScript *npcScript, uint dialogueId) {
	if (!npcScript || !roomScript) {
		++_inputCtr;
		return SCR_5;
	}

	ScriptChangedResult result = roomScript->notifyScript(npcScript, dialogueId);
	if (result == SCR_1)
		result = npcScript->notifyScript(roomScript, dialogueId);

	if (result != SCR_3 && result != SCR_4)
		return result;

	error("TODO: CScriptHandler::scriptChanged");
}

void CScriptHandler::processInput(TTRoomScript *roomScript, TTNpcScript *npcScript,
		const TTString &line) {
	if (!roomScript || !line.isValid())
		return;
	
	// TODO
}

SimpleFile *CScriptHandler::openResource(const CString &name) {
	return _owner->open(name);
}

} // End of namespace Titanic
