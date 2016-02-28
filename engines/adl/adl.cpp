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
#include "common/savefile.h"

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
		_isRestarting(false) {
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
	Common::Array<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->noun != noun || item->room != _state.room)
			continue;

		if (item->state == IDI_ITEM_DOESNT_MOVE) {
			printEngineMessage(IDI_MSG_ITEM_DOESNT_MOVE);
			return;
		}

		if (item->state == IDI_ITEM_MOVED) {
			item->room = IDI_NONE;
			return;
		}

		Common::Array<byte>::const_iterator pic;
		for (pic = item->roomPictures.begin(); item->roomPictures.end(); ++pic) {
			if (*pic == _state.rooms[_state.room].curPicture) {
				item->room = IDI_NONE;
				item->state = IDI_ITEM_MOVED;
				return;
			}
		}
	}

	printEngineMessage(IDI_MSG_ITEM_NOT_HERE);
}

void AdlEngine::dropItem(byte noun) {
	Common::Array<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->noun != noun || item->room != IDI_NONE)
			continue;

		item->room = _state.room;
		item->state = IDI_ITEM_MOVED;
		return;
	}

	printEngineMessage(IDI_MSG_DONT_UNDERSTAND);
}

#define ARG(N) (command.script[offset + (N)])

void AdlEngine::doActions(const Command &command, byte noun, byte offset) {
	for (uint i = 0; i < command.numAct; ++i) {
		switch (ARG(0)) {
		case IDO_ACT_VAR_ADD:
			_state.vars[ARG(2)] += ARG(1);
			offset += 3;
			break;
		case IDO_ACT_VAR_SUB:
			_state.vars[ARG(2)] -= ARG(1);
			offset += 3;
			break;
		case IDO_ACT_VAR_SET:
			_state.vars[ARG(1)] = ARG(2);
			offset += 3;
			break;
		case IDO_ACT_LIST_ITEMS: {
			Common::Array<Item>::const_iterator item;

			for (item = _state.items.begin(); item != _state.items.end(); ++item)
				if (item->room == IDI_NONE)
					printMessage(item->description);

			++offset;
			break;
		}
		case IDO_ACT_MOVE_ITEM:
			_state.items[ARG(1) - 1].room = ARG(2);
			offset += 3;
			break;
		case IDO_ACT_SET_ROOM:
			_state.rooms[_state.room].curPicture = _state.rooms[_state.room].picture;
			_state.room = ARG(1);
			offset += 2;
			break;
		case IDO_ACT_SET_CUR_PIC:
			_state.rooms[_state.room].curPicture = ARG(1);
			offset += 2;
			break;
		case IDO_ACT_SET_PIC:
			_state.rooms[_state.room].picture = _state.rooms[_state.room].curPicture = ARG(1);
			offset += 2;
			break;
		case IDO_ACT_PRINT_MSG:
			printMessage(ARG(1));
			offset += 2;
			break;
		case IDO_ACT_SET_LIGHT:
			_state.isDark = false;
			++offset;
			break;
		case IDO_ACT_SET_DARK:
			_state.isDark = true;
			++offset;
			break;
		case IDO_ACT_SAVE:
			saveState(0);
			++offset;
			break;
		case IDO_ACT_LOAD:
			loadState(0);
			++offset;
			// Original engine continues processing here (?)
			break;
		case IDO_ACT_RESTART: {
			_display->printString(_strings[IDI_STR_PLAY_AGAIN]);
			Common::String input = _display->inputString();
			if (input.size() == 0 || input[0] != APPLECHAR('N')) {
				_isRestarting = true;
				_display->clear(0x00);
				_display->decodeFrameBuffer();
				restartGame();
				return;
			}
			// Fall-through
		}
		case IDO_ACT_QUIT:
			printEngineMessage(IDI_MSG_THANKS_FOR_PLAYING);
			quitGame();
			return;
		case IDO_ACT_PLACE_ITEM:
			_state.items[ARG(1) - 1].room = ARG(2);
			_state.items[ARG(1) - 1].position.x = ARG(3);
			_state.items[ARG(1) - 1].position.y = ARG(4);
			offset += 5;
			break;
		case IDO_ACT_SET_ITEM_PIC:
			_state.items[ARG(2) - 1].picture = ARG(1);
			offset += 3;
			break;
		case IDO_ACT_RESET_PIC:
			_state.rooms[_state.room].curPicture = _state.rooms[_state.room].picture;
			++offset;
			break;
		case IDO_ACT_GO_NORTH:
		case IDO_ACT_GO_SOUTH:
		case IDO_ACT_GO_EAST:
		case IDO_ACT_GO_WEST:
		case IDO_ACT_GO_UP:
		case IDO_ACT_GO_DOWN: {
			byte room = _state.rooms[_state.room].connections[ARG(0) - 0x15];

			if (room == 0) {
				printEngineMessage(IDI_MSG_CANT_GO_THERE);
				return;
			}

			_state.rooms[_state.room].curPicture = _state.rooms[_state.room].picture;
			_state.room = room;
			return;
		}
		case IDO_ACT_TAKE_ITEM:
			takeItem(noun);
			++offset;
			break;
		case IDO_ACT_DROP_ITEM:
			dropItem(noun);
			++offset;
			break;
		case IDO_ACT_SET_ROOM_PIC:
			_state.rooms[ARG(1)].picture = _state.rooms[ARG(1)].curPicture = ARG(2);
			offset += 3;
			break;
		default:
			error("Invalid action opcode %02x", ARG(0));
		}
	}
}

bool AdlEngine::checkCommand(const Command &command, byte verb, byte noun) {
	if (command.room != IDI_NONE && command.room != _state.room)
		return false;

	if (command.verb != IDI_NONE && command.verb != verb)
		return false;

	if (command.noun != IDI_NONE && command.noun != noun)
		return false;

	uint offset = 0;
	for (uint i = 0; i < command.numCond; ++i) {
		switch (command.script[offset]) {
		case IDO_CND_ITEM_IN_ROOM:
			if (_state.items[ARG(1) - 1].room != ARG(2))
				return false;
			offset += 3;
			break;
		case IDO_CND_MOVES_GE:
			if (ARG(1) > _state.moves)
				return false;
			offset += 2;
			break;
		case IDO_CND_VAR_EQ:
			if (_state.vars[ARG(1)] != ARG(2))
				return false;
			offset += 3;
			break;
		case IDO_CND_CUR_PIC_EQ:
			if (_state.rooms[_state.room].curPicture != ARG(1))
				return false;
			offset += 2;
			break;
		case IDO_CND_ITEM_PIC_EQ:
			if (_state.items[ARG(1) - 1].picture != ARG(2))
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

#undef ARG

bool AdlEngine::doOneCommand(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator cmd;

	for (cmd = commands.begin(); cmd != commands.end(); ++cmd)
		if (checkCommand(*cmd, verb, noun))
			return true;

	return false;
}

void AdlEngine::doAllCommands(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator cmd;

	for (cmd = commands.begin(); cmd != commands.end(); ++cmd) {
		checkCommand(*cmd, verb, noun);
		if (_isRestarting)
			return;
	}
}

void AdlEngine::clearScreen() {
	_display->setMode(Display::kModeMixed);
	_display->clear(0x00);
}

void AdlEngine::drawItems() {
	Common::Array<Item>::const_iterator item;

	uint dropped = 0;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->room != _state.room)
			continue;

		if (item->state == IDI_ITEM_MOVED) {
			if (_state.rooms[_state.room].picture == _state.rooms[_state.room].curPicture) {
				const Common::Point &p =  _itemOffsets[dropped];
				if (item->isLineArt)
					_display->drawLineArt(_lineArt[item->picture - 1], p);
				else
					drawPic(item->picture, p);
				++dropped;
			}
			continue;
		}

		Common::Array<byte>::const_iterator pic;

		for (pic = item->roomPictures.begin(); pic != item->roomPictures.end(); ++pic) {
			if (*pic == _state.rooms[_state.room].curPicture) {
				if (item->isLineArt)
					_display->drawLineArt(_lineArt[item->picture - 1], item->position);
				else
					drawPic(item->picture, item->position);
				continue;
			}
		}
	}
}

void AdlEngine::showRoom() {
	if (!_state.isDark) {
		drawPic(_state.rooms[_state.room].curPicture);
		drawItems();
	}

	_display->decodeFrameBuffer();
	printMessage(_state.rooms[_state.room].description, false);
}

bool AdlEngine::saveState(uint slot) {
	Common::String fileName = Common::String::format("%s.s%02d", _targetName.c_str(), slot);
	Common::OutSaveFile *outFile = getSaveFileManager()->openForSaving(fileName);

	if (!outFile) {
		warning("Failed to open file '%s'", fileName.c_str());
		return false;
	}

	outFile->writeUint32BE(getTag());
	outFile->writeByte(SAVEGAME_VERSION);

	outFile->writeByte(_state.room);
	outFile->writeByte(_state.moves);
	outFile->writeByte(_state.isDark);

	outFile->writeUint32BE(_state.rooms.size());
	for (uint i = 0; i < _state.rooms.size(); ++i) {
		outFile->writeByte(_state.rooms[i].picture);
		outFile->writeByte(_state.rooms[i].curPicture);
	}

	outFile->writeUint32BE(_state.items.size());
	for (uint i = 0; i < _state.items.size(); ++i) {
		outFile->writeByte(_state.items[i].room);
		outFile->writeByte(_state.items[i].picture);
		outFile->writeByte(_state.items[i].position.x);
		outFile->writeByte(_state.items[i].position.y);
		outFile->writeByte(_state.items[i].state);
	}

	outFile->writeUint32BE(_state.vars.size());
	for (uint i = 0; i < _state.vars.size(); ++i)
		outFile->writeByte(_state.vars[i]);

	outFile->finalize();

	if (outFile->err()) {
		delete outFile;
		warning("Failed to save game '%s'", fileName.c_str());
		return false;
	}

	delete outFile;
	return true;
}

bool AdlEngine::loadState(uint slot) {
	Common::String fileName = Common::String::format("%s.s%02d", _targetName.c_str(), slot);
	Common::InSaveFile *inFile = getSaveFileManager()->openForLoading(fileName);

	if (!inFile) {
		warning("Failed to open file '%s'", fileName.c_str());
		return false;
	}

	if (inFile->readUint32BE() != getTag()) {
		warning("No header found in '%s'", fileName.c_str());
		delete inFile;
		return false;
	}

	byte saveVersion = inFile->readByte();
	if (saveVersion != SAVEGAME_VERSION) {
		warning("Save game version %i not supported", saveVersion);
		delete inFile;
		return false;
	}

	initState();

	_state.room = inFile->readByte();
	_state.moves = inFile->readByte();
	_state.isDark = inFile->readByte();

	uint32 size = inFile->readUint32BE();
	if (size != _state.rooms.size())
		error("Room count mismatch (expected %i; found %i)", _state.rooms.size(), size);

	for (uint i = 0; i < size; ++i) {
		_state.rooms[i].picture = inFile->readByte();
		_state.rooms[i].curPicture = inFile->readByte();
	}

	size = inFile->readUint32BE();
	if (size != _state.items.size())
		error("Item count mismatch (expected %i; found %i)", _state.items.size(), size);

	for (uint i = 0; i < size; ++i) {
		_state.items[i].room = inFile->readByte();
		_state.items[i].picture = inFile->readByte();
		_state.items[i].position.x = inFile->readByte();
		_state.items[i].position.y = inFile->readByte();
		_state.items[i].state = inFile->readByte();
	}

	size = inFile->readUint32BE();
	if (size != _state.vars.size())
		error("Variable count mismatch (expected %i; found %i)", _state.vars.size(), size);

	for (uint i = 0; i < size; ++i)
		_state.vars[i] = inFile->readByte();

	if (inFile->err() || inFile->eos())
		error("Failed to load game '%s'", fileName.c_str());

	delete inFile;
	return true;
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
