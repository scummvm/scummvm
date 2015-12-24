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

#include "lab/dispman.h"
#include "lab/music.h"
#include "lab/processroom.h"
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

TextFont *Resource::getFont(const Common::String fileName) {
	// TODO: Add support for the font format of the Amiga version
	Common::File *dataFile = openDataFile(fileName, MKTAG('V', 'G', 'A', 'F'));

	uint32 headerSize = 4 + 2 + 256 * 3 + 4;
	uint32 fileSize = dataFile->size();
	if (fileSize <= headerSize)
		return nullptr;

	_vm->updateMusicAndEvents();

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

Common::String Resource::getText(const Common::String fileName) {
	Common::File *dataFile = openDataFile(fileName);

	_vm->updateMusicAndEvents();

	uint32 count = dataFile->size();
	byte *buffer = new byte[count];
	byte *text = buffer;
	dataFile->read(buffer, count);

	while (text && (*text != '\0'))
		*text++ -= (byte)95;

	delete dataFile;

	Common::String str = (char *)buffer;
	delete[] buffer;

	return str;
}

void Resource::readRoomData(const Common::String fileName) {
	Common::File *dataFile = openDataFile(fileName, MKTAG('D', 'O', 'R', '1'));

	_vm->_manyRooms = dataFile->readUint16LE();
	_vm->_highestCondition = dataFile->readUint16LE();
	_vm->_rooms = new RoomData[_vm->_manyRooms + 1];

	for (int i = 1; i <= _vm->_manyRooms; i++) {
		RoomData *curRoom = &_vm->_rooms[i];
		curRoom->_doors[kDirectionNorth] = dataFile->readUint16LE();
		curRoom->_doors[kDirectionSouth] = dataFile->readUint16LE();
		curRoom->_doors[kDirectionEast] = dataFile->readUint16LE();
		curRoom->_doors[kDirectionWest] = dataFile->readUint16LE();
		curRoom->_transitionType = dataFile->readByte();

		curRoom->_view[kDirectionNorth] = nullptr;
		curRoom->_view[kDirectionSouth] = nullptr;
		curRoom->_view[kDirectionEast] = nullptr;
		curRoom->_view[kDirectionWest] = nullptr;
	}

	delete dataFile;
}

InventoryData *Resource::readInventory(const Common::String fileName) {
	Common::File *dataFile = openDataFile(fileName, MKTAG('I', 'N', 'V', '1'));

	_vm->_numInv = dataFile->readUint16LE();
	InventoryData *inventory = new InventoryData[_vm->_numInv + 1];

	for (int i = 1; i <= _vm->_numInv; i++) {
		inventory[i]._quantity = dataFile->readUint16LE();
		inventory[i]._name = readString(dataFile);
		inventory[i]._bitmapName = readString(dataFile);
	}

	delete dataFile;
	return inventory;
}

void Resource::readViews(uint16 roomNum) {
	Common::String fileName = "LAB:Rooms/" + Common::String::format("%d", roomNum);
	Common::File *dataFile = openDataFile(fileName, MKTAG('R', 'O', 'M', '4'));

	freeViews(roomNum);
	RoomData *curRoom = &_vm->_rooms[roomNum];

	curRoom->_roomMsg = readString(dataFile);
	curRoom->_view[kDirectionNorth] = readView(dataFile);
	curRoom->_view[kDirectionSouth] = readView(dataFile);
	curRoom->_view[kDirectionEast] = readView(dataFile);
	curRoom->_view[kDirectionWest] = readView(dataFile);
	readRule(dataFile, curRoom->_rules);

	_vm->updateMusicAndEvents();
	delete dataFile;
}

void Resource::freeViews(uint16 roomNum) {
	if (!_vm->_rooms)
		return;

	for (int i = 0; i < 4; i++)
		freeView(_vm->_rooms[roomNum]._view[i]);
}

Common::String Resource::translateFileName(const Common::String filename) {
	Common::String upperFilename = filename;
	upperFilename.toUppercase();
	Common::String fileNameStrFinal;

	if (upperFilename.hasPrefix("P:") || upperFilename.hasPrefix("F:")) {
		if (_vm->_isHiRes)
			fileNameStrFinal = "GAME/SPICT/";
		else
			fileNameStrFinal = "GAME/PICT/";

		if (_vm->getPlatform() == Common::kPlatformAmiga) {
			if (upperFilename.hasPrefix("P:")) {
				fileNameStrFinal = "PICT/";
			} else {
				fileNameStrFinal = "LABFONTS/";
				upperFilename += "T";	// all the Amiga fonts have a ".FONT" suffix
			}
		}
	} else if (upperFilename.hasPrefix("LAB:")) {
		if (_vm->getPlatform() != Common::kPlatformAmiga)
			fileNameStrFinal = "GAME/";
	} else if (upperFilename.hasPrefix("MUSIC:")) {
		if (_vm->getPlatform() != Common::kPlatformAmiga)
			fileNameStrFinal = "GAME/MUSIC/";
		else
			fileNameStrFinal = "MUSIC/";
	}

	if (upperFilename.contains(':')) {
		while (upperFilename[0] != ':') {
			upperFilename.deleteChar(0);
		}

		upperFilename.deleteChar(0);
	}

	fileNameStrFinal += upperFilename;

	return fileNameStrFinal;
}

Common::File *Resource::openDataFile(const Common::String fileName, uint32 fileHeader) {
	Common::File *dataFile = new Common::File();
	dataFile->open(translateFileName(fileName));
	if (!dataFile->isOpen())
		error("openDataFile: Couldn't open %s (%s)", translateFileName(fileName).c_str(), fileName.c_str());

	if (fileHeader > 0) {
		uint32 headerTag = dataFile->readUint32BE();
		if (headerTag != fileHeader) {
			dataFile->close();
			error("openDataFile: Unexpected header in %s (%s) - expected: %d, got: %d", translateFileName(fileName).c_str(), fileName.c_str(), fileHeader, headerTag);
		}
	}

	return dataFile;
}

Common::String Resource::readString(Common::File *file) {
	byte size = file->readByte();
	if (!size)
		return Common::String("");

	char *str = new char[size];
	for (int i = 0; i < size; i++) {
		char c = file->readByte();
		// Decrypt char
		c = (i < size - 1) ? c - 95 : '\0';
		str[i] = c;
	}

	Common::String result = str;
	delete[] str;
	return result;
}

Common::Array<int16> Resource::readConditions(Common::File *file) {
	int16 cond;
	Common::Array<int16> list;

	while ((cond = file->readUint16LE()) != 0)
		list.push_back(cond);

	if (list.size() > 24) {
		// The original only allocated 24 elements, and silently
		// dropped remaining parts.
		warning("More than 24 parts in condition");
	}

	return list;
}

void Resource::readRule(Common::File *file, RuleList &rules) {
	rules.clear();
	while (file->readByte() == 1) {
		Rule rule;
		rule._ruleType = (RuleType)file->readSint16LE();
		rule._param1 = file->readSint16LE();
		rule._param2 = file->readSint16LE();
		rule._condition = readConditions(file);
		rule._actionList = readAction(file);
		rules.push_back(rule);
	}
}

Common::List<Action> Resource::readAction(Common::File *file) {
	Common::List<Action> list;

	while (file->readByte() == 1) {
		list.push_back(Action());
		Action &action = list.back();

		action._actionType = (ActionType)file->readSint16LE();
		action._param1 = file->readSint16LE();
		action._param2 = file->readSint16LE();
		action._param3 = file->readSint16LE();

		if (action._actionType == kActionShowMessages) {
			action._messages.reserve(action._param1);
			for (int i = 0; i < action._param1; i++)
				action._messages.push_back(readString(file));
		} else {
			action._messages.push_back(readString(file));
		}
	}

	return list;
}

CloseData *Resource::readCloseUps(uint16 depth, Common::File *file) {
	CloseData *closeup = nullptr;
	CloseData *prev = nullptr;
	CloseData *head = nullptr;

	while (file->readByte() != '\0') {
		closeup = new CloseData();
		if (!head)
			head = closeup;
		if (prev)
			prev->_nextCloseUp = closeup;
		closeup->_x1 = file->readUint16LE();
		closeup->_y1 = file->readUint16LE();
		closeup->_x2 = file->readUint16LE();
		closeup->_y2 = file->readUint16LE();
		closeup->_closeUpType = file->readSint16LE();
		closeup->_depth = depth;
		closeup->_graphicName = readString(file);
		closeup->_message = readString(file);
		closeup->_subCloseUps = readCloseUps(depth + 1, file);
		closeup->_nextCloseUp = nullptr;
		prev = closeup;
	}

	return head;
}

void Resource::freeCloseUps(CloseData *closeUps) {
	while (closeUps) {
		CloseData *nextCloseUp = closeUps->_nextCloseUp;
		freeCloseUps(closeUps->_subCloseUps);
		delete closeUps;
		closeUps = nextCloseUp;
	}
}

ViewData *Resource::readView(Common::File *file) {
	ViewData *view = nullptr;
	ViewData *prev = nullptr;
	ViewData *head = nullptr;

	while (file->readByte() == 1) {
		view = new ViewData();
		if (!head)
			head = view;
		if (prev)
			prev->_nextCondition = view;
		view->_condition = readConditions(file);
		view->_graphicName = readString(file);
		view->_closeUps = readCloseUps(0, file);
		view->_nextCondition = nullptr;
		prev = view;
	}

	return head;
}

void Resource::freeView(ViewData *view) {
	while (view) {
		ViewData *nextView = view->_nextCondition;
		freeCloseUps(view->_closeUps);
		delete view;
		view = nextView;
	}
}

} // End of namespace Lab
