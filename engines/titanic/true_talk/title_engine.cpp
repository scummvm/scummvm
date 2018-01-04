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

#include "titanic/true_talk/title_engine.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

CTitleEngine::CTitleEngine() : _script(nullptr), _scriptHandler(nullptr) {
}

CTitleEngine::~CTitleEngine() {
	delete _script;
	delete _scriptHandler;
}

void CTitleEngine::setup(int val1, VocabMode vocabMode) {
	_script = new TTTitleScript();
	_scriptHandler = new CScriptHandler(this, val1, vocabMode);
}

/*------------------------------------------------------------------------*/

STtitleEngine::STtitleEngine(): CTitleEngine(),
		_responseP(nullptr), _stream(nullptr) {
}

STtitleEngine::~STtitleEngine() {
	delete _stream;
}

void STtitleEngine::reset() {
	_indexes.clear();
}

void STtitleEngine::setup(int val1, VocabMode vocabMode) {
	CTitleEngine::setup(val1, TRANSLATE(VOCAB_MODE_EN, VOCAB_MODE_DE));
}

int STtitleEngine::setResponse(TTscriptBase *script, TTresponse *response) {
	_responseP = response;
	_indexes.clear();
	for (TTresponse *respP = response; respP; respP = respP->getNext()) {
		_indexes.push_back(respP->getDialogueId());
	}

	return 0;
}

SimpleFile *STtitleEngine::open(const CString &name) {
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource(
		CString::format("TEXT/%s", name.c_str()));
	assert(stream);

	SimpleFile *file = new SimpleFile();
	file->open(stream);
	return file;
}

} // End of namespace Titanic
