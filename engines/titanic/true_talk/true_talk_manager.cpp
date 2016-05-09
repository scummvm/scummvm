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

#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/core/tree_item.h"
#include "titanic/npcs/true_talk_npc.h"
#include "titanic/game_manager.h"

#define MKTAG_BE(a3,a2,a1,a0) ((uint32)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

namespace Titanic {

int CTrueTalkManager::_v1;
int CTrueTalkManager::_v2;
int CTrueTalkManager::_v3;
bool CTrueTalkManager::_v4;
bool CTrueTalkManager::_v5;
int CTrueTalkManager::_v6;
int CTrueTalkManager::_v7;
bool CTrueTalkManager::_v8;
int CTrueTalkManager::_v9;
bool CTrueTalkManager::_v10;
int CTrueTalkManager::_v11[41];
CTrueTalkNPC *CTrueTalkManager::_currentNPC;

/*------------------------------------------------------------------------*/

CTrueTalkManager::CTrueTalkManager(CGameManager *owner) : 
		_gameManager(owner), _scripts(&_titleEngine), _currentCharId(0),
		_dialogueFile(nullptr), _dialogueId(0) {
	_titleEngine.setup(3, 3);
	_currentNPC = nullptr;
}

CTrueTalkManager::~CTrueTalkManager() {
	clear();
}

void CTrueTalkManager::save(SimpleFile *file) const {
	saveStatics(file);

	saveNPC(file, 101);
	saveNPC(file, 103);
	saveNPC(file, 104);
	saveNPC(file, 105);
	saveNPC(file, 111);
	saveNPC(file, 100);
	saveNPC(file, 112);
	saveNPC(file, 107);
	file->writeNumber(0);
}

void CTrueTalkManager::load(SimpleFile *file) {
	loadStatics(file);

	// Iterate through loading characters
	int charId = file->readNumber();
	while (charId) {
		loadNPC(file, charId);

		int ident1 = file->readNumber();
		int ident2 = file->readNumber();

		if (ident1 != MKTAG_BE('U', 'R', 'A', 'H')) {
			while (ident2 != MKTAG_BE('A', 'K', 'E', 'R')) {
				ident1 = ident2;
				ident2 = file->readNumber();

				if (!ident1)
					break;
			}
		}

		// Get start of next character
		charId = file->readNumber();
	}
}

void CTrueTalkManager::loadStatics(SimpleFile *file) {
	int count = file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_v3 = file->readNumber();
	_v4 = file->readNumber() != 0;
	_v5 = file->readNumber() != 0;
	_v6 = file->readNumber();
	_v7 = file->readNumber();
	_v8 = file->readNumber() != 0;
	_v9 = file->readNumber();
	_v10 = file->readNumber() != 0;

	for (int idx = count; count > 10; --idx)
		file->readNumber();

	int count2 = file->readNumber();
	for (int idx = 0; idx < count2; ++idx) {
		int v = file->readNumber();
		if (idx < 41)
			_v11[idx] = v;
	}
}

void CTrueTalkManager::saveStatics(SimpleFile *file) {
	file->writeNumber(10);
	file->writeNumber(_v1);
	file->writeNumber(_v2);
	file->writeNumber(_v3);
	file->writeNumber(_v4 ? 1 : 0);
	file->writeNumber(_v5 ? 1 : 0);
	file->writeNumber(_v6);
	file->writeNumber(_v7);
	file->writeNumber(_v8 ? 1 : 0);
	file->writeNumber(_v9);
	file->writeNumber(_v10 ? 1 : 0);

	file->writeNumber(41);
	for (int idx = 0; idx < 41; ++idx)
		file->writeNumber(_v11[idx]);
}

void CTrueTalkManager::clear() {
	delete _dialogueFile;
	_dialogueFile = nullptr;
	_currentCharId = 0;
}

void CTrueTalkManager::setFlags(int index, int val) {
	switch (index) {
	case 1:
		if (val >= 1 && val <= 3)
			_v3 = val;
		break;

	case 2:
		_v4 = !val;
		break;

	case 3:
		_v5 = val != 0;
		break;

	case 4:
		if (val >= 0 && val <= 3)
			_v6 = val;
		break;

	case 5:
		_v7 = val;
		break;

	case 6:
		_v8 = val != 0;
		break;

	default:
		if (index < 41)
			_v11[index] = val;
		break;
	}
}

void CTrueTalkManager::loadNPC(SimpleFile *file, int charId) {
	TTNpcScript *script = _scripts.getNpcScript(charId);
	if (script)
		script->load(file);
}

void CTrueTalkManager::saveNPC(SimpleFile *file, int charId) const {
	TTNpcScript *script = _scripts.getNpcScript(charId);
	if (script) {
		script->save(file);
		file->writeNumber(MKTAG_BE('U', 'R', 'A', 'H'));
		file->writeNumber(MKTAG_BE('A', 'K', 'E', 'R'));
	}
}

void CTrueTalkManager::preLoad() {
	// Delete any previous talkers
	for (TTTalkerList::iterator i = _talkers.begin(); i != _talkers.end(); ++i)
		delete *i;
	_talkers.clear();
}

void CTrueTalkManager::removeCompleted() {
	for (TTTalkerList::iterator i = _talkers.begin(); i != _talkers.end(); ) {
		TTTalker *talker = *i;
		
		if (talker->_done) {
			i = _talkers.erase(i);
			delete talker;
		} else {
			++i;
		}
	}
}

void CTrueTalkManager::update2() {
	//warning("CTrueTalkManager::update2");
}

void CTrueTalkManager::start(CTrueTalkNPC *npc, uint id, CViewItem *view) {
	TTNpcScript *npcScript = getNpcScript(npc);
	TTRoomScript *roomScript = getRoomScript();
	
	_titleEngine.reset();
	uint charId = npcScript->charId();
	loadAssets(npc, charId);

	_currentNPC = npc;
	_titleEngine._scriptHandler->scriptChanged(roomScript, npcScript, id);
	_currentNPC = nullptr;

	setDialogue(npc, roomScript, view);
}

TTNpcScript *CTrueTalkManager::getTalker(const CString &name) const {
	if (name.contains("Doorbot"))
		return _scripts.getNpcScript(104);
	else if (name.contains("DeskBot"))
		return _scripts.getNpcScript(103);
	else if (name.contains("LiftBot"))
		return _scripts.getNpcScript(105);
	else if (name.contains("Parrot"))
		return _scripts.getNpcScript(107);
	else if (name.contains("BarBot"))
		return _scripts.getNpcScript(100);
	else if (name.contains("ChatterBot"))
		return _scripts.getNpcScript(102);
	else if (name.contains("BellBot"))
		return _scripts.getNpcScript(101);
	else if (name.contains("MaitreD"))
		return _scripts.getNpcScript(112);
	else if (name.contains("Succubus") || name.contains("Sub"))
		return _scripts.getNpcScript(111);

	return nullptr;
}

TTNpcScript *CTrueTalkManager::getNpcScript(CTrueTalkNPC *npc) const {
	CString npcName = npc->getName();
	TTNpcScript *script = getTalker(npcName);

	if (!script) {
		// Fall back on the default NPC script
		script = _scripts.getNpcScript(101);
	}

	return script;
}

TTRoomScript *CTrueTalkManager::getRoomScript() const {
	CRoomItem *room = _gameManager->getRoom();
	TTRoomScript *script = nullptr;
	if (room) {
		int scriptId = room->getScriptId();
		if (scriptId)
			script = _scripts.getRoomScript(scriptId);
	}

	if (!script) {
		// Fall back on the default Room script
		script = _scripts.getRoomScript(110);
	}

	return script;
}

void CTrueTalkManager::loadAssets(CTrueTalkNPC *npc, int charId) {
	// If assets for the character are already loaded, simply exit
	if (_currentCharId == charId)
		return;

	// Clear any previously loaded data
	clear();

	// Signal the NPC to get the asset details
	CTrueTalkGetAssetDetailsMsg detailsMsg;
	detailsMsg.execute(npc);

	if (!detailsMsg._filename.empty()) {
		_dialogueFile = new CDialogueFile(detailsMsg._filename, 20);
		_dialogueId = detailsMsg._numValue + 1;
	}
}

void CTrueTalkManager::processInput(CTrueTalkNPC *npc, CTextInputMsg *msg, CViewItem *view) {
	TTNpcScript *npcScript = getNpcScript(npc);
	TTRoomScript *roomScript = getRoomScript();
	_titleEngine.reset();

	if (npcScript && roomScript) {
		_currentNPC = npc;
		_titleEngine._scriptHandler->processInput(roomScript, npcScript,
			TTString(msg->_input));
		_currentNPC = nullptr;

		loadAssets(npc, npcScript->charId());
		setDialogue(npc, roomScript, view);
	}
	
	_currentNPC = nullptr;
}

void CTrueTalkManager::setDialogue(CTrueTalkNPC *npc, TTRoomScript *roomScript, CViewItem *view) {
	// Get the dialog text
	CString dialogueStr = readDialogueString();
	if (dialogueStr.empty())
		return;

	int soundId = readDialogSound();
	TTTalker *talker = new TTTalker(this, npc);
	_talkers.push_back(talker);

	bool isParrot = npc->getName().contains("parrot");
	triggerNPC(npc);
	setTalker(talker, roomScript, view, isParrot);
	talker->speechStarted(dialogueStr, _titleEngine._indexes[0], soundId);
}

#define STRING_BUFFER_SIZE 2048

CString CTrueTalkManager::readDialogueString() {
	byte buffer[STRING_BUFFER_SIZE];
	CString result;

	for (uint idx = 0; idx < _titleEngine._indexes.size(); ++idx) {
		if (idx != 0)
			result += " ";

		// Open a text entry from the dialogue file for access
		DialogueResource *textRes = _dialogueFile->openTextEntry(
			_titleEngine._indexes[idx] - _dialogueId);
		if (!textRes)
			continue;

		size_t entrySize = textRes->size();
		byte *tempBuffer = (entrySize < STRING_BUFFER_SIZE) ? buffer :
			new byte[entrySize + 1];
		
		_dialogueFile->read(textRes, tempBuffer, entrySize);
		buffer[entrySize] = '\0';

		// Close the resource
		_dialogueFile->closeEntry(textRes);

		// Strip off any non-printable characters
		for (byte *p = buffer; *p != '\0'; ++p) {
			if (*p < 32 || *p > 127)
				*p = ' ';
		}

		// Add string to result
		result += CString((const char *)buffer);

		// Free buffer if one was allocated
		if (entrySize >= STRING_BUFFER_SIZE)
			delete[] tempBuffer;
	}

	return result;
}

int CTrueTalkManager::readDialogSound() {
	_field18 = 0;

	for (uint idx = 0; idx < _titleEngine._indexes.size(); ++idx) {
		CSoundItem *soundItem = _gameManager->_sound.getTrueTalkSound(
			_dialogueFile, _titleEngine._indexes[idx] - _dialogueId);
		if (soundItem) {			
			_field18 = soundItem->fn1();
		}
	}

	return _field18;
}

void CTrueTalkManager::triggerNPC(CTrueTalkNPC *npc) {
	CTrueTalkSelfQueueAnimSetMsg queueSetMsg;
	if (queueSetMsg.execute(npc)) {
		if (_field18 > 300) {
			CTrueTalkQueueUpAnimSetMsg upMsg(_field18);
			upMsg.execute(npc);
		}
	} else {
		CTrueTalkGetAnimSetMsg getAnimMsg;
		if (_field18 > 300) {
			do {
				getAnimMsg.execute(npc);
				if (!getAnimMsg._endFrame)
					break;

				npc->playMovie(getAnimMsg._startFrame, getAnimMsg._endFrame, 0);
				getAnimMsg._endFrame = 0;

				uint numFrames = getAnimMsg._endFrame - getAnimMsg._startFrame;
				int64 val = (numFrames * 1000) * 0x88888889;
				uint diff = (val >> (32 + 5)) - 500;
				_field18 += diff;

				getAnimMsg._index++;
			} while (_field18 > 0);
		}
	}
}

void CTrueTalkManager::setTalker(TTTalker *talker, TTRoomScript *roomScript, CViewItem *view, bool isParrot) {
	warning("TODO: CTrueTalkManager::setTalker");
}

int CTrueTalkManager::getStateVal(int stateNum) {
	if (!_currentNPC)
		return -1000;

	CTrueTalkGetStateValueMsg msg(stateNum, -1000);
	msg.execute(_currentNPC);
	return msg._stateVal;
}

bool CTrueTalkManager::triggerAction(int action, int param) {
	if (!_currentNPC)
		return false;

	CTrueTalkTriggerActionMsg msg(action, param, 0);
	msg.execute(_currentNPC);
	return true;
}

} // End of namespace Titanic
