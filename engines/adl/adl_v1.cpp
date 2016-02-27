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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/events.h"
#include "common/stream.h"
#include "graphics/palette.h"

#include "engines/util.h"

#include "adl/adl_v1.h"
#include "adl/display.h"
#include "adl/parser.h"

namespace Adl {

static uint exeStrings[STR_MH_TOTAL] = {
	23484, 23375, 23438, 27658, 0x6c31, 27729, 27772, 0x5f1e
};

AdlEngine_v1::AdlEngine_v1(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine(syst, gd) {
	_variables.resize(20);
}

void AdlEngine_v1::runIntro() {
	Common::File file;

	if (!file.open("AUTO LOAD OBJ"))
		error("Failed to open file");

	file.seek(0x1003);
	_display->setMode(Display::kModeHires);
	_display->loadFrameBuffer(file);
	_display->decodeFrameBuffer();
	_display->delay(4000);

	if (shouldQuit())
		return;

	_display->setMode(Display::kModeText);

	Common::File basic;
	if (!basic.open("MYSTERY.HELLO"))
		error("Failed to open file");

	Common::String str;

	basic.seek(93);
	str = readString(basic, '"');
	_display->printASCIIString(str + '\r');

	basic.seek(299);
	str = readString(basic, '"');
	_display->printASCIIString(str + "\r\r");

	basic.seek(365);
	str = readString(basic, '"');
	_display->printASCIIString(str + "\r\r");

	basic.seek(601);
	str = readString(basic, '"');
	_display->printASCIIString(str + '\r');

	_display->inputKey();
	if (g_engine->shouldQuit())
		return;

	_display->setMode(Display::kModeMixed);

	file.seek(15);
	str = readString(file);

	while (1) {
		_display->printString(str);
		Common::String s = _display->inputString();

		if (g_engine->shouldQuit())
			break;

		if (s.empty())
			continue;

		if ((byte)s[0] == ('I' | 0x80))
			break;
		else if ((byte)s[0] == ('G' | 0x80))
			return;
	};

	_display->setMode(Display::kModeText);
	file.seek(102);

	const int pages[] = { 6, 6, 4, 5, 8, 7, 0 };

	int page = 0;
	while (pages[page] != 0) {
		_display->home();
		printStrings(file, pages[page++]);
		_display->inputString();

		if (g_engine->shouldQuit())
			return;

		file.seek(9, SEEK_CUR);
	}
}

void AdlEngine_v1::drawPic(Common::ReadStream &stream, byte xOffset, byte yOffset) {
	byte x, y;
	bool bNewLine = false;
	byte oldX = 0, oldY = 0;
	while (1) {
		x = stream.readByte();
		y = stream.readByte();

		if (stream.err() || stream.eos())
			error("Failed to read picture");

		if (x == 0xff && y == 0xff)
			return;

		if (x == 0 && y == 0) {
			bNewLine = true;
			continue;
		}

		x += xOffset;
		y += yOffset;

		if (y > 160)
			y = 160;

		if (bNewLine) {
			_display->drawPixel(x, y, 0x7f);
			bNewLine = false;
		} else {
			_display->drawLine(Common::Point(oldX, oldY), Common::Point(x, y), 0x7f);
		}

		oldX = x;
		oldY = y;
	}
}

void AdlEngine_v1::drawPic(byte pic, byte xOffset, byte yOffset) {
	Common::File f;
	Common::String name = Common::String::format("BLOCK%i", _pictures[pic].block);

	if (!f.open(name))
		error("Failed to open file");

	f.seek(_pictures[pic].offset);
	drawPic(f, xOffset, yOffset);
}

void AdlEngine_v1::drawItems() {
	Common::Array<Item>::const_iterator it;

	uint dropped = 0;

	for (it = _inventory.begin(); it != _inventory.end(); ++it) {
		if (it->field2 != _room)
			continue;

		if (it->field7 == 1) {
			if (_rooms[_room].field8 == _rooms[_room].picture) {
				const Common::Point &p =  _itemOffsets[dropped];
				if (it->field4)
					_display->drawRightAngles(_drawings[it->field3 - 1], Common::Point(p.x, p.y), 0, 1, 0x7f);
				else
					drawPic(it->field3, p.x, p.y);
				++dropped;
			}
			continue;
		}

		Common::Array<byte>::const_iterator it2;

		for (it2 = it->field10.begin(); it2 != it->field10.end(); ++it2) {
			if (*it2 == _rooms[_room].picture) {
				if (it->field4)
					_display->drawRightAngles(_drawings[it->field3 - 1], Common::Point(it->field5, it->field6), 0, 1, 0x7f);
				else
					drawPic(it->field3, it->field5, it->field6);
				continue;
			}
		}
	}
}

void AdlEngine_v1::showRoom() {
	if (!_isDark) {
		drawPic(_rooms[_room].picture, 0, 0);
		drawItems();
	}

	_display->decodeFrameBuffer();
	printMessage(_rooms[_room].description, false);
}

Common::String AdlEngine_v1::getExeString(uint idx) {
	return _exeStrings[idx];
}

void AdlEngine_v1::wordWrap(Common::String &str) {
	uint end = 39;

	while (1) {
		if (str.size() <= end)
			return;

		while (str[end] != APPLECHAR(' '))
			--end;

		str.setChar(APPLECHAR('\r'), end);
		end += 40;
	}
}

void AdlEngine_v1::printMessage(uint idx, bool wait) {
	// Hardcoded overrides that don't wait after printing
	// Note: strings may differ slightly from the ones in MESSAGES
	switch (idx) {
	case 137:
		_display->printString(_exeStrings[STR_MH_DIRERR]);
		return;
	case 127:
		_display->printString(_exeStrings[STR_MH_DONTHAVEIT]);
		return;
	case 37:
		_display->printString(_exeStrings[STR_MH_DONTUNDERSTAND]);
		return;
	case 7:
		_display->printString(_exeStrings[STR_MH_GETTINGDARK]);
		return;
	}

	Common::String msg = _msgStrings[idx - 1];
	wordWrap(msg);
	_display->printString(msg);

	if (wait)
		_display->delay(14 * 166018 / 1000);
}

void AdlEngine_v1::readCommands(Common::ReadStream &stream, Commands &commands) {
	while (1) {
		Command command;
		command.room = stream.readByte();

		if (command.room == 0xff)
			return;

		command.verb = stream.readByte();
		command.noun = stream.readByte();

		byte scriptSize = stream.readByte() - 6;

		command.numCond = stream.readByte();
		command.numAct = stream.readByte();

		for (uint i = 0; i < scriptSize; ++i)
			command.script.push_back(stream.readByte());

		if (stream.eos() || stream.err())
			error("Failed to read commands");

		commands.push_back(command);
	}
}

void AdlEngine_v1::takeItem(byte noun) {
	Common::Array<Item>::iterator it;

	for (it = _inventory.begin(); it != _inventory.end(); ++it) {
		if (it->field1 != noun || it->field2 != _room)
			continue;

		if (it->field7 == 2) {
			// It doesn't move
			printMessage(151);
			return;
		}

		if (it->field7 == 1) {
			it->field2 = 0xfe;
			it->field7 = 1;
			return;
		}

		Common::Array<byte>::const_iterator it2;
		for (it2 = it->field10.begin(); it->field10.end(); ++it2) {
			if (*it2 == _rooms[_room].picture) {
				it->field2 = 0xfe;
				it->field7 = 1;
				return;
			}
		}
	}

	// Item not here
	printMessage(152);
}

void AdlEngine_v1::dropItem(byte noun) {
	Common::Array<Item>::iterator it;

	for (it = _inventory.begin(); it != _inventory.end(); ++it) {
		if (it->field1 != noun || it->field2 != 0xfe)
			continue;

		it->field2 = _room;
		it->field7 = 1;
		return;
	}

	// Don't understand
	printMessage(37);
}

void AdlEngine_v1::doActions(const Command &command, byte noun, byte offset) {
	for (uint i = 0; i < command.numAct; ++i) {
		switch (command.script[offset]) {
		case 1:
			_variables[command.script[offset + 2]] += command.script[offset + 1];
			offset += 3;
			break;
		case 2:
			_variables[command.script[offset + 2]] -= command.script[offset + 1];
			offset += 3;
			break;
		case 3:
			_variables[command.script[offset + 1]] = command.script[offset + 2];
			offset += 3;
			break;
		case 4: {
			Common::Array<Item>::const_iterator it;

			for (it = _inventory.begin(); it != _inventory.end(); ++it)
				if (it->field2 == 0xfe)
					printMessage(it->field8);

			++offset;
			break;
		}
		case 5:
			_inventory[command.script[offset + 1] - 1].field2 = command.script[offset + 2];
			offset += 3;
			break;
		case 6:
			_rooms[_room].picture = _rooms[_room].field8;
			_room = command.script[offset + 1];
			offset += 2;
			break;
		case 7:
			_rooms[_room].picture = command.script[offset + 1];
			offset += 2;
			break;
		case 8:
			_rooms[_room].field8 = _rooms[_room].picture = command.script[offset + 1];
			offset += 2;
			break;
		case 9:
			printMessage(command.script[offset + 1]);
			offset += 2;
			break;
		case 0xa:
			_isDark = false;
			++offset;
			break;
		case 0xb:
			_isDark = true;
			++offset;
			break;
		case 0xf:
			warning("Save game not implemented");
			++offset;
			break;
		case 0x10:
			warning("Load game not implemented");
			++offset;
			break;
		case 0x11: {
			_display->printString(_exeStrings[STR_MH_PLAYAGAIN]);
			Common::String input = _display->inputString();
			if (input.size() == 0 || input[0] != APPLECHAR('N')) {
				warning("Restart game not implemented");
				return;
			}
			// Fall-through
		}
		case 0xd:
			printMessage(140);
			quitGame();
			return;
		case 0x12: {
			byte item = command.script[offset + 1] - 1;
			_inventory[item].field2 = command.script[offset + 2];
			_inventory[item].field5 = command.script[offset + 3];
			_inventory[item].field6 = command.script[offset + 4];
			offset += 5;
			break;
		}
		case 0x13: {
			byte item = command.script[offset + 2] - 1;
			_inventory[item].field3 = command.script[offset + 1];
			offset += 3;
			break;
		}
		case 0x14:
			_rooms[_room].picture = _rooms[_room].field8;
			++offset;
			break;
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a: {
			byte room = _rooms[_room].connections[command.script[offset] - 0x15];

			if (room == 0) {
				printMessage(137);
				return;
			}

			_rooms[_room].picture = _rooms[_room].field8;
			_room = room;
			return;
		}
		case 0x1b:
			takeItem(noun);
			++offset;
			break;
		case 0x1c:
			dropItem(noun);
			++offset;
			break;
		case 0x1d:
			_rooms[command.script[offset + 1]].field8 = _rooms[command.script[offset + 1]].picture = command.script[offset + 2];
			offset += 3;
			break;
		default:
			error("Invalid action opcode %02x", command.script[offset]);
		}
	}
}

bool AdlEngine_v1::checkCommand(const Command &command, byte verb, byte noun) {
	if (command.room != 0xfe && command.room != _room)
		return false;

	if (command.verb != 0xfe && command.verb != verb)
		return false;

	if (command.noun != 0xfe && command.noun != noun)
		return false;

	uint offset = 0;
	for (uint i = 0; i < command.numCond; ++i) {
		switch (command.script[offset]) {
		case 3:
			if (_inventory[command.script[offset + 1] - 1].field2 != command.script[offset + 2])
				return false;
			offset += 3;
			break;
		case 5:
			if (command.script[offset + 1] > _steps)
				return false;
			offset += 2;
			break;
		case 6:
			if (_variables[command.script[offset + 1]] != command.script[offset + 2])
				return false;
			offset += 3;
			break;
		case 9:
			if (_rooms[_room].picture != command.script[offset + 1])
				return false;
			offset += 2;
			break;
		case 10:
			if (_inventory[command.script[offset + 1] - 1].field3 != command.script[offset + 2])
				return false;
			offset += 3;
			break;
		default:
			error("Invalid condition opcode %02x", command.script[offset]);
		}
	}

	doActions(command, noun, offset);

	return true;
}

bool AdlEngine_v1::doOneCommand(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator it;

	for (it = commands.begin(); it != commands.end(); ++it)
		if (checkCommand(*it, verb, noun))
			return true;

	return false;
}

void AdlEngine_v1::doAllCommands(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator it;

	for (it = commands.begin(); it != commands.end(); ++it)
		checkCommand(*it, verb, noun);
}

void AdlEngine_v1::clearScreen() {
	_display->setMode(Display::kModeMixed);
	_display->clear(0x00);
}

void AdlEngine_v1::runGame() {
	runIntro();
	_display->printASCIIString("\r");

	Common::File f;

	if (!f.open("MESSAGES"))
		error("Failed to open file");

	while (!f.eos() && !f.err())
		_msgStrings.push_back(readString(f, APPLECHAR('\r')) + APPLECHAR('\r'));

	f.close();

	if (!f.open("ADVENTURE"))
		error("Failed to open file");

	// Load strings from executable
	for (uint idx = 0; idx < STR_MH_TOTAL; ++idx) {
		f.seek(exeStrings[idx]);
		_exeStrings.push_back(readString(f));
	}

	// Load room data from executable
	f.seek(1280);
	for (uint i = 0; i < MH_ROOMS; ++i) {
		struct Room room;
		f.readByte();
		room.description = f.readByte();
		for (uint j = 0; j < 6; ++j)
			room.connections[j] = f.readByte();
		room.field8 = f.readByte();
		room.picture = f.readByte();
		_rooms.push_back(room);
	}

	// Load inventory data from executable
	f.seek(0x100);
	while (f.readByte() != 0xff) {
		struct Item item;
		item.field1 = f.readByte();
		item.field2 = f.readByte();
		item.field3 = f.readByte();
		item.field4 = f.readByte();
		item.field5 = f.readByte();
		item.field6 = f.readByte();
		item.field7 = f.readByte();
		item.field8 = f.readByte();

		f.readByte();

		byte size = f.readByte();

		for (uint i = 0; i < size; ++i)
			item.field10.push_back(f.readByte());

		_inventory.push_back(item);
	}

	// Load picture data from executable
	f.seek(0x4b00);
	for (uint i = 0; i < MH_PICS; ++i) {
		struct Picture pic;
		pic.block = f.readByte();
		pic.offset = f.readUint16LE();
		_pictures.push_back(pic);
	}

	// Load commands from executable
	f.seek(0x3D00);
	readCommands(f, _roomCommands);

	f.seek(0x3C00);
	readCommands(f, _globalCommands);

	// Load dropped item offsets
	f.seek(0x68ff);
	for (uint i = 0; i < MH_ITEM_OFFSETS; ++i) {
		Common::Point p;
		p.x = f.readByte();
		p.y = f.readByte();
		_itemOffsets.push_back(p);
	}

	// Load right-angle drawings
	f.seek(0x4f00);
	uint16 drawingsTotal = f.readUint16LE();
	for (uint i = 0; i < drawingsTotal; ++i) {
		f.seek(0x4f00 + 2 + i * 2);
		uint16 offset = f.readUint16LE();
		f.seek(0x4f00 + offset);

		Common::Array<byte> drawing;
		byte b = f.readByte();
		while (b != 0) {
			drawing.push_back(b);
			b = f.readByte();
		}
		_drawings.push_back(drawing);
	}

	// Title screen shown during loading
	f.seek(0x1800);
	_display->loadFrameBuffer(f);
	_display->decodeFrameBuffer();
	_display->delay(2000);

	f.seek(0x3800);
	_parser->loadVerbs(f);

	f.seek(0xf00);
	_parser->loadNouns(f);

	while (1) {
		uint verb = 0, noun = 0;
		clearScreen();
		showRoom();
		_parser->getInput(verb, noun);

		if (!doOneCommand(_roomCommands, verb, noun))
			printMessage(37);
		doAllCommands(_globalCommands, verb, noun);

		_steps++;

		if (shouldQuit())
			return;
	}
}

AdlEngine *AdlEngine_v1__create(OSystem *syst, const AdlGameDescription *gd) {
	return new AdlEngine_v1(syst, gd);
}

} // End of namespace Adl
