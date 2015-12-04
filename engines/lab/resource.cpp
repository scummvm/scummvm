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
#include "lab/text.h"
#include "lab/resource.h"

namespace Lab {

extern RoomData *_rooms;
extern uint16 NumInv, ManyRooms, HighestCondition;

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
	textfont->DataLength = fileSize - headerSize;
	textfont->Height = dataFile->readUint16LE();
	dataFile->read(textfont->Widths, 256);
	for (int i = 0; i < 256; i++)
		textfont->Offsets[i] = dataFile->readUint16LE();
	dataFile->skip(4);
	textfont->data = new byte[textfont->DataLength + 4];
	dataFile->read(textfont->data, textfont->DataLength);
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

	ManyRooms = dataFile->readUint16LE();
	HighestCondition = dataFile->readUint16LE();
	_rooms = (RoomData *)malloc((ManyRooms + 1) * sizeof(RoomData));
	memset(_rooms, 0, (ManyRooms + 1) * sizeof(RoomData));

	for (uint16 i = 1; i <= ManyRooms; i++) {
		_rooms[i]._northDoor = dataFile->readUint16LE();
		_rooms[i]._southDoor = dataFile->readUint16LE();
		_rooms[i]._eastDoor = dataFile->readUint16LE();
		_rooms[i]._westDoor = dataFile->readUint16LE();
		_rooms[i]._wipeType = dataFile->readByte();

		_rooms[i]._view[NORTH] = nullptr;
		_rooms[i]._view[SOUTH] = nullptr;
		_rooms[i]._view[EAST] = nullptr;
		_rooms[i]._view[WEST] = nullptr;
		_rooms[i]._rules = nullptr;
		_rooms[i]._roomMsg = nullptr;
	}

	delete dataFile;
	return true;
}

InventoryData *Resource::readInventory(const char *fileName) {
	Common::File *dataFile = openDataFile(fileName, MKTAG('I', 'N', 'V', '1'));

	NumInv = dataFile->readUint16LE();
	InventoryData *inventory = (InventoryData *)malloc((NumInv + 1) * sizeof(InventoryData));

	for (uint16 i = 1; i <= NumInv; i++) {
		inventory[i].Many = dataFile->readUint16LE();
		inventory[i].name = readString(dataFile);
		inventory[i].BInvName = readString(dataFile);
	}

	delete dataFile;

	return inventory;
}

bool Resource::readViews(uint16 roomNum) {
	Common::String fileName = "LAB:Rooms/" + Common::String::format("%d", roomNum);
	Common::File *dataFile = openDataFile(fileName.c_str(), MKTAG('R', 'O', 'M', '4'));

	_rooms[roomNum]._roomMsg = readString(dataFile);
	_rooms[roomNum]._view[NORTH] = readView(dataFile);
	_rooms[roomNum]._view[SOUTH] = readView(dataFile);
	_rooms[roomNum]._view[EAST] = readView(dataFile);
	_rooms[roomNum]._view[WEST] = readView(dataFile);
	_rooms[roomNum]._rules = readRule(dataFile);

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
		error("openDataFile: Couldn't open %s (%s)", translateFileName(fileName), fileName);

	if (fileHeader > 0) {
		uint32 headerTag = dataFile->readUint32BE();
		if (headerTag != fileHeader) {
			dataFile->close();
			error("openDataFile: Unexpected header in %s (%s) - expected: %d, got: %d", translateFileName(fileName), fileName, fileHeader, headerTag);
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
	RuleList *rules = new Common::List<Rule *>();

	do {
		c = file->readByte();

		if (c == 1) {
			Rule *rule = new Rule();;
			rule->RuleType = file->readSint16LE();
			rule->Param1 = file->readSint16LE();
			rule->Param2 = file->readSint16LE();
			rule->Condition = readConditions(file);
			rule->ActionList = readAction(file);
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
				prev->NextAction = action;
			action->ActionType = file->readSint16LE();
			action->Param1 = file->readSint16LE();
			action->Param2 = file->readSint16LE();
			action->Param3 = file->readSint16LE();

			if (action->ActionType == SHOWMESSAGES) {
				messages = (char **)malloc(action->Param1 * 4);

				for (int i = 0; i < action->Param1; i++)
					messages[i] = readString(file);

				action->Data = (byte *)messages;
			} else {
				action->Data = (byte *)readString(file);
			}

			action->NextAction = NULL;
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
				prev->NextCloseUp = closeup;
			closeup->x1 = file->readUint16LE();
			closeup->y1 = file->readUint16LE();
			closeup->x2 = file->readUint16LE();
			closeup->y2 = file->readUint16LE();
			closeup->CloseUpType = file->readSint16LE();
			closeup->depth = depth;
			closeup->GraphicName = readString(file);
			closeup->Message = readString(file);
			closeup->SubCloseUps = readCloseUps(depth + 1, file);
			closeup->NextCloseUp = NULL;
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
				prev->NextCondition = view;
			view->Condition = readConditions(file);
			view->GraphicName = readString(file);
			view->closeUps = readCloseUps(0, file);
			view->NextCondition = NULL;
			prev = view;
		}
	} while (c == 1);

	return head;
}

} // End of namespace Lab
