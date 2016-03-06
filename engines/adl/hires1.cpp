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

#include "common/system.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/file.h"
#include "common/stream.h"

#include "adl/hires1.h"
#include "adl/display.h"

namespace Adl {

#define IDS_HR1_EXE_0    "AUTO LOAD OBJ"
#define IDS_HR1_EXE_1    "ADVENTURE"
#define IDS_HR1_LOADER   "MYSTERY.HELLO"
#define IDS_HR1_MESSAGES "MESSAGES"

#define IDI_HR1_NUM_ROOMS         41
#define IDI_HR1_NUM_PICS          98
#define IDI_HR1_NUM_VARS          20
#define IDI_HR1_NUM_ITEM_OFFSETS  21
#define IDI_HR1_NUM_MESSAGES     167

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
	IDI_HR1_STR_PRESS_RETURN,

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
	{ IDI_HR1_STR_GETTING_DARK,    0x6c7c },
	{ IDI_HR1_STR_PRESS_RETURN,    0x5f68 }
};

#define IDI_HR1_OFS_PD_TEXT_0    0x005d
#define IDI_HR1_OFS_PD_TEXT_1    0x012b
#define IDI_HR1_OFS_PD_TEXT_2    0x016d
#define IDI_HR1_OFS_PD_TEXT_3    0x0259

#define IDI_HR1_OFS_INTRO_TEXT   0x0066
#define IDI_HR1_OFS_GAME_OR_HELP 0x000f

#define IDI_HR1_OFS_LOGO_0       0x1003
#define IDI_HR1_OFS_LOGO_1       0x1800

#define IDI_HR1_OFS_ITEMS        0x0100
#define IDI_HR1_OFS_ROOMS        0x050a
#define IDI_HR1_OFS_PICS         0x4b00
#define IDI_HR1_OFS_CMDS_0       0x3c00
#define IDI_HR1_OFS_CMDS_1       0x3d00

#define IDI_HR1_OFS_ITEM_OFFSETS 0x68ff
#define IDI_HR1_OFS_LINE_ART     0x4f00

#define IDI_HR1_OFS_VERBS        0x3800
#define IDI_HR1_OFS_NOUNS        0x0f00

HiRes1Engine::HiRes1Engine(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine(syst, gd) {
}

void HiRes1Engine::runIntro() {
	Common::File file;

	if (!file.open(IDS_HR1_EXE_0))
		error("Failed to open file '" IDS_HR1_EXE_0 "'");

	file.seek(IDI_HR1_OFS_LOGO_0);
	_display->setMode(DISPLAY_MODE_HIRES);
	_display->loadFrameBuffer(file);
	_display->updateHiResScreen();
	delay(4000);

	if (shouldQuit())
		return;

	_display->setMode(DISPLAY_MODE_TEXT);

	Common::File basic;
	if (!basic.open(IDS_HR1_LOADER))
		error("Failed to open file '" IDS_HR1_LOADER "'");

	Common::String str;

	basic.seek(IDI_HR1_OFS_PD_TEXT_0);
	str = readString(basic, '"');
	printASCIIString(str + '\r');

	basic.seek(IDI_HR1_OFS_PD_TEXT_1);
	str = readString(basic, '"');
	printASCIIString(str + "\r\r");

	basic.seek(IDI_HR1_OFS_PD_TEXT_2);
	str = readString(basic, '"');
	printASCIIString(str + "\r\r");

	basic.seek(IDI_HR1_OFS_PD_TEXT_3);
	str = readString(basic, '"');
	printASCIIString(str + '\r');

	inputKey();
	if (g_engine->shouldQuit())
		return;

	_display->setMode(DISPLAY_MODE_MIXED);

	file.seek(IDI_HR1_OFS_GAME_OR_HELP);
	str = readString(file);

	bool instructions = false;

	while (1) {
		_display->printString(str);
		Common::String s = inputString();

		if (g_engine->shouldQuit())
			break;

		if (s.empty())
			continue;

		if (s[0] == APPLECHAR('I')) {
			instructions = true;
			break;
		} else if (s[0] == APPLECHAR('G')) {
			break;
		}
	};

	if (instructions) {
		_display->setMode(DISPLAY_MODE_TEXT);
		file.seek(IDI_HR1_OFS_INTRO_TEXT);

		const uint pages[] = { 6, 6, 4, 5, 8, 7, 0 };

		uint page = 0;
		while (pages[page] != 0) {
			_display->home();
			printStrings(file, pages[page++]);
			inputString();

			if (g_engine->shouldQuit())
				return;

			file.seek(9, SEEK_CUR);
		}
	}

	printASCIIString("\r");

	file.close();

	_display->setMode(DISPLAY_MODE_MIXED);

	if (!file.open(IDS_HR1_EXE_1))
		error("Failed to open file '" IDS_HR1_EXE_1 "'");

	// Title screen shown during loading
	file.seek(IDI_HR1_OFS_LOGO_1);
	_display->loadFrameBuffer(file);
	_display->updateHiResScreen();
	delay(2000);
}

void HiRes1Engine::drawPic(Common::ReadStream &stream, const Common::Point &pos) const {
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

		x += pos.x;
		y += pos.y;

		if (y > 160)
			y = 160;

		if (bNewLine) {
			_display->putPixel(Common::Point(x, y), 0x7f);
			bNewLine = false;
		} else {
			drawLine(Common::Point(oldX, oldY), Common::Point(x, y), 0x7f);
		}

		oldX = x;
		oldY = y;
	}
}

void HiRes1Engine::drawPic(byte pic, Common::Point pos) const {
	Common::File f;
	Common::String name = Common::String::format("BLOCK%i", _pictures[pic].block);

	if (!f.open(name))
		error("Failed to open file '%s'", name.c_str());

	f.seek(_pictures[pic].offset);
	drawPic(f, pos);
}

void HiRes1Engine::initState() {
	Common::File f;

	_state.room = 1;
	_state.moves = 0;
	_state.isDark = false;

	_state.vars.clear();
	_state.vars.resize(IDI_HR1_NUM_VARS);

	if (!f.open(IDS_HR1_EXE_1))
		error("Failed to open file '" IDS_HR1_EXE_1 "'");

	// Load room data from executable
	_state.rooms.clear();
	f.seek(IDI_HR1_OFS_ROOMS);
	for (uint i = 0; i < IDI_HR1_NUM_ROOMS; ++i) {
		Room room;
		f.readByte();
		room.description = f.readByte();
		for (uint j = 0; j < 6; ++j)
			room.connections[j] = f.readByte();
		room.picture = f.readByte();
		room.curPicture = f.readByte();
		_state.rooms.push_back(room);
	}

	// Load item data from executable
	_state.items.clear();
	f.seek(IDI_HR1_OFS_ITEMS);
	while (f.readByte() != 0xff) {
		Item item;
		item.noun = f.readByte();
		item.room = f.readByte();
		item.picture = f.readByte();
		item.isLineArt = f.readByte();
		item.position.x = f.readByte();
		item.position.y = f.readByte();
		item.state = f.readByte();
		item.description = f.readByte();

		f.readByte();

		byte size = f.readByte();

		for (uint i = 0; i < size; ++i)
			item.roomPictures.push_back(f.readByte());

		_state.items.push_back(item);
	}
}

void HiRes1Engine::restartGame() {
	initState();
	_display->printString(_strings[IDI_HR1_STR_PRESS_RETURN]);
	inputString(); // Missing in the original
	printASCIIString("\r\r\r\r\r");
}

void HiRes1Engine::loadData() {
	Common::File f;

	if (!f.open(IDS_HR1_MESSAGES))
		error("Failed to open file '" IDS_HR1_MESSAGES "'");

	for (uint i = 0; i < IDI_HR1_NUM_MESSAGES; ++i)
		_messages.push_back(readString(f, APPLECHAR('\r')) + APPLECHAR('\r'));

	f.close();

	if (!f.open(IDS_HR1_EXE_1))
		error("Failed to open file '" IDS_HR1_EXE_1 "'");

	// Load strings from executable
	_strings.resize(IDI_HR1_STR_TOTAL);
	for (uint idx = 0; idx < IDI_HR1_STR_TOTAL; ++idx) {
		f.seek(stringOffsets[idx].offset);
		_strings[stringOffsets[idx].stringIdx] = readString(f);
	}

	// Load picture data from executable
	f.seek(IDI_HR1_OFS_PICS);
	for (uint i = 0; i < IDI_HR1_NUM_PICS; ++i) {
		struct Picture pic;
		pic.block = f.readByte();
		pic.offset = f.readUint16LE();
		_pictures.push_back(pic);
	}

	// Load commands from executable
	f.seek(IDI_HR1_OFS_CMDS_1);
	readCommands(f, _roomCommands);

	f.seek(IDI_HR1_OFS_CMDS_0);
	readCommands(f, _globalCommands);

	// Load dropped item offsets
	f.seek(IDI_HR1_OFS_ITEM_OFFSETS);
	for (uint i = 0; i < IDI_HR1_NUM_ITEM_OFFSETS; ++i) {
		Common::Point p;
		p.x = f.readByte();
		p.y = f.readByte();
		_itemOffsets.push_back(p);
	}

	// Load right-angle line art
	f.seek(IDI_HR1_OFS_LINE_ART);
	uint16 lineArtTotal = f.readUint16LE();
	for (uint i = 0; i < lineArtTotal; ++i) {
		f.seek(IDI_HR1_OFS_LINE_ART + 2 + i * 2);
		uint16 offset = f.readUint16LE();
		f.seek(IDI_HR1_OFS_LINE_ART + offset);

		Common::Array<byte> lineArt;
		byte b = f.readByte();
		while (b != 0) {
			lineArt.push_back(b);
			b = f.readByte();
		}
		_lineArt.push_back(lineArt);
	}

	if (f.eos() || f.err())
		error("Failed to read game data from '" IDS_HR1_EXE_1 "'");

	f.seek(IDI_HR1_OFS_VERBS);
	loadWords(f, _verbs);

	f.seek(IDI_HR1_OFS_NOUNS);
	loadWords(f, _nouns);
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
	case IDI_HR1_MSG_DONT_UNDERSTAND:
		_display->printString(_strings[IDI_HR1_STR_DONT_UNDERSTAND]);
		return;
	case IDI_HR1_MSG_GETTING_DARK:
		_display->printString(_strings[IDI_HR1_STR_GETTING_DARK]);
		return;
	}

	AdlEngine::printMessage(idx, wait);
}

uint HiRes1Engine::getEngineMessage(EngineMessage msg) const {
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

void HiRes1Engine::drawLine(const Common::Point &p1, const Common::Point &p2, byte color) const {
	// This draws a four-connected line

	int16 deltaX = p2.x - p1.x;
	int8 xStep = 1;

	if (deltaX < 0) {
		deltaX = -deltaX;
		xStep = -1;
	}

	int16 deltaY = p2.y - p1.y;
	int8 yStep = -1;

	if (deltaY > 0) {
		deltaY = -deltaY;
		yStep = 1;
	}

	Common::Point p(p1);
	int16 steps = deltaX - deltaY + 1;
	int16 err = deltaX + deltaY;

	while (1) {
		_display->putPixel(p, color);

		if (--steps == 0)
			return;

		if (err < 0) {
			p.y += yStep;
			err += deltaX;
		} else {
			p.x += xStep;
			err += deltaY;
		}
	}
}

AdlEngine *HiRes1Engine__create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes1Engine(syst, gd);
}

} // End of namespace Adl
