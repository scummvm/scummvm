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

#ifndef TITANIC_SCRIPT_HANDLER_H
#define TITANIC_SCRIPT_HANDLER_H

#include "titanic/true_talk/tt_npc_script.h"
#include "titanic/true_talk/tt_parser.h"
#include "titanic/true_talk/tt_room_script.h"
#include "titanic/true_talk/tt_string.h"
#include "titanic/true_talk/tt_vocab.h"
#include "titanic/support/exe_resources.h"

namespace Titanic {

class CTitleEngine;

class CScriptHandler {
private:
	CTitleEngine *_owner;
	int _inputCtr;
private:
	void handleWord1(const TTstring *str);
	void handleWord2(const TTstring *str);
public:
	TTparser _parser;
	TTvocab *_vocab;
	TTscriptBase *_script;
	TTconcept *_concept1P;
	TTconcept *_concept2P;
	TTconcept *_concept3P;
	TTconcept *_concept4P;
public:
	CScriptHandler(CTitleEngine *owner, int val1, VocabMode vocabMode);
	~CScriptHandler();

	/**
	 * Set the character and room
	 */
	ScriptChangedResult scriptChanged(TTroomScript *roomScript,
		TTnpcScript *npcScript, uint dialogueId);

	int processInput(TTroomScript *roomScript, TTnpcScript *npcScript,
		const TTstring &line);

	/**
	 * Open a resource for access
	 */
	SimpleFile *openResource(const CString &name);

	/**
	 * Called when concept data is copied from one to another
	 */
	void setParserConcept(TTconcept *newConcept, TTconcept *oldConcept);

	/**
	 * Sets a conversation reponse
	 */
	int setResponse(TTscriptBase *script, TTresponse *response);

	void handleWord(const TTstring *str);
};

} // End of namespace Titanic

#endif /* TITANIC_SCRIPT_HANDLER_H */
