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

#include "adl/hires1.h"
#include "adl/display.h"
#include "adl/parser.h"

namespace Adl {

// Messages used outside of scripts
#define IDI_HR1_MSG_CANT_GO_THERE      137
#define IDI_HR1_MSG_DONT_UNDERSTAND     37
#define IDI_HR1_MSG_ITEM_DOESNT_MOVE   151
#define IDI_HR1_MSG_ITEM_NOT_HERE      152
#define IDI_HR1_MSG_THANKS_FOR_PLAYING 140
#define IDI_HR1_MSG_DONT_HAVE_IT       127
#define IDI_HR1_MSG_GETTING_DARK         7

// Strings embedded in the executable
enum {
	IDI_HR1_STR_CANT_GO_THERE = IDI_STR_TOTAL,
	IDI_HR1_STR_DONT_HAVE_IT,
	IDI_HR1_STR_DONT_UNDERSTAND,
	IDI_HR1_STR_GETTING_DARK,

	IDI_HR1_STR_TOTAL
};

// Offsets for strings inside executable
static const StringOffset stringOffsets[] = {
	{ IDI_STR_ENTER_COMMAND,       0x5bbc },
	{ IDI_STR_VERB_ERROR,          0x5b4f },
	{ IDI_STR_NOUN_ERROR,          0x5b8e },
	{ IDI_STR_PLAY_AGAIN,          0x5f1e },
	{ IDI_HR1_STR_CANT_GO_THERE,   0x6c0a },
	{ IDI_HR1_STR_DONT_HAVE_IT,    0x6c31 },
	{ IDI_HR1_STR_DONT_UNDERSTAND, 0x6c51 },
	{ IDI_HR1_STR_GETTING_DARK,    0x6c7c }
};

#define IDI_HR1_OFS_PD_TEXT_0 0x5d
#define IDI_HR1_OFS_PD_TEXT_1 0x12b
#define IDI_HR1_OFS_PD_TEXT_2 0x16d
#define IDI_HR1_OFS_PD_TEXT_3 0x259

#define IDI_HR1_OFS_INTRO_TEXT 0x66
#define IDI_HR1_OFS_GAME_OR_HELP 0xf

#define IDI_HR1_OFS_LOGO_0 0x1003

HiRes1Engine::HiRes1Engine(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine(syst, gd) {
	_variables.resize(20);
}

void HiRes1Engine::runIntro() {
	Common::File file;

	if (!file.open("AUTO LOAD OBJ"))
		error("Failed to open file");

	file.seek(IDI_HR1_OFS_LOGO_0);
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

	basic.seek(IDI_HR1_OFS_PD_TEXT_0);
	str = readString(basic, '"');
	_display->printASCIIString(str + '\r');

	basic.seek(IDI_HR1_OFS_PD_TEXT_1);
	str = readString(basic, '"');
	_display->printASCIIString(str + "\r\r");

	basic.seek(IDI_HR1_OFS_PD_TEXT_2);
	str = readString(basic, '"');
	_display->printASCIIString(str + "\r\r");

	basic.seek(IDI_HR1_OFS_PD_TEXT_3);
	str = readString(basic, '"');
	_display->printASCIIString(str + '\r');

	_display->inputKey();
	if (g_engine->shouldQuit())
		return;

	_display->setMode(Display::kModeMixed);

	file.seek(IDI_HR1_OFS_GAME_OR_HELP);
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
	file.seek(IDI_HR1_OFS_INTRO_TEXT);

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

void HiRes1Engine::drawPic(Common::ReadStream &stream, byte xOffset, byte yOffset) {
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

void HiRes1Engine::drawPic(byte pic, byte xOffset, byte yOffset) {
	Common::File f;
	Common::String name = Common::String::format("BLOCK%i", _pictures[pic].block);

	if (!f.open(name))
		error("Failed to open file");

	f.seek(_pictures[pic].offset);
	drawPic(f, xOffset, yOffset);
}

void HiRes1Engine::drawItems() {
	Common::Array<Item>::const_iterator item;

	uint dropped = 0;

	for (item = _inventory.begin(); item != _inventory.end(); ++item) {
		if (item->room != _room)
			continue;

		if (item->state == IDI_ITEM_MOVED) {
			if (_rooms[_room].picture == _rooms[_room].curPicture) {
				const Common::Point &p =  _itemOffsets[dropped];
				if (item->isDrawing)
					_display->drawRightAngles(_drawings[item->picture - 1], Common::Point(p.x, p.y), 0, 1, 0x7f);
				else
					drawPic(item->picture, p.x, p.y);
				++dropped;
			}
			continue;
		}

		Common::Array<byte>::const_iterator pic;

		for (pic = item->roomPictures.begin(); pic != item->roomPictures.end(); ++pic) {
			if (*pic == _rooms[_room].curPicture) {
				if (item->isDrawing)
					_display->drawRightAngles(_drawings[item->picture - 1], item->position, 0, 1, 0x7f);
				else
					drawPic(item->picture, item->position.x, item->position.y);
				continue;
			}
		}
	}
}

void HiRes1Engine::showRoom() {
	if (!_isDark) {
		drawPic(_rooms[_room].curPicture, 0, 0);
		drawItems();
	}

	_display->decodeFrameBuffer();
	printMessage(_rooms[_room].description, false);
}

void HiRes1Engine::runGame() {
	runIntro();
	_display->printASCIIString("\r");

	Common::File f;

	if (!f.open("MESSAGES"))
		error("Failed to open file");

	while (!f.eos() && !f.err())
		_messages.push_back(readString(f, APPLECHAR('\r')) + APPLECHAR('\r'));

	f.close();

	if (!f.open("ADVENTURE"))
		error("Failed to open file");

	// Load strings from executable
	_strings.resize(IDI_HR1_STR_TOTAL);
	for (uint idx = 0; idx < IDI_HR1_STR_TOTAL; ++idx) {
		f.seek(stringOffsets[idx].offset);
		_strings[stringOffsets[idx].stringIdx] = readString(f);
	}

	// Load room data from executable
	f.seek(1280);
	for (uint i = 0; i < MH_ROOMS; ++i) {
		struct Room room;
		f.readByte();
		room.description = f.readByte();
		for (uint j = 0; j < 6; ++j)
			room.connections[j] = f.readByte();
		room.picture = f.readByte();
		room.curPicture = f.readByte();
		_rooms.push_back(room);
	}

	// Load inventory data from executable
	f.seek(0x100);
	while (f.readByte() != 0xff) {
		struct Item item;
		item.noun = f.readByte();
		item.room = f.readByte();
		item.picture = f.readByte();
		item.isDrawing = f.readByte();
		item.position.x = f.readByte();
		item.position.y = f.readByte();
		item.state = f.readByte();
		item.description = f.readByte();

		f.readByte();

		byte size = f.readByte();

		for (uint i = 0; i < size; ++i)
			item.roomPictures.push_back(f.readByte());

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

void HiRes1Engine::printMessage(uint idx, bool wait) {
	// Hardcoded overrides that don't wait after printing
	// Note: strings may differ slightly from the ones in MESSAGES
	switch (idx) {
	case IDI_HR1_MSG_CANT_GO_THERE:
		_display->printString(_strings[IDI_HR1_STR_CANT_GO_THERE]);
		return;
	case IDI_HR1_MSG_DONT_HAVE_IT:
		_display->printString(_strings[IDI_HR1_STR_DONT_HAVE_IT]);
		return;
	case IDI_MSG_DONT_UNDERSTAND:
		_display->printString(_strings[IDI_HR1_STR_DONT_UNDERSTAND]);
		return;
	case IDI_HR1_MSG_GETTING_DARK:
		_display->printString(_strings[IDI_HR1_STR_GETTING_DARK]);
		return;
	}

	AdlEngine::printMessage(idx, wait);
}

uint HiRes1Engine::getEngineMessage(EngineMessage msg) {
	switch (msg) {
	case IDI_MSG_CANT_GO_THERE:
		return IDI_HR1_MSG_CANT_GO_THERE;
	case IDI_MSG_DONT_UNDERSTAND:
		return IDI_HR1_MSG_DONT_UNDERSTAND;
	case IDI_MSG_ITEM_DOESNT_MOVE:
		return IDI_HR1_MSG_ITEM_DOESNT_MOVE;
	case IDI_MSG_ITEM_NOT_HERE:
		return IDI_HR1_MSG_ITEM_NOT_HERE;
	case IDI_MSG_THANKS_FOR_PLAYING:
		return IDI_HR1_MSG_THANKS_FOR_PLAYING;
	default:
		error("Cannot find engine message %i", msg);
	}
}

AdlEngine *HiRes1Engine__create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes1Engine(syst, gd);
}

} // End of namespace Adl
