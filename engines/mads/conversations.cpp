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

namespace MADS {

struct ConvData {
	uint16 nodes;
	uint16 unk1;
	uint16 messages;
	uint16 unk2;
	uint16 unk3;
	uint16 imports;
	uint16 speakers;
	Common::List<Common::String> portraits;
	Common::String speechFile;
};

#define MAX_SPEAKERS 5

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
	inFile.open(fileName);
	MadsPack convFileUnpacked(&inFile);
	Common::SeekableReadStream *convFile = convFileUnpacked.getItemStream(0);

	char buffer[16];

	ConvData conv;

	// Section 0: Header
	conv.nodes = convFile->readUint16LE();
	conv.unk1 = convFile->readUint16LE();
	conv.messages = convFile->readUint16LE();
	conv.unk2 = convFile->readUint16LE();
	conv.unk3 = convFile->readUint16LE();
	conv.imports = convFile->readUint16LE();
	conv.speakers = convFile->readUint16LE();

	debug("Conv %d has %d nodes, %d messages, %d imports and %d speakers", id, conv.nodes, conv.messages, conv.imports, conv.speakers);

	for (uint16 i = 0; i < MAX_SPEAKERS; i++) {
		convFile->read(buffer, 16);
		Common::String portrait = buffer;
		debug("Speaker %d, portrait %s", i, portrait.c_str());
		conv.portraits.push_back(portrait);
	}

	for (uint16 i = 0; i < MAX_SPEAKERS; i++) {
		convFile->skip(2);
	}

	convFile->read(buffer, 14);
	conv.speechFile = Common::String(buffer);
	debug("Speech file %s", conv.speechFile.c_str());

	convFile->skip(32);	// unknown bytes

	// Section 1
	convFile = convFileUnpacked.getItemStream(1);

	for (uint16 i = 0; i < conv.nodes; i++) {
		uint16 nodeIndex = convFile->readUint16LE();
		debug("Node %d, index %d", i, nodeIndex);
		convFile->skip(2);	// 01 00
		convFile->skip(6);
	}

	// TODO: Read the rest of the sections

	inFile.close();

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
