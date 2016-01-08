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
	_nextStartNode = nullptr;
	_playerEnabled = false;
	_inputMode = kInputBuildingSentences;
	_startFrameNumber = 0;
	_val1 = _val2 = _val3 = _val4 = _val5 = 0;

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
	_val1 = _val2 = _val3 = 0;
	_val4 = 0;
	_val5 = -1;

	// Initialize speaker arrays
	Common::fill(&_speakerActive[0], &_speakerActive[MAX_SPEAKERS], false);
	Common::fill(&_speakerPortraits[0], &_speakerPortraits[MAX_SPEAKERS], -1);
	Common::fill(&_speakerExists[0], &_speakerExists[MAX_SPEAKERS], 1);
	Common::fill(&_arr4[0], &_arr4[MAX_SPEAKERS], 0x8000);
	Common::fill(&_arr5[0], &_arr5[MAX_SPEAKERS], 0x8000);
	Common::fill(&_arr6[0], &_arr6[MAX_SPEAKERS], 30);

	// Start the conversation
	start();

	// Set variables
	setVariable(2, 0x4F78);
	setVariable(3, 0x4F50);
	setVariable(4, 0x4F52);
	setVariable(5, 0x4F54);
	setVariable(6, 0x4F56);
	setVariable(7, 0x4F58);
	setVariable(8, 0x4F5A);
	setVariable(9, 0x4F5C);
	setVariable(10, 0x4F5E);
	setVariable(11, 0x4F60);
	setVariable(12, 0x4F62);
	setVariable(13, 0x4F64);
	setVariable(14, 0x4F66);
	setVariable(15, 0x4F68);
	setVariable(16, 0x4F6A);
	setVariable(17, 0x4F6C);
	setVariable(18, 0x4F6E);
	setVariable(19, 0x4F70);
	setVariable(20, 0x4F72);
	setVariable(21, 0x4F74);
	setVariable(22, 0x4F76);

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

	warning("TODO: GameConversations::start");
}

void GameConversations::setVariable(uint idx, int v1, int v2) {
	if (active()) {
		_runningConv->_cnd._vars[idx].v1 = v1;
		_runningConv->_cnd._vars[idx].v2 = v2;
	}
}

void GameConversations::stop() {
	warning("TODO GameConversations::stop");
}

void GameConversations::exportPointer(int *val) {
	warning("TODO GameConversations::exportPointer");
}

void GameConversations::exportValue(int val) {
	warning("TODO GameConversations::exportValue");
}

void GameConversations::setHeroTrigger(int val) {
	_vm->_game->_trigger = val;	// HACK
	//_running = -1;	// HACK
	warning("TODO: GameConversations::setHeroTrigger");
}

void GameConversations::setInterlocutorTrigger(int val) {
	warning("TODO: GameConversations::setInterlocutorTrigger");
}

int* GameConversations::getVariable(int idx) {
	warning("TODO: GameConversations::getVariable");
	return nullptr;
}

void GameConversations::hold() {
	warning("TODO: GameConversations::hold");
}

void GameConversations::release() {
	warning("TODO: GameConversations::release");
}

void GameConversations::reset(int id) {
	warning("TODO: GameConversations::reset");
}

void GameConversations::abortConv() {
	warning("TODO: GameConversations::abort");
}

/*------------------------------------------------------------------------*/

void ConversationData::load(const Common::String &filename) {
	Common::File inFile;
	char buffer[16];

	inFile.open(filename);
	MadsPack convFileUnpacked(&inFile);
	Common::SeekableReadStream *convFile = convFileUnpacked.getItemStream(0);

	// **** Section 0: Header *************************************************
	_nodeCount = convFile->readUint16LE();
	_dialogCount = convFile->readUint16LE();
	_messageCount = convFile->readUint16LE();
	_textLineCount = convFile->readUint16LE();
	_unk2 = convFile->readUint16LE();
	_importCount = convFile->readUint16LE();
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

	_convNodes.clear();
	for (uint i = 0; i < _nodeCount; i++) {
		ConvNode node;
		node._index = convFile->readUint16LE();
		node._dialogCount = convFile->readUint16LE();
		node._unk1 = convFile->readSint16LE();	// TODO
		node._unk2 = convFile->readSint16LE();	// TODO
		node._unk3 = convFile->readSint16LE();	// TODO
		_convNodes.push_back(node);
		//debug("Node %d, index %d, entries %d - %d, %d, %d", i, node.index, node.dialogCount, node.unk1, node.unk2, node.unk3);
	}
	delete convFile;

	// **** Section 2: Dialogs ************************************************
	convFile = convFileUnpacked.getItemStream(2);
	assert(convFile->size() == _dialogCount * 8);

	for (uint idx = 0; idx < _nodeCount; ++idx) {
		uint dialogCount = _convNodes[idx]._dialogCount;

		for (uint j = 0; j < dialogCount; ++j) {
			ConvDialog dialog;
			dialog._textLineIndex = convFile->readSint16LE();
			dialog._speechIndex = convFile->readSint16LE();
			dialog._nodeOffset = convFile->readUint16LE();
			dialog._nodeSize = convFile->readUint16LE();
			_convNodes[idx]._dialogs.push_back(dialog);
		}
	}
	delete convFile;

	// **** Section 3: Messages ***********************************************
	convFile = convFileUnpacked.getItemStream(3);
	assert(convFile->size() == _messageCount * 8);

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

	// **** Section 6: Node entry commands ************************************
	convFile = convFileUnpacked.getItemStream(6);
	assert(convFile->size() == _commandsSize);

	for (uint16 i = 0; i < _nodeCount; i++) {
		uint16 dialogCount = _convNodes[i]._dialogCount;

		for (uint16 j = 0; j < dialogCount; j++) {
			//ConvDialog dialog = _convNodes[i].dialogs[j];
			byte command;
			uint16 chk;

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
						/*uint16 nodeRef = */convFile->readUint16LE();
						//debug("Hide node %d", nodeRef);
					}

				}
							  break;
				case cmdUnhide: {
					byte count = convFile->readByte();
					for (byte k = 0; k < count; k++) {
						/*uint16 nodeRef = */convFile->readUint16LE();
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
					/*byte nodeRef = */convFile->readByte();
					//debug("Goto %d", nodeRef);
				}
							  break;
				case cmdAssign: {
					convFile->skip(3);	// unused?
					/*uint16 value = */convFile->readUint16LE();
					/*uint16 variable = */convFile->readUint16LE();
					//debug("Variable %d = %d", variable, value);
				}
								break;
				default:
					error("Unknown conversation command %d", command);
					break;
				}
			} while (command != cmdNodeEnd && command != cmdDialogEnd);
		}
	}

	delete convFile;
	inFile.close();

	// TODO: Still stuff to do
	warning("TODO GameConversations::get");
}

/*------------------------------------------------------------------------*/

void ConversationCnd::load(const Common::String &filename) {
	// TODO
}

} // End of namespace MADS
