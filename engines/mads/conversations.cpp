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

#define MAX_SPEAKERS 5

enum DialogCommands {
	cmdNodeEnd = 0,
	//
	cmdHide = 2,
	cmdUnhide = 3,
	cmdMessage = 4,
	//
	//
	cmdGoto = 7,
	//
	cmdAssign = 9,
	cmdDialogEnd = 255
};

struct ConvDialog {
	int16 textLineIndex;	// 0-based
	int16 speechIndex;		// 1-based
	uint16 nodeOffset;		// offset in section 6
	uint16 nodeSize;		// size in section 6
};

struct ConvNode {
	uint16 index;
	uint16 dialogCount;
	int16 unk1;
	int16 unk2;
	int16 unk3;

	Common::Array<ConvDialog> dialogs;
};

struct ConvData {
	uint16 nodeCount;		// conversation nodes, each one containing several dialog options and messages
	uint16 dialogCount;		// messages (non-selectable) + texts (selectable)
	uint16 messageCount;	// messages (non-selectable)
	uint16 textLineCount;
	uint16 unk2;
	uint16 importCount;
	uint16 speakerCount;
	Common::String portraits[MAX_SPEAKERS];
	bool speakerExists[MAX_SPEAKERS];
	Common::String speechFile;
	Common::Array<Common::String> textLines;
	Common::Array<ConvNode> convNodes;
};

GameConversation::GameConversation(MADSEngine *vm)
	: _vm(vm) {
	_running = _restoreRunning = 0;
	_nextStartNode = nullptr;
}

GameConversation::~GameConversation() {
}

void GameConversation::get(int id) {
	Common::File inFile;
	Common::String fileName = Common::String::format("CONV%03d.CNV", id);
	// TODO: Also handle the .CND file

	inFile.open(fileName);
	MadsPack convFileUnpacked(&inFile);
	Common::SeekableReadStream *convFile = convFileUnpacked.getItemStream(0);

	char buffer[16];

	ConvData conv;

	// **** Section 0: Header *************************************************
	conv.nodeCount = convFile->readUint16LE();
	conv.dialogCount = convFile->readUint16LE();
	conv.messageCount = convFile->readUint16LE();
	conv.textLineCount = convFile->readUint16LE();
	conv.unk2 = convFile->readUint16LE();
	conv.importCount = convFile->readUint16LE();
	conv.speakerCount = convFile->readUint16LE();

	//debug("Conv %d has %d nodes, %d dialogs, %d messages, %d text lines, %d unk2, %d imports and %d speakers",
	//		id, conv.nodeCount, conv.dialogCount, conv.messageCount, conv.textLineCount, conv.unk2, conv.importCount, conv.speakerCount);

	for (uint16 i = 0; i < MAX_SPEAKERS; i++) {
		convFile->read(buffer, 16);
		conv.portraits[i] = buffer;
		//debug("Speaker %d, portrait %s", i, conv.portraits[i].c_str());
	}

	for (uint16 i = 0; i < MAX_SPEAKERS; i++) {
		conv.speakerExists[i] = convFile->readUint16LE();
		//debug("Speaker %d exists: %d", i, conv.speakerExists[i]);
	}

	convFile->read(buffer, 14);
	conv.speechFile = Common::String(buffer);
	//debug("Speech file %s", conv.speechFile.c_str());

	uint16 textLength = convFile->readUint16LE();	// Total text length in section 5
	convFile->skip(2);	// TODO: unknown
	uint16 commandLength = convFile->readUint16LE();	// Total length of commands in section 6
	//debug("Node entry commands length: %d", commandLength);

	/*debug("Section 0 unknown bytes");
	byte *tmp0 = new byte[26];
	convFile->read(tmp0, 26);
	Common::hexdump(tmp0, 26);
	delete[] tmp0;*/
	
	// **** Section 1: Nodes **************************************************
	convFile = convFileUnpacked.getItemStream(1);

	for (uint16 i = 0; i < conv.nodeCount; i++) {
		ConvNode node;
		node.index = convFile->readUint16LE();
		node.dialogCount = convFile->readUint16LE();
		node.unk1 = convFile->readSint16LE();	// TODO
		node.unk2 = convFile->readSint16LE();	// TODO
		node.unk3 = convFile->readSint16LE();	// TODO
		conv.convNodes.push_back(node);
		//debug("Node %d, index %d, entries %d - %d, %d, %d", i, node.index, node.dialogCount, node.unk1, node.unk2, node.unk3);
	}

	// **** Section 2: Dialogs ************************************************
	convFile = convFileUnpacked.getItemStream(2);
	assert(convFile->size() == conv.dialogCount * 8);

	for (uint16 i = 0; i < conv.nodeCount; i++) {
		uint16 dialogCount = conv.convNodes[i].dialogCount;

		for (uint16 j = 0; j < dialogCount; j++) {
			ConvDialog dialog;
			dialog.textLineIndex = convFile->readSint16LE();
			dialog.speechIndex = convFile->readSint16LE();
			dialog.nodeOffset = convFile->readUint16LE();
			dialog.nodeSize = convFile->readUint16LE();
			conv.convNodes[i].dialogs.push_back(dialog);
			//debug("Node %d, dialog %d: text line %d, speech index %d, node offset %d, node size %d", j, i, dialog.textLineIndex, dialog.speechIndex, dialog.nodeOffset, dialog.nodeSize);
		}
	}

	// **** Section 3: ???? ***************************************************
	/*debug("Section 3");
	convFile = convFileUnpacked.getItemStream(3);
	byte *tmp1 = new byte[convFile->size()];
	convFile->read(tmp1, convFile->size());
	Common::hexdump(tmp1, convFile->size());
	delete[] tmp1;*/
	// TODO

	// **** Section 4: Text line offsets **************************************
	convFile = convFileUnpacked.getItemStream(4);
	assert(convFile->size() == conv.textLineCount * 2);

	uint16 *textLineOffsets = new uint16[conv.textLineCount];	// deleted below in section 5
	for (uint16 i = 0; i < conv.textLineCount; i++)
		textLineOffsets[i] = convFile->readUint16LE();

	// **** Section 5: Text lines *********************************************
	convFile = convFileUnpacked.getItemStream(5);
	assert(convFile->size() == textLength);

	Common::String textLine;
	conv.textLines.resize(conv.textLineCount);
	char textLineBuffer[256];
	uint16 nextOffset;
	for (uint16 i = 0; i < conv.textLineCount; i++) {
		nextOffset = (i != conv.textLineCount - 1) ? textLineOffsets[i + 1] : convFile->size();
		convFile->read(textLineBuffer, nextOffset - textLineOffsets[i]);
		conv.textLines[i] = Common::String(textLineBuffer);	
		//debug("Text line %d: %s", i, conv.textLines[i].c_str());
	}

	delete[] textLineOffsets;

	// **** Section 6: Node entry commands ************************************
	convFile = convFileUnpacked.getItemStream(6);
	assert(convFile->size() == commandLength);

	for (uint16 i = 0; i < conv.nodeCount; i++) {
		uint16 dialogCount = conv.convNodes[i].dialogCount;

		for (uint16 j = 0; j < dialogCount; j++) {
			//ConvDialog dialog = conv.convNodes[i].dialogs[j];
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

	inFile.close();

	/*
	// DEBUG: Show the very first message, and play the very first speech
	_vm->_audio->setSoundGroup(conv.speechFile);
	uint16 firstText = 0, firstSpeech = 1;

	for (uint16 i = 0; i < conv.convNodes.size(); i++) {
		for (uint16 k = 0; k < conv.convNodes[i].dialogs.size(); k++) {
			if (conv.convNodes[i].dialogs[k].textLineIndex >= 0) {
				firstText = conv.convNodes[i].dialogs[k].textLineIndex;
				firstSpeech = conv.convNodes[i].dialogs[k].speechIndex;
				break;
			}
		}
	}

	_vm->_audio->playSound(firstSpeech - 1);

	TextDialog *dialog = new TextDialog(_vm, FONT_INTERFACE, Common::Point(0, 100), 30);
	dialog->addLine(conv.textLines[firstText]);
	dialog->show();
	delete dialog;
	*/

	warning("TODO GameConversation::get");
}

void GameConversation::run(int id) {
	warning("TODO GameConversation::run");
}

void GameConversation::stop() {
	warning("TODO GameConversation::stop");
}

void GameConversation::exportPointer(int *val) {
	warning("TODO GameConversation::exportPointer");
}

void GameConversation::exportValue(int val) {
	warning("TODO GameConversation::exportValue");
}

void GameConversation::setHeroTrigger(int val) {
	_vm->_game->_trigger = val;	// HACK
	_running = -1;	// HACK
	warning("TODO: GameConversation::setHeroTrigger");
}

void GameConversation::setInterlocutorTrigger(int val) {
	warning("TODO: GameConversation::setInterlocutorTrigger");
}

int* GameConversation::getVariable(int idx) {
	warning("TODO: GameConversation::getVariable");
	return nullptr;
}

void GameConversation::hold() {
	warning("TODO: GameConversation::hold");
}

void GameConversation::release() {
	warning("TODO: GameConversation::release");
}

void GameConversation::reset(int id) {
	warning("TODO: GameConversation::reset");
}

void GameConversation::abortConv() {
	warning("TODO: GameConversation::abort");
}
} // End of namespace MADS
