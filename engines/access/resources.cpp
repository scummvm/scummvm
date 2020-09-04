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

#include "access/resources.h"
#include "access/access.h"
#include "access/amazon/amazon_resources.h"
#include "access/martian/martian_resources.h"
#include "common/translation.h"

namespace Access {

Resources *Resources::init(AccessEngine *vm) {
	if (vm->getGameID() == GType_Amazon)
		return new Amazon::AmazonResources(vm);
	else if (vm->getGameID() == GType_MartianMemorandum)
		return new Martian::MartianResources(vm);

	error("Unknown game");
}

bool Resources::load(Common::U32String &errorMessage) {
	Common::File f;
	Common::String filename = "access.dat";
	if (!f.open(filename.c_str())) {
		errorMessage = Common::U32String::format(_("Unable to locate the '%s' engine data file."), filename.c_str());
		return false;
	}

	// Check for the magic identifier
	char buffer[4];
	f.read(buffer, 4);
	if (strncmp(buffer, "SVMA", 4)) {
		errorMessage = Common::U32String::format(_("The '%s' engine data file is corrupt."), filename.c_str());
		return false;
	}

	// Validate the version number
	uint expectedVersion = 1;
	uint version = f.readUint16LE();
	if (version != expectedVersion) {
		errorMessage = Common::U32String::format(
			_("Incorrect version of the '%s' engine data file found. Expected %d.%d but got %d.%d."),
			filename.c_str(), expectedVersion, 0, version, 0);
		return false;
	}

	// Load in the index
	uint count = f.readUint16LE();
	_datIndex.resize(count);
	for (uint idx = 0; idx < _datIndex.size(); ++idx) {
		_datIndex[idx]._gameId = f.readByte();
		_datIndex[idx]._discType = f.readByte();
		_datIndex[idx]._demoType = f.readByte();

		byte language = f.readByte();
		switch (language) {
		case 0:
			_datIndex[idx]._language = (Common::Language)0;
			break;
		case 5:
			_datIndex[idx]._language = Common::EN_ANY;
			break;
		default:
			error("Unknown language");
			break;
		}

		_datIndex[idx]._fileOffset = f.readUint32LE();
	}

	// Load in the data for the game
	load(f);

	return true;
}

void Resources::load(Common::SeekableReadStream &s) {
	uint count;

	// Get the offset of the data for the game
	uint entryOffset = findEntry(_vm->getGameID(), _vm->isCD() ? 1 : 0,
		_vm->isDemo() ? 1 : 0, _vm->getLanguage());
	s.seek(entryOffset);

	// Load filename list
	count = s.readUint16LE();
	FILENAMES.resize(count);
	for (uint idx = 0; idx < count; ++idx)
		FILENAMES[idx] = readString(s);

	// Load the character data
	count = s.readUint16LE();
	CHARTBL.resize(count);
	for (uint idx = 0; idx < count; ++idx) {
		uint count2 = s.readUint16LE();
		CHARTBL[idx].resize(count2);
		if (count2 > 0)
			s.read(&CHARTBL[idx][0], count2);
	}

	// Load the room data
	count = s.readUint16LE();
	ROOMTBL.resize(count);
	for (uint idx = 0; idx < count; ++idx) {
		ROOMTBL[idx]._desc = readString(s);
		ROOMTBL[idx]._travelPos.x = s.readSint16LE();
		ROOMTBL[idx]._travelPos.y = s.readSint16LE();
		uint count2 = s.readUint16LE();
		ROOMTBL[idx]._data.resize(count2);
		if (count2 > 0)
			s.read(&ROOMTBL[idx]._data[0], count2);
	}

	// Load the deaths list
	count = s.readUint16LE();
	DEATHS.resize(count);
	for (uint idx = 0; idx < count; ++idx) {
		DEATHS[idx]._screenId = s.readByte();
		DEATHS[idx]._msg = readString(s);
	}

	// Load in the inventory list
	count = s.readUint16LE();
	INVENTORY.resize(count);
	for (uint idx = 0; idx < count; ++idx) {
		INVENTORY[idx]._desc = readString(s);
		for (uint idx2 = 0; idx2 < 4; ++idx2)
			INVENTORY[idx]._combo[idx2] = s.readSint16LE();
	}
}

uint Resources::findEntry(byte gameId, byte discType, byte demoType, Common::Language language) {
	for (uint idx = 0; idx < _datIndex.size(); ++idx) {
		DATEntry &de = _datIndex[idx];
		if (de._gameId == gameId && de._discType == discType &&
			de._demoType == demoType && de._language == language)
			return de._fileOffset;
	}

	error("Could not locate appropriate access.dat entry");
}

Common::String Resources::readString(Common::SeekableReadStream &s) {
	Common::String result;
	char c;

	while ((c = s.readByte()) != 0)
		result += c;

	return result;
}

/*------------------------------------------------------------------------*/

const byte INITIAL_PALETTE[18 * 3] = {
	0x00, 0x00, 0x00,
	0xff, 0xff, 0xff,
	0xf0, 0xf0, 0xf0,
	0xe0, 0xe0, 0xe0,
	0xd0, 0xd0, 0xd0,
	0xc0, 0xc0, 0xc0,
	0xb0, 0xb0, 0xb0,
	0xa0, 0xa0, 0xa0,
	0x90, 0x90, 0x90,
	0x80, 0x80, 0x80,
	0x70, 0x70, 0x70,
	0x60, 0x60, 0x60,
	0x50, 0x50, 0x50,
	0x40, 0x40, 0x40,
	0x30, 0x30, 0x30,
	0x20, 0x20, 0x20,
	0x10, 0x10, 0x10,
	0x00, 0x00, 0x00
};

const char *const GENERAL_MESSAGES[] = {
	"LOOKING THERE REVEALS NOTHING OF INTEREST.", // LOOK_MESSAGE
	"THAT DOESN'T OPEN.",               // OPEN_MESSAGE
	"THAT WON'T MOVE.",                 // MOVE_MESSAGE
	"YOU CAN'T TAKE THAT.",             // GET_MESSAGE
	"THAT DOESN'T SEEM TO WORK.",       // USE_MESSAGE
	"YOU CAN'T CLIMB THAT.",            // GO_MESSAGE
	"THERE SEEMS TO BE NO RESPONSE.",   // TALK_MESSAGE
	"THIS OBJECT REQUIRES NO HINTS",    // HELP_MESSAGE
	"THIS OBJECT REQUIRES NO HINTS",    // HELP_MESSAGE
	"THAT DOESN'T SEEM TO WORK."        // USE_MESSAGE
};

const int INVCOORDS[][4] = {
	{ 23, 68, 15, 49 },
	{ 69, 114, 15, 49 },
	{ 115, 160, 15, 49 },
	{ 161, 206, 15, 49 },
	{ 207, 252, 15, 49 },
	{ 253, 298, 15, 49 },
	{ 23, 68, 50, 84 },
	{ 69, 114, 50, 84 },
	{ 115, 160, 50, 84 },
	{ 161, 206, 50, 84 },
	{ 207, 252, 50, 84 },
	{ 253, 298, 50, 84 },
	{ 23, 68, 85, 119 },
	{ 69, 114, 85, 119 },
	{ 115, 160, 85, 119 },
	{ 161, 206, 85, 119 },
	{ 207, 252, 85, 119 },
	{ 253, 298, 85, 119 },
	{ 23, 68, 120, 154 },
	{ 69, 114, 120, 154 },
	{ 115, 160, 120, 154 },
	{ 161, 206, 120, 154 },
	{ 207, 252, 120, 154 },
	{ 253, 298, 120, 154 },
	{ 237, 298, 177, 193 },
	{ 25, 85, 177, 193 }
};

} // End of namespace Access
