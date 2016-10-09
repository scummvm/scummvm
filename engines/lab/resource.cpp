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

#include "common/file.h"

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

	TextFont *textfont = new TextFont();
	textfont->_dataLength = fileSize - headerSize;
	textfont->_height = dataFile->readUint16LE();
	dataFile->read(textfont->_widths, 256);
	for (int i = 0; i < 256; i++)
		textfont->_offsets[i] = dataFile->readUint16LE();
	dataFile->skip(4);
	textfont->_data = new byte[textfont->_dataLength + 4];
	dataFile->read(textfont->_data, textfont->_dataLength);
	delete dataFile;
	return textfont;
}

Common::String Resource::getText(const Common::String fileName) {
	Common::File *dataFile = openDataFile(fileName);

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

	RoomData *curRoom = &_vm->_rooms[roomNum];

	curRoom->_roomMsg = readString(dataFile);
	readView(dataFile, curRoom->_view[kDirectionNorth]);
	readView(dataFile, curRoom->_view[kDirectionSouth]);
	readView(dataFile, curRoom->_view[kDirectionEast]);
	readView(dataFile, curRoom->_view[kDirectionWest]);
	readRule(dataFile, curRoom->_rules);

	delete dataFile;
}

Common::String Resource::translateFileName(const Common::String filename) {
	Common::String upperFilename;

	// The DOS and Windows version aren't looking for the right file,
	if (!filename.compareToIgnoreCase("P:ZigInt/BLK") && (_vm->getPlatform() != Common::kPlatformAmiga))
		upperFilename = "P:ZigInt/ZIGINT.BLK";
	else
		upperFilename = filename;

	upperFilename.toUppercase();
	Common::String fileNameStrFinal;

	if (upperFilename.hasPrefix("P:") || upperFilename.hasPrefix("F:")) {
		if (_vm->_isHiRes)
			fileNameStrFinal = "SPICT/";
		else
			fileNameStrFinal = "PICT/";

		if (_vm->getPlatform() == Common::kPlatformAmiga) {
			if (upperFilename.hasPrefix("P:")) {
				fileNameStrFinal = "PICT/";
			} else {
				fileNameStrFinal = "LABFONTS/";
				upperFilename += "T";	// all the Amiga fonts have a ".FONT" suffix
			}
		}
	} else if (upperFilename.hasPrefix("LAB:")) {
		// Look inside the game folder
	} else if (upperFilename.hasPrefix("MUSIC:")) {
		fileNameStrFinal = "MUSIC/";
	}

	if (upperFilename.contains(':')) {
		while (upperFilename[0] != ':') {
			upperFilename.deleteChar(0);
		}

		upperFilename.deleteChar(0);
	}

	if (_vm->getPlatform() == Common::kPlatformDOS) {
		// Some script of the DOS version uses names used in the Amiga (and Windows) version,
		// which isn't limited to 8.3 characters. We need to parse upperFilename to detect
		// the filename, and fix it if required so it matches a DOS filename.
		while (upperFilename.contains('/') && upperFilename.size()) {
			fileNameStrFinal += upperFilename[0];
			upperFilename.deleteChar(0);
		}

		for (int i = 0; (i < 8) && upperFilename.size() && (upperFilename[0] != '.'); i++) {
			fileNameStrFinal += upperFilename[0];
			upperFilename.deleteChar(0);
		}

		// Remove the extra character in the filename
		while (upperFilename.size() && (upperFilename[0] != '.'))
			upperFilename.deleteChar(0);

		// copy max 4 characters for the extension ('.foo')
		for (int i = 0; (i < 4) && upperFilename.size(); i++) {
			fileNameStrFinal += upperFilename[0];
			upperFilename.deleteChar(0);
		}

		// Skip the extra characters of the extension
		upperFilename.clear();
	}

	fileNameStrFinal += upperFilename;

	return fileNameStrFinal;
}

Common::File *Resource::openDataFile(const Common::String filename, uint32 fileHeader) {
	Common::File *dataFile = new Common::File();
	dataFile->open(translateFileName(filename));

	if (!dataFile->isOpen()) {
		// The DOS version is known to have some missing files
		if (_vm->getPlatform() == Common::kPlatformDOS) {
			warning("Incomplete DOS version, skipping file %s", filename.c_str());
			return nullptr;
		} else
			error("openDataFile: Couldn't open %s (%s)", translateFileName(filename).c_str(), filename.c_str());
	}
	if (fileHeader > 0) {
		uint32 headerTag = dataFile->readUint32BE();
		if (headerTag != fileHeader) {
			dataFile->close();
			error("openDataFile: Unexpected header in %s (%s) - expected: %d, got: %d", translateFileName(filename).c_str(), filename.c_str(), fileHeader, headerTag);
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
		rules.push_back(Rule());
		Rule &rule = rules.back();

		rule._ruleType = (RuleType)file->readSint16LE();
		rule._param1 = file->readSint16LE();
		rule._param2 = file->readSint16LE();
		rule._condition = readConditions(file);
		readAction(file, rule._actionList);
	}
}

void Resource::readAction(Common::File *file, ActionList &list) {
	list.clear();

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
}

void Resource::readCloseUps(uint16 depth, Common::File *file, CloseDataList &list) {
	list.clear();
	while (file->readByte() != '\0') {
		list.push_back(CloseData());
		CloseData &closeup = list.back();

		closeup._x1 = file->readUint16LE();
		closeup._y1 = file->readUint16LE();
		closeup._x2 = file->readUint16LE();
		closeup._y2 = file->readUint16LE();
		closeup._closeUpType = file->readSint16LE();
		closeup._depth = depth;
		closeup._graphicName = readString(file);
		closeup._message = readString(file);
		readCloseUps(depth + 1, file, closeup._subCloseUps);
	}
}

void Resource::readView(Common::File *file, ViewDataList &list) {
	list.clear();
	while (file->readByte() == 1) {
		list.push_back(ViewData());
		ViewData &view = list.back();

		view._condition = readConditions(file);
		view._graphicName = readString(file);
		readCloseUps(0, file, view._closeUps);
	}
}

} // End of namespace Lab
