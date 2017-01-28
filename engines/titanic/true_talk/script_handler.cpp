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
#include "titanic/true_talk/tt_concept.h"
#include "titanic/true_talk/tt_sentence.h"
#include "titanic/true_talk/tt_parser.h"
#include "titanic/true_talk/tt_word.h"
#include "titanic/titanic.h"

namespace Titanic {

/*------------------------------------------------------------------------*/

CScriptHandler::CScriptHandler(CTitleEngine *owner, int val1, int val2) :
		_owner(owner), _script(owner->_script), _parser(this), _inputCtr(0), _conceptObject(nullptr),
		_conceptActor(nullptr), _conceptUnused(nullptr), _conceptVerb(nullptr) {
	g_vm->_scriptHandler = this;
	g_vm->_script = _script;
	g_vm->_exeResources.reset(this, val1, val2);
	_vocab = new TTvocab(val2);
}

CScriptHandler::~CScriptHandler() {
	delete _vocab;
	delete _conceptObject;
	delete _conceptActor;
	delete _conceptUnused;
	delete _conceptVerb;
}

ScriptChangedResult CScriptHandler::scriptChanged(TTroomScript *roomScript, TTnpcScript *npcScript, uint dialogueId) {
	if (!npcScript || !roomScript) {
		++_inputCtr;
		return SCR_5;
	}

	ScriptChangedResult result = roomScript->notifyScript(npcScript, dialogueId);
	if (result == SCR_1)
		result = npcScript->notifyScript(roomScript, dialogueId);

	if (dialogueId == 3 || dialogueId == 4) {
		delete _conceptObject;
		delete _conceptActor;
		delete _conceptUnused;
		delete _conceptVerb;
		_conceptObject = nullptr;
		_conceptActor = nullptr;
		_conceptUnused = nullptr;
		_conceptVerb = nullptr;
	}

	++_inputCtr;
	return result;
}

int CScriptHandler::processInput(TTroomScript *roomScript, TTnpcScript *npcScript,
		const TTstring &line) {
	if (!roomScript || !line.isValid())
		return SS_5;

	TTsentence *sentence = new TTsentence(_inputCtr++, line, this, roomScript, npcScript);
	int result = _parser.preprocess(sentence);
	roomScript->scriptPreprocess(sentence);
	npcScript->scriptPreprocess(sentence);

	int canProcess = 0;
	if (result) {
		sentence->setState(result);
		if (roomScript->canRespond(npcScript, sentence, result)) {
			canProcess = npcScript->chooseResponse(roomScript, sentence, result);
		}
	}

	if (canProcess == 0 || canProcess == 1) {
		if (!_parser.findFrames(sentence)) {
			if (roomScript->canProcess(npcScript, sentence) && npcScript) {
				npcScript->process(roomScript, sentence);
			}
		}
	}

	delete sentence;
	return SS_VALID;
}

SimpleFile *CScriptHandler::openResource(const CString &name) {
	return _owner->open(name);
}

void CScriptHandler::setParserConcept(TTconcept *newConcept, TTconcept *oldConcept) {
	_parser.conceptChanged(newConcept, oldConcept);
}

int CScriptHandler::setResponse(TTscriptBase *script, TTresponse *response) {
	return _owner->setResponse(script, response);
}

void CScriptHandler::setActorObject(const TTstring *str) {
	setActor(str);
	setObject(str);
}

void CScriptHandler::setActor(const TTstring *str) {
	if (_conceptActor)
		delete _conceptActor;
	_conceptActor = nullptr;

	if (str) {
		TTword word(*str, WC_UNKNOWN, 0);
		_conceptActor = new TTconcept(&word);
	}
}

void CScriptHandler::setObject(const TTstring *str) {
	if (_conceptObject)
		delete _conceptObject;
	_conceptObject = nullptr;

	if (str) {
		TTword word(*str, WC_UNKNOWN, 0);
		_conceptObject = new TTconcept(&word);
	}
}

} // End of namespace Titanic
