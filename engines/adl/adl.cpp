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
#include "common/error.h"
#include "common/file.h"
#include "common/system.h"
#include "common/events.h"
#include "common/stream.h"

#include "engines/util.h"

#include "graphics/palette.h"

#include "adl/adl.h"
#include "adl/display.h"
#include "adl/parser.h"

namespace Adl {

AdlEngine::AdlEngine(OSystem *syst, const AdlGameDescription *gd) :
		Engine(syst),
		_gameDescription(gd),
		_display(nullptr),
		_parser(nullptr),
		_room(1),
		_steps(0),
		_isDark(false) {
}

AdlEngine::~AdlEngine() {
	delete _parser;
	delete _display;
}

Common::Error AdlEngine::run() {
	initGraphics(560, 384, true);

	byte palette[6 * 3] = {
		0x00, 0x00, 0x00,
		0xff, 0xff, 0xff,
		0xc7, 0x34, 0xff,
		0x38, 0xcb, 0x00,
		0x00, 0x00, 0xff, // FIXME
		0xff, 0xa5, 0x00  // FIXME
	};

	g_system->getPaletteManager()->setPalette(palette, 0, 6);

	_display = new Display();
	_parser = new Parser(*this, *_display);

	runGame();

	return Common::kNoError;
}

Common::String AdlEngine::readString(Common::ReadStream &stream, byte until) {
	Common::String str;

	while (1) {
		byte b = stream.readByte();

		if (stream.eos() || stream.err() || b == until)
			break;

		str += b;
	};

	return str;
}

void AdlEngine::printStrings(Common::SeekableReadStream &stream, int count) {
	while (1) {
		Common::String str = readString(stream);
		_display->printString(str);

		if (--count == 0)
			break;

		stream.seek(3, SEEK_CUR);
	};
}

Common::String AdlEngine::getEngineString(int str) {
	return _strings[str];
}

void AdlEngine::wordWrap(Common::String &str) {
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

void AdlEngine::printMessage(uint idx, bool wait) {
	Common::String msg = _messages[idx - 1];
	wordWrap(msg);
	_display->printString(msg);

	if (wait)
		_display->delay(14 * 166018 / 1000);
}

void AdlEngine::printEngineMessage(EngineMessage msg) {
	printMessage(getEngineMessage(msg));
}

void AdlEngine::readCommands(Common::ReadStream &stream, Commands &commands) {
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

void AdlEngine::takeItem(byte noun) {
	Common::Array<Item>::iterator it;

	for (it = _inventory.begin(); it != _inventory.end(); ++it) {
		if (it->noun != noun || it->room != _room)
			continue;

		if (it->state == IDI_ITEM_DOESNT_MOVE) {
			printEngineMessage(IDI_MSG_ITEM_DOESNT_MOVE);
			return;
		}

		if (it->state == IDI_ITEM_MOVED) {
			it->room = IDI_NONE;
			return;
		}

		Common::Array<byte>::const_iterator it2;
		for (it2 = it->roomPictures.begin(); it->roomPictures.end(); ++it2) {
			if (*it2 == _rooms[_room].picture) {
				it->room = IDI_NONE;
				it->state = IDI_ITEM_MOVED;
				return;
			}
		}
	}

	printEngineMessage(IDI_MSG_ITEM_NOT_HERE);
}

void AdlEngine::dropItem(byte noun) {
	Common::Array<Item>::iterator it;

	for (it = _inventory.begin(); it != _inventory.end(); ++it) {
		if (it->noun != noun || it->room != IDI_NONE)
			continue;

		it->room = _room;
		it->state = IDI_ITEM_MOVED;
		return;
	}

	// Don't understand
	printEngineMessage(IDI_MSG_DONT_UNDERSTAND);
}

void AdlEngine::doActions(const Command &command, byte noun, byte offset) {
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
				if (it->room == IDI_NONE)
					printMessage(it->description);

			++offset;
			break;
		}
		case 5:
			_inventory[command.script[offset + 1] - 1].room = command.script[offset + 2];
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
			_display->printString(_strings[IDI_STR_PLAY_AGAIN]);
			Common::String input = _display->inputString();
			if (input.size() == 0 || input[0] != APPLECHAR('N')) {
				warning("Restart game not implemented");
				return;
			}
			// Fall-through
		}
		case 0xd:
			printEngineMessage(IDI_MSG_THANKS_FOR_PLAYING);
			quitGame();
			return;
		case 0x12: {
			byte item = command.script[offset + 1] - 1;
			_inventory[item].room = command.script[offset + 2];
			_inventory[item].position.x = command.script[offset + 3];
			_inventory[item].position.y = command.script[offset + 4];
			offset += 5;
			break;
		}
		case 0x13: {
			byte item = command.script[offset + 2] - 1;
			_inventory[item].picture = command.script[offset + 1];
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
				printEngineMessage(IDI_MSG_CANT_GO_THERE);
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

bool AdlEngine::checkCommand(const Command &command, byte verb, byte noun) {
	if (command.room != IDI_NONE && command.room != _room)
		return false;

	if (command.verb != IDI_NONE && command.verb != verb)
		return false;

	if (command.noun != IDI_NONE && command.noun != noun)
		return false;

	uint offset = 0;
	for (uint i = 0; i < command.numCond; ++i) {
		switch (command.script[offset]) {
		case 3:
			if (_inventory[command.script[offset + 1] - 1].room != command.script[offset + 2])
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
			if (_inventory[command.script[offset + 1] - 1].picture != command.script[offset + 2])
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

bool AdlEngine::doOneCommand(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator it;

	for (it = commands.begin(); it != commands.end(); ++it)
		if (checkCommand(*it, verb, noun))
			return true;

	return false;
}

void AdlEngine::doAllCommands(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator it;

	for (it = commands.begin(); it != commands.end(); ++it)
		checkCommand(*it, verb, noun);
}

void AdlEngine::clearScreen() {
	_display->setMode(Display::kModeMixed);
	_display->clear(0x00);
}

AdlEngine *AdlEngine::create(GameType type, OSystem *syst, const AdlGameDescription *gd) {
	switch(type) {
	case kGameTypeHires1:
		return HiRes1Engine__create(syst, gd);
	default:
		error("Unknown GameType");
	}
}

} // End of namespace Adl
