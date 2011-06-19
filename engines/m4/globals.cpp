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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/textconsole.h"

#include "m4/m4.h"
#include "m4/globals.h"
#include "m4/graphics.h"
#include "m4/gui.h"
#include "m4/viewmgr.h"
#include "m4/script.h"
#include "m4/m4_views.h"
#include "m4/compression.h"

namespace M4 {

Kernel::Kernel(MadsM4Engine *vm) : _vm(vm) {
	daemonTriggerAvailable = true;
	firstFadeColorIndex = 0;
	paused = false;
	betweenRooms = false;
	currentSection = 0;
	newSection = 0;
	previousSection = 0;
	currentRoom = 0;
	newRoom = 0;
	previousRoom = 0;
	trigger = 0;
	triggerMode = KT_DAEMON;

	_globalDaemonFn = NULL;
	_globalParserFn = NULL;

	_sectionInitFn = NULL;
	_sectionDaemonFn = NULL;
	_sectionParserFn = NULL;

	_roomInitFn = NULL;
	_roomDaemonFn = NULL;
	_roomPreParserFn = NULL;
	_roomParserFn = NULL;

}

int32 Kernel::createTrigger(int32 triggerNum) {
	if (triggerNum < 0)
		return triggerNum;
	else
		return triggerNum | (currentRoom << 16) | (triggerMode << 28);
}

bool Kernel::sendTrigger(int32 triggerNum) {
	return handleTrigger(createTrigger(triggerNum));
}

bool Kernel::handleTrigger(int32 triggerNum) {

	debugCN(kDebugScript, "betweenRooms = %d; triggerNum = %08X\n", betweenRooms, (uint)triggerNum);

	if (betweenRooms)
		return true;

	if (triggerNum < 0)
		return false;

	KernelTriggerType saveTriggerMode = triggerMode;
	int32 saveTrigger = trigger;
	bool result = false;

	int room = (triggerNum >> 16) & 0xFFF;

	debugCN(kDebugScript, "room = %d; currentRoom = %d\n", room, currentRoom);

	if (room != currentRoom) {
		debugCN(kDebugScript, "Kernel::handleTrigger() Trigger from another room\n");
		return false;
	}

	trigger = triggerNum & 0xFFFF;
	KernelTriggerType mode = (KernelTriggerType)(triggerNum >> 28);

	switch (mode) {

	case KT_PREPARSE:
		if (trigger < 32000) {
			triggerMode = KT_PREPARSE;
			roomPreParser();
			result = true;
		}
		break;

	case KT_PARSE:
		if (trigger < 32000) {
			triggerMode = KT_PARSE;
			// TODO player.commandReady = TRUE;
			roomParser();
			/* TODO
			if (player.commandReady)
				globalParser();
			*/
			result = true;
		}
		break;

	case KT_DAEMON:
		debugCN(kDebugScript, "KT_DAEMON\n");
		triggerMode = KT_DAEMON;
		daemonTriggerAvailable = false;
		roomDaemon();
		if (daemonTriggerAvailable) {
			daemonTriggerAvailable = false;
			sectionDaemon();
		}
		if (daemonTriggerAvailable) {
			daemonTriggerAvailable = false;
			globalDaemon();
		}

		break;

	default:
		debugCN(kDebugScript, "Kernel::handleTrigger() Unknown trigger mode %d\n", mode);

	}

	triggerMode = saveTriggerMode;
	trigger = saveTrigger;

	return result;
}

void Kernel::loadGlobalScriptFunctions() {
	_globalDaemonFn = _vm->_script->loadFunction("global_daemon");
	_globalParserFn = _vm->_script->loadFunction("global_parser");
}

void Kernel::loadSectionScriptFunctions() {
	Common::String tempFnName;
	tempFnName = Common::String::format("section_init_%d", currentSection);
	_sectionInitFn = _vm->_script->loadFunction(tempFnName);
	tempFnName = Common::String::format("section_daemon_%d", currentSection);
	_sectionDaemonFn = _vm->_script->loadFunction(tempFnName);
	tempFnName = Common::String::format("section_parser_%d", currentSection);
	_sectionParserFn = _vm->_script->loadFunction(tempFnName);
}

void Kernel::loadRoomScriptFunctions() {
	Common::String tempFnName;
	tempFnName = Common::String::format("room_init_%d", currentRoom);
	_roomInitFn = _vm->_script->loadFunction(tempFnName);
	tempFnName = Common::String::format("room_daemon_%d", currentRoom);
	_roomDaemonFn = _vm->_script->loadFunction(tempFnName);
	tempFnName = Common::String::format("room_pre_parser_%d", currentRoom);
	_roomPreParserFn = _vm->_script->loadFunction(tempFnName);
	tempFnName = Common::String::format("room_parser_%d", currentRoom);
	_roomParserFn = _vm->_script->loadFunction(tempFnName);
}

void Kernel::globalDaemon() {
	if (_globalDaemonFn)
		_vm->_script->runFunction(_globalDaemonFn);
	else {
		debugCN(kDebugScript, "Kernel::globalDaemon() _globalDaemonFn is NULL\n");
	}
}

void Kernel::globalParser() {
	if (_globalParserFn)
		_vm->_script->runFunction(_globalParserFn);
	else {
		debugCN(kDebugScript, "Kernel::globalParser() _globalParserFn is NULL\n");
	}
}

void Kernel::sectionInit() {
	if (_sectionInitFn)
		_vm->_script->runFunction(_sectionInitFn);
	else {
		debugCN(kDebugScript, "Kernel::sectionInit() _sectionInitFn is NULL\n");
	}
}

void Kernel::sectionDaemon() {
	if (_sectionDaemonFn)
		_vm->_script->runFunction(_sectionDaemonFn);
	else {
		debugCN(kDebugScript, "Kernel::sectionDaemon() _sectionDaemonFn is NULL\n");
	}
}

void Kernel::sectionParser() {
	if (_sectionParserFn)
		_vm->_script->runFunction(_sectionParserFn);
	else {
		debugCN(kDebugScript, "Kernel::sectionParser() _sectionParserFn is NULL\n");
	}
}

void Kernel::roomInit() {
	if (_roomInitFn)
		_vm->_script->runFunction(_roomInitFn);
	else {
		debugCN(kDebugScript, "Kernel::roomInit() _roomInitFn is NULL\n");
	}
}

void Kernel::roomDaemon() {
	if (_roomDaemonFn)
		_vm->_script->runFunction(_roomDaemonFn);
	else {
		debugCN(kDebugScript, "Kernel::roomDaemon() _roomDaemonFn is NULL\n");
	}
}

void Kernel::roomPreParser() {
	if (_roomPreParserFn)
		_vm->_script->runFunction(_roomPreParserFn);
	else {
		debugCN(kDebugScript, "Kernel::roomPreParser() _roomPreParserFn is NULL\n");
	}
}

void Kernel::roomParser() {
	if (_roomParserFn)
		_vm->_script->runFunction(_roomParserFn);
	else {
		debugCN(kDebugScript, "Kernel::roomParser() _roomParserFn is NULL\n");
	}
}

void Kernel::pauseGame(bool value) {
	paused = value;

	if (paused) pauseEngines();
	else unpauseEngines();
}

void Kernel::pauseEngines() {
	// TODO: A proper implementation of game pausing. At the moment I'm using a hard-coded
	// check in events.cpp on Kernel::paused to prevent any events going to the scene
}

void Kernel::unpauseEngines() {
	// TODO: A proper implementation of game unpausing
}

/*--------------------------------------------------------------------------*/

Globals::Globals(MadsM4Engine *vm): _vm(vm) {
}

bool Globals::isInterfaceVisible() {
	return _m4Vm->scene()->getInterface()->isVisible();
}

/*--------------------------------------------------------------------------*/

MadsGlobals::MadsGlobals(MadsEngine *vm): Globals(vm) {
	_vm = vm;

	playerSpriteChanged = false;
	dialogType = DIALOG_NONE;
	sceneNumber = -1;
	for (int i = 0; i < 3; ++i)
		actionNouns[i] = 0;
	_difficultyLevel = 0;
}

MadsGlobals::~MadsGlobals() {
	uint32 i;

	for (i = 0; i < _madsVocab.size(); i++)
		free(_madsVocab[i]);
	_madsVocab.clear();

	for (i = 0; i < _madsQuotes.size(); i++)
		free(_madsQuotes[i]);
	_madsQuotes.clear();

	_madsMessages.clear();
}

void MadsGlobals::loadMadsVocab() {
	Common::SeekableReadStream *vocabS = _vm->res()->get("vocab.dat");
	int curPos = 0;

	char buffer[30];
	strcpy(buffer, "");

	while (true) {
		uint8 b = vocabS->readByte();
		if (vocabS->eos()) break;

		buffer[curPos++] = b;
		if (buffer[curPos - 1] == '\0') {
			// end of string, add it to the strings list
			_madsVocab.push_back(strdup(buffer));
			curPos = 0;
			strcpy(buffer, "");
		}
	}

	_vm->res()->toss("vocab.dat");
}

void MadsGlobals::loadQuotes() {
	Common::SeekableReadStream *quoteS = _vm->res()->get("quotes.dat");
	int curPos = 0;

	char buffer[128];
	strcpy(buffer, "");

	while (true) {
		uint8 b = quoteS->readByte();
		if (quoteS->eos()) break;

		buffer[curPos++] = b;
		if (buffer[curPos - 1] == '\0') {
			// end of string, add it to the strings list
			_madsQuotes.push_back(strdup(buffer));
			curPos = 0;
			strcpy(buffer, "");
		}
	}

	_vm->res()->toss("quotes.dat");
}

void MadsGlobals::loadMadsMessagesInfo() {
	Common::SeekableReadStream *messageS = _vm->res()->get("messages.dat");

	int16 count = messageS->readUint16LE();
	//debugCN(kDebugScript, "%i messages\n", count);

	for (int i = 0; i < count; i++) {
		MessageItem curMessage;
		curMessage.id = messageS->readUint32LE();
		curMessage.offset = messageS->readUint32LE();
		curMessage.uncompSize = messageS->readUint16LE();

		if (i > 0)
			_madsMessages[i - 1].compSize = curMessage.offset - _madsMessages[i - 1].offset;

		if (i == count - 1)
			curMessage.compSize = messageS->size() - curMessage.offset;

		//debugCN(kDebugScript, "id: %i, offset: %i, uncomp size: %i\n", curMessage->id, curMessage->offset, curMessage->uncompSize);
		_madsMessages.push_back(curMessage);
	}

	_vm->res()->toss("messages.dat");
}

void MadsGlobals::loadMadsObjects() {
	Common::SeekableReadStream *objList = _vm->res()->get("objects.dat");
	int numObjects = objList->readUint16LE();

	for (int i = 0; i < numObjects; ++i)
		_madsObjects.push_back(MadsObjectArray::value_type(new MadsObject(objList)));

	_vm->res()->toss("objects.dat");
}

int MadsGlobals::messageIndexOf(uint32 messageId) {
	for (uint i = 0; i < _madsMessages.size(); ++i)
	{
		if (_madsMessages[i].id == messageId)
			return i;
	}
	return -1;
}

const char *MadsGlobals::loadMessage(uint index) {
	if (index > _madsMessages.size() - 1) {
		warning("Invalid message index: %i", index);
		return NULL;
	}

	FabDecompressor fab;
	byte *compData = new byte[_madsMessages[index].compSize];
	byte *buffer = new byte[_madsMessages[index].uncompSize];

	Common::SeekableReadStream *messageS = _vm->res()->get("messages.dat");
	messageS->seek(_madsMessages[index].offset, SEEK_SET);
	messageS->read(compData, _madsMessages[index].compSize);
	fab.decompress(compData, _madsMessages[index].compSize, buffer, _madsMessages[index].uncompSize);

	for (int i = 0; i < _madsMessages[index].uncompSize - 1; i++)
		if (buffer[i] == '\0') buffer[i] = '\n';

	_vm->res()->toss("messages.dat");
	delete[] compData;

	return (char*)buffer;
}

/**
 * Adds the specified scene number to list of scenes previously visited
 */
void MadsGlobals::addVisitedScene(int newSceneNumber) {
	if (!isSceneVisited(newSceneNumber))
		_visitedScenes.push_back(newSceneNumber);
}

/**
 * Returns true if the specified scene has been previously visited
 */
bool MadsGlobals::isSceneVisited(int checkSceneNumber) {
	Common::List<int>::iterator i;
	for (i = _visitedScenes.begin(); i != _visitedScenes.end(); ++i)
		if (*i == checkSceneNumber)
			return true;
	return false;
}

void MadsGlobals::removeVisitedScene(int oldSceneNumber) {
	_visitedScenes.remove(oldSceneNumber);
}

/*--------------------------------------------------------------------------*/

M4Globals::M4Globals(M4Engine *vm): Globals(vm) {
	_vm = vm;
}

/*--------------------------------------------------------------------------*/

Player::Player(MadsM4Engine *vm) : _vm(vm) {
	commandsAllowed = true;
	needToWalk = false;
	readyToWalk = false;
	waitingForWalk = false;
	commandReady = false;
	strcpy(verb, "");
	strcpy(noun, "");
	strcpy(prep, "");
	strcpy(object, "");
}

void Player::setCommandsAllowed(bool value) {
	setCommandsAllowedFlag = true;
	commandsAllowed = value;
	if (value) {
		// Player commands are enabled again
		_vm->_mouse->lockCursor(CURSOR_ARROW);
		//_m4Vm->scene()->getInterface()->cancelSentence();
	} else {
		// Player commands are disabled, so show hourglass cursor
		_vm->_mouse->lockCursor(CURSOR_HOURGLASS);
	}
}

bool Player::said(const char *word1, const char *word2, const char *word3) {
	const char *words[3];
	words[0] = word1;
	words[1] = word2;
	words[2] = word2;
	for (int i = 0; i < 3; i++) {
		if (words[i])
			if ((scumm_stricmp(noun, words[i])) &&
				(scumm_stricmp(object, words[i])) &&
				(scumm_stricmp(verb, words[i])))
				return false;
	}
	return true;
}

bool Player::saidAny(const char *word1, const char *word2, const char *word3,
	const char *word4, const char *word5, const char *word6, const char *word7,
	const char *word8, const char *word9, const char *word10) {
	const char *words[10];
	words[0] = word1;
	words[1] = word2;
	words[2] = word3;
	words[3] = word4;
	words[4] = word5;
	words[5] = word6;
	words[6] = word7;
	words[7] = word8;
	words[8] = word9;
	words[9] = word10;
	for (int i = 0; i < 10; i++) {
		if (words[i]) {
			if (!scumm_stricmp(noun, words[i]))
				return true;
			if (!scumm_stricmp(object, words[i]))
				return true;
			if (!scumm_stricmp(verb, words[i]))
				return true;
		}
	}
	return false;
}

/*--------------------------------------------------------------------------*/

MadsObject::MadsObject(Common::SeekableReadStream *stream) {
	load(stream);
}

void MadsObject::load(Common::SeekableReadStream *stream) {
	// Get the next data block
	uint8 obj[0x30];
	stream->read(obj, 0x30);

	// Extract object data fields
	_descId = READ_LE_UINT16(&obj[0]);
	_roomNumber = READ_LE_UINT16(&obj[2]);
	_article = (MADSArticles)obj[4];
	_vocabCount = obj[5] & 0x7f;
	// Phantom / Dragon
	if (_vocabCount > 3)
		warning("MadsObject::load(), vocab cound > 3 (it's %d)", _vocabCount);

	for (int i = 0; i < _vocabCount; ++i) {
		_vocabList[i].flags1 = obj[6 + i * 4];
		_vocabList[i].flags2 = obj[7 + i * 4];
		_vocabList[i].vocabId = READ_LE_UINT16(&obj[8 + i * 4]);
	}
}

void MadsObject::setRoom(int roomNumber) {

}

} // End of namespace M4
