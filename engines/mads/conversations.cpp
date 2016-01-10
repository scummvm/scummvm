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

#include "mads/conversations.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "common/file.h"
#include "common/util.h"	// for Common::hexdump

namespace MADS {

GameConversations::GameConversations(MADSEngine *vm) : _vm(vm) {
	_runningConv = nullptr;
	_restoreRunning = 0;
	_playerEnabled = false;
	_inputMode = kInputBuildingSentences;
	_startFrameNumber = 0;
	_speakerVal = 0;
	_currentMode = CONVMODE_NONE;
	_priorMode = CONVMODE_NONE;
	_val1 = 0;
	_verbId = 0;
	_vars = _nextStartNode = nullptr;
	_heroTrigger = 0;
	_heroTriggerMode = SEQUENCE_TRIGGER_PARSER;
	_interlocutorTrigger = 0;
	_interlocutorTriggerMode = SEQUENCE_TRIGGER_PARSER;
	_currentNode = 0;
	_dialogNodeOffset = _dialogNodeSize = 0;

	// Mark all conversation slots as empty
	for (int idx = 0; idx < MAX_CONVERSATIONS; ++idx)
		_conversations[idx]._convId = -1;
}

GameConversations::~GameConversations() {
}

void GameConversations::load(int id) {
	// Scan through the conversation list for a free slot
	int slotIndex = -1;
	for (int idx = 0; idx < MAX_CONVERSATIONS && slotIndex == -1; ++idx) {
		if (_conversations[idx]._convId == -1)
			slotIndex = idx;
	}
	if (slotIndex == -1)
		error("Too many conversations loaded");

	// Set the conversation the slot will contain
	_conversations[slotIndex]._convId = id;

	// Load the conversation data
	Common::String cnvFilename = Common::String::format("CONV%03d.CNV", id);	
	_conversations[slotIndex]._data.load(cnvFilename);

	// Load the conversation's CND data
	Common::String cndFilename = Common::String::format("CONV%03d.CND", id);
	_conversations[slotIndex]._cnd.load(cndFilename);
}

ConversationEntry *GameConversations::getConv(int convId) {
	for (uint idx = 0; idx < MAX_CONVERSATIONS; ++idx) {
		if (_conversations[idx]._convId == convId)
			return &_conversations[idx];
	}

	return nullptr;
}

void GameConversations::run(int id) {
	// If another conversation is running, then stop it first
	if (_runningConv)
		stop();

	// Get the next conversation to run
	_runningConv = getConv(id);
	if (!_runningConv)
		error("Specified conversation %d not loaded", id);

	// Initialize needed fields
	_startFrameNumber = _vm->_events->getFrameCounter();
	_playerEnabled = _vm->_game->_player._stepEnabled;
	_inputMode = _vm->_game->_screenObjects._inputMode;
	_heroTrigger = 0;
	_interlocutorTrigger = 0;
	_val1 = 0;
	_currentMode = CONVMODE_0;
	_verbId = -1;
	_speakerVal = 1;

	// Initialize speaker arrays
	Common::fill(&_speakerActive[0], &_speakerActive[MAX_SPEAKERS], false);
	Common::fill(&_speakerPortraits[0], &_speakerPortraits[MAX_SPEAKERS], -1);
	Common::fill(&_speakerExists[0], &_speakerExists[MAX_SPEAKERS], 1);
	Common::fill(&_arr4[0], &_arr4[MAX_SPEAKERS], 0x8000);
	Common::fill(&_arr5[0], &_arr5[MAX_SPEAKERS], 0x8000);
	Common::fill(&_arr6[0], &_arr6[MAX_SPEAKERS], 30);

	// Start the conversation
	start();

	// Setup variables to point to data in the speaker arrays
	setVariable(2, &_speakerVal);
	for (int idx = 0; idx < MAX_SPEAKERS; ++idx) {
		setVariable(3 + idx, &_speakerExists[idx]);
		setVariable(8 + idx, &_arr4[idx]);
		setVariable(13 + idx, &_arr5[idx]);
		setVariable(18 + idx, &_arr6[idx]);
	}

	// Load sprite data for speaker portraits
	for (uint idx = 0; idx < _runningConv->_data._speakerCount; ++idx) {
		const Common::String &portraitName = _runningConv->_data._portraits[idx];
		_speakerPortraits[idx] = _vm->_game->_scene._sprites.addSprites(portraitName, PALFLAG_RESERVED);

		if (_speakerPortraits[idx] > 0) {
			_speakerActive[idx] = true;
			_speakerExists[idx] = _runningConv->_data._speakerExists[idx];
		}
	}

	// Refresh colors if needed
	if (_vm->_game->_kernelMode == KERNEL_ACTIVE_CODE)
		_vm->_palette->refreshSceneColors();
}

void GameConversations::start() {
	assert(_runningConv->_cnd._vars.size() >= 2);
	_vars = &_runningConv->_cnd._vars[0];
	_nextStartNode = &_runningConv->_cnd._vars[1];

	_runningConv->_cnd._currentNode = -1;
	_runningConv->_cnd._numImports = 0;
	_runningConv->_cnd._vars[0].setValue(_nextStartNode->_val);
}

void GameConversations::setVariable(uint idx, int val) {
	if (active())
		_runningConv->_cnd._vars[idx].setValue(val);
}

void GameConversations::setVariable(uint idx, int *val) {
	if (active())
		_runningConv->_cnd._vars[idx].setValue(val);
}

void GameConversations::setStartNode(uint nodeIndex) {
	assert(_nextStartNode && _nextStartNode->_isPtr == false);
	_nextStartNode->_val = nodeIndex;
}

void GameConversations::stop() {
	// Only need to proceed if there is an active conversation
	if (!active())
		return;

	// Reset player enabled state if needed
	if (_vm->_game->_kernelMode == KERNEL_ACTIVE_CODE)
		_vm->_game->_player._stepEnabled = _playerEnabled;

	// Remove any visible dialog window
	removeActiveWindow();

	// Release any sprites used for character portraits
	for (int idx = 0; idx < _runningConv->_data._speakerCount; ++idx) {
		if (_speakerActive[idx])
			_vm->_game->_scene._sprites.remove(_speakerPortraits[idx]);
	}

	// Flag conversation as no longer running
	_runningConv = nullptr;

	if (_inputMode == kInputConversation)
		_vm->_game->_scene._userInterface.emptyConversationList();

	_vm->_game->_scene._userInterface.setup(_inputMode);
}

void GameConversations::exportPointer(int *ptr) {
	// Only need to proceed if there is an active conversation
	if (!active())
		return;

	// Also don't proceed if the number of allowed imports has already been reached
	if (_runningConv->_cnd._numImports >= _runningConv->_data._maxImports)
		return;

	// Get the variable to use for this next import and set it's value
	int variableIndex = _runningConv->_cnd._importVariables[
		_runningConv->_cnd._numImports++];
	setVariable(variableIndex, ptr);
}

void GameConversations::exportValue(int val) {
	// Only need to proceed if there is an active conversation
	if (!active())
		return;

	// Also don't proceed if the number of allowed imports has already been reached
	if (_runningConv->_cnd._numImports >= _runningConv->_data._maxImports)
		return;

	// Get the variable to use for this next import and set it's value
	int variableIndex = _runningConv->_cnd._importVariables[
		_runningConv->_cnd._numImports++];
	setVariable(variableIndex, val);
}

void GameConversations::setHeroTrigger(int val) {
	_heroTrigger = val;
	_heroTriggerMode = _vm->_game->_triggerSetupMode;
}

void GameConversations::setInterlocutorTrigger(int val) {
	_interlocutorTrigger = val;
	_interlocutorTriggerMode = _vm->_game->_triggerSetupMode;
}

int *GameConversations::getVariable(int idx) {
	assert(idx >= 0);	// TODO: Some negative values are allowed? Investigate
	return _vars[idx].getValue();
}

void GameConversations::hold() {
	if (_currentMode != CONVMODE_NONE) {
		_priorMode = _currentMode;
		_currentMode = CONVMODE_NONE;
	}
}

void GameConversations::release() {
	if (_currentMode == CONVMODE_NONE) {
		_currentMode = _priorMode;
		if (_currentMode == 1 || _currentMode == 2)
			update(true);
	}
}

void GameConversations::flagEntry(DialogCommand mode, int entryIndex) {
	assert(_runningConv);
	uint &flags = _runningConv->_cnd._entryFlags[entryIndex];

	switch (mode) {
	case CMD_1:
		flags |= ENTRYFLAG_4000;
		flags &= ~ENTRYFLAG_8000;
		break;

	case CMD_HIDE:
		flags &= ~ENTRYFLAG_8000;
		break;

	case CMD_UNHIDE:
		if (!(flags & ENTRYFLAG_4000))
			flags |= ENTRYFLAG_8000;
		break;

	default:
		break;
	}
}

void GameConversations::reset(int id) {
	warning("TODO: GameConversations::reset");
}

void GameConversations::update(bool flag) {
	// Only need to proceed if there is an active conversation
	if (!active())
		return;

	ConversationVar &var0 = _runningConv->_cnd._vars[0];

	switch (_currentMode) {
	case CONVMODE_0:
		assert(var0.isNumeric());
		if (var0._val < 0) {
			if (_vm->_game->_scene._frameStartTime >= _startFrameNumber) {
				removeActiveWindow();
				if (_heroTrigger) {
					_vm->_game->_scene._action._activeAction._verbId = _verbId;
					_vm->_game->_trigger = _heroTrigger;
					_vm->_game->_triggerMode = _heroTriggerMode;
					_heroTrigger = 0;
				}

				_currentMode = CONVMODE_STOP;
			}
		} else {
			bool isActive = nextNode();
			_currentNode = var0._val;

			if (isActive) {
				_verbId = _runningConv->_data._nodes[_currentNode]._index;
				_vm->_game->_scene._action._activeAction._verbId = _verbId;
				_vm->_game->_scene._action._inProgress = true;
				_vm->_game->_scene._action._savedFields._commandError = false;
				_currentMode = CONVMODE_1;
			} else {
				_currentMode = generateMenu();
			}
		}
		break;

	case CONVMODE_1:
		if (flag)
			_currentMode = CONVMODE_3;
		break;

	case CONVMODE_2:
		if (flag) {
			_vm->_game->_player._stepEnabled = false;
			_verbId = _vm->_game->_scene._action._activeAction._verbId;

			if (!(_runningConv->_cnd._entryFlags[_verbId] & ENTRYFLAG_2))
				flagEntry(CMD_HIDE, _verbId);

			removeActiveWindow();
			_vm->_game->_scene._userInterface.emptyConversationList();
			_vm->_game->_scene._userInterface.setup(kInputConversation);
			_vm->_events->clearEvents();
			executeEntry(_verbId);

			ConvDialog &dialog = _runningConv->_data._dialogs[_verbId];
			if (dialog._speechIndex) {
				_runningConv->_cnd._field50 = dialog._speechIndex;
				_runningConv->_cnd._field10 = 1;
			}

			generateText(dialog._textLineIndex, _runningConv->_cnd._field10,
				&_runningConv->_cnd._field50);
			_currentMode = CONVMODE_0;

			if (_heroTrigger) {
				_vm->_game->_scene._action._activeAction._verbId = _verbId;
				_vm->_game->_trigger = _heroTrigger;
				_vm->_game->_triggerMode = _heroTriggerMode;
				_heroTrigger = 0;
			}
		}
		break;

	case CONVMODE_3:
		if (_vm->_game->_scene._frameStartTime >= _startFrameNumber) {
			removeActiveWindow();
			_vm->_events->clearEvents();
			executeEntry(_verbId);
			generateMessage(_runningConv->_cnd._fieldC, _runningConv->_cnd._field10,
				&_runningConv->_cnd._field50, &_runningConv->_cnd._field28);
		
			if (_heroTrigger && _val1) {
				_vm->_game->_scene._action._activeAction._verbId = _verbId;
				_vm->_game->_trigger = _heroTrigger;
				_vm->_game->_triggerMode = _heroTriggerMode;
				_heroTrigger = 0;
			}

			_currentMode = CONVMODE_4;
		}
		break;

	case CONVMODE_4:
		if (_vm->_game->_scene._frameStartTime >= _startFrameNumber) {
			removeActiveWindow();
			_vm->_events->clearEvents();

			generateMessage(_runningConv->_cnd._fieldE, _runningConv->_cnd._field12,
				&_runningConv->_cnd._field64, &_runningConv->_cnd._field3C);

			if (_interlocutorTrigger && _val1) {
				_vm->_game->_scene._action._activeAction._verbId = _verbId;
				_vm->_game->_trigger = _interlocutorTrigger;
				_vm->_game->_triggerMode = _interlocutorTriggerMode;
				_interlocutorTrigger = 0;
			}
		}
		break;

	case CONVMODE_STOP:
		stop();
		break;

	default:
		break;
	}

	warning("TODO: GameConversations::update");
}

void GameConversations::removeActiveWindow() {
	warning("TODO: GameConversations::removeActiveWindow");
}

ConversationMode GameConversations::generateMenu() {
	error("TODO: GameConversations::generateMenu");
}

void GameConversations::generateText(int textLineIndex, int v2, int *v3) {
	error("TODO: GameConversations::generateText");
}

void GameConversations::generateMessage(int textLineIndex, int v2, int *v3, int *v4) {
	error("TODO: GameConversations::generateMessage");
}

bool GameConversations::nextNode() {
	ConversationVar &var0 = _runningConv->_cnd._vars[0];
	_runningConv->_cnd._currentNode = var0._val;
	return _runningConv->_data._nodes[var0._val]._active;
}

int GameConversations::executeEntry(int index) {
	ConvDialog &dlg = _runningConv->_data._dialogs[index];
	ConversationVar &var0 = _runningConv->_cnd._vars[0];

	_runningConv->_cnd._fieldC = 0;
	_runningConv->_cnd._fieldE = 0;
	_runningConv->_cnd._field10 = 0;
	_runningConv->_cnd._field12 = 0;
	_nextStartNode->_val = var0._val;

	bool flag = true;
	for (uint scriptIdx = 0; scriptIdx < dlg._script.size(); ++scriptIdx) {
		DialogCommand cmd = dlg._script[scriptIdx]._command;
		// TODO
	}

	if (flag) {
		var0._val = -1;
	}

	return var0._val;
}

/*------------------------------------------------------------------------*/

void ConversationData::load(const Common::String &filename) {
	Common::File inFile;
	char buffer[16];

	inFile.open(filename);
	MadsPack convFileUnpacked(&inFile);

	// **** Section 0: Header *************************************************
	Common::SeekableReadStream *convFile = convFileUnpacked.getItemStream(0);

	_nodeCount = convFile->readUint16LE();
	_dialogCount = convFile->readUint16LE();
	_messageCount = convFile->readUint16LE();
	_textLineCount = convFile->readUint16LE();
	_unk2 = convFile->readUint16LE();
	_maxImports = convFile->readUint16LE();
	_speakerCount = convFile->readUint16LE();

	for (uint idx = 0; idx < MAX_SPEAKERS; ++idx) {
		convFile->read(buffer, 16);
		_portraits[idx] = buffer;
	}

	for (uint idx = 0; idx < MAX_SPEAKERS; ++idx) {
		_speakerExists[idx] = convFile->readUint16LE();
	}

	convFile->read(buffer, 14);
	_speechFile = Common::String(buffer);

	// Total text length in section 5
	_textSize = convFile->readUint32LE();
	_commandsSize = convFile->readUint32LE();

	// The rest of the section 0 is padding to allow room for a set of pointers
	// to the contents of the remaining sections loaded into memory as a
	// continuous data block containing both the header and the sections
	delete convFile;

	// **** Section 1: Nodes **************************************************
	convFile = convFileUnpacked.getItemStream(1);

	_nodes.clear();
	for (uint i = 0; i < _nodeCount; i++) {
		ConvNode node;
		node._index = convFile->readUint16LE();
		node._dialogCount = convFile->readUint16LE();
		node._unk1 = convFile->readSint16LE();	// TODO
		node._active = convFile->readSint16LE() != 0;
		node._unk3 = convFile->readSint16LE();	// TODO
		_nodes.push_back(node);
	}

	delete convFile;

	// **** Section 2: Dialogs ************************************************
	convFile = convFileUnpacked.getItemStream(2);
	assert(convFile->size() == _dialogCount * 8);

	_dialogs.resize(_dialogCount);
	for (uint idx = 0; idx < _dialogCount; ++idx) {
		_dialogs[idx]._textLineIndex = convFile->readSint16LE();
		_dialogs[idx]._speechIndex = convFile->readSint16LE();
		_dialogs[idx]._scriptOffset = convFile->readUint16LE();
		_dialogs[idx]._scriptSize = convFile->readUint16LE();
	}

	delete convFile;

	// **** Section 3: Messages ***********************************************
	convFile = convFileUnpacked.getItemStream(3);
	assert(convFile->size() == _messageCount * 4);

	_messages.resize(_messageCount);
	for (uint idx = 0; idx < _messageCount; ++idx)
		_messages[idx] = convFile->readUint32LE();

	delete convFile;

	// **** Section 4: Text line offsets **************************************
	convFile = convFileUnpacked.getItemStream(4);
	assert(convFile->size() == _textLineCount * 2);

	uint16 *textLineOffsets = new uint16[_textLineCount];	// deleted below in section 5
	for (uint16 i = 0; i < _textLineCount; i++)
		textLineOffsets[i] = convFile->readUint16LE();

	delete convFile;

	// **** Section 5: Text lines *********************************************
	convFile = convFileUnpacked.getItemStream(5);
	assert(convFile->size() == _textSize);

	Common::String textLine;
	_textLines.resize(_textLineCount);
	char textLineBuffer[256];
	uint16 nextOffset;
	for (uint16 i = 0; i < _textLineCount; i++) {
		nextOffset = (i != _textLineCount - 1) ? textLineOffsets[i + 1] : convFile->size();
		convFile->read(textLineBuffer, nextOffset - textLineOffsets[i]);
		_textLines[i] = Common::String(textLineBuffer);
	}

	delete[] textLineOffsets;
	delete convFile;

	// **** Section 6: Scripts ************************************************
	convFile = convFileUnpacked.getItemStream(6);
	assert(convFile->size() == _commandsSize);

	for (uint idx = 0; idx < _dialogs.size(); ++idx) {
		// Move to the correct position for the dialog's script, and create
		// a memory stream to represent the data for just that script
		convFile->seek(_dialogs[idx]._scriptOffset);
		Common::SeekableReadStream *scriptStream = convFile->readStream(_dialogs[idx]._scriptSize);

		// Pass it to the dialog's script set class to parse into commands
		_dialogs[idx]._script.load(*scriptStream, _dialogs[idx]._scriptOffset);
		delete scriptStream;
	}

	delete convFile;
	inFile.close();
}

/*------------------------------------------------------------------------*/

void ConversationConditionals::load(const Common::String &filename) {
	Common::File inFile;
	Common::SeekableReadStream *convFile;

	// Open up the file for access
	inFile.open(filename);
	MadsPack convFileUnpacked(&inFile);

	// **** Section 0: Header *************************************************
	convFile = convFileUnpacked.getItemStream(0);

	_currentNode = convFile->readUint16LE();
	int entryFlagsCount = convFile->readUint16LE();
	int varsCount = convFile->readUint16LE();
	int importsCount = convFile->readUint16LE();

	convFile->skip(2);
	_fieldC = convFile->readUint16LE();
	_fieldE = convFile->readUint16LE();
	_field10 = convFile->readUint16LE();
	_field12 = convFile->readUint16LE();
	convFile->seek(0x28);
	_field28 = convFile->readUint16LE();
	convFile->seek(0x3C);
	_field3C = convFile->readUint16LE();
	convFile->seek(0x50);
	_field50 = convFile->readUint16LE();
	convFile->seek(0x64);
	_field64 = convFile->readUint16LE();

	delete convFile;

	// **** Section: Imports *************************************************
	int streamNum = 1;
	
	_importVariables.resize(importsCount);
	if (importsCount > 0) {
		convFile = convFileUnpacked.getItemStream(streamNum++);

		// Read in the variable indexes that each import value will be stored in
		for (int idx = 0; idx < importsCount; ++idx)
			_importVariables[idx] = convFile->readUint16LE();

		delete convFile;
	}

	// **** Section: Entry Flags *********************************************
	convFile = convFileUnpacked.getItemStream(streamNum++);
	assert(convFile->size() == (entryFlagsCount * 2));

	_entryFlags.resize(entryFlagsCount);
	for (int idx = 0; idx < entryFlagsCount; ++idx)
		_entryFlags[idx] = convFile->readUint16LE();

	delete convFile;

	// **** Section: Variables ***********************************************
	convFile = convFileUnpacked.getItemStream(streamNum);
	assert(convFile->size() == (varsCount * 6));

	_vars.resize(varsCount);
	for (int idx = 0; idx < varsCount; ++idx) {
		convFile->skip(2);	// Loaded values are never pointers, so don't need this
		_vars[idx]._isPtr = false;
		_vars[idx]._val = convFile->readSint16LE();
		convFile->skip(2);	// Unused segment selector for pointer values
	}

	delete convFile;
}

/*------------------------------------------------------------------------*/

void ConversationVar::setValue(int val) {
	_isPtr = false;
	_valPtr = nullptr;
	_val = val;
}

void ConversationVar::setValue(int *val) {
	_isPtr = true;
	_valPtr = val;
	_val = 0;
}

/*------------------------------------------------------------------------*/

void DialogScript::load(Common::SeekableReadStream &s, uint startingOffset) {
	clear();
	Common::HashMap<uint, uint> instructionOffsets;

	// Iterate getting each instruction in turn
	while (s.pos() < s.size()) {
		// Create a new entry for the next script command
		instructionOffsets[startingOffset + s.pos()] = s.size();
		push_back(ScriptEntry());
		ScriptEntry &se = (*this)[size() - 1];
		
		// Load the instruction
		se.load(s);
	}

	// Do a final iteration over the loaded instructions to convert
	// any GOTO instructions from original offsets to instruction indexes
	for (uint idx = 0; idx < size(); ++idx) {
		ScriptEntry &se = (*this)[idx];

		if (se._command == CMD_GOTO)
			se._params[0] = instructionOffsets[se._params[0]];
	}
}

/*------------------------------------------------------------------------*/

void ScriptEntry::load(Common::SeekableReadStream &s) {
	// Get the command byte
	_command = (DialogCommand)s.readByte();

	if (!(_command == CMD_DIALOG_END || (_command >= CMD_NODE_END && _command <= CMD_ASSIGN))) {
		warning("unknown opcode - %d", _command);
		s.seek(0, SEEK_END);
		return;
	}

	// Get in the conditional values
	int numConditionals = 1;
	if (_command == CMD_7)
		numConditionals = 3;
	else if (_command == CMD_ERROR)
		numConditionals = 0;

	for (int idx = 0; idx < numConditionals; ++idx)
		_conditionals[idx].load(s);

	// Get further parameters
	switch (_command) {
	case CMD_1:
	case CMD_HIDE:
	case CMD_UNHIDE: {
		// Read in the list of entries whose flags are to be updated
		int count = s.readByte();
		for (int idx = 0; idx < count; ++idx)
			_params.push_back(s.readSint16LE());
		break;
	}

	case CMD_MESSAGE:
	case CMD_5: {
		int count1 = s.readByte();
		int count2 = s.readByte();
		_params.push_back(count1);
		_params.push_back(count2);

		for (int idx = 0; idx < count1; ++idx)
			_params.push_back(s.readByte());
		for (int idx = 0; idx < count1; ++idx)
			_params.push_back(s.readUint16LE());
		for (int idx = 0; idx < count2; ++idx)
			_params.push_back(s.readUint16LE());
		break;
	}

	case CMD_ERROR:
	case CMD_7:
		// These opcodes have no extra parameters
		break;

	case CMD_GOTO:
	case CMD_ASSIGN:
		// Goto has a single extra parameter for the destination
		// Assign has a single extra parameter for the variable index
		//		that the value resulting from the condition will be set to
		_params.push_back(s.readUint16LE());
		break;

	

	default:
		break;
	}
}

void ScriptEntry::Conditional::load(Common::SeekableReadStream &s) {
	_paramsFlag = s.readUint16LE();

	if (_paramsFlag == 0xff) {
		_param1._isVariable = false;
		_param1._val = 0;
		_param2._isVariable = false;
		_param2._val = 0;
	} else {
		_param1._isVariable = s.readByte() != 0;
		_param1._val = 0;
		_param2._isVariable = s.readByte() != 0;
		_param2._val = 0;
	}
}

/*
do {
command = convFile->readByte();
chk = convFile->readUint16BE();
if (chk != 0xFF00 && chk != 0x0000) {
warning("Error while reading conversation node entries - bailing out");
break;
}

switch (command) {
case cmdNodeEnd:
//debug("Node end");
break;
case cmdDialogEnd:
//debug("Dialog end");
break;
case cmdHide: {
byte count = convFile->readByte();
for (byte k = 0; k < count; k++) {
//uint16 nodeRef = convFile->readUint16LE();
//debug("Hide node %d", nodeRef);
}

}
break;
case cmdUnhide: {
byte count = convFile->readByte();
for (byte k = 0; k < count; k++) {
//uint16 nodeRef = convFile->readUint16LE();
//debug("Unhide node %d", nodeRef);
}

}
break;
case cmdMessage:
//debug("Message");
convFile->skip(7);	// TODO
break;
case cmdGoto: {
convFile->skip(3);	// unused?
//byte nodeRef = convFile->readByte();
//debug("Goto %d", nodeRef);
}
break;
case cmdAssign: {
convFile->skip(3);	// unused?
//uint16 value = convFile->readUint16LE();
//uint16 variable = convFile->readUint16LE();
//debug("Variable %d = %d", variable, value);
}
break;
default:
error("Unknown conversation command %d", command);
break;
}
} while (command != cmdNodeEnd && command != cmdDialogEnd);
*/

} // End of namespace MADS
