/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "agi/agi.h"

namespace Agi {

/**
 * Decode logic resource
 * This function decodes messages from the specified raw logic resource
 * into a message list.
 * @param logicNr  The number of the logic resource to decode.
 */
int AgiEngine::decodeLogic(int16 logicNr) {
	AgiLogic &logic = _game.logics[logicNr];
	AgiDir &dirLogic = _game.dirLogic[logicNr];

	// bytecode section:
	// u16  bytecode size
	// u8[] bytecode
	uint16 bytecodeSize = READ_LE_UINT16(logic.data);

	// message section:
	// u8       message count
	// u16      messages size (2 + offsets + strings)
	// u16[]    string offsets (relative to message section + 1)
	// string[] strings (null terminated, possibly encrypted)
	int messageSectionPos = 2 + bytecodeSize;
	uint8 messageCount = logic.data[messageSectionPos];
	uint16 messagesSize = READ_LE_UINT16(logic.data + messageSectionPos + 1);
	int stringOffsetsPos = messageSectionPos + 3;
	int stringsPos = stringOffsetsPos + (2 * messageCount);
	int stringsSize = messagesSize - 2 - (2 * messageCount);

	// decrypt the message strings if the logic was not compressed
	// and the logic has messages.
	if ((~dirLogic.flags & RES_COMPRESSED) && messageCount > 0) {
		decrypt(logic.data + stringsPos, stringsSize);
	}

	// reset logic pointers
	logic.sIP = 2;
	logic.cIP = 2;
	logic.size = messageSectionPos; // exclude messages from logic size

	// allocate list of pointers to message texts. last entry is null.
	logic.numTexts = messageCount;
	logic.texts = (const char **)calloc(1 + logic.numTexts, sizeof(char *));
	if (logic.texts == nullptr) {
		free(logic.data);
		logic.data = nullptr;
		logic.numTexts = 0;
		return errNotEnoughMemory;
	}

	// populate list of pointers to message texts
	for (int i = 0; i < messageCount; i++) {
		int stringOffset = READ_LE_UINT16(logic.data + stringOffsetsPos + (i * 2));
		if (stringOffset != 0) {
			// offset is relative to the message section + 1
			stringOffset += messageSectionPos + 1;
			logic.texts[i] = (const char *)(logic.data + stringOffset);
		} else {
			// TODO: does this happen? when is a string offset zero?
			logic.texts[i] = "";
		}
	}

	// set loaded flag
	dirLogic.flags |= RES_LOADED;
	return errOK;
}

/**
 * Unload logic resource
 * This function unloads the specified logic resource, freeing any
 * memory chunks allocated for this resource.
 * @param logicNr  The number of the logic resource to unload
 */
void AgiEngine::unloadLogic(int16 logicNr) {
	AgiLogic &logic = _game.logics[logicNr];
	AgiDir &dirLogic = _game.dirLogic[logicNr];

	if (dirLogic.flags & RES_LOADED) {
		free(logic.data);
		logic.data = nullptr;
		free(logic.texts);
		logic.texts = nullptr;
		logic.numTexts = 0;
		dirLogic.flags &= ~RES_LOADED;
	}

	logic.sIP = 2;
	logic.cIP = 2;
}

} // End of namespace Agi
