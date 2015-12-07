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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/lab.h"
#include "lab/resource.h"

namespace Lab {

Resource::Resource(LabEngine *vm) : _vm(vm) {
	readStaticText();
}

void Resource::readStaticText() {
	Common::File *labTextFile = openDataFile("Lab:Rooms/LabText");

	for (int i = 0; i < 48; i++)
		_staticText[i] = labTextFile->readLine();

	delete labTextFile;
}

TextFont *Resource::getFont(const char *fileName) {
	Common::File *dataFile = openDataFile(fileName, MKTAG('V', 'G', 'A', 'F'));

	uint32 headerSize = 4L + 2L + 256 * 3 + 4L;
	uint32 fileSize = dataFile->size();
	if (fileSize <= headerSize)
		return NULL;

	_vm->_music->updateMusic();

	TextFont *textfont = new TextFont();
	textfont->_dataLength = fileSize - headerSize;
	textfont->_height = dataFile->readUint16LE();
	dataFile->read(textfont->_widths, 256);
	for (int i = 0; i < 256; i++)
		textfont->_offsets[i] = dataFile->readUint16LE();
	dataFile->skip(4);
	textfont->_data = new byte[textfont->_dataLength + 4];
	dataFile->read(textfont->_data, textfont->_dataLength);
	return textfont;
}

char *Resource::getText(const char *fileName) {
	Common::File *dataFile = openDataFile(fileName);

	g_lab->_music->updateMusic();

	uint32 count = dataFile->size();
	byte *buffer = new byte[count];
	byte *text = buffer;
	dataFile->read(buffer, count);

	while (text && *text != '\0')
		*text++ -= (byte)95;

	delete dataFile;
	return (char *)buffer;
}

bool Resource::readRoomData(const char *fileName) {
	Common::File *dataFile = openDataFile(fileName, MKTAG('D', 'O', 'R', '1'));

	_vm->_manyRooms = dataFile->readUint16LE();
	_vm->_highestCondition = dataFile->readUint16LE();
	_vm->_rooms = (RoomData *)malloc((_vm->_manyRooms + 1) * sizeof(RoomData));
	memset(_vm->_rooms, 0, (_vm->_manyRooms + 1) * sizeof(RoomData));

	for (uint16 i = 1; i <= _vm->_manyRooms; i++) {
		_vm->_rooms[i]._northDoor = dataFile->readUint16LE();
		_vm->_rooms[i]._southDoor = dataFile->readUint16LE();
		_vm->_rooms[i]._eastDoor = dataFile->readUint16LE();
		_vm->_rooms[i]._westDoor = dataFile->readUint16LE();
		_vm->_rooms[i]._transitionType = dataFile->readByte();

		_vm->_rooms[i]._view[NORTH] = nullptr;
		_vm->_rooms[i]._view[SOUTH] = nullptr;
		_vm->_rooms[i]._view[EAST] = nullptr;
		_vm->_rooms[i]._view[WEST] = nullptr;
		_vm->_rooms[i]._rules = nullptr;
		_vm->_rooms[i]._roomMsg = nullptr;
	}

	delete dataFile;
	return true;
}

InventoryData *Resource::readInventory(const char *fileName) {
	Common::File *dataFile = openDataFile(fileName, MKTAG('I', 'N', 'V', '1'));

	_vm->_numInv = dataFile->readUint16LE();
	InventoryData *inventory = (InventoryData *)malloc((_vm->_numInv + 1) * sizeof(InventoryData));

	for (uint16 i = 1; i <= _vm->_numInv; i++) {
		inventory[i]._many = dataFile->readUint16LE();
		inventory[i]._name = readString(dataFile);
		inventory[i]._bitmapName = readString(dataFile);
	}

	delete dataFile;

	return inventory;
}

bool Resource::readViews(uint16 roomNum) {
	Common::String fileName = "LAB:Rooms/" + Common::String::format("%d", roomNum);
	Common::File *dataFile = openDataFile(fileName.c_str(), MKTAG('R', 'O', 'M', '4'));

	_vm->_rooms[roomNum]._roomMsg = readString(dataFile);
	_vm->_rooms[roomNum]._view[NORTH] = readView(dataFile);
	_vm->_rooms[roomNum]._view[SOUTH] = readView(dataFile);
	_vm->_rooms[roomNum]._view[EAST] = readView(dataFile);
	_vm->_rooms[roomNum]._view[WEST] = readView(dataFile);
	_vm->_rooms[roomNum]._rules = readRule(dataFile);

	_vm->_music->updateMusic();

	delete dataFile;
	return true;
}

Common::String Resource::translateFileName(Common::String filename) {
	filename.toUppercase();
	Common::String fileNameStrFinal;

	if (filename.hasPrefix("P:")) {
		if (g_lab->_isHiRes)
			fileNameStrFinal = "GAME/SPICT/";
		else
			fileNameStrFinal = "GAME/PICT/";
	}
	else if (filename.hasPrefix("LAB:"))
		fileNameStrFinal = "GAME/";
	else if (filename.hasPrefix("MUSIC:"))
		fileNameStrFinal = "GAME/MUSIC/";

	if (filename.contains(':')) {
		while (filename[0] != ':') {
			filename.deleteChar(0);
		}

		filename.deleteChar(0);
	}

	fileNameStrFinal += filename;

	return fileNameStrFinal;
}

Common::File *Resource::openDataFile(const char *fileName, uint32 fileHeader) {
	Common::File *dataFile = new Common::File();
	dataFile->open(translateFileName(fileName));
	if (!dataFile->isOpen())
		error("openDataFile: Couldn't open %s (%s)", translateFileName(fileName).c_str(), fileName);

	if (fileHeader > 0) {
		uint32 headerTag = dataFile->readUint32BE();
		if (headerTag != fileHeader) {
			dataFile->close();
			error("openDataFile: Unexpected header in %s (%s) - expected: %d, got: %d", translateFileName(fileName).c_str(), fileName, fileHeader, headerTag);
		}
	}

	return dataFile;
}

char *Resource::readString(Common::File *file) {
	byte size = file->readByte();
	if (!size)
		return NULL;
	char *str = (char *)malloc(size);
	char *c = str;
	for (int i = 0; i < size; i++) {
		*c = file->readByte();
		// Decrypt char
		*c++ = (i < size - 1) ? *c - 95 : '\0';
	}

	return str;
}

int16 *Resource::readConditions(Common::File *file) {
	int16 i = 0, cond;
	//int16 *list = new int16[25];
	int16 *list = (int16 *)malloc(25 * 2);
	memset(list, 0, 25 * 2);

	do {
		cond = file->readUint16LE();
		if (i < 25)
			list[i++] = cond;
	} while (cond);

	return list;
}

RuleList *Resource::readRule(Common::File *file) {
	char c;
	RuleList *rules = new RuleList();

	do {
		c = file->readByte();

		if (c == 1) {
			Rule *rule = new Rule();;
			rule->_ruleType = file->readSint16LE();
			rule->_param1 = file->readSint16LE();
			rule->_param2 = file->readSint16LE();
			rule->_condition = readConditions(file);
			rule->_actionList = readAction(file);
			rules->push_back(rule);
		}
	} while (c == 1);

	return rules;
}

Action *Resource::readAction(Common::File *file) {
	char c;
	Action *action = NULL;
	Action *prev = NULL;
	Action *head = NULL;
	char **messages;

	do {
		c = file->readByte();

		if (c == 1) {
			action = (Action *)malloc(sizeof(Action));
			if (!head)
				head = action;
			if (prev)
				prev->_nextAction = action;
			action->_actionType = file->readSint16LE();
			action->_param1 = file->readSint16LE();
			action->_param2 = file->readSint16LE();
			action->_param3 = file->readSint16LE();

			if (action->_actionType == SHOWMESSAGES) {
				messages = (char **)malloc(action->_param1 * 4);

				for (int i = 0; i < action->_param1; i++)
					messages[i] = readString(file);

				action->_data = (byte *)messages;
			} else {
				action->_data = (byte *)readString(file);
			}

			action->_nextAction = NULL;
			prev = action;
		}
	} while (c == 1);

	return head;
}

CloseData *Resource::readCloseUps(uint16 depth, Common::File *file) {
	char c;
	CloseData *closeup = NULL;
	CloseData *prev = NULL;
	CloseData *head = NULL;

	do {
		c = file->readByte();

		if (c != '\0') {
			closeup = (CloseData *)malloc(sizeof(CloseData));
			if (!head)
				head = closeup;
			if (prev)
				prev->_nextCloseUp = closeup;
			closeup->x1 = file->readUint16LE();
			closeup->y1 = file->readUint16LE();
			closeup->x2 = file->readUint16LE();
			closeup->y2 = file->readUint16LE();
			closeup->_closeUpType = file->readSint16LE();
			closeup->_depth = depth;
			closeup->_graphicName = readString(file);
			closeup->_message = readString(file);
			closeup->_subCloseUps = readCloseUps(depth + 1, file);
			closeup->_nextCloseUp = NULL;
			prev = closeup;
		}
	} while (c != '\0');

	return head;
}

ViewData *Resource::readView(Common::File *file) {
	char c;
	ViewData *view = NULL;
	ViewData *prev = NULL;
	ViewData *head = NULL;

	do {
		c = file->readByte();

		if (c == 1) {
			view = (ViewData *)malloc(sizeof(ViewData));
			if (!head)
				head = view;
			if (prev)
				prev->_nextCondition = view;
			view->_condition = readConditions(file);
			view->_graphicName = readString(file);
			view->_closeUps = readCloseUps(0, file);
			view->_nextCondition = NULL;
			prev = view;
		}
	} while (c == 1);

	return head;
}

} // End of namespace Lab
